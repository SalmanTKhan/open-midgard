#include "FrameCapture.h"

#include "CaptureConfig.h"
#include "WebmRecorder.h"
#include "WebpEncoder.h"

#include "DebugLog.h"

#include <chrono>
#include <mutex>
#include <utility>

namespace capture {

namespace {

struct State {
    std::mutex              mutex;
    Config                  config;
    BackendReadbackFn       readback;
    BackendArmFn            arm;
    WebmRecorder            recorder;
    std::chrono::steady_clock::time_point recordingStart;
    bool                    initialised = false;
    // Screenshot is deferred to the next presented frame once the backend is
    // armed, because the first post-arm Present is when the readback ring
    // starts producing valid bytes.
    int                     screenshotPending = 0;  // frames remaining after arm
    bool                    recordingPendingStart = false;  // arm fired, waiting on first frame
};

State& S()
{
    static State s;
    return s;
}

int64_t MicrosSinceRecordingStart(const State& s)
{
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now() - s.recordingStart).count();
}

}  // namespace

void Init(const std::filesystem::path& runtimeRoot)
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    s.config = LoadConfig(runtimeRoot);
    s.initialised = true;
    DbgLog("[Capture] init: screenshots=%s recordings=%s\n",
        s.config.screenshotDir.string().c_str(),
        s.config.recordingDir.string().c_str());
}

void SetBackendReadback(BackendReadbackFn fn)
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    s.readback = std::move(fn);
}

void SetBackendArm(BackendArmFn fn)
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    s.arm = std::move(fn);
}

namespace {

struct FitResult {
    int width;
    int height;
};

// Aspect-preserving fit within (maxW, maxH). Non-positive max values mean
// "no limit on that axis"; if both are non-positive the input passes through.
FitResult FitWithin(int srcW, int srcH, int maxW, int maxH)
{
    FitResult out{ srcW, srcH };
    if (maxW > 0 && out.width > maxW) {
        out.height = static_cast<int>(
            static_cast<int64_t>(out.height) * maxW / out.width);
        out.width = maxW;
    }
    if (maxH > 0 && out.height > maxH) {
        out.width = static_cast<int>(
            static_cast<int64_t>(out.width) * maxH / out.height);
        out.height = maxH;
    }
    // Round to even for VP8.
    out.width  &= ~1;
    out.height &= ~1;
    if (out.width  < 2) out.width  = 2;
    if (out.height < 2) out.height = 2;
    return out;
}

bool StartRecorderFromFrameLocked(State& s, const FrameView& frame)
{
    if (!EnsureDirectory(s.config.recordingDir)) {
        DbgLog("[Capture] cannot create recording dir: %s\n",
            s.config.recordingDir.string().c_str());
        return false;
    }

    const FitResult fit = FitWithin(frame.width, frame.height,
                                    s.config.recordingMaxWidth,
                                    s.config.recordingMaxHeight);
    const auto outPath = BuildTimestampedPath(s.config.recordingDir, "open-midgard", "webm");
    if (!s.recorder.Start(outPath, frame.width, frame.height, frame.format,
                          fit.width, fit.height,
                          s.config.webmTargetFps, s.config.webmBitrateKbps)) {
        DbgLog("[Capture] recorder start failed: %s\n", outPath.string().c_str());
        return false;
    }

    DbgLog("[Capture] recording started: %s (in=%dx%d out=%dx%d @ %d fps, %d kbps)\n",
        outPath.string().c_str(), frame.width, frame.height,
        fit.width, fit.height,
        s.config.webmTargetFps, s.config.webmBitrateKbps);

    s.recorder.PushFrame(frame.pixels, frame.strideBytes, 0);
    s.recordingPendingStart = false;
    return true;
}

// Shared writer; caller must hold s.mutex. Returns output path on success.
std::filesystem::path SaveScreenshotLocked(State& s)
{
    if (!s.readback) {
        DbgLog("[Capture] screenshot: no backend readback registered\n");
        return {};
    }

    FrameView frame;
    if (!s.readback(&frame) || !frame.pixels || frame.width <= 0 || frame.height <= 0) {
        DbgLog("[Capture] screenshot: backend readback returned no frame\n");
        return {};
    }

    if (!EnsureDirectory(s.config.screenshotDir)) {
        DbgLog("[Capture] cannot create screenshot dir: %s\n",
            s.config.screenshotDir.string().c_str());
        return {};
    }

    const auto outPath = BuildTimestampedPath(s.config.screenshotDir, "open-midgard", "webp");
    if (!EncodeWebp(frame.pixels, frame.width, frame.height, frame.strideBytes,
                    frame.format, s.config.webpQuality, outPath)) {
        DbgLog("[Capture] WebP encode failed for %s\n", outPath.string().c_str());
        return {};
    }

    DbgLog("[Capture] screenshot saved: %s\n", outPath.string().c_str());
    return outPath;
}

}  // namespace

std::filesystem::path RequestScreenshot()
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.initialised) {
        DbgLog("[Capture] screenshot requested before init\n");
        return {};
    }

    // If a recording is already running the ring is already armed and the
    // readable slot is fresh — capture immediately.
    if (s.recorder.IsRunning()) {
        return SaveScreenshotLocked(s);
    }

    // Otherwise arm the backend and defer one frame so the readback ring has
    // valid bytes. OnFramePresented will save and disarm.
    if (s.arm) {
        s.arm(true);
    }
    // Two frames: one to fill a ring slot, one to become readable after the
    // next fence wait. Anything ≥1 works with the current ring design but 2 is
    // the safe number independent of how the ring is implemented.
    s.screenshotPending = 2;
    return {};
}

bool ToggleRecording()
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.initialised) {
        DbgLog("[Capture] recording toggle before init\n");
        return false;
    }

    if (s.recorder.IsRunning()) {
        const bool ok = s.recorder.Stop();
        DbgLog("[Capture] recording stopped (ok=%d)\n", ok ? 1 : 0);
        if (s.arm && s.screenshotPending == 0) {
            s.arm(false);  // nothing else wants the ring
        }
        return ok;
    }

    if (!s.readback) {
        DbgLog("[Capture] recording requested but no backend readback is registered\n");
        return false;
    }

    // Arm the backend so the ring starts refilling. We need at least one
    // post-arm frame in the ring before we can read dimensions, so probe
    // lazily on the first OnFramePresented — that path calls
    // StartRecorderIfPendingLocked which knows the real input size.
    if (s.arm) {
        s.arm(true);
    }
    s.recordingStart = std::chrono::steady_clock::now();

    // Try a probe in case the ring already has data (e.g. the user stopped
    // and immediately restarted a recording, or a screenshot had just armed
    // the ring). Otherwise OnFramePresented will start the recorder next
    // frame via the same code path.
    FrameView probe;
    if (s.readback(&probe) && probe.width > 0 && probe.height > 0) {
        return StartRecorderFromFrameLocked(s, probe);
    }

    // Mark "recording requested but not yet started"; OnFramePresented will
    // complete the start once a frame is available.
    s.recordingPendingStart = true;
    DbgLog("[Capture] recording armed; will start on next frame\n");
    return true;
}

void OnFramePresented()
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    if (!s.readback) {
        return;
    }

    const bool wantsWork =
        s.recorder.IsRunning() ||
        s.recordingPendingStart ||
        s.screenshotPending > 0;
    if (!wantsWork) {
        return;
    }

    // Handle pending screenshot first — the warmup frame count decrements
    // each presented frame; 0 means "capture this one".
    if (s.screenshotPending > 0) {
        if (--s.screenshotPending == 0) {
            SaveScreenshotLocked(s);
            if (s.arm && !s.recorder.IsRunning() && !s.recordingPendingStart) {
                s.arm(false);
            }
        }
    }

    // Kick off the recorder if we were waiting for a post-arm frame.
    if (s.recordingPendingStart) {
        FrameView probe;
        if (s.readback(&probe) && probe.width > 0 && probe.height > 0) {
            StartRecorderFromFrameLocked(s, probe);
        }
        return;
    }

    if (s.recorder.IsRunning()) {
        FrameView frame;
        if (!s.readback(&frame) || !frame.pixels) {
            return;
        }
        s.recorder.PushFrame(frame.pixels, frame.strideBytes, MicrosSinceRecordingStart(s));
    }
}

bool IsRecording()
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    return s.recorder.IsRunning();
}

Status GetStatus()
{
    State& s = S();
    std::lock_guard<std::mutex> lock(s.mutex);
    Status out;
    if (!s.recorder.IsRunning()) {
        return out;
    }
    out.recording     = true;
    out.elapsedMillis = MicrosSinceRecordingStart(s) / 1000;
    out.encodedFrames = s.recorder.EncodedFrameCount();
    out.droppedFrames = s.recorder.DroppedFrameCount();
    out.outputWidth   = s.recorder.OutputWidth();
    out.outputHeight  = s.recorder.OutputHeight();
    // Bitrate is target kbps; forecast bytes = bitrate × seconds / 8.
    const int64_t kbps = s.recorder.BitrateKbps();
    if (kbps > 0 && out.elapsedMillis > 0) {
        out.estimatedBytes = static_cast<uint64_t>(kbps) * 1000ull
            * static_cast<uint64_t>(out.elapsedMillis) / 8000ull;
    }
    return out;
}

}  // namespace capture

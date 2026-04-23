#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>

#include "FrameCapture.h"  // PixelFormat

namespace capture {

class WebmRecorderImpl;

// Thin wrapper around libvpx (VP8) + libwebm (mkvmuxer) for silent WebM output.
// One recorder encodes one file from Start() to Stop(). Frames are expected in
// RGBA8 top-down, same width/height across the whole recording. Frames are
// resampled to `targetFps` by dropping or duplicating based on the monotonic
// timestamp supplied to PushFrame.
class WebmRecorder {
public:
    WebmRecorder();
    ~WebmRecorder();

    WebmRecorder(const WebmRecorder&) = delete;
    WebmRecorder& operator=(const WebmRecorder&) = delete;

    // inWidth/inHeight describe the frames PushFrame will deliver. inFormat
    // is the packed pixel layout for those frames (RGBA or BGRA).
    // outWidth/outHeight are the encoded dimensions — sws_scale downscales
    // (or passes through if equal) as part of the conversion to YUV420.
    bool Start(const std::filesystem::path& outPath,
               int inWidth, int inHeight, PixelFormat inFormat,
               int outWidth, int outHeight,
               int targetFps, int bitrateKbps);

    // pixels is inWidth*inHeight*4 bytes (stride = inWidth*4 minimum). pts is
    // in microseconds relative to Start(). Returns true on success (including
    // dropped-due-to-fps).
    bool PushFrame(const uint8_t* pixels, int stride, int64_t ptsMicros);

    // Accessors for the overlay / status reporting. Safe to call from any
    // thread; return a best-effort snapshot.
    uint64_t EncodedFrameCount() const;
    uint64_t DroppedFrameCount() const;
    int64_t  BitrateKbps() const;
    int      OutputWidth() const;
    int      OutputHeight() const;

    bool Stop();

    bool IsRunning() const;

private:
    std::unique_ptr<WebmRecorderImpl> m_impl;
};

}  // namespace capture

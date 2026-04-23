#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

namespace capture {

enum class PixelFormat {
    Rgba8,  // bytes: R, G, B, A
    Bgra8,  // bytes: B, G, R, A  (matches Vulkan's common VK_FORMAT_B8G8R8A8_*)
};

struct FrameView {
    const uint8_t* pixels = nullptr;  // top-down, packed according to `format`
    int width = 0;
    int height = 0;
    int strideBytes = 0;
    PixelFormat format = PixelFormat::Rgba8;
};

// Fills `outFrame` with the most recently presented backbuffer. Backends wire
// an implementation via SetBackendReadback(); if no backend registered, returns
// false and capture hotkeys surface a "backend readback unavailable" log line.
// The returned `pixels` must remain valid until the callback returns — the
// capture code will copy/encode before the readback buffer is reused.
using BackendReadbackFn = std::function<bool(FrameView* outFrame)>;

// Toggles the backend's per-frame swapchain readback copy. When disarmed the
// backend pays zero capture cost; when armed it refills the readback ring
// every frame. Expected to be cheap (an atomic store).
using BackendArmFn = std::function<void(bool armed)>;

// Register the backend-specific readback. Call once from the active render
// backend after it has its swapchain and a staging buffer ready. Pass {} to
// clear on shutdown.
void SetBackendReadback(BackendReadbackFn fn);

// Register the backend-specific arm toggle. Optional — if the backend does
// not set one, FrameCapture assumes readback is always available (legacy
// behavior) and runs at elevated idle cost. Pass {} to clear on shutdown.
void SetBackendArm(BackendArmFn fn);

// Must be called once at startup, before any hotkey dispatch. runtimeRoot is
// used to resolve default capture directories.
void Init(const std::filesystem::path& runtimeRoot);

// One-shot screenshot: grab the current backbuffer and encode to WebP.
// Returns the output path on success, empty path on failure.
std::filesystem::path RequestScreenshot();

// Toggle video recording. First call starts a new .webm; second call stops
// and finalises the file. Returns true if the state changed.
bool ToggleRecording();

// Called from the main loop (after the render backend presents a frame).
// Pushes a frame into the active recorder if recording is in progress. No-op
// if not recording. Safe to call every frame.
void OnFramePresented();

bool IsRecording();

// Lightweight snapshot of the active recording, for UI overlays. When
// `recording` is false the other fields are meaningless. `estimatedBytes` is
// a rough forecast based on bitrate × elapsed time — it will be within a few
// percent of the final file for typical VP8 VBR content, but don't treat it
// as byte-exact.
struct Status {
    bool     recording = false;
    int64_t  elapsedMillis = 0;
    uint64_t encodedFrames = 0;
    uint64_t droppedFrames = 0;
    uint64_t estimatedBytes = 0;
    int      outputWidth = 0;
    int      outputHeight = 0;
};

Status GetStatus();

}  // namespace capture

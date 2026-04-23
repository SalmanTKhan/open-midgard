#pragma once

#include <filesystem>
#include <string>

namespace capture {

struct Config {
    std::filesystem::path screenshotDir;
    std::filesystem::path recordingDir;
    int webpQuality = 90;            // 0-100
    int webmBitrateKbps = 6000;
    int webmTargetFps = 30;
    // Aspect-preserving cap applied to the recording output resolution.
    // Zero or negative on either axis disables that axis's limit.
    int recordingMaxWidth  = 1280;
    int recordingMaxHeight = 720;
};

// Load from [Capture] section of open-midgard.ini, falling back to defaults
// under the runtime root (resolved by ResolveRuntimeRoot in WinMain).
// runtimeRoot must be an absolute path; used to build default capture dirs.
Config LoadConfig(const std::filesystem::path& runtimeRoot);

// Ensures the directory exists, creating it lazily. Returns true on success.
bool EnsureDirectory(const std::filesystem::path& dir);

// Builds "{stem}_YYYYMMDD_HHMMSS.{ext}" in `dir`. ext must omit the dot.
std::filesystem::path BuildTimestampedPath(const std::filesystem::path& dir,
                                           const char* stem,
                                           const char* ext);

}  // namespace capture

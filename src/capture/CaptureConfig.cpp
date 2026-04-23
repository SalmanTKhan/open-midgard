#include "CaptureConfig.h"

#include "core/SettingsIni.h"

#include <chrono>
#include <cstdio>
#include <ctime>
#include <system_error>

namespace capture {

namespace {

constexpr const char* kSection = "Capture";

std::filesystem::path ResolveDir(const char* key,
                                 const std::filesystem::path& runtimeRoot,
                                 const char* defaultSubdir)
{
    std::string value;
    if (TryLoadSettingsIniString(kSection, key, &value) && !value.empty()) {
        std::filesystem::path p(value);
        if (p.is_relative()) {
            p = runtimeRoot / p;
        }
        return p;
    }

    const std::filesystem::path def = runtimeRoot / defaultSubdir;
    SaveSettingsIniString(kSection, key, def.string());
    return def;
}

int ResolveInt(const char* key, int defaultValue, int minValue, int maxValue)
{
    const int raw = LoadSettingsIniInt(kSection, key, defaultValue);
    if (raw < minValue) return minValue;
    if (raw > maxValue) return maxValue;
    return raw;
}

// Resolve an integer that allows non-positive values to mean "no limit".
int ResolveIntAllowZero(const char* key, int defaultValue, int maxValue)
{
    const int raw = LoadSettingsIniInt(kSection, key, defaultValue);
    if (raw <= 0) return 0;
    if (raw > maxValue) return maxValue;
    return raw;
}

}  // namespace

Config LoadConfig(const std::filesystem::path& runtimeRoot)
{
    Config cfg;
    cfg.screenshotDir   = ResolveDir("ScreenshotDir", runtimeRoot, "screenshots");
    cfg.recordingDir    = ResolveDir("RecordingDir", runtimeRoot, "recordings");
    cfg.webpQuality        = ResolveInt("WebpQuality", 90, 0, 100);
    cfg.webmBitrateKbps    = ResolveInt("WebmBitrateKbps", 6000, 200, 100000);
    cfg.webmTargetFps      = ResolveInt("WebmTargetFps", 30, 1, 120);
    cfg.recordingMaxWidth  = ResolveIntAllowZero("RecordingMaxWidth",  1280, 7680);
    cfg.recordingMaxHeight = ResolveIntAllowZero("RecordingMaxHeight",  720, 4320);
    return cfg;
}

bool EnsureDirectory(const std::filesystem::path& dir)
{
    if (dir.empty()) {
        return false;
    }
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    return !ec || std::filesystem::exists(dir, ec);
}

std::filesystem::path BuildTimestampedPath(const std::filesystem::path& dir,
                                           const char* stem,
                                           const char* ext)
{
    const auto now = std::chrono::system_clock::now();
    const std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_{};
#if defined(_WIN32)
    localtime_s(&tm_, &t);
#else
    localtime_r(&t, &tm_);
#endif
    char buf[64] = {};
    std::snprintf(buf, sizeof(buf), "%s_%04d%02d%02d_%02d%02d%02d.%s",
                  stem ? stem : "capture",
                  tm_.tm_year + 1900, tm_.tm_mon + 1, tm_.tm_mday,
                  tm_.tm_hour, tm_.tm_min, tm_.tm_sec,
                  ext ? ext : "bin");
    return dir / buf;
}

}  // namespace capture

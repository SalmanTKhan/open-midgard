#pragma once
// Minimal debug logger – writes to a per-process log file in the working directory.
// Each call also writes to a per-tag file (debug_hp_{PID}_<tag>.log) so noisy tags
// can be filtered out of the combined log without losing them.
#include "platform/WindowsCompat.h"
#include <cstdio>
#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

inline bool& DbgLogConsoleEnabled()
{
    static bool enabled = false;
    return enabled;
}

inline void EnableDbgLogConsole(bool enabled)
{
#if RO_PLATFORM_WINDOWS
    if (!enabled) {
        return;
    }

    static bool initialized = false;
    if (!initialized) {
        if (!GetConsoleWindow()) {
            if (!AllocConsole()) {
                return;
            }
        }

        FILE* stream = nullptr;
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);
        freopen_s(&stream, "CONIN$", "r", stdin);
        setvbuf(stdout, nullptr, _IONBF, 0);
        setvbuf(stderr, nullptr, _IONBF, 0);
        initialized = true;
    }

    DbgLogConsoleEnabled() = true;
#else
    (void)enabled;
#endif
}

namespace ro_debuglog_detail {

struct Sink {
    std::mutex mutex;
    std::unordered_map<std::string, FILE*> files; // key "" = combined log
    std::unordered_set<std::string> disabled;
    std::unordered_set<std::string> combinedDisabled;
    bool perTagEnabled = true;
    bool combinedEnabled = true;
    bool atexitRegistered = false;

    static Sink& Get()
    {
        static Sink instance;
        return instance;
    }

    static void CloseAll()
    {
        Sink& s = Get();
        std::lock_guard<std::mutex> g(s.mutex);
        for (auto& kv : s.files) {
            if (kv.second) {
                std::fclose(kv.second);
                kv.second = nullptr;
            }
        }
    }
};

inline std::string LowerRange(const char* begin, std::size_t len)
{
    std::string out;
    out.reserve(len);
    for (std::size_t i = 0; i < len; ++i) {
        const unsigned char c = static_cast<unsigned char>(begin[i]);
        out.push_back(static_cast<char>(std::tolower(c)));
    }
    return out;
}

inline std::string ExtractTagLower(const char* msg)
{
    if (!msg || msg[0] != '[') {
        return {};
    }
    const char* end = std::strchr(msg + 1, ']');
    if (!end) {
        return {};
    }
    const std::size_t len = static_cast<std::size_t>(end - (msg + 1));
    if (len == 0) {
        return {};
    }
    return LowerRange(msg + 1, len);
}

inline void AssignCsv(std::unordered_set<std::string>& dst, const char* csv)
{
    dst.clear();
    if (!csv) {
        return;
    }
    const char* p = csv;
    while (*p) {
        while (*p == ' ' || *p == ',' || *p == '\t') {
            ++p;
        }
        const char* start = p;
        while (*p && *p != ',') {
            ++p;
        }
        const char* end = p;
        while (end > start && (end[-1] == ' ' || end[-1] == '\t')) {
            --end;
        }
        if (end > start) {
            dst.insert(LowerRange(start, static_cast<std::size_t>(end - start)));
        }
    }
}

inline FILE* OpenOrGetLocked(Sink& s, const std::string& key, const char* path)
{
    auto it = s.files.find(key);
    if (it != s.files.end() && it->second) {
        return it->second;
    }
    FILE* f = nullptr;
    if (fopen_s(&f, path, "ab") == 0 && f) {
        if (!s.atexitRegistered) {
            std::atexit(&Sink::CloseAll);
            s.atexitRegistered = true;
        }
        s.files[key] = f;
        return f;
    }
    return nullptr;
}

} // namespace ro_debuglog_detail

inline void SetDbgLogPerTagFilesEnabled(bool enabled)
{
    auto& s = ro_debuglog_detail::Sink::Get();
    std::lock_guard<std::mutex> g(s.mutex);
    s.perTagEnabled = enabled;
}

inline void SetDbgLogCombinedFileEnabled(bool enabled)
{
    auto& s = ro_debuglog_detail::Sink::Get();
    std::lock_guard<std::mutex> g(s.mutex);
    s.combinedEnabled = enabled;
}

inline void SetDbgLogDisabledTags(const char* csv)
{
    auto& s = ro_debuglog_detail::Sink::Get();
    std::lock_guard<std::mutex> g(s.mutex);
    ro_debuglog_detail::AssignCsv(s.disabled, csv);
}

inline void SetDbgLogCombinedDisabledTags(const char* csv)
{
    auto& s = ro_debuglog_detail::Sink::Get();
    std::lock_guard<std::mutex> g(s.mutex);
    ro_debuglog_detail::AssignCsv(s.combinedDisabled, csv);
}

inline void DbgLog(const char* fmt, ...)
{
    char msg[1024];
    va_list args;
    va_start(args, fmt);
    _vsnprintf_s(msg, sizeof(msg), _TRUNCATE, fmt, args);
    va_end(args);

    const DWORD pid = GetCurrentProcessId();
    const DWORD tick = GetTickCount();

    char line[1152];
    _snprintf_s(line, sizeof(line), _TRUNCATE, "[pid:%lu t:%lu] %s",
        static_cast<unsigned long>(pid),
        static_cast<unsigned long>(tick),
        msg);

    const std::string tag = ro_debuglog_detail::ExtractTagLower(msg);

    auto& sink = ro_debuglog_detail::Sink::Get();
    std::lock_guard<std::mutex> g(sink.mutex);

    if (!tag.empty() && sink.disabled.count(tag) != 0) {
        return;
    }

    // Always mirror to debugger output (visible in VS Output / debug console).
    OutputDebugStringA(line);

    const bool inCombinedDisabled = !tag.empty() && sink.combinedDisabled.count(tag) != 0;
    if (sink.combinedEnabled && !inCombinedDisabled) {
        char combinedPath[MAX_PATH];
        _snprintf_s(combinedPath, sizeof(combinedPath), _TRUNCATE, "debug_hp_%lu.log",
            static_cast<unsigned long>(pid));
        if (FILE* f = ro_debuglog_detail::OpenOrGetLocked(sink, std::string(), combinedPath)) {
            std::fputs(line, f);
            std::fflush(f);
        }
    }

    if (sink.perTagEnabled && !tag.empty()) {
        char tagPath[MAX_PATH];
        _snprintf_s(tagPath, sizeof(tagPath), _TRUNCATE, "debug_hp_%lu_%s.log",
            static_cast<unsigned long>(pid),
            tag.c_str());
        if (FILE* f = ro_debuglog_detail::OpenOrGetLocked(sink, tag, tagPath)) {
            std::fputs(line, f);
            std::fflush(f);
        }
    }

    if (DbgLogConsoleEnabled()) {
        std::fputs(line, stdout);
        std::fflush(stdout);
    }
}

#pragma once
// Minimal debug logger – writes to a per-process log file in the working directory.
// Include and call DbgLog(...) wherever needed. Remove file when done debugging.
#include "platform/WindowsCompat.h"
#include <cstdio>
#include <cstdarg>
#include <cstring>

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
    if (std::strncmp(msg, "[", 1) == 0) {
        _snprintf_s(line, sizeof(line), _TRUNCATE, "[pid:%lu t:%lu] %s",
            static_cast<unsigned long>(pid),
            static_cast<unsigned long>(tick),
            msg);
    } else {
        _snprintf_s(line, sizeof(line), _TRUNCATE, "[pid:%lu t:%lu] %s",
            static_cast<unsigned long>(pid),
            static_cast<unsigned long>(tick),
            msg);
    }

    // Write to debugger output (visible in VS Output window)
    OutputDebugStringA(line);

    // Also append to a per-process log file in the CWD.
    char logPath[MAX_PATH];
    _snprintf_s(logPath, sizeof(logPath), _TRUNCATE, "debug_hp_%lu.log",
        static_cast<unsigned long>(pid));

    FILE* f = nullptr;
    if (fopen_s(&f, logPath, "a") == 0 && f)
    {
        fputs(line, f);
        fclose(f);
    }

    if (DbgLogConsoleEnabled()) {
        fputs(line, stdout);
        fflush(stdout);
    }
}

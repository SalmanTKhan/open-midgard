#pragma once

#include <windows.h>

#include <cstdio>
#include <cstring>

inline void FillPacketPadding(void* pad, int len)
{
    if (!pad || len <= 0) {
        return;
    }

    char buf[20] = {};
    const DWORD shifted = timeGetTime() >> len;
    const DWORD now = timeGetTime();
    std::snprintf(buf, sizeof(buf), "%lx", static_cast<unsigned long>(now ^ shifted));

    const size_t textLen = std::strlen(buf);
    const size_t fallbackIndex = textLen > 0 ? textLen - 1 : 0;
    unsigned char* const bytes = static_cast<unsigned char*>(pad);
    int srcIndex = static_cast<int>(textLen) - 1;
    for (int i = 0; i < len; ++i, --srcIndex) {
        const char ch = srcIndex >= 0 ? buf[srcIndex] : buf[fallbackIndex];
        bytes[i] = static_cast<unsigned char>(ch);
    }
    bytes[len - 1] = 0;
}
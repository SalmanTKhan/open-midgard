#pragma once

#include "windows.h"

using MMRESULT = UINT;

inline MMRESULT timeBeginPeriod(UINT)
{
    return 0;
}

inline MMRESULT timeEndPeriod(UINT)
{
    return 0;
}
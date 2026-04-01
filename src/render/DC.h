#pragma once

#include <windows.h>

void BlitMotionToArgb(unsigned int* dest, int destW, int destH, int baseX, int baseY, class CSprRes* sprRes, const struct CMotion* motion, unsigned int* palette);
bool DrawActMotionToHdc(HDC hdc, int x, int y, class CSprRes* sprRes, const struct CMotion* motion, unsigned int* palette);

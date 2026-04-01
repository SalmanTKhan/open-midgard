#pragma once

#include <windows.h>

void BlitMotionToArgb(unsigned int* dest, int destW, int destH, int baseX, int baseY, class CSprRes* sprRes, const struct CMotion* motion, unsigned int* palette);
bool AlphaBlendArgbToHdc(HDC hdc,
                         int dstX,
                         int dstY,
                         int dstWidth,
                         int dstHeight,
                         const unsigned int* pixels,
                         int pixelWidth,
                         int pixelHeight,
                         int srcX = 0,
                         int srcY = 0,
                         int srcWidth = -1,
                         int srcHeight = -1);
bool DrawActMotionToHdc(HDC hdc, int x, int y, class CSprRes* sprRes, const struct CMotion* motion, unsigned int* palette);

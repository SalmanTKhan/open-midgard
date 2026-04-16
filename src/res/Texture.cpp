#include "Texture.h"

#include "render3d/D3dutil.h"
#include "render3d/Device.h"
#include "render3d/GraphicsSettings.h"
#include "render3d/RenderDevice.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>
#include "../DebugLog.h"

namespace {

constexpr bool kLogTexture = false;

unsigned int GetTextureUpscaleFactor(bool allowUpscale, unsigned int minUpscaleFactor)
{
    if (!allowUpscale) {
        return 1u;
    }

    const unsigned int configuredFactor = static_cast<unsigned int>((std::max)(1, GetCachedGraphicsSettings().textureUpscaleFactor));
    return (std::max)(configuredFactor, (std::max)(1u, minUpscaleFactor));
}

unsigned int CountTrailingZeros(unsigned int mask)
{
    if (mask == 0u) {
        return 0u;
    }

    unsigned int shift = 0u;
    while ((mask & 1u) == 0u) {
        mask >>= 1u;
        ++shift;
    }
    return shift;
}

unsigned int CountBits(unsigned int mask)
{
    unsigned int bits = 0u;
    while (mask != 0u) {
        bits += mask & 1u;
        mask >>= 1u;
    }
    return bits;
}

unsigned int PackChannel(unsigned int value, unsigned int mask)
{
    if (mask == 0u) {
        return 0u;
    }

    const unsigned int shift = CountTrailingZeros(mask);
    const unsigned int bits = CountBits(mask);
    if (bits == 0u) {
        return 0u;
    }

    const unsigned int maxValue = (1u << bits) - 1u;
    const unsigned int scaled = (value * maxValue + 127u) / 255u;
    return (scaled << shift) & mask;
}

unsigned int ConvertArgbToSurfacePixel(unsigned int argb, const DDPIXELFORMAT& pf)
{
    const unsigned int alpha = (argb >> 24) & 0xFFu;
    const unsigned int red = (argb >> 16) & 0xFFu;
    const unsigned int green = (argb >> 8) & 0xFFu;
    const unsigned int blue = argb & 0xFFu;

    if (pf.dwRGBBitCount == 32
        && pf.dwRBitMask == 0x00FF0000u
        && pf.dwGBitMask == 0x0000FF00u
        && pf.dwBBitMask == 0x000000FFu
        && pf.dwRGBAlphaBitMask == 0xFF000000u) {
        return argb;
    }

    return PackChannel(alpha, pf.dwRGBAlphaBitMask)
        | PackChannel(red, pf.dwRBitMask)
        | PackChannel(green, pf.dwGBitMask)
        | PackChannel(blue, pf.dwBBitMask);
}

unsigned int GetSurfaceColorKey(const DDPIXELFORMAT& pf)
{
    return pf.dwRBitMask | pf.dwBBitMask;
}

float ClampUnitFloat(float value)
{
    if (value < 0.0f) {
        return 0.0f;
    }
    if (value > 1.0f) {
        return 1.0f;
    }
    return value;
}

unsigned char FloatToByte(float value)
{
    const float clamped = ClampUnitFloat(value);
    return static_cast<unsigned char>(clamped * 255.0f + 0.5f);
}

unsigned int LoadArgbPixel(const unsigned int* data, int srcWidth, int srcHeight, int srcPitch, int x, int y)
{
    if (!data || srcWidth <= 0 || srcHeight <= 0) {
        return 0u;
    }

    x = (std::max)(0, (std::min)(srcWidth - 1, x));
    y = (std::max)(0, (std::min)(srcHeight - 1, y));
    const unsigned char* base = reinterpret_cast<const unsigned char*>(data) + static_cast<size_t>(y) * static_cast<size_t>(srcPitch);
    const unsigned int* row = reinterpret_cast<const unsigned int*>(base);
    return row[x];
}

void DecodeArgb(unsigned int argb, float* alpha, float* red, float* green, float* blue)
{
    if (alpha) {
        *alpha = static_cast<float>((argb >> 24) & 0xFFu) / 255.0f;
    }
    if (red) {
        *red = static_cast<float>((argb >> 16) & 0xFFu) / 255.0f;
    }
    if (green) {
        *green = static_cast<float>((argb >> 8) & 0xFFu) / 255.0f;
    }
    if (blue) {
        *blue = static_cast<float>(argb & 0xFFu) / 255.0f;
    }
}

unsigned int EncodeArgb(float alpha, float red, float green, float blue)
{
    return (static_cast<unsigned int>(FloatToByte(alpha)) << 24)
        | (static_cast<unsigned int>(FloatToByte(red)) << 16)
        | (static_cast<unsigned int>(FloatToByte(green)) << 8)
        | static_cast<unsigned int>(FloatToByte(blue));
}

unsigned int SampleBilinearArgb(const unsigned int* data, int srcWidth, int srcHeight, int srcPitch, float sampleX, float sampleY)
{
    const float clampedX = (std::max)(0.0f, (std::min)(static_cast<float>(srcWidth - 1), sampleX));
    const float clampedY = (std::max)(0.0f, (std::min)(static_cast<float>(srcHeight - 1), sampleY));
    const int x0 = static_cast<int>(std::floor(clampedX));
    const int y0 = static_cast<int>(std::floor(clampedY));
    const int x1 = (std::min)(srcWidth - 1, x0 + 1);
    const int y1 = (std::min)(srcHeight - 1, y0 + 1);
    const float tx = clampedX - static_cast<float>(x0);
    const float ty = clampedY - static_cast<float>(y0);

    const unsigned int p00 = LoadArgbPixel(data, srcWidth, srcHeight, srcPitch, x0, y0);
    const unsigned int p10 = LoadArgbPixel(data, srcWidth, srcHeight, srcPitch, x1, y0);
    const unsigned int p01 = LoadArgbPixel(data, srcWidth, srcHeight, srcPitch, x0, y1);
    const unsigned int p11 = LoadArgbPixel(data, srcWidth, srcHeight, srcPitch, x1, y1);

    float a00 = 0.0f;
    float r00 = 0.0f;
    float g00 = 0.0f;
    float b00 = 0.0f;
    float a10 = 0.0f;
    float r10 = 0.0f;
    float g10 = 0.0f;
    float b10 = 0.0f;
    float a01 = 0.0f;
    float r01 = 0.0f;
    float g01 = 0.0f;
    float b01 = 0.0f;
    float a11 = 0.0f;
    float r11 = 0.0f;
    float g11 = 0.0f;
    float b11 = 0.0f;
    DecodeArgb(p00, &a00, &r00, &g00, &b00);
    DecodeArgb(p10, &a10, &r10, &g10, &b10);
    DecodeArgb(p01, &a01, &r01, &g01, &b01);
    DecodeArgb(p11, &a11, &r11, &g11, &b11);

    const float w00 = (1.0f - tx) * (1.0f - ty);
    const float w10 = tx * (1.0f - ty);
    const float w01 = (1.0f - tx) * ty;
    const float w11 = tx * ty;
    const float alpha = (a00 * w00) + (a10 * w10) + (a01 * w01) + (a11 * w11);
    const float red = (r00 * w00) + (r10 * w10) + (r01 * w01) + (r11 * w11);
    const float green = (g00 * w00) + (g10 * w10) + (g01 * w01) + (g11 * w11);
    const float blue = (b00 * w00) + (b10 * w10) + (b01 * w01) + (b11 * w11);
    return EncodeArgb(alpha, red, green, blue);
}

void BuildSupersampledUpscale(const unsigned int* srcData, int srcWidth, int srcHeight, int srcPitch,
    int scale, std::vector<unsigned int>* dstPixels)
{
    if (!srcData || srcWidth <= 0 || srcHeight <= 0 || scale <= 1 || !dstPixels) {
        return;
    }

    const int dstWidth = srcWidth * scale;
    const int dstHeight = srcHeight * scale;
    dstPixels->resize(static_cast<size_t>(dstWidth) * static_cast<size_t>(dstHeight));

    const float invScale = 1.0f / static_cast<float>(scale);
    static constexpr float kSubsampleOffsets[4][2] = {
        { -0.25f, -0.25f },
        {  0.25f, -0.25f },
        { -0.25f,  0.25f },
        {  0.25f,  0.25f },
    };

    for (int dstY = 0; dstY < dstHeight; ++dstY) {
        unsigned int* dstRow = dstPixels->data() + static_cast<size_t>(dstY) * static_cast<size_t>(dstWidth);
        const float baseY = ((static_cast<float>(dstY) + 0.5f) * invScale) - 0.5f;
        for (int dstX = 0; dstX < dstWidth; ++dstX) {
            const float baseX = ((static_cast<float>(dstX) + 0.5f) * invScale) - 0.5f;

            float accumAlpha = 0.0f;
            float accumRed = 0.0f;
            float accumGreen = 0.0f;
            float accumBlue = 0.0f;
            for (const float (&offset)[2] : kSubsampleOffsets) {
                const unsigned int sample = SampleBilinearArgb(
                    srcData,
                    srcWidth,
                    srcHeight,
                    srcPitch,
                    baseX + (offset[0] * invScale),
                    baseY + (offset[1] * invScale));
                float alpha = 0.0f;
                float red = 0.0f;
                float green = 0.0f;
                float blue = 0.0f;
                DecodeArgb(sample, &alpha, &red, &green, &blue);
                accumAlpha += alpha;
                accumRed += red;
                accumGreen += green;
                accumBlue += blue;
            }

            dstRow[dstX] = EncodeArgb(
                accumAlpha * 0.25f,
                accumRed * 0.25f,
                accumGreen * 0.25f,
                accumBlue * 0.25f);
        }
    }
}

void WritePackedPixel(unsigned char* dst, unsigned int bytesPerPixel, unsigned int value)
{
    switch (bytesPerPixel) {
    case 4:
        *reinterpret_cast<unsigned int*>(dst) = value;
        break;
    case 3:
        dst[0] = static_cast<unsigned char>(value & 0xFFu);
        dst[1] = static_cast<unsigned char>((value >> 8) & 0xFFu);
        dst[2] = static_cast<unsigned char>((value >> 16) & 0xFFu);
        break;
    case 2:
        *reinterpret_cast<unsigned short*>(dst) = static_cast<unsigned short>(value & 0xFFFFu);
        break;
    case 1:
        *dst = static_cast<unsigned char>(value & 0xFFu);
        break;
    default:
        break;
    }
}

} // namespace

float CTexture::m_uOffset = 0.0f;
float CTexture::m_vOffset = 0.0f;

CSurface::CSurface(unsigned int w, unsigned int h)
    : m_pddsSurface(nullptr), m_w(w), m_h(h) {}

CSurface::CSurface(unsigned int w, unsigned int h, IDirectDrawSurface7* pSurface)
    : m_pddsSurface(pSurface), m_w(w), m_h(h) {}

CSurface::~CSurface() {
#if RO_PLATFORM_WINDOWS
    if (m_pddsSurface) {
        m_pddsSurface->Release();
        m_pddsSurface = nullptr;
    }
#else
    m_pddsSurface = nullptr;
#endif
}

bool CSurface::Create(unsigned int w, unsigned int h) {
    m_w = w;
    m_h = h;
    return true;
}

void CSurface::ClearSurface(RECT* r, unsigned int col) {(void)r; (void)col;}
void CSurface::DrawSurface(int x, int y, int w, int h, unsigned int flags) {(void)x; (void)y; (void)w; (void)h; (void)flags;}

void CSurface::DrawSurfaceStretch(int x, int y, int w, int h) {
#if !RO_PLATFORM_WINDOWS
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    return;
#else
    DbgLog("[DrawSurfaceStretch] pixels=%p hWnd=%p x=%d y=%d w=%d h=%d\n",
           (const void*)GetSoftwarePixels(), (void*)GetRenderDevice().GetWindowHandle(), x, y, w, h);
    if (!HasSoftwarePixels() || !GetRenderDevice().GetWindowHandle()) {
        DbgLog("[DrawSurfaceStretch] SKIP: null backing data\n");
        return;
    }

    HDC target = GetDC(GetRenderDevice().GetWindowHandle());
    if (!target) {
        return;
    }

    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = static_cast<LONG>(m_w);
    bmi.bmiHeader.biHeight = -static_cast<LONG>(m_h);
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    SetStretchBltMode(target, HALFTONE);
    const int blitOk = StretchDIBits(target,
        x,
        y,
        w,
        h,
        0,
        0,
        static_cast<int>(m_w),
        static_cast<int>(m_h),
        GetSoftwarePixels(),
        &bmi,
        DIB_RGB_COLORS,
        SRCCOPY);
    DbgLog("[DrawSurfaceStretch] StretchBlt(%d,%d,%d,%d from %dx%d) -> %d (err=%lu)\n",
           x, y, w, h, static_cast<int>(m_w), static_cast<int>(m_h), blitOk, blitOk == GDI_ERROR ? GetLastError() : 0UL);
    ReleaseDC(GetRenderDevice().GetWindowHandle(), target);
#endif
}

void CSurface::Update(int x, int y, int w, int h, unsigned int* data, bool b, int p) {
    (void)x; (void)y; (void)w; (void)h; (void)data; (void)b; (void)p;
}

CSurfaceWallpaper::CSurfaceWallpaper(unsigned int w, unsigned int h)
    : CSurface(w, h) {}

CSurfaceWallpaper::~CSurfaceWallpaper() {}

void CSurfaceWallpaper::Update(int x, int y, int w, int h, unsigned int* data, bool b, int p) {
    (void)x; (void)y; (void)b;

    if (!data || w <= 0 || h <= 0) {
        return;
    }

    m_softwarePixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h));
    const int sourcePitch = p > 0 ? p : w * static_cast<int>(sizeof(unsigned int));
    for (int row = 0; row < h; ++row) {
        const unsigned char* srcRow = reinterpret_cast<const unsigned char*>(data) + static_cast<size_t>(row) * static_cast<size_t>(sourcePitch);
        unsigned int* dstRow = m_softwarePixels.data() + static_cast<size_t>(row) * static_cast<size_t>(w);
        std::memcpy(dstRow, srcRow, static_cast<size_t>(w) * sizeof(unsigned int));
    }
    m_w = static_cast<unsigned int>(w);
    m_h = static_cast<unsigned int>(h);
}

CTexture::CTexture(unsigned int w, unsigned int h, PixelFormat format)
    : CSurface(w, h), m_pf(format), m_lockCnt(0), m_timeStamp(0), m_updateWidth(0), m_updateHeight(0), m_surfaceUpdateWidth(0), m_surfaceUpdateHeight(0),
            m_upscaleFactor(1), m_backendTextureObject(nullptr), m_backendTextureView(nullptr), m_backendTextureUpload(nullptr) {
    m_texName[0] = '\0';
}

CTexture::CTexture(unsigned int w, unsigned int h, PixelFormat format, IDirectDrawSurface7* pSurface)
    : CSurface(w, h, pSurface), m_pf(format), m_lockCnt(0), m_timeStamp(0), m_updateWidth(0), m_updateHeight(0), m_surfaceUpdateWidth(0), m_surfaceUpdateHeight(0),
            m_upscaleFactor(1), m_backendTextureObject(nullptr), m_backendTextureView(nullptr), m_backendTextureUpload(nullptr) {
    m_texName[0] = '\0';
}

CTexture::~CTexture() {
    GetRenderDevice().ReleaseTextureResource(this);
}

bool CTexture::Create(unsigned int w, unsigned int h, PixelFormat format, bool allowUpscale, unsigned int minUpscaleFactor) {
    GetRenderDevice().ReleaseTextureResource(this);

    m_upscaleFactor = GetTextureUpscaleFactor(allowUpscale, minUpscaleFactor);

    const unsigned int scaledWidth = (std::max)(1u, w * m_upscaleFactor);
    const unsigned int scaledHeight = (std::max)(1u, h * m_upscaleFactor);

    unsigned int textureW = 0;
    unsigned int textureH = 0;
    if (!GetRenderDevice().CreateTextureResource(this, scaledWidth, scaledHeight, static_cast<int>(format), &textureW, &textureH)) {
        if constexpr (kLogTexture) {
            DbgLog("[Texture] CreateTextureResource failed name='%s' requested=%ux%u\n", m_texName, w, h);
        }
        return false;
    }

    m_w = textureW;
    m_h = textureH;
    m_pf = format;
    SetUVAdjust(w, h);
    m_surfaceUpdateWidth = (std::min)(textureW, scaledWidth);
    m_surfaceUpdateHeight = (std::min)(textureH, scaledHeight);
    return true;
}
bool CTexture::CreateBump(unsigned int w, unsigned int h, bool allowUpscale, unsigned int minUpscaleFactor) { return Create(w, h, PF_BUMP, allowUpscale, minUpscaleFactor); }
bool CTexture::CreateBump(unsigned int w, unsigned int h, IDirectDrawSurface7* pSurface, bool allowUpscale, unsigned int minUpscaleFactor) { (void)pSurface; return Create(w, h, PF_BUMP, allowUpscale, minUpscaleFactor); }
void CTexture::SetUVAdjust(unsigned int width, unsigned int height)
{
    unsigned int adjustedWidth = width;
    while (adjustedWidth > m_w) {
        adjustedWidth >>= 1;
    }

    unsigned int adjustedHeight = height;
    while (adjustedHeight > m_h) {
        adjustedHeight >>= 1;
    }

    m_updateWidth = adjustedWidth;
    m_updateHeight = adjustedHeight;
    m_surfaceUpdateWidth = adjustedWidth;
    m_surfaceUpdateHeight = adjustedHeight;
}
void CTexture::UpdateMipmap(RECT* r) {(void)r;}

bool CTexture::CopyTexture(CTexture* tex, int, int, int, int, int, int, int, int) { return tex != nullptr; }
int CTexture::Lock() { return 0; }
int CTexture::Unlock() { return 0; }

void CTexture::ClearSurface(RECT* r, unsigned int col) { CSurface::ClearSurface(r, col); }
void CTexture::DrawSurface(int x, int y, int w, int h, unsigned int flags) { CSurface::DrawSurface(x, y, w, h, flags); }
void CTexture::DrawSurfaceStretch(int x, int y, int w, int h) { CSurface::DrawSurfaceStretch(x, y, w, h); }
void CTexture::Update(int x, int y, int w, int h, unsigned int* data, bool b, int p) {
    const int scale = static_cast<int>((std::max)(1u, m_upscaleFactor));
    const int scaledX = x * scale;
    const int scaledY = y * scale;
    const int scaledW = w * scale;
    const int scaledH = h * scale;

    const unsigned int* uploadData = data;
    int uploadPitch = p;
    std::vector<unsigned int> scaledPixels;
    if (scale > 1 && data && w > 0 && h > 0) {
        const int srcPitch = p > 0 ? p : w * static_cast<int>(sizeof(unsigned int));
        BuildSupersampledUpscale(data, w, h, srcPitch, scale, &scaledPixels);
        uploadData = scaledPixels.data();
        uploadPitch = scaledW * static_cast<int>(sizeof(unsigned int));
    }

    if (!GetRenderDevice().UpdateTextureResource(this, scaledX, scaledY, scaledW, scaledH, uploadData, b, uploadPitch)) {
        return;
    }
    m_updateWidth = (std::max)(m_updateWidth, static_cast<unsigned int>(x + w));
    m_updateHeight = (std::max)(m_updateHeight, static_cast<unsigned int>(y + h));
    m_surfaceUpdateWidth = (std::max)(m_surfaceUpdateWidth, static_cast<unsigned int>(scaledX + scaledW));
    m_surfaceUpdateHeight = (std::max)(m_surfaceUpdateHeight, static_cast<unsigned int>(scaledY + scaledH));
}

void CTexture::UpdateSprite(int a, int b, int c, int d, SprImg& img, unsigned int* pal) { (void)a; (void)b; (void)c; (void)d; (void)img; (void)pal; }

#include "WebpEncoder.h"

#include <cstdio>
#include <cstdlib>

#include <webp/encode.h>

namespace capture {

bool EncodeWebp(const uint8_t* pixels,
                int width, int height, int stride,
                PixelFormat format,
                int quality,
                const std::filesystem::path& outPath)
{
    if (!pixels || width <= 0 || height <= 0 || stride < width * 4) {
        return false;
    }

    uint8_t* encoded = nullptr;
    size_t encodedSize = 0;
    const float q = static_cast<float>(quality);

    switch (format) {
    case PixelFormat::Rgba8:
        encodedSize = WebPEncodeRGBA(pixels, width, height, stride, q, &encoded);
        break;
    case PixelFormat::Bgra8:
        encodedSize = WebPEncodeBGRA(pixels, width, height, stride, q, &encoded);
        break;
    }

    if (encodedSize == 0 || !encoded) {
        if (encoded) WebPFree(encoded);
        return false;
    }

#if defined(_WIN32)
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, outPath.wstring().c_str(), L"wb") != 0 || !fp) {
        WebPFree(encoded);
        return false;
    }
#else
    FILE* fp = std::fopen(outPath.string().c_str(), "wb");
    if (!fp) {
        WebPFree(encoded);
        return false;
    }
#endif

    const size_t written = std::fwrite(encoded, 1, encodedSize, fp);
    std::fclose(fp);
    WebPFree(encoded);
    return written == encodedSize;
}

}  // namespace capture

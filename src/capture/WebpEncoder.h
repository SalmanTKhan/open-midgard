#pragma once

#include <cstdint>
#include <filesystem>

#include "FrameCapture.h"  // PixelFormat

namespace capture {

// Encodes an 8-bit RGBA or BGRA image to WebP and writes it to `outPath`.
// `pixels` is width*height*4 bytes; stride is bytes per row.
// quality is 0-100; higher is better. Returns true on success.
bool EncodeWebp(const uint8_t* pixels,
                int width, int height, int stride,
                PixelFormat format,
                int quality,
                const std::filesystem::path& outPath);

}  // namespace capture

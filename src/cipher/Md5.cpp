#include "Md5.h"

#include <array>
#include <cstring>
#include <vector>

namespace ro::cipher {
namespace {

constexpr std::array<u32, 64> kShift = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 5, 9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
};

constexpr std::array<u32, 64> kTable = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
};

inline u32 RotateLeft(u32 value, u32 bits)
{
    return (value << bits) | (value >> (32 - bits));
}

} // namespace

std::array<u8, 16> ComputeMd5(const void* data, size_t size)
{
    const auto* input = static_cast<const u8*>(data);
    const u64 bitLength = static_cast<u64>(size) * 8ULL;

    size_t paddedLength = size + 1;
    while ((paddedLength % 64) != 56) {
        ++paddedLength;
    }

    std::vector<u8> buffer(paddedLength + 8, 0);
    if (size > 0 && input) {
        std::memcpy(buffer.data(), input, size);
    }
    buffer[size] = 0x80;

    for (size_t index = 0; index < 8; ++index) {
        buffer[paddedLength + index] = static_cast<u8>((bitLength >> (index * 8)) & 0xFFu);
    }

    u32 a0 = 0x67452301u;
    u32 b0 = 0xefcdab89u;
    u32 c0 = 0x98badcfeu;
    u32 d0 = 0x10325476u;

    for (size_t offset = 0; offset < buffer.size(); offset += 64) {
        u32 block[16] = {};
        for (size_t wordIndex = 0; wordIndex < 16; ++wordIndex) {
            const size_t base = offset + wordIndex * 4;
            block[wordIndex] = static_cast<u32>(buffer[base]) |
                (static_cast<u32>(buffer[base + 1]) << 8) |
                (static_cast<u32>(buffer[base + 2]) << 16) |
                (static_cast<u32>(buffer[base + 3]) << 24);
        }

        u32 a = a0;
        u32 b = b0;
        u32 c = c0;
        u32 d = d0;

        for (u32 round = 0; round < 64; ++round) {
            u32 f = 0;
            u32 g = 0;
            if (round < 16) {
                f = (b & c) | (~b & d);
                g = round;
            } else if (round < 32) {
                f = (d & b) | (~d & c);
                g = (5 * round + 1) % 16;
            } else if (round < 48) {
                f = b ^ c ^ d;
                g = (3 * round + 5) % 16;
            } else {
                f = c ^ (b | ~d);
                g = (7 * round) % 16;
            }

            const u32 next = d;
            d = c;
            c = b;
            b = b + RotateLeft(a + f + kTable[round] + block[g], kShift[round]);
            a = next;
        }

        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

    std::array<u8, 16> digest{};
    const u32 state[4] = { a0, b0, c0, d0 };
    for (size_t wordIndex = 0; wordIndex < 4; ++wordIndex) {
        const u32 word = state[wordIndex];
        digest[wordIndex * 4 + 0] = static_cast<u8>(word & 0xFFu);
        digest[wordIndex * 4 + 1] = static_cast<u8>((word >> 8) & 0xFFu);
        digest[wordIndex * 4 + 2] = static_cast<u8>((word >> 16) & 0xFFu);
        digest[wordIndex * 4 + 3] = static_cast<u8>((word >> 24) & 0xFFu);
    }

    return digest;
}

} // namespace ro::cipher
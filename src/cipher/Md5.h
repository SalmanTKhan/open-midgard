#pragma once

#include <array>
#include <cstddef>

#include "Types.h"

namespace ro::cipher {

std::array<u8, 16> ComputeMd5(const void* data, size_t size);

} // namespace ro::cipher
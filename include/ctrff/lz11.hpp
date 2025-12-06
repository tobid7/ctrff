#pragma once

#include <ctrff/types.hpp>

namespace ctrff {
namespace LZ11 {
CTRFF_API std::vector<ctrff::u8> Compress(const std::vector<ctrff::u8>& in);
}
}  // namespace ctrff
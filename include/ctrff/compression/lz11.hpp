#pragma once

#include <ctrff/core/streams.hpp>
#include <ctrff/core/types.hpp>

namespace ctrff {
namespace LZ11 {
CTRFF_API std::vector<ctrff::u8> Compress(const std::vector<ctrff::u8>& in);
CTRFF_API std::vector<ctrff::u8> Decompress(const std::vector<ctrff::u8>& in);
CTRFF_API std::vector<ctrff::u8> DecompressStream(Stream& stream,
                                                  size_t& bytes_read);
}  // namespace LZ11
}  // namespace ctrff
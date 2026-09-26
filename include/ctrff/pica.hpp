#pragma once

#include <ctrff/types.hpp>

/**
 * 3ds GPU Stuff
 */

namespace ctrff {
namespace Pica {
enum Color : u32 {
  RGBA8888,
  RGB888,
  RGBA5551,
  RGB565,
  RGBA4444,
  LA8,
  HILO8,
  L8,
  A8,
  LA4,
  L4,
  A4,
  ETC1,
  ETC1A4,
};
CTRFF_API void EncodeImage(std::vector<ctrff::u8>& ret,
                           const std::vector<ctrff::u8>& rgba, int w, int h,
                           Color dst);
CTRFF_API void DecodeImage(std::vector<ctrff::u8>& ret,
                           const std::vector<ctrff::u8>& pixels, int w, int h,
                           Color src);
}  // namespace Pica
}  // namespace ctrff
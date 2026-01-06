#pragma once

#include <ctrff/types.hpp>

/**
 * 3ds GPU Stuff
 */

namespace ctrff {
namespace Pica {
enum Color : u32 {
  L8,  // tested
  A8,  // tested
  LA4,
  LA8,
  HILO8,
  RGB565,  // tested
  RGB888,  // tested
  RGBA5551,
  RGBA4444,
  RGBA8888,  // tested
  ETC1,
  ETC1A4,
  L4,  // tested
  A4,  // tested
};
CTRFF_API void EncodeImage(std::vector<ctrff::u8>& ret,
                           std::vector<ctrff::u8> rgba, int w, int h,
                           Color dst);
CTRFF_API void DecodeImage(std::vector<ctrff::u8>& ret,
                           std::vector<ctrff::u8> pixels, int w, int h,
                           Color src);
}  // namespace Pica
}  // namespace ctrff
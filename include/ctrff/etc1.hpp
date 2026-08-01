#pragma once

#include <ctrff/types.hpp>

namespace ctrff {
// Based on:
// https://github.com/gdkchan/SPICA/blob/master/SPICA/PICA/Converters/
// and
// https://github.com/Gericom/EveryFileExplorer/blob/master/LibEveryFileExplorer/GFX/ETC1.cs
namespace Etc1 {
struct EtcColor {
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
};
EtcColor Pixel(u32 r, u32 g, u32 b, int x, int y, u32 block, u32 table);
std::vector<u8> Tile(u64 block);
u64 Tile(const std::vector<u8>& data);
void Decode(std::vector<u8>& res, const std::vector<u8>& buf, int w, int h,
            bool a4);
void Encode(std::vector<u8>& res, const std::vector<u8>& rgba, int w, int h,
            bool a4);
}  // namespace Etc1
}  // namespace ctrff
#pragma once

#include <ctrff/types.hpp>

namespace ctrff {
CTRFF_API void String2U16(ctrff::u16 *res, const std::string &src, size_t max);
CTRFF_API std::string U16toU8(ctrff::u16 *in, size_t max);
CTRFF_API void RGB565toRGBA(std::vector<ctrff::u8> &img, ctrff::u16 *icon,
                            const int &w, const int &h);
// Image can only be rgba8888
CTRFF_API void RGBA2RGB565(ctrff::u16 *out, const std::vector<ctrff::u8> &img,
                           const int &w, const int &h);
CTRFF_API std::vector<ctrff::u8> DownscaleImage(
    const std::vector<ctrff::u8> &img, int w, int h, int scale);
}  // namespace ctrff
#include <ctrff/helper.hpp>
#include <cwchar>
#include <iostream>

void MakePixelRGBA(PD::u8 &r, PD::u8 &g, PD::u8 &b, PD::u8 &a, PD::u16 px) {
  b = (px & 0x1f) << 3;
  g = ((px >> 0x5) & 0x3f) << 2;
  r = ((px >> 0xb) & 0x1f) << 3;
  a = 0xff;
}

CTRFF_API PD::u16 MakePixel565(const PD::u8 &r, const PD::u8 &g,
                               const PD::u8 &b) {
  PD::u16 res = 0;
  res |= (r & ~0x7) << 8;
  res |= (g & ~0x3) << 3;
  res |= (b) >> 3;
  return res;
}

CTRFF_API PD::u32 TileIndex(const int &x, const int &y, const int &w) {
  return (((y >> 3) * (w >> 3) + (x >> 3)) << 6) +
         ((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
          ((x & 4) << 2) | ((y & 4) << 3));
}

// TODO: Fix colors
CTRFF_API void ctrff::RGB565toRGBA(std::vector<PD::u8> &img, PD::u16 *icon,
                                   const int &w, const int &h) {
  if (img.size() != (48 * 48 * 4)) {
    img.resize(48 * 48 * 4);
    img.clear();
  }
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      auto idx = TileIndex(x, y, w);
      PD::u32 pos = (y * w + x) * 4;
      MakePixelRGBA(img[pos + 0], img[pos + 1], img[pos + 2], img[pos + 3],
                    icon[idx]);
    }
  }
}

CTRFF_API void ctrff::RGBA2RGB565(PD::u16 *out, const std::vector<PD::u8> &img,
                                  const int &w, const int &h) {
  if (img.size() != size_t(w * h * 4)) return;
  std::vector<PD::u8> px8 = img;
  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      auto idx = TileIndex(x, y, w);
      PD::u32 pos = (y * w + x) * 4;
      out[idx] = MakePixel565(img[pos], img[pos + 1], img[pos + 2]);
    }
  }
}

CTRFF_API std::vector<PD::u8> ctrff::DownscaleImage(
    const std::vector<PD::u8> &img, int w, int h, int scale) {
  std::vector<PD::u8> res(((w / scale) * (h / scale)) * 4);
  int samples = scale * scale;
  for (int y = 0; y < h; y += scale) {
    for (int x = 0; x < w; x += scale) {
      PD::u32 r = 0;
      PD::u32 g = 0;
      PD::u32 b = 0;
      PD::u32 a = 0;
      for (int oy = 0; oy < scale; oy++) {
        for (int ox = 0; ox < scale; ox++) {
          int pos = ((y + oy) * w + (x + ox)) * 4;
          r += img[pos++];
          g += img[pos++];
          b += img[pos++];
          a += img[pos++];
        }
      }
      int pos = ((y / scale) * (w / scale) + (x / scale)) * 4;
      res[pos++] = (PD::u8)(r / samples);
      res[pos++] = (PD::u8)(g / samples);
      res[pos++] = (PD::u8)(b / samples);
      res[pos++] = (PD::u8)(a / samples);
    }
  }
  return res;
}

CTRFF_API void ctrff::String2U16(PD::u16 *res, const std::string &src,
                                 size_t max) {
  /// GOT FORCED TO REPLACE std::wstring_convert by some
  /// manual work as it got removed in cxx20
  /// TODO ///
  /// ADD SOME ERROR API IN HERE
  if (max == 0) return;
  size_t len = 0;
  size_t i = 0;
  while (i < src.size() && len < max) {
    PD::u8 c = src[i];

    if (c < 0x80) {
      // 1byte
      res[len++] = c;
      i++;
    } else if ((c >> 5) == 0x6) {
      // 2byte
      if (i + 1 >= src.size())
        throw std::invalid_argument("Invalid UTF-8 sequence");
      res[len++] = ((c & 0x1F) << 6) | (src[i + 1] & 0x3F);
      i += 2;
    } else if ((c >> 4) == 0xE) {
      // 3byte
      if (i + 2 >= src.size())
        throw std::invalid_argument("Invalid UTF-8 sequence");
      res[len++] =
          ((c & 0x0F) << 12) | ((src[i + 1] & 0x3F) << 6) | (src[i + 2] & 0x3F);
      i += 3;
    } else if ((c >> 3) == 0x1E) {
      // 4byte
      if (i + 3 >= src.size())
        throw std::invalid_argument("Invalid UTF-8 sequence");
      PD::u32 codepoint = ((c & 0x07) << 18) | ((src[i + 1] & 0x3F) << 12) |
                          ((src[i + 2] & 0x3F) << 6) | (src[i + 3] & 0x3F);
      codepoint -= 0x10000;
      res[len++] = 0xD800 | ((codepoint >> 10) & 0x3FF);
      res[len++] = 0xDC00 | (codepoint & 0x3FF);
      i += 4;
    } else {
      return;
    }
  }
}

CTRFF_API std::string ctrff::U16toU8(PD::u16 *in, size_t max) {
  /// GOT FORCED TO REPLACE std::wstring_convert by some
  /// manual work as it got removed in cxx20
  if (!in || max == 0) {
    return "";
  }

  std::string result;
  result.reserve(max * 3);

  for (size_t i = 0; i < max; i++) {
    uint16_t c = in[i];

    if (c < 0x80) {
      result.push_back(static_cast<char>(c));
    } else if (c < 0x800) {
      result.push_back(static_cast<char>(0xC0 | (c >> 6)));
      result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else if (c < 0x10000) {
      result.push_back(static_cast<char>(0xE0 | (c >> 12)));
      result.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F)));
      result.push_back(static_cast<char>(0x80 | (c & 0x3F)));
    } else {
      continue;
    }
  }

  return result;
}

#include <ctrff/helper.hpp>
#include <ctrff/pica.hpp>

namespace ctrff {
namespace Pica {
CTRFF_API void EncodeImage(std::vector<ctrff::u8>& ret,
                           std::vector<ctrff::u8> rgba, int w, int h,
                           Color dst_color) {
  // Only used in rgb/rgba
  int bpp = dst_color == RGBA8888 ? 4 : 3;
  switch (dst_color) {
    case RGB565:
      ret.resize(w * h * 2);
      ctrff::RGBA2RGB565(reinterpret_cast<ctrff::u16*>(ret.data()), rgba, w, h);
      break;
    case RGB888:
    case RGBA8888:
      ret.resize(w * h * bpp);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w) * bpp;
          for (int i = 0; i < bpp; i++) {
            ret[dst + bpp - 1 - i] = rgba[src + i];
          }
        }
      }
      break;
    case A8:
      ret.resize(w * h);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w);
          ret[dst] = rgba[src + 3];  // extract alpha only
        }
      }
      break;
    case A4:
      ret.resize((w * h) >> 1);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w);
          ctrff::u8 tmp = rgba[src + 3] >> 4;
          int a4pos = dst >> 1;
          if ((dst & 1) == 0) {
            ret[a4pos] = (tmp << 4) | (ret[a4pos] & 0x0f);
          } else {
            ret[a4pos] = (ret[a4pos] & 0xf0) | tmp;
          }
        }
      }
      break;
    case L8:
      ret.resize(w * h);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w);
          // Basic luminance calculation (already used in renderd7)
          ret[dst] =
              (rgba[src + 0] * 77 + rgba[src + 1] * 150 + rgba[src + 2] * 29) >>
              8;
        }
      }
      break;
    case L4:
      ret.resize((w * h) >> 1);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w);
          // Basically same as a4 just with the luminance calculation func
          ctrff::u8 tmp = ((rgba[src + 0] * 77 + rgba[src + 1] * 150 +
                            rgba[src + 2] * 29) >>
                           8) >>
                          4;
          int a4pos = dst >> 1;
          if ((dst & 1) == 0) {
            ret[a4pos] = (tmp << 4) | (ret[a4pos] & 0x0f);
          } else {
            ret[a4pos] = (ret[a4pos] & 0xf0) | tmp;
          }
        }
      }
      break;
    case RGBA4444:
      ret.resize(w * h * 2);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w) * 2;
          ret[dst + 0] = rgba[src + 3] >> 4;
          ret[dst + 0] |= (rgba[src + 2] >> 4) << 4;
          ret[dst + 1] = rgba[src + 1] >> 4;
          ret[dst + 1] |= (rgba[src + 0] >> 4) << 4;
        }
      }
      break;
    case RGBA5551:  // not working ???
      ret.resize(w * h * 2);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w) * 2;
          ctrff::u16 px = 0;
          px |= (rgba[src + 3] < 128 ? 0 : 1) << 15;  // A
          px |= ((rgba[src + 2] >> 3) & 0x1f) << 10;  // BBBBB
          px |= ((rgba[src + 1] >> 3) & 0x1f) << 5;   // GGGGG
          px |= ((rgba[src + 0] >> 3) & 0x1f);        // RRRRR
          ret[dst] = px & 0xff;
          ret[dst + 1] = (px << 8) & 0xff;
        }
      }
      break;
    case LA8:
      ret.resize(w * h * 2);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w) * 2;
          ret[dst] =
              (rgba[src + 0] * 77 + rgba[src + 1] * 150 + rgba[src + 2] * 29) >>
              8;
          ret[dst + 1] = rgba[src + 3];
        }
      }
      break;
    case LA4:
      ret.resize(w * h);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = (y * w + x) * 4;  // basic rgba indexing btw
          int dst = ctrff::TileIndex(x, y, w);
          ret[dst] = 0;
          ret[dst] |= (((rgba[src + 0] * 77 + rgba[src + 1] * 150 +
                         rgba[src + 2] * 29) >>
                        8) >>
                       4) &
                      0xf;
          ret[dst] |= (((rgba[src + 3] >> 4) & 0xf) << 4) | ret[dst];
        }
      }
      break;

    default:
      throw std::runtime_error("[ctrff] Pica: Unsupported Color format: " +
                               std::to_string((int)dst_color));
      break;
  }
}

CTRFF_API void DecodeImage(std::vector<ctrff::u8>& ret,
                           std::vector<ctrff::u8> pixels, int w, int h,
                           Color src_color) {
  switch (src_color) {
    case RGB565:
      ret.resize(w * h * 4);
      ctrff::RGB565toRGBA(
          ret, reinterpret_cast<const ctrff::u16*>(pixels.data()), w, h);
      break;
    case RGB888:
    case RGBA8888:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int bpp = src_color == RGBA8888 ? 4 : 3;
          int src = ctrff::TileIndex(x, y, w) * bpp;
          int dst = (y * w + x) * 4;  // basic rgba indexing btw
          for (int i = 0; i < bpp; i++) {
            ret[dst + i] = pixels[src + bpp - 1 - i];
          }
          if (src_color == RGB888) {
            ret[dst + 3] = 255;
          }
        }
      }
      break;
    case A8:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w);
          int dst = (y * w + x) * 4;
          ret[dst + 0] = 255;
          ret[dst + 1] = 255;
          ret[dst + 2] = 255;
          ret[dst + 3] = pixels[src];
        }
      }
      break;
    case A4:  // most hated by me (tobid7)
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w);
          int dst = (y * w + x) * 4;
          /**
           * Basically checking by %2 aka &1 if we have a high or low nibble
           * by this we either extract the high or low one and multiply
           * by 17 aka 0x11 to get the resulting A8 Value
           */
          ctrff::u8 a4 = src & 1 ? ((pixels[src >> 1] >> 4) & 0xf) * 0x11
                                 : (pixels[src >> 1] & 0xf) * 0x11;
          ret[dst + 0] = 255;
          ret[dst + 1] = 255;
          ret[dst + 2] = 255;
          ret[dst + 3] = a4;
        }
      }
      break;
    case L4:  // most hated by me (tobid7)
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w);
          int dst = (y * w + x) * 4;
          /**
           * Same as A4
           * Basically checking by %2 aka &1 if we have a high or low nibble
           * by this we either extract the high or low one and multiply
           * by 17 aka 0x11 to get the resulting A8 Value
           */
          ctrff::u8 a4 = src & 1 ? ((pixels[src >> 1] >> 4) & 0xf) * 0x11
                                 : (pixels[src >> 1] & 0xf) * 0x11;
          ret[dst + 0] = a4;
          ret[dst + 1] = a4;
          ret[dst + 2] = a4;
          ret[dst + 3] = 255;
        }
      }
      break;
    case L8:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w);
          int dst = (y * w + x) * 4;
          ret[dst + 0] = pixels[src];
          ret[dst + 1] = pixels[src];
          ret[dst + 2] = pixels[src];
          ret[dst + 3] = 255;
        }
      }
      break;
    case RGBA4444:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w) * 2;
          int dst = (y * w + x) * 4;  // basic rgba indexing btw
          ret[dst + 0] = ((pixels[src + 1] >> 4) & 0xf) << 4;
          ret[dst + 1] = (pixels[src + 1] & 0xf) << 4;
          ret[dst + 2] = ((pixels[src + 0] >> 4) & 0xf) << 4;
          ret[dst + 3] = (pixels[src + 0] & 0xf) << 4;
        }
      }
      break;
    case RGBA5551:  // not working
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w) * 2;
          int dst = (y * w + x) * 4;  // basic rgba indexing btw
          ctrff::u16 px = ret[src] | (ret[src + 1] << 8);

          ret[dst + 0] = (px & 0x1f) * 0x1f;
          ret[dst + 1] = ((px >> 5) & 0x1f) * 0x1f;
          ret[dst + 2] = ((px >> 10) & 0x1f) * 0x1f;
          ret[dst + 3] = 255;  // ((*px >> 15) & 0x1) ? 255 : 0;
        }
      }
      break;
    case LA8:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w) * 2;
          int dst = (y * w + x) * 4;
          ret[dst + 0] = pixels[src];
          ret[dst + 1] = pixels[src];
          ret[dst + 2] = pixels[src];
          ret[dst + 3] = pixels[src + 1];
        }
      }
      break;
    case LA4:
      ret.resize(w * h * 4);
      for (int x = 0; x < w; x++) {
        for (int y = 0; y < h; y++) {
          int src = ctrff::TileIndex(x, y, w);
          int dst = (y * w + x) * 4;
          uint8_t l = (pixels[src] & 0xf) * 0x11;
          uint8_t a = ((pixels[src] >> 4) & 0xf) * 0x11;
          ret[dst + 0] = l;
          ret[dst + 1] = l;
          ret[dst + 2] = l;
          ret[dst + 3] = a;
        }
      }
      break;

    default:
      throw std::runtime_error("[ctrff] Pica: Unsupported Color format: " +
                               std::to_string((int)src_color));
      break;
  }
}
}  // namespace Pica
}  // namespace ctrff
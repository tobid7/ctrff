#include <ctrff/etc1.hpp>
#include <ctrff/helper.hpp>

namespace ctrff {
namespace Etc1 {
u64 Swap64(u64 v) {
  u64 res = 0;
  res |= (v >> 56) & 0xFF;
  res |= ((v >> 48) & 0xFF) << 8;
  res |= ((v >> 40) & 0xFF) << 16;
  res |= ((v >> 32) & 0xFF) << 24;
  res |= ((v >> 24) & 0xFF) << 32;
  res |= ((v >> 16) & 0xFF) << 40;
  res |= ((v >> 8) & 0xFF) << 48;
  res |= (v & 0xFF) << 56;
  return res;
}

u64 Grab64(const u8* ref, size_t& pos, size_t max) {
  if (pos + 8 > max) {
    return 0;
  }
  u64 res = 0x0ULL;
  res |= static_cast<u64>(ref[pos++]);
  res |= static_cast<u64>(ref[pos++]) << 8;
  res |= static_cast<u64>(ref[pos++]) << 16;
  res |= static_cast<u64>(ref[pos++]) << 24;
  res |= static_cast<u64>(ref[pos++]) << 32;
  res |= static_cast<u64>(ref[pos++]) << 40;
  res |= static_cast<u64>(ref[pos++]) << 48;
  res |= static_cast<u64>(ref[pos++]) << 56;
  return res;
}

void Put64(u64 block, std::vector<u8>& ref) {
  ref.push_back(static_cast<u8>(block & 0xff));
  ref.push_back(static_cast<u8>((block >> 8) & 0xff));
  ref.push_back(static_cast<u8>((block >> 16) & 0xff));
  ref.push_back(static_cast<u8>((block >> 24) & 0xff));
  ref.push_back(static_cast<u8>((block >> 32) & 0xff));
  ref.push_back(static_cast<u8>((block >> 40) & 0xff));
  ref.push_back(static_cast<u8>((block >> 48) & 0xff));
  ref.push_back(static_cast<u8>((block >> 56) & 0xff));
}

int XT[4] = {0, 4, 0, 4};
int YT[4] = {0, 0, 4, 4};
void Decode(std::vector<u8>& res, const std::vector<u8>& buf, int w, int h,
            bool a4) {
  res.resize(w * h * 4);
  size_t bufpos = 0;
  for (int ty = 0; ty < h; ty += 8) {
    for (int tx = 0; tx < w; tx += 8) {
      for (int t = 0; t < 4; t++) {
        u64 alpha_block = 0xffffffffffffffff;
        if (a4) {
          alpha_block = Grab64(buf.data(), bufpos, buf.size());
        }
        u64 block = Swap64(Grab64(buf.data(), bufpos, buf.size()));
        auto tile = Tile(block);
        int tileoff = 0;
        for (int py = YT[t]; py < 4 + YT[t]; py++) {
          for (int px = XT[t]; px < 4 + XT[t]; px++) {
            int offs = ((h - 1 - (ty + py)) * w + tx + px) * 4;
            for (int i = 0; i < 3; i++) {
              res[offs + i] = tile[tileoff + i];
            }
            int alpha_shift = ((px & 3) * 4 + (py & 3)) << 2;
            u8 a = static_cast<u8>(alpha_block >> alpha_shift) & 0xf;
            res[offs + 3] = static_cast<u8>(a << 4) | a;
            tileoff += 4;
          }
        }
      }
    }
  }
  Flip(res, w, h);
}

static int ETC1LUT[8][4] = {{2, 8, -2, -8},       {5, 17, -5, -17},
                            {9, 29, -9, -29},     {13, 42, -13, -42},
                            {18, 60, -18, -60},   {24, 80, -24, -80},
                            {33, 106, -33, -106}, {47, 183, -47, -183}};

u8 ColorClamp(int v) {
  if (v > 0xff) return 0xff;
  if (v < 0) return 0;

  return static_cast<u8>(v);
}

static inline int Signed3(u32 v) { return (int)((int8_t)(v << 5) >> 5); }

/**
 * Calculate the error between two colors (quality check)
 */
int GetError(const EtcColor& clr, int r2, int g2, int b2) {
  int r = clr.r - ColorClamp(r2);
  int g = clr.g - ColorClamp(g2);
  int b = clr.b - ColorClamp(b2);
  return r * r + g * g + b * b;
}

EtcColor Pixel(u32 r, u32 g, u32 b, int x, int y, u32 block, u32 table) {
  int idx = x * 4 + y;
  u32 msb = block << 1;

  int pixel = (idx < 8) ? ETC1LUT[table][((block >> (idx + 24)) & 1) +
                                         ((msb >> (idx + 8)) & 2)]
                        : ETC1LUT[table][((block >> (idx + 8)) & 1) +
                                         ((msb >> (idx - 8)) & 2)];

  r = ColorClamp(static_cast<int>(r + pixel));
  g = ColorClamp(static_cast<int>(g + pixel));
  b = ColorClamp(static_cast<int>(b + pixel));

  return {(u8)r, (u8)g, (u8)b};
}

std::vector<u8> Tile(u64 block) {
  u32 low = static_cast<u32>(block >> 32);
  u32 high = static_cast<u32>(block);
  bool flip = (high & 0x01000000) != 0;
  bool diff = (high & 0x02000000) != 0;
  u32 r1 = 0, g1 = 0, b1 = 0;
  u32 r2 = 0, g2 = 0, b2 = 0;
  if (diff) {
    b1 = (high & 0x0000f8);
    g1 = (high & 0x00f800) >> 8;
    r1 = (high & 0xf80000) >> 16;

    b2 = (u32)((int)(b1 >> 3) + Signed3(high & 0x7));
    g2 = (u32)((int)(g1 >> 3) + Signed3((high >> 8) & 0x7));
    r2 = (u32)((int)(r1 >> 3) + Signed3((high >> 16) & 0x7));

    b1 |= b1 >> 5;
    g1 |= g1 >> 5;
    r1 |= r1 >> 5;

    b2 = (b2 << 3) | (b2 >> 2);
    g2 = (g2 << 3) | (g2 >> 2);
    r2 = (r2 << 3) | (r2 >> 2);
  } else {
    b1 = (high & 0x0000f0);
    g1 = (high & 0x00f000) >> 8;
    r1 = (high & 0xf00000) >> 16;

    b2 = (high & 0x00000f) << 4;
    g2 = (high & 0x000f00) >> 4;
    r2 = (high & 0x0f0000) >> 12;

    b1 |= b1 >> 4;
    g1 |= g1 >> 4;
    r1 |= r1 >> 4;

    b2 |= b2 >> 4;
    g2 |= g2 >> 4;
    r2 |= r2 >> 4;
  }
  u32 table1 = (high >> 29) & 0x7;
  u32 table2 = (high >> 26) & 0x7;

  std::vector<u8> res(4 * 4 * 4, 0x0);
  if (!flip) {
    for (int y = 0; y < 4; y++) {
      for (int x = 0; x < 2; x++) {
        EtcColor col1 = Pixel(r1, g1, b1, x + 0, y, low, table1);
        EtcColor col2 = Pixel(r2, g2, b2, x + 2, y, low, table2);

        int off1 = (y * 4 + x) * 4;

        res[off1 + 0] = col1.b;
        res[off1 + 1] = col1.g;
        res[off1 + 2] = col1.r;

        int off2 = (y * 4 + x + 2) * 4;

        res[off2 + 0] = col2.b;
        res[off2 + 1] = col2.g;
        res[off2 + 2] = col2.r;
      }
    }
  } else {
    for (int y = 0; y < 2; y++) {
      for (int x = 0; x < 4; x++) {
        EtcColor col1 = Pixel(r1, g1, b1, x, y + 0, low, table1);
        EtcColor col2 = Pixel(r2, g2, b2, x, y + 2, low, table2);

        int off1 = (y * 4 + x) * 4;

        res[off1 + 0] = col1.b;
        res[off1 + 1] = col1.g;
        res[off1 + 2] = col1.r;

        int off2 = ((y + 2) * 4 + x) * 4;

        res[off2 + 0] = col2.b;
        res[off2 + 1] = col2.g;
        res[off2 + 2] = col2.r;
      }
    }
  }

  return res;
}

u64 Tile(const std::vector<u8>& data) {
  u64 block;
  int min_err = std::numeric_limits<int>::max();
  for (int i = 0; i < 2; i++) {  // flip check
    int sum_r[2] = {0, 0};
    int sum_g[2] = {0, 0};
    int sum_b[2] = {0, 0};
    int count[2] = {0, 0};
    for (int j = 0; j < 16; j++) {
      int x = j % 4;
      int y = j / 4;
      int block_idx = 0;
      (i == 0) ? block_idx = ((x < 2) ? 0 : 1) : block_idx = ((y < 2) ? 0 : 1);
      sum_b[block_idx] += data[j * 4 + 0];
      sum_g[block_idx] += data[j * 4 + 1];
      sum_r[block_idx] += data[j * 4 + 2];
      count[block_idx]++;
    }
    // Average color calculation
    for (int j = 0; j < 2; j++) {
      if (count[j] > 0) {
        sum_r[j] /= count[j];
        sum_g[j] /= count[j];
        sum_b[j] /= count[j];
      }
    }

    int r1 = sum_r[0] >> 3;
    int g1 = sum_g[0] >> 3;
    int b1 = sum_b[0] >> 3;

    int r2 = sum_r[1] >> 3;
    int g2 = sum_g[1] >> 3;
    int b2 = sum_b[1] >> 3;

    int dr = r2 - r1;
    int dg = g2 - g1;
    int db = b2 - b1;
    bool diff =
        (dr >= -4 && dr <= 3) && (dg >= -4 && dg <= 3) && (db >= -4 && db <= 3);
    int br1 = 0, bg1 = 0, bb1 = 0, br2 = 0, bg2 = 0, bb2 = 0;
    u64 high = 0x0;
    if (diff) {  // Diff mode
      high |= (1 << 25);
      high |= (static_cast<u64>(r1 & 0x1f) << 27 |
               static_cast<u64>(g1 & 0x1f) << 19 |
               static_cast<u64>(b1 & 0x1f) << 11);
      high |=
          (static_cast<u64>(dr & 0x7) << 24 | static_cast<u64>(dg & 0x7) << 16 |
           static_cast<u64>(db & 0x7) << 8);

      // 5 bit to 8 bit btw
      br1 = (r1 << 3) | (r1 >> 2);
      bg1 = (g1 << 3) | (g1 >> 2);
      bb1 = (b1 << 3) | (b1 >> 2);
      br2 = ((r1 + dr) << 3) | ((r1 + dr) >> 2);
      bg2 = ((g1 + dg) << 3) | ((g1 + dg) >> 2);
      bb2 = ((b1 + db) << 3) | ((b1 + db) >> 2);
    } else {  // Individual mode
      int ir1 = sum_r[0] >> 4;
      int ig1 = sum_g[0] >> 4;
      int ib1 = sum_b[0] >> 4;
      int ir2 = sum_r[1] >> 4;
      int ig2 = sum_g[1] >> 4;
      int ib2 = sum_b[1] >> 4;

      high |= ((u64)ir1 << 28) | ((u64)ig1 << 20) | ((u64)ib1 << 12);
      high |= ((u64)ir2 << 24) | ((u64)ig2 << 16) | ((u64)ib2 << 8);

      // 4 Bits to 8 btw
      br1 = (ir1 << 4) | ir1;
      bg1 = (ig1 << 4) | ig1;
      bb1 = (ib1 << 4) | ib1;
      br2 = (ir2 << 4) | ir2;
      bg2 = (ig2 << 4) | ig2;
      bb2 = (ib2 << 4) | ib2;
    }
    if (i == 1) {
      high |= 1ULL;
    }
    int table[2] = {0, 0};
    u32 sectors = 0;
    int err = 0;
    for (int s = 0; s < 2; s++) {
      int suberr = std::numeric_limits<int>::max();
      int bT = 0;
      u32 bestSel = 0;
      int br = (s == 0) ? br1 : br2;
      int bg = (s == 0) ? bg1 : bg2;
      int bb = (s == 0) ? bb1 : bb2;

      for (int t = 0; t < 8; t++) {
        int table_err = 0;
        u32 currentSel = 0;
        for (int j = 0; j < 16; j++) {
          int x = j % 4;
          int y = j / 4;
          int sub = (i == 0) ? ((x < 2) ? 0 : 1) : ((y < 2) ? 0 : 1);
          if (sub != s) continue;
          int bme = std::numeric_limits<int>::max();
          int bmi = 0;
          for (int m = 0; m < 4; m++) {  // testing all 4 modifiers
            int v = ETC1LUT[t][m];
            int e =
                GetError({data[j * 4 + 2], data[j * 4 + 1], data[j * 4 + 0]},
                         br + v, bg + v, bb + v);
            if (e < bme) {
              bme = e;
              bmi = m;
            }
          }
          table_err += bme;

          int lsb = (bmi & 1);
          int msb = ((bmi >> 1) & 1);

          // Pixel index like in tile decoder
          int pidx = x * 4 + y;
          if (pidx < 8) {
            if (lsb) currentSel |= (1U << (pidx + 24));
            if (msb) currentSel |= (1U << (pidx + 8));
          } else {
            if (lsb) currentSel |= (1U << (pidx + 8));
            if (msb) currentSel |= (1U << (pidx - 8));
          }
        }
        if (table_err < suberr) {
          suberr = table_err;
          bT = t;
          bestSel = currentSel;
        }
        err += suberr;
        table[s] = bT;
        sectors |= bestSel;
      }
      if (err < min_err) {
        min_err = err;
        // Lets build this block
        u64 fhigh = high;
        fhigh |= static_cast<u64>(table[0] << 29);
        fhigh |= static_cast<u64>(table[1] << 26);
        if (i == 1) {
          fhigh |= 0x1000000;
        }
        u32 high32 = 0;
        if (diff) {
          high32 |= (table[0] & 0x7) << 29;
          high32 |= (table[1] & 0x7) << 26;
          high32 |= 0x1 << 25;
          if (i == 1) {
            high32 |= 1 << 24;
          }
          high32 |= (r1 & 0x1f) << 19;
          high32 |= (g1 & 0x1f) << 11;
          high32 |= (b1 & 0x1f) << 3;
          high32 |= (dr & 0x7) << 16;
          high32 |= (dg & 0x7) << 8;
          high32 |= (db & 0x7) << 0;
        } else {  // Individual
          high32 |= (table[0] & 0x7) << 29;
          high32 |= (table[1] & 0x7) << 26;
          if (i == 1) {
            high32 |= 1 << 24;
          }
          high32 |= (br1 >> 4) << 20;
          high32 |= (bg1 >> 4) << 12;
          high32 |= (bb1 >> 4) << 4;

          high32 |= (br2 >> 4) << 16;
          high32 |= (bg2 >> 4) << 8;
          high32 |= (bb2 >> 4) << 0;
        }
        block = (static_cast<u64>(sectors) << 32) | high32;
      }
    }
  }
  return block;
}

void Encode(std::vector<u8>& res, const std::vector<u8>& rgba, int w, int h,
            bool a4) {
  std::vector<u8> rgbaflip = rgba;
  ctrff::Flip(rgbaflip, w, h);
  res.clear();  // Make sure we have no trouble here
  for (int ty = 0; ty < h; ty += 8) {
    for (int tx = 0; tx < w; tx += 8) {
      for (int t = 0; t < 4; t++) {
        std::vector<u8> pixels(4 * 4 * 4, 0x0);
        u64 alpha_block = 0;  // Only in A4
        int idx = 0;
        for (int py = YT[t]; py < 4 + YT[t]; py++) {
          for (int px = XT[t]; px < 4 + XT[t]; px++) {
            int srcX = tx + px;
            int srcY = ty + py;
            if (srcX < w && srcY < h) {
              int offs = ((h - 1 - srcY) * w + srcX) * 4;
              pixels[idx + 0] = rgbaflip[offs + 0];
              pixels[idx + 1] = rgbaflip[offs + 1];
              pixels[idx + 2] = rgbaflip[offs + 2];
              pixels[idx + 3] = rgbaflip[offs + 3];
            }
            if (a4) {
              int shift = ((px & 3) * 4 + (py & 3)) << 2;
              u64 a4v = pixels[idx + 3] >> 4;
              alpha_block |= (a4v << shift);
            }
            idx += 4;
          }
        }
        u64 block = Swap64(Tile(pixels));
        if (a4) {
          Put64(alpha_block, res);
        }
        Put64(block, res);
      }
    }
  }
}
}  // namespace Etc1
}  // namespace ctrff
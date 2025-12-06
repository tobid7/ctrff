#include <cstring>
#include <ctrff/lz11.hpp>

/// REWRITTEN CODE FROM BANNERTOOL !!!!!!

namespace ctrff {
namespace LZ11 {
ctrff::u32 GetOccurenceLength(const ctrff::u8* new_ptr, size_t new_len,
                              const ctrff::u8* old_ptr, size_t old_len,
                              ctrff::u32& disp) {
  disp = 0;
  if (new_len == 0) {
    return 0;
  }
  ctrff::u32 res = 0;
  if (old_len > 0) {
    for (size_t i = 0; i < old_len - 1; i++) {
      auto ref = old_ptr + i;
      size_t len = 0;
      for (size_t j = 0; j < new_len; j++) {
        if (*(ref + j) != (*new_ptr + j)) {
          break;
        }
        len++;
      }
      if (len > res) {
        res = len;
        disp = old_len - i;
        if (res == new_len) {
          break;
        }
      }
    }
  }
  return res;
}

CTRFF_API std::vector<ctrff::u8> Compress(const std::vector<ctrff::u8>& in) {
  if (in.size() > 0xFFFFFF) {
    std::cout << "ERROR: LZ11 input is too large!" << std::endl;
    return std::vector<ctrff::u8>();
  }
  std::stringstream s;
  // SETUP HEADER //
  s << static_cast<ctrff::u8>(0x11);
  s << static_cast<ctrff::u8>(in.size() & 0xFF);
  s << static_cast<ctrff::u8>((in.size() >> 8) & 0xFF);
  s << static_cast<ctrff::u8>((in.size() >> 16) & 0xFF);

  size_t res_len = 4;       // 4-byte header
  ctrff::u8 out_buf[0x21];  // 33 bytes
  out_buf[0] = 0;
  size_t obl = 1;  // out_buf_len
  size_t buf_blocks = 0;
  size_t rb = 0;

  while (rb < in.size()) {
    if (buf_blocks == 8 || obl >= sizeof(out_buf) - 3) {
      s.write(reinterpret_cast<const char*>(out_buf), obl);
      res_len += obl;
      out_buf[0] = 0;
      obl = 1;
      buf_blocks = 0;
    }

    ctrff::u32 disp = 0;
    size_t old_len = std::min(rb, static_cast<size_t>(0x1000));
    size_t len = LZ11::GetOccurenceLength(
        in.data() + rb, std::min(in.size() - rb, static_cast<size_t>(0x10110)),
        in.data() + rb - old_len, old_len, disp);

    if (len < 3) {
      out_buf[obl++] = in[rb++];
    } else {
      rb += len;
      out_buf[0] |= static_cast<ctrff::u8>(1 << (7 - buf_blocks));
      if (len > 0x110) {
        out_buf[obl++] =
            0x10 | static_cast<ctrff::u8>(((len - 0x111) >> 12) & 0x0F);
        out_buf[obl++] = static_cast<ctrff::u8>(((len - 0x111) >> 4) & 0xFF);
        out_buf[obl] = static_cast<ctrff::u8>(((len - 0x111) << 4) & 0xF0);
      } else if (len > 0x10) {
        out_buf[obl++] =
            0x00 | static_cast<ctrff::u8>(((len - 0x11) >> 4) & 0x0F);
        out_buf[obl] = static_cast<ctrff::u8>(((len - 0x11) << 4) & 0xF0);
      } else {
        out_buf[obl] |= static_cast<ctrff::u8>(((len - 1) << 4) & 0xF0);
      }
      obl++;
      out_buf[obl++] = static_cast<ctrff::u8>(((disp - 1) >> 8) & 0x0F);
      out_buf[obl++] = static_cast<ctrff::u8>((disp - 1) & 0xFF);
    }
    buf_blocks++;
  }

  if (buf_blocks > 0) {
    s.write(reinterpret_cast<const char*>(out_buf), obl);
    res_len += obl;
  }

  if (res_len % 4 != 0) {
    ctrff::u32 pad_len = 4 - (res_len % 4);
    ctrff::u8 pad[4] = {0};
    s.write(reinterpret_cast<const char*>(pad), pad_len);
    res_len += pad_len;
  }

  std::vector<ctrff::u8> res(res_len);
  s.read(reinterpret_cast<char*>(res.data()), res.size());
  return res;
}
}  // namespace LZ11
}  // namespace ctrff
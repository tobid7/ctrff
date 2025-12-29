#include <cstring>
#include <ctrff/lz11.hpp>

/// REWRITTEN CODE FROM BANNERTOOL !!!!!!

namespace ctrff {
namespace LZ11 {
ctrff::u32 GetOccurenceLength(const ctrff::u8* new_ptr, u32 new_len,
                              const ctrff::u8* old_ptr, u32 old_len,
                              ctrff::u32& disp) {
  disp = 0;
  if (new_len == 0) {
    return 0;
  }
  ctrff::u32 res = 0;
  if (old_len > 0) {
    for (u32 i = 0; i < old_len - 1; i++) {
      auto ref = old_ptr + i;
      u32 len = 0;
      for (u32 j = 0; j < new_len; j++) {
        if (*(ref + j) != *(new_ptr + j)) {
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
  struct header {
    u8 Magic;
    u8 Size1;
    u8 Size2;
    u8 Size3;
  };
  header hdr;
  hdr.Magic = 0x11;
  hdr.Size1 = in.size() & 0xFF;
  hdr.Size2 = (in.size() >> 8) & 0xFF;
  hdr.Size3 = (in.size() >> 16) & 0xFF;
  // SETUP HEADER //
  s.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));

  u32 res_len = 4;          // 4-byte header
  ctrff::u8 out_buf[0x21];  // 33 bytes
  memset(out_buf, 0x0, sizeof(out_buf));
  u32 obl = 1;  // out_buf_len
  u32 buf_blocks = 0;
  u32 rb = 0;

  while (rb < in.size()) {
    if (buf_blocks == 8) {
      s.write(reinterpret_cast<const char*>(out_buf), obl);
      res_len += obl;
      memset(out_buf, 0x0, sizeof(out_buf));
      obl = 1;
      buf_blocks = 0;
    }

    ctrff::u32 disp = 0;
    u32 old_len = std::min(rb, static_cast<u32>(0x1000));
    u32 len = LZ11::GetOccurenceLength(
        in.data() + rb,
        std::min((u32)in.size() - rb, static_cast<u32>(0x10110)),
        in.data() + rb - old_len, old_len, disp);

    if (len < 3) {
      out_buf[obl++] = in[rb++];
    } else {
      rb += len;
      out_buf[0] |= static_cast<ctrff::u8>(1 << (7 - buf_blocks));
      if (len > 0x110) {
        out_buf[obl] = 0x10;
        out_buf[obl] |= static_cast<ctrff::u8>(((len - 0x111) >> 12) & 0x0F);
        obl++;
        out_buf[obl] = static_cast<ctrff::u8>(((len - 0x111) >> 4) & 0xFF);
        obl++;
        out_buf[obl] = static_cast<ctrff::u8>(((len - 0x111) << 4) & 0xF0);
      } else if (len > 0x10) {
        out_buf[obl] = 0x00;
        out_buf[obl] |= static_cast<ctrff::u8>(((len - 0x111) >> 4) & 0x0F);
        obl++;
        out_buf[obl] = static_cast<ctrff::u8>(((len - 0x111) << 4) & 0xF0);
      } else {
        out_buf[obl] = static_cast<ctrff::u8>(((len - 1) << 4) & 0xF0);
      }
      out_buf[obl] |= static_cast<ctrff::u8>(((disp - 1) >> 8) & 0x0F);
      obl++;
      out_buf[obl] = static_cast<ctrff::u8>((disp - 1) & 0xFF);
      obl++;
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

  std::string tmp = s.str();
  return std::vector<u8>(tmp.begin(), tmp.end());
}
}  // namespace LZ11
}  // namespace ctrff
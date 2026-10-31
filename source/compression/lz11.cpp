#include <cstring>
#include <ctrff/compression/lz11.hpp>

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
    throw std::runtime_error("[ctrff] LZ11: Input is tool large to compress");
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

CTRFF_API std::vector<ctrff::u8> Decompress(const std::vector<ctrff::u8>& in) {
  ctrff::MemoryStream ms(in);
  size_t rd = 0;  // not relevant for this func
  return DecompressStream(ms, rd);
}

// Elite API

CTRFF_API std::vector<ctrff::u8> DecompressStream(Stream& stream,
                                                  size_t& bytes_read) {
  // Quick setup (and zero out bytes_read / maybe a user forgets to initialize
  // it as 0)
  bytes_read = 0;
  const size_t beg = stream.tellg();
  const size_t avail = stream.size() > beg ? stream.size() - beg : 0;
  size_t pos = 0;
  if (avail == 0) {
    throw std::runtime_error("[ctrff] LZ11: Cannot decompress empty buffer!");
  }
  // Functional stuff
  // read byte
  auto rb = [&](u8& v) -> bool {
    if (pos >= avail) return false;
    char c;
    stream.read(&c, 1);
    v = static_cast<u8>(c);
    pos++;
    return true;
  };
  // read header and setup expected result size
  u8 hdr[4] = {0};
  for (int i = 0; i < 4; i++) {
    if (!rb(hdr[i])) {
      throw std::runtime_error("[ctrff] LZ11: Truncated header!");
    }
  }
  if (hdr[0] != 0x11) {
    throw std::runtime_error("[ctrff] LZ11: Not a lz11 file!");
  }

  const size_t len = hdr[1] | (hdr[2] << 8) | (hdr[3] << 16);
  std::vector<u8> ret(len, 0x0);
  size_t dst_off = 0;
  // pack everything into a func to escape with early return
  // This way we can keep the DecompressStream alive to return whatever we got
  // as data.
  auto run = [&]() {
    while (true) {
      u8 header;
      if (!rb(header)) return;
      for (int i = 0; i < 8; i++) {
        if ((header & 0x80) == 0) {
          u8 v;
          if (!rb(v)) return;
          if (dst_off < len) ret[dst_off] = v;
          dst_off++;
        } else {
          u8 a, b, c, d;
          size_t off2, length;
          if (!rb(a)) return;
          if ((a >> 4) == 0) {
            if (!rb(b) || !rb(c)) return;
            length = (((a & 0xF) << 4) | (b >> 4)) + 0x11;
            off2 = (((b & 0xF) << 8) | c) + 1;
          } else if ((a >> 4) == 1) {
            if (!rb(b) || !rb(c) || !rb(d)) return;
            length = (((a & 0xF) << 12) | (b << 4) | (c >> 4)) + 0x111;
            off2 = (((c & 0xF) << 8) | d) + 1;
          } else {
            if (!rb(b)) return;
            off2 = (((a & 0xF) << 8) | b) + 1;
            length = (a >> 4) + 1;
          }
          for (size_t j = 0; j < length; j++) {
            if (dst_off < len) {
              if (off2 > dst_off) {
                throw std::runtime_error(
                    "[ctrff] LZ11: Underflow in decompress");
              }
              ret[dst_off] = ret[dst_off - off2];
            }
            dst_off++;
          }
        }
        if (dst_off >= len) return;
        header <<= 1;
      }
    }
  };
  run();
  bytes_read = pos;
  return ret;
}
}  // namespace LZ11
}  // namespace ctrff
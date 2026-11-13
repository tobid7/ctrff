#include <ctrff/files/cgfx.hpp>

namespace ctrff {
CTRFF_API void CGFX::Write(Stream& f) const {}

CTRFF_API void CGFX::Read(Stream& f) {
  Header h;
  ReadT(f, h);
  SetEndianess(h.Endianness == 0xfffe);
  f.seekg(0);
  ReadT(f, pHeader);
  if (pHeader.Magic != 0x58464743) {
    throw std::runtime_error("[ctrff] CBMD: Not a CGFX file!");
  }
}
}  // namespace ctrff

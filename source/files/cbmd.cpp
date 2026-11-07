#include <ctrff/files/cbmd.hpp>

namespace ctrff {
CTRFF_API void CBMD::Write(Stream& f) const {}

CTRFF_API void CBMD::Read(Stream& f) {
  size_t size = f.size();
  if (size < sizeof(Header)) {
    throw std::runtime_error("Invalid File!");
  }
  ReadT(f, pHeader);
  if (pHeader.Magic != 0x444d4243) {
    throw std::runtime_error("[ctrff] CBMD: Not a banner file!");
  }
}
}  // namespace ctrff
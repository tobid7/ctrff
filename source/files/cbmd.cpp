#include <ctrff/compression/lz11.hpp>
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
  // decompression (streaming)
  for (int i = 0; i < 14; i++) {
    if (pHeader.Offsets[i] == 0 || pHeader.Offsets[i] >= f.size()) continue;
    f.seekg(pHeader.Offsets[i], std::ios::beg);
    size_t bytes_read = 0;
    auto raw = LZ11::DecompressStream(f, bytes_read);
    pCgfxSizes[i] = raw.size();
    CGFX cgfx;
    cgfx.Load(raw);
    pCgfxs.push_back(cgfx);
  }
}
}  // namespace ctrff
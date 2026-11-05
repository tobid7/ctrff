#pragma once

#include <cstring>
#include <ctrff/core/binutil.hpp>
// #include <ctrff/files/cgfx.hpp> // to be implemented
#include <ctrff/core/helper.hpp>
#include <ctrff/core/types.hpp>
#include <ctrff/files/bcwav.hpp>
#include <filesystem>

namespace ctrff {
class CTRFF_API CBMD : public BinFile {
 public:
  CBMD() : pHeader(Header::Default()) {}
  ~CBMD() {}

  struct Header {
    static Header Default() {
      Header h;
      h.Magic = 0x444d4243;
      h.Zero = 0;
      for (auto& it : h.Offsets) it = 0x0;  // als ob das geht
      for (auto& it : h.Padding) it = 0x0;  // selbes gilt für hier
      h.CwavOffset = 0;
      return h;
    }
    u32 Magic;  // 0x4d494c43 "CLIM"
    u32 Zero;
    u32 Offsets[0xe];
    u8 Padding[0x44];
    u32 CwavOffset;
  };

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

 private:
  Header pHeader;
  // std::vector<CGFX> pCgfxs;
  std::array<u32, 0xe> pCgfxSizes;
  BCWAV pBcwav;
  u32 pBcwavSize;
};
}  // namespace ctrff
#pragma once

#include <ctrff/core/binutil.hpp>
#include <ctrff/core/helper.hpp>
#include <ctrff/core/types.hpp>

namespace ctrff {
class CTRFF_API CGFX : public BinFile {
 public:
  CGFX() {}
  ~CGFX() {}

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  struct Header {
    Header() {
      Magic = 0;
      Endianness = 0xfeff;
      HeaderSize = sizeof(Header);
      Revision = 0;
      FileSize = 0;
      Entries = 0;
    }

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Magic);
      f(s.Endianness);
      f(s.HeaderSize);
      f(s.Revision);
      f(s.FileSize);
      f(s.Entries);
    }

    ctrff::u32 Magic;
    ctrff::u16 Endianness;
    ctrff::u16 HeaderSize;
    ctrff::u32 Revision;
    ctrff::u32 FileSize;
    ctrff::u32 Entries;
  };

 private:
  Header pHeader;
};
}  // namespace ctrff

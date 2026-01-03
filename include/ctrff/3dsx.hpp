#pragma once

#include <ctrff/helper.hpp>
#include <ctrff/smdh.hpp>
#include <ctrff/types.hpp>

namespace ctrff {
class CTRFF_API _3dsx : public BinFile {
 public:
  _3dsx() {}
  ~_3dsx() {}

  void Load(const std::string& path) {
    std::fstream f(path, std::ios::in | std::ios::binary);
    Read(f);
    f.close();
  }

  bool HasMeta() { return SMDHSize == SMDH_Size; }

  /** Write not supported btw */
  void Write(std::fstream& f) const override;
  void Read(std::fstream& f) override;

  ctrff::u32 Magic;  // 0x58534433 "3DSX"
  ctrff::u16 HeaderSize;
  ctrff::u16 RelocHeaderSize;
  ctrff::u32 FormatVersion;
  ctrff::u32 Flags;
  // Sizes of the code, rodata and data segments +
  // size of the BSS section (uninitialized latter half of the data segment)
  ctrff::u32 CodeSegSize;
  ctrff::u32 RodataSegSize;
  ctrff::u32 DataSegSize;
  ctrff::u32 BssSize;
  /// Extended Header ///
  // smdh offset
  ctrff::u32 SMDHOff;
  // smdh size
  ctrff::u32 SMDHSize;
  // fs offset
  ctrff::u32 FsOff;
  SMDH Meta;
};
/** Probably only germen people will understand */
using DreiDSX = _3dsx;
}  // namespace ctrff
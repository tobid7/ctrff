#pragma once

#include <ctrff/helper.hpp>
#include <ctrff/pd_p_api.hpp>
#include <ctrff/smdh.hpp>
#include <pd.hpp>

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

  PD::u32 Magic;
  PD::u16 HeaderSize;
  PD::u16 RelocHeaderSize;
  PD::u32 FormatVersion;
  PD::u32 Flags;
  // Sizes of the code, rodata and data segments +
  // size of the BSS section (uninitialized latter half of the data segment)
  PD::u32 CodeSegSize;
  PD::u32 RodataSegSize;
  PD::u32 DataSegSize;
  PD::u32 BssSize;
  /// Extended Header ///
  // smdh offset
  PD::u32 SMDHOff;
  // smdh size
  PD::u32 SMDHSize;
  // fs offset
  PD::u32 FsOff;
  SMDH Meta;
};
/** Probably only germen people will understand */
using DreiDSX = _3dsx;
}  // namespace ctrff
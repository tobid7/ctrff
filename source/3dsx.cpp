#include <ctrff/3dsx.hpp>
#include <ctrff/binutil.hpp>

namespace ctrff {
CTRFF_API void _3dsx::Write(std::fstream& f) const {
  // To be written
}

CTRFF_API void _3dsx::Read(std::fstream& f) {
  BinUtil r(f);
  r.ReadEx(Magic);
  r.ReadEx(HeaderSize);
  r.ReadEx(RelocHeaderSize);
  r.ReadEx(FormatVersion);
  r.ReadEx(Flags);
  r.ReadEx(CodeSegSize);
  r.ReadEx(RodataSegSize);
  r.ReadEx(DataSegSize);
  r.ReadEx(BssSize);
  r.ReadEx(SMDHOff);
  r.ReadEx(SMDHSize);
  r.ReadEx(FsOff);

  if (HasMeta()) {
    f.seekg(SMDHOff, std::ios::beg);
    Meta.Read(f);
  }
}
}  // namespace ctrff
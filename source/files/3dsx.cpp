#include <ctrff/core/binutil.hpp>
#include <ctrff/files/3dsx.hpp>

namespace ctrff {
CTRFF_API void _3dsx::Write(Stream& f) const {
  // To be written
}

CTRFF_API void _3dsx::Read(Stream& f) {
  SetEndianess(false);
  auto rd = [&](auto& m) { ReadT(f, m); };

  Visit(*this, rd);
  if (Magic != 0x58534433) {
    throw std::runtime_error("3DSX: Invalid 3DSX file!");
  }
  if (HeaderSize >= 44) {
    VisitExt(*this, rd);
  }

  if (HasMeta()) {
    f.seekg(SMDHOff, std::ios::beg);
    Meta.Read(f);
  }
}
}  // namespace ctrff
#include <ctrff/files/cgfx.hpp>

namespace ctrff {
CTRFF_API void CGFX::Write(Stream& f) const {}

CTRFF_API void CGFX::Read(Stream& f) { ReadT(f, pHeader); }
}  // namespace ctrff

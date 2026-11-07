#include <ctrff/files/bclyt.hpp>

namespace ctrff {
void BcLyt::Write(Stream& f) const {
  SetEndianess(pHeader.Endianness == 0xfffe);
  WriteT(f, pHeader);
  f.write(reinterpret_cast<const char*>(pBuffer.data()), pBuffer.size());
}
}  // namespace ctrff
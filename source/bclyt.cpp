#include <ctrff/bclyt.hpp>

namespace ctrff {
void BcLyt::Write(Stream& f) const {
  BinUtil u(f);
  u.SetEndianess(pHeader.Endianness == 0xfffe);
  u.Write(pHeader.Magic);
  u.Write(pHeader.Endianness);
  u.Write(pHeader.HeaderSize);
  u.Write(pHeader.Version);
  u.Write(pHeader.FileSize);
  u.Write(pHeader.NumSections);
  f.write(reinterpret_cast<const char*>(pBuffer.data()), pBuffer.size());
}
}  // namespace ctrff
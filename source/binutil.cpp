#include <ctrff/binutil.hpp>

namespace ctrff {
/** Supported reads */
template CTRFF_API void BinUtil::Read<PD::u8>(PD::u8&);
template CTRFF_API void BinUtil::Read<PD::u16>(PD::u16&);
template CTRFF_API void BinUtil::Read<PD::u32>(PD::u32&);
template CTRFF_API void BinUtil::Read<PD::u64>(PD::u64&);
/** Supported writes */
template CTRFF_API void BinUtil::Write<PD::u8>(const PD::u8&);
template CTRFF_API void BinUtil::Write<PD::u16>(const PD::u16&);
template CTRFF_API void BinUtil::Write<PD::u32>(const PD::u32&);
template CTRFF_API void BinUtil::Write<PD::u64>(const PD::u64&);

template <typename T>
void BinUtil::Read(T& v) {
  // Check if Value could be Read
  static_assert(std::is_integral<T>::value, "Cannot Read type T");
  v = 0;  // Set value to 0 (most cases a windows problem)
  std::vector<PD::u8> buf(sizeof(T), 0);  // declare buffer
  // Read data into buffer
  m_file.read(reinterpret_cast<char*>(buf.data()), sizeof(T));
  // Loop or in be reverse loop and chift the values
  for (int i = 0; i < sizeof(T); i++) {
    v |= static_cast<T>(buf[m_big ? sizeof(T) - 1 - i : i]) << (8 * i);
  }
}
template <typename T>
void BinUtil::Write(const T& v) {
  // Check if Value could Write
  static_assert(std::is_integral<T>::value, "Cannot Write type T");
  std::vector<PD::u8> buf(sizeof(T), 0);  // declare buffer
  // Loop or in be reverse loop and write the values
  for (size_t i = 0; i < sizeof(T); i++) {
    buf[(m_big ? sizeof(T) - 1 - i : i)] = buf[m_big ? sizeof(T) - 1 - i : i] =
        static_cast<PD::u8>((v >> (8 * i)) & 0xFF);
  }
  // Write buffer into file
  m_file.write(reinterpret_cast<const char*>(buf.data()), sizeof(T));
}
}  // namespace ctrff
#include <ctrff/binutil.hpp>

namespace ctrff {
/** Supported reads */
template CTRFF_API void BinUtil::Read<ctrff::u8>(ctrff::u8&);
template CTRFF_API void BinUtil::Read<ctrff::u16>(ctrff::u16&);
template CTRFF_API void BinUtil::Read<ctrff::u32>(ctrff::u32&);
template CTRFF_API void BinUtil::Read<ctrff::u64>(ctrff::u64&);
/** Supported writes */
template CTRFF_API void BinUtil::Write<ctrff::u8>(const ctrff::u8&);
template CTRFF_API void BinUtil::Write<ctrff::u16>(const ctrff::u16&);
template CTRFF_API void BinUtil::Write<ctrff::u32>(const ctrff::u32&);
template CTRFF_API void BinUtil::Write<ctrff::u64>(const ctrff::u64&);

template <typename T>
void BinUtil::Read(T& v) {
  // Check if Value could be Read
  static_assert(std::is_integral<T>::value, "Cannot Read type T");
  v = 0;  // Set value to 0 (most cases a windows problem)
  std::vector<ctrff::u8> buf(sizeof(T), 0);  // declare buffer
  // Read data into buffer
  m_file.read(reinterpret_cast<char*>(buf.data()), sizeof(T));
  // Loop or in be reverse loop and chift the values
  for (size_t i = 0; i < sizeof(T); i++) {
    v |= static_cast<T>(buf[m_big ? sizeof(T) - 1 - i : i]) << (8 * i);
  }
}
template <typename T>
void BinUtil::Write(const T& v) {
  // Check if Value could Write
  static_assert(std::is_integral<T>::value, "Cannot Write type T");
  std::vector<ctrff::u8> buf(sizeof(T), 0);  // declare buffer
  // Loop or in be reverse loop and write the values
  for (size_t i = 0; i < sizeof(T); i++) {
    buf[(m_big ? sizeof(T) - 1 - i : i)] = buf[m_big ? sizeof(T) - 1 - i : i] =
        static_cast<ctrff::u8>((v >> (8 * i)) & 0xFF);
  }
  // Write buffer into file
  m_file.write(reinterpret_cast<const char*>(buf.data()), sizeof(T));
}
}  // namespace ctrff

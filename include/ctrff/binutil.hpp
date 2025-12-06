#pragma once

#include <ctrff/types.hpp>

namespace ctrff {
class BinFile {
 public:
  BinFile() = default;
  ~BinFile() = default;

  virtual void Write(std::fstream& s) const = 0;
  virtual void Read(std::fstream& s) = 0;
};

class CTRFF_API BinUtil {
 public:
  BinUtil(std::fstream& f, bool big = false) : m_file(f), m_big(big) {}
  ~BinUtil() = default;

  void SetEndianess(bool big) { m_big = big; }

  template <typename T>
  void Read(T& v);
  template <typename T>
  void Write(const T& v);
  /** Note that this func ignores Endianness */
  template <typename T>
  void ReadEx(T& v) {
    static_assert(std::is_trivially_copyable_v<T>, "Cannot Read type T");
    m_file.read(reinterpret_cast<char*>(&v), sizeof(T));
  }
  /** Note that this func ignores Endianness */
  template <typename T>
  void WriteEx(T& v) {
    m_file.write(reinterpret_cast<const char*>(&v), sizeof(T));
  }

 private:
  std::fstream& m_file;
  bool m_big;
};
}  // namespace ctrff
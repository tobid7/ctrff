#pragma once

#include <ctrff/streams.hpp>

namespace ctrff {
class BinFile {
 public:
  BinFile() = default;
  ~BinFile() = default;

  virtual void Write(Stream& s) const = 0;
  virtual void Read(Stream& s) = 0;
};

class CTRFF_API BinUtil {
 public:
  BinUtil(Stream& f, bool big = false) : pStream(f), pBig(big) {}
  ~BinUtil() = default;

  void SetEndianess(bool big) { pBig = big; }

  template <typename T>
  void Read(T& v);
  template <typename T>
  void Write(const T& v);
  /** Note that this func ignores Endianness */
  template <typename T>
  void ReadEx(T& v) {
    static_assert(std::is_trivially_copyable_v<T>, "Cannot Read type T");
    pStream.read(reinterpret_cast<char*>(&v), sizeof(T));
  }
  /** Note that this func ignores Endianness */
  template <typename T>
  void WriteEx(T& v) {
    pStream.write(reinterpret_cast<const char*>(&v), sizeof(T));
  }

 private:
  Stream& pStream;
  bool pBig;
};
}  // namespace ctrff
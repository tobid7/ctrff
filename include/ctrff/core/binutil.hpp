#pragma once

#include <ctrff/core/streams.hpp>

namespace ctrff {
class BinFile {
 public:
  BinFile() = default;
  virtual ~BinFile() = default;

  virtual void Load(const std::string& path) {
    FileStream f(path, std::ios::in | std::ios::binary);
    Read(f);
    f.close();
  }

  virtual void Load(const std::vector<u8>& mem) {
    MemoryStream ms(mem);
    Read(ms);
  }

  virtual void Save(const std::string& path) {
    FileStream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  virtual void Save(std::vector<u8>& mem) {
    MemoryStream ms(mem);
    Write(ms);
  }

  virtual void Write(Stream& s) const = 0;
  virtual void Read(Stream& s) = 0;

  void SetEndianess(bool big) const { pBig = big; }

 protected:
  template <typename T>
  void ReadT(Stream& s, T& v) const {
    if constexpr (std::is_arithmetic_v<T>) {
      s.read(reinterpret_cast<char*>(&v), sizeof(T));
      if (NeedSwap()) v = Swap(v);
    } else if constexpr (std::is_array_v<T>) {
      using E = std::remove_extent_t<T>;
      if constexpr (sizeof(E) == 1 && std::is_arithmetic_v<E>) {
        s.read(reinterpret_cast<char*>(v), sizeof(T));
      } else {
        for (auto& e : v) ReadT(s, e);
      }
    } else {
      T::Visit(v, [&](auto& m) { ReadT(s, m); });
    }
  }

  template <typename T>
  void WriteT(Stream& s, const T& v) const {
    if constexpr (std::is_arithmetic_v<T>) {
      T tmp = NeedSwap() ? Swap(v) : v;
      s.write(reinterpret_cast<const char*>(&tmp), sizeof(T));
    } else if constexpr (std::is_array_v<T>) {
      using E = std::remove_extent_t<T>;
      if constexpr (sizeof(E) == 1 && std::is_arithmetic_v<E>) {
        s.write(reinterpret_cast<const char*>(v), sizeof(T));
      } else {
        for (const auto& e : v) WriteT(s, e);
      }
    } else {
      T::Visit(v, [&](const auto& m) { WriteT(s, m); });
    }
  }

 private:
  bool NeedSwap() const {
    return pBig != (std::endian::native == std::endian::big);
  }

  template <typename T>
  static T Swap(T v) {
    if constexpr (sizeof(T) > 1) {
      u8 b[sizeof(T)];
      std::memcpy(b, &v, sizeof(T));
      std::reverse(b, b + sizeof(T));
      std::memcpy(&v, b, sizeof(T));
    }
    return v;
  }

  mutable bool pBig = false;
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
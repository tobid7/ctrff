#pragma once

#include <cstring>
#include <ctrff/core/binutil.hpp>
#include <ctrff/core/helper.hpp>
#include <ctrff/core/types.hpp>
#include <ctrff/textures/pica.hpp>
#include <filesystem>

namespace ctrff {
class CTRFF_API BCLIM : public BinFile {
 public:
  BCLIM() : pCurrent(Header::Default()), pImag(ImagHeader::Default()) {}
  ~BCLIM() {}

  enum Format : u32 {
    L8,  // tested
    A8,  // tested
    LA4,
    LA8,     // tested
    HILO8,   // tested
    RGB565,  // tested
    RGB888,
    RGBA5551,  // tested
    RGBA4444,  // tested
    RGBA8888,  // tested
    ETC1,      // tested
    ETC1A4,    // tested
    L4,
    A4,
  };

  struct Header {
    static Header Default() {
      Header h;
      h.Magic = 0x4d494c43;
      h.Endianness = 0xfeff;
      h.HeaderSize = 0x14;
      h.Version = 0x2020000;
      h.FileSize = 0;
      h.NumSections = 0;
      return h;
    }

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Magic);
      f(s.Endianness);
      f(s.HeaderSize);
      f(s.Version);
      f(s.FileSize);
      f(s.NumSections);
    }

    u32 Magic;  // 0x4d494c43 "CLIM"
    u16 Endianness;
    u16 HeaderSize;
    u32 Version;
    u32 FileSize;
    u32 NumSections;
  };

  struct ImagHeader {
    static ImagHeader Default() {
      ImagHeader h;
      h.Magic = 0x67616d69;
      h.HeaderSize = 0x14;
      h.Width = 0;
      h.Height = 0;
      h.Format = A8;
      h.ImageSize = 0;
      return h;
    }

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Magic);
      f(s.HeaderSize);
      f(s.Width);
      f(s.Height);
      f(s.Format);
      f(s.ImageSize);
    }

    u32 Magic;  // 0x67616d69 "imag"
    u32 HeaderSize;
    u16 Width;
    u16 Height;
    u32 Format;  // A bit waste of data lol
    u32 ImageSize;
  };

  void CreateByImage(const std::vector<u8>& data, int w, int h, Format fmt);

  Format GetFmt() const { return (Format)pImag.Format; }
  std::vector<u8> GetImage() { return pBuffer; }
  int GetWidth() const { return pImag.Width; }
  int GetHeight() const { return pImag.Height; }
  size_t GetByteCount() const { return pBuffer.size(); }

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  static Pica::Color Format2GpuColor(Format fmt);

 private:
  std::vector<u8> pBuffer;
  Header pCurrent;
  ImagHeader pImag;
  bool CreateMode = false;
};
}  // namespace ctrff
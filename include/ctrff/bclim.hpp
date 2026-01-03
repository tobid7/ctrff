#pragma once

#include <cstring>
#include <ctrff/binutil.hpp>
#include <ctrff/helper.hpp>
#include <ctrff/types.hpp>
#include <filesystem>

namespace ctrff {
class CTRFF_API BCLIM : public BinFile {
 public:
  BCLIM() {}
  ~BCLIM() {}

  enum Format : u32 {
    L8,
    A8,  // tested
    LA4,
    LA8,
    HILO8,
    RGB565,  // tested
    RGB888,
    RGBA5551,
    RGBA4444,
    RGBA8888,  // tested
    ETC1,
    ETC1A4,
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
    u32 Magic;  // 0x67616d69 "imag"
    u32 HeaderSize;
    u16 Width;
    u16 Height;
    u32 Format;  // A bit waste of data lol
    u32 ImageSize;
  };

  void Load(const std::string& path) {
    std::fstream f(path, std::ios::in | std::ios::binary);
    Read(f);
    f.close();
  }

  void Save(const std::string& path) {
    std::fstream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  void CreateByImage(const std::vector<u8>& data, int w, int h, Format fmt);

  Format GetFmt() const { return (Format)pImag.Format; }
  std::vector<u8> GetImage() { return pBuffer; }
  int GetWidth() const { return pImag.Width; }
  int GetHeight() const { return pImag.Height; }

  /** Write not supported btw */
  void Write(std::fstream& f) const override;
  void Read(std::fstream& f) override;

 private:
  std::vector<u8> pBuffer;
  Header pCurrent;
  ImagHeader pImag;
  bool CreateMode = false;
};
}  // namespace ctrff
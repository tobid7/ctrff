#pragma once

#include <ctrff/core/binutil.hpp>
#include <ctrff/core/helper.hpp>
#include <ctrff/core/types.hpp>

namespace ctrff {
class CTRFF_API CGFX : public BinFile {
 public:
  CGFX() {}
  ~CGFX() {}

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  enum class DataType {
    Models,
    Textures,
    Luts,
    Materials,
    Shaders,
    Cameras,
    Lights,
    Fog,
    Environments,
    SkeletonAnimations,
    TextureAnimations,
    VisibilityAnimations,
    CameraAnimations,
    LightAnimations,
    Emitters,
    Unknown,
  };

  struct Header {
    Header() {
      Magic = 0;
      Endianness = 0xfeff;
      HeaderSize = sizeof(Header);
      Revision = 0;
      FileSize = 0;
      Entries = 0;
    }

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Magic);
      f(s.Endianness);
      f(s.HeaderSize);
      f(s.Revision);
      f(s.FileSize);
      f(s.Entries);
    }

    ctrff::u32 Magic;
    ctrff::u16 Endianness;
    ctrff::u16 HeaderSize;
    ctrff::u32 Revision;
    ctrff::u32 FileSize;
    ctrff::u32 Entries;
  };

  struct CgfxDictHeader {
    ctrff::u32 Magic;  // 0x41544144 'DATA'
    ctrff::u32 DictSize;
    ctrff::u32 EntryCount;
    ctrff::u32 Unknown1;
    ctrff::u16 Unknown2;
    std::array<ctrff::u8, 0xa> Unknown3;
  };

  struct CgfxDictEntry {
    ctrff::u32 Unknown1;
    ctrff::u16 Unkwnown2;
    ctrff::u16 Unkwnown3;
    ctrff::u32 Offset2sym;
    ctrff::u32 Offset2obj;
  };

 private:
  Header pHeader;
};
}  // namespace ctrff

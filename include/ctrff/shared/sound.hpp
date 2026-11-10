#pragma once

#include <ctrff/core/binutil.hpp>
#include <ctrff/core/types.hpp>

namespace ctrff {
namespace shared {
namespace sound {
enum Endianness : ctrff::u16 {
  Big = 0xfffe,     ///< Big Endian
  Little = 0xfeff,  ///< Little Endian
};

enum Encoding : ctrff::u8 {
  PCM8 = 0,
  PCM16 = 1,
  DSP_ADPCM = 2,
  IMA_ADPCM = 3,
};

struct Reference {
  Reference() : TypeID(0), Padding(0), Offset(0) {}
  Reference(ctrff::u16 t, ctrff::u16 p, ctrff::u32 o)
      : TypeID(t), Padding(p), Offset(o) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.TypeID);
    f(s.Padding);
    f(s.Offset);
  }

  ctrff::u16 TypeID;
  ctrff::u16 Padding;
  ctrff::u32 Offset; /** null -> uint32_max */
};

struct ReferenceTable {
  ReferenceTable() : Count(0) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.Count);
    if constexpr (!std::is_const_v<Self>) {
      s.Refs.resize(s.Count);
    }
    f(s.Refs);
  }

  ctrff::u32 Count;
  std::vector<Reference> Refs;
};

struct SizedReference {
  SizedReference() : Size(0) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.Ref);
    f(s.Size);
  }

  Reference Ref;
  ctrff::u32 Size;
};

struct BlockHeader {
  BlockHeader() : Magic(0), Size(0) {}
  BlockHeader(ctrff::u32 m, ctrff::u32 s) : Magic(m), Size(s) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.Magic);
    f(s.Size);
  }

  ctrff::u32 Magic;
  ctrff::u32 Size;
};

struct DSP_ADPCM_Param {
  // Lets keep this not clean here :/
  DSP_ADPCM_Param() {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.Coefficients);
  }

  ctrff::u16 Coefficients[0x10];
};

struct DSP_ADPCM_Context {
  DSP_ADPCM_Context()
      : PredictorScale(0),
        Reserved(0),
        PreviousSample(0),
        SecondPreviousSample(0) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.PredictorScale);
    f(s.Reserved);
    f(s.PreviousSample);
    f(s.SecondPreviousSample);
  }

  ctrff::u8 PredictorScale;
  ctrff::u8 Reserved;
  ctrff::u16 PreviousSample;
  ctrff::u16 SecondPreviousSample;
};

struct DSP_ADPCM_Info {
  DSP_ADPCM_Info() : Padding(0) {}

  template <class Self, class F>
  static void Visit(Self& s, F&& f) {
    f(s.Param);
    f(s.Context);
    f(s.LoopContext);
    f(s.Padding);
  }

  DSP_ADPCM_Param Param;
  DSP_ADPCM_Context Context;
  DSP_ADPCM_Context LoopContext;
  ctrff::u16 Padding;
};
}  // namespace sound
}  // namespace shared
}  // namespace ctrff
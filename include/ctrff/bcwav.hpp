#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/types.hpp>

namespace ctrff {
class CTRFF_API BCWAV {
 public:
  BCWAV() : pReader(pFile) {}
  ~BCWAV() { CleanUp(); }

  void LoadFile(const std::string& path);
  void CleanUp();
  void ReadGotoBeginning(bool use_loop_beg = false);
  void ReadBlock(ctrff::u32 block, ctrff::u8* ref);

  /** Internal Data (can be made private with private: but public by default) */
  enum Endianness : ctrff::u16 {
    Big = 0xfffe,     ///< Big Endian
    Little = 0xfeff,  ///< Little Endian
  };

  enum ReferenceTypes : ctrff::u16 {
    Ref_DSP_ADPCM_Info = 0x0300,
    Ref_IMA_ADPCM_Info = 0x0301,
    Ref_SampleData = 0x1f00,
    Ref_InfoBlock = 0x7000,
    Ref_DataBlock = 0x7001,
    Ref_ChannelInfo = 0x7100,
  };

  enum Encoding : ctrff::u8 {
    PCM8 = 0,
    PCM16 = 1,
    /** Only supported encoding in BCSTM-Player */
    DSP_ADPCM = 2,
    IMA_ADPCM = 3,
  };

  struct Reference {
    Reference() : TypeID(0), Padding(0), Offset(0) {}
    ctrff::u16 TypeID;
    ctrff::u16 Padding;
    ctrff::u32 Offset; /** null -> uint32_max */
  };

  struct ReferenceTable {
    ReferenceTable() : Count(0) {}
    ctrff::u32 Count;
    std::vector<Reference> Refs;
  };

  struct SizedReference {
    SizedReference() : Size(0) {}
    Reference Ref;
    ctrff::u32 Size;
  };

  struct BlockHeader {
    BlockHeader() : Magic(0), Size(0) {}
    ctrff::u32 Magic;
    ctrff::u32 Size;
  };

  struct InfoBlock {
    InfoBlock()
        : Encoding(0),
          Loop(0),
          Padding(0),
          SampleRate(0),
          LoopStartFrame(0),
          LoopEndFrame(0),
          Reserved(0) {}
    BlockHeader Header;
    ctrff::u8 Encoding;
    ctrff::u8 Loop;
    ctrff::u16 Padding;
    ctrff::u32 SampleRate;
    ctrff::u32 LoopStartFrame;
    ctrff::u32 LoopEndFrame;
    ctrff::u32 Reserved;
    ReferenceTable ChannelInfoTab;
    std::vector<Reference> ChannelInfoRefs; /** The refs of the refs ?? */
  };

  struct DataBlock {
    DataBlock() {
      for (int i = 0; i < 3; i++) {
        Padding[i] = 0;
      }
    }
    BlockHeader Header;
    ctrff::u32 Padding[3];
    std::vector<ctrff::u8> Data;
  };

  struct DSP_ADPCM_Param {
    ctrff::u16 Coefficients[0x10];
  };
  struct DSP_ADPCM_Context {
    DSP_ADPCM_Context()
        : PredictorScale(0),
          Reserved(0),
          PreviousSample(0),
          SecondPreviousSample(0) {}
    ctrff::u8 PredictorScale;
    ctrff::u8 Reserved;
    ctrff::u16 PreviousSample;
    ctrff::u16 SecondPreviousSample;
  };

  struct DSP_ADPCM_Info {
    DSP_ADPCM_Info() : Padding(0) {}
    DSP_ADPCM_Param Param;
    DSP_ADPCM_Context Context;
    DSP_ADPCM_Context LoopContext;
    ctrff::u16 Padding;
  };

  struct Header {
    Header()
        : Magic(0),
          Endianness(Little),
          HeaderSize(0),
          Version(0),
          FileSize(0),
          NumBlocks(0),
          Reserved(0) {}
    ctrff::u32 Magic;               /** CWAV */
    ctrff::u16 Endianness = Little; /** Default */
    ctrff::u16 HeaderSize;          /** Header Size probably */
    ctrff::u32 Version;             /** Format Version? */
    ctrff::u32 FileSize;            /** File Size */
    ctrff::u16 NumBlocks;           /** Number of blocks */
    ctrff::u16 Reserved;            /** Reserved */
  };

  Header pHeader;
  SizedReference pInfoBlockRef;
  SizedReference pDataBlockRef;
  InfoBlock pInfoBlock;
  DataBlock pDataBlock;
  std::vector<DSP_ADPCM_Info> pDSP_ADPCM_Info;
  /** File Stream */
  std::fstream pFile;
  /** Endianness based reader */
  BinUtil pReader;

  void ReadReference(Reference& ref);
  void ReadSizedReference(SizedReference& ref);
  void ReadInfoBlock(InfoBlock& block);
  void ReadReferenceTab(ReferenceTable& tab);

  static std::string Endianness2String(const Endianness& e) {
    switch (e) {
      case Little:
        return "Little";
      case Big:
        return "Big";
      default:
        return "Unknown";
    }
  }

  static std::string ReferenceType2String(const ReferenceTypes& e) {
    switch (e) {
      case Ref_DSP_ADPCM_Info:
        return "DSP_ADPCM_Info";
      case Ref_IMA_ADPCM_Info:
        return "IMA_ADPCM_Info";
      case Ref_SampleData:
        return "SampleData";
      case Ref_InfoBlock:
        return "InfoBlock";
      case Ref_DataBlock:
        return "DataBlock";
      case Ref_ChannelInfo:
        return "ChannelInfo";
      default:
        return "Unknown";
    }
  }

  static std::string Encoding2String(const Encoding& e) {
    switch (e) {
      case PCM8:
        return "PCM8";
      case PCM16:
        return "PCM16";
      case DSP_ADPCM:
        return "DSP ADPCM";
      case IMA_ADPCM:
        return "IMA ADPCM";
      default:
        return "Unknown";
    }
  }
};
}  // namespace ctrff
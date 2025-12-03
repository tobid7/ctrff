#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/pd_p_api.hpp>
#include <pd.hpp>

namespace ctrff {
class CTRFF_API BCWAV {
 public:
  BCWAV() : pReader(pFile) {}
  ~BCWAV() { CleanUp(); }

  void LoadFile(const std::string& path);
  void CleanUp();
  void ReadGotoBeginning(bool use_loop_beg = false);
  void ReadBlock(PD::u32 block, PD::u8* ref);

  /** Internal Data (can be made private with private: but public by default) */
  enum Endianness : PD::u16 {
    Big = 0xfffe,     ///< Big Endian
    Little = 0xfeff,  ///< Little Endian
  };

  enum ReferenceTypes : PD::u16 {
    Ref_DSP_ADPCM_Info = 0x0300,
    Ref_IMA_ADPCM_Info = 0x0301,
    Ref_SampleData = 0x1f00,
    Ref_InfoBlock = 0x7000,
    Ref_DataBlock = 0x7001,
    Ref_ChannelInfo = 0x7100,
  };

  enum Encoding : PD::u8 {
    PCM8 = 0,
    PCM16 = 1,
    /** Only supported encoding in BCSTM-Player */
    DSP_ADPCM = 2,
    IMA_ADPCM = 3,
  };

  struct Reference {
    PD::u16 TypeID;
    PD::u16 Padding;
    PD::u32 Offset; /** null -> uint32_max */
  };

  struct ReferenceTable {
    PD::u32 Count;
    std::vector<Reference> Refs;
  };

  struct SizedReference {
    Reference Ref;
    PD::u32 Size;
  };

  struct StreamInfo {
    PD::u8 Encoding;
    PD::u8 Loop;
    PD::u8 ChannelCount;
    PD::u8 Padding;
    PD::u32 SampleRate;
    PD::u32 LoopStartFrame;
    PD::u32 LoopEndFrame;
    PD::u32 SampleBlockNum;
    PD::u32 SampleBlockSize;
    PD::u32 SampleBlockSampleNum;
    PD::u32 LastSampleBlockSize;
    PD::u32 LastSampleBlockSampleNum;
    PD::u32 LastSampleBlockPaddedSize;
    PD::u32 SeekDataSize;
    PD::u32 SeekIntervalSampleNum;
    Reference SampleDataRef;
  };

  struct BlockHeader {
    PD::u32 Magic;
    PD::u32 Size;
  };

  struct InfoBlock {
    BlockHeader Header;
    PD::u8 Encoding;
    PD::u8 Loop;
    PD::u16 Padding;
    PD::u32 SampleRate;
    PD::u32 LoopStartFrame;
    PD::u32 LoopEndFrame;
    PD::u32 Reserved;
    ReferenceTable ChannelInfoTab;
    std::vector<Reference> ChannelInfoRefs; /** The refs of the refs ?? */
  };

  struct DataBlock {
    BlockHeader Header;
    PD::u32 Padding[3];
    std::vector<PD::u8> Data;
  };

  struct DSP_ADPCM_Param {
    PD::u16 Coefficients[0x10];
  };
  struct DSP_ADPCM_Context {
    PD::u8 PredictorScale;
    PD::u8 Reserved;
    PD::u16 PreviousSample;
    PD::u16 SecondPreviousSample;
  };

  struct DSP_ADPCM_Info {
    DSP_ADPCM_Param Param;
    DSP_ADPCM_Context Context;
    DSP_ADPCM_Context LoopContext;
    PD::u16 Padding;
  };

  struct Header {
    PD::u32 Magic;               /** CWAV */
    PD::u16 Endianness = Little; /** Default */
    PD::u16 HeaderSize;          /** Header Size probably */
    PD::u32 Version;             /** Format Version? */
    PD::u32 FileSize;            /** File Size */
    PD::u16 NumBlocks;           /** Number of blocks */
    PD::u16 Reserved;            /** Reserved */
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
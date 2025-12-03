#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/pd_p_api.hpp>
#include <pd.hpp>

namespace ctrff {
class CTRFF_API BCSTM {
 public:
  BCSTM() : pReader(pFile) {}
  ~BCSTM() { CleanUp(); }

  void LoadFile(const std::string& path);
  void CleanUp();
  void ReadGotoBeginning(bool use_loop_beg = false);
  void ReadBlock(PD::u32 block, PD::u8* ref);

  /** Some useful Getters */
  PD::u8 GetNumChannels() const { return pInfoBlock.StreamInfo.ChannelCount; }
  PD::u32 GetSampleRate() const { return pInfoBlock.StreamInfo.SampleRate; }
  PD::u32 GetBlockSize() const { return pInfoBlock.StreamInfo.SampleBlockSize; }
  PD::u32 GetNumBlocks() const { return pInfoBlock.StreamInfo.SampleBlockNum; }
  PD::u32 GetBlockSamples() const {
    return pInfoBlock.StreamInfo.SampleBlockSampleNum;
  }
  PD::u32 GetLastBlockSamples() const {
    return pInfoBlock.StreamInfo.LastSampleBlockSampleNum;
  }
  bool IsLooping() const { return pInfoBlock.StreamInfo.Loop; }
  PD::u32 GetLoopStart() const {
    return pInfoBlock.StreamInfo.LoopStartFrame / GetBlockSamples();
  }
  PD::u32 GetLoopEnd() const {
    /** Get temp references for better readability */
    const PD::u32& loop_end = pInfoBlock.StreamInfo.LoopEndFrame;
    const PD::u32& block_samples = GetBlockSamples();
    return (loop_end % block_samples ? GetNumBlocks()
                                     : loop_end / block_samples);
  }

  /** Internal Data (can be made private with private: but public by default) */
  enum Endianness : PD::u16 {
    Big = 0xfffe,     ///< Big Endian
    Little = 0xfeff,  ///< Little Endian
  };

  enum ReferenceTypes : PD::u16 {
    Ref_ByteTable = 0x0100,
    Ref_ReferenceTable = 0x0101,
    Ref_DSP_ADPCM_Info = 0x0300,
    Ref_IMA_ADPCM_Info = 0x0301,
    Ref_SampleData = 0x1f00,
    Ref_InfoBlock = 0x4000,
    Ref_SeekBlock = 0x4001,
    Ref_DataBlock = 0x4002,
    Ref_StreamInfo = 0x4100,
    Ref_TrackInfo = 0x4101,
    Ref_ChannelInfo = 0x4102,
  };

  enum Encoding : PD::u8 {
    PCM8 = 0,
    PCM16 = 1,
    /** Only supported encoding in BCSTM-Player */
    DSP_ADPCM = 2,
    IMA_ADPCM = 3,
  };

  struct Reference {
    Reference() : TypeID(0), Padding(0), Offset(0) {}
    Reference(PD::u16 t, PD::u16 p, PD::u32 o)
        : TypeID(t), Padding(p), Offset(o) {}
    PD::u16 TypeID;
    PD::u16 Padding;
    PD::u32 Offset; /** null -> uint32_max */
  };

  struct ReferenceTable {
    ReferenceTable() : Count(0) {}
    PD::u32 Count;
    std::vector<Reference> Refs;
  };

  struct SizedReference {
    SizedReference() : Size(0) {}
    Reference Ref;
    PD::u32 Size;
  };

  struct StreamInfo {
    StreamInfo();
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
    BlockHeader() : Magic(0), Size(0) {}
    BlockHeader(PD::u32 m, PD::u32 s) : Magic(m), Size(s) {}
    PD::u32 Magic;
    PD::u32 Size;
  };

  struct InfoBlock {
    InfoBlock() = default;
    BlockHeader Header;
    Reference StreamInfoRef;
    Reference TrackInfoTabRef;
    Reference ChannelInfoTabRef;
    BCSTM::StreamInfo StreamInfo;
    ReferenceTable TrackInfoTab;
    ReferenceTable ChannelInfoTab;
    std::vector<Reference> ChannelInfoRefs; /** The refs of the refs ?? */
  };

  /** SeekDataBlock cause they are the same struct */
  struct SD_Block {
    SD_Block() {}
    BlockHeader Header;
    std::vector<PD::u8> Data;
  };

  struct DSP_ADPCM_Param {
    // Lets keep this not clean here :/
    DSP_ADPCM_Param() {}
    PD::u16 Coefficients[0x10];
  };

  struct DSP_ADPCM_Context {
    DSP_ADPCM_Context()
        : PredictorScale(0),
          Reserved(0),
          PreviousSample(0),
          SecondPreviousSample(0) {}
    PD::u8 PredictorScale;
    PD::u8 Reserved;
    PD::u16 PreviousSample;
    PD::u16 SecondPreviousSample;
  };

  struct DSP_ADPCM_Info {
    DSP_ADPCM_Info() : Padding(0) {}
    DSP_ADPCM_Param Param;
    DSP_ADPCM_Context Context;
    DSP_ADPCM_Context LoopContext;
    PD::u16 Padding;
  };

  struct ByteTable {
    ByteTable(PD::u32 size = 0) : Size(size) {}
    PD::u32 Size;
    std::vector<PD::u8> Table;
  };

  struct TrackInfo {
    TrackInfo() : Volume(0), Pan(0), Padding(0) {}
    PD::u8 Volume;
    PD::u8 Pan;
    PD::u16 Padding;
    Reference ChennelIndexTabRef;
    ByteTable ChannelIndexTab;
  };

  struct Header {
    // Warum sieht dass so ~~nicht~~ gut aus...
    Header()
        : Magic(0),
          Endianness(Little),
          HeaderSize(0),
          Version(0),
          FileSize(0),
          NumBlocks(0),
          Reserved(0) {}
    PD::u32 Magic;               /** CSTM */
    PD::u16 Endianness = Little; /** Default */
    PD::u16 HeaderSize;          /** Header Size probably */
    PD::u32 Version;             /** Format Version? */
    PD::u32 FileSize;            /** File Size */
    PD::u16 NumBlocks;           /** Number of blocks */
    PD::u16 Reserved;            /** Reserved */
  };

  Header pHeader;
  SizedReference pInfoBlockRef;
  SizedReference pSeekBlockRef;
  SizedReference pDataBlockRef;
  InfoBlock pInfoBlock;
  SD_Block pSeekBlock;
  SD_Block pDataBlock;
  std::vector<DSP_ADPCM_Info> pDSP_ADPCM_Info;
  /** File Stream */
  std::fstream pFile;
  /** Endianness based reader */
  BinUtil pReader;

  void ReadReference(Reference& ref);
  void ReadSizedReference(SizedReference& ref);
  void ReadInfoBlock(InfoBlock& block);
  void ReadSeekBlock(SD_Block& block);
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
      case Ref_ByteTable:
        return "ByteTable";
      case Ref_ReferenceTable:
        return "ReferenceTable";
      case Ref_DSP_ADPCM_Info:
        return "DSP_ADPCM_Info";
      case Ref_IMA_ADPCM_Info:
        return "IMA_ADPCM_Info";
      case Ref_SampleData:
        return "SampleData";
      case Ref_InfoBlock:
        return "InfoBlock";
      case Ref_SeekBlock:
        return "SeekBlock";
      case Ref_DataBlock:
        return "DataBlock";
      case Ref_StreamInfo:
        return "StreamInfo";
      case Ref_TrackInfo:
        return "TrackInfo";
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

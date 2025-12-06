#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/types.hpp>

namespace ctrff {
class CTRFF_API BCSTM {
 public:
  BCSTM() : pReader(pFile) {}
  ~BCSTM() { CleanUp(); }

  void LoadFile(const std::string& path);
  void CleanUp();
  void ReadGotoBeginning(bool use_loop_beg = false);
  void ReadBlock(ctrff::u32 block, ctrff::u8* ref);

  /** Some useful Getters */
  ctrff::u8 GetNumChannels() const {
    return pInfoBlock.StreamInfo.ChannelCount;
  }
  ctrff::u32 GetSampleRate() const { return pInfoBlock.StreamInfo.SampleRate; }
  ctrff::u32 GetBlockSize() const {
    return pInfoBlock.StreamInfo.SampleBlockSize;
  }
  ctrff::u32 GetNumBlocks() const {
    return pInfoBlock.StreamInfo.SampleBlockNum;
  }
  ctrff::u32 GetBlockSamples() const {
    return pInfoBlock.StreamInfo.SampleBlockSampleNum;
  }
  ctrff::u32 GetLastBlockSamples() const {
    return pInfoBlock.StreamInfo.LastSampleBlockSampleNum;
  }
  bool IsLooping() const { return pInfoBlock.StreamInfo.Loop; }
  ctrff::u32 GetLoopStart() const {
    return pInfoBlock.StreamInfo.LoopStartFrame / GetBlockSamples();
  }
  ctrff::u32 GetLoopEnd() const {
    /** Get temp references for better readability */
    const ctrff::u32& loop_end = pInfoBlock.StreamInfo.LoopEndFrame;
    const ctrff::u32& block_samples = GetBlockSamples();
    return (loop_end % block_samples ? GetNumBlocks()
                                     : loop_end / block_samples);
  }

  /** Internal Data (can be made private with private: but public by default) */
  enum Endianness : ctrff::u16 {
    Big = 0xfffe,     ///< Big Endian
    Little = 0xfeff,  ///< Little Endian
  };

  enum ReferenceTypes : ctrff::u16 {
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

  enum Encoding : ctrff::u8 {
    PCM8 = 0,
    PCM16 = 1,
    /** Only supported encoding in BCSTM-Player */
    DSP_ADPCM = 2,
    IMA_ADPCM = 3,
  };

  struct Reference {
    Reference() : TypeID(0), Padding(0), Offset(0) {}
    Reference(ctrff::u16 t, ctrff::u16 p, ctrff::u32 o)
        : TypeID(t), Padding(p), Offset(o) {}
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

  struct StreamInfo {
    StreamInfo();
    ctrff::u8 Encoding;
    ctrff::u8 Loop;
    ctrff::u8 ChannelCount;
    ctrff::u8 Padding;
    ctrff::u32 SampleRate;
    ctrff::u32 LoopStartFrame;
    ctrff::u32 LoopEndFrame;
    ctrff::u32 SampleBlockNum;
    ctrff::u32 SampleBlockSize;
    ctrff::u32 SampleBlockSampleNum;
    ctrff::u32 LastSampleBlockSize;
    ctrff::u32 LastSampleBlockSampleNum;
    ctrff::u32 LastSampleBlockPaddedSize;
    ctrff::u32 SeekDataSize;
    ctrff::u32 SeekIntervalSampleNum;
    Reference SampleDataRef;
  };

  struct BlockHeader {
    BlockHeader() : Magic(0), Size(0) {}
    BlockHeader(ctrff::u32 m, ctrff::u32 s) : Magic(m), Size(s) {}
    ctrff::u32 Magic;
    ctrff::u32 Size;
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
    std::vector<ctrff::u8> Data;
  };

  struct DSP_ADPCM_Param {
    // Lets keep this not clean here :/
    DSP_ADPCM_Param() {}
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

  struct ByteTable {
    ByteTable(ctrff::u32 size = 0) : Size(size) {}
    ctrff::u32 Size;
    std::vector<ctrff::u8> Table;
  };

  struct TrackInfo {
    TrackInfo() : Volume(0), Pan(0), Padding(0) {}
    ctrff::u8 Volume;
    ctrff::u8 Pan;
    ctrff::u16 Padding;
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
    ctrff::u32 Magic;               /** CSTM */
    ctrff::u16 Endianness = Little; /** Default */
    ctrff::u16 HeaderSize;          /** Header Size probably */
    ctrff::u32 Version;             /** Format Version? */
    ctrff::u32 FileSize;            /** File Size */
    ctrff::u16 NumBlocks;           /** Number of blocks */
    ctrff::u16 Reserved;            /** Reserved */
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

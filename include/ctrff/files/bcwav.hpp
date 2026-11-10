#pragma once

#include <ctrff/core/binutil.hpp>
#include <ctrff/core/types.hpp>
#include <ctrff/shared/sound.hpp>

namespace ctrff {
class CTRFF_API BCWAV : public BinFile {
 public:
  BCWAV() : pReader(pFile), BinFile(true) {}
  ~BCWAV() { CleanUp(); }

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  void LoadFile(const std::string& path);
  void CleanUp();
  void ReadGotoBeginning(bool use_loop_beg = false);
  void ReadBlock(ctrff::u32 block, ctrff::u8* ref);

  using Endianess = shared::sound::Endianness;
  enum ReferenceTypes : ctrff::u16 {
    Ref_DSP_ADPCM_Info = 0x0300,
    Ref_IMA_ADPCM_Info = 0x0301,
    Ref_SampleData = 0x1f00,
    Ref_InfoBlock = 0x7000,
    Ref_DataBlock = 0x7001,
    Ref_ChannelInfo = 0x7100,
  };

  using Encoding = shared::sound::Encoding;
  using Reference = shared::sound::Reference;
  using SizedReference = shared::sound::SizedReference;
  using ReferenceTable = shared::sound::ReferenceTable;
  using BlockHeader = shared::sound::BlockHeader;

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

  using DSP_ADPCM_Param = shared::sound::DSP_ADPCM_Param;
  using DSP_ADPCM_Context = shared::sound::DSP_ADPCM_Context;
  using DSP_ADPCM_Info = shared::sound::DSP_ADPCM_Info;

  struct Header {
    Header()
        : Magic(0),
          Endianness(Endianess::Little),
          HeaderSize(0),
          Version(0),
          FileSize(0),
          NumBlocks(0),
          Reserved(0) {}
    ctrff::u32 Magic;                          /** CWAV */
    ctrff::u16 Endianness = Endianess::Little; /** Default */
    ctrff::u16 HeaderSize;                     /** Header Size probably */
    ctrff::u32 Version;                        /** Format Version? */
    ctrff::u32 FileSize;                       /** File Size */
    ctrff::u16 NumBlocks;                      /** Number of blocks */
    ctrff::u16 Reserved;                       /** Reserved */
  };

  Header pHeader;
  SizedReference pInfoBlockRef;
  SizedReference pDataBlockRef;
  InfoBlock pInfoBlock;
  DataBlock pDataBlock;
  std::vector<DSP_ADPCM_Info> pDSP_ADPCM_Info;
  /** File Stream */
  FileStream pFile;
  /** Endianness based reader */
  BinUtil pReader;

  void ReadReference(Reference& ref);
  void ReadSizedReference(SizedReference& ref);
  void ReadInfoBlock(InfoBlock& block);
  void ReadReferenceTab(ReferenceTable& tab);

  static std::string Endianness2String(const Endianess& e) {
    switch (e) {
      case Endianess::Little:
        return "Little";
      case Endianess::Big:
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
      case Encoding::PCM8:
        return "PCM8";
      case Encoding::PCM16:
        return "PCM16";
      case Encoding::DSP_ADPCM:
        return "DSP ADPCM";
      case Encoding::IMA_ADPCM:
        return "IMA ADPCM";
      default:
        return "Unknown";
    }
  }
};
}  // namespace ctrff
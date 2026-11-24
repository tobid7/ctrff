#pragma once

#include <ctrff/core/binutil.hpp>
#include <ctrff/core/types.hpp>
#include <ctrff/shared/sound.hpp>

namespace ctrff {
class CTRFF_API BCWAV : public SoundFile {
 public:
  BCWAV() {}
  ~BCWAV() { CleanUp(); }

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  void CleanUp() override;
  // placeholder
  void ReadGotoBeginning(bool use_loop_beg = false) override {}
  void ReadBlock(ctrff::u8 chn, ctrff::u32 block, ctrff::u8* ref) override;

  /**
   * As BCWAV has no Blocks so we split the Samples into virtual ones
   * 0x3800 samples = 0x2000 bytes DSP ADPCM (multiple of 14 samples)
   */
  static constexpr ctrff::u32 BlockSamples = 0x3800;

  ctrff::u8 GetEncoding() const override { return pInfoBlock.Encoding; }
  ctrff::u8 GetNumChannels() const override {
    return pInfoBlock.ChannelInfoTab.Count;
  }
  ctrff::u32 GetSampleRate() const override { return pInfoBlock.SampleRate; }
  /** LoopEndFrame is also the total number of samples */
  ctrff::u32 GetNumSamples() const { return pInfoBlock.LoopEndFrame; }
  ctrff::u32 GetBlockSize() const override {
    return SamplesToBytes(BlockSamples);
  }
  ctrff::u32 GetNumBlocks() const override {
    return (GetNumSamples() + BlockSamples - 1) / BlockSamples;
  }
  ctrff::u32 GetBlockSamples() const override { return BlockSamples; }
  ctrff::u32 GetLastBlockSamples() const override {
    if (!GetNumBlocks()) return 0;
    return GetNumSamples() - (GetNumBlocks() - 1) * BlockSamples;
  }
  bool IsLooping() const override { return pInfoBlock.Loop; }
  ctrff::u32 GetLoopStart() const override {
    return pInfoBlock.LoopStartFrame / BlockSamples;
  }
  ctrff::u32 GetLoopEnd() const override { return GetNumBlocks(); }

  using Endianness = shared::sound::Endianness;
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
  using DSP_ADPCM_Param = shared::sound::DSP_ADPCM_Param;
  using DSP_ADPCM_Context = shared::sound::DSP_ADPCM_Context;
  using DSP_ADPCM_Info = shared::sound::DSP_ADPCM_Info;

  struct ChannelInfo {
    ChannelInfo() : Reserved(0) {}

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.SampleRef);
      f(s.AdpcmInfoRef);
      f(s.Reserved);
    }

    Reference SampleRef;    /** Relative to DataBlock + 8 */
    Reference AdpcmInfoRef; /** Relative to the ChannelInfo itself */
    ctrff::u32 Reserved;
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

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Header);
      f(s.Encoding);
      f(s.Loop);
      f(s.Padding);
      f(s.SampleRate);
      f(s.LoopStartFrame);
      f(s.LoopEndFrame);
      f(s.Reserved);
      f(s.ChannelInfoTab);
    }

    BlockHeader Header;
    ctrff::u8 Encoding;
    ctrff::u8 Loop;
    ctrff::u16 Padding;
    ctrff::u32 SampleRate;
    ctrff::u32 LoopStartFrame;
    ctrff::u32 LoopEndFrame;
    ctrff::u32 Reserved;
    ReferenceTable ChannelInfoTab; /** Offsets relative to the table */
    std::vector<ChannelInfo> ChannelInfos;
  };

  struct Header {
    Header()
        : Magic(0),
          Endianness(Endianness::Little),
          HeaderSize(0),
          Version(0),
          FileSize(0),
          NumBlocks(0),
          Reserved(0) {}

    template <class Self, class F>
    static void Visit(Self& s, F&& f) {
      f(s.Magic);
      f(s.Endianness);
      f(s.HeaderSize);
      f(s.Version);
      f(s.FileSize);
      f(s.NumBlocks);
      f(s.Reserved);
    }

    ctrff::u32 Magic;                           /** CWAV */
    ctrff::u16 Endianness = Endianness::Little; /** Default */
    ctrff::u16 HeaderSize;                      /** Header Size probably */
    ctrff::u32 Version;                         /** Format Version? */
    ctrff::u32 FileSize;                        /** File Size */
    ctrff::u16 NumBlocks;                       /** Number of blocks */
    ctrff::u16 Reserved;                        /** Reserved */
  };

  Header pHeader;
  SizedReference pInfoBlockRef;
  SizedReference pDataBlockRef;
  InfoBlock pInfoBlock;

  void ReadInfoBlock(Stream& f, InfoBlock& block);
  ctrff::u32 SamplesToBytes(ctrff::u32 samples) const;

  static std::string Endianness2String(const Endianness& e) {
    switch (e) {
      case Endianness::Little:
        return "Little";
      case Endianness::Big:
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

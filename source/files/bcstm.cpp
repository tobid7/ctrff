#include <ctrff/files/bcstm.hpp>

/** Using this a single time so inline it */
inline ctrff::u32 Swap32(ctrff::u32 in) {
  return (in >> 24) | ((in >> 8) & 0x0000FF00) | ((in << 8) & 0x00FF0000) |
         (in << 24);
}

namespace ctrff {
CTRFF_API void BCSTM::Write(Stream& f) const {}

CTRFF_API void BCSTM::Read(Stream& f) {
  Header h;
  ReadT(f, h);
  SetEndianess(h.Endianness == 0xfffe);
  f.seekg(0, std::ios::beg);
  ReadT(f, pHeader);
  /** Check for 'CSTM' */
  if (pHeader.Magic != 0x4D545343) {
    throw std::runtime_error("BCSTM Error: Invalid File!");
  }
  for (u16 i = 0; i < pHeader.NumBlocks; i++) {
    SizedReference r;
    ReadT(f, r);
    if (r.Ref.TypeID == Ref_InfoBlock) {
      pInfoBlockRef = r;
    } else if (r.Ref.TypeID == Ref_SeekBlock) {
      pSeekBlockRef = r;
    } else if (r.Ref.TypeID == Ref_DataBlock) {
      pDataBlockRef = r;
    }
  }
  f.seekg(pInfoBlockRef.Ref.Offset);
  ReadInfoBlock(f, pInfoBlock);
  ReadGotoBeginning();
}

CTRFF_API void BCSTM::ReadGotoBeginning(bool use_loop_beg) {
  /**
   * Go Up by 0x20 to skip header and empty section
   * due to 0x20 alignment
   */
  size_t off = pDataBlockRef.Ref.Offset + 0x20;
  /** Shift to loop start if enabled */
  if (use_loop_beg) {
    off += GetLoopStart() * GetBlockSize() * GetNumChannels();
  }
  Seek(off);

  if (TellG() > pHeader.FileSize) {
    throw std::runtime_error("BCSTM: Seeked Out of range!");
  }
}

CTRFF_API void BCSTM::ReadBlock(ctrff::u32 block, ctrff::u8* ref) {
  if (!ref) {
    throw std::runtime_error("BCSTM: pointer ref is nullptr!");
  }
  if (TellG() > pHeader.FileSize || block >= GetNumBlocks()) {
    throw std::runtime_error(std::format(
        "BCSTM: Decode block out of range! ({}/{})", block, GetNumBlocks()));
  }
  ReadRaw(reinterpret_cast<char*>(ref),
          (block == (GetNumBlocks() - 1)
               ? pInfoBlock.StreamInfo.LastSampleBlockPaddedSize
               : GetBlockSize()));
}

CTRFF_API void BCSTM::ReadInfoBlock(Stream& f, InfoBlock& block) {
  ReadT(f, block);
  // I know we are at this offset after reading and this seek is useless
  f.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
          block.StreamInfoRef.Offset);
  ReadT(f, block.StreamInfo);
  f.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
          block.TrackInfoTabRef.Offset);
  ReadT(f, block.TrackInfoTab);
  f.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
          block.ChannelInfoTabRef.Offset);
  ReadT(f, block.ChannelInfoTab);
  for (auto& it : block.ChannelInfoTab.Refs) {
    f.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
                block.ChannelInfoTabRef.Offset + it.Offset,
            std::ios::beg);
    Reference r;
    ReadT(f, r);
    block.ChannelInfoRefs.push_back(r);
  }
  if (block.StreamInfo.Encoding == Encoding::DSP_ADPCM) {
    SetEndianess(false);  // little endian
    for (size_t i = 0; i < block.ChannelInfoRefs.size(); i++) {
      size_t off = pInfoBlockRef.Ref.Offset;
      off += sizeof(BlockHeader);
      off += block.ChannelInfoTabRef.Offset;
      off += block.ChannelInfoTab.Refs[i].Offset;
      off += block.ChannelInfoRefs[i].Offset;
      f.seekg(off, std::ios::beg);
      DSP_ADPCM_Info t; /** temp */
      ReadT(f, t);      /** This Section gets read normally */
      pDSP_ADPCM_Info.push_back(t);
    }
    SetEndianess(pHeader.Endianness == 0xfffe);
  }
}

CTRFF_API void BCSTM::CleanUp() {
  pInfoBlock = InfoBlock();
  pHeader = Header();
  pInfoBlockRef = SizedReference();
  pSeekBlockRef = SizedReference();
  pDataBlockRef = SizedReference();
  pInfoBlock = InfoBlock();
  pSeekBlock = SD_Block();
  pDataBlock = SD_Block();
  pDSP_ADPCM_Info.clear();
}

CTRFF_API BCSTM::StreamInfo::StreamInfo() {
  ChannelCount = 0;
  Encoding = 0;
  LastSampleBlockPaddedSize = 0;
  LastSampleBlockSampleNum = 0;
  LastSampleBlockSize = 0;
  Loop = 0;
  LoopEndFrame = 0;
  LoopStartFrame = 0;
  Padding = 0;
  SampleBlockNum = 0;
  SampleBlockSampleNum = 0;
  SampleBlockSize = 0;
  SampleRate = 0;
  SeekDataSize = 0;
  SeekIntervalSampleNum = 0;
}
}  // namespace ctrff

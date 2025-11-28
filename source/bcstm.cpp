#include <ctrff/bcstm.hpp>

/** Using this a single time so inline it */
inline PD::u32 Swap32(PD::u32 in) {
  return (in >> 24) | ((in >> 8) & 0x0000FF00) | ((in << 8) & 0x00FF0000) |
         (in << 24);
}

namespace ctrff {
CTRFF_API void BCSTM::LoadFile(const std::string& path) {
  CleanUp();
  pFile.open(path, std::ios::in | std::ios::binary);
  if (!pFile.is_open()) {
    throw std::runtime_error("BCSTM Error: File not found!");
  }

  pReader.Read(pHeader.Magic);
  pReader.Read(pHeader.Endianness);
  if (pHeader.Endianness == Big) {
    pHeader.Magic = Swap32(pHeader.Magic);
  }
  /** Check for 'CSTM' */
  if (pHeader.Magic != 0x4D545343) {
    throw std::runtime_error("BCSTM Error: Invalid File!");
  }
  pReader.Read(pHeader.HeaderSize);
  pReader.Read(pHeader.Version);
  pReader.Read(pHeader.FileSize);
  pReader.Read(pHeader.NumBlocks);
  pReader.Read(pHeader.Reserved);
  for (PD::u16 i = 0; i < pHeader.NumBlocks; i++) {
    SizedReference ref;
    ReadSizedReference(ref);
    if (ref.Ref.TypeID == Ref_InfoBlock) {
      pInfoBlockRef = ref;
    } else if (ref.Ref.TypeID == Ref_SeekBlock) {
      pSeekBlockRef = ref;
    } else if (ref.Ref.TypeID == Ref_DataBlock) {
      pDataBlockRef = ref;
    }
  }
  pFile.seekg(pInfoBlockRef.Ref.Offset);
  ReadInfoBlock(pInfoBlock);
  ReadGotoBeginning();
}

CTRFF_API void BCSTM::ReadReference(Reference& ref) {
  pReader.Read(ref.TypeID);
  pReader.Read(ref.Padding);
  pReader.Read(ref.Offset);
}
CTRFF_API void BCSTM::ReadSizedReference(SizedReference& ref) {
  ReadReference(ref.Ref);
  pReader.Read(ref.Size);
}

CTRFF_API void BCSTM::ReadInfoBlock(InfoBlock& block) {
  pReader.Read(block.Header.Magic);
  pReader.Read(block.Header.Size);
  ReadReference(block.StreamInfoRef);
  ReadReference(block.TrackInfoTabRef);
  ReadReference(block.ChannelInfoTabRef);
  pReader.Read(block.StreamInfo.Encoding);
  if (block.StreamInfo.Encoding != DSP_ADPCM) {
    throw std::runtime_error("Only DSP ADPCM is supported yet!");
  }
  pReader.Read(block.StreamInfo.Loop);
  pReader.Read(block.StreamInfo.ChannelCount);
  pReader.Read(block.StreamInfo.Padding);
  pReader.Read(block.StreamInfo.SampleRate);
  pReader.Read(block.StreamInfo.LoopStartFrame);
  pReader.Read(block.StreamInfo.LoopEndFrame);
  pReader.Read(block.StreamInfo.SampleBlockNum);
  pReader.Read(block.StreamInfo.SampleBlockSize);
  pReader.Read(block.StreamInfo.SampleBlockSampleNum);
  pReader.Read(block.StreamInfo.LastSampleBlockSize);
  pReader.Read(block.StreamInfo.LastSampleBlockSampleNum);
  pReader.Read(block.StreamInfo.LastSampleBlockPaddedSize);
  pReader.Read(block.StreamInfo.SeekDataSize);
  pReader.Read(block.StreamInfo.SeekIntervalSampleNum);
  ReadReference(block.StreamInfo.SampleDataRef);
  if (block.TrackInfoTabRef.Offset != 0xffffffff) {
    pFile.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
                    block.TrackInfoTabRef.Offset,
                std::ios::beg);
    ReadReferenceTab(block.TrackInfoTab);
  }
  if (block.ChannelInfoTabRef.Offset != 0xffffffff) {
    pFile.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
                    block.ChannelInfoTabRef.Offset,
                std::ios::beg);
    ReadReferenceTab(block.ChannelInfoTab);
  }
  for (auto& it : block.ChannelInfoTab.Refs) {
    pFile.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) +
                    block.ChannelInfoTabRef.Offset + it.Offset,
                std::ios::beg);
    Reference r;
    ReadReference(r);
    block.ChannelInfoRefs.Add(r);
  }
  for (size_t i = 0; i < block.ChannelInfoRefs.Size(); i++) {
    size_t off = pInfoBlockRef.Ref.Offset;
    off += sizeof(BlockHeader);
    off += block.ChannelInfoTabRef.Offset;
    off += block.ChannelInfoTab.Refs[i].Offset;
    off += block.ChannelInfoRefs[i].Offset;
    pFile.seekg(off, std::ios::beg);
    DSP_ADPCM_Info t;  /** temp */
    pReader.ReadEx(t); /** This Section gets read normally */
    pDSP_ADPCM_Info.Add(t);
  }
}

CTRFF_API void BCSTM::ReadSeekBlock(SD_Block& block) {
  pReader.Read(pSeekBlock.Header.Magic);
  pReader.Read(pSeekBlock.Header.Size);
  if ((pSeekBlock.Header.Size % 20) != 0) {
    throw std::runtime_error("BCSTM: SeekBlock Size is not 0x20 aligned!");
  }

  pSeekBlock.Data.Reserve(pSeekBlock.Header.Size + 1);
  for (PD::u32 i = 0; i < pSeekBlock.Header.Size; i++) {
    PD::u8 v;
    pReader.Read(v);
    pSeekBlock.Data.Add(v);
  }
}

CTRFF_API void BCSTM::ReadReferenceTab(ReferenceTable& tab) {
  pReader.Read(tab.Count);
  tab.Refs.Reserve(tab.Count + 1);
  for (PD::u32 i = 0; i < tab.Count; i++) {
    Reference r;
    pReader.Read(r.TypeID);
    pReader.Read(r.Padding);
    pReader.Read(r.Offset);
    tab.Refs.Add(r);
  }
}

CTRFF_API void BCSTM::ReadGotoBeginning(bool use_loop_beg) {
  /**
   * Go Up by 0x20 to skip header and empty section
   * due to 0x20 alignment
   */
  size_t off = pDataBlockRef.Ref.Offset + 0x20;
  /** Shift to loop start if enabled */
  if (use_loop_beg) {
    off += GetNumBlocks() * GetNumChannels() * GetLoopStart();
    // off += GetNumChannels() * pInfoBlock.StreamInfo.LoopStartFrame;
  }
  // block_size * channel_count * loop_start
  try {
    pFile.seekg(off, std::ios::beg);
  } catch (const std::exception& e) {
    throw std::runtime_error(e.what());
  }
  if (pFile.tellg() > pHeader.FileSize) {
    throw std::runtime_error("BCSTM: Seeked Out of range!");
  }
}

CTRFF_API void BCSTM::ReadBlock(PD::u32 block, PD::u8* ref) {
  if (!ref) {
    throw std::runtime_error("BCSTM: pointer ref is nullptr!");
  }
  if (pFile.tellg() > pHeader.FileSize || block >= GetNumBlocks()) {
    throw std::runtime_error(std::format(
        "BCSTM: Decode block out of range! ({}/{})", block, GetNumBlocks()));
  }
  pFile.read(
      reinterpret_cast<char*>(ref),
      (block == (GetNumBlocks() - 1) ? pInfoBlock.StreamInfo.LastSampleBlockSize
                                     : GetBlockSize()));
}

CTRFF_API void BCSTM::CleanUp() {
  if (pFile.is_open()) {
    try {
      pFile.close();
    } catch (const std::exception& e) {
      throw std::runtime_error(e.what());
    }
  }
  pInfoBlock.ChannelInfoRefs.Clear();
  pInfoBlock.ChannelInfoTab.Refs.Clear();
  pInfoBlock.ChannelInfoTab.Count = 0;
  pInfoBlock.TrackInfoTab.Refs.Clear();
  pInfoBlock.TrackInfoTab.Count = 0;
  pDSP_ADPCM_Info.Clear();
}
}  // namespace ctrff

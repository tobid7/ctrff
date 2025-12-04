#include <ctrff/bcwav.hpp>

/** Using this a single time so inline it */
inline PD::u32 Swap32(PD::u32 in) {
  return (in >> 24) | ((in >> 8) & 0x0000FF00) | ((in << 8) & 0x00FF0000) |
         (in << 24);
}

namespace ctrff {
CTRFF_API void BCWAV::LoadFile(const std::string& path) {
  CleanUp();
  pFile.open(path, std::ios::in | std::ios::binary);
  if (!pFile.is_open()) {
    throw std::runtime_error("BCWAV Error: File not found!");
  }

  pReader.Read(pHeader.Magic);
  pReader.Read(pHeader.Endianness);
  if (pHeader.Endianness == Big) {
    pHeader.Magic = Swap32(pHeader.Magic);
  }
  /** Check for 'CWAV' */
  if (pHeader.Magic != 0x56415743) {
    throw std::runtime_error("BCWAV Error: Invalid File!");
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
    } else if (ref.Ref.TypeID == Ref_DataBlock) {
      pDataBlockRef = ref;
    }
  }
  pFile.seekg(pInfoBlockRef.Ref.Offset);
  ReadInfoBlock(pInfoBlock);
}

CTRFF_API void BCWAV::ReadReference(Reference& ref) {
  pReader.Read(ref.TypeID);
  pReader.Read(ref.Padding);
  pReader.Read(ref.Offset);
}
CTRFF_API void BCWAV::ReadSizedReference(SizedReference& ref) {
  ReadReference(ref.Ref);
  pReader.Read(ref.Size);
}

CTRFF_API void BCWAV::ReadInfoBlock(InfoBlock& block) {
  pReader.Read(block.Header.Magic);
  pReader.Read(block.Header.Size);
  pReader.Read(block.Encoding);
  pReader.Read(block.Loop);
  pReader.Read(block.Padding);
  pReader.Read(block.SampleRate);
  pReader.Read(block.LoopStartFrame);
  pReader.Read(block.LoopEndFrame);
  pReader.Read(block.Reserved);
  ReadReferenceTab(block.ChannelInfoTab);
  for (auto& it : block.ChannelInfoTab.Refs) {
    pFile.seekg(pInfoBlockRef.Ref.Offset + sizeof(BlockHeader) + it.Offset,
                std::ios::beg);
    Reference r;
    ReadReference(r);
    block.ChannelInfoRefs.push_back(r);
  }
  if (block.Encoding == DSP_ADPCM) {
    for (size_t i = 0; i < block.ChannelInfoRefs.size(); i++) {
      size_t off = pInfoBlockRef.Ref.Offset;
      off += sizeof(BlockHeader);
      off += block.ChannelInfoTab.Refs[i].Offset;
      off += block.ChannelInfoRefs[i].Offset;
      pFile.seekg(off, std::ios::beg);
      DSP_ADPCM_Info t;  /** temp */
      pReader.ReadEx(t); /** This Section gets read normally */
      pDSP_ADPCM_Info.push_back(t);
    }
  }
}

CTRFF_API void BCWAV::ReadReferenceTab(ReferenceTable& tab) {
  pReader.Read(tab.Count);
  tab.Refs.reserve(tab.Count + 1);
  for (PD::u32 i = 0; i < tab.Count; i++) {
    Reference r;
    pReader.Read(r.TypeID);
    pReader.Read(r.Padding);
    pReader.Read(r.Offset);
    tab.Refs.push_back(r);
  }
}

CTRFF_API void BCWAV::ReadGotoBeginning(bool use_loop_beg) {
  /**
   * Go Up by 0x20 to skip header and empty section
   * due to 0x20 alignment
   */
  size_t off = pDataBlockRef.Ref.Offset + 0x20;
  /** Shift to loop start if enabled */
  if (use_loop_beg) {
    // off += GetNumBlocks() * GetNumChannels() * GetLoopStart();
  }
  pFile.seekg(off, std::ios::beg);
  if (pFile.tellg() > pHeader.FileSize) {
    throw std::runtime_error("BCWAV: Seeked Out of range!");
  }
}

CTRFF_API void BCWAV::ReadBlock(PD::u32 block, PD::u8* ref) {
  // if (pFile.tellg() > pHeader.FileSize || block >= GetNumBlocks()) {
  //   throw std::runtime_error("BCWAV: Decode block Out of range!");
  // }
  // pFile.read(
  //     reinterpret_cast<char*>(ref),
  //     (block == (GetNumBlocks() - 1) ?
  //     pInfoBlock.StreamInfo.LastSampleBlockSize
  //                                    : GetBlockSize()));
}

CTRFF_API void BCWAV::CleanUp() {
  if (pFile.is_open()) {
    try {
      pFile.close();
    } catch (const std::exception& e) {
      throw std::runtime_error(e.what());
    }
  }
  pInfoBlock = InfoBlock();
  pHeader = Header();
  pInfoBlockRef = SizedReference();
  pDataBlockRef = SizedReference();
  // Does not get read idk why i added it
  // cause it needs to be streamed
  pDataBlock = DataBlock();
  pDSP_ADPCM_Info.clear();
}
}  // namespace ctrff

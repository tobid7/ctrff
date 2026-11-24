#include <ctrff/files/bcwav.hpp>

namespace ctrff {
CTRFF_API void BCWAV::Write(Stream& f) const {}

CTRFF_API void BCWAV::Read(Stream& f) {
  Header h;
  ReadT(f, h);
  SetEndianess(h.Endianness == Endianness::Big);
  f.seekg(0, std::ios::beg);
  ReadT(f, pHeader);
  /** Check for 'CWAV' */
  if (pHeader.Magic != 0x56415743) {
    throw std::runtime_error("BCWAV Error: Invalid File!");
  }
  for (u16 i = 0; i < pHeader.NumBlocks; i++) {
    SizedReference r;
    ReadT(f, r);
    if (r.Ref.TypeID == Ref_InfoBlock) {
      pInfoBlockRef = r;
    } else if (r.Ref.TypeID == Ref_DataBlock) {
      pDataBlockRef = r;
    }
  }
  f.seekg(pInfoBlockRef.Ref.Offset);
  ReadInfoBlock(f, pInfoBlock);
}

CTRFF_API void BCWAV::ReadInfoBlock(Stream& f, InfoBlock& block) {
  ReadT(f, block);
  /** The channel info reference table starts at 0x1C of the info block */
  size_t tab = pInfoBlockRef.Ref.Offset + 0x1c;
  for (auto& it : block.ChannelInfoTab.Refs) {
    f.seekg(tab + it.Offset, std::ios::beg);
    ChannelInfo c;
    ReadT(f, c);
    block.ChannelInfos.push_back(c);
  }
  if (block.Encoding == Encoding::DSP_ADPCM) {
    SetEndianess(false);  // little endian
    for (size_t i = 0; i < block.ChannelInfos.size(); i++) {
      f.seekg(tab + block.ChannelInfoTab.Refs[i].Offset +
                  block.ChannelInfos[i].AdpcmInfoRef.Offset,
              std::ios::beg);
      DSP_ADPCM_Info t;
      ReadT(f, t);
      pDSP_ADPCM_Info.push_back(t);
    }
    SetEndianess(pHeader.Endianness == Endianness::Big);
  }
}

CTRFF_API void BCWAV::ReadBlock(ctrff::u8 chn, ctrff::u32 block,
                                ctrff::u8* ref) {
  if (!ref) {
    throw std::runtime_error("BCWAV: pointer ref is nullptr!");
  }
  if (chn >= GetNumChannels() || block >= GetNumBlocks()) {
    throw std::runtime_error(
        std::format("BCWAV: Decode block out of range! ({}/{}, chn {})", block,
                    GetNumBlocks(), chn));
  }
  size_t off = pDataBlockRef.Ref.Offset + sizeof(BlockHeader) +
               pInfoBlock.ChannelInfos[chn].SampleRef.Offset +
               block * GetBlockSize();
  Seek(off);
  ReadRaw(reinterpret_cast<char*>(ref),
          (block == (GetNumBlocks() - 1)
               ? SamplesToBytes(GetLastBlockSamples())
               : GetBlockSize()));
}

CTRFF_API ctrff::u32 BCWAV::SamplesToBytes(ctrff::u32 samples) const {
  if (GetEncoding() == Encoding::DSP_ADPCM) {
    /** 8 byte frames of 14 samples */
    return (samples + 13) / 14 * 8;
  } else if (GetEncoding() == Encoding::PCM16) {
    return samples * 2;
  }
  return samples;
}

CTRFF_API void BCWAV::CleanUp() {
  pHeader = Header();
  pInfoBlockRef = SizedReference();
  pDataBlockRef = SizedReference();
  pInfoBlock = InfoBlock();
  pDSP_ADPCM_Info.clear();
}
}  // namespace ctrff

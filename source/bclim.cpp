#include <ctrff/bclim.hpp>

namespace ctrff {
CTRFF_API void BCLIM::Write(std::fstream& f) const {
  f.write(reinterpret_cast<const char*>(pBuffer.data()), pBuffer.size());
  f.write((const char*)&pCurrent.Magic, sizeof(pCurrent.Magic));
  f.write((const char*)&pCurrent.Endianness, sizeof(pCurrent.Endianness));
  f.write((const char*)&pCurrent.HeaderSize, sizeof(pCurrent.HeaderSize));
  f.write((const char*)&pCurrent.Version, sizeof(pCurrent.Version));
  f.write((const char*)&pCurrent.FileSize, sizeof(pCurrent.FileSize));
  f.write((const char*)&pCurrent.NumSections, sizeof(pCurrent.NumSections));
  f.write((const char*)&pImag.Magic, sizeof(pImag.Magic));
  f.write((const char*)&pImag.HeaderSize, sizeof(pImag.HeaderSize));
  f.write((const char*)&pImag.Width, sizeof(pImag.Width));
  f.write((const char*)&pImag.Height, sizeof(pImag.Height));
  f.write((const char*)&pImag.Format, sizeof(pImag.Format));
  f.write((const char*)&pImag.ImageSize, sizeof(pImag.ImageSize));
}

CTRFF_API void BCLIM::Read(std::fstream& f) {
  f.seekg(0, std::ios::end);
  size_t size = f.tellg();
  if (size < (sizeof(Header) + sizeof(ImagHeader))) {
    throw std::runtime_error("Invalid File!");
  }
  f.seekg(size - sizeof(Header) - sizeof(ImagHeader));
  f.read(reinterpret_cast<char*>(&pCurrent), sizeof(pCurrent));
  f.read(reinterpret_cast<char*>(&pImag), sizeof(pImag));
  if (pCurrent.Magic != 0x4d494c43) {
    throw std::runtime_error("[ctrff] BCLIM: Not a bclim file!");
  }
  if (pImag.Magic != 0x67616d69) {
    throw std::runtime_error("[ctrff] BCLIM: Invalid Data");
  }
  f.seekg(0, std::ios::beg);
  pBuffer.resize(pImag.ImageSize);
  f.read(reinterpret_cast<char*>(pBuffer.data()), pBuffer.size());
}

CTRFF_API void BCLIM::CreateByImage(const std::vector<u8>& data, int w, int h,
                                    Format fmt) {
  CreateMode = true;
  pImag = ImagHeader::Default();
  pCurrent = Header::Default();
  pImag.Format = fmt;
  pImag.Width = w;
  pImag.Height = h;
  pImag.ImageSize = data.size();
  pBuffer.resize(data.size());
  for (int i = 0; i < data.size(); i++) {
    pBuffer[i] = data[i];
  }
  pCurrent.FileSize = pBuffer.size() + pCurrent.HeaderSize + pImag.HeaderSize;
  pCurrent.NumSections = 1;
}
}  // namespace ctrff
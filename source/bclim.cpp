#include <ctrff/bclim.hpp>

namespace ctrff {
CTRFF_API void BCLIM::Write(Stream& f) const {
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

CTRFF_API void BCLIM::Read(Stream& f) {
  size_t size = f.size();
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
  for (size_t i = 0; i < data.size(); i++) {
    pBuffer[i] = data[i];
  }
  pCurrent.FileSize = pBuffer.size() + pCurrent.HeaderSize + pImag.HeaderSize;
  pCurrent.NumSections = 1;
}

Pica::Color BCLIM::Format2GpuColor(Format fmt) {
  switch (fmt) {
    case Format::L8:
      return Pica::Color::L8;
    case Format::A8:
      return Pica::Color::A8;
    case Format::LA4:
      return Pica::Color::LA4;
    case Format::LA8:
      return Pica::Color::LA8;
    case Format::HILO8:
      return Pica::Color::HILO8;
    case Format::RGB565:
      return Pica::Color::RGB565;
    case Format::RGB888:
      return Pica::Color::RGB888;
    case Format::RGBA5551:
      return Pica::Color::RGBA5551;
    case Format::RGBA4444:
      return Pica::Color::RGBA4444;
    case Format::RGBA8888:
      return Pica::Color::RGBA8888;
    case Format::ETC1:
      return Pica::Color::ETC1;
    case Format::ETC1A4:
      return Pica::Color::ETC1A4;
    case Format::L4:
      return Pica::Color::L4;
    case Format::A4:
      return Pica::Color::A4;
    default:
      return Pica::Color::A8;
  }
}
}  // namespace ctrff
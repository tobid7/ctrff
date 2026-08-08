#include <ctrff/bclan.hpp>

namespace ctrff {
void BcLan::pInsertU32(std::vector<u8>& d, u32 val) {
  d.push_back(val & 0xFF);
  d.push_back((val >> 8) & 0xFF);
  d.push_back((val >> 16) & 0xFF);
  d.push_back((val >> 24) & 0xFF);
}

void BcLan::pInsertString(std::vector<u8>& d, const std::string& str,
                          size_t fixlen) {
  for (size_t i = 0; i < fixlen; ++i) {
    if (i < str.length())
      d.push_back(str[i]);
    else
      d.push_back(0);
  }
}

void BcLan::pAddSection(u32 magic, const std::vector<u8>& data) {
  pInsertU32(pBuffer, magic);
  pInsertU32(pBuffer, 8 + data.size());
  pBuffer.insert(pBuffer.end(), data.begin(), data.end());
  pHeader.NumSections++;
}

void BcLan::CreateLogoAnim(const std::string& scene_name,
                           const std::string& group_name) {
  pHeader.FileSize = 0;
  pHeader.NumSections = 0;
  pBuffer.clear();

  // pat1
  std::vector<u8> pat1;
  pInsertU32(pat1, 0x10000);
  pInsertU32(pat1, 0x1c);
  pInsertU32(pat1, 0x28);
  pInsertU32(pat1, -15);
  pInsertU32(pat1, 0x1);
  pInsertString(pat1, scene_name, 0xc);
  pInsertString(pat1, group_name, 0x14);
  pAddSection(0x31746170, pat1);

  // pai1
  std::vector<u8> pai1;
  pInsertU32(pai1, 15);
  pai1.push_back(0);
  pai1.push_back(0);
  pai1.push_back(2);
  pai1.push_back(0);  // num_entries = 2
  pInsertU32(pai1, 0x14);
  pInsertU32(pai1, 0x1c);
  pInsertU32(pai1, 0x28 + 0x18);

  pInsertString(pai1, "HbMat", 0x14);
  pInsertU32(pai1, 0x0);
  pInsertString(pai1, "HbRoot0", 0x14);
  pInsertU32(pai1, 0x0);
  pAddSection(0x31696170, pai1);

  pHeader.FileSize = sizeof(BcLan::Header) + pBuffer.size();
}

void BcLan::Write(Stream& f) const {
  BinUtil u(f);
  u.SetEndianess(pHeader.Endianness == 0xfffe);
  u.Write(pHeader.Magic);
  u.Write(pHeader.Endianness);
  u.Write(pHeader.HeaderSize);
  u.Write(pHeader.Version);
  u.Write(pHeader.FileSize);
  u.Write(pHeader.NumSections);
  f.write(reinterpret_cast<const char*>(pBuffer.data()), pBuffer.size());
}
}  // namespace ctrff
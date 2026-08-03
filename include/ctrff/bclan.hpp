#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/types.hpp>

// based on yellows8/ctr-logobuilder

namespace ctrff {
class CTRFF_API BcLan : public BinFile {
 public:
  struct Header {
    static Header Default() {
      Header h;
      h.Magic = 0x4e414c43;  // "CLAN"
      h.Endianness = 0xfeff;
      h.HeaderSize = 0x14;
      h.Version = 0x2020000;
      h.FileSize = 0;
      h.NumSections = 0;
      return h;
    }
    u32 Magic;
    u16 Endianness;
    u16 HeaderSize;
    u32 Version;
    u32 FileSize;
    u32 NumSections;
  };
  BcLan() { pHeader = Header::Default(); }
  ~BcLan() {}

  void Save(const std::string& path) {
    std::fstream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  void Write(std::fstream& f) const;
  void Read(std::fstream& f) override {}

  void CreateLogoAnim(const std::string& scene_name,
                      const std::string& group_name);

 private:
  Header pHeader;
  std::vector<u8> pBuffer;
  void pInsertU32(std::vector<u8>& vec, u32 val);
  void pInsertString(std::vector<u8>& vec, const std::string& str,
                     size_t fixlen);
  void pAddSection(u32 magic, const std::vector<u8>& data);
};
}  // namespace ctrff
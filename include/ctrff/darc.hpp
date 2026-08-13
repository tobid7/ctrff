#pragma once

#include <cstring>
#include <ctrff/binutil.hpp>
#include <ctrff/helper.hpp>
#include <ctrff/types.hpp>
#include <filesystem>

namespace ctrff {
class CTRFF_API Darc : public BinFile {
 public:
  Darc() {}
  ~Darc() {}

  struct DarcNode {
    std::u16string Name;
    std::string FsPath;
    bool IsDir;

    u32 NameOffset = 0;
    u32 DataOffset = 0;
    u32 DataSize = 0;
  };

  void Load(const std::string& path) {
    pStream = new FileStream(path, std::ios::in | std::ios::binary);
    Read(*pStream);
    pStream->close();
    pLoadedFilePath = path;
  }

  void Load(std::vector<u8> mem) {
    // taking mem for not going out of scope
    pMemBuf = std::move(mem);
    pStream = new MemoryStream(pMemBuf);
    Read(*pStream);
    // just something telling ExtractTo we are in loading mode
    pLoadedFilePath = "<mem>";
  }

  void Save(const std::string& path) {
    FileStream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  void Save(std::vector<u8>& mem) {
    MemoryStream ms(mem);
    Write(ms);
  }

  void Write(Stream& f) const override;
  void Read(Stream& f) override;

  void BuildFromDirectory(const std::string& path);
  void ExtractTo(const std::string& path) const;
  const std::vector<DarcNode>& GetEntries() { return pEntries; }
  std::vector<u8> ExtractFile(u32 idx);

  struct Header {
    static Header Default() {
      Header h;
      h.Magic = 0x63726164;
      h.Endianness = 0xfeff;
      h.HeaderSize = 0x1c;
      h.Version = 0x1000000;
      h.TableOffset = 0x1c;
      h.TableSize = 0x0;
      h.DataOffset = 0x0;
      h.FileSize = 0x0;
      return h;
    }
    u32 Magic;  // 0x63726164 "darc"
    u16 Endianness;
    u16 HeaderSize;
    u32 Version;
    u32 FileSize;
    u32 TableOffset;
    u32 TableSize;
    u32 DataOffset;
  };

  struct TableEntry {
    TableEntry() {
      FileNameOffset = 0;
      Offset = 0;
      Size = 0;
    }
    u32 FileNameOffset;
    u32 Offset;
    u32 Size;
  };

  // private:
  std::vector<ctrff::u8> pMemBuf;
  Stream* pStream = nullptr;
  std::vector<DarcNode> pEntries;
  u32 pTableSize = 0;
  u32 pDataOffsetBase = 0;
  u32 pTotalFileSize = 0;
  u32 AddDirectory(const std::string& path);
  void pBuildPaths(u32 start_idx, u32 end_idx, const std::string& base);
  // This way we can check if we are in creation or loading mode
  std::string pLoadedFilePath = "";
};
}  // namespace ctrff
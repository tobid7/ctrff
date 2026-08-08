#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/types.hpp>

// based on yellows8/ctr-logobuilder

namespace ctrff {
class CTRFF_API BcLyt : public BinFile {
 public:
  struct Header {
    static Header Default() {
      Header h;
      h.Magic = 0x54594c43;  // "CLYT"
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
  BcLyt() { pHeader = Header::Default(); }
  ~BcLyt() {}

  void Load(const std::string& path) {
    FileStream f(path, std::ios::in | std::ios::binary);
    Read(f);
    f.close();
  }

  void Save(const std::string& path) {
    FileStream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  void Write(Stream& f) const override;
  void Read(Stream& f) override {}

  // private:
  using DataBuffer = std::vector<u8>;  // make naming better
  void pInsertU32(DataBuffer& d, u32 v) {
    d.push_back(v & 0xFF);
    d.push_back((v >> 8) & 0xFF);
    d.push_back((v >> 16) & 0xFF);
    d.push_back((v >> 24) & 0xFF);
  }
  void pInsertFloat(DataBuffer& d, float v) {
    u32 t;
    std::memcpy(&t, &v, sizeof(float));
    pInsertU32(d, t);
  }
  void pInsertString(DataBuffer& d, const std::string& str, size_t fixlen) {
    for (size_t i = 0; i < fixlen; i++) {
      if (i < str.length()) {
        d.push_back(str[i]);
      } else {
        d.push_back(0);
      }
    }
  }
  void pAddSection(u32 magic, const std::vector<u8>& data) {
    pInsertU32(pBuffer, magic);
    pInsertU32(pBuffer, 8 + data.size());
    pBuffer.insert(pBuffer.end(), data.begin(), data.end());
    pHeader.NumSections++;
  }
  void pAddMatSection(const std::string& name) {
    std::vector<u8> data;
    pInsertU32(data, 1);
    pInsertU32(data, 0x10);
    pInsertString(data, name, 0x14);

    pInsertU32(data, 0x00ffffff);
    for (int i = 0; i < 3; i++) pInsertU32(data, 0xffffffff);  // colors

    pInsertU32(data, 0x15);        // flags
    pInsertU32(data, 0x04040000);  // Texture index/mapping

    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);  // Trans
    pInsertFloat(data, 0.0f);  // Rot
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 1.0f);  // Scale
    pInsertU32(data, 0x0);

    pAddSection(0x3174616d, data);
  }
  void pAddPanSection(const std::string& name, u8 visibility, float width,
                      float height) {
    std::vector<u8> data(0x1C, 0);
    data[0] = visibility;
    data[1] = 0x4;   // origin
    data[2] = 0xff;  // alpha
    std::memcpy(&data[4], name.c_str(), std::min(name.length(), (size_t)0xf));

    // Trans
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    // Rot
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    // Scale
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, width);
    pInsertFloat(data, height);

    pAddSection(0x316e6170, data);
    pAddSection(0x31736170, {});  // Start Pane
  }
  void pAddPicSection(const std::string& name, float width, float height,
                      float x, float y, float z) {
    std::vector<u8> data(0x1C, 0);
    data[0] = 0x1;
    data[1] = 0x7;
    data[2] = 0xff;
    data[3] = 0x0;
    std::memcpy(&data[4], name.c_str(), std::min(name.length(), (size_t)0x17));

    pInsertFloat(data, x);
    pInsertFloat(data, y);
    pInsertFloat(data, z);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, width);
    pInsertFloat(data, height);

    pInsertU32(data, 0xffffffff);
    pInsertU32(data, 0xffffffff);
    pInsertU32(data, 0xffffffff);
    pInsertU32(data, 0xffffffff);
    pInsertU32(data, 0x10000);
    pInsertU32(data, 0x0);
    pInsertU32(data, 0x0);

    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 0.0f);
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 1.0f);
    pInsertFloat(data, 1.0f);

    pAddSection(0x31636970, data);
  }
  void pAddGroupSection(const std::string& groupname,
                        const std::vector<std::string>& panerefs) {
    std::vector<u8> data;
    pInsertString(data, groupname, 0x10);
    pInsertU32(data, panerefs.size());
    for (const auto& ref : panerefs) {
      pInsertString(data, ref, 0x10);
    }
    pAddSection(0x31707267, data);
  }
  void CreateLogoLayout(int screenid, const std::string& tex_filename,
                        const float tex_coords[5]) {
    pHeader = Header::Default();
    pBuffer.clear();
    float screen_width = (screenid == 0) ? 320.0f : 400.0f;

    std::vector<u8> lyt1;
    pInsertU32(lyt1, 0x1);
    pInsertFloat(lyt1, screen_width);
    pInsertFloat(lyt1, 240.f);
    pAddSection(0x3174796c, lyt1);

    std::vector<u8> txl1;
    pInsertU32(txl1, 0x1);
    pInsertU32(txl1, 0x4);
    size_t size = tex_filename.length() + 1;
    size = (size + 0x3) & ~0x3;
    pInsertString(txl1, tex_filename, size);
    pAddSection(0x316c7874, txl1);

    pAddMatSection("HbMat");
    pAddPanSection("HbRootPane", 0x1, screen_width, 240.f);
    pAddPanSection("HbRoot0", 0x3, 40.f, 40.f);
    pAddPicSection("HbMat", tex_coords[0], tex_coords[1], tex_coords[2],
                   tex_coords[3], tex_coords[4]);

    pAddSection(0x31656170, {});  // Finish Pane (Pic)
    pAddSection(0x31656170, {});  // Finish Pane (Root0)
    pAddSection(0x31656170, {});  // Finish Pane (RootPane)

    pAddGroupSection("HbRootGrp", {});
    pAddSection(0x31737267, {});  // Start Group

    pAddGroupSection("G_A_00", {"HbRoot0", "HbMat"});
    pAddGroupSection("G_B_00", {"HbMat"});
    pAddGroupSection("G_C_00", {"HbRoot0"});

    pAddSection(0x31657267, {});  // End Group

    pHeader.FileSize = sizeof(Header) + pBuffer.size();
  }
  Header pHeader;
  std::vector<u8> pBuffer;
};
}  // namespace ctrff
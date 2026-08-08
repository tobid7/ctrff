#include <ctrff/darc.hpp>

namespace ctrff {
u32 Darc::AddDirectory(const std::string& path) {
  u32 entries = 0;

  for (auto& it : std::filesystem::directory_iterator(path)) {
    u32 idx = pEntries.size();

    u16 buffer[256] = {0};
    String2U16(buffer, it.path().filename().string(), sizeof(buffer));
    std::u16string name16 = (char16_t*)buffer;

    if (it.is_directory()) {
      pEntries.push_back({name16, it.path().string(), true, 0, 1, 0});
      u32 childs = AddDirectory(it.path().string());

      pEntries[idx].DataSize = idx + 1 + childs;
      entries += 1 + childs;
    } else if (it.is_regular_file()) {
      u32 fsize = std::filesystem::file_size(it.path());
      pEntries.push_back({name16, it.path().string(), false, 0, 0, fsize});
      entries += 1;
    }
  }
  return entries;
}

void Darc::pBuildPaths(u32 start_idx, u32 end_idx, const std::string& base) {
  u32 i = start_idx;
  while (i < end_idx) {
    auto& node = pEntries[i];

    std::string node_name_u8;
    for (char16_t c : node.Name) {
      node_name_u8 += static_cast<char>(c);
    }

    node.FsPath = base.empty() ? node_name_u8 : base + "/" + node_name_u8;

    if (node.IsDir) {
      pBuildPaths(i + 1, node.DataSize, node.FsPath);
      i = node.DataSize;
    } else {
      i++;
    }
  }
}

CTRFF_API void Darc::BuildFromDirectory(const std::string& path) {
  pEntries.clear();
  // Base setup
  pEntries.push_back({std::u16string(u""), "", true, 0, 1, 0});
  pEntries.push_back({std::u16string(u"."), "", true, 0, 1, 0});

  AddDirectory(path);

  pEntries[0].DataSize = pEntries.size();
  pEntries[1].DataSize = pEntries.size();

  // string table
  u32 current_string_offset = 0;
  for (auto& entry : pEntries) {
    entry.NameOffset = current_string_offset;
    if (entry.IsDir) {
      entry.NameOffset |= 0x01000000;
    }
    current_string_offset += (entry.Name.length() + 1) * 2;
  }

  u32 string_table_bytes = current_string_offset;
  u32 table_byte_size =
      (pEntries.size() * sizeof(TableEntry)) + string_table_bytes;

  table_byte_size = (table_byte_size + 3) & ~3;
  u32 filedata_offset = 0x1C + table_byte_size;

  u32 tmp = filedata_offset & 0x1F;
  if (tmp) {
    table_byte_size += 0x20 - tmp;
    filedata_offset += 0x20 - tmp;
  }

  pTableSize = table_byte_size;
  pDataOffsetBase = filedata_offset;

  // Setup offsets
  // bclim files get a special alignment
  for (size_t i = 2; i < pEntries.size(); i++) {
    auto& entry = pEntries[i];
    if (!entry.IsDir) {
      // force 0x80 alignment for GPU textures
      if (entry.FsPath.length() >= 6 &&
          entry.FsPath.substr(entry.FsPath.length() - 6) == ".bclim") {
        filedata_offset = (filedata_offset + 0x7F) & ~0x7F;
      }

      entry.DataOffset = filedata_offset;
      filedata_offset += entry.DataSize;

      // 0x20 padding for everything else
      if (i != pEntries.size() - 1) {
        filedata_offset = (filedata_offset + 0x1F) & ~0x1F;
      }
    }
  }

  pTotalFileSize = filedata_offset;
}

void Darc::Write(Stream& f) const {
  Header h = Header::Default();
  h.TableSize = pTableSize;
  h.DataOffset = pDataOffsetBase;
  h.FileSize = pTotalFileSize;

  BinUtil u(f);
  u.SetEndianess(h.Endianness == 0xFFFE);
  u.Write(h.Magic);
  u.Write(h.Endianness);
  u.Write(h.HeaderSize);
  u.Write(h.Version);
  u.Write(h.FileSize);
  u.Write(h.TableOffset);
  u.Write(h.TableSize);
  u.Write(h.DataOffset);

  // Table
  for (const auto& entry : pEntries) {
    u.Write(entry.NameOffset);
    u.Write(entry.DataOffset);
    u.Write(entry.DataSize);
  }

  // Stringtable
  for (const auto& entry : pEntries) {
    for (char16_t c : entry.Name) {
      u.Write(static_cast<u16>(c));
    }
    u.Write(static_cast<u16>(0));
  }

  // Insert padding
  while (static_cast<u32>(f.tellp()) < h.DataOffset) {
    char pad = 0;
    f.write(&pad, 1);
  }

  // Write files and padding
  for (const auto& entry : pEntries) {
    if (!entry.IsDir) {
      while (static_cast<u32>(f.tellp()) < entry.DataOffset) {
        char pad = 0;
        f.write(&pad, 1);
      }

      std::ifstream in(entry.FsPath, std::ios::binary);
      if (in) {
        f << in;
      }
    }
  }
}

void Darc::Read(Stream& f) {
  pEntries.clear();
  BinUtil u(f);

  Header h;
  u.Read(h.Magic);
  u.Read(h.Endianness);

  // check for both variants
  if (h.Magic != 0x63726164 && h.Magic != 0x64617263) {
    throw std::runtime_error("[ctrff] DARC: Invalid magic number!");
  }

  u.SetEndianess(h.Endianness == 0xFFFE);
  u.Read(h.HeaderSize);
  u.Read(h.Version);
  u.Read(h.FileSize);
  u.Read(h.TableOffset);
  u.Read(h.TableSize);
  u.Read(h.DataOffset);

  f.seekg(h.TableOffset, std::ios::beg);
  TableEntry first_entry;
  u.Read(first_entry.FileNameOffset);
  u.Read(first_entry.Offset);
  u.Read(first_entry.Size);

  u32 total_entries = first_entry.Size;

  std::vector<TableEntry> raw_entries(total_entries);
  f.seekg(h.TableOffset, std::ios::beg);
  for (u32 i = 0; i < total_entries; i++) {
    u.Read(raw_entries[i].FileNameOffset);
    u.Read(raw_entries[i].Offset);
    u.Read(raw_entries[i].Size);
  }

  u32 string_table_offset =
      h.TableOffset + (total_entries * sizeof(TableEntry));
  u32 string_table_size = h.DataOffset - string_table_offset;
  u32 string_count = string_table_size / 2;

  std::vector<char16_t> string_table(string_count);
  f.seekg(string_table_offset, std::ios::beg);
  for (u32 i = 0; i < string_count; i++) {
    u16 c;
    u.Read(c);
    string_table[i] = static_cast<char16_t>(c);
  }

  pEntries.resize(total_entries);
  for (u32 i = 0; i < total_entries; i++) {
    pEntries[i].IsDir = (raw_entries[i].FileNameOffset & 0x01000000) != 0;
    pEntries[i].NameOffset = raw_entries[i].FileNameOffset;
    pEntries[i].DataOffset = raw_entries[i].Offset;
    pEntries[i].DataSize = raw_entries[i].Size;

    u32 name_pos_bytes = (raw_entries[i].FileNameOffset & 0x00FFFFFF);
    pEntries[i].Name = std::u16string(&string_table[name_pos_bytes / 2]);
  }

  if (total_entries > 2) {
    pBuildPaths(2, total_entries, "");
  }

  pTableSize = h.TableSize;
  pDataOffsetBase = h.DataOffset;
  pTotalFileSize = h.FileSize;
}

void Darc::ExtractTo(const std::string& path) const {
  if (pEntries.empty()) {
    return;
  }

  if (pLoadedFilePath.empty()) {
    throw std::runtime_error(
        "[ctrff] DARC: Archive was not loaded from a file!");
  }

  pStream->reopen();

  std::filesystem::create_directories(path);

  // skip <Root> and <.>
  for (size_t i = 2; i < pEntries.size(); i++) {
    const auto& node = pEntries[i];

    std::string full_path = path + "/" + node.FsPath;

    if (node.IsDir) {
      std::filesystem::create_directories(full_path);
    } else {
      std::filesystem::create_directories(
          std::filesystem::path(full_path).parent_path());

      pStream->seekg(node.DataOffset, std::ios::beg);
      std::vector<char> buffer(node.DataSize);
      pStream->read(buffer.data(), node.DataSize);

      std::ofstream out(full_path, std::ios::binary);
      out.write(buffer.data(), node.DataSize);
      out.close();
    }
  }
}

std::vector<u8> Darc::ExtractFile(u32 idx) {
  std::vector<u8> ret;
  if (idx >= pEntries.size()) return ret;
  pStream->reopen();
  pStream->seekg(pEntries[idx].DataOffset, std::ios::beg);
  ret.resize(pEntries[idx].DataSize);
  pStream->read(reinterpret_cast<char*>(ret.data()), ret.size());
  pStream->close();
  return ret;
}
}  // namespace ctrff

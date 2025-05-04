#include <cstring>
#include <ctrff/smdh.hpp>

// magic
static const std::string smdh_magic = "SMDH";

CTRFF_API void ctrff::SMDH::Write(std::fstream &f) const {
  f.write(reinterpret_cast<const char *>(Magic), sizeof(Magic));
  f.write(reinterpret_cast<const char *>(&Version), sizeof(Version));
  f.write(reinterpret_cast<const char *>(&Reserved), sizeof(Reserved));
  for (size_t i = 0; i < 16; i++) {
    f.write(reinterpret_cast<const char *>(&Titles[i]), sizeof(Title));
  }
  f.write(reinterpret_cast<const char *>(&Settings), sizeof(Settings));
  f.write(reinterpret_cast<const char *>(&Reserved1), sizeof(Reserved1));
  f.write(reinterpret_cast<const char *>(&IconSmall), sizeof(IconSmall));
  f.write(reinterpret_cast<const char *>(&IconLarge), sizeof(IconLarge));
}

CTRFF_API void ctrff::SMDH::Read(std::fstream &f) {
  f.seekg(0, std::ios::end);
  if (f.tellg() != SMDH_Size) {
    throw std::runtime_error(
        "SMDH: File size does not match the SMDH Header size!");
  }
  f.seekg(0, std::ios::beg);
  f.read(reinterpret_cast<char *>(Magic), sizeof(Magic));
  if (std::string(Magic) != smdh_magic) {
    throw std::runtime_error("SMDH: Invalid SMDH file!");
  }
  f.read(reinterpret_cast<char *>(&Version), sizeof(Version));
  f.read(reinterpret_cast<char *>(&Reserved), sizeof(Reserved));
  for (size_t i = 0; i < 16; i++) {
    f.read(reinterpret_cast<char *>(&Titles[i]), sizeof(Title));
  }
  f.read(reinterpret_cast<char *>(&Settings), sizeof(Settings));
  f.read(reinterpret_cast<char *>(&Reserved1), sizeof(Reserved1));
  f.read(reinterpret_cast<char *>(&IconSmall), sizeof(IconSmall));
  f.read(reinterpret_cast<char *>(&IconLarge), sizeof(IconLarge));
}

CTRFF_API void ctrff::SMDH::SetIcon(const std::vector<PD::u8> &buf) {
  RGBA2RGB565(IconLarge, buf, 48, 48);
  auto small_icon = DownscaleImage(buf, 48, 48, 2);
  RGBA2RGB565(IconSmall, small_icon, 24, 24);
}

CTRFF_API std::vector<PD::u8> ctrff::SMDH::GetIcon() {
  std::vector<PD::u8> res(48 * 48 * 4);
  ctrff::RGB565toRGBA(res, IconLarge, 48, 48);
  return res;
}

CTRFF_API void ctrff::SMDH::SetShortTitle(const std::string &t, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      ctrff::String2U16(Titles[i].ShortTitle, t, 0x40);
    }
    return;
  }
  ctrff::String2U16(Titles[l].ShortTitle, t, 0x40);
}

CTRFF_API void ctrff::SMDH::SetLongTitle(const std::string &t, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      ctrff::String2U16(Titles[i].LongTitle, t, 0x80);
    }
    return;
  }
  ctrff::String2U16(Titles[l].LongTitle, t, 0x80);
}

CTRFF_API void ctrff::SMDH::SetAuthor(const std::string &t, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      ctrff::String2U16(Titles[i].Author, t, 0x40);
    }
    return;
  }
  ctrff::String2U16(Titles[l].Author, t, 0x40);
}

CTRFF_API std::string ctrff::SMDH::GetShortTitle(Language l) {
  if (l == Language_All) {
    return ctrff::U16toU8(Titles[0].ShortTitle, 0x40);
  }
  return ctrff::U16toU8(Titles[l].ShortTitle, 0x40);
}

CTRFF_API std::string ctrff::SMDH::GetLongTitle(Language l) {
  if (l == Language_All) {
    return ctrff::U16toU8(Titles[0].LongTitle, 0x80);
  }
  return ctrff::U16toU8(Titles[l].LongTitle, 0x80);
}

CTRFF_API std::string ctrff::SMDH::GetAuthor(Language l) {
  if (l == Language_All) {
    return ctrff::U16toU8(Titles[0].Author, 0x40);
  }
  return ctrff::U16toU8(Titles[l].Author, 0x40);
}

CTRFF_API ctrff::SMDH ctrff::SMDH::Default() {
  SMDH n3w; /** new */
  for (int i = 0; i < 4; i++) {
    n3w.Magic[i] = smdh_magic[i];
  }
  // Set Defaults
  n3w.Settings.MatchmakerID = 0;
  n3w.Settings.MatchmakerBitID = 0;
  n3w.Settings.EulaVersion = 0;
  n3w.Settings.OptimalBannerFrame = 0;
  n3w.Settings.StreetpassID = 0;
  for (size_t i = 0; i < 16; i++) {
    n3w.Settings.Ratings[i] = 0;
  }
  n3w.Version = 0;
  n3w.Reserved = 0;
  n3w.Reserved1 = 0;
  n3w.Settings.Flags = Flag_DEFAULT;
  n3w.Settings.RegionLock = Region_FREE;
  std::fill_n(n3w.IconSmall, 0x240, 0);
  std::fill_n(n3w.IconLarge, 0x900, 0);
  return n3w;
}
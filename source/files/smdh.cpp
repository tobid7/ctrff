#include <cstring>
#include <ctrff/files/smdh.hpp>

// magic
static const std::string smdh_magic = "SMDH";

CTRFF_API void ctrff::SMDH::Write(Stream& f) const { WriteT(f, *this); }

CTRFF_API void ctrff::SMDH::Read(Stream& f) {
  ReadT(f, *this);
  if (std::string(Magic, 4) != smdh_magic) {
    throw std::runtime_error("SMDH: Invalid SMDH file!");
  }
}

CTRFF_API void ctrff::SMDH::SetIcon(const std::vector<ctrff::u8>& buf) {
  RGBA2RGB565(IconLarge, buf, 48, 48);
  auto small_icon = DownscaleImage(buf, 48, 48, 2);
  RGBA2RGB565(IconSmall, small_icon, 24, 24);
}

CTRFF_API std::vector<ctrff::u8> ctrff::SMDH::GetIcon() {
  std::vector<ctrff::u8> res(48 * 48 * 4);
  ctrff::RGB565toRGBA(res, IconLarge, 48, 48);
  return res;
}

CTRFF_API void ctrff::SMDH::SetShortTitle(const std::string& t, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      ctrff::String2U16(Titles[i].ShortTitle, t, 0x40);
    }
    return;
  }
  ctrff::String2U16(Titles[l].ShortTitle, t, 0x40);
}

CTRFF_API void ctrff::SMDH::SetLongTitle(const std::string& t, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      ctrff::String2U16(Titles[i].LongTitle, t, 0x80);
    }
    return;
  }
  ctrff::String2U16(Titles[l].LongTitle, t, 0x80);
}

CTRFF_API void ctrff::SMDH::SetAuthor(const std::string& t, Language l) {
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

CTRFF_API ctrff::SMDH::Rating ctrff::SMDH::GetRating(Language l) {
  if (l == Language_All) {
    return (ctrff::SMDH::Rating)Settings.Ratings[0];
  }
  return (ctrff::SMDH::Rating)Settings.Ratings[l];
}

CTRFF_API void ctrff::SMDH::SetRating(Rating r, Language l) {
  if (l == Language_All) {
    for (int i = 0; i < 16; i++) {
      Settings.Ratings[i] = r;
    }
  }
  Settings.Ratings[l] = r;
}

CTRFF_API void ctrff::SMDH::SetFlags(Flag flags) { Settings.Flags = flags; }

CTRFF_API void ctrff::SMDH::EnableFlag(Flag flag) { Settings.Flags |= flag; }

CTRFF_API void ctrff::SMDH::DisableFlag(Flag flag) { Settings.Flags &= ~flag; }

CTRFF_API ctrff::SMDH::Flag ctrff::SMDH::GetFlags() {
  return (ctrff::SMDH::Flag)Settings.Flags;
}

CTRFF_API bool ctrff::SMDH::IsFlagEnabled(Flag flag) {
  return Settings.Flags & flag;
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
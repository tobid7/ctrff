#pragma once

#include <ctrff/binutil.hpp>
#include <ctrff/helper.hpp>
#include <ctrff/pd_p_api.hpp>
#include <pd.hpp>

// Basic Info
// language_slots: 16
// valid_language_slots: 12
// rating_slots: 16
// small_icon: 24
// large_icon = 48

namespace ctrff {
// SMDH Size (Note that this needs to be declared here as
// a sizeof(SMDH) will not return the expected size due to
// use of Serializable)
constexpr PD::u32 SMDH_Size = 0x36C0;
struct CTRFF_API SMDH {
  SMDH() {
    std::fill_n(Magic, PD::ArraySize(Magic), 0);
    std::fill_n(IconSmall, PD::ArraySize(IconSmall), 0);
    std::fill_n(IconLarge, PD::ArraySize(IconLarge), 0);
  }
  ~SMDH() = default;
  static SMDH Default();
  PD_SMART_CTOR(SMDH);

  enum Language {
    Language_Japanese,
    Language_English,
    Language_French,
    Language_German,
    Language_Italian,
    Language_Spanish,
    Language_Chinese_Simplified,
    Language_Korean,
    Language_Dutch,
    Language_Portuguese,
    Language_Russian,
    Language_Chinese_Traditional,
    // To Overrite Aall Languages
    // returns japanese on get funcs
    Language_All = 0x47,
  };
  enum Rating {
    Rating_CERO = 0,
    Rating_ESRB = 1,
    Rating_USK = 3,
    Rating_PEGI_GEN = 4,
    Rating_PEGI_PTR = 6,
    Rating_PEGI_BBFC = 7,
    Rating_COB = 8,
    Rating_GRB = 9,
    Rating_CGSRR = 10,
  };
  enum Region {
    Region_JAPAN = 1 << 0,
    Region_NORTH_AMERICA = 1 << 1,
    Region_EUROPE = 1 << 2,
    Region_AUSTRALIA = 1 << 3,
    Region_CHINA = 1 << 4,
    Region_KOREA = 1 << 5,
    Region_TAIWAN = 1 << 6,
    // Not a bitmask, but a value.
    Region_FREE = 0x7FFFFFFF,
  };

  enum Flag {
    Flag_VISIBLE = 1 << 0,
    Flag_AUTO_BOOT = 1 << 1,
    Flag_ALLOW_3D = 1 << 2,
    Flag_REQUIRE_EULA = 1 << 3,
    Flag_AUTO_SAVE_ON_EXIT = 1 << 4,
    Flag_USE_EXTENDED_BANNER = 1 << 5,
    Flag_RATING_REQUIED = 1 << 6,
    Flag_USE_SAVE_DATA = 1 << 7,
    Flag_RECORD_USAGE = 1 << 8,
    Flag_DISABLE_SAVE_BACKUPS = 1 << 10,
    Flag_NEW_3DS = 1 << 12,
    Flag_DEFAULT = Flag_VISIBLE | Flag_ALLOW_3D | Flag_RECORD_USAGE,
  };

  void Load(const std::string &path) {
    std::fstream f(path, std::ios::in | std::ios::binary);
    Read(f);
    f.close();
  }

  void Save(const std::string &path) {
    std::fstream f(path, std::ios::out | std::ios::binary);
    Write(f);
    f.close();
  }

  void Write(std::fstream &f) const;
  void Read(std::fstream &f);

  void SetIcon(const std::vector<PD::u8> &buf);
  std::vector<PD::u8> GetIcon();
  void SetShortTitle(const std::string &t, Language l = Language_All);
  void SetLongTitle(const std::string &t, Language l = Language_All);
  void SetAuthor(const std::string &t, Language l = Language_All);
  std::string GetShortTitle(Language l = Language_All);
  std::string GetLongTitle(Language l = Language_All);
  std::string GetAuthor(Language l = Language_All);

  struct CTRFF_API Title {
    Title() {
      std::fill_n(ShortTitle, PD::ArraySize(ShortTitle), 0);
      std::fill_n(LongTitle, PD::ArraySize(LongTitle), 0);
      std::fill_n(Author, PD::ArraySize(Author), 0);
    };

    PD::u16 ShortTitle[0x40];
    PD::u16 LongTitle[0x80];
    PD::u16 Author[0x40];
  };

  struct CTRFF_API Settings {
    Settings() { std::fill_n(Ratings, PD::ArraySize(Ratings), 0); };
    PD::u8 Ratings[16];
    PD::u32 RegionLock = 0;
    PD::u32 MatchmakerID = 0;
    PD::u64 MatchmakerBitID = 0;
    PD::u32 Flags = 0;
    PD::u16 EulaVersion = 0;
    PD::u16 Reserved = 0;
    PD::u32 OptimalBannerFrame = 0;
    PD::u32 StreetpassID = 0;
  };

  char Magic[4];
  PD::u16 Version = 0;
  PD::u16 Reserved = 0;
  Title Titles[16];
  Settings Settings;
  PD::u64 Reserved1 = 0;
  PD::u16 IconSmall[0x240];  // 24x24
  PD::u16 IconLarge[0x900];  // 48x48
};
}  // namespace ctrff
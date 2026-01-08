#include <cli-fancy.hpp>
#include <cmath>
#include <ctrff.hpp>
#include <format>
#include <fstream>
#include <iomanip>
#include <map>
#include <palladium>
#include <string>
#include <utility>

/** Import palladium stb image */
#include <pd/external/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
/** Could not use the palladium func due to only using headers */
const std::string FormatBytes(unsigned long long bytes) {
  if (bytes == 1) {
    // Only one Byte
    return std::format("{} Byte", bytes);
  } else if (bytes < 1024) {
    // less than one kilobyte
    return std::format("{} Bytes", bytes);
  } else if (bytes < 1048576) {
    // less than one megabyte
    return std::format("{:.1f} KB", (float)bytes / 1024.f);
  } else if (bytes < 1073741824) {
    // less than one gigabyte
    return std::format("{:.1f} MB", (float)bytes / 1048576.f);
  } else {
    // gigabyte
    return std::format("{:.1f} GB", (float)bytes / 1073741824.f);
  }
}

const std::map<ctrff::SMDH::Language, std::string> smdh_lang_table = {
    {ctrff::SMDH::Language_Japanese, "Japanese"},
    {ctrff::SMDH::Language_English, "English"},
    {ctrff::SMDH::Language_French, "French"},
    {ctrff::SMDH::Language_German, "German"},
    {ctrff::SMDH::Language_Italian, "Italian"},
    {ctrff::SMDH::Language_Spanish, "Spanish"},
    {ctrff::SMDH::Language_Chinese_Simplified, "Chinese Simplified"},
    {ctrff::SMDH::Language_Korean, "Korean"},
    {ctrff::SMDH::Language_Dutch, "Dutch"},
    {ctrff::SMDH::Language_Portuguese, "Portuguese"},
    {ctrff::SMDH::Language_Russian, "Russian"},
    {ctrff::SMDH::Language_Chinese_Traditional, "Chinese Traditional"},
};

const std::map<ctrff::SMDH::Rating, std::string> smdh_rating_table = {
    {ctrff::SMDH::Rating_CERO, "CERO"},
    {ctrff::SMDH::Rating_ESRB, "ESRB"},
    {ctrff::SMDH::Rating_USK, "USK"},
    {ctrff::SMDH::Rating_PEGI_GEN, "PEGI_GEN"},
    {ctrff::SMDH::Rating_PEGI_PTR, "PEGI_PTR"},
    {ctrff::SMDH::Rating_PEGI_BBFC, "PEGI_BBFC"},
    {ctrff::SMDH::Rating_COB, "COB"},
    {ctrff::SMDH::Rating_GRB, "GRB"},
    {ctrff::SMDH::Rating_CGSRR, "CGSRR"},
};

const std::map<ctrff::SMDH::Region, std::string> smdh_region_table = {
    {ctrff::SMDH::Region_JAPAN, "Japan"},
    {ctrff::SMDH::Region_NORTH_AMERICA, "North America"},
    {ctrff::SMDH::Region_EUROPE, "Europe"},
    {ctrff::SMDH::Region_AUSTRALIA, "Australia"},
    {ctrff::SMDH::Region_CHINA, "China"},
    {ctrff::SMDH::Region_KOREA, "Korea"},
    {ctrff::SMDH::Region_TAIWAN, "Taiwan"},
    {ctrff::SMDH::Region_FREE, "Free"},
};

const std::map<ctrff::SMDH::Flag, std::string> smdh_flag_table{
    {ctrff::SMDH::Flag_VISIBLE, "Visible"},
    {ctrff::SMDH::Flag_AUTO_BOOT, "Auto Boot"},
    {ctrff::SMDH::Flag_ALLOW_3D, "Allow 3D"},
    {ctrff::SMDH::Flag_REQUIRE_EULA, "Require EULA"},
    {ctrff::SMDH::Flag_AUTO_SAVE_ON_EXIT, "Auto Save on Exit"},
    {ctrff::SMDH::Flag_USE_EXTENDED_BANNER, "Use Extended Banner"},
    {ctrff::SMDH::Flag_RATING_REQUIED, "Rating required"},
    {ctrff::SMDH::Flag_USE_SAVE_DATA, "Use Save Data"},
    {ctrff::SMDH::Flag_RECORD_USAGE, "Record usage"},
    {ctrff::SMDH::Flag_DISABLE_SAVE_BACKUPS, "Disable Save backups"},
    {ctrff::SMDH::Flag_NEW_3DS, "New 3DS"},
};

const std::map<std::string, ctrff::SMDH::Language> smdh_lang_stropts = {
    {"", ctrff::SMDH::Language_All},
    {"jp", ctrff::SMDH::Language_Japanese},
    {"en", ctrff::SMDH::Language_English},
    {"fr", ctrff::SMDH::Language_French},
    {"de", ctrff::SMDH::Language_German},
    {"it", ctrff::SMDH::Language_Italian},
    {"es", ctrff::SMDH::Language_Spanish},
    {"cs", ctrff::SMDH::Language_Chinese_Simplified},
    {"ko", ctrff::SMDH::Language_Korean},
    {"du", ctrff::SMDH::Language_Dutch},
    {"po", ctrff::SMDH::Language_Portuguese},
    {"ru", ctrff::SMDH::Language_Russian},
    {"ct", ctrff::SMDH::Language_Chinese_Traditional},
};

void MakeSMDH(const cf7::command::ArgumentList &data) {
  std::string l = cf7::command::GetArg(data, "long");
  std::string s = cf7::command::GetArg(data, "short");
  std::string a = cf7::command::GetArg(data, "author");
  std::string i = cf7::command::GetArg(data, "icon");
  std::string o = cf7::command::GetArg(data, "output");
  if (l.empty() || s.empty() || a.empty() || i.empty() || o.empty()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("One or more Arguments are not set!",
                       cf7::col(130, 0, 0)),
    });
    return;
  }
  ctrff::SMDH smdh = ctrff::SMDH::Default();
  smdh.SetLongTitle(l);
  smdh.SetShortTitle(s);
  smdh.SetAuthor(a);
  std::vector<unsigned char> img;
  int w, h, c;
  ctrff::u8 *buf = stbi_load(i.c_str(), &w, &h, &c, 4);
  if (buf == nullptr) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Can't open icon File!", cf7::col(130, 0, 0)),
    });
    return;
  }
  if (w != 48 || h != 48) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Icon is not 48x48 pixels!", cf7::col(130, 0, 0)),
    });
    return;
  }
  img.assign(buf, buf + (w * h * 4));
  smdh.SetIcon(img);
  std::fstream f(o, std::ios::out | std::ios::binary);
  smdh.Write(f);
  cf7::PrintFancy({
      std::make_pair("File Generated", cf7::col(0, 190, 0)),
  });
}

void ReadSMDH(const cf7::command::ArgumentList &data) {
  ctrff::SMDH smdh;
  if (cf7::command::GetArg(data, "input").empty()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Input Argument not found!", cf7::col(130, 0, 0)),
    });
    return;
  }
  try {
    smdh.Load(cf7::command::GetArg(data, "input"));
  } catch (const std::exception &e) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair(e.what(), cf7::col(130, 0, 0)),
    });
    return;
  }
  cf7::PrintFancy({
      std::make_pair("CTRFF", cf7::col(220, 160, 0)),
      std::make_pair("SMDH-Parser", cf7::col(240, 200, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Language", cf7::col(220, 160, 0)),
      std::make_pair("Short", cf7::col(240, 200, 0)),
      std::make_pair("Long", cf7::col(255, 230, 0)),
      std::make_pair("Author", cf7::col(255, 255, 0)),
  });
  for (auto &e : smdh_lang_table) {
    cf7::PrintFancy({
        std::make_pair("  " + e.second, cf7::col(220, 160, 0)),
        std::make_pair(smdh.GetShortTitle(e.first), cf7::col(240, 200, 0)),
        std::make_pair(smdh.GetLongTitle(e.first), cf7::col(255, 230, 0)),
        std::make_pair(smdh.GetAuthor(e.first), cf7::col(255, 255, 0)),
    });
  }
  cf7::PrintFancy({
      std::make_pair("Version", cf7::col(220, 160, 0)),
      std::make_pair(std::to_string(smdh.Version), cf7::col(240, 200, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Ratings", cf7::col(220, 160, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Flags", cf7::col(220, 160, 0)),
  });
  for (auto &e : smdh_flag_table) {
    cf7::PrintFancy({
        std::make_pair("  " + e.second, cf7::col(220, 160, 0)),
        std::make_pair(
            std::string((smdh.Settings.Flags & e.first) ? "true" : "false"),
            (smdh.Settings.Flags & e.first) ? cf7::col(0, 190, 0)
                                            : cf7::col(190, 0, 0)),
    });
  }
  if (smdh.Settings.RegionLock == ctrff::SMDH::Region_FREE) {
    cf7::PrintFancy({
        std::make_pair("Region", cf7::col(220, 160, 0)),
        std::make_pair("Free", cf7::col(240, 200, 0)),
    });
  } else {
    for (auto &e : smdh_region_table) {
      cf7::PrintFancy({
          std::make_pair("Regions", cf7::col(220, 160, 0)),
      });
      cf7::PrintFancy({
          std::make_pair(e.second, cf7::col(220, 160, 0)),
          std::make_pair(
              std::string(smdh.Settings.RegionLock & e.first ? "true"
                                                             : "false"),
              smdh.Settings.RegionLock & e.first ? cf7::col(0, 190, 0)
                                                 : cf7::col(190, 0, 0)),
      });
    }
  }
  cf7::PrintFancy({
      std::make_pair("MatchmakerID", cf7::col(220, 160, 0)),
      std::make_pair(std::to_string(smdh.Settings.MatchmakerID),
                     cf7::col(240, 200, 0)),
  });
  std::string icon_path = cf7::command::GetArg(data, "extract-icon");
  if (!icon_path.empty()) {
    auto icon = smdh.GetIcon();
    stbi_write_png(icon_path.c_str(), 48, 48, 4, icon.data(), 48 * 4);
    /* if (res) {
       // Error
     } else {*/
    cf7::PrintFancy({
        std::make_pair("Export", cf7::col(0, 190, 0)),
        std::make_pair(icon_path, cf7::col(0, 130, 0)),
        std::make_pair("Success", cf7::col(0, 80, 0)),
    });
    // }
  }
}

void Read3DSX(const cf7::command::ArgumentList &data) {
  ctrff::_3dsx _3dsx;
  if (cf7::command::GetArg(data, "input").empty()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Input Argument not found!", cf7::col(130, 0, 0)),
    });
    return;
  }
  /*if (!*/ _3dsx.Load(cf7::command::GetArg(data, "input")); /*) {
      cf7::PrintFancy({
          std::make_pair("Error", cf7::col(190, 0, 0)),
          std::make_pair("Could not load 3dsx!", cf7::col(130, 0, 0)),
      });
      return;
    }*/
  if (!_3dsx.HasMeta()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("3dsx has no meta!", cf7::col(130, 0, 0)),
    });
    return;
  }
  ctrff::SMDH smdh = _3dsx.Meta;
  cf7::PrintFancy({
      std::make_pair("CTRFF", cf7::col(220, 160, 0)),
      std::make_pair("SMDH-Parser", cf7::col(240, 200, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Language", cf7::col(220, 160, 0)),
      std::make_pair("Short", cf7::col(240, 200, 0)),
      std::make_pair("Long", cf7::col(255, 230, 0)),
      std::make_pair("Author", cf7::col(255, 255, 0)),
  });
  for (auto &e : smdh_lang_table) {
    cf7::PrintFancy({
        std::make_pair("  " + e.second, cf7::col(220, 160, 0)),
        std::make_pair(smdh.GetShortTitle(e.first), cf7::col(240, 200, 0)),
        std::make_pair(smdh.GetLongTitle(e.first), cf7::col(255, 230, 0)),
        std::make_pair(smdh.GetAuthor(e.first), cf7::col(255, 255, 0)),
    });
  }
  cf7::PrintFancy({
      std::make_pair("Version", cf7::col(220, 160, 0)),
      std::make_pair(std::to_string(smdh.Version), cf7::col(240, 200, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Ratings", cf7::col(220, 160, 0)),
  });
  cf7::PrintFancy({
      std::make_pair("Flags", cf7::col(220, 160, 0)),
  });
  for (auto &e : smdh_flag_table) {
    cf7::PrintFancy({
        std::make_pair("  " + e.second, cf7::col(220, 160, 0)),
        std::make_pair(
            std::string((smdh.Settings.Flags & e.first) ? "true" : "false"),
            (smdh.Settings.Flags & e.first) ? cf7::col(0, 190, 0)
                                            : cf7::col(190, 0, 0)),
    });
  }
  if (smdh.Settings.RegionLock == ctrff::SMDH::Region_FREE) {
    cf7::PrintFancy({
        std::make_pair("Region", cf7::col(220, 160, 0)),
        std::make_pair("Free", cf7::col(240, 200, 0)),
    });
  } else {
    for (auto &e : smdh_region_table) {
      cf7::PrintFancy({
          std::make_pair("Regions", cf7::col(220, 160, 0)),
      });
      cf7::PrintFancy({
          std::make_pair(e.second, cf7::col(220, 160, 0)),
          std::make_pair(
              std::string(smdh.Settings.RegionLock & e.first ? "true"
                                                             : "false"),
              smdh.Settings.RegionLock & e.first ? cf7::col(0, 190, 0)
                                                 : cf7::col(190, 0, 0)),
      });
    }
  }
  cf7::PrintFancy({
      std::make_pair("MatchmakerID", cf7::col(220, 160, 0)),
      std::make_pair(std::to_string(smdh.Settings.MatchmakerID),
                     cf7::col(240, 200, 0)),
  });
  std::string icon_path = cf7::command::GetArg(data, "extract-icon");
  if (!icon_path.empty()) {
    auto icon = smdh.GetIcon();
    stbi_write_png(icon_path.c_str(), 48, 48, 4, icon.data(), 48 * 4);
    /* if (res) {
       // Error
     } else {*/
    cf7::PrintFancy({
        std::make_pair("Export", cf7::col(0, 190, 0)),
        std::make_pair(icon_path, cf7::col(0, 130, 0)),
        std::make_pair("Success", cf7::col(0, 80, 0)),
    });
    // }
  }
}

void Hex(const cf7::command::ArgumentList &data) {
  if (cf7::command::GetArg(data, "input").empty()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Input Argument not found!", cf7::col(130, 0, 0)),
    });
    return;
  }
  std::ifstream fr(data[0].second, std::ios::binary);
  if (!fr.is_open()) {
    cf7::PrintFancy({
        std::make_pair("Error", cf7::col(190, 0, 0)),
        std::make_pair("Failed to open Input File!", cf7::col(130, 0, 0)),
    });
    return;
  }
  std::vector<unsigned char> buf(std::istreambuf_iterator<char>(fr), {});
  fr.close();
  cf7::PrintFancy({
      std::make_pair("CTRFF HEXDUMP", cf7::col(255, 165, 0)),
      std::make_pair(data[0].second, cf7::col(255, 210, 0)),
      std::make_pair(FormatBytes(buf.size()), cf7::col(255, 255, 0)),
  });
  std::cout << "+----------+-------------------------------------------------+-"
               "-----------------+"
            << std::endl;
  std::cout << "| Adress   | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F | "
               "ASCII            |"
            << std::endl;
  std::cout << "+----------+-------------------------------------------------+-"
               "-----------------+"
            << std::endl;
  std::cout << "| " << std::hex << std::setw(8) << std::setfill('0')
            << (unsigned int)0 << " | ";
  std::vector<char> ascii;
  int lidx = 0;
  for (size_t i = 0; i < buf.size(); i++) {
    lidx++;
    ascii.push_back((char)buf[i]);
    std::cout << std::hex << std::setw(2) << std::setfill('0') << std::uppercase
              << (int)buf[i] << " ";
    if (((i + 1) % 0x10) == 0 && i > 14) {
      std::cout << "| ";
      for (auto &it : ascii)
        std::cout << std::dec
                  << (((int)it >= 32) && ((int)it <= 126) ? it : '.');
      ascii.clear();
      std::cout << " |" << std::endl;
      std::cout << "| " << std::hex << std::setw(8) << std::setfill('0')
                << (unsigned int)i + 1 << " | ";
      lidx = 0;
    }
  }
  if (lidx < 16) {
    for (int i = lidx; i < 16; i++) {
      ascii.push_back('X');
      std::cout << "XX ";
      if (i == 15) {
        std::cout << "| ";
        for (auto &it : ascii)
          std::cout << std::dec
                    << (((int)it >= 32) && ((int)it <= 126) ? it : '.');
        ascii.clear();
        std::cout << " |" << std::endl;
      }
    }
    std::cout
        << "+----------+-------------------------------------------------+-"
           "-----------------+"
        << std::endl;
  }
}

void LZ11Compress(const cf7::command::ArgumentList &data) {
  std::string i = cf7::command::GetArg(data, "input");
  std::string o = cf7::command::GetArg(data, "output");
  std::fstream in(i, std::ios::in | std::ios::binary);
  in.seekg(0, std::ios::end);
  size_t s = in.tellg();
  in.seekg(0, std::ios::beg);
  std::vector<unsigned char> buf(s);
  in.read(reinterpret_cast<char *>(buf.data()), s);
  in.close();
  cf7::PrintFancy({
      std::make_pair("Input", cf7::col(255, 165, 0)),
      std::make_pair(i, cf7::col(255, 210, 0)),
      std::make_pair(FormatBytes(buf.size()), cf7::col(255, 255, 0)),
  });
  std::vector<ctrff::u8> res;
  if (buf[0] == 0x11) {
    res = ctrff::LZ11::Decompress(buf);
  } else {
    res = ctrff::LZ11::Compress(buf);
  }
  cf7::PrintFancy({
      std::make_pair("Output", cf7::col(255, 165, 0)),
      std::make_pair(o, cf7::col(255, 210, 0)),
      std::make_pair(FormatBytes(res.size()), cf7::col(255, 255, 0)),
  });
  std::fstream out(o, std::ios::out | std::ios::binary);
  out.write(reinterpret_cast<const char *>(res.data()), res.size());
  out.close();
}

void BCLIMMaker(const cf7::command::ArgumentList &data) {
  std::string i = cf7::command::GetArg(data, "input");
  std::string o = cf7::command::GetArg(data, "output");
  std::string f = cf7::command::GetArg(data, "format");
  if (i.empty() || o.empty()) {
    std::cout << "[ctrff] BCLIM: Error, no input or output" << std::endl;
    return;
  }
  PD::Image::Ref img = PD::Image::New();
  img->Load(i);
  if (img->GetBuffer().empty()) {
    std::cout << "[ctrff] BCLIM: Failed to load image " + i << std::endl;
    return;
  }
  if (!PD::BitUtil::IsSingleBit(img->Width()) ||
      !PD::BitUtil::IsSingleBit(img->Height())) {
    std::cout << "[ctrff] BCLIM: Image with and height must be a power of 8!";
    return;
  }
  img->Convert(img, img->RGBA);
  std::vector<ctrff::u8> res;
  ctrff::Pica::Color fmt = ctrff::Pica::A8;
  if (f == "a4") {
    fmt = ctrff::Pica::A4;
  } else if (f == "l4") {
    fmt = ctrff::Pica::L4;
  } else if (f == "a8") {
    fmt = ctrff::Pica::A8;
  } else if (f == "l8") {
    fmt = ctrff::Pica::L8;
  } else if (f == "la4") {
    fmt = ctrff::Pica::LA4;
  } else if (f == "la8") {
    fmt = ctrff::Pica::LA8;
  } else if (f == "rgb565") {
    fmt = ctrff::Pica::RGB565;
  } else if (f == "rgb888") {
    fmt = ctrff::Pica::RGB888;
  } else if (f == "rgba4444") {
    fmt = ctrff::Pica::RGBA4444;
  } else if (f == "rgba5551") {
    fmt = ctrff::Pica::RGBA5551;
  } else if (f == "rgba8888") {
    fmt = ctrff::Pica::RGBA8888;
  }
  ctrff::Pica::EncodeImage(res, img->GetBuffer(), img->Width(), img->Height(),
                           fmt);

  ctrff::BCLIM file;
  ctrff::BCLIM::Format _fmt = (ctrff::BCLIM::Format)fmt;
  file.CreateByImage(res, img->Width(), img->Height(), _fmt);
  file.Save(o);
  std::cout << "File " + o + " created" << std::endl;
}

int main(int argc, char *argv[]) {
  cf7::fancy_print = false;
  cf7::colors_supported = false;
  cf7::arg_mgr mgr(argc, argv);
  mgr.SetAppInfo("ctrff", "1.0.0");
  auto makesmdh_cmd = cf7::command("makesmdh", "Create a SMDH File");
  makesmdh_cmd.AddSubEntry(
      cf7::command::sub("i", "icon", "Icon file path (48x48)", true));
  makesmdh_cmd.AddSubEntry(
      cf7::command::sub("o", "output", "Output smdh path", true));
  for (auto &it : smdh_lang_stropts) {
    if (it.first.empty()) {
      makesmdh_cmd.AddSubEntry(cf7::command::sub(
          "s", "short", "Short Title (All Languages)", false));
      makesmdh_cmd.AddSubEntry(
          cf7::command::sub("l", "long", "Long Title (All Languages)", false));
      makesmdh_cmd.AddSubEntry(
          cf7::command::sub("a", "author", "Author (All Languages)", false));
    } else {
      makesmdh_cmd.AddSubEntry(cf7::command::sub(
          "s-" + it.first, "short-" + it.first,
          "Short Title " + smdh_lang_table.at(it.second), false));
      makesmdh_cmd.AddSubEntry(cf7::command::sub(
          "l-" + it.first, "long-" + it.first,
          "Long Title " + smdh_lang_table.at(it.second), false));
      makesmdh_cmd.AddSubEntry(
          cf7::command::sub("a-" + it.first, "author-" + it.first,
                            "Author " + smdh_lang_table.at(it.second), false));
    }
  }
  makesmdh_cmd.SetFunction(MakeSMDH);
  mgr.AddCommand(makesmdh_cmd);
  mgr.AddCommand(
      cf7::command("readsmdh", "Reads a smdh file and can extract icon")
          .AddSubEntry(
              cf7::command::sub("i", "input", "Input smdh path!", true))
          .AddSubEntry(cf7::command::sub("e", "extract-icon",
                                         "Path to Extract Icon to", false))
          .SetFunction(ReadSMDH));
  mgr.AddCommand(cf7::command("read3dsx", "Read Data of a 3dsx file")
                     .AddSubEntry(cf7::command::sub("i", "input",
                                                    "Input 3dsx path!", true))
                     .AddSubEntry(cf7::command::sub("e", "extract-icon",
                                                    "Output icon path!", false))
                     .SetFunction(Read3DSX));
  mgr.AddCommand(
      cf7::command("lz11", "Compress/Decompress a file with LZ11")
          .AddSubEntry(cf7::command::sub("i", "input", "Input file path", true))
          .AddSubEntry(
              cf7::command::sub("o", "output", "Output file path", true))
          .SetFunction(LZ11Compress));
  mgr.AddCommand(
      cf7::command("hex", "Show Hex view of a File")
          .AddSubEntry(cf7::command::sub("i", "input", "Input File path", true))
          .SetFunction(Hex));
  mgr.AddCommand(
      cf7::command("makebclim", "Create CTR Layout Image")
          .AddSubEntry(cf7::command::sub("i", "input", "Input png|bmp", true))
          .AddSubEntry(cf7::command::sub("o", "output",
                                         "Output path of .bclim file", true))
          .AddSubEntry(cf7::command::sub(
              "f", "format",
              "Image format "
              "rgba8888|rgb888|rgba4444|rgba5551|rgb565|a8|l8|a4|l4|la4|la8",
              false))
          .SetFunction(BCLIMMaker));
  mgr.Execute();
  return 0;
}
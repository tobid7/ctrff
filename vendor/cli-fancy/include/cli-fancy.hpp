#pragma once

/*
MIT License

Copyright (c) 2024 René Amthor (tobid7)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

// CLI FANCY
namespace cf7 {
static bool fancy_print = true;
static bool colors_supported = true;
namespace sym {
const std::string arrow = "";
}
class col {
 public:
  col() { m_isres = true; }
  col(bool fg, unsigned char r, unsigned char g, unsigned char b) {
    m_isres = false;
    m_r = r;
    m_g = g;
    m_b = b;
    m_fg = fg;
  }
  col(unsigned char r, unsigned char g, unsigned char b) {
    m_isres = false;
    m_r = r;
    m_g = g;
    m_b = b;
    // Default Fancy Api uses GetAs
    m_fg = false;
  }
  ~col() = default;

  std::string Get() const { return GetAs(m_fg); }

  std::string GetAs(bool fg) const {
    if (m_isres) {
      return "\033[00m";
    }
    std::string col = "\033[";
    col += fg ? "38" : "48";
    col += ";2;";
    col += std::to_string(m_r) + ";";
    col += std::to_string(m_g) + ";";
    col += std::to_string(m_b) + "m";
    return col;
  }

  operator std::string() const { return Get(); }
  operator const char *() const {
    /// CREATE DYNAMIC BUFFER TO ALWAYS RETURN STABLE CONST CHAR
    static char buf[32];
    strcpy(buf, Get().c_str());
    buf[sizeof(buf) - 1] = '\0';
    return buf;
  }

 private:
  bool m_isres = true;
  bool m_fg = false;
  unsigned char m_r = 0;
  unsigned char m_g = 0;
  unsigned char m_b = 0;
};
void PrintFancy(const std::vector<std::pair<std::string, col>> &e) {
  if (fancy_print && colors_supported) {
    std::cout << col();
    for (int i = 0; i < e.size(); i++) {
      std::cout << e[i].second.GetAs(false) << col(true, 255, 255, 255);
      std::cout << " " << e[i].first << " ";
      if (i == e.size() - 1) {
        std::cout << col() << e[i].second.GetAs(true) << sym::arrow;
      } else {
        std::cout << e[i + 1].second.GetAs(false) << e[i].second.GetAs(true)
                  << sym::arrow;
      }
    }
    std::cout << col() << std::endl;
  } else {
    for (int i = 0; i < e.size(); i++) {
      if (colors_supported) {
        std::cout << e[i].second.GetAs(true);
      }
      std::cout << e[i].first;
      if (i == e.size() - 1) {
        if (colors_supported) {
          std::cout << col();
        }
      } else {
        if (colors_supported) {
          std::cout << col();
        }
        std::cout << " -> ";
      }
    }
    std::cout << std::endl;
  }
}
class command {
 public:
  using ArgumentList = std::vector<std::pair<std::string, std::string>>;
  using Function = std::function<void(const ArgumentList &)>;
  class sub {
   public:
    sub(const std::string &_short, const std::string &_long,
        const std::string desc, bool req) {
      m_isrequired = req;
      m_long = _long;
      m_short = _short;
      m_desc = desc;
    }
    ~sub() = default;

    bool IsRequired() const { return m_isrequired; }
    const std::string &GetShort() const { return m_short; }
    const std::string &GetLong() const { return m_long; }
    const std::string &GetDesc() const { return m_desc; }

   private:
    bool m_isrequired;
    std::string m_long;
    std::string m_short;
    std::string m_desc;
  };
  command(std::string base, std::string desc) {
    m_name = base;
    m_desc = desc;
  }
  ~command() = default;

  command &AddSubEntry(const sub &sub) {
    m_sub.push_back(sub);
    return *this;
  }

  command &SetFunction(Function fun) {
    m_fun = fun;
    return *this;
  }

  Function Func() const { return m_fun; }

  static std::string GetArg(ArgumentList list, std::string arg) {
    for (const auto &it : list) {
      if (it.first == arg) {
        return it.second;
      }
    }
    return "";
  }

  const std::string &GetName() const { return m_name; }
  const std::string &GetDesc() const { return m_desc; }
  const std::vector<sub> &GetArgs() const { return m_sub; }

 private:
  std::string m_name;
  std::string m_desc;
  std::vector<sub> m_sub;
  Function m_fun;
};
class arg_mgr {
 public:
  arg_mgr() {}
  arg_mgr(char **args, int argc) { Parse(args, argc); }
  arg_mgr(int argc, char **args) { Parse(args, argc); }
  ~arg_mgr() = default;

  void SetAppInfo(const std::string &name, const std::string &ver) {
    app_name = name;
    app_version = ver;
  }

  bool FindShort(const std::vector<std::string> &l, std::string w) {
    auto tf = "-" + w;
    return (std::find(l.begin(), l.end(), tf) != l.end());
  }

  std::string GetArg(std::string w, std::string def = "") {
    if (!FindShort(m_args, w)) return def;
    for (size_t i = 0; i < m_args.size() - 1; i++) {
      if (m_args[i] == std::string("-" + w)) {
        return m_args[i + 1];
      }
    }
    return def;
  }
  void Parse(char **args, int argc) {
    for (int i = 0; i < argc; i++) {
      m_args.push_back(args[i]);
    }
  }

  void PrintHelp(const std::string &cmd = "") {
    if (!app_name.empty() && !app_version.empty()) {
      PrintFancy({
          std::make_pair(app_name, col(20, 20, 20)),
          std::make_pair(app_version, col(50, 50, 50)),
      });
    }
    PrintFancy({
        std::make_pair("Build", col(40, 40, 40)),
        std::make_pair(std::string(__TIMESTAMP__), col(70, 70, 70)),
    });
    if (!cmd.empty()) {
      for (const auto &c : m_commands) {
        if (c.GetName() == cmd) {
          PrintFancy({std::make_pair(c.GetName(), col(10, 10, 10)),
                      std::make_pair("Help", col(30, 30, 30))});
          for (const auto &it : c.GetArgs()) {
            if (!it.GetShort().empty()) {
              PrintFancy({
                  std::make_pair("--" + it.GetLong(), col(30, 30, 30)),
                  std::make_pair("-" + it.GetShort(), col(50, 50, 50)),
                  std::make_pair(it.GetDesc(), col(70, 70, 70)),
                  std::make_pair(it.IsRequired() ? "true" : "false",
                                 col(90, 90, 90)),
              });
            } else {
              PrintFancy({
                  std::make_pair("--" + it.GetLong(), col(30, 30, 30)),
                  std::make_pair(it.GetDesc(), col(50, 50, 50)),
                  std::make_pair(it.IsRequired() ? "true" : "false",
                                 col(70, 70, 70)),
              });
            }
          }
          return;
        }
      }
    }
    PrintFancy({std::make_pair("Commands", col(30, 30, 30))});
    for (command c : m_commands) {
      PrintFancy({
          std::make_pair(c.GetName(), col(40, 40, 40)),
          std::make_pair(c.GetDesc(), col(60, 60, 60)),
      });
    }
  }

  void Execute() {
    command::ArgumentList arglist;
    if (m_args.size() > 1) {
      for (const auto &c : m_commands) {
        if (c.GetName() == m_args[1]) {
          for (const auto &j : c.GetArgs()) {
            bool ishort = FindShort(m_args, j.GetShort());
            bool ilong = FindShort(m_args, "-" + j.GetLong());
            if (!ishort && !ilong) {
              if (j.IsRequired()) {
                PrintHelp(c.GetName());
                return;
              }
            } else if (ishort && ilong) {
              PrintHelp(c.GetName());
              return;
            } else {
              arglist.push_back(std::make_pair(
                  j.GetLong(),
                  GetArg((ishort ? j.GetShort() : ("-" + j.GetLong())))));
            }
          }
          c.Func()(arglist);
          return;
        }
      }
    }
    PrintHelp("");
  }

  void AddCommand(command cmd) { m_commands.push_back(cmd); }

 private:
  std::string app_name;
  std::string app_version;
  std::vector<std::string> m_args;
  std::vector<command> m_commands;
};
}  // namespace cf7
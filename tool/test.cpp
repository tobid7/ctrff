#include <ctrff.hpp>
#include <iomanip>

template <typename T>
void Result(const std::string& name, T a, T b) {
  std::cout << "Test: " << typeid(a).name() << " > ";
  std::cout << "0x" << std::hex << std::setw(sizeof(a) * 2) << std::setfill('0')
            << a;
  std::cout << std::dec << " - ";
  std::cout << "0x" << std::hex << std::setw(sizeof(a) * 2) << std::setfill('0')
            << b;
  std::cout << std::dec << std::endl;
  std::cout << "Test " << name << " " << (a == b ? "passed" : "failed") << "!"
            << std::endl;
}

void BinTest(bool be) {
  std::fstream s("test.bin", std::ios::out | std::ios::binary);
  ctrff::u8 t8 = 0xdf;
  ctrff::u16 t16 = 0x4564;
  ctrff::u32 t32 = 0x58464743;
  ctrff::u64 t64 = 1234567890123456789ULL;
  ctrff::BinUtil u(s, be);
  u.Write(t8);
  u.Write(t16);
  u.Write(t32);
  u.Write(t64);
  s.close();
  ctrff::u8 r8 = 0;
  ctrff::u16 r16 = 0;
  ctrff::u32 r32 = 0;
  ctrff::u64 r64 = 0;
  s.open("test.bin", std::ios::in | std::ios::binary);
  u.Read(r8);
  u.Read(r16);
  u.Read(r32);
  u.Read(r64);
  Result("u8 " + std::string(be ? "big" : "little"), r8, t8);
  Result("u16 " + std::string(be ? "big" : "little"), r16, t16);
  Result("u32 " + std::string(be ? "big" : "little"), r32, t32);
  Result("u64 " + std::string(be ? "big" : "little"), r64, t64);
}

int main() {
  BinTest(false);
  BinTest(true);
  return 0;
}
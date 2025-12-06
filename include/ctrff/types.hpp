#pragma once

/*
MIT License

Copyright (c) 2024 - 2025 tobid7

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

#ifdef _WIN32  // Windows (MSVC Tested)
#ifdef CTRFF_BUILD_SHARED
#define CTRFF_API __declspec(dllexport)
#else
#define CTRFF_API __declspec(dllimport)
#endif
#elif defined(__APPLE__)  // macOS (untested yet)
#ifdef CTRFF_BUILD_SHARED
#define CTRFF_API __attribute__((visibility("default")))
#else
#define CTRFF_API
#endif
#elif defined(__linux__)  // Linux (untested yet)
#ifdef CTRFF_BUILD_SHARED
#define CTRFF_API __attribute__((visibility("default")))
#else
#define CTRFF_API
#endif
#elif defined(__3DS__)  // 3ds Specific
// Only Static supported
#define CTRFF_API
#else
#define CTRFF_API
#endif

#include <cstddef>

namespace ctrff {
using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;

/**
 * Function to get Arraysize for any type using modern c++ to
 * get the size at compiletime instead of runtime
 * @note this function only works for Arrays declared as
 * type arr[size] and not for pointer references.
 * This function will precalculate the size at compile time
 * while keeping the code clean to not hardcode arraysizes
 * into functions like std::fill_n
 */
template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N]) noexcept {
  return N;
}
}  // namespace ctrff
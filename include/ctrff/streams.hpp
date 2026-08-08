#pragma once

#include <ctrff/types.hpp>
#include <fstream>
#include <string>

namespace ctrff {
class Stream {
 public:
  Stream() {}
  virtual ~Stream() {}

  virtual void close() = 0;
  virtual void read(char* ptr, size_t bytes) = 0;
  virtual void write(const char* ptr, size_t bytes) = 0;
  virtual void seekg(size_t off, std::ios::seekdir dir) = 0;
  virtual void seekg(size_t off) = 0;
  virtual size_t tellg() = 0;
  virtual size_t tellp() = 0;
  virtual bool is_open() = 0;
  virtual void reopen() = 0;

  size_t size() const { return pSize; }

  Stream& operator<<(Stream& in) {
    size_t r = in.size() - in.tellg();
    constexpr size_t BUFFER_SIZE = 8192;  // 8kb
    char buffer[BUFFER_SIZE];

    while (r > 0) {
      size_t to_read = std::min(r, BUFFER_SIZE);
      in.read(buffer, to_read);
      this->write(buffer, to_read);
      r -= to_read;
    }
    return *this;
  }

  Stream& operator<<(std::istream& in) {
    constexpr size_t BUFFER_SIZE = 8192;  // 8kb
    char buffer[BUFFER_SIZE];

    while (in.read(buffer, BUFFER_SIZE)) {
      this->write(buffer, in.gcount());
    }
    if (in.gcount() > 0) {
      this->write(buffer, in.gcount());
    }
    return *this;
  }

 protected:
  void SetSize(size_t bytes) { pSize = bytes; }

 private:
  size_t pSize = 0;
};

class MemoryStream : public Stream {
 public:
  MemoryStream(std::vector<u8>& mem) : ref(&mem), cref(&mem) {
    SetSize(mem.size());
  }
  MemoryStream(const std::vector<u8>& mem) : ref(nullptr), cref(&mem) {
    SetSize(mem.size());
  }
  ~MemoryStream() {}

  bool is_open() override { return true; }
  void close() override {}
  void seekg(size_t off, std::ios::seekdir dir) override {
    if (dir == std::ios::beg) {
      pPosR = off;
    } else if (dir == std::ios::cur) {
      pPosR += off;
    } else if (dir == std::ios::end) {
      pPosR = size() - off - 1;
    }
  }
  void seekg(size_t off) override { pPosR = off; }
  size_t tellg() override { return pPosR; }
  size_t tellp() override { return pPosP; }
  void read(char* ptr, size_t bytes) override {
    if ((pPosR + bytes) > size())
      throw std::runtime_error(
          "[ctrff] MemoryStream::read: out of range!\n" +
          std::format("R: {}\nB: {}\nS: {}", pPosR, bytes, size()));
    std::memcpy(ptr, &(*cref)[pPosR], bytes);
    pPosR += bytes;
  }

  void write(const char* ptr, size_t bytes) override {
    if (bytes == 0) return;
    if (pPosP + bytes > ref->size()) {
      ref->resize(pPosP + bytes);
      SetSize(ref->size());
    }

    std::memcpy(&(*ref)[pPosP], ptr, bytes);
    pPosP += bytes;
  }

  void reopen() override {
    pPosP = 0;
    pPosR = 0;
  }

 private:
  size_t pPosR = 0;
  size_t pPosP = 0;
  std::vector<u8>* ref = nullptr;
  const std::vector<u8>* cref = nullptr;
};

class FileStream : public Stream {
 public:
  FileStream(const std::string& path, std::ios::openmode mode) {
    open(path, mode);
  }
  FileStream() {}
  ~FileStream() { close(); }

  void open(const std::string& path, std::ios::openmode mode) {
    this->path = path;
    this->mode = mode;
    file.open(path, mode);
    file.seekg(0, std::ios::end);
    SetSize(file.tellg());
    file.seekg(0, std::ios::beg);
  }
  void close() override {
    if (file.is_open()) file.close();
  }
  void seekg(size_t off, std::ios::seekdir dir) override {
    file.seekg(off, dir);
  }
  void seekg(size_t off) override { file.seekg(off); }
  void read(char* ptr, size_t bytes) override { file.read(ptr, bytes); }
  void write(const char* ptr, size_t bytes) override { file.write(ptr, bytes); }
  size_t tellg() override { return file.tellg(); }
  size_t tellp() override { return file.tellp(); }
  bool is_open() override { return file.is_open(); }
  void reopen() override { open(path, mode); }

 private:
  std::fstream file;
  std::string path;
  std::ios::openmode mode;
};
}  // namespace ctrff
#pragma once

#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace OpenABL {

static inline bool directoryExists(const std::string &name) {
  struct stat info;
  return stat(name.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

static inline bool createDirectory(const std::string &name) {
  if (directoryExists(name)) {
    return true;
  }

  // TODO Windows?
  return mkdir(name.c_str(), 0755) == 0;
}

static inline void writeToFile(const std::string &name, const std::string &contents) {
  std::ofstream f(name);
  f << contents;
}

static inline void copyFile(const std::string &from, const std::string &to) {
  std::ifstream src(from);
  std::ofstream dst(to);
  dst << src.rdbuf();
}

static inline void makeFileExecutable(const std::string &name) {
  // TODO Windows?
  chmod(name.c_str(), 0755);
}

}

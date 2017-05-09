#include "FileUtil.hpp"
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace OpenABL {

bool directoryExists(const std::string &name) {
  struct stat info;
  return stat(name.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

bool createDirectory(const std::string &name) {
  if (directoryExists(name)) {
    return true;
  }

#ifdef _WIN32
  return CreateDirectory(name.c_str(), NULL) != 0;
#else
  return mkdir(name.c_str(), 0755) == 0;
#endif
}

void writeToFile(const std::string &name, const std::string &contents) {
  std::ofstream f(name);
  f << contents;
}

void copyFile(const std::string &from, const std::string &to) {
  std::ifstream src(from);
  std::ofstream dst(to);
  dst << src.rdbuf();
}

void makeFileExecutable(const std::string &name) {
#ifndef _WIN32
  chmod(name.c_str(), 0755);
#endif
}

}

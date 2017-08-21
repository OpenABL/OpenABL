#include "FileUtil.hpp"
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace OpenABL {

bool fileExists(const std::string &name) {
  struct stat info;
  return stat(name.c_str(), &info) == 0 && (info.st_mode & S_IFREG);
}

bool directoryExists(const std::string &name) {
  struct stat info;
  return stat(name.c_str(), &info) == 0 && (info.st_mode & S_IFDIR);
}

void createDirectory(const std::string &name) {
  if (directoryExists(name)) {
    return;
  }

#ifdef _WIN32
  bool success = CreateDirectory(name.c_str(), NULL) != 0;
#else
  bool success = mkdir(name.c_str(), 0755) == 0;
#endif
  if (!success) {
    throw FileError("Failed to create directory \"" + name + "\"");
  }
}

void writeToFile(const std::string &name, const std::string &contents) {
  std::ofstream f(name);
  if (!f.is_open()) {
    throw FileError("Failed to open file \"" + name + "\"");
  }

  f << contents;
}

void copyFile(const std::string &from, const std::string &to) {
  std::ifstream src(from);
  if (!src.is_open()) {
    throw FileError("Failed to open source file \"" + from + "\"");
  }

  std::ofstream dst(to);
  if (!dst.is_open()) {
    throw FileError("Failed to open destination file \"" + to + "\"");
  }

  dst << src.rdbuf();
}

void makeFileExecutable(const std::string &name) {
#ifndef _WIN32
  if (chmod(name.c_str(), 0755)) {
    throw FileError("Failed to make \"" + name + "\" executable");
  }
#endif
}

void changeWorkingDirectory(const std::string &name) {
  if (chdir(name.c_str())) {
    throw FileError("Failed to change CWD to \"" + name + "\"");
  }
}

bool executeCommand(const std::string &cmd) {
  return system(cmd.c_str()) == 0;
}

}

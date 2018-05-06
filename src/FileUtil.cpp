/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

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

std::string createTemporaryDirectory() {
  char dir[] = "/tmp/openabl_XXXXXX";
  char *result = mkdtemp(dir);
  if (!result) {
    throw FileError("Failure to create temporary directory");
  }

  return result;
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

std::string getAbsolutePath(const std::string &name) {
  char *ret = realpath(name.c_str(), nullptr);
  if (!ret) {
    return "";
  }

  std::string absPath(ret);
  free(ret);
  return absPath;
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

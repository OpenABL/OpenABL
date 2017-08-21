#pragma once

#include <string>
#include <stdexcept>

namespace OpenABL {

struct FileError : public std::runtime_error {
  FileError(const std::string &msg) : std::runtime_error(msg) {}
};

bool fileExists(const std::string &name);
bool directoryExists(const std::string &name);
void createDirectory(const std::string &name);
void writeToFile(const std::string &name, const std::string &contents);
void copyFile(const std::string &from, const std::string &to);
void makeFileExecutable(const std::string &name);
void changeWorkingDirectory(const std::string &name);
bool executeCommand(const std::string &cmd);

}

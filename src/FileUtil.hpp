#pragma once

#include <string>

namespace OpenABL {

bool directoryExists(const std::string &name);
bool createDirectory(const std::string &name);
void writeToFile(const std::string &name, const std::string &contents);
void copyFile(const std::string &from, const std::string &to);
void makeFileExecutable(const std::string &name);

}

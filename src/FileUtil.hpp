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
std::string createTemporaryDirectory();
void writeToFile(const std::string &name, const std::string &contents);
void copyFile(const std::string &from, const std::string &to);
void makeFileExecutable(const std::string &name);
std::string getAbsolutePath(const std::string &name);
void changeWorkingDirectory(const std::string &name);
bool executeCommand(const std::string &cmd);

}

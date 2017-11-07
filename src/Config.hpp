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

#include <map>

namespace OpenABL {

struct ConfigError : public std::runtime_error {
  ConfigError(const std::string &msg) : std::runtime_error(msg) {}
};

struct Config {
  std::map<std::string, std::string> config;

  bool getBool(const std::string &name, bool defaultValue) const;
  long getInt(const std::string &name, long defaultValue) const;
};

}

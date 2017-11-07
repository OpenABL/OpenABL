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

#include "Analysis.hpp"
#include "Config.hpp"

namespace OpenABL {

bool Config::getBool(
    const std::string &name, bool defaultValue) const {
  auto it = config.find(name);
  if (it == config.end()) {
    return defaultValue;
  }

  Value v = Value::fromString(it->second);
  if (!v.isBool()) {
    throw ConfigError("Value of " + name + " must be boolean");
  }

  return v.getBool();
}

long Config::getInt(
    const std::string &name, long defaultValue) const {
  auto it = config.find(name);
  if (it == config.end()) {
    return defaultValue;
  }

  Value v = Value::fromString(it->second);
  if (!v.isInt()) {
    throw ConfigError("Value of " + name + " must be integer");
  }

  return v.getInt();
}

}

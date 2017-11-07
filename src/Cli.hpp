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
#include <string>

namespace OpenABL {
namespace Cli {

struct OptionError : public std::runtime_error {
  OptionError(const std::string &msg) : std::runtime_error(msg) {}
};

struct Options {
  bool help;
  bool lintOnly;
  bool build;
  bool run;
  std::string fileName;
  std::string backend;
  std::string outputDir;
  std::string assetDir;
  std::map<std::string, std::string> params;
  std::map<std::string, std::string> config;
};

Options parseOptions(int argc, char **argv);

}
}

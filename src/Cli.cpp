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

#include "Cli.hpp"

namespace OpenABL {
namespace Cli {

static std::pair<std::string, std::string> parsePair(const std::string &str, const char *type) {
  size_t pos = str.find('=');
  if (pos == std::string::npos) {
    throw OptionError(std::string("Malformed ") + type + ": Missing \"=\"");
  }

  std::string name = std::string(str, 0, pos);
  std::string value = std::string(str, pos + 1);
  return { name, value };
}

Options parseOptions(int argc, char **argv) {
  Options options = {};
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      options.help = true;
      return options;
    }

    if (arg == "--lint-only") {
      options.lintOnly = true;
      continue;
    } else if (arg == "--build" || arg == "-B") {
      options.build = true;
      continue;
    } else if (arg == "--run" || arg == "-R") {
      options.run = true;
      continue;
    }

    if (i + 1 == argc) {
      throw OptionError("Missing argument for option \"" + arg + "\"");
    }

    if (arg == "-b" || arg == "--backend") {
      options.backend = argv[++i];
    } else if (arg == "-i" || arg == "--input") {
      options.fileName = argv[++i];
    } else if (arg == "-o" || arg == "--output-dir") {
      options.outputDir = argv[++i];
    } else if (arg == "-A" || arg == "--asset-dir") {
      options.assetDir = argv[++i];
    } else if (arg == "-P" || arg == "--param") {
      options.params.insert(parsePair(argv[++i], "parameter"));
    } else if (arg == "-C" || arg == "--config") {
      options.config.insert(parsePair(argv[++i], "configuration value"));
    } else {
      throw OptionError("Unknown option \"" + arg + "\"");
    }
  }

  if (options.fileName.empty()) {
    throw OptionError("Missing input file (-i or --input)");
  }

  if (options.assetDir.empty()) {
    options.assetDir = "./asset";
  }

  if (options.lintOnly) {
    return options;
  }

  if (options.backend.empty()) {
    throw OptionError("Missing backend (-b or --backend)");
  }

  return options;
}

}
}

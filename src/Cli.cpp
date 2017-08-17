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

  if (options.outputDir.empty()) {
    throw OptionError("Missing output directory (-o or --output-dir)");
  }

  if (options.backend.empty()) {
    options.backend = "c";
  }

  return options;
}

}
}

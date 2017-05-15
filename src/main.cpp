#include <iostream>
#include "ParserContext.hpp"
#include "Analysis.hpp"
#include "AnalysisVisitor.hpp"
#include "FileUtil.hpp"
#include "backend/Backend.hpp"

namespace OpenABL {

void registerBuiltinFunctions(BuiltinFunctions &funcs) {
  funcs.add("dot", "dot_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT32);
  funcs.add("dot", "dot_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT32);
  funcs.add("length", "length_float2", { Type::VEC2 }, Type::FLOAT32);
  funcs.add("length", "length_float3", { Type::VEC3 }, Type::FLOAT32);
  funcs.add("dist", "dist_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT32);
  funcs.add("dist", "dist_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT32);
  funcs.add("random", "random_float", { Type::FLOAT32, Type::FLOAT32 }, Type::FLOAT32);
  funcs.add("random", "random_float2", { Type::VEC2, Type::VEC2 }, Type::VEC2);
  funcs.add("random", "random_float3", { Type::VEC3, Type::VEC3 }, Type::VEC3);

  // Agent specific functions
  funcs.add("add", "add", { Type::AGENT }, Type::VOID);
  funcs.add("near", "near", { Type::AGENT, Type::FLOAT32 }, { Type::ARRAY, Type::AGENT });
  funcs.add("save", "save", { Type::STRING }, Type::VOID);
}

std::map<std::string, std::unique_ptr<Backend>> getBackends() {
  std::map<std::string, std::unique_ptr<Backend>> backends;
  backends["c"] = std::unique_ptr<Backend>(new CBackend);
  backends["flame"] = std::unique_ptr<Backend>(new FlameBackend);
  backends["flamegpu"] = std::unique_ptr<Backend>(new FlameGPUBackend);
  backends["mason"] = std::unique_ptr<Backend>(new MasonBackend);
  return backends;
}

}

struct Options {
  bool help;
  bool lintOnly;
  std::string fileName;
  std::string backend;
  std::string outputDir;
  std::string assetDir;
};

static Options parseCliOptions(int argc, char **argv) {
  Options options = {};
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg == "-h" || arg == "--help") {
      options.help = true;
      return options;
    }

    if (i + 1 == argc) {
      std::cerr << "Missing argument for option \"" << arg << "\"" << std::endl;
      return {};
    }
    if (arg == "-b" || arg == "--backend") {
      options.backend = argv[++i];
    } else if (arg == "-i" || arg == "--input") {
      options.fileName = argv[++i];
    } else if (arg == "-o" || arg == "--output-dir") {
      options.outputDir = argv[++i];
    } else if (arg == "-A" || arg == "--asset-dir") {
      options.assetDir = argv[++i];
    } else if (arg == "--lint-only") {
      options.lintOnly = true;
    } else {
      std::cerr << "Unknown option \"" << arg << "\"" << std::endl;
      return {};
    }
  }

  if (options.fileName.empty()) {
    std::cerr << "Missing input file (-i or --input)" << std::endl;
    return {};
  }

  if (options.lintOnly) {
    return options;
  }

  if (options.outputDir.empty()) {
    std::cerr << "Missing output directory (-o or --output-dir)" << std::endl;
    return {};
  }

  if (options.backend.empty()) {
    options.backend = "c";
  }

  if (options.assetDir.empty()) {
    options.assetDir = "./asset";
  }

  return options;
}

void printHelp() {
  std::cout << "Usage: ./OpenABL -i input.abl -o ./output-dir\n\n"
               "Options:\n"
               "  -A, --asset-dir    Asset directory (default: ./asset)\n"
               "  -b, --backend      Backend (default: c)\n"
               "  -h, --help         Display this help\n"
               "  -i, --input        Input file\n"
               "  -o, --output-dir   Output directory\n\n"
               "Available backends:\n"
               " * c        (working)\n"
               " * flame    (partially working)\n"
               " * flamegpu (not working)\n"
               " * mason    (not working)"
            << std::endl;
}

int main(int argc, char **argv) {
  Options options = parseCliOptions(argc, argv);
  if (options.help) {
    printHelp();
    return 0;
  }

  if (options.fileName.empty()) {
    return 1;
  }

  FILE *file = fopen(options.fileName.c_str(), "r");
  if (!file) {
    std::cerr << "File \"" << options.fileName << "\" could not be opened." << std::endl;
    return 1;
  }

  OpenABL::ParserContext ctx(file);
  if (!ctx.parse()) {
    return 1;
  }

  OpenABL::AST::Script &script = *ctx.script;

  int retval = 0;
  OpenABL::ErrorStream err([&retval](const OpenABL::Error &err) {
    std::cerr << err.msg << " on line " << err.loc.begin.line << std::endl;
    retval = 1;
  });

  OpenABL::BuiltinFunctions funcs;
  registerBuiltinFunctions(funcs);

  OpenABL::AnalysisVisitor visitor(script, err, funcs);
  script.accept(visitor);

  if (options.lintOnly) {
    return retval;
  }

  if (!OpenABL::createDirectory(options.outputDir)) {
    std::cerr << "Failed to create directory \"" << options.outputDir << "\"." << std::endl;
    return 1;
  }

  auto backends = OpenABL::getBackends();
  auto it = backends.find(options.backend);
  if (it == backends.end()) {
    std::cerr << "Unknown backend \"" << options.backend << "\"" << std::endl;
    return 1;
  }

  if (!OpenABL::directoryExists(options.assetDir)) {
    std::cerr << "Asset directory \"" << options.assetDir << "\" does not exist "
              << "(override with -A or --asset-dir)" << std::endl;
    return 1;
  }

  OpenABL::Backend &backend = *it->second;
  backend.generate(script, options.outputDir, options.assetDir);

  fclose(file);
  return retval;
}

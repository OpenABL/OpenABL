#include <chrono>
#include <iostream>
#include "Cli.hpp"
#include "ParserContext.hpp"
#include "Analysis.hpp"
#include "AnalysisVisitor.hpp"
#include "FileUtil.hpp"
#include "backend/Backend.hpp"

namespace OpenABL {

void registerBuiltinFunctions(FunctionList &funcs) {
  funcs.add("dot", "dot_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT);
  funcs.add("dot", "dot_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT);
  funcs.add("length", "length_float2", { Type::VEC2 }, Type::FLOAT);
  funcs.add("length", "length_float3", { Type::VEC3 }, Type::FLOAT);
  funcs.add("dist", "dist_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT);
  funcs.add("dist", "dist_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT);
  funcs.add("normalize", "normalize_float2", { Type::VEC2 }, Type::VEC2);
  funcs.add("normalize", "normalize_float3", { Type::VEC3 }, Type::VEC3);
  funcs.add("random", "random_float", { Type::FLOAT, Type::FLOAT }, Type::FLOAT);
  funcs.add("randomInt", "random_int", { Type::INT32, Type::INT32 }, Type::INT32);

  funcs.add("sin", { Type::FLOAT }, Type::FLOAT);
  funcs.add("cos", { Type::FLOAT }, Type::FLOAT);
  funcs.add("tan", { Type::FLOAT }, Type::FLOAT);
  funcs.add("sinh", { Type::FLOAT }, Type::FLOAT);
  funcs.add("cosh", { Type::FLOAT }, Type::FLOAT);
  funcs.add("tanh", { Type::FLOAT }, Type::FLOAT);
  funcs.add("asin", { Type::FLOAT }, Type::FLOAT);
  funcs.add("acos", { Type::FLOAT }, Type::FLOAT);
  funcs.add("atan", { Type::FLOAT }, Type::FLOAT);
  funcs.add("exp", { Type::FLOAT }, Type::FLOAT);
  funcs.add("log", { Type::FLOAT }, Type::FLOAT);
  funcs.add("sqrt", { Type::FLOAT }, Type::FLOAT);
  funcs.add("round", { Type::FLOAT }, Type::FLOAT);
  funcs.add("pow", { Type::FLOAT, Type::FLOAT }, Type::FLOAT);

  // Agent specific functions
  funcs.add("add", { Type::AGENT }, Type::VOID);
  funcs.add("near", { Type::AGENT, Type::FLOAT }, { Type::ARRAY, Type::AGENT });
  funcs.add("save", { Type::STRING }, Type::VOID);
}

std::map<std::string, std::unique_ptr<Backend>> getBackends() {
  std::map<std::string, std::unique_ptr<Backend>> backends;
  backends["c"] = std::unique_ptr<Backend>(new CBackend);
  backends["flame"] = std::unique_ptr<Backend>(new FlameBackend);
  backends["flamegpu"] = std::unique_ptr<Backend>(new FlameGPUBackend);
  backends["mason"] = std::unique_ptr<Backend>(new MasonBackend);
  backends["dmason"] = std::unique_ptr<Backend>(new DMasonBackend);
  return backends;
}

void printHelp() {
  std::cout << "Usage: ./OpenABL -i input.abl -o ./output-dir -b backend\n\n"
               "Options:\n"
               "  -A, --asset-dir    Asset directory (default: ./asset)\n"
               "  -b, --backend      Backend (default: c)\n"
               "  -B, --build        Build the generated code\n"
               "  -C, --config       Specify a configuration value (name=value)\n"
               "  -h, --help         Display this help\n"
               "  -i, --input        Input file\n"
               "  -o, --output-dir   Output directory\n"
               "  -P, --param        Specify a simulation parameter (name=value)\n"
               "  -R, --run          Build and run the generated code\n"
               "\n"
               "Available backends:\n"
               " * c        (working)\n"
               " * flame    (mostly working)\n"
               " * flamegpu (mostly working)\n"
               " * mason    (mostly working)\n"
               " * dmason   (mostly working)\n"
               "\n"
               "Available configuration options:\n"
               " * bool use_float (default: false)\n"
            << std::flush;
}

int main(int argc, char **argv) {
  Cli::Options options;
  try {
    options = Cli::parseOptions(argc, argv);
  } catch (const Cli::OptionError &e) {
    printHelp();
    std::cerr << "\nERROR: " << e.what() << std::endl;
    return 1;
  }

  if (options.help) {
    printHelp();
    return 0;
  }

  if (options.fileName.empty()) {
    return 1;
  }

  FILE *mainFile = fopen(options.fileName.c_str(), "r");
  if (!mainFile) {
    std::cerr << "File \"" << options.fileName << "\" could not be opened." << std::endl;
    return 1;
  }

  if (!directoryExists(options.assetDir)) {
    std::cerr << "Asset directory \"" << options.assetDir << "\" does not exist "
              << "(override with -A or --asset-dir)" << std::endl;
    return 1;
  }

  std::string libFileName = options.assetDir + "/lib.abl";
  FILE *libFile = fopen(libFileName.c_str(), "r");
  if (!libFile) {
    std::cerr << "Library file \"" << libFileName << "\" could not be opened." << std::endl;
    return 1;
  }

  ParserContext mainCtx(mainFile);
  ParserContext libCtx(libFile);
  if (!mainCtx.parse() || !libCtx.parse()) {
    return 1;
  }

  AST::Script &mainScript = *mainCtx.script;
  AST::Script &libScript = *libCtx.script;

  int numErrors = 0;
  ErrorStream err([&numErrors](const Error &err) {
    std::cerr << err.msg << " on line " << err.loc.begin.line << std::endl;
    numErrors++;
  });

  FunctionList funcs;
  registerBuiltinFunctions(funcs);

  AnalysisVisitor visitor(mainScript, options.params, err, funcs, options.backend);
  visitor.handleLibScript(libScript);
  visitor.handleMainScript(mainScript);

  if (numErrors > 0) {
    // There were errors, abort
    return 1;
  }

  if (options.lintOnly) {
    // Linting only, don't try to generate output
    return 0;
  }

  createDirectory(options.outputDir);

  auto backends = getBackends();
  auto it = backends.find(options.backend);
  if (it == backends.end()) {
    std::cerr << "Unknown backend \"" << options.backend << "\"" << std::endl;
    return 1;
  }

  Backend &backend = *it->second;
  try {
    BackendContext backendCtx = {
      options.outputDir, options.assetDir, Config { options.config }
    };
    backend.generate(mainScript, backendCtx);
  } catch (const std::runtime_error &e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }

  if (options.build || options.run) {
    changeWorkingDirectory(options.outputDir);
    if (!fileExists("./build.sh")) {
      std::cerr << "Build file for this backend not found" << std::endl;
      return 1;
    }

    if (!executeCommand("./build.sh")) {
      std::cerr << "Build failed" << std::endl;
      return 1;
    }

    if (options.run) {
      if (!fileExists("./run.sh")) {
        std::cerr << "Run file for this backend not found" << std::endl;
        return 1;
      }

      auto start = std::chrono::high_resolution_clock::now();
      if (!executeCommand("./run.sh")) {
        std::cerr << "Run failed" << std::endl;
        return 1;
      }
      auto end = std::chrono::high_resolution_clock::now();

      auto msecs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      auto secs = msecs/1000.0;
      std::cout << "Execution time: " << secs << "s" << std::endl;
    }
  }

  fclose(mainFile);
  return 0;
}

}

int main(int argc, char **argv) {
  return OpenABL::main(argc, argv);
}

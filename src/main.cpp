#include <iostream>
#include "ParserContext.hpp"
#include "Analysis.hpp"
#include "AnalysisVisitor.hpp"
#include "backend/CBackend.hpp"

namespace OpenABL {

void registerBuiltinFunctions(BuiltinFunctions &funcs) {
  funcs.add("dist", "dist_float2", { Type::VEC2, Type::VEC2 }, Type::FLOAT32);
  funcs.add("dist", "dist_float3", { Type::VEC3, Type::VEC3 }, Type::FLOAT32);
  funcs.add("near", "near",
      { { Type::ARRAY, Type::AGENT }, Type::AGENT, Type::FLOAT32 },
      { Type::ARRAY, Type::AGENT });
  funcs.add("save", "save", { { Type::ARRAY, Type::AGENT }, Type::STRING }, Type::VOID);
}

}

struct Options {
  const char *fileName;
  const char *backend;
};

static Options parseCliOptions(int argc, char **argv) {
  Options options = {};
  for (int i = 1; i < argc; i++) {
    std::string arg = argv[i];
    if (arg[0] == '-') {
      if (i + 1 == argc) {
        std::cerr << "Missing argument for option " << arg << std::endl;
        return {};
      }
      if (arg == "-b" || arg == "--backend") {
        options.backend = argv[++i];
      }
    } else {
      if (options.fileName) {
        std::cerr << "Expected exactly one file name" << std::endl;
        return {};
      }
      options.fileName = argv[i];
    }
  }

  if (!options.fileName) {
    std::cerr << "Missing file name" << std::endl;
    return {};
  }

  if (!options.backend) {
    options.backend = "c";
  }

  return options;
}

int main(int argc, char **argv) {
  Options options = parseCliOptions(argc, argv);
  if (!options.fileName) {
    return 1;
  }

  FILE *file = fopen(options.fileName, "r");
  if (!file) {
    std::cerr << "File \"" << options.fileName << "\" could not be opened." << std::endl;
    return 1;
  }

  OpenABL::ParserContext ctx(file);
  if (!ctx.parse()) {
    return 1;
  }

  OpenABL::AST::Script &script = *ctx.script;

  OpenABL::ErrorStream err([](const OpenABL::Error &err) {
    std::cerr << err.msg << " on line " << err.loc.begin.line << std::endl;
  });

  OpenABL::BuiltinFunctions funcs;
  registerBuiltinFunctions(funcs);

  OpenABL::AnalysisVisitor visitor(err, funcs);
  script.accept(visitor);

  OpenABL::CPrinter printer;
  printer.print(script);
  std::cout << printer.extractStr() << std::endl;

  fclose(file);
  return 0;
}

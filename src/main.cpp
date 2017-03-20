#include <iostream>
#include "ParserContext.hpp"
#include "Analysis.hpp"
#include "AnalysisVisitor.hpp"
#include "backend/CBackend.hpp"

namespace OpenABL {

void registerBuiltinFunctions(BuiltinFunctions &funcs) {
  funcs.add("dist", { Type::VEC2, Type::VEC2 }, Type::FLOAT32);
  funcs.add("dist", { Type::VEC3, Type::VEC3 }, Type::FLOAT32);
  funcs.add("near", { Type::AGENT, Type::FLOAT32 }, { Type::ARRAY, Type::AGENT });
  funcs.add("save", { { Type::ARRAY, Type::AGENT }, Type::STRING }, Type::VOID);
}

}

int main(int argc, char **argv) {
  if (argc != 2) {
    std::cerr << "Missing filename argument." << std::endl;
    return 1;
  }

  char *fileName = argv[1];
  FILE *file = fopen(fileName, "r");
  if (!file) {
    std::cerr << "File " << fileName << " could not be opened." << std::endl;
    return 1;
  }

  OpenABL::ParserContext ctx(file);
  if (!ctx.parse()) {
    return 1;
  }

  OpenABL::AST::Script &script = *ctx.script;

  OpenABL::ErrorStream err([](const OpenABL::Error &err) {
    std::cout << err.msg << " on line " << err.loc.begin.line << std::endl;
  });

  OpenABL::BuiltinFunctions funcs;
  registerBuiltinFunctions(funcs);

  OpenABL::AnalysisVisitor visitor(err, funcs);
  script.accept(visitor);

  OpenABL::CPrinter printer;
  printer.print(script);
  std::cout << "Printed:\n" << printer.extractStr() << std::endl;

  fclose(file);
  return 0;
}

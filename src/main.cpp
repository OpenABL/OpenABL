#include <iostream>
#include "ParserContext.hpp"
#include "AnalysisVisitor.hpp"

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
  OpenABL::AnalysisVisitor visitor;
  script.accept(visitor);

  fclose(file);
  return 0;
}

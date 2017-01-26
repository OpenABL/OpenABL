#pragma once

#include "Parser.hpp"

namespace OpenABL {

struct ParserContext {
  FILE *file;
  AST::Script *script;

  ParserContext(FILE *file)
    : file{file}, script{nullptr} {}

  bool parse();

private:
  void initLexer();
};

}

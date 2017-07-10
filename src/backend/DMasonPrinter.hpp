#pragma once
#include "MasonPrinter.hpp"

namespace OpenABL {

struct DMasonPrinter : public MasonPrinter {
  using MasonPrinter::print;

  DMasonPrinter(AST::Script &script)
    : MasonPrinter(script) {}

  // TODO Overridden methods are declared here
  void print(const AST::Script &);
};

}

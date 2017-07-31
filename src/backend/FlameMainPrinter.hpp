#pragma once

#include "CPrinter.hpp"

namespace OpenABL {

/* This printer prints the main() function code for the Flame and FlameGPU backends, as these
 * backends do not provide a native way of executing code outside the simulation. The generated
 * code is based on the C backend, which is however only used to generate the agents and then
 * export/import them in the format required by Flame and FlameGPU. */
struct FlameMainPrinter : public CPrinter {
  using CPrinter::print;

  FlameMainPrinter(AST::Script &script)
    : CPrinter(script) {}

  void print(const AST::SimulateStatement &);
  void print(const AST::FunctionDeclaration &);
};

}

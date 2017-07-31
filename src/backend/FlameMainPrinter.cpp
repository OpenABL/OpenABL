#include "FlameMainPrinter.hpp"

namespace OpenABL {

void FlameMainPrinter::print(const AST::SimulateStatement &) {
  // TODO
  *this << "save(&agents, agents_info, \"iterations/0.xml\", SAVE_FLAME_XML);";
}

void FlameMainPrinter::print(const AST::FunctionDeclaration &decl) {
  if (!decl.isMain()) {
    // Only print the main() function
    // TODO We also need to print helper functions
    return;
  }

  CPrinter::print(decl);
}

}

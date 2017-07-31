#include "FlameMainPrinter.hpp"

namespace OpenABL {

void FlameMainPrinter::print(const AST::SimulateStatement &stmt) {
  *this << "save(&agents, agents_info, \"iterations/0.xml\", SAVE_FLAME_XML);" << nl;
  if (forGPU) {
    // TODO execute using FlameGPU
  } else {
    *this << "char _cmd_buf[100];" << nl
          << "snprintf(_cmd_buf, sizeof(_cmd_buf), \"./main %d iterations/0.xml\", "
          << *stmt.timestepsExpr << ");" << nl
          << "int _cmd_ret = system(_cmd_buf);" << nl
          << "if (_cmd_ret != 0) { return _cmd_ret; }" << nl;
  }
  // TODO reload agents and continue running
}

void FlameMainPrinter::print(const AST::FunctionDeclaration &decl) {
  if (!decl.isMain()) {
    // Only print the main() function
    // TODO Do we also need to print helper functions?
    return;
  }

  CPrinter::print(decl);
}

void FlameMainPrinter::print(const AST::Script &script) {
  *this << "#include <stdio.h>\n";
  CPrinter::print(script);
}

}

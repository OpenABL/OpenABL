/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "FlameMainPrinter.hpp"

namespace OpenABL {

void FlameMainPrinter::print(const AST::SimulateStatement &stmt) {
  if (params.forGPU) {
    *this << "save(&agents, agents_info, \"iterations/0.xml\", SAVE_FLAMEGPU_XML);"
          << "char _cmd_buf[100];" << nl;
    std::string binary = "./main";
    if (params.profile) {
      binary = "nvprof --csv --print-api-trace --log-file profile.csv " + binary;
    }
    if (params.visualize) {
      // Visualization does not use timesteps
      *this << "snprintf(_cmd_buf, sizeof(_cmd_buf), \"" + binary + " iterations/0.xml\");" << nl;
    } else {
      *this << "snprintf(_cmd_buf, sizeof(_cmd_buf), \"" + binary + " iterations/0.xml %d\", "
            << *stmt.timestepsExpr << ");" << nl;
    }
    *this << "int _cmd_ret = system(_cmd_buf);" << nl
          << "if (_cmd_ret != 0) { return _cmd_ret; }" << nl;
  } else {
    *this << "save(&agents, agents_info, \"iterations/0.xml\", SAVE_FLAME_XML);"
          << "char _cmd_buf[100];" << nl
          << "snprintf(_cmd_buf, sizeof(_cmd_buf), ";
    if (params.parallel) {
      *this << "\"mpirun -np 4 ./main %d iterations/0.xml -r\"";
    } else {
      *this << "\"./main %d iterations/0.xml\"";
    }
    *this << ", " << *stmt.timestepsExpr << ");" << nl
          << "int _cmd_ret = system(_cmd_buf);" << nl
          << "if (_cmd_ret != 0) { return _cmd_ret; }" << nl;
  }

  // TODO reload agents and continue running
}

void FlameMainPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.isAnyStep()) {
    // Don't print step functions, those are in the simulation code
    return;
  }

  CPrinter::print(decl);
}

void FlameMainPrinter::print(const AST::Script &script) {
  *this << "#include <stdio.h>\n";
  CPrinter::print(script);
}

}

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

#pragma once

#include "CPrinter.hpp"

namespace OpenABL {

/* This printer prints the main() function code for the Flame and FlameGPU backends, as these
 * backends do not provide a native way of executing code outside the simulation. The generated
 * code is based on the C backend, which is however only used to generate the agents and then
 * export/import them in the format required by Flame and FlameGPU. */
struct FlameMainPrinter : public CPrinter {
  struct Params {
    bool forGPU;
    bool useFloat;
    bool parallel;  // Flame only
    bool visualize; // FlameGPU only
    bool profile;   // FlameGPU only

    static Params createForFlame(bool useFloat, bool parallel) {
      return { false, useFloat, parallel, false, false };
    }
    static Params createForFlameGPU(bool useFloat, bool visualize, bool profile) {
      return { true, useFloat, false, visualize, profile };
    }
  };

  using CPrinter::print;

  FlameMainPrinter(AST::Script &script, Params params)
    : CPrinter(script, params.useFloat), params(params) {}

  void print(const AST::SimulateStatement &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::Script &);

private:
  Params params;
};

}

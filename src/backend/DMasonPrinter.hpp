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

#include "MasonPrinter.hpp"
#include "Config.hpp"

namespace OpenABL {

struct DMasonPrinter : public MasonPrinter {
  using MasonPrinter::print;

  DMasonPrinter(AST::Script &script)
    : MasonPrinter(script) {}

  void printStubAgent(const AST::AgentDeclaration &);
  void printLocalTestCode(const Config &);
  void print(const AST::Script &);
  void print(const AST::CallExpression &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::SimulateStatement &);

  void printAgentImports();
  void printAgentExtends(const AST::AgentDeclaration &);
  void printAgentExtraCode(const AST::AgentDeclaration &);
  void printAgentExtraCtorArgs();
  void printAgentExtraCtorCode();
  void printStepDefaultCode(const AST::AgentDeclaration &);
  void printUIExtraImports();
  void printUICtors();
};

}

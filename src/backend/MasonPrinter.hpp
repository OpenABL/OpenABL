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
#include "GenericPrinter.hpp"

namespace OpenABL {

struct MasonPrinter : public GenericPrinter {
  using GenericPrinter::print;

  MasonPrinter(AST::Script &script)
    : GenericPrinter(script, true) {}

  void print(const AST::VarExpression &);
  void print(const AST::UnaryOpExpression &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::AssignOpStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::Script &);

  void printType(Type t);

  void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
  bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);

  virtual void printAgentImports();
  virtual void printAgentExtends(const AST::AgentDeclaration &);
  virtual void printAgentExtraCode(const AST::AgentDeclaration &);
  virtual void printAgentExtraCtorArgs();
  virtual void printAgentExtraCtorCode();
  virtual void printStepDefaultCode(const AST::AgentDeclaration &);
  virtual void printUIExtraImports();
  virtual void printUICtors();

  void printUI();

protected:
  const char *getSimVarName() const {
    return inAgent ? "_sim" : "this";
  }

  // Current input and output variables inside a step function
  VarId currentInVar;
  VarId currentOutVar;
  // Whether we're in agent code
  bool inAgent = false;
  // Whether we're in the main function
  bool inMain = false;
};

}

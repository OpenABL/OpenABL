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

#include "AST.hpp"
#include "GenericPrinter.hpp"
#include "FlameModel.hpp"

namespace OpenABL {

struct FlameGPUPrinter : public GenericPrinter {
  using GenericPrinter::print;

  FlameGPUPrinter(AST::Script &script, const FlameModel &model, bool useFloat)
    : GenericPrinter(script, true), script(script), model(model), useFloat(useFloat) {}

  void print(const AST::Literal &);
  void print(const AST::VarExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::Script &);

  void printType(Type t);

  virtual void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
  virtual bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);

private:
  AST::Script &script;
  const FlameModel &model;
  bool useFloat;

  const FlameModel::Func *currentFunc = nullptr;
  // Current agent, input and output variables inside a step function
  const AST::AgentDeclaration *currentAgent = nullptr;
  const AST::Var *currentInVar = nullptr;
  const AST::Var *currentOutVar = nullptr;
  const AST::Var *currentNearVar = nullptr;
};

}

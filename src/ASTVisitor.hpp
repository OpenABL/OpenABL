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

namespace OpenABL {
namespace AST {

struct Visitor {
  virtual void enter(Var &) {};
  virtual void enter(Literal &) {};
  virtual void enter(VarExpression &) {};
  virtual void enter(UnaryOpExpression &) {};
  virtual void enter(BinaryOpExpression &) {};
  virtual void enter(CallExpression &) {};
  virtual void enter(MemberAccessExpression &) {};
  virtual void enter(EnvironmentAccessExpression &) {};
  virtual void enter(ArrayAccessExpression &) {};
  virtual void enter(TernaryExpression &) {};
  virtual void enter(MemberInitEntry &) {};
  virtual void enter(AgentCreationExpression &) {};
  virtual void enter(ArrayInitExpression &) {};
  virtual void enter(NewArrayExpression &) {};
  virtual void enter(ExpressionStatement &) {};
  virtual void enter(AssignStatement &) {};
  virtual void enter(AssignOpStatement &) {};
  virtual void enter(BlockStatement &) {};
  virtual void enter(VarDeclarationStatement &) {};
  virtual void enter(IfStatement &) {};
  virtual void enter(WhileStatement &) {};
  virtual void enter(ForStatement &) {};
  virtual void enter(SimulateStatement &) {};
  virtual void enter(ReturnStatement &) {};
  virtual void enter(BreakStatement &) {};
  virtual void enter(ContinueStatement &) {};
  virtual void enter(SimpleType &) {};
  virtual void enter(Param &) {};
  virtual void enter(FunctionDeclaration &) {};
  virtual void enter(AgentMember &) {};
  virtual void enter(AgentDeclaration &) {};
  virtual void enter(ConstDeclaration &) {};
  virtual void enter(EnvironmentDeclaration &) {};
  virtual void enter(Script &) {};

  virtual void leave(Var &) {};
  virtual void leave(Literal &) {};
  virtual void leave(VarExpression &) {};
  virtual void leave(UnaryOpExpression &) {};
  virtual void leave(BinaryOpExpression &) {};
  virtual void leave(CallExpression &) {};
  virtual void leave(MemberAccessExpression &) {};
  virtual void leave(EnvironmentAccessExpression &) {};
  virtual void leave(ArrayAccessExpression &) {};
  virtual void leave(TernaryExpression &) {};
  virtual void leave(MemberInitEntry &) {};
  virtual void leave(AgentCreationExpression &) {};
  virtual void leave(ArrayInitExpression &) {};
  virtual void leave(NewArrayExpression &) {};
  virtual void leave(ExpressionStatement &) {};
  virtual void leave(AssignStatement &) {};
  virtual void leave(AssignOpStatement &) {};
  virtual void leave(BlockStatement &) {};
  virtual void leave(VarDeclarationStatement &) {};
  virtual void leave(IfStatement &) {};
  virtual void leave(WhileStatement &) {};
  virtual void leave(ForStatement &) {};
  virtual void leave(SimulateStatement &) {};
  virtual void leave(ReturnStatement &) {};
  virtual void leave(BreakStatement &) {};
  virtual void leave(ContinueStatement &) {};
  virtual void leave(SimpleType &) {};
  virtual void leave(Param &) {};
  virtual void leave(FunctionDeclaration &) {};
  virtual void leave(AgentMember &) {};
  virtual void leave(AgentDeclaration &) {};
  virtual void leave(ConstDeclaration &) {};
  virtual void leave(EnvironmentDeclaration &) {};
  virtual void leave(Script &) {};

  void replaceExpr(Expression *expr) {
    assert(inExpr);
    replacementExpr = expr;
  }

  bool inExpr = false;
  Expression *replacementExpr = nullptr;
};

}
}

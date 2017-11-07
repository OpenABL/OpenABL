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

#include <map>
#include <stack>
#include "ASTVisitor.hpp"
#include "Analysis.hpp"
#include "ErrorHandling.hpp"

namespace OpenABL {

struct AnalysisVisitor : public AST::Visitor {
  AnalysisVisitor(
      AST::Script &script, const std::map<std::string, std::string> &params,
      ErrorStream &err, FunctionList &builtins, const std::string &backend
  ) : script(script), params(params), funcs(builtins), err(err),
      scope(script.scope), backend(backend) {}

  void enter(AST::Var &);
  void enter(AST::Literal &);
  void enter(AST::VarExpression &);
  void enter(AST::UnaryOpExpression &);
  void enter(AST::BinaryOpExpression &);
  void enter(AST::CallExpression &);
  void enter(AST::MemberAccessExpression &);
  void enter(AST::ArrayAccessExpression &);
  void enter(AST::TernaryExpression &);
  void enter(AST::MemberInitEntry &);
  void enter(AST::AgentCreationExpression &);
  void enter(AST::ArrayInitExpression &);
  void enter(AST::NewArrayExpression &);
  void enter(AST::ExpressionStatement &);
  void enter(AST::AssignStatement &);
  void enter(AST::AssignOpStatement &);
  void enter(AST::BlockStatement &);
  void enter(AST::VarDeclarationStatement &);
  void enter(AST::IfStatement &);
  void enter(AST::WhileStatement &);
  void enter(AST::ForStatement &);
  void enter(AST::SimulateStatement &);
  void enter(AST::ReturnStatement &);
  void enter(AST::BreakStatement &);
  void enter(AST::ContinueStatement &);
  void enter(AST::SimpleType &);
  void enter(AST::Param &);
  void enter(AST::FunctionDeclaration &);
  void enter(AST::AgentMember &);
  void enter(AST::AgentDeclaration &);
  void enter(AST::ConstDeclaration &);
  void enter(AST::EnvironmentDeclaration &);
  void enter(AST::Script &);
  void leave(AST::Var &);
  void leave(AST::Literal &);
  void leave(AST::VarExpression &);
  void leave(AST::UnaryOpExpression &);
  void leave(AST::BinaryOpExpression &);
  void leave(AST::CallExpression &);
  void leave(AST::MemberAccessExpression &);
  void leave(AST::ArrayAccessExpression &);
  void leave(AST::TernaryExpression &);
  void leave(AST::MemberInitEntry &);
  void leave(AST::AgentCreationExpression &);
  void leave(AST::ArrayInitExpression &);
  void leave(AST::NewArrayExpression &);
  void leave(AST::ExpressionStatement &);
  void leave(AST::AssignStatement &);
  void leave(AST::AssignOpStatement &);
  void leave(AST::BlockStatement &);
  void leave(AST::VarDeclarationStatement &);
  void leave(AST::IfStatement &);
  void leave(AST::WhileStatement &);
  void leave(AST::ForStatement &);
  void leave(AST::SimulateStatement &);
  void leave(AST::ReturnStatement &);
  void leave(AST::BreakStatement &);
  void leave(AST::ContinueStatement &);
  void leave(AST::SimpleType &);
  void leave(AST::Param &);
  void leave(AST::FunctionDeclaration &);
  void leave(AST::AgentMember &);
  void leave(AST::AgentDeclaration &);
  void leave(AST::ConstDeclaration &);
  void leave(AST::EnvironmentDeclaration &);
  void leave(AST::Script &);

  void handleLibScript(AST::Script &script) {
    isLib = true;
    script.accept(*this);
  }

  void handleMainScript(AST::Script &script) {
    isLib = false;
    script.accept(*this);
  }

private:
  void declareVar(AST::Var &, Type, bool isConst, bool isGlobal, Value val);
  void pushVarScope();
  void popVarScope();
  Type resolveAstType(const AST::Type &);
  Value evalExpression(const AST::Expression &expr);

  using VarMap = std::map<std::string, VarId>;

  // Analyzed script
  AST::Script &script;
  // Simulation parameters
  const std::map<std::string, std::string> &params;
  // Functions
  FunctionList &funcs;
  // Stream for error reporting
  ErrorStream &err;
  // Information about *all* variables, indexed by unique VarId's
  Scope &scope;
  // The backend that is going to be used
  const std::string &backend;

  // Whether the current script is a library or main script
  bool isLib;
  // Currently *visible* variables
  VarMap varMap;
  // Stack of previous visible variable scopes
  // This is inefficient, but we don't actually care...
  std::stack<VarMap> varMapStack;
  // Current function
  AST::FunctionDeclaration *currentFunc;
  // Declared agents by name
  std::map<std::string, AST::AgentDeclaration *> agents;
  // Declared functions by name
  std::map<std::string, AST::FunctionDeclaration *> funcDecls;
  // Variable on which member accesses should be collected
  // (for FunctionDeclaration::accessedMembers)
  VarId collectAccessVar;
  // Radiuses used in near() loops
  std::vector<Value> radiuses;
  // In how many loops we are right now
  int loopNestingLevel = 0;
};

}

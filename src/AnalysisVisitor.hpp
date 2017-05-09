#pragma once

#include <map>
#include <stack>
#include "ASTVisitor.hpp"
#include "Analysis.hpp"
#include "ErrorHandling.hpp"

namespace OpenABL {

struct AnalysisVisitor : public AST::Visitor {
  AnalysisVisitor(AST::Script &script, ErrorStream &err, BuiltinFunctions &builtins)
    : script(script), builtins(builtins), err(err) {}

  void enter(AST::Var &);
  void enter(AST::Literal &);
  void enter(AST::VarExpression &);
  void enter(AST::UnaryOpExpression &);
  void enter(AST::BinaryOpExpression &);
  void enter(AST::Arg &);
  void enter(AST::CallExpression &);
  void enter(AST::MemberAccessExpression &);
  void enter(AST::TernaryExpression &);
  void enter(AST::MemberInitEntry &);
  void enter(AST::AgentCreationExpression &);
  void enter(AST::NewArrayExpression &);
  void enter(AST::ExpressionStatement &);
  void enter(AST::AssignStatement &);
  void enter(AST::AssignOpStatement &);
  void enter(AST::BlockStatement &);
  void enter(AST::VarDeclarationStatement &);
  void enter(AST::IfStatement &);
  void enter(AST::ForStatement &);
  void enter(AST::SimulateStatement &);
  void enter(AST::ReturnStatement &);
  void enter(AST::SimpleType &);
  void enter(AST::ArrayType &);
  void enter(AST::Param &);
  void enter(AST::FunctionDeclaration &);
  void enter(AST::AgentMember &);
  void enter(AST::AgentDeclaration &);
  void enter(AST::ConstDeclaration &);
  void enter(AST::Script &);
  void leave(AST::Var &);
  void leave(AST::Literal &);
  void leave(AST::VarExpression &);
  void leave(AST::UnaryOpExpression &);
  void leave(AST::BinaryOpExpression &);
  void leave(AST::Arg &);
  void leave(AST::CallExpression &);
  void leave(AST::MemberAccessExpression &);
  void leave(AST::TernaryExpression &);
  void leave(AST::MemberInitEntry &);
  void leave(AST::AgentCreationExpression &);
  void leave(AST::NewArrayExpression &);
  void leave(AST::ExpressionStatement &);
  void leave(AST::AssignStatement &);
  void leave(AST::AssignOpStatement &);
  void leave(AST::BlockStatement &);
  void leave(AST::VarDeclarationStatement &);
  void leave(AST::IfStatement &);
  void leave(AST::ForStatement &);
  void leave(AST::SimulateStatement &);
  void leave(AST::ReturnStatement &);
  void leave(AST::SimpleType &);
  void leave(AST::ArrayType &);
  void leave(AST::Param &);
  void leave(AST::FunctionDeclaration &);
  void leave(AST::AgentMember &);
  void leave(AST::AgentDeclaration &);
  void leave(AST::ConstDeclaration &);
  void leave(AST::Script &);

private:
  void declareVar(AST::Var &, Type);
  void pushVarScope();
  void popVarScope();
  Type resolveAstType(AST::Type &);

  using VarMap = std::map<std::string, VarId>;

  // Analyzed script
  AST::Script &script;
  // Builtin functions
  BuiltinFunctions &builtins;
  // Stream for error reporting
  ErrorStream &err;

  // Currently *visible* variables
  VarMap varMap;
  // Stack of previous visible variable scopes
  // This is inefficient, but we don't actually care...
  std::stack<VarMap> varMapStack;
  // Information about *all* variables, indexed by unique VarId's
  Scope scope;
  // Current function
  AST::FunctionDeclaration *currentFunc;
  // Declared agents by name
  std::map<std::string, AST::AgentDeclaration *> agents;
  // Declared functions by name
  std::map<std::string, AST::FunctionDeclaration *> funcs;
  // Variable on which member accesses should be collected
  // (for FunctionDeclaration::accessedMembers)
  VarId collectAccessVar;
};

}

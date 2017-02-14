#pragma once

#include <map>
#include <stack>
#include "ASTVisitor.hpp"
#include "Analysis.hpp"

namespace OpenABL {

struct AnalysisVisitor : public AST::Visitor {
  void enter(AST::Var &);
  void enter(AST::Literal &);
  void enter(AST::VarExpression &);
  void enter(AST::UnaryOpExpression &);
  void enter(AST::BinaryOpExpression &);
  void enter(AST::AssignOpExpression &);
  void enter(AST::AssignExpression &);
  void enter(AST::Arg &);
  void enter(AST::CallExpression &);
  void enter(AST::MemberAccessExpression &);
  void enter(AST::TernaryExpression &);
  void enter(AST::NewArrayExpression &);
  void enter(AST::ExpressionStatement &);
  void enter(AST::BlockStatement &);
  void enter(AST::VarDeclarationStatement &);
  void enter(AST::IfStatement &);
  void enter(AST::ForStatement &);
  void enter(AST::ParallelForStatement &);
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
  void leave(AST::AssignOpExpression &);
  void leave(AST::AssignExpression &);
  void leave(AST::Arg &);
  void leave(AST::CallExpression &);
  void leave(AST::MemberAccessExpression &);
  void leave(AST::TernaryExpression &);
  void leave(AST::NewArrayExpression &);
  void leave(AST::ExpressionStatement &);
  void leave(AST::BlockStatement &);
  void leave(AST::VarDeclarationStatement &);
  void leave(AST::IfStatement &);
  void leave(AST::ForStatement &);
  void leave(AST::ParallelForStatement &);
  void leave(AST::SimpleType &);
  void leave(AST::ArrayType &);
  void leave(AST::Param &);
  void leave(AST::FunctionDeclaration &);
  void leave(AST::AgentMember &);
  void leave(AST::AgentDeclaration &);
  void leave(AST::ConstDeclaration &);
  void leave(AST::Script &);

private:
  VarId declareVar(std::string, Type);
  void pushVarScope();
  void popVarScope();
  Type resolveAstType(AST::Type &);

  using VarMap = std::map<std::string, VarId>;
  // Currently *visible* variables
  VarMap varMap;
  // Stack of previous visible variable scopes
  // This is inefficient, but we don't actually care...
  std::stack<VarMap> varMapStack;
  // Information about *all* variables, indexed by unique VarId's
  Scope scope;

  // Declared agents by name
  std::map<std::string, AST::AgentDeclaration *> agents;
  // Declared functions by name
  std::map<std::string, AST::FunctionDeclaration *> funcs;
};

}

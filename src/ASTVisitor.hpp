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
  virtual void enter(AssignOpExpression &) {};
  virtual void enter(AssignExpression &) {};
  virtual void enter(Arg &) {};
  virtual void enter(CallExpression &) {};
  virtual void enter(MemberAccessExpression &) {};
  virtual void enter(TernaryExpression &) {};
  virtual void enter(ExpressionStatement &) {};
  virtual void enter(BlockStatement &) {};
  virtual void enter(VarDeclarationStatement &) {};
  virtual void enter(IfStatement &) {};
  virtual void enter(ForStatement &) {};
  virtual void enter(Type &) {};
  virtual void enter(Param &) {};
  virtual void enter(FunctionDeclaration &) {};
  virtual void enter(AgentMember &) {};
  virtual void enter(AgentDeclaration &) {};
  virtual void enter(ConstDeclaration &) {};
  virtual void enter(Script &) {};

  virtual void leave(Var &) {};
  virtual void leave(Literal &) {};
  virtual void leave(VarExpression &) {};
  virtual void leave(UnaryOpExpression &) {};
  virtual void leave(BinaryOpExpression &) {};
  virtual void leave(AssignOpExpression &) {};
  virtual void leave(AssignExpression &) {};
  virtual void leave(Arg &) {};
  virtual void leave(CallExpression &) {};
  virtual void leave(MemberAccessExpression &) {};
  virtual void leave(TernaryExpression &) {};
  virtual void leave(ExpressionStatement &) {};
  virtual void leave(BlockStatement &) {};
  virtual void leave(VarDeclarationStatement &) {};
  virtual void leave(IfStatement &) {};
  virtual void leave(ForStatement &) {};
  virtual void leave(Type &) {};
  virtual void leave(Param &) {};
  virtual void leave(FunctionDeclaration &) {};
  virtual void leave(AgentMember &) {};
  virtual void leave(AgentDeclaration &) {};
  virtual void leave(ConstDeclaration &) {};
  virtual void leave(Script &) {};
};

}
}

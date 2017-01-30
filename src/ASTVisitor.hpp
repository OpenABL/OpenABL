#pragma once

#include "AST.hpp"

namespace OpenABL {
namespace AST {

struct Visitor {
  virtual void enter(Literal &node) {};
  virtual void enter(VarExpression &node) {};
  virtual void enter(UnaryOpExpression &node) {};
  virtual void enter(BinaryOpExpression &node) {};
  virtual void enter(AssignOpExpression &node) {};
  virtual void enter(AssignExpression &node) {};
  virtual void enter(Arg &node) {};
  virtual void enter(CallExpression &node) {};
  virtual void enter(MemberAccessExpression &node) {};
  virtual void enter(TernaryExpression &node) {};
  virtual void enter(ExpressionStatement &node) {};
  virtual void enter(BlockStatement &node) {};
  virtual void enter(VarDeclarationStatement &node) {};
  virtual void enter(IfStatement &node) {};
  virtual void enter(ForStatement &node) {};
  virtual void enter(Type &node) {};
  virtual void enter(Param &node) {};
  virtual void enter(FunctionDeclaration &node) {};
  virtual void enter(AgentMember &node) {};
  virtual void enter(AgentDeclaration &node) {};
  virtual void enter(ConstDeclaration &node) {};
  virtual void enter(Script &node) {};

  virtual void leave(Literal &node) {};
  virtual void leave(VarExpression &node) {};
  virtual void leave(UnaryOpExpression &node) {};
  virtual void leave(BinaryOpExpression &node) {};
  virtual void leave(AssignOpExpression &node) {};
  virtual void leave(AssignExpression &node) {};
  virtual void leave(Arg &node) {};
  virtual void leave(CallExpression &node) {};
  virtual void leave(MemberAccessExpression &node) {};
  virtual void leave(TernaryExpression &node) {};
  virtual void leave(ExpressionStatement &node) {};
  virtual void leave(BlockStatement &node) {};
  virtual void leave(VarDeclarationStatement &node) {};
  virtual void leave(IfStatement &node) {};
  virtual void leave(ForStatement &node) {};
  virtual void leave(Type &node) {};
  virtual void leave(Param &node) {};
  virtual void leave(FunctionDeclaration &node) {};
  virtual void leave(AgentMember &node) {};
  virtual void leave(AgentDeclaration &node) {};
  virtual void leave(ConstDeclaration &node) {};
  virtual void leave(Script &node) {};
};

}
}

#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"

namespace OpenABL {

struct CPrinter : public GenericPrinter {
  using GenericPrinter::print;

  CPrinter(AST::Script &script, bool useFloat)
    : GenericPrinter(script, false), script(script), useFloat(useFloat) {}

  void print(const AST::UnaryOpExpression &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::Script &);

  void printType(Type t);

  virtual void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
  virtual bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);

private:
  AST::Script &script;
  bool useFloat;
};

}

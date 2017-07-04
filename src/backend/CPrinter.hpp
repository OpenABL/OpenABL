#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"

namespace OpenABL {

struct CPrinter : public GenericPrinter {
  using GenericPrinter::print;

  CPrinter(AST::Script &script)
    : GenericPrinter(script), script(script) {}

  void print(const AST::UnaryOpExpression &);
  void print(const AST::BinaryOpExpression &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::AssignOpStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::SimpleType &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::Script &);

private:
  AST::Script &script;
};

}

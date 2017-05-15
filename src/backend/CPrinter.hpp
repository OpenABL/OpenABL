#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"

namespace OpenABL {

struct CPrinter : public GenericPrinter {
  using GenericPrinter::print;

  CPrinter(AST::Script &script)
    : GenericPrinter(script), script(script) {}

  void print(AST::UnaryOpExpression &);
  void print(AST::BinaryOpExpression &);
  void print(AST::CallExpression &);
  void print(AST::MemberInitEntry &);
  void print(AST::AgentCreationExpression &);
  void print(AST::NewArrayExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::AssignStatement &);
  void print(AST::AssignOpStatement &);
  void print(AST::VarDeclarationStatement &);
  void print(AST::ForStatement &);
  void print(AST::SimulateStatement &);
  void print(AST::SimpleType &);
  void print(AST::ArrayType &);
  void print(AST::AgentMember &);
  void print(AST::AgentDeclaration &);
  void print(AST::ConstDeclaration &);
  void print(AST::Script &);

private:
  AST::Script &script;
};

}

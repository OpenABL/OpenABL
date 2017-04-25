#pragma once

#include "AST.hpp"
#include "GenericCPrinter.hpp"

namespace OpenABL {

struct CPrinter : public GenericCPrinter {
  using GenericCPrinter::print;

  CPrinter(AST::Script &script)
    : GenericCPrinter(script), script(script) {}

  void print(AST::UnaryOpExpression &);
  void print(AST::BinaryOpExpression &);
  void print(AST::AssignExpression &);
  void print(AST::CallExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::TernaryExpression &);
  void print(AST::MemberInitEntry &);
  void print(AST::AgentCreationExpression &);
  void print(AST::NewArrayExpression &);
  void print(AST::AssignOpStatement &);
  void print(AST::VarDeclarationStatement &);
  void print(AST::ForStatement &);
  void print(AST::SimulateStatement &);
  void print(AST::ReturnStatement &);
  void print(AST::SimpleType &);
  void print(AST::ArrayType &);
  void print(AST::Param &);
  void print(AST::FunctionDeclaration &);
  void print(AST::AgentMember &);
  void print(AST::AgentDeclaration &);
  void print(AST::ConstDeclaration &);
  void print(AST::Script &);

private:
  AST::Script &script;
};

}

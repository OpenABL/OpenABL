#pragma once
#include "GenericPrinter.hpp"

namespace OpenABL {

struct MasonPrinter : public GenericPrinter {
  using GenericPrinter::print;

  MasonPrinter(AST::Script &script)
    : GenericPrinter(script), script(script) {}

  void print(AST::UnaryOpExpression &);
  void print(AST::BinaryOpExpression &);
  void print(AST::Arg &);
  void print(AST::CallExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::MemberInitEntry &);
  void print(AST::AgentCreationExpression &);
  void print(AST::NewArrayExpression &);
  void print(AST::ExpressionStatement &);
  void print(AST::AssignOpStatement &);
  void print(AST::VarDeclarationStatement &);
  void print(AST::ForStatement &);
  void print(AST::SimulateStatement &);
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

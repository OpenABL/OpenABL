#pragma once
#include "GenericPrinter.hpp"

namespace OpenABL {

struct MasonPrinter : public GenericPrinter {
  using GenericPrinter::print;

  MasonPrinter(AST::Script &script)
    : GenericPrinter(script), script(script) {}

  void print(const AST::UnaryOpExpression &);
  void print(const AST::BinaryOpExpression &);
  void print(const AST::Arg &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::ExpressionStatement &);
  void print(const AST::AssignOpStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::SimpleType &);
  void print(const AST::ArrayType &);
  void print(const AST::Param &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::Script &);

private:
  AST::Script &script;
};

}

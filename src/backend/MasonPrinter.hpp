#pragma once
#include "Printer.hpp"

namespace OpenABL {

struct MasonPrinter : Printer {
  MasonPrinter(AST::Script &script)
    : script(script) {}

  void print(AST::Var &);
  void print(AST::Literal &);
  void print(AST::VarExpression &);
  void print(AST::UnaryOpExpression &);
  void print(AST::BinaryOpExpression &);
  void print(AST::Arg &);
  void print(AST::CallExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::TernaryExpression &);
  void print(AST::MemberInitEntry &);
  void print(AST::AgentCreationExpression &);
  void print(AST::NewArrayExpression &);
  void print(AST::ExpressionStatement &);
  void print(AST::AssignStatement &);
  void print(AST::AssignOpStatement &);
  void print(AST::BlockStatement &);
  void print(AST::VarDeclarationStatement &);
  void print(AST::IfStatement &);
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

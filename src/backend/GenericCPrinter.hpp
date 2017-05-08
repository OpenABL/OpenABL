#pragma once

#include "AST.hpp"
#include "Printer.hpp"

namespace OpenABL {

struct GenericCPrinter : public Printer {
  GenericCPrinter(AST::Script &script) : script(script) {}

  virtual void print(AST::Var &);
  virtual void print(AST::Literal &);
  virtual void print(AST::VarExpression &);
  virtual void print(AST::UnaryOpExpression &);
  virtual void print(AST::BinaryOpExpression &);
  virtual void print(AST::TernaryExpression &);
  virtual void print(AST::MemberAccessExpression &);
  virtual void print(AST::Arg &);
  virtual void print(AST::ExpressionStatement &);
  virtual void print(AST::AssignStatement &);
  virtual void print(AST::AssignOpStatement &);
  virtual void print(AST::BlockStatement &);
  virtual void print(AST::IfStatement &);
  virtual void print(AST::VarDeclarationStatement &);
  virtual void print(AST::ReturnStatement &);
  virtual void print(AST::ConstDeclaration &);
  virtual void print(AST::Param &);
  virtual void print(AST::FunctionDeclaration &);

  void printArgs(AST::CallExpression &);

private:
  AST::Script &script;
};

}

#pragma once

#include "AST.hpp"
#include "Printer.hpp"

namespace OpenABL {

/* This printer provides printing implementations for parts
 * that are likely to be the same for many backends. Of course,
 * it is possible to overwrite the default implementations with
 * more specific ones. */
struct GenericPrinter : public Printer {
  GenericPrinter(AST::Script &script, bool supportsOverloads)
    : script(script), supportsOverloads(supportsOverloads) {}

  virtual void print(const AST::SimpleType &);
  virtual void print(const AST::Var &);
  virtual void print(const AST::Literal &);
  virtual void print(const AST::VarExpression &);
  virtual void print(const AST::UnaryOpExpression &);
  virtual void print(const AST::BinaryOpExpression &);
  virtual void print(const AST::TernaryExpression &);
  virtual void print(const AST::MemberAccessExpression &);
  virtual void print(const AST::ArrayAccessExpression &);
  virtual void print(const AST::ArrayInitExpression &);
  virtual void print(const AST::ExpressionStatement &);
  virtual void print(const AST::AssignStatement &);
  virtual void print(const AST::AssignOpStatement &);
  virtual void print(const AST::BlockStatement &);
  virtual void print(const AST::IfStatement &);
  virtual void print(const AST::WhileStatement &);
  virtual void print(const AST::VarDeclarationStatement &);
  virtual void print(const AST::ReturnStatement &);
  virtual void print(const AST::BreakStatement &);
  virtual void print(const AST::ContinueStatement &);
  virtual void print(const AST::ConstDeclaration &);
  virtual void print(const AST::Param &);
  virtual void print(const AST::FunctionDeclaration &);

  virtual void printType(Type t) = 0;

  virtual void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &) {
    assert(0);
  }
  virtual bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &) {
    return false;
  }

  void printArgs(const AST::CallExpression &);
  void printParams(const AST::FunctionDeclaration &);

  virtual void print(const AST::EnvironmentDeclaration &) {
    // Often not used explicitly
    assert(0);
  }

protected:
  AST::Script &script;
  bool supportsOverloads;
};

}

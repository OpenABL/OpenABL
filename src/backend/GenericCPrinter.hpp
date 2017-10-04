#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"

namespace OpenABL {

struct GenericCPrinter : public GenericPrinter {
  using GenericPrinter::print;

  GenericCPrinter(AST::Script &script)
    : GenericPrinter(script, false) {}

  virtual void print(const AST::UnaryOpExpression &);
  virtual void print(const AST::ConstDeclaration &);

  virtual void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
  virtual bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
};

}

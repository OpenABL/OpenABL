#pragma once

#include "AST.hpp"
#include "Printer.hpp"

namespace OpenABL {

struct GenericCPrinter : public Printer {
  GenericCPrinter(AST::Script &script) : script(script) {}

  virtual void print(AST::ExpressionStatement &);

private:
  AST::Script &script;
};

}

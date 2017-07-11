#pragma once
#include "MasonPrinter.hpp"

namespace OpenABL {

struct DMasonPrinter : public MasonPrinter {
  using MasonPrinter::print;

  DMasonPrinter(AST::Script &script)
    : MasonPrinter(script) {}

  // TODO Overridden methods are declared here
  void print(const AST::AgentDeclaration &);
  void printStubAgent(const AST::AgentDeclaration &);
  void printLocalTestCode();
  void print(const AST::Script &);
  void print(const AST::CallExpression &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentCreationExpression &);
};

}

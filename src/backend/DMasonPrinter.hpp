#pragma once

#include "Mason2Printer.hpp"

namespace OpenABL {

struct DMasonPrinter : public Mason2Printer {
  using Mason2Printer::print;

  DMasonPrinter(AST::Script &script)
    : Mason2Printer(script) {}

  void printStubAgent(const AST::AgentDeclaration &);
  void printLocalTestCode();
  void print(const AST::Script &);
  void print(const AST::CallExpression &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::SimulateStatement &);

  void printAgentImports();
  void printAgentExtends(const AST::AgentDeclaration &);
  void printAgentExtraCtorArgs();
  void printAgentExtraCtorCode();
  void printStepDefaultCode(const AST::FunctionDeclaration &);
};

}

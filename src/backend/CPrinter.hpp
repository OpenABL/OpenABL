#pragma once

#include "AST.hpp"
#include "GenericCPrinter.hpp"

namespace OpenABL {

struct CPrinter : public GenericCPrinter {
  using GenericCPrinter::print;

  CPrinter(AST::Script &script, bool useFloat)
    : GenericCPrinter(script), script(script), useFloat(useFloat) {}

  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::Script &);

  void printType(Type t);

private:
  AST::Script &script;
  bool useFloat;
};

}

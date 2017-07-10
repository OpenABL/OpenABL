#pragma once
#include "GenericPrinter.hpp"

namespace OpenABL {

struct MasonPrinter : public GenericPrinter {
  using GenericPrinter::print;

  MasonPrinter(AST::Script &script)
    : GenericPrinter(script) {}

  void print(const AST::VarExpression &);
  void print(const AST::UnaryOpExpression &);
  void print(const AST::BinaryOpExpression &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::AssignOpStatement &);
  void print(const AST::VarDeclarationStatement &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::SimpleType &);
  void print(const AST::FunctionDeclaration &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::Script &);

protected:
  const char *getSimVarName() const {
    return inAgent ? "_sim" : "this";
  }

  // Current input and output variables inside a step function
  VarId currentInVar;
  VarId currentOutVar;
  // Whether we're in agent code or in main simulation code
  bool inAgent;
};

}

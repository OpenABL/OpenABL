#pragma once

#include "AST.hpp"
#include "GenericCPrinter.hpp"
#include "FlameModel.hpp"

namespace OpenABL {

struct FlamePrinter : public GenericCPrinter {
  using GenericCPrinter::print;

  FlamePrinter(AST::Script &script, const FlameModel &model, bool useFloat)
    : GenericCPrinter(script), script(script), model(model), useFloat(useFloat) {}

  void print(const AST::AssignStatement &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::Script &);

  void printType(Type t);

private:
  AST::Script &script;
  const FlameModel &model;
  bool useFloat;

  const FlameModel::Func *currentFunc = nullptr;
  const AST::Var *currentNearVar = nullptr;
};

}

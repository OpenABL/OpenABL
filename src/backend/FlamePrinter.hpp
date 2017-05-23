#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"
#include "FlameModel.hpp"

namespace OpenABL {

struct FlamePrinter : public GenericPrinter {
  using GenericPrinter::print;

  FlamePrinter(AST::Script &script, const FlameModel &model)
    : GenericPrinter(script), script(script), model(model) {}

  void print(const AST::BinaryOpExpression &);
  void print(const AST::UnaryOpExpression &);
  void print(const AST::AssignStatement &);
  void print(const AST::AssignOpStatement &);
  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::SimpleType &);
  void print(const AST::ArrayType &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::Script &);

private:
  AST::Script &script;
  const FlameModel &model;
  const FlameModel::Func *currentFunc = nullptr;
};

}

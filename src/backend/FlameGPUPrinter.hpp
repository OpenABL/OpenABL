#pragma once

#include "AST.hpp"
#include "GenericPrinter.hpp"
#include "FlameModel.hpp"

namespace OpenABL {

struct FlameGPUPrinter : public GenericPrinter {
  using GenericPrinter::print;

  FlameGPUPrinter(AST::Script &script, const FlameModel &model)
    : GenericPrinter(script), script(script), model(model) {}

  void print(const AST::CallExpression &);
  void print(const AST::MemberInitEntry &);
  void print(const AST::AgentCreationExpression &);
  void print(const AST::NewArrayExpression &);
  void print(const AST::MemberAccessExpression &);
  void print(const AST::ForStatement &);
  void print(const AST::SimulateStatement &);
  void print(const AST::SimpleType &);
  void print(const AST::AgentMember &);
  void print(const AST::AgentDeclaration &);
  void print(const AST::ConstDeclaration &);
  void print(const AST::Script &);

private:
  AST::Script &script;
  const FlameModel &model;
  const FlameModel::Func *currentFunc = nullptr;
};

}

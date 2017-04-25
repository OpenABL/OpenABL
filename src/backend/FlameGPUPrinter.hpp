#pragma once

#include "AST.hpp"
#include "GenericCPrinter.hpp"
#include "FlameModel.hpp"

namespace OpenABL {

struct FlameGPUPrinter : public GenericCPrinter {
  using GenericCPrinter::print;

  FlameGPUPrinter(AST::Script &script, const FlameModel &model)
    : GenericCPrinter(script), script(script), model(model) {}

  void print(AST::CallExpression &);
  void print(AST::MemberInitEntry &);
  void print(AST::AgentCreationExpression &);
  void print(AST::NewArrayExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::ForStatement &);
  void print(AST::SimulateStatement &);
  void print(AST::SimpleType &);
  void print(AST::ArrayType &);
  void print(AST::AgentMember &);
  void print(AST::AgentDeclaration &);
  void print(AST::ConstDeclaration &);
  void print(AST::Script &);

private:
  AST::Script &script;
  const FlameModel &model;
  const FlameModel::Func *currentFunc = nullptr;
};

}

#include "AnalysisVisitor.hpp"

namespace OpenABL {

// TODO move?
uint32_t VarId::max_id;

void AnalysisVisitor::enter(AST::VarExpression &expr) {
  AST::Var &var = *expr.var;
  auto it = varMap.find(var.name);
  if (it == varMap.end()) {
    std::cout << "Use of undeclared variable " << var.name << std::endl;
    return;
  }

  var.id = it->second;
}

void AnalysisVisitor::enter(AST::VarDeclarationStatement &decl) {
  AST::Var &var = *decl.var;
  auto it = varMap.find(var.name);
  if (it != varMap.end()) {
    std::cout << "Cannot redeclare " << var.name << std::endl;
  }

  var.id = VarId::make();
  varMap.insert({ var.name, var.id });
}

}

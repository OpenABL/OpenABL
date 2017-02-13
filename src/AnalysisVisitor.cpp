#include "AnalysisVisitor.hpp"

namespace OpenABL {

// TODO move?
uint32_t VarId::max_id;

VarId AnalysisVisitor::declareVar(std::string name) {
  auto it = varMap.find(name);
  if (it != varMap.end()) {
    std::cout << "Cannot redeclare " << name << std::endl;
  }

  VarId id = VarId::make();
  varMap.insert({ name, id });
  return id;
}

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
  decl.var->id = declareVar(decl.var->name);
}

void AnalysisVisitor::enter(AST::Param &param) {
  param.var->id = declareVar(param.var->name);
  if (param.outVar) {
    param.outVar->id = declareVar(param.outVar->name);
  }
}

}

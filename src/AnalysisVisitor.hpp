#pragma once

#include <map>
#include "ASTVisitor.hpp"
#include "Scope.hpp"

namespace OpenABL {

struct AnalysisVisitor : public AST::Visitor {
  void enter(AST::VarExpression &);
  void enter(AST::VarDeclarationStatement &);
  void enter(AST::Param &);

private:
  VarId declareVar(std::string);

  std::map<std::string, VarId> varMap;
  Scope scope;
};

}

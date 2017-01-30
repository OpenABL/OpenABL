#pragma once

#include <map>
#include "ASTVisitor.hpp"
#include "Scope.hpp"

namespace OpenABL {

struct AnalysisVisitor : public AST::Visitor {
  void enter(AST::VarExpression &);
  void enter(AST::VarDeclarationStatement &);

private:
  std::map<std::string, VarId> varMap;
  Scope scope;
};

}

/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

#include "AST.hpp"
#include "ASTVisitor.hpp"
#include "Printer.hpp"

#define VISIT_EXPR(expr) do { \
  bool oldInExpr = visitor.inExpr; \
  visitor.inExpr = true; \
  expr->accept(visitor); \
  visitor.inExpr = oldInExpr; \
  if (visitor.replacementExpr != nullptr) { \
    expr.reset(visitor.replacementExpr); \
    visitor.replacementExpr = nullptr; \
  } \
} while (0)

namespace OpenABL {
namespace AST {

void Var::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void Literal::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void VarExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  var->accept(visitor);
  visitor.leave(*this);
}

void UnaryOpExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(expr);
  visitor.leave(*this);
}

void BinaryOpExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(left);
  VISIT_EXPR(right);
  visitor.leave(*this);
}

void CallExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (ExpressionPtr &arg : *args) {
    VISIT_EXPR(arg);
  }
  visitor.leave(*this);
}

void MemberAccessExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(expr);
  visitor.leave(*this);
}

void ArrayAccessExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(arrayExpr);
  VISIT_EXPR(offsetExpr);
  visitor.leave(*this);
}

void TernaryExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(condExpr);
  VISIT_EXPR(ifExpr);
  VISIT_EXPR(elseExpr);
  visitor.leave(*this);
}

void MemberInitEntry::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(expr);
  visitor.leave(*this);
}

void AgentCreationExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (MemberInitEntryPtr &member : *members) {
    member->accept(visitor);
  }
  visitor.leave(*this);
}

void ArrayInitExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (ExpressionPtr &expr : *exprs) {
    VISIT_EXPR(expr);
  }
  visitor.leave(*this);
}

void NewArrayExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  elemType->accept(visitor);
  VISIT_EXPR(sizeExpr);
  visitor.leave(*this);
}

void ExpressionStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(expr);
  visitor.leave(*this);
}

void AssignStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(left);
  VISIT_EXPR(right);
  visitor.leave(*this);
}

void AssignOpStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(left);
  VISIT_EXPR(right);
  visitor.leave(*this);
}

void BlockStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (StatementPtr &stmt : *stmts) {
    stmt->accept(visitor);
  }
  visitor.leave(*this);
}

void VarDeclarationStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  if (initializer) {
    VISIT_EXPR(initializer);
  }
  visitor.leave(*this);
}

void IfStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(condExpr);
  ifStmt->accept(visitor);
  if (elseStmt) {
    elseStmt->accept(visitor);
  }
  visitor.leave(*this);
}

void WhileStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(expr);
  stmt->accept(visitor);
  visitor.leave(*this);
}

void ForStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  VISIT_EXPR(expr);
  stmt->accept(visitor);
  visitor.leave(*this);
}

void SimulateStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  VISIT_EXPR(timestepsExpr);
  visitor.leave(*this);
}

void ReturnStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  if (expr) {
    VISIT_EXPR(expr);
  }
  visitor.leave(*this);
}

void BreakStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void ContinueStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void SimpleType::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void Param::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  if (outVar) {
    outVar->accept(visitor);
  }
  visitor.leave(*this);
}

void FunctionDeclaration::accept(Visitor &visitor) {
  visitor.enter(*this);
  returnType->accept(visitor);
  for (ParamPtr &param : *params) {
    param->accept(visitor);
  }
  for (StatementPtr &stmt : *stmts) {
    stmt->accept(visitor);
  }
  visitor.leave(*this);
}

void AgentMember::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  visitor.leave(*this);
}

void AgentDeclaration::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (AgentMemberPtr &member : *members) {
    member->accept(visitor);
  }
  visitor.leave(*this);
}

void ConstDeclaration::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  VISIT_EXPR(expr);
  visitor.leave(*this);
}

void EnvironmentDeclaration::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (MemberInitEntryPtr &member : *members) {
    member->accept(visitor);
  }
  visitor.leave(*this);
}

void Script::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (DeclarationPtr &decl : *decls) {
    decl->accept(visitor);
  }
  visitor.leave(*this);
}

void Var::print(Printer &printer) const { printer.print(*this); }
void Literal::print(Printer &printer) const { printer.print(*this); }
void VarExpression::print(Printer &printer) const { printer.print(*this); }
void UnaryOpExpression::print(Printer &printer) const { printer.print(*this); }
void BinaryOpExpression::print(Printer &printer) const { printer.print(*this); }
void CallExpression::print(Printer &printer) const { printer.print(*this); }
void MemberAccessExpression::print(Printer &printer) const { printer.print(*this); }
void ArrayAccessExpression::print(Printer &printer) const { printer.print(*this); }
void TernaryExpression::print(Printer &printer) const { printer.print(*this); }
void MemberInitEntry::print(Printer &printer) const { printer.print(*this); }
void AgentCreationExpression::print(Printer &printer) const { printer.print(*this); }
void ArrayInitExpression::print(Printer &printer) const { printer.print(*this); }
void NewArrayExpression::print(Printer &printer) const { printer.print(*this); }
void ExpressionStatement::print(Printer &printer) const { printer.print(*this); }
void AssignStatement::print(Printer &printer) const { printer.print(*this); }
void AssignOpStatement::print(Printer &printer) const { printer.print(*this); }
void BlockStatement::print(Printer &printer) const { printer.print(*this); }
void VarDeclarationStatement::print(Printer &printer) const { printer.print(*this); }
void IfStatement::print(Printer &printer) const { printer.print(*this); }
void WhileStatement::print(Printer &printer) const { printer.print(*this); }
void ForStatement::print(Printer &printer) const { printer.print(*this); }
void SimulateStatement::print(Printer &printer) const { printer.print(*this); }
void ReturnStatement::print(Printer &printer) const { printer.print(*this); }
void BreakStatement::print(Printer &printer) const { printer.print(*this); }
void ContinueStatement::print(Printer &printer) const { printer.print(*this); }
void SimpleType::print(Printer &printer) const { printer.print(*this); }
void Param::print(Printer &printer) const { printer.print(*this); }
void FunctionDeclaration::print(Printer &printer) const { printer.print(*this); }
void AgentMember::print(Printer &printer) const { printer.print(*this); }
void AgentDeclaration::print(Printer &printer) const { printer.print(*this); }
void ConstDeclaration::print(Printer &printer) const { printer.print(*this); }
void EnvironmentDeclaration::print(Printer &printer) const { printer.print(*this); }
void Script::print(Printer &printer) const { printer.print(*this); }

}
}

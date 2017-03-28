#include "AST.hpp"
#include "ASTVisitor.hpp"
#include "Printer.hpp"

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
  expr->accept(visitor);
  visitor.leave(*this);
}

void BinaryOpExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  left->accept(visitor);
  right->accept(visitor);
  visitor.leave(*this);
}

void AssignOpExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  left->accept(visitor);
  right->accept(visitor);
  visitor.leave(*this);
}

void AssignExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  left->accept(visitor);
  right->accept(visitor);
  visitor.leave(*this);
}

void Arg::accept(Visitor &visitor) {
  visitor.enter(*this);
  expr->accept(visitor);
  if (outExpr) {
    outExpr->accept(visitor);
  }
  visitor.leave(*this);
}

void CallExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (ArgPtr &arg : *args) {
    arg->accept(visitor);
  }
  visitor.leave(*this);
}

void MemberAccessExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  expr->accept(visitor);
  visitor.leave(*this);
}

void TernaryExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  condExpr->accept(visitor);
  ifExpr->accept(visitor);
  elseExpr->accept(visitor);
  visitor.leave(*this);
}

void NewArrayExpression::accept(Visitor &visitor) {
  visitor.enter(*this);
  elemType->accept(visitor);
  sizeExpr->accept(visitor);
  visitor.leave(*this);
}

void ExpressionStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  expr->accept(visitor);
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
    initializer->accept(visitor);
  }
  visitor.leave(*this);
}

void IfStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  condExpr->accept(visitor);
  ifStmt->accept(visitor);
  if (elseStmt) {
    elseStmt->accept(visitor);
  }
  visitor.leave(*this);
}

void ForStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  expr->accept(visitor);
  stmt->accept(visitor);
  visitor.leave(*this);
}

void ParallelForStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
  var->accept(visitor);
  outVar->accept(visitor);
  expr->accept(visitor);
  stmt->accept(visitor);
  visitor.leave(*this);
}

void ReturnStatement::accept(Visitor &visitor) {
  visitor.enter(*this);
  expr->accept(visitor);
  visitor.leave(*this);
}

void SimpleType::accept(Visitor &visitor) {
  visitor.enter(*this);
  visitor.leave(*this);
}

void ArrayType::accept(Visitor &visitor) {
  visitor.enter(*this);
  type->accept(visitor);
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
  if (returnType) {
    returnType->accept(visitor);
  }
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
  expr->accept(visitor);
  visitor.leave(*this);
}

void Script::accept(Visitor &visitor) {
  visitor.enter(*this);
  for (DeclarationPtr &decl : *decls) {
    decl->accept(visitor);
  }
  visitor.leave(*this);
}

void Var::print(Printer &printer) { printer.print(*this); }
void Literal::print(Printer &printer) { printer.print(*this); }
void VarExpression::print(Printer &printer) { printer.print(*this); }
void UnaryOpExpression::print(Printer &printer) { printer.print(*this); }
void BinaryOpExpression::print(Printer &printer) { printer.print(*this); }
void AssignOpExpression::print(Printer &printer) { printer.print(*this); }
void AssignExpression::print(Printer &printer) { printer.print(*this); }
void Arg::print(Printer &printer) { printer.print(*this); }
void CallExpression::print(Printer &printer) { printer.print(*this); }
void MemberAccessExpression::print(Printer &printer) { printer.print(*this); }
void TernaryExpression::print(Printer &printer) { printer.print(*this); }
void NewArrayExpression::print(Printer &printer) { printer.print(*this); }
void ExpressionStatement::print(Printer &printer) { printer.print(*this); }
void BlockStatement::print(Printer &printer) { printer.print(*this); }
void VarDeclarationStatement::print(Printer &printer) { printer.print(*this); }
void IfStatement::print(Printer &printer) { printer.print(*this); }
void ForStatement::print(Printer &printer) { printer.print(*this); }
void ParallelForStatement::print(Printer &printer) { printer.print(*this); }
void ReturnStatement::print(Printer &printer) { printer.print(*this); }
void SimpleType::print(Printer &printer) { printer.print(*this); }
void ArrayType::print(Printer &printer) { printer.print(*this); }
void Param::print(Printer &printer) { printer.print(*this); }
void FunctionDeclaration::print(Printer &printer) { printer.print(*this); }
void AgentMember::print(Printer &printer) { printer.print(*this); }
void AgentDeclaration::print(Printer &printer) { printer.print(*this); }
void ConstDeclaration::print(Printer &printer) { printer.print(*this); }
void Script::print(Printer &printer) { printer.print(*this); }

}
}

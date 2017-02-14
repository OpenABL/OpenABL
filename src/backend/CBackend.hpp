#pragma once

#include <string>
#include <AST.hpp>
#include <Printer.hpp>

namespace OpenABL {

struct CPrinter : public Printer {
  void print(AST::Var &);
  void print(AST::Literal &);
  void print(AST::VarExpression &);
  void print(AST::UnaryOpExpression &);
  void print(AST::BinaryOpExpression &);
  void print(AST::AssignOpExpression &);
  void print(AST::AssignExpression &);
  void print(AST::Arg &);
  void print(AST::CallExpression &);
  void print(AST::MemberAccessExpression &);
  void print(AST::TernaryExpression &);
  void print(AST::ExpressionStatement &);
  void print(AST::BlockStatement &);
  void print(AST::VarDeclarationStatement &);
  void print(AST::IfStatement &);
  void print(AST::ForStatement &);
  void print(AST::ParallelForStatement &);
  void print(AST::SimpleType &);
  void print(AST::ArrayType &);
  void print(AST::Param &);
  void print(AST::FunctionDeclaration &);
  void print(AST::AgentMember &);
  void print(AST::AgentDeclaration &);
  void print(AST::ConstDeclaration &);
  void print(AST::Script &);
};

struct CBackend {
  std::string astToCode(AST::Script &script);

private:
  CPrinter printer;
};

std::string CBackend::astToCode(AST::Script &script) {
  script.print(printer);
  return printer.extractStr();
}

void CPrinter::print(AST::Var &var) {
  *this << var.name;
}
void CPrinter::print(AST::Literal &lit) {
  if (AST::IntLiteral *ilit = dynamic_cast<AST::IntLiteral *>(&lit)) {
    *this << ilit->value;
  } else if (AST::FloatLiteral *flit = dynamic_cast<AST::FloatLiteral *>(&lit)) {
    *this << flit->value;
  } else if (AST::BoolLiteral *blit = dynamic_cast<AST::BoolLiteral *>(&lit)) {
    *this << blit->value;
  } else {
    assert(0);
  }
}
void CPrinter::print(AST::VarExpression &expr) {
  *this << *expr.var;
}
void CPrinter::print(AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
}
void CPrinter::print(AST::BinaryOpExpression &expr) {
  Type l = expr.left->type;
  Type r = expr.right->type;
  if (l.isVec() || r.isVec()) {
    Type v = l.isVec() ? l : r;
    *this << (v.getTypeId() == Type::VEC2 ? "vec2_" : "vec3_");
    switch (expr.op) {
      case AST::BinaryOp::ADD: *this << "add"; break;
      case AST::BinaryOp::SUB: *this << "sub"; break;
      case AST::BinaryOp::DIV: *this << "div_scalar"; break;
      case AST::BinaryOp::MUL:
        *this << "mul_scalar";
        if (r.isVec()) {
          // Normalize to right multiplication
          // TODO Move into analysis
          *this << "(" << *expr.right << ", " << *expr.left << ")";
          return;
        }
        break;
      default:
        assert(0);
    }
    *this << "(" << *expr.left << ", " << *expr.right << ")";
    return;
  }

  *this << "(" << *expr.left << " " << AST::getBinaryOpSigil(expr.op)
        << " " << *expr.right << ")";
}
void CPrinter::print(AST::AssignOpExpression &expr) {
  *this << "(" << *expr.left << " " << AST::getBinaryOpSigil(expr.op)
        << "= " << *expr.right << ")";
}
void CPrinter::print(AST::AssignExpression &expr) {
  *this << "(" << *expr.left << " = " << *expr.right << ")";
}
void CPrinter::print(AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}
void CPrinter::print(AST::CallExpression &expr) {
  *this << expr.name << "(";
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) *this << ", ";
    first = false;
    *this << *arg;
  }
  *this << ")";
}
void CPrinter::print(AST::MemberAccessExpression &expr) {
  *this << *expr.expr << "." << expr.member;
}
void CPrinter::print(AST::TernaryExpression &expr) {
  *this << "(" << *expr.condExpr << " ? " << *expr.ifExpr << " : " << *expr.elseExpr << ")";
}
void CPrinter::print(AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}
void CPrinter::print(AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
}
void CPrinter::print(AST::VarDeclarationStatement &stmt) {
  *this << *stmt.type << " " << *stmt.var;
  if (stmt.initializer) {
    *this << " = " << *stmt.initializer;
  }
  *this << ";";
}
void CPrinter::print(AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
}
void CPrinter::print(AST::ForStatement &) {
  // TODO
}
void CPrinter::print(AST::ParallelForStatement &) {
  // TODO
}
void CPrinter::print(AST::SimpleType &type) {
  *this << type.name;
}
void CPrinter::print(AST::ArrayType &type) {
  *this << *type.type << "*";
}
void CPrinter::print(AST::Param &param) {
  *this << *param.type << " " << *param.var;
  if (param.outVar) {
    *this << ", " << *param.type << " " << *param.outVar;
  }
}
void CPrinter::print(AST::FunctionDeclaration &decl) {
  if (decl.returnType) {
    *this << *decl.returnType;
  } else {
    *this << "void";
  }
  *this << " " << decl.name << "(";
  bool first = true;
  for (const AST::ParamPtr &param : *decl.params) {
    if (!first) *this << ", ";
    first = false;
    *this << *param;
  }
  *this << ") {" << indent << *decl.stmts << outdent << nl << "}";
}
void CPrinter::print(AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}
void CPrinter::print(AST::AgentDeclaration &decl) {
  *this << "struct " << decl.name
        << " {" << indent << *decl.members << outdent << nl << "};";
}
void CPrinter::print(AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var << " = " << *decl.value << ";";
}
void CPrinter::print(AST::Script &script) {
  for (const AST::DeclarationPtr &decl : *script.decls) {
    *this << *decl << nl;
  }
}

}

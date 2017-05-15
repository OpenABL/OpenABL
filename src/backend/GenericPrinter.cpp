#include "GenericPrinter.hpp"

namespace OpenABL {

static void printStringLiteral(Printer &p, const std::string &str) {
  p << '"';
  for (char c : str) {
    if (c == '"' || c == '\\') {
      p << '\\' << c;
    } else {
      p << c;
    }
  }
  p << '"';
}

void GenericPrinter::print(AST::Var &var) {
  *this << var.name;
}
void GenericPrinter::print(AST::Literal &lit) {
  if (AST::IntLiteral *ilit = dynamic_cast<AST::IntLiteral *>(&lit)) {
    *this << ilit->value;
  } else if (AST::FloatLiteral *flit = dynamic_cast<AST::FloatLiteral *>(&lit)) {
    *this << flit->value;
  } else if (AST::BoolLiteral *blit = dynamic_cast<AST::BoolLiteral *>(&lit)) {
    *this << blit->value;
  } else if (AST::StringLiteral *slit = dynamic_cast<AST::StringLiteral *>(&lit)) {
    printStringLiteral(*this, slit->value);
  } else {
    assert(0);
  }
}

void GenericPrinter::print(AST::VarExpression &expr) {
  *this << *expr.var;
}
void GenericPrinter::print(AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
}
void GenericPrinter::print(AST::BinaryOpExpression &expr) {
  *this << "(" << *expr.left << " "
        << AST::getBinaryOpSigil(expr.op) << " " << *expr.right << ")";
}
void GenericPrinter::print(AST::TernaryExpression &expr) {
  *this << "(" << *expr.condExpr << " ? " << *expr.ifExpr << " : " << *expr.elseExpr << ")";
}
void GenericPrinter::print(AST::MemberAccessExpression &expr) {
  *this << *expr.expr << "." << expr.member;
}

void GenericPrinter::print(AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}

void GenericPrinter::printArgs(AST::CallExpression &expr) {
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) *this << ", ";
    first = false;
    *this << *arg;
  }
}

void GenericPrinter::print(AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}
void GenericPrinter::print(AST::AssignStatement &expr) {
  *this << *expr.left << " = " << *expr.right << ";";
}
void GenericPrinter::print(AST::AssignOpStatement &stmt) {
  *this << *stmt.left << " " << AST::getBinaryOpSigil(stmt.op)
        << "= " << *stmt.right << ";";
}

void GenericPrinter::print(AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
}
void GenericPrinter::print(AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
}

void GenericPrinter::print(AST::VarDeclarationStatement &stmt) {
    *this << *stmt.type << " " << *stmt.var;
    if (stmt.initializer) {
      *this << " = " << *stmt.initializer;
    }
    *this << ";";
}
void GenericPrinter::print(AST::ReturnStatement &stmt) {
  if (stmt.expr) {
    *this << "return " << *stmt.expr << ";";
  } else {
    *this << "return;";
  }
}
void GenericPrinter::print(AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var << " = " << *decl.expr << ";";
}

void GenericPrinter::print(AST::Param &param) {
  *this << *param.type << " " << *param.var;
  if (param.outVar) {
    *this << ", " << *param.type << " " << *param.outVar;
  }
}
void GenericPrinter::print(AST::FunctionDeclaration &decl) {
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
  *this << ") {" << indent;
  *this << *decl.stmts << outdent << nl << "}";
}

}

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

void GenericPrinter::print(const AST::Var &var) {
  *this << var.name;
}
void GenericPrinter::print(const AST::Literal &lit) {
  if (const AST::IntLiteral *ilit = dynamic_cast<const AST::IntLiteral *>(&lit)) {
    *this << ilit->value;
  } else if (const AST::FloatLiteral *flit = dynamic_cast<const AST::FloatLiteral *>(&lit)) {
    *this << flit->value;
  } else if (const AST::BoolLiteral *blit = dynamic_cast<const AST::BoolLiteral *>(&lit)) {
    *this << blit->value;
  } else if (const AST::StringLiteral *slit = dynamic_cast<const AST::StringLiteral *>(&lit)) {
    printStringLiteral(*this, slit->value);
  } else {
    assert(0);
  }
}

void GenericPrinter::print(const AST::VarExpression &expr) {
  *this << *expr.var;
}
void GenericPrinter::print(const AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
}
void GenericPrinter::print(const AST::BinaryOpExpression &expr) {
  *this << "(" << *expr.left << " "
        << AST::getBinaryOpSigil(expr.op) << " " << *expr.right << ")";
}
void GenericPrinter::print(const AST::TernaryExpression &expr) {
  *this << "(" << *expr.condExpr << " ? " << *expr.ifExpr << " : " << *expr.elseExpr << ")";
}
void GenericPrinter::print(const AST::MemberAccessExpression &expr) {
  *this << *expr.expr << "." << expr.member;
}

void GenericPrinter::print(const AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}

void GenericPrinter::printArgs(const AST::CallExpression &expr) {
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) *this << ", ";
    first = false;
    *this << *arg;
  }
}

void GenericPrinter::print(const AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}
void GenericPrinter::print(const AST::AssignStatement &expr) {
  *this << *expr.left << " = " << *expr.right << ";";
}
void GenericPrinter::print(const AST::AssignOpStatement &stmt) {
  *this << *stmt.left << " " << AST::getBinaryOpSigil(stmt.op)
        << "= " << *stmt.right << ";";
}

void GenericPrinter::print(const AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
}
void GenericPrinter::print(const AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
}

void GenericPrinter::print(const AST::VarDeclarationStatement &stmt) {
    *this << *stmt.type << " " << *stmt.var;
    if (stmt.initializer) {
      *this << " = " << *stmt.initializer;
    }
    *this << ";";
}
void GenericPrinter::print(const AST::ReturnStatement &stmt) {
  if (stmt.expr) {
    *this << "return " << *stmt.expr << ";";
  } else {
    *this << "return;";
  }
}
void GenericPrinter::print(const AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var << " = " << *decl.expr << ";";
}

void GenericPrinter::print(const AST::Param &param) {
  *this << *param.type << " " << *param.var;
  if (param.outVar) {
    *this << ", " << *param.type << " " << *param.outVar;
  }
}

void GenericPrinter::printParams(const AST::FunctionDeclaration &decl) {
  bool first = true;
  for (const AST::ParamPtr &param : *decl.params) {
    if (!first) *this << ", ";
    first = false;
    *this << *param;
  }
}

void GenericPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.returnType) {
    *this << *decl.returnType;
  } else {
    *this << "void";
  }
  *this << " " << decl.name << "(";
  printParams(decl);
  *this << ") {" << indent;
  *this << *decl.stmts << outdent << nl << "}";
}

}

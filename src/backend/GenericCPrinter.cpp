#include "GenericCPrinter.hpp"

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

void GenericCPrinter::print(AST::Var &var) {
  *this << var.name;
}
void GenericCPrinter::print(AST::Literal &lit) {
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

void GenericCPrinter::print(AST::VarExpression &expr) {
  *this << *expr.var;
}
void GenericCPrinter::print(AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
}
void GenericCPrinter::print(AST::BinaryOpExpression &expr) {
  *this << "(" << *expr.left << " "
        << AST::getBinaryOpSigil(expr.op) << " " << *expr.right << ")";
}

void GenericCPrinter::print(AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}

void GenericCPrinter::printArgs(AST::CallExpression &expr) {
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) *this << ", ";
    first = false;
    *this << *arg;
  }
}

void GenericCPrinter::print(AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}
void GenericCPrinter::print(AST::AssignOpStatement &stmt) {
  *this << *stmt.left << " " << AST::getBinaryOpSigil(stmt.op)
        << "= " << *stmt.right << ";";
}

void GenericCPrinter::print(AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
}
void GenericCPrinter::print(AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
}

void GenericCPrinter::print(AST::VarDeclarationStatement &stmt) {
    *this << *stmt.type << " " << *stmt.var;
    if (stmt.initializer) {
      *this << " = " << *stmt.initializer;
    }
    *this << ";";
}

}

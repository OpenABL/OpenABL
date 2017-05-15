#include "MasonPrinter.hpp"

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

void MasonPrinter::print(AST::Arg &) {}
void MasonPrinter::print(AST::CallExpression &) {}
void MasonPrinter::print(AST::MemberInitEntry &) {}
void MasonPrinter::print(AST::AgentCreationExpression &) {}
void MasonPrinter::print(AST::NewArrayExpression &) {}
void MasonPrinter::print(AST::ExpressionStatement &) {}
void MasonPrinter::print(AST::SimulateStatement &) {}
void MasonPrinter::print(AST::ArrayType &) {}
void MasonPrinter::print(AST::Param &) {}
void MasonPrinter::print(AST::ConstDeclaration &) {}

void MasonPrinter::print(AST::SimpleType &type) {
  switch (type.resolved.getTypeId()) {
    case Type::BOOL:
    case Type::INT32:
    case Type::FLOAT32:
      *this << type.resolved;
      return;
    case Type::STRING:
      *this << "String";
      return;
    case Type::VEC2:
      *this << "Double2D";
      return;
    case Type::VEC3:
      *this << "Double3D";
      return;
    default:
      assert(0); // TODO
  }
}

void MasonPrinter::print(AST::Var &var) {
  *this << var.name;
}

void MasonPrinter::print(AST::Literal &lit) {
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

void MasonPrinter::print(AST::VarExpression &expr) {
  *this << *expr.var;
}

void MasonPrinter::print(AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    // TODO Right now this simply refers to "this", which is obviously
    // incorrect. Correct mapping of the in/out variables, near agents etc
    // needs to be implemented here
    *this << "this." << expr.member;
  } else {
    *this << *expr.expr << "." << expr.member;
  }
}

void MasonPrinter::print(AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
}

void MasonPrinter::print(AST::BinaryOpExpression &expr) {
  *this << "(" << *expr.left << " " << AST::getBinaryOpSigil(expr.op)
        << " " << *expr.right << ")";
}

void MasonPrinter::print(AST::TernaryExpression &expr) {
  *this << "(" << *expr.condExpr << " ? " << *expr.ifExpr << " : " << *expr.elseExpr << ")";
}

void MasonPrinter::print(AST::AssignStatement &stmt) {
  *this << *stmt.left << " = " << *stmt.right << ";";
}

void MasonPrinter::print(AST::AssignOpStatement &stmt) {
  *this << *stmt.left << " " << AST::getBinaryOpSigil(stmt.op)
        << "= " << *stmt.right << ";";
}

void MasonPrinter::print(AST::VarDeclarationStatement &stmt) {
  *this << *stmt.type << " " << *stmt.var;
  if (stmt.initializer) {
    *this << " = " << *stmt.initializer;
  }
  *this << ";";
}

void MasonPrinter::print(AST::ReturnStatement &stmt) {
  if (stmt.expr) {
    *this << "return " << *stmt.expr << ";";
  } else {
    *this << "return;";
  }
}

void MasonPrinter::print(AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
}

void MasonPrinter::print(AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
}

void MasonPrinter::print(AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    // TODO Nearest neighbor loop
    *this << "for (TODO) " << *stmt.stmt;
  } else if (stmt.isRange()) {
    // TODO Range loop
  } else {
    // TODO Ordinary collection loop
  }
}

void MasonPrinter::print(AST::FunctionDeclaration &decl) {
  if (decl.isStep) {
    *this << "public void " << decl.name << "(SimState state) {" << indent
          << *decl.stmts
          << outdent << nl << "}";
    // TODO
  } else {
    assert(0); // TODO
  }
}

void MasonPrinter::print(AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

void MasonPrinter::print(AST::AgentDeclaration &decl) {
  *this << "import sim.util.*;" << nl << nl
        << "public class " << decl.name << " {" << indent
        << *decl.members << nl << nl;

  // Print constructor
  *this << "public " << decl.name << "(";
  bool first = true;
  for (AST::AgentMemberPtr &member : *decl.members) {
    if (!first) *this << ", ";
    first = false;
    *this << *member->type << " " << member->name;
  }
  *this << ") {" << indent;
  for (AST::AgentMemberPtr &member : *decl.members) {
    *this << nl << "this." << member->name << " = " << member->name << ";";
  }
  *this << outdent << nl << "}";

  for (AST::FunctionDeclaration *fn : script.funcs) {
    if (fn->isStep && &fn->stepAgent() == &decl) {
      *this << nl << nl << *fn;
    }
  }

  *this << outdent << nl << "}";
}

void MasonPrinter::print(AST::Script &script) {
  *this << "import sim.engine.*;\n\n"
        << "public class Sim extends SimState {" << indent << nl
        << "public Sim(long seed) {" << indent << nl
        << "super(seed);"
        << outdent << nl << "}" << nl << nl
        << "public void start() {" << indent << nl
        << "super.start();"
        // TODO Agent initialization here
        << outdent << nl << "}" << nl
        << "public static void main(String[] args) {" << indent << nl
        << "doLoop(Sim.class, args);" << nl
        << "System.exit(0);"
        << outdent << nl << "}"
        << outdent << nl << "}";
}

}

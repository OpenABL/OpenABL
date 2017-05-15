#include "MasonPrinter.hpp"

namespace OpenABL {

void MasonPrinter::print(AST::Arg &) {}
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

void MasonPrinter::print(AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    // TODO Right now this simply refers to "this", which is obviously
    // incorrect. Correct mapping of the in/out variables, near agents etc
    // needs to be implemented here
    *this << "this." << expr.member;
  } else {
    GenericPrinter::print(expr);
  }
}

void MasonPrinter::print(AST::UnaryOpExpression &expr) {
  // TODO Handle vec operations
  GenericPrinter::print(expr);
}

void MasonPrinter::print(AST::BinaryOpExpression &expr) {
  // TODO Handle vec operations
  GenericPrinter::print(expr);
}

void MasonPrinter::print(AST::CallExpression &expr) {
  // TODO This is wrong for Java
  *this << expr.name << "(";
  this->printArgs(expr);
  *this << ")";
}

void MasonPrinter::print(AST::AssignOpStatement &stmt) {
  // TODO Handle vec operations
  GenericPrinter::print(stmt);
}

void MasonPrinter::print(AST::VarDeclarationStatement &stmt) {
  *this << *stmt.type << " " << *stmt.var;
  if (stmt.initializer) {
    *this << " = " << *stmt.initializer;
  }
  *this << ";";
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

#include "MasonPrinter.hpp"

namespace OpenABL {

void MasonPrinter::print(AST::Var &) {}
void MasonPrinter::print(AST::Literal &) {}
void MasonPrinter::print(AST::VarExpression &) {}
void MasonPrinter::print(AST::UnaryOpExpression &) {}
void MasonPrinter::print(AST::BinaryOpExpression &) {}
void MasonPrinter::print(AST::Arg &) {}
void MasonPrinter::print(AST::CallExpression &) {}
void MasonPrinter::print(AST::MemberAccessExpression &) {}
void MasonPrinter::print(AST::TernaryExpression &) {}
void MasonPrinter::print(AST::MemberInitEntry &) {}
void MasonPrinter::print(AST::AgentCreationExpression &) {}
void MasonPrinter::print(AST::NewArrayExpression &) {}
void MasonPrinter::print(AST::ExpressionStatement &) {}
void MasonPrinter::print(AST::AssignStatement &) {}
void MasonPrinter::print(AST::AssignOpStatement &) {}
void MasonPrinter::print(AST::BlockStatement &) {}
void MasonPrinter::print(AST::VarDeclarationStatement &) {}
void MasonPrinter::print(AST::IfStatement &) {}
void MasonPrinter::print(AST::ForStatement &) {}
void MasonPrinter::print(AST::SimulateStatement &) {}
void MasonPrinter::print(AST::ReturnStatement &) {}
void MasonPrinter::print(AST::ArrayType &) {}
void MasonPrinter::print(AST::Param &) {}
void MasonPrinter::print(AST::FunctionDeclaration &) {}
void MasonPrinter::print(AST::ConstDeclaration &) {}

void MasonPrinter::print(AST::SimpleType &type) {
  switch (type.resolved.getTypeId()) {
    case Type::BOOL:
    case Type::INT32:
    case Type::FLOAT32:
      *this << type;
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

void MasonPrinter::print(AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

void MasonPrinter::print(AST::AgentDeclaration &decl) {
  *this << "public class " << decl.name << " {" << indent
        << *decl.members << outdent << nl << "}";
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

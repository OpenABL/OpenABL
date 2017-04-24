#include "FlameGPUPrinter.hpp"

namespace OpenABL {

void FlameGPUPrinter::print(AST::Var &) {}
void FlameGPUPrinter::print(AST::Literal &) {}
void FlameGPUPrinter::print(AST::VarExpression &) {}
void FlameGPUPrinter::print(AST::UnaryOpExpression &) {}
void FlameGPUPrinter::print(AST::BinaryOpExpression &) {}
void FlameGPUPrinter::print(AST::AssignOpExpression &) {}
void FlameGPUPrinter::print(AST::AssignExpression &) {}
void FlameGPUPrinter::print(AST::Arg &) {}
void FlameGPUPrinter::print(AST::CallExpression &) {}
void FlameGPUPrinter::print(AST::MemberAccessExpression &) {}
void FlameGPUPrinter::print(AST::TernaryExpression &) {}
void FlameGPUPrinter::print(AST::MemberInitEntry &) {}
void FlameGPUPrinter::print(AST::AgentCreationExpression &) {}
void FlameGPUPrinter::print(AST::NewArrayExpression &) {}
void FlameGPUPrinter::print(AST::ExpressionStatement &) {}
void FlameGPUPrinter::print(AST::BlockStatement &) {}
void FlameGPUPrinter::print(AST::VarDeclarationStatement &) {}
void FlameGPUPrinter::print(AST::IfStatement &) {}
void FlameGPUPrinter::print(AST::ForStatement &) {}
void FlameGPUPrinter::print(AST::SimulateStatement &) {}
void FlameGPUPrinter::print(AST::ReturnStatement &) {}
void FlameGPUPrinter::print(AST::SimpleType &) {}
void FlameGPUPrinter::print(AST::ArrayType &) {}
void FlameGPUPrinter::print(AST::Param &) {}
void FlameGPUPrinter::print(AST::FunctionDeclaration &) {}
void FlameGPUPrinter::print(AST::AgentMember &) {}
void FlameGPUPrinter::print(AST::AgentDeclaration &) {}
void FlameGPUPrinter::print(AST::ConstDeclaration &) {}

void FlameGPUPrinter::print(AST::Script &) {
  *this << "#ifndef _FUNCTIONS_H_\n"
           "#define _FUNCTIONS_H_\n\n"
           "#include \"header.h\"\n\n";

  for (const FlameModel::Func &func : model.funcs) {
    std::string msgName = !func.inMsgName.empty() ? func.inMsgName : func.outMsgName;
    const FlameModel::Message *msg = model.getMessageByName(msgName);

    *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
          << "xmachine_memory_" << func.agent->name << "* xmemory, "
          << "xmachine_message_" << msgName << "_list* "
          << msgName << "_messages) {" << indent;

    if (!func.func) {
      // This is an automatically generated "output" function
      *this << nl << "add_" << msgName << "_message("
            << msgName << "_messages, xmemory->id";
      for (const AST::AgentMember *member : msg->members) {
        const std::string &name = member->name;
        *this << ", ";

        // TODO reuse pushMembers() code?
        switch (member->type->resolved.getTypeId()) {
          case Type::VEC2:
            *this << "xmemory->" << name << "_x, "
                  << "xmemory->" << name << "_y";
            break;
          case Type::VEC3:
            *this << "xmemory->" << name << "_x, "
                  << "xmemory->" << name << "_y, "
                  << "xmemory->" << name << "_z";
            break;
          default:
            *this << "xmemory->" << name;
        }
        *this << member->name;
      }
      *this << ");";
    } else {
      // TODO Real step function
    }

    *this << nl << "return 0;" << outdent << nl << "}\n\n";
  }

  *this << "#endif // #ifndef _FUNCTIONS_H\n";
}

}

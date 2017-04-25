#include "FlameGPUPrinter.hpp"

namespace OpenABL {

void FlameGPUPrinter::print(AST::AssignExpression &) {}
void FlameGPUPrinter::print(AST::MemberAccessExpression &) {}
void FlameGPUPrinter::print(AST::TernaryExpression &) {}
void FlameGPUPrinter::print(AST::MemberInitEntry &) {}
void FlameGPUPrinter::print(AST::AgentCreationExpression &) {}
void FlameGPUPrinter::print(AST::NewArrayExpression &) {}
void FlameGPUPrinter::print(AST::SimulateStatement &) {}
void FlameGPUPrinter::print(AST::ReturnStatement &) {}
void FlameGPUPrinter::print(AST::ArrayType &) {}
void FlameGPUPrinter::print(AST::Param &) {}
void FlameGPUPrinter::print(AST::FunctionDeclaration &) {}
void FlameGPUPrinter::print(AST::AgentMember &) {}
void FlameGPUPrinter::print(AST::AgentDeclaration &) {}
void FlameGPUPrinter::print(AST::ConstDeclaration &) {}

void FlameGPUPrinter::print(AST::SimpleType &type) {
  Type t = type.resolved;
  switch (t.getTypeId()) {
    case Type::BOOL:
    case Type::INT32:
    case Type::FLOAT32:
      *this << t;
      return;
    case Type::VEC2:
      *this << "glm::vec2";
      return;
    case Type::VEC3:
      *this << "glm::vec3";
      return;
    default:
      assert(0);
  }
}

static void printTypeCtor(FlameGPUPrinter &p, AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    if (t.getTypeId() == Type::VEC2) {
      p << "glm::vec2";
    } else {
      p << "glm::vec3";
    }
    p << "(";
    p.printArgs(expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *(*expr.args)[0];
  }
}
void FlameGPUPrinter::print(AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    // TODO
    //printBuiltin(*this, expr);
  } else {
    *this << expr.name << "(";
    this->printArgs(expr);
    *this << ")";
  }
}

void FlameGPUPrinter::print(AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    // TODO
    return;
  }

  // TODO
  assert(0);
}

void FlameGPUPrinter::print(AST::Script &) {
  *this << "#ifndef _FUNCTIONS_H_\n"
           "#define _FUNCTIONS_H_\n\n"
           "#include \"header.h\"\n\n";

  for (const FlameModel::Func &func : model.funcs) {
    std::string msgName = !func.inMsgName.empty() ? func.inMsgName : func.outMsgName;
    const FlameModel::Message *msg = model.getMessageByName(msgName);

    *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
          << "xmachine_memory_" << func.agent->name << "* xmemory, ";

    if (!func.func) {
      // This is an automatically generated "output" function
      *this << "xmachine_message_" << msgName << "_list* "
            << msgName << "_messages) {" << indent;
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
      *this << ");" << nl << "return 0;" << outdent << nl << "}\n\n";
    } else {
      // For now assuming there is a partition matrix
      *this << "xmachine_message_" << msgName << "_list* "
            << msgName << "_messages, xmachine_" << msgName
            << "_PBM* partition_matrix) {" << indent
            << *func.func->stmts
            << nl << "return 0;" << outdent << nl << "}\n\n";
    }

  }

  *this << "#endif // #ifndef _FUNCTIONS_H\n";
}

}

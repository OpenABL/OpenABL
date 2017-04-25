#include "FlameGPUPrinter.hpp"

namespace OpenABL {

void FlameGPUPrinter::print(AST::MemberInitEntry &) {}
void FlameGPUPrinter::print(AST::AgentCreationExpression &) {}
void FlameGPUPrinter::print(AST::NewArrayExpression &) {}
void FlameGPUPrinter::print(AST::SimulateStatement &) {}
void FlameGPUPrinter::print(AST::ArrayType &) {}
void FlameGPUPrinter::print(AST::AgentMember &) {}
void FlameGPUPrinter::print(AST::AgentDeclaration &) {}
void FlameGPUPrinter::print(AST::ConstDeclaration &) {}

void FlameGPUPrinter::print(AST::SimpleType &type) {
  Type t = type.resolved;
  switch (t.getTypeId()) {
    case Type::VOID:
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
    assert(currentFunc);
    const std::string &msgName = currentFunc->inMsgName;
    const AST::AgentDeclaration &agent = *currentFunc->agent;
    //const FlameModel::Message &msg = *model.getMessageByName(msgName);

    std::string msgVar = msgName + "_message";
    std::string posMember = agent.getPositionMember()->name;
    *this << "xmachine_message_" << msgName << "* " << msgVar
          << " = get_first_location_message(" << msgName + "_messages, "
          << "partition_matrix, (float) xmemory->" << posMember << "_x, "
          << "(float) xmemory->" << posMember << "_y, "
          << "(float) xmemory->" << posMember << "_z);" << nl
          << "while (" << msgVar << ") {" << indent
          << *stmt.stmt // Big TODO here: We still need to remap agent->message access!
          << nl << msgVar << " = get_next_location_message(" << msgVar
          << ", " << msgName << "_messages, partition_matrix);"
          << outdent << nl << "}";

    // TODO What are our semantics on agent self-interaction?
    return;
  }

  // TODO
  assert(0);
}

void FlameGPUPrinter::print(AST::Script &script) {
  *this << "#ifndef _FUNCTIONS_H_\n"
           "#define _FUNCTIONS_H_\n\n"
           "#include \"header.h\"\n\n";

  for (AST::FunctionDeclaration *func : script.funcs) {
    if (func->isStep()) {
      // Step functions will be handled separately later
      continue;
    }

    if (func->isMain()) {
      // TODO Not sure what to do about this one yet
      continue;
    }

    *this << "__device__ " << *func << nl;
  }

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
      }
      *this << ");" << nl << "return 0;" << outdent << nl << "}\n\n";
    } else {
      // For now assuming there is a partition matrix
      currentFunc = &func;
      *this << "xmachine_message_" << msgName << "_list* "
            << msgName << "_messages, xmachine_" << msgName
            << "_PBM* partition_matrix) {" << indent
            << *func.func->stmts
            << nl << "return 0;" << outdent << nl << "}\n\n";
      currentFunc = nullptr;
    }

  }

  *this << "#endif // #ifndef _FUNCTIONS_H\n";
}

}

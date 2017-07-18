#include "FlameGPUPrinter.hpp"

namespace OpenABL {

void FlameGPUPrinter::print(const AST::MemberInitEntry &) {}
void FlameGPUPrinter::print(const AST::AgentCreationExpression &) {}
void FlameGPUPrinter::print(const AST::NewArrayExpression &) {}
void FlameGPUPrinter::print(const AST::SimulateStatement &) {}
void FlameGPUPrinter::print(const AST::AgentMember &) {}
void FlameGPUPrinter::print(const AST::AgentDeclaration &) {}

void FlameGPUPrinter::print(const AST::Literal &lit) {
  GenericPrinter::print(lit);
  if (dynamic_cast<const AST::FloatLiteral *>(&lit)) {
    // We're using floats for FlameGPU
    *this << "f";
  }
}

static void printType(Printer &p, Type t) {
  switch (t.getTypeId()) {
    case Type::VOID:
    case Type::BOOL:
    case Type::INT32:
    case Type::FLOAT32:
      p << t;
      return;
    case Type::VEC2:
      p << "glm::vec2";
      return;
    case Type::VEC3:
      p << "glm::vec3";
      return;
    case Type::ARRAY:
      // Print base type only
      printType(p, t.getBaseType());
      return;
    default:
      assert(0);
  }
}

void FlameGPUPrinter::print(const AST::SimpleType &type) {
  printType(*this, type.resolved);
}

void FlameGPUPrinter::print(const AST::AssignStatement &stmt) {
  if (auto *memAcc = dynamic_cast<AST::MemberAccessExpression *>(&*stmt.left)) {
    if (auto *var = dynamic_cast<AST::VarExpression *>(&*memAcc->expr)) {
      if (currentOutVar && var->var->id == currentOutVar->id) {
        // Write to current agent variable. Rewrite it to the input variable instead
        const std::string &name = memAcc->member;
        std::string inVar = currentInVar->name;
        if (memAcc->type.isVec()) {
          const AST::AgentMember *posMember = currentAgent->getPositionMember();
          if (posMember && name == posMember->name) {
            // Special case position member, as FlameGPU requires specific names here...
            for (const std::string &member : memAcc->type.getVecMembers()) {
              *this << "in->" << member
                    << " = " << *stmt.right << "." << member << ";" << nl;
            }
          } else {
            for (const std::string &member : memAcc->type.getVecMembers()) {
              *this << "in->" << name << "_" << member
                    << " = " << *stmt.right << "." << member << ";" << nl;
            }
          }
        } else {
          *this << inVar << "->" << name << " = " << *stmt.right << ";";
        }
        return;
      }
    }
  }
  GenericPrinter::print(stmt);
}

static void printTypeCtor(FlameGPUPrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    p << "glm::vec" << t.getVecLen() << "(";
    p.printArgs(expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *(*expr.args)[0];
  }
}
void FlameGPUPrinter::print(const AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    if (expr.name == "dist") {
      *this << "glm::distance(";
      printArgs(expr);
      *this << ")";
    } else {
      *this << expr.name << "(";
      printArgs(expr);
      *this << ")";
    }
  } else {
    *this << expr.name << "(";
    printArgs(expr);
    *this << ")";
  }
}

void FlameGPUPrinter::print(const AST::MemberAccessExpression &expr) {
  if (expr.type.isVec()) {
    *this << *expr.expr << "_" << expr.member;
  } else if (expr.type.isAgent()) {
    *this << *expr.expr << "->" << expr.member;
  } else {
    GenericPrinter::print(expr);
  }
}

void FlameGPUPrinter::print(const AST::ConstDeclaration &stmt) {
  *this << "static const __device__ " << *stmt.type << " " << *stmt.var
        << " = " << *stmt.expr << ";";
}

// Extract vecN members into separate glm::vecN variables
static void extractMember(
    FlameGPUPrinter &p, const AST::AgentMember &member,
    const std::string &fromVar, const std::string &toVar) {
  Type type = member.type->resolved;
  const std::string &name = member.name;
  if (type.isVec()) {
    // Position members are mapped to x, y, z without prefix
    std::string namePrefix = member.isPosition ? "" : name + "_";

    p << Printer::nl << *member.type << " " << toVar << "_" << name
      << " = " << *member.type << "("
      << fromVar << "->" << namePrefix << "x, "
      << fromVar << "->" << namePrefix<< "y";
    if (type.getTypeId() == Type::VEC3) {
      p << ", " << fromVar << "->" << namePrefix << "z";
    }
    p << ");";
  }
}

static void extractMsgMembers(
    FlameGPUPrinter &p, const FlameModel::Message &msg, const std::string &toVar) {
  std::string msgVar = msg.name + "_message";
  for (const AST::AgentMember *member : msg.members) {
    extractMember(p, *member, msgVar, toVar);
  }
}

static void extractAgentMembers(
    FlameGPUPrinter &p, const AST::AgentDeclaration &agent, const std::string &var) {
  for (const AST::AgentMemberPtr &member : *agent.members) {
    extractMember(p, *member, var, var);
  }
}

void FlameGPUPrinter::print(const AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    assert(currentFunc);
    const std::string &msgName = currentFunc->inMsgName;
    const AST::AgentDeclaration &agent = *currentFunc->agent;
    const FlameModel::Message &msg = *model.getMessageByName(msgName);
    const std::string &agentVar = (*currentFunc->func->params)[0]->var->name;

    std::string msgVar = msgName + "_message";
    const AST::AgentMember *posMember = agent.getPositionMember();
    std::string posName = posMember->name;
    *this << "xmachine_message_" << msgName << "* " << msgVar
          << " = get_first_" << msgName << "_message(" << msgName + "_messages, "
          << "partition_matrix, (float) " << agentVar << "->x, "
          << "(float) " << agentVar << "->y, (float) " << agentVar << "->z"
          << ");" << nl << "while (" << msgVar << ") {" << indent;
    extractMsgMembers(*this, msg, stmt.var->name);
    *this << nl << *stmt.stmt
          << nl << msgVar << " = get_next_" << msgName << "_message(" << msgVar
          << ", " << msgName << "_messages, partition_matrix);"
          << outdent << nl << "}";

    // TODO What are our semantics on agent self-interaction?
    return;
  }

  // TODO
  assert(0);
}

void FlameGPUPrinter::print(const AST::Script &script) {
  *this << "#ifndef _FUNCTIONS_H_\n"
           "#define _FUNCTIONS_H_\n\n"
           "#include \"header.h\"\n\n";

  for (const AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  for (const AST::FunctionDeclaration *func : script.funcs) {
    if (func->isStep) {
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

    if (!func.func) {
      // This is an automatically generated "output" function
      *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
            << "xmachine_memory_" << func.agent->name << "* xmemory, "
            << "xmachine_message_" << msgName << "_list* "
            << msgName << "_messages) {" << indent
            << nl << "add_" << msgName << "_message("
            << msgName << "_messages";
      for (const auto &member : FlameModel::getUnpackedMembers(msg->members, true)) {
        const std::string &name = member.first;
        *this << ", xmemory->" << name;
      }
      *this << ");" << nl << "return 0;" << outdent << nl << "}\n\n";
    } else {
      // For now assuming there is a partition matrix
      currentFunc = &func;
      const AST::Param &param = *(*func.func->params)[0];
      const std::string &paramName = param.var->name;
      *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
            << "xmachine_memory_" << func.agent->name << "* " << paramName
            << ", xmachine_message_" << msgName << "_list* "
            << msgName << "_messages, xmachine_message_" << msgName
            << "_PBM* partition_matrix) {" << indent;
      extractAgentMembers(*this, *func.agent, paramName);

      // Remember the agent variables we're currently working on
      currentAgent = func.agent;
      currentInVar = &*param.var;
      currentOutVar = &*param.outVar;
      *this << *func.func->stmts;
      currentAgent = nullptr;
      currentInVar = nullptr;
      currentOutVar = nullptr;

      *this << nl << "return 0;" << outdent << nl << "}\n\n";
      currentFunc = nullptr;
    }

  }

  *this << "#endif // #ifndef _FUNCTIONS_H\n";
}

}

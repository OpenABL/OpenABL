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
  if (useFloat && dynamic_cast<const AST::FloatLiteral *>(&lit)) {
    *this << "f";
  }
}

void FlameGPUPrinter::printType(Type t) {
  switch (t.getTypeId()) {
    case Type::VOID:
    case Type::BOOL:
    case Type::INT32:
      *this << t;
      return;
    case Type::FLOAT:
      *this << (useFloat ? "float" : "double");
      return;
    case Type::VEC2:
      *this << (useFloat ? "glm::vec2" : "glm::dvec2");
      return;
    case Type::VEC3:
      *this << (useFloat ? "glm::vec3" : "glm::dvec3");
      return;
    case Type::ARRAY:
      // Print base type only
      printType(t.getBaseType());
      return;
    default:
      assert(0);
  }
}

void FlameGPUPrinter::print(const AST::VarExpression &expr) {
  const AST::Var &var = *expr.var;
  const ScopeEntry &e = script.scope.get(var.id);
  if (e.val.isVec()) {
    // We can't create vec constants on FlameGPU, so inline their values instead
    const AST::Expression *expr = e.val.toExpression();
    *this << *expr;
    delete expr;
  } else {
    GenericPrinter::print(expr);
  }
}

bool FlameGPUPrinter::isSpecialBinaryOp(
    AST::BinaryOp op, const AST::Expression &left, const AST::Expression &right) {
  Type l = left.type, r = right.type;
  return op == AST::BinaryOp::MOD && !(l.isInt() && r.isInt());
}
void FlameGPUPrinter::printSpecialBinaryOp(
    AST::BinaryOp, const AST::Expression &left, const AST::Expression &right) {
  *this << "fmod(" << left << ", " << right << ")";
}

static bool isSameVar(const AST::Expression &expr, const AST::Var *var) {
  if (var == nullptr) {
    return false;
  }

  if (const auto *varExpr = dynamic_cast<const AST::VarExpression *>(&expr)) {
    return varExpr->var->id == var->id;
  }
  return false;
}

void FlameGPUPrinter::print(const AST::AssignStatement &stmt) {
  if (auto *memAcc = dynamic_cast<AST::MemberAccessExpression *>(&*stmt.left)) {
    if (isSameVar(*memAcc->expr, currentOutVar)) {
      // Write to current agent variable. Rewrite it to the input variable instead
      const std::string &name = memAcc->member;
      std::string inVar = currentInVar->name;
      if (memAcc->type.isVec()) {
        std::string varName;
        if (auto *var = dynamic_cast<AST::VarExpression *>(&*stmt.right)) {
          varName = var->var->name;
        } else {
          varName = makeAnonLabel();
          printType(memAcc->type);
          *this << " " << varName << " = " << *stmt.right << ";";
        }

        const AST::AgentMember *posMember = currentAgent->getPositionMember();
        if (posMember && name == posMember->name) {
          // Special case position member, as FlameGPU requires specific names here...
          for (const std::string &member : memAcc->type.getVecMembers()) {
            *this << "in->" << member
                  << " = " << varName << "." << member << ";" << nl;
          }
        } else {
          for (const std::string &member : memAcc->type.getVecMembers()) {
            *this << "in->" << name << "_" << member
                  << " = " << varName << "." << member << ";" << nl;
          }
        }
      } else {
        *this << inVar << "->" << name << " = " << *stmt.right << ";";
      }
      return;
    }
  }
  GenericPrinter::print(stmt);
}

void FlameGPUPrinter::print(const AST::CallExpression &expr) {
  if (expr.isCtor()) {
    Type t = expr.type;
    if (t.isVec()) {
      *this << (useFloat ? "glm::vec" : "glm::dvec")
            << t.getVecLen() << "(";
      printArgs(expr);
      *this << ")";
    } else {
      *this << "(";
      printType(t);
      *this << ") " << expr.getArg(0);
    }
  } else {
    const FunctionSignature &sig = expr.calledSig;
    if (!sig.decl && (expr.name == "random" || expr.name == "randomInt")) {
      *this << expr.name << "(rand48, ";
      printArgs(expr);
      *this << ")";
      return;
    }

    if (expr.name == "dist") {
      *this << "glm::distance";
    } else if (expr.name == "length") {
      *this << "glm::length";
    } else if (expr.name == "normalize") {
      *this << "glm::normalize";
    } else {
      *this << expr.name;
    }
    *this << "(";
    if (expr.calledFunc && expr.calledFunc->usesRng) {
      *this << "rand48";
      if (!expr.args->empty()) {
        *this << ", ";
      }
    }
    printArgs(expr);
    *this << ")";
  }
}

void FlameGPUPrinter::print(const AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    if (expr.type.isVec()) {
      *this << *expr.expr << "_" << expr.member;
    } else if (isSameVar(*expr.expr, currentNearVar)) {
      // Access to a message variable in a for-near loop
      *this << currentFunc->inMsgName << "_message->" << expr.member;
    } else {
      *this << *expr.expr << "->" << expr.member;
    }
  } else {
    GenericPrinter::print(expr);
  }
}

void FlameGPUPrinter::print(const AST::ConstDeclaration &stmt) {
  if (stmt.type->resolved.isVec()) {
    // Don't generate constants for vec2/vec3, as we can't initialized them
    return;
  }

  *this << "static const __device__ " << *stmt.type << " " << *stmt.var
        << (stmt.isArray ? "[]" : "")
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
    const AST::AgentMember &posMember = *agent.getPositionMember();
    const FlameModel::Message &msg = *model.getMessageByName(msgName);
    const AST::Expression &radiusExpr = stmt.getNearRadius();
    const std::string &agentVar = (*currentFunc->func->params)[0]->var->name;

    std::string msgVar = msgName + "_message";
    *this << "xmachine_message_" << msgName << "* " << msgVar << ";"
          << nl << "for (" << indent
          << nl << msgVar << " = get_first_" << msgName << "_message(" << msgName + "_messages, "
          << "partition_matrix, " << agentVar << "->x, "
          << agentVar << "->y, " << agentVar << "->z"
          << ");"
          << nl << "" << msgVar << ";"
          << nl << msgVar << " = get_next_" << msgName << "_message(" << msgVar
          << ", " << msgName << "_messages, partition_matrix)"
          << outdent << nl << ") {" << indent;
    extractMsgMembers(*this, msg, stmt.var->name);
    *this << nl << "if (glm::distance(" << stmt.var->name << "_" << posMember.name
          << ", " << agentVar << "_" << posMember.name
          << ") >= " << radiusExpr << ") continue;";

    currentNearVar = &*stmt.var;
    *this << nl << *stmt.stmt;
    currentNearVar = nullptr;

    *this << outdent << nl << "}";

    // TODO What are our semantics on agent self-interaction?
    return;
  }

  // TODO
  assert(0);
}

void FlameGPUPrinter::print(const AST::FunctionDeclaration &decl) {
  *this << *decl.returnType << " " << decl.name << "(";
  if (decl.usesRng) {
    *this << "RNG_rand48 *rand48";
    if (!decl.params->empty()) {
      *this << ", ";
    }
  }
  printParams(decl);
  *this << ") {" << indent;
  *this << *decl.stmts << outdent << nl << "}";
}

void FlameGPUPrinter::print(const AST::Script &script) {
  *this << "#ifndef _FUNCTIONS_H_\n"
           "#define _FUNCTIONS_H_\n\n"
           "#include \"header.h\"\n"
           "#include \"libabl_flamegpu.h\"\n\n";

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
    if (!func.func) {
      // This is an automatically generated "output" function
      const std::string &msgName = func.outMsgName;
      const FlameModel::Message *msg = model.getMessageByName(msgName);
      *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
            << "xmachine_memory_" << func.agent->name << "* xmemory, "
            << "xmachine_message_" << msgName << "_list* "
            << msgName << "_messages) {" << indent
            << nl << "add_" << msgName << "_message("
            << msgName << "_messages";
      for (const auto &member : FlameModel::getUnpackedMembers(msg->members, useFloat, true)) {
        const std::string &name = member.first;
        *this << ", xmemory->" << name;
      }
      *this << ");" << nl << "return 0;" << outdent << nl << "}\n\n";
    } else {
      const AST::Param &param = *(*func.func->params)[0];
      const std::string &paramName = param.var->name;
      *this << "__FLAME_GPU_FUNC__ int " << func.name << "("
            << "xmachine_memory_" << func.agent->name << "* " << paramName;
      if (!func.inMsgName.empty()) {
        // For now assuming there is a partition matrix
        const std::string &msgName = func.inMsgName;
        *this << ", xmachine_message_" << msgName << "_list* "
              << msgName << "_messages, xmachine_message_" << msgName
              << "_PBM* partition_matrix";
      }
      if (func.func->usesRng) {
        *this << ", RNG_rand48 *rand48";
      }
      *this << ") {" << indent;
      extractAgentMembers(*this, *func.agent, paramName);

      // Remember the agent variables we're currently working on
      currentFunc = &func;
      currentAgent = func.agent;
      currentInVar = &*param.var;
      currentOutVar = &*param.outVar;
      *this << *func.func->stmts;
      currentFunc = nullptr;
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

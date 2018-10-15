/* Copyright 2017 OpenABL Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. */

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
    // out.member = value
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

    // out.vec_member.x = value
    if (auto *memAcc2 = dynamic_cast<AST::MemberAccessExpression *>(&*memAcc->expr)) {
      if (isSameVar(*memAcc2->expr, currentOutVar)) {
        assert(memAcc2->type.isVec());

        const std::string &memberName = memAcc2->member;
        const std::string &dimName = memAcc->member;
        const AST::AgentMember *posMember = currentAgent->getPositionMember();
        if (posMember && memberName == posMember->name) {
          // Special case position member, as FlameGPU requires specific names here...
          *this << "in->" << dimName << " = " << *stmt.right << ";";
        } else {
          *this << "in->" << memberName << "_" << dimName << " = " << *stmt.right << ";";
        }
        return;
      }
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
    if (expr.isBuiltin()) {
      // Handle some special builtins
      if (expr.name == "random" || expr.name == "randomInt") {
        *this << expr.name << "(rand48, ";
        printArgs(expr);
        *this << ")";
        return;
      } else if (expr.name == "removeCurrent") {
        *this << "_isDead = true";
        return;
      } else if (expr.name == "add") {
        const auto *agentExpr = dynamic_cast<const AST::AgentCreationExpression *>(&expr.getArg(0));
        const AST::AgentDeclaration *addedAgent = agentExpr->type.getAgentDecl();

        // Create variables for vector members, to avoid double-evaluation
        for (const AST::AgentMemberPtr &member : *addedAgent->members) {
          if (member->type->resolved.isVec()) {
            const AST::Expression &initExpr = agentExpr->getExprFor(member->name);
            *this << *member->type << " _" << member->name << " = " << initExpr << ";" << nl;
          }
        }

        *this << "add_" << addedAgent->name << "_agent(agent_"
              << addedAgent->name << "_agents, ";
        printCommaSeparated(*addedAgent->members, [&](const AST::AgentMemberPtr &member) {
          const AST::Expression &initExpr = agentExpr->getExprFor(member->name);
          if (initExpr.type.isVec2()) {
            *this << "_" << member->name << ".x, _" << member->name << ".y";
            if (member->isPosition) {
              *this << ", 0.0";
            }
          } else if (initExpr.type.isVec3()) {
            *this << "_" << member->name << ".x, _" << member->name
                  << ".y, _" << member->name << ".z";
          } else {
            *this << initExpr;
          }
        });
        *this << ")";
        return;
      } else if (expr.name == "count") {
        Type type = expr.getArg(0).type;
        if (expr.getNumArgs() == 2) {
          AST::AgentDeclaration *decl = type.getAgentDecl();
          AST::AgentMember *member = type.getAgentMember();
          std::string state = decl->name + "_default";
          *this << "count_" << decl->name << "_" << state << "_" << member->name
                << "_variable(" << expr.getArg(1) << ")";
        } else {
          AST::AgentDeclaration *decl = type.getAgentDecl();
          std::string state = decl->name + "_default";
          *this << "get_agent_" << decl->name << "_" << state << "_count()";
        }
        return;
      } else if (expr.name == "sum") {
        Type type = expr.getArg(0).type;
        AST::AgentDeclaration *decl = type.getAgentDecl();
        AST::AgentMember *member = type.getAgentMember();
        std::string state = decl->name + "_default";
        *this << "reduce_" << decl->name << "_" << state
              << "_" << member->name << "_variable()";
        return;
      } else if (expr.name == "log_csv") {
        std::string formatStr;
        bool first = true;
        for (const AST::ExpressionPtr &arg : *expr.args) {
          if (!first) {
            formatStr += ",";
          }
          first = false;
          if (arg->type.isInt()) {
            formatStr += "%d";
          } else if (arg->type.isFloat()) {
            formatStr += "%f";
          } else {
            assert(0); // TODO
          }
        }
        *this << "fprintf(openabl_log_file, \"" << formatStr << "\\n\", ";
        printArgs(expr);
        *this << ")";
        return;
      } else if (expr.name == "getLastExecTime") {
        *this << "(openabl_event_elapsed / 1000)";
        return;
      }
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

void FlameGPUPrinter::print(const AST::ReturnStatement &stmt) {
  if (currentFunc) {
    // We're inside a step function, where we need to "return 0;"
    assert(!stmt.expr);
    *this << "return 0;";
    return;
  }

  GenericPrinter::print(stmt);
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
  if (decl.isSequentialStep() && script.usesTiming) {
    *this << nl << "cudaEventRecord(openabl_event_stop);"
          << nl << "cudaEventSynchronize(openabl_event_stop);"
          << nl << "float openabl_event_elapsed = 0.;"
          << nl << "cudaEventElapsedTime(&openabl_event_elapsed, openabl_event_start, openabl_event_stop);"
          << nl << "cudaEventRecord(openabl_event_start);";
  }
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

  if (script.usesLogging) {
    *this << "FILE *openabl_log_file;" << nl;
  }
  if (script.usesTiming) {
    *this << "cudaEvent_t openabl_event_start, openabl_event_stop;" << nl;
  }

  *this << nl << "__FLAME_GPU_INIT_FUNC__ void openabl_init() {" << indent;
  if (script.usesLogging) {
    *this << nl << "openabl_log_file = fopen(\"log.csv\", \"w\");";
  }
  if (script.usesTiming) {
    *this << nl << "cudaEventCreate(&openabl_event_start);"
          << nl << "cudaEventCreate(&openabl_event_stop);"
          << nl << "cudaEventRecord(openabl_event_start);";
  }

  *this << outdent << nl << "}" << nl << nl
        << "__FLAME_GPU_EXIT_FUNC__ void openabl_exit() {" << indent;
  if (script.usesLogging) {
    *this << nl << "fclose(openabl_log_file);";
  }
  if (script.usesTiming) {
    *this << nl << "cudaEventDestroy(openabl_event_start);"
          << nl << "cudaEventDestroy(openabl_event_stop);";
  }
  *this << outdent << nl << "}" << nl;

  for (const AST::FunctionDeclaration *func : script.funcs) {
    if (func->isParallelStep()) {
      // Step functions will be handled separately later
      continue;
    }

    if (func->isMain()) {
      // Handled by FlameMainPrinter
      continue;
    }

    if (func->isSequentialStep()) {
      *this << "__FLAME_GPU_STEP_FUNC__ ";
    } else {
      *this << "__device__ ";
    }

    *this << *func << nl;
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
      if (func.func->runtimeAddedAgent) {
        const AST::AgentDeclaration *addedAgent = func.func->runtimeAddedAgent;
        *this << ", xmachine_memory_" << addedAgent->name << "_list *agent_"
              << addedAgent->name << "_agents";
      }
      if (func.func->usesRng) {
        *this << ", RNG_rand48 *rand48";
      }
      *this << ") {" << indent;
      if (func.func->usesRuntimeRemoval) {
        *this << nl << "bool _isDead = false;";
      }
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

      if (func.func->usesRuntimeRemoval) {
        *this << nl << "return _isDead;";
      } else {
        *this << nl << "return 0;";
      }
      *this << outdent << nl << "}\n\n";
    }
  }

  *this << "#endif // #ifndef _FUNCTIONS_H\n";
}

}

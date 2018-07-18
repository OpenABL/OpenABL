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

#include "FlamePrinter.hpp"

namespace OpenABL {

// TODO Deduplicate with FlameGPUPrinter!
// This is a lot of copy&paste

void FlamePrinter::print(const AST::MemberInitEntry &) {}
void FlamePrinter::print(const AST::AgentCreationExpression &) {}
void FlamePrinter::print(const AST::NewArrayExpression &) {}
void FlamePrinter::print(const AST::SimulateStatement &) {}
void FlamePrinter::print(const AST::AgentMember &) {}
void FlamePrinter::print(const AST::AgentDeclaration &) {}

void FlamePrinter::printType(Type t) {
  if (t.isArray()) {
    // Print only the base type
    printType(t.getBaseType());
  } else if (t.isFloat()) {
    *this << (useFloat ? "float" : "double");
  } else {
    *this << t;
  }
}

void FlamePrinter::print(const AST::AssignStatement &stmt) {
  if (auto memAcc = dynamic_cast<AST::MemberAccessExpression *>(&*stmt.left)) {
    if (memAcc->expr->type.isAgent()) {
      // TODO We're assuming here that this is a write to the "out" variable (obviously wrong)
      // Write to agent member (convert to set_* call)
      const std::string &name = memAcc->member;
      if (memAcc->type.isVec()) {
        for (const std::string &member : memAcc->type.getVecMembers()) {
          // TODO Prevent double evaluation of RHS?
          *this << "set_" << name << "_" << member
                << "(" << *stmt.right << "." << member << ");" << nl;
        }
      } else {
        *this << "set_" << name << "(" << *stmt.right << ");";
      }
      return;
    }
  }
  GenericPrinter::print(stmt);
}

static void printTypeCtor(FlamePrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    size_t numArgs = expr.args->size();
    if (t.getTypeId() == Type::VEC2) {
      p << (numArgs == 1 ? "float2_fill" : "float2_create");
    } else {
      p << (numArgs == 1 ? "float3_fill" : "float3_create");
    }
    p << "(";
    p.printArgs(expr);
    p << ")";
  } else {
    p << "(";
    p.printType(t);
    p << ") " << expr.getArg(0);
  }
}
void FlamePrinter::print(const AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else {
    const FunctionSignature &sig = expr.calledSig;

    // TODO
    /*if (sig.name == "add") {
      AST::AgentDeclaration *agent = sig.paramTypes[0].getAgentDecl();
      p << "*DYN_ARRAY_PLACE(&agents.agents_" << agent->name
        << ", " << agent->name << ") = " << *(*expr.args)[0];
      return;
    } else if (sig.name == "save") {
      p << "save(&agents, agents_info, " << *(*expr.args)[0] << ")";
      return;
    }*/

    *this << sig.name << "(";
    printArgs(expr);
    *this << ")";
  }
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

void FlamePrinter::print(const AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    if (expr.type.isVec()) {
      *this << *expr.expr << "_" << expr.member;
    } else if (isSameVar(*expr.expr, currentNearVar)) {
      // Access to a message variable in a for-near loop
      *this << currentFunc->inMsgName << "_message->" << expr.member;
    } else {
      // TODO Assuming its the current agent for now, obviously wrong
      *this << "get_" << expr.member << "()" ;
    }
  } else {
    GenericPrinter::print(expr);
  }
}

static std::string acc(const std::string &fromVar, const std::string &member) {
  if (fromVar.empty()) {
    return "get_" + member + "()";
  } else {
    return fromVar + "->" + member;
  }
}

// Extract vecN members into separate glm::vecN variables
static void extractMember(
    FlamePrinter &p, const AST::AgentMember &member,
    const std::string &fromVar, const std::string &toVar) {
  AST::Type &type = *member.type;
  const std::string &name = member.name;
  if (type.resolved.isVec()) {
    p << Printer::nl << type << " " << toVar << "_" << name << " = "
      << type << "_create("
      << acc(fromVar, name + "_x") << ", "
      << acc(fromVar, name + "_y");
    if (type.resolved.getTypeId() == Type::VEC3) {
      p << ", " << acc(fromVar, name + "_z");
    }
    p << ");";

    // Variable might not be used, suppress warning
    // TODO We should try to avoid generating it in the first place
    p <<" (void) " << toVar << "_" << name << ";";
  }
}

static void extractMsgMembers(
    FlamePrinter &p, const FlameModel::Message &msg, const std::string &toVar) {
  std::string msgVar = msg.name + "_message";
  for (const AST::AgentMember *member : msg.members) {
    extractMember(p, *member, msgVar, toVar);
  }
}

static void extractAgentMembers(
    FlamePrinter &p, const AST::AgentDeclaration &agent, const std::string &var) {
  for (const AST::AgentMemberPtr &member : *agent.members) {
    extractMember(p, *member, "", var);
  }
}

void FlamePrinter::print(const AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    assert(currentFunc);
    const std::string &msgName = currentFunc->inMsgName;
    const AST::AgentDeclaration &agent = *currentFunc->agent;
    const AST::AgentMember &posMember = *agent.getPositionMember();
    const FlameModel::Message &msg = *model.getMessageByName(msgName);
    const AST::Expression &agentExpr = stmt.getNearAgent();
    const AST::Expression &radiusExpr = stmt.getNearRadius();
    const char *dist_fn = posMember.type->resolved == Type::VEC2
      ? "dist_float2" : "dist_float3";

    std::string msgVar = msgName + "_message";
    std::string upperMsgName = msgVar;
    for (char &c : upperMsgName) c = toupper(c);

    *this << "START_" << upperMsgName << "_LOOP" << indent;
    extractMsgMembers(*this, msg, stmt.var->name);
    *this << nl << "if (" << dist_fn << "(" << stmt.var->name << "_" << posMember.name
          << ", " << agentExpr << "_" << posMember.name
          << ") >= " << radiusExpr << ") continue;";

    currentNearVar = &*stmt.var;
    *this << nl << *stmt.stmt;
    currentNearVar = nullptr;

    *this << outdent << nl << "FINISH_" << upperMsgName << "_LOOP";

    // TODO What are our semantics on agent self-interaction?
    return;
  }

  // TODO
  assert(0);
}

void FlamePrinter::print(const AST::Script &script) {
  *this << "#include \"header.h\"\n"
        << "#include \"libabl.h\"\n\n";

  for (AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  for (AST::FunctionDeclaration *func : script.funcs) {
    if (func->isAnyStep()) {
      // Step functions will be handled separately later
      continue;
    }

    if (func->isMain()) {
      // Handled by FlameMainPrinter
      continue;
    }

    *this << *func << nl;
  }

  for (const FlameModel::Func &func : model.funcs) {
    std::string msgName = !func.inMsgName.empty() ? func.inMsgName : func.outMsgName;
    const FlameModel::Message *msg = model.getMessageByName(msgName);

    if (!func.func) {
      // This is an automatically generated "output" function
      *this << "int " << func.name << "() {" << indent
            << nl << "add_" << msgName << "_message(";
      printCommaSeparated(msg->members, [&](const AST::AgentMember *member) {
        // TODO reuse pushMembers() code?
        const std::string &name = member->name;
        switch (member->type->resolved.getTypeId()) {
          case Type::VEC2:
            *this << "get_" << name << "_x(), "
                  << "get_" << name << "_y()";
            break;
          case Type::VEC3:
            *this << "get_" << name << "_x(), "
                  << "get_" << name << "_y(), "
                  << "get_" << name << "_z()";
            break;
          default:
            *this << "get_" << name << "()";
        }
      });
      *this << ");" << nl << "return 0;" << outdent << nl << "}\n\n";
    } else {
      // For now assuming there is a partition matrix
      currentFunc = &func;
      const std::string &paramName = (*func.func->params)[0]->var->name;
      *this << "int " << func.name << "() {" << indent;
      extractAgentMembers(*this, *func.agent, paramName);
      *this << *func.func->stmts
            << nl << "return 0;" << outdent << nl << "}\n\n";
      currentFunc = nullptr;
    }
  }
}

}

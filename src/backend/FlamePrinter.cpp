#include "FlamePrinter.hpp"

namespace OpenABL {

// TODO Deduplicate with FlameGPUPrinter!
// This is a lot of copy&paste

void FlamePrinter::print(AST::MemberInitEntry &) {}
void FlamePrinter::print(AST::AgentCreationExpression &) {}
void FlamePrinter::print(AST::NewArrayExpression &) {}
void FlamePrinter::print(AST::SimulateStatement &) {}
void FlamePrinter::print(AST::ArrayType &) {}
void FlamePrinter::print(AST::AgentMember &) {}
void FlamePrinter::print(AST::AgentDeclaration &) {}
void FlamePrinter::print(AST::ConstDeclaration &) {}

void FlamePrinter::print(AST::SimpleType &type) {
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

static void printTypeCtor(FlamePrinter &p, AST::CallExpression &expr) {
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
void FlamePrinter::print(AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    // TODO
    //printBuiltin(*this, expr);
    *this << expr.name << "(";
    this->printArgs(expr);
    *this << ")";
  } else {
    *this << expr.name << "(";
    this->printArgs(expr);
    *this << ")";
  }
}

void FlamePrinter::print(AST::MemberAccessExpression &expr) {
  if (expr.type.isVec()) {
    *this << *expr.expr << "_" << expr.member;
  } else if (expr.expr->type.isAgent()) {
    // TODO Assuming its the current agent for now, obviously wrong
    *this << "get_" << expr.member << "()" ;
  } else {
    GenericCPrinter::print(expr);
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
    p << Printer::nl << type << " " << toVar << "_" << name << " = " << type << "("
      << acc(fromVar, name + "_x") << ", "
      << acc(fromVar, name + "_y");
    if (type.resolved.getTypeId() == Type::VEC3) {
      p << ", " << acc(fromVar, name + "_z");
    }
    p << ");";
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

void FlamePrinter::print(AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    assert(currentFunc);
    const std::string &msgName = currentFunc->inMsgName;
    const AST::AgentDeclaration &agent = *currentFunc->agent;
    const FlameModel::Message &msg = *model.getMessageByName(msgName);

    std::string msgVar = msgName + "_message";
    std::string upperMsgName = msgVar;
    for (char &c : upperMsgName) c = toupper(c);

    std::string posMember = agent.getPositionMember()->name;
    *this << "START_" << upperMsgName << "_LOOP" << indent;
    extractMsgMembers(*this, msg, stmt.var->name);
    *this << nl << *stmt.stmt
          << outdent << nl << "END_" << upperMsgName << "_LOOP";

    // TODO What are our semantics on agent self-interaction?
    return;
  }

  // TODO
  assert(0);
}

void FlamePrinter::print(AST::Script &script) {
  *this << "#include \"header.h\"\n\n";

  for (AST::FunctionDeclaration *func : script.funcs) {
    if (func->isStep()) {
      // Step functions will be handled separately later
      continue;
    }

    if (func->isMain()) {
      // TODO Not sure what to do about this one yet
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
      bool first = true;
      for (const AST::AgentMember *member : msg->members) {
        const std::string &name = member->name;
        if (!first) *this << ", ";
        first = true;

        // TODO reuse pushMembers() code?
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
      }
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

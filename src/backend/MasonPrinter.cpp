#include "MasonPrinter.hpp"

namespace OpenABL {

void MasonPrinter::print(const AST::MemberInitEntry &) {}
void MasonPrinter::print(const AST::NewArrayExpression &) {}

static void printType(Printer &p, Type type) {
  switch (type.getTypeId()) {
    case Type::BOOL:
      p << "boolean";
      return;
    case Type::INT32:
      p << "int";
      return;
    case Type::FLOAT32:
      // TODO Use doubles, because that's what Mason uses.
      // Need to figure out what semantics we want to have here.
      p << "double";
      return;
    case Type::STRING:
      p << "String";
      return;
    case Type::VEC2:
      p << "Double2D";
      return;
    case Type::VEC3:
      p << "Double3D";
      return;
    case Type::ARRAY:
      printType(p, type.getBaseType());
      p << "[]";
      return;
    default:
      assert(0); // TODO
  }
}

void MasonPrinter::print(const AST::SimpleType &type) {
  printType(*this, type.resolved);
}

void MasonPrinter::print(const AST::VarExpression &expr) {
  VarId id = expr.var->id;
  const ScopeEntry &entry = script.scope.get(id);
  if (entry.isGlobal) {
    *this << "Sim." << *expr.var;
  } else {
    if (id == currentInVar || id == currentOutVar) {
      // TODO Correct mapping for "out" variable
      *this << "this";
      return;
    }

    *this << *expr.var;
  }
}

static void printBinaryOp(MasonPrinter &p, AST::BinaryOp op,
                          const AST::Expression &left, const AST::Expression &right) {
  Type l = left.type;
  Type r = right.type;
  if (l.isVec() || r.isVec()) {
    p << left << ".";
    switch (op) {
      case AST::BinaryOp::ADD: p << "add"; break;
      case AST::BinaryOp::SUB: p << "subtract"; break;
      case AST::BinaryOp::MUL: p << "multiply"; break;
      case AST::BinaryOp::DIV:
        // Emulate divide via multiply by reciprocal
        p << "multiply(1. / " << right << ")";
        return;
      default:
        assert(0);
    }
    p << "(" << right << ")";
    return;
  }

  p << "(" << left << " " << AST::getBinaryOpSigil(op) << " " << right << ")";
}

void MasonPrinter::print(const AST::BinaryOpExpression &expr) {
  printBinaryOp(*this, expr.op, *expr.left, * expr.right);
}

static void printVecCtorArgs(MasonPrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  int vecLen = t.getVecLen();
  size_t numArgs = expr.args->size();
  if (numArgs == 1) {
    // TODO Multiple evaluation
    const AST::Expression &arg = *expr.getArg(0).expr;
    p << arg << ", " << arg;
    if (vecLen == 3) {
      p << ", " << arg;
    }
  } else {
    p.printArgs(expr);
  }
}

static void printTypeCtor(MasonPrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    int vecLen = t.getVecLen();
    p << "new Double" << vecLen << "D(";
    printVecCtorArgs(p, expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *expr.getArg(0).expr;
  }
}

void MasonPrinter::print(const AST::CallExpression &expr) {
  const std::string &name = expr.name;
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    if (name == "dist") {
      *this << expr.getArg(0) << ".distance(" << expr.getArg(1) << ")";
    } else if (name == "length") {
      *this << expr.getArg(0) << ".length()";
    } else if (name == "normalize") {
      *this << expr.getArg(0) << ".normalize()";
    } else if (name == "random") {
      *this << "Util.random(" << getSimVarName() << ".random, ";
      printArgs(expr);
      *this << ")";
    } else if (name == "sin" || name == "cos" || name == "tan"
            || name == "sinh" || name == "cosh" || name == "tanh"
            || name == "asin" || name == "acos" || name == "atan"
            || name == "exp" || name == "log" || name == "sqrt"
            || name == "round"
    ) {
      // Basic math functions
      *this << "java.lang.Math." << name << "(";
      printArgs(expr);
      *this << ")";
    } else if (name == "add") {
      std::string aLabel = makeAnonLabel();
      const AST::Arg &arg = expr.getArg(0);
      Type type = arg.expr->type;
      AST::AgentDeclaration *agent = type.getAgentDecl();
      AST::AgentMember *posMember = agent->getPositionMember();

      *this << type << " " << aLabel << " = " << arg << ";" << nl;
      if (posMember) {
        *this << "env.setObjectLocation(" << aLabel << ", "
              << aLabel << "." << posMember->name << ");" << nl;
      }

      // TODO There are some ordering issues here, which we ignore for now
      // This does not fully respect the order between different agent types
      *this << "schedule.scheduleRepeating(" << aLabel << ")";
    } else if (name == "save") {
      // TODO Handle save
      *this << "//save()";
    } else {
      // TODO Handle other builtins
      const FunctionSignature &sig = expr.calledSig;
      *this << sig.name << "(";
      printArgs(expr);
      *this << ")";
    }
  } else {
    *this << getSimVarName() << "." << expr.name << "(";
    printArgs(expr);
    *this << ")";
  }
}

void MasonPrinter::print(const AST::AssignStatement &stmt) {
  if (auto *access = dynamic_cast<const AST::MemberAccessExpression *>(&*stmt.left)) {
    if (access->expr->type.isVec()) {
      // Assignment to vector component
      // Convert into creation of new DoubleND, because it is immmutable
      // TODO Automatic conversion to MutableDoubleND would be nice
      Type vecType = access->expr->type;
      unsigned vecLen = vecType.getVecLen();
      *this << *access->expr << " = new Double" << vecLen << "D(";
      printCommaSeparated(vecType.getVecMembers(), [&](const std::string &member) {
        if (access->member == member) {
          *this << *stmt.right;
        } else {
          *this << *access->expr << "." << member;
        }
      });
      *this << ");";
      return;
    }
  }

  GenericPrinter::print(stmt);
}

void MasonPrinter::print(const AST::AssignOpStatement &stmt) {
  if (auto *access = dynamic_cast<const AST::MemberAccessExpression *>(&*stmt.left)) {
    if (access->expr->type.isVec()) {
      // Assignment to vector component, same as above
      // Some copy-paste here, as the code is hard to combine elegantly
      Type vecType = access->expr->type;
      unsigned vecLen = vecType.getVecLen();
      *this << *access->expr << " = new Double" << vecLen << "D(";
      printCommaSeparated(vecType.getVecMembers(), [&](const std::string &member) {
        *this << *access->expr << "." << member;
        if (access->member == member) {
          *this << " " << getBinaryOpSigil(stmt.op) << " " << *stmt.right;
        }
      });
      *this << ");";
      return;
    }
  }

  *this << *stmt.left << " = ";
  printBinaryOp(*this, stmt.op, *stmt.left, *stmt.right);
  *this << ";";
}

void MasonPrinter::print(const AST::UnaryOpExpression &expr) {
  Type t = expr.expr->type;
  if (t.isVec()) {
    if (expr.op == AST::UnaryOp::PLUS) {
      // Nothing to do
      *this << *expr.expr;
    } else if (expr.op == AST::UnaryOp::MINUS) {
      *this << *expr.expr << ".negate()";
    } else {
      assert(0);
    }
    return;
  }

  GenericPrinter::print(expr);
}

void MasonPrinter::print(const AST::AgentCreationExpression &expr) {
  AST::AgentDeclaration *agent = expr.type.getAgentDecl();

  *this << "new " << expr.name << "(";
  printCommaSeparated(*agent->members, [&](const AST::AgentMemberPtr &member) {
    auto it = expr.memberMap.find(member->name);
    assert(it != expr.memberMap.end());
    *this << *it->second;
  });
  *this <<")";
}

void MasonPrinter::print(const AST::VarDeclarationStatement &stmt) {
  *this << *stmt.type << " " << *stmt.var;
  if (stmt.initializer) {
    *this << " = " << *stmt.initializer;
  }
  *this << ";";
}

void MasonPrinter::print(const AST::ForStatement &stmt) {
  if (stmt.isNear()) {
    AST::AgentDeclaration *agentDecl = stmt.type->resolved.getAgentDecl();
    std::string iLabel = makeAnonLabel();
    AST::Expression &nearAgent = stmt.getNearAgent();
    AST::AgentDeclaration *nearAgentDecl = nearAgent.type.getAgentDecl();
    AST::Expression &nearRadius = stmt.getNearRadius();

    *this << "Bag _bag = _sim.env.getNeighborsWithinDistance("
          << nearAgent << "." << nearAgentDecl->getPositionMember()->name
          << ", " << nearRadius << ");" << nl
          << "for (int " << iLabel << " = 0; " << iLabel << " < _bag.size(); "
          << iLabel << "++) {" << indent << nl
          << agentDecl->name << " " << *stmt.var << " = "
          << "(" << agentDecl->name << ") _bag.get(" << iLabel << ");" << nl
          << *stmt.stmt
          << outdent << nl << "}";
  } else if (stmt.isRange()) {
    std::string eLabel = makeAnonLabel();
    auto range = stmt.getRange();
    *this << "for (int " << *stmt.var << " = " << range.first
          << ", " << eLabel << " = " << range.second << "; "
          << *stmt.var << " < " << eLabel << "; ++" << *stmt.var << ") "
          << *stmt.stmt;
  } else {
    // TODO Ordinary collection loop
    assert(0);
  }
}

void MasonPrinter::print(const AST::ConstDeclaration &decl) {
  *this << "public static " << *decl.type << " " << *decl.var
        << " = " << *decl.expr << ";";
}

void MasonPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.isStep) {
    const AST::Param &param = decl.stepParam();
    AST::AgentDeclaration &agent = decl.stepAgent();
    AST::AgentMember *posMember = agent.getPositionMember();

    currentInVar = param.var->id;
    currentOutVar = param.outVar->id;

    *this << "public void " << decl.name << "(SimState state) {" << indent << nl
          << "Sim _sim = (Sim) state;"
          << *decl.stmts;
    if (posMember) {
      *this << nl << "_sim.env.setObjectLocation(this, this." << posMember->name << ");";
    }
    *this << outdent << nl << "}";
    
    currentInVar.reset();
    currentOutVar.reset();
  } else {
    *this << "public " << *decl.returnType << " " << decl.name << "(";
    printParams(decl);
    *this << ") {" << indent << *decl.stmts << outdent << nl << "}";
  }
}

void MasonPrinter::print(const AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

void MasonPrinter::print(const AST::AgentDeclaration &decl) {
  inAgent = true;

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl << nl
        << "public class " << decl.name << " implements Steppable {" << indent
        << *decl.members << nl << nl;

  // Print constructor
  *this << "public " << decl.name << "(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << *member->type << " " << member->name;
  });
  *this << ") {" << indent;
  for (AST::AgentMemberPtr &member : *decl.members) {
    *this << nl << "this." << member->name << " = " << member->name << ";";
  }
  *this << outdent << nl << "}" << nl;

  // Print main step functions that dispatches to sub-steps
  *this << "public void step(SimState state) {" << indent;
  for (const AST::FunctionDeclaration *stepFn : script.simStmt->stepFuncDecls) {
    if (&stepFn->stepAgent() == &decl) {
      *this << nl << stepFn->name << "(state);";
    }
  }
  *this << outdent << nl << "}";

  for (AST::FunctionDeclaration *fn : script.funcs) {
    if (fn->isStep && &fn->stepAgent() == &decl) {
      *this << nl << nl << *fn;
    }
  }

  *this << outdent << nl << "}";
}

void MasonPrinter::print(const AST::SimulateStatement &) {
  // TODO
}

void MasonPrinter::print(const AST::Script &script) {
  inAgent = false; // Printing main simulation code

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl
        << "import sim.field.continuous.*;" << nl << nl
        << "public class Sim extends SimState {" << indent << nl;

  for (const AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  // TODO Replace dummy granularity
  if (script.envDecl && script.envDecl->sizeExpr) {
    const AST::CallExpression &expr =
      *dynamic_cast<const AST::CallExpression *>(&*script.envDecl->sizeExpr);
    int vecLen = expr.type.getVecLen();
    *this << "public Continuous" << vecLen << "D env = new Continuous" << vecLen << "D(1.0, ";
    printVecCtorArgs(*this, expr);
    *this << ");" << nl << nl;
  }

  *this << "public Sim(long seed) {" << indent << nl
        << "super(seed);"
        << outdent << nl << "}" << nl << nl
        << "public void start() {" << indent << nl
        << "super.start();";

  // TODO This needs to be split in start + end
  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  if (mainFunc) {
    *this << *mainFunc->stmts;
  }

  *this << outdent << nl << "}" << nl
        << "public static void main(String[] args) {" << indent
        << nl << "doLoop(Sim.class, args);"
        << nl << "System.exit(0);"
        << outdent << nl << "}";

  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isStep && !decl->isMain()) {
      *this << nl << nl << *decl;
    }
  }

  *this << outdent << nl << "}";
}

}

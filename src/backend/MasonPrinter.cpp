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

#include "MasonPrinter.hpp"

namespace OpenABL {

void MasonPrinter::print(const AST::MemberInitEntry &) {}
void MasonPrinter::print(const AST::NewArrayExpression &) {}

void MasonPrinter::printType(Type type) {
  switch (type.getTypeId()) {
    case Type::VOID:
      *this << "void";
      return;
    case Type::BOOL:
      *this << "boolean";
      return;
    case Type::INT32:
      *this << "int";
      return;
    case Type::FLOAT:
      *this << "double";
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
    case Type::AGENT:
      *this << type.getAgentDecl()->name;
      return;
    case Type::ARRAY:
      printType(type.getBaseType());
      *this << "[]";
      return;
    default:
      assert(0); // TODO
  }
}

void MasonPrinter::print(const AST::VarExpression &expr) {
  VarId id = expr.var->id;
  const ScopeEntry &entry = script.scope.get(id);
  if (entry.isGlobal) {
    *this << "Sim." << *expr.var;
  } else {
    *this << *expr.var;
  }
}

bool MasonPrinter::isSpecialBinaryOp(
    AST::BinaryOp, const AST::Expression &left, const AST::Expression &right) {
  return left.type.isVec() || right.type.isVec();
}
void MasonPrinter::printSpecialBinaryOp(
    AST::BinaryOp op, const AST::Expression &left, const AST::Expression &right) {
  if (op == AST::BinaryOp::NOT_EQUALS) {
    // Map to !equals()
    *this << "!" << left << ".equals(" << right << ")";
    return;
  }

  *this << left << ".";
  switch (op) {
    case AST::BinaryOp::ADD: *this << "add"; break;
    case AST::BinaryOp::SUB: *this << "subtract"; break;
    case AST::BinaryOp::MUL: *this << "multiply"; break;
    case AST::BinaryOp::DIV:
      // Emulate divide via multiply by reciprocal
      *this << "multiply(1. / " << right << ")";
      return;
    case AST::BinaryOp::EQUALS: *this << "equals"; break;
    default:
      assert(0);
  }
  *this << "(" << right << ")";
}

static void printVecCtorArgs(MasonPrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  int vecLen = t.getVecLen();
  size_t numArgs = expr.args->size();
  if (numArgs == 1) {
    // TODO Multiple evaluation
    const AST::Expression &arg = expr.getArg(0);
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
  } else if (t.isInt() && expr.getArg(0).type.isBool()) {
    // Java does not allow bool to int casts, even explicitly
    p << "(" << expr.getArg(0) << " ? 1 : 0)";
  } else {
    p << "(" << t << ") " << expr.getArg(0);
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
    } else if (name == "random" || name == "randomInt") {
      *this << "Util." << name << "(" << getSimVarName() << ".random, ";
      printArgs(expr);
      *this << ")";
    } else if (name == "sin" || name == "cos" || name == "tan"
            || name == "sinh" || name == "cosh" || name == "tanh"
            || name == "asin" || name == "acos" || name == "atan"
            || name == "exp" || name == "log" || name == "sqrt"
            || name == "cbrt" || name == "round" || name == "pow"
            || name == "min" || name == "max"
    ) {
      // Basic math functions
      *this << "java.lang.Math." << name << "(";
      printArgs(expr);
      *this << ")";
    } else if (name == "add") {
      std::string aLabel = makeAnonLabel();
      const AST::Expression &arg = expr.getArg(0);
      Type type = arg.type;
      AST::AgentDeclaration *agent = type.getAgentDecl();
      AST::AgentMember *posMember = agent->getPositionMember();
      const std::string &simVar = getSimVarName();
      bool isRuntimeAdd = !inMain;

      *this << "final " << type << " " << aLabel << " = " << arg << ";" << nl;
      if (posMember) {
        *this << simVar << ".env.setObjectLocation(" << aLabel << ", "
              << aLabel << ".getInState()." << posMember->name << ");" << nl;
      }

      std::string scheduleVar = simVar + ".schedule";
      std::string time = isRuntimeAdd ? scheduleVar + ".getTime() + 1.0" : "Schedule.EPOCH";

      if (agent->usesRuntimeRemoval) {
        *this << scheduleVar << ".scheduleOnce(" << time << ", " << aLabel << ")";
      } else {
        *this << scheduleVar << ".scheduleRepeating(" << time << ", " << aLabel << ")";
      }
    } else if (name == "save") {
      *this << "Util.save(env.getAllObjects(), " << expr.getArg(0) << ")";
    } else if (name == "removeCurrent") {
      *this << "_isDead = true";
    } else if (name == "count") {
      Type type = expr.getArg(0).type;
      if (expr.getNumArgs() == 2) {
        AST::AgentDeclaration *decl = type.getAgentDecl();
        AST::AgentMember *member = type.getAgentMember();
        *this << "count" << decl->name << "_" << member->name << "("
              << expr.getArg(1) << ")";
      } else {
        AST::AgentDeclaration *decl = type.getAgentDecl();
        *this << "count" << decl->name << "()";
      }
    } else if (name == "sum") {
      Type type = expr.getArg(0).type;
      AST::AgentDeclaration *decl = type.getAgentDecl();
      AST::AgentMember *member = type.getAgentMember();
      *this << "sum" << decl->name << "_" << member->name << "()";
    } else if (name == "log_csv") {
      bool first = true;
      for (const AST::ExpressionPtr &arg : *expr.args) {
        if (!first) {
          *this << "logWriter.print(',');" << nl;
        }
        first = false;
        *this << "logWriter.print(" << *arg << ");" << nl;
      }
      *this << "logWriter.println()";
    } else if (name == "getLastExecTime") {
      *this << "lastExecTime";
    } else {
      assert(0);
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

  GenericPrinter::print(stmt);
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
    const AST::Expression &nearAgent = stmt.getNearAgent();
    AST::AgentDeclaration *nearAgentDecl = nearAgent.type.getAgentDecl();
    const AST::Expression &nearRadius = stmt.getNearRadius();

    *this << "Bag _bag = _sim.env.getNeighborsExactlyWithinDistance("
          << nearAgent << "." << nearAgentDecl->getPositionMember()->name
          << ", " << nearRadius << ");" << nl
          << "for (int " << iLabel << " = 0; " << iLabel << " < _bag.size(); "
          << iLabel << "++) {" << indent << nl
          << "Object _agent = _bag.get(" << iLabel << ");" << nl;
    if (script.agents.size() > 1) {
      // If there is more than one agent type, we have to check that we only the agents that
      // were asked for
      *this << "if (!(_agent instanceof " << agentDecl->name << ")) continue;" << nl;
    }
    *this << agentDecl->name << ".State " << *stmt.var << " = "
          << "((" << agentDecl->name << ") _agent)"
          << (agentDecl == nearAgentDecl ? ".getState(currentState);" : ".getInState();") << nl
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
  if (decl.isParallelStep()) {
    const AST::Param &param = decl.stepParam();
    AST::AgentDeclaration &agent = decl.stepAgent();
    AST::AgentMember *posMember = agent.getPositionMember();

    currentInVar = param.var->id;
    currentOutVar = param.outVar->id;

    // Use a leading "_" to avoid clashes with existing methods like "step()"
    *this << "public void _" << decl.name << "(SimState state) {" << indent << nl
          << "Sim _sim = (Sim) state;" << nl
          << "prepareOutState();" << nl
          << "State " << *param.var << " = getInState();" << nl
          << "State " << *param.outVar << " = getOutState();"
          << *decl.stmts;
    if (posMember) {
      if (agent.usesRuntimeRemoval) {
        *this << nl << "if (_isDead) {" << indent << nl
              << "_sim.env.remove(this);"
              << outdent << nl << "} else {" << indent
              << nl << "_sim.env.setObjectLocation(this, "
              << *param.outVar << "." << posMember->name << ");"
              << nl << "_sim.schedule.scheduleOnceIn(1.0, this);"
              << nl << "swapStates();"
              << outdent << nl << "}";
      } else {
        *this << nl << "_sim.env.setObjectLocation(this, "
              << *param.outVar << "." << posMember->name << ");"
              << nl << "swapStates();";
      }
    }
    *this << outdent << nl << "}";

    currentInVar.reset();
    currentOutVar.reset();
  } else if (decl.name == "getColor" || decl.name == "getSize") {
    const AST::Param &param = *(*decl.params)[0];
    *this << "public " << *decl.returnType << " " << decl.name << "("
          << *param.type << " _" << *param.var << ") {" << indent
          << nl << *param.type << ".State " << *param.var
          << " = _" << *param.var << ".getInState();"
          << *decl.stmts << outdent << nl << "}";
  } else {
    *this << "public " << *decl.returnType << " " << decl.name << "(";
    printParams(decl);
    *this << ") {" << indent << *decl.stmts << outdent << nl << "}";
  }
}

void MasonPrinter::print(const AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

void MasonPrinter::printAgentImports() {
  *this << "import java.io.Serializable;" << nl
        << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl;
}
void MasonPrinter::printAgentExtends(const AST::AgentDeclaration &) {
  *this << " implements Steppable";
}
void MasonPrinter::printAgentExtraCode(const AST::AgentDeclaration &) { }
void MasonPrinter::printAgentExtraCtorArgs() { }
void MasonPrinter::printAgentExtraCtorCode() { }
void MasonPrinter::printStepDefaultCode(const AST::AgentDeclaration &decl) {
  if (decl.usesRuntimeRemoval) {
    // Make sure we always reschedule the agent, even if no step function is run
    *this << "((Sim) state).schedule.scheduleOnceIn(1.0, this);" << nl;
  }
}

void MasonPrinter::print(const AST::AgentDeclaration &decl) {
  const auto &stepFns = script.simStmt->stepFuncDecls;

  inAgent = true;

  printAgentImports();
  *this << nl
        << "public class " << decl.name;
  printAgentExtends(decl);
  *this << " {" << indent << nl
        << "public class State implements Serializable {" << indent
        << *decl.members << nl;

  // State constructor
  *this << "State(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << *member->type << " " << member->name;
  });
  *this << ") {" << indent;
  for (AST::AgentMemberPtr &member : *decl.members) {
    *this << nl << "this." << member->name << " = " << member->name << ";";
  }
  *this << outdent << nl << "}"
        << outdent << nl << "}" << nl
        << "State state0;" << nl
        << "State state1;" << nl
        << "int currentState = 0;" << nl;
  if (stepFns.size() != 1) {
    *this << "int currentStep = 0;" << nl;
  }
  if (decl.usesRuntimeRemoval) {
    *this << "public boolean _isDead = false;" << nl;
  }

  printAgentExtraCode(decl);

  // Agent constructor
  *this << "public " << decl.name << "(";
  printAgentExtraCtorArgs();
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << *member->type << " " << member->name;
  });
  *this << ") {" << indent << nl;
  printAgentExtraCtorCode();
  *this << "state0 = new State(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << member->name;
  });
  *this << ");" << nl
        << "state1 = new State(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << member->name;
  });
  *this << ");"
        << outdent << nl << "}" << nl;

  // Add some helper functions
  *this << "public State getState(int n) {" << indent << nl
        << "return n == 0 ? state0 : state1;"
        << outdent << nl << "}" << nl
        << "public State getInState() {" << indent << nl
        << "return currentState == 0 ? state0 : state1;"
        << outdent << nl << "}" << nl
        << "State getOutState() {" << indent << nl
        << "return currentState == 0 ? state1 : state0;"
        << outdent << nl << "}" << nl
        << "void prepareOutState() {" << indent << nl
        << "State outState = getOutState();" << nl
        << "State inState = getInState();";
  for (const AST::AgentMemberPtr &member : *decl.members) {
    *this << nl << "outState." << member->name << " = " << "inState." << member->name << ";";
  };
  *this << outdent << nl << "}" << nl
        << "void swapStates() {" << indent << nl
        << "currentState ^= 1;"
        << outdent << nl << "}" << nl
        << "public void step(SimState state) {" << indent << nl;
  if (stepFns.size() == 1) {
    // If there is only one step function, just call it directly
    const AST::FunctionDeclaration *stepFn = script.simStmt->stepFuncDecls[0];
    *this << "_" << stepFn->name << "(state);";
  } else {
    // Otherwise cycle through step functions with a counter
    *this << "switch (currentStep) {" << indent;
    for (size_t i = 0; i < stepFns.size(); i++) {
      *this << nl << "case " << i << ":" << indent << nl;
      const auto *stepFn = stepFns[i];
      if (&stepFn->stepAgent() == &decl) {
        *this << "_" << stepFn->name << "(state);" << nl;
      } else {
        printStepDefaultCode(decl);
      }
      *this << "break;" << outdent;
    }
    *this << outdent << nl << "}" << nl
          << "currentStep++;" << nl
          << "if (currentStep == " << stepFns.size() << ") {" << indent << nl
          << "currentStep = 0;"
          << outdent << nl << "}";
  }
  *this << outdent << nl << "}";

  for (AST::FunctionDeclaration *fn : script.funcs) {
    if (fn->isParallelStep() && &fn->stepAgent() == &decl) {
      *this << nl << nl << *fn;
    }
  }

  *this << outdent << nl << "}";
}

void MasonPrinter::print(const AST::SimulateStatement &stmt) {
  AST::FunctionDeclaration *seqStep = stmt.seqStepDecl;
  size_t numStepFuncs = stmt.stepFuncDecls.size();
  std::string tLabel = makeAnonLabel();
  std::string timerLabel = makeAnonLabel();
  *this << "int " << tLabel << " = " << *stmt.timestepsExpr
        << " * " << numStepFuncs << ";" << nl
        << "long lastTime = System.currentTimeMillis();" << nl
        << "do {" << indent << nl
        << "if (!_sim.schedule.step(_sim)) break;" << nl
        << "if (_sim.schedule.getSteps() % " << numStepFuncs << " == 0) {" << indent << nl
        << "long curTime = System.currentTimeMillis();" << nl
        << "_sim.lastExecTime = (curTime - lastTime) / 1000.0;" << nl
        << "lastTime = curTime;";
  if (seqStep) {
    *this << "_sim." << seqStep->name << "();" << nl;
  }
  *this << outdent << nl << "}"
        << outdent << nl << "} while (_sim.schedule.getSteps() < " << tLabel << ");";
}

void MasonPrinter::print(const AST::Script &script) {
  inAgent = false; // Printing main simulation code

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl
        << "import sim.field.continuous.*;" << nl;
  if (script.usesLogging) {
    *this << "import java.io.*;" << nl;
  }
  *this << nl << "public class Sim extends SimState {" << indent << nl;

  for (const AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  const auto *envDecl = script.envDecl;
  if (envDecl && envDecl->envSize.isValid()) {
    const Value &size = envDecl->envSize;
    int vecLen = size.getType().getVecLen();
    *this << "public Continuous" << vecLen << "D env = new Continuous" << vecLen << "D("
          << envDecl->envGranularity << ", ";
    printCommaSeparated(size.getVec(), [&](double d) {
        *this << d;
    });
    *this << ");" << nl;
  }

  *this << "private double lastExecTime;" << nl;
  if (script.usesLogging) {
    *this << "private PrintWriter logWriter;" << nl;
  }

  *this << nl << "public Sim(long seed) {" << indent << nl
        << "super(seed);"
        << outdent << nl << "}" << nl;

  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  inMain = true;
  *this << "public void start() {" << indent
        << nl << "super.start();"
        << nl << "env.clear();";
  if (script.usesLogging) {
    *this << nl << "try {"
          << nl << "    logWriter = new PrintWriter(\"log.csv\", \"UTF-8\");"
          << nl << "} catch (Exception e) { e.printStackTrace(); }";
  }
  *this << nl << mainFunc->getStmtsBeforeSimulate()
        << outdent << nl << "}"
        << nl << "public void finish() {" << indent
        << nl << "super.finish();";
  if (script.usesLogging) {
    *this << nl << "logWriter.close();";
  }
  *this << nl << mainFunc->getStmtsAfterSimulate()
        << outdent << nl << "}"
        << nl << "public static void main(String[] args) {" << indent
        << nl << "Sim _sim = new Sim(System.currentTimeMillis());"
        << nl << "_sim.start();"
        << nl << *script.simStmt
        << nl << "_sim.finish();"
        << nl << "System.exit(0);"
        << outdent << nl << "}"
        << nl << "public static int getColor(Object obj) {" << indent
        << nl << "return 0;"
        << outdent << nl << "}"
        << nl << "public static int getSize(Object obj) {" << indent
        << nl << "return 1;"
        << outdent << nl << "}" << nl;
  inMain = false;

  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isParallelStep() && !decl->isMain()) {
      *this << nl << *decl << nl;
    }
  }

  // Print reducton helper functions
  for (const ReductionInfo &info : script.reductions) {
    ReductionKind kind = info.first;
    Type type = info.second;
    if (kind == ReductionKind::COUNT_TYPE) {
      AST::AgentDeclaration *decl = type.getAgentDecl();
      *this << nl << "public int count" << decl->name << "() {" << indent << nl
            << "Bag bag = env.getAllObjects();" << nl
            << "int count = 0;" << nl
            << "for (int i = 0; i < bag.size(); i++) {" << indent << nl
            << "Object agent = bag.get(i);" << nl
            << "if (agent instanceof " << decl->name << ") count++;"
            << outdent << nl << "}" << nl
            << "return count;"
            << outdent << nl << "}" << nl;
    } else if (kind == ReductionKind::COUNT_MEMBER) {
      AST::AgentDeclaration *decl = type.getAgentDecl();
      AST::AgentMember *member = type.getAgentMember();
      Type memberType = member->type->resolved;
      *this << nl << "public int count" << decl->name << "_" << member->name << "("
            << memberType << " value) {" << indent << nl
            << "Bag bag = env.getAllObjects();" << nl
            << "int count = 0;" << nl
            << "for (int i = 0; i < bag.size(); i++) {" << indent << nl
            << "Object maybe_agent = bag.get(i);" << nl
            << "if (!(maybe_agent instanceof " << decl->name << ")) continue;"
            << decl->name << " agent = (" << decl->name << ") maybe_agent;" << nl
            << "if (agent.getInState()." << member->name << " == value) count++;" << nl
            << outdent << nl << "}" << nl
            << "return count;"
            << outdent << nl << "}" << nl;
    } else if (kind == ReductionKind::SUM_MEMBER) {
      AST::AgentDeclaration *decl = type.getAgentDecl();
      AST::AgentMember *member = type.getAgentMember();
      Value identity = Value::getSumIdentity(member->type->resolved);
      Type resultType = identity.getType();
      AST::Expression *identityExpr = identity.toExpression();
      *this << nl << "public " << resultType << " sum"
            << decl->name << "_" << member->name << "() {" << indent << nl
            << "Bag bag = env.getAllObjects();" << nl
            << resultType << " result = " << *identityExpr << ";" << nl
            << "for (int i = 0; i < bag.size(); i++) {" << indent << nl
            << "Object maybe_agent = bag.get(i);" << nl
            << "if (!(maybe_agent instanceof " << decl->name << ")) continue;" << nl
            << decl->name << " agent = (" << decl->name << ") maybe_agent;" << nl;
      if (member->type->resolved.isVec()) {
        *this << "result = result.add(agent.getInState()." << member->name << ");";
      } else if (member->type->resolved.isBool()) {
        *this << "result += agent.getInState()." << member->name << " ? 1 : 0;";
      } else {
        *this << "result += agent.getInState()." << member->name << ";";
      }
      *this << outdent << nl << "}" << nl
            << "return result;"
            << outdent << nl << "}" << nl;
      delete identityExpr;
    } else {
      assert(0);
    }
  }

  *this << outdent << nl << "}";
}

void MasonPrinter::printUIExtraImports() { }
void MasonPrinter::printUICtors() {
  *this <<
  "    public static void main(String[] args) {\n"
  "        SimWithUI vid = new SimWithUI();\n"
  "        Console c = new Console(vid);\n"
  "        c.setVisible(true);\n"
  "    }\n"
  "\n"
  "    public SimWithUI() {\n"
  "        super(new Sim(System.currentTimeMillis()));\n"
  "    }\n"
  "    public SimWithUI(SimState state) {\n"
  "        super(state);\n"
  "    }\n";
}

void MasonPrinter::printUI() {
  int dim = script.envDecl->envDimension;
  size_t numSteps = script.simStmt->stepFuncDecls.size();
  *this <<
  "import sim.portrayal.continuous.*;\n"
  "import sim.portrayal.simple.*;\n"
  "import sim.portrayal.*;\n"
  "import sim.portrayal3d.continuous.*;\n"
  "import sim.portrayal3d.simple.*;\n"
  "import sim.engine.*;\n"
  "import sim.display.*;\n"
  "import sim.display3d.*;\n"
  "import javax.swing.*;\n"
  "import java.awt.*;\n";
  if (dim == 3) {
    *this << "import javax.media.j3d.TransformGroup;\n";
  }
  printUIExtraImports();
  *this <<
  "\n"
  "public class SimWithUI extends GUIState {\n"
  "    class MyDisplay extends Display" << dim << "D {\n"
  "        MyDisplay(double width, double height, GUIState sim) {\n"
  "            super(width, height, sim);\n"
  "            updateRule = Display2D.UPDATE_RULE_STEPS;\n"
  "            stepInterval = " << numSteps << ";\n"
  "        }\n"
  "    }\n"
  "    private static final int SIZE = 500;\n"
  "    public Display" << dim << "D display;\n"
  "    public JFrame displayFrame;\n"
  "    public static String name = \"Visualization\";\n"
  "    ContinuousPortrayal" << dim << "D envPortrayal = new ContinuousPortrayal" << dim << "D();\n"
  "\n";
  printUICtors();
  *this <<
  "\n"
  "    public static String getName() {\n"
  "        return name;\n"
  "    }\n"
  "\n"
  "    public void start() {\n"
  "        super.start();\n"
  "        setupPortrayals();\n"
  "    }\n"
  "    public void load(SimState state) {\n"
  "        super.load(state);\n"
  "        setupPortrayals();\n"
  "    }\n"
  "\n"
  "    public void setupPortrayals() {\n"
  "        Sim sim = (Sim) state;\n"
  "        final double defaultScale = 4 * sim.env.width / SIZE;\n"
  "\n"
  "        envPortrayal.setField(sim.env);\n";

  for (const AST::AgentDeclaration *agent : script.agents) {
    if (dim == 2) {
      *this << "        envPortrayal.setPortrayalForClass("
            << agent->name << ".class, new OvalPortrayal2D() {\n"
            "            public void draw(Object object, Graphics2D graphics, DrawInfo2D info) {\n"
            "                " << agent->name << " agent = (" << agent->name << ") object;\n"
            "                paint = new Color(sim.getColor(agent));\n"
            "                scale = defaultScale * sim.getSize(agent);\n"
            "                super.draw(object, graphics, info);\n"
            "            }\n"
            "        });\n";
    } else {
      *this << "        envPortrayal.setPortrayalForClass("
            << agent->name << ".class, new SpherePortrayal3D() {\n"
            "            public TransformGroup getModel(Object object, TransformGroup j3dModel) {\n"
            "                " << agent->name << " agent = (" << agent->name << ") object;\n"
            "                Color color = new Color(sim.getColor(agent));\n"
            "                setAppearance(null, appearanceForColor(color));\n"
            "                setScale(null, defaultScale * sim.getSize(agent));\n"
            "                return super.getModel(object, j3dModel);\n"
            "            }\n"
            "        });\n";
    }
  }
  *this << "\n";

  if (dim == 2) {
    *this << "        display.repaint();\n";
  } else {
    *this << "        display.createSceneGraph();\n";
    *this << "        display.reset();\n";
  }

  *this <<
  "    }\n"
  "\n"
  "    public void init(Controller c) {\n"
  "        super.init(c);\n"
  "\n"
  "        display = new MyDisplay(SIZE, SIZE, this);\n"
  "        display.setBackdrop(Color.white);\n"
  "        //display.setClipping(false);\n";
  if (dim == 3) {
    Value::Vec3 envMin = script.envDecl->envMin.extendToVec3().getVec3();
    Value::Vec3 envSize = script.envDecl->envSize.extendToVec3().getVec3();
    double translate_x = -envSize.x / 2.0 + envMin.x;
    double translate_y = -envSize.y / 2.0 + envMin.y;
    double maxSize = std::max(envSize.x, envSize.y);
    *this << "        display.translate(" << translate_x << ", " << translate_y << ", 0);\n"
          << "        display.scale(1.0 / " << maxSize << ");";
  }
  *this <<
  "\n"
  "        displayFrame = display.createFrame();\n"
  "        displayFrame.setTitle(\"Visualization Display\");\n"
  "        c.registerFrame(displayFrame);\n"
  "        displayFrame.setVisible(true);\n"
  "        display.attach(envPortrayal, \"Environment\");\n"
  "    }\n"
  "\n"
  "    public void quit() {\n"
  "        super.quit();\n"
  "        if (displayFrame != null) displayFrame.dispose();\n"
  "        displayFrame = null;\n"
  "        display = null;\n"
  "    }\n"
  "}\n";
}

}

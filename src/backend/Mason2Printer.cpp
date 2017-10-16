#include "Mason2Printer.hpp"

namespace OpenABL {

void Mason2Printer::print(const AST::MemberInitEntry &) {}
void Mason2Printer::print(const AST::NewArrayExpression &) {}

void Mason2Printer::printType(Type type) {
  switch (type.getTypeId()) {
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

void Mason2Printer::print(const AST::VarExpression &expr) {
  VarId id = expr.var->id;
  const ScopeEntry &entry = script.scope.get(id);
  if (entry.isGlobal) {
    *this << "Sim." << *expr.var;
  } else {
    *this << *expr.var;
  }
}

bool Mason2Printer::isSpecialBinaryOp(
    AST::BinaryOp, const AST::Expression &left, const AST::Expression &right) {
  return left.type.isVec() || right.type.isVec();
}
void Mason2Printer::printSpecialBinaryOp(
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

static void printVecCtorArgs(Mason2Printer &p, const AST::CallExpression &expr) {
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

static void printTypeCtor(Mason2Printer &p, const AST::CallExpression &expr) {
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

void Mason2Printer::print(const AST::CallExpression &expr) {
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

      *this << "final Schedule _schedule = " << simVar << ".schedule;" << nl
            << "double _time = "
            << (isRuntimeAdd ? "_schedule.getTime() + 1" : "_schedule.EPOCH") << ";" << nl;

      size_t numStepFns = script.simStmt->stepFuncDecls.size();
      for (size_t i = 0; i < numStepFns; i++) {
        const AST::FunctionDeclaration *stepFn = script.simStmt->stepFuncDecls[i];
        const AST::AgentDeclaration &stepAgent = stepFn->stepAgent();
        if (&stepAgent != agent) {
          continue;
        }

        if (stepAgent.usesRuntimeRemoval) {
          // If runtime removal is used, we scheduleOnce and check the _isDead flag
          *this << "_schedule.scheduleOnce(_time, " << i
                << ", new Steppable() {" << indent << nl
                << "public void step(SimState state) {" << indent << nl
                << aLabel << "._" << stepFn->name << "(state);" << nl
                << "if (!" << aLabel << "._isDead) {" << indent << nl
                << "_schedule.scheduleOnceIn(1.0, this, " << i << ");"
                << outdent << nl << "}"
                << outdent << nl << "}"
                << outdent << nl << "})" << nl;
        } else {
          *this << "_schedule.scheduleRepeating(_time, " << i
                << ", new Steppable() {" << indent << nl
                << "public void step(SimState state) {" << indent << nl
                << aLabel << "._" << stepFn->name << "(state);"
                << outdent << nl << "}"
                << outdent << nl << "})";
        }
        if (i != numStepFns - 1) {
          *this << ";" << nl;
        }
      }
    } else if (name == "save") {
      *this << "Util.save(env.getAllObjects(), " << expr.getArg(0) << ")";
    } else if (name == "removeCurrent") {
      *this << "_isDead = true";
    } else {
      assert(0);
    }
  } else {
    *this << getSimVarName() << "." << expr.name << "(";
    printArgs(expr);
    *this << ")";
  }
}

void Mason2Printer::print(const AST::AssignStatement &stmt) {
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

void Mason2Printer::print(const AST::AssignOpStatement &stmt) {
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

void Mason2Printer::print(const AST::UnaryOpExpression &expr) {
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

void Mason2Printer::print(const AST::AgentCreationExpression &expr) {
  AST::AgentDeclaration *agent = expr.type.getAgentDecl();

  *this << "new " << expr.name << "(";
  printCommaSeparated(*agent->members, [&](const AST::AgentMemberPtr &member) {
    auto it = expr.memberMap.find(member->name);
    assert(it != expr.memberMap.end());
    *this << *it->second;
  });
  *this <<")";
}

void Mason2Printer::print(const AST::VarDeclarationStatement &stmt) {
  *this << *stmt.type << " " << *stmt.var;
  if (stmt.initializer) {
    *this << " = " << *stmt.initializer;
  }
  *this << ";";
}

void Mason2Printer::print(const AST::ForStatement &stmt) {
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

void Mason2Printer::print(const AST::ConstDeclaration &decl) {
  *this << "public static " << *decl.type << " " << *decl.var
        << " = " << *decl.expr << ";";
}

void Mason2Printer::print(const AST::FunctionDeclaration &decl) {
  if (decl.isStep) {
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
        *this << nl << "if (_isDead) { _sim.env.remove(this); return; }";
      }
      *this << nl << "_sim.env.setObjectLocation(this, getOutState()." << posMember->name << ");";
      *this << nl << "swapStates();";
    }
    *this << outdent << nl << "}";
    
    currentInVar.reset();
    currentOutVar.reset();
  } else if (decl.name == "getColor") {
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

void Mason2Printer::print(const AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

void Mason2Printer::print(const AST::AgentDeclaration &decl) {
  inAgent = true;

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl << nl
        << "public class " << decl.name << " {" << indent << nl
        << "public class State {" << indent
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
        << "int currentState;" << nl;
  if (decl.usesRuntimeRemoval) {
    *this << "public boolean _isDead = false;" << nl;
  }

  // Agent constructor
  *this << "public " << decl.name << "(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << *member->type << " " << member->name;
  });
  *this << ") {" << indent << nl
        << "state0 = new State(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << member->name;
  });
  *this << ");" << nl
        << "state1 = new State(";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << member->name;
  });
  *this << ");" << nl
        << "currentState = 0;"
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
        << outdent << nl << "}";

  for (AST::FunctionDeclaration *fn : script.funcs) {
    if (fn->isStep && &fn->stepAgent() == &decl) {
      *this << nl << nl << *fn;
    }
  }

  *this << outdent << nl << "}";
}

void Mason2Printer::print(const AST::SimulateStatement &stmt) {
  *this << "do {" << indent << nl
        << "if (!_sim.schedule.step(_sim)) break;" << outdent << nl
        << "} while (_sim.schedule.getSteps() < "
        << *stmt.timestepsExpr << ");";
}

void Mason2Printer::print(const AST::Script &script) {
  inAgent = false; // Printing main simulation code

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl
        << "import sim.field.continuous.*;" << nl << nl
        << "public class Sim extends SimState {" << indent << nl;

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
    *this << ");" << nl << nl;
  }

  *this << "public Sim(long seed) {" << indent << nl
        << "super(seed);"
        << outdent << nl << "}" << nl;

  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  inMain = true;
  *this << "public void start() {" << indent
        << nl << "super.start();"
        << nl << "env.clear();"
        << nl << mainFunc->getStmtsBeforeSimulate()
        << outdent << nl << "}"
        << nl << "public void finish() {" << indent
        << nl << "super.finish();"
        << nl << mainFunc->getStmtsAfterSimulate()
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
        << outdent << nl << "}";
  inMain = false;

  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isStep && !decl->isMain()) {
      *this << nl << nl << *decl;
    }
  }

  *this << outdent << nl << "}";
}

void Mason2Printer::printUI() {
  int dim = script.envDecl->envDimension;
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
  "import java.awt.*;\n"
  "\n"
  "public class SimWithUI extends GUIState\n"
  "{\n"
  "    private static final int SIZE = 500;\n"
  "    public Display" << dim << "D display;\n"
  "    public JFrame displayFrame;\n"
  "    ContinuousPortrayal" << dim << "D envPortrayal = new ContinuousPortrayal" << dim << "D();\n"
  "\n"
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
  "    }\n"
  "    public static String getName() {\n"
  "        return \"Visualization\";\n"
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
  "        double scale = 4 * sim.env.width / SIZE;\n"
  "\n"
  "        envPortrayal.setField(sim.env);\n";
  if (dim == 2) {
    for (const AST::AgentDeclaration *agent : script.agents) {
      *this << "        envPortrayal.setPortrayalForClass("
            << agent->name << ".class, new OvalPortrayal2D(scale) {\n"
            "            public void draw(Object object, Graphics2D graphics, DrawInfo2D info) {\n"
            "                " << agent->name << " agent = (" << agent->name << ") object;\n"
            "                paint = new Color(sim.getColor(agent));\n"
            "                super.draw(object, graphics, info);\n"
            "            }\n"
            "        });\n";
    }
  } else {
    *this << "        envPortrayal.setPortrayalForAll(new SpherePortrayal3D(scale));\n";
  }
  *this <<
  "\n"
  "        display.reset();\n";

  if (dim == 2) {
    *this << "        display.setBackdrop(Color.white);\n";
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
  "        display = new Display" << dim << "D(SIZE, SIZE, this);\n"
  "        //display.setClipping(false);\n"
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

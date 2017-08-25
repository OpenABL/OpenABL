#include "MasonPrinter.hpp"

namespace OpenABL {

void MasonPrinter::print(const AST::MemberInitEntry &) {}
void MasonPrinter::print(const AST::NewArrayExpression &) {}

void MasonPrinter::printType(Type type) {
  switch (type.getTypeId()) {
    case Type::BOOL:
      *this << "boolean";
      return;
    case Type::INT32:
      *this << "int";
      return;
    case Type::FLOAT:
      // TODO Use doubles, because that's what Mason uses.
      // Need to figure out what semantics we want to have here.
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
    if (id == currentInVar || id == currentOutVar) {
      // TODO Correct mapping for "out" variable
      *this << "this";
      return;
    }

    *this << *expr.var;
  }
}

bool MasonPrinter::isSpecialBinaryOp(
    AST::BinaryOp, const AST::Expression &left, const AST::Expression &right) {
  return left.type.isVec() || right.type.isVec();
}
void MasonPrinter::printSpecialBinaryOp(
    AST::BinaryOp op, const AST::Expression &left, const AST::Expression &right) {
  *this << left << ".";
  switch (op) {
    case AST::BinaryOp::ADD: *this << "add"; break;
    case AST::BinaryOp::SUB: *this << "subtract"; break;
    case AST::BinaryOp::MUL: *this << "multiply"; break;
    case AST::BinaryOp::DIV:
      // Emulate divide via multiply by reciprocal
      *this << "multiply(1. / " << right << ")";
      return;
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
    } else if (name == "random") {
      *this << "Util.random(" << getSimVarName() << ".random, ";
      printArgs(expr);
      *this << ")";
    } else if (name == "sin" || name == "cos" || name == "tan"
            || name == "sinh" || name == "cosh" || name == "tanh"
            || name == "asin" || name == "acos" || name == "atan"
            || name == "exp" || name == "log" || name == "sqrt"
            || name == "round" || name == "pow"
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
      *this << name << "(";
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

    *this << "Bag _bag = _sim.env.getNeighborsWithinDistance("
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
    *this << agentDecl->name << " " << *stmt.var << " = "
          << "(" << agentDecl->name << ") _agent;" << nl
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

void MasonPrinter::print(const AST::SimulateStatement &stmt) {
  *this << "do {" << indent << nl
        << "if (!_sim.schedule.step(_sim)) break;" << outdent << nl
        << "} while (_sim.schedule.getSteps() < "
        << *stmt.timestepsExpr << ");";
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
        << *script.simStmt
        << nl << "_sim.finish();"
        << nl << "System.exit(0);"
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

void MasonPrinter::printUI() {
  int dim = script.envDecl->envDimension;
  *this <<
  "import sim.portrayal.continuous.*;\n"
  "import sim.portrayal.simple.*;\n"
  "import sim.portrayal3d.continuous.*;\n"
  "import sim.portrayal3d.simple.*;\n"
  "import sim.engine.*;\n"
  "import sim.display.*;\n"
  "import sim.display3d.*;\n"
  "import javax.swing.*;\n"
  "import java.awt.Color;\n"
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
    *this << "        envPortrayal.setPortrayalForAll(new OvalPortrayal2D(scale));\n";
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

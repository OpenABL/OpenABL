#include "DMasonPrinter.hpp"

namespace OpenABL {

// XXX: The following code is just copy & pasted from MasonPrinter to have an example of
//      how to extend the code

void DMasonPrinter::print(const AST::AgentDeclaration &decl) {
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

void DMasonPrinter::print(const AST::Script &script) {
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

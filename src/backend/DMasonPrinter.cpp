#include "DMasonPrinter.hpp"

namespace OpenABL {

// XXX: The following code is just copy & pasted from MasonPrinter to have an example of
//      how to extend the code

void DMasonPrinter::printStubAgent(const AST::AgentDeclaration &decl) {
	*this << "import java.io.Serializable;" << nl
	<< "import it.isislab.dmason.sim.engine.DistributedState;" << nl
	<< "import it.isislab.dmason.sim.engine.RemotePositionedAgent;" << nl
	<< "public abstract class Remote"<< decl.name <<"<E>  implements Serializable, RemotePositionedAgent<E> {"<< indent << nl
	<< "private static final long serialVersionUID = 1L;"<< nl
	<< "public E pos; "<< nl
	<< "public String id; " << nl
	<< "public Remote"<< decl.name <<"() {}" << nl
	<< "public Remote"<< decl.name <<"(DistributedState<E> state) {" << indent << nl
	<< "int i=state.nextId();" << nl
	<< "this.id=state.getType().toString()+\"-\"+i;"<< nl
	<< "}"<< outdent << nl
	<< "public E getPos() { return pos; }"<< nl
	<< "public void setPos(E pos) { this.pos = pos; }"<< nl
	<< "public String getId() {return id;}"<< nl
	<< "public void setId(String id) {this.id = id;}"<< nl
	<< "public boolean equals(Object obj) {"<< indent << nl
	<< "if (this == obj) return true;"<< nl
	<< "if (obj == null) return false;"<< nl
	<< "if (getClass() != obj.getClass()) return false;"<< nl
	<< "Remote"<< decl.name <<" other = (Remote"<< decl.name <<") obj;"<< nl
	<< "if (id == null) { "<< nl
	<<"     if (other.id != null) return false;"<< nl
	<< "} else if (!id.equals(other.id)) return false;"<< nl
	<< "if (pos == null) {"<< nl
	<< "     if (other.pos != null) return false;"<< nl
	<< "} else if (!pos.equals(other.pos)) return false;"<< nl
	<< "return true;"<< nl
	<< "}"<< outdent << nl
	<< "}"<< nl << nl;

}
void DMasonPrinter::print(const AST::AgentDeclaration &decl) {
  inAgent = true;

  *this << "import sim.engine.*;" << nl
	<<"import it.isislab.dmason.exception.DMasonException;"<< nl
	<<"import it.isislab.dmason.sim.engine.DistributedState;"<< nl
        << "import sim.util.*;" << nl << nl
        << "public class " << decl.name << " extends  Remote" << decl.name << " {" << indent
        << *decl.members << nl << nl;
  *this << "public " << decl.name << "(){}"<< nl << nl;
  // Print constructor
  *this << "public " << decl.name << "(DistributedState<Double2D> sm,";
  printCommaSeparated(*decl.members, [&](const AST::AgentMemberPtr &member) {
    *this << *member->type << " " << member->name;
  });
  *this << ") {" << indent << nl;
  *this << "super(sm);"<< nl;
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
void DMasonPrinter::print(const AST::FunctionDeclaration &decl) {
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
      *this << nl << "_sim.env.setDistributedObjectLocation(this, this." << posMember->name << ");";
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
static void printTypeCtor(DMasonPrinter &p, const AST::CallExpression &expr) {
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

void DMasonPrinter::print(const AST::CallExpression &expr) {
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

      *this << type << " " << aLabel << " = new " << type << "(this,new Double2D(0,0));" << nl;
      *this << aLabel << ".pos = env.getAvailableRandomLocation();"<< nl; 
      if (posMember) {
        *this << "env.setObjectLocation(" << aLabel << ", "
              << aLabel << "." << posMember->name << ");" << nl;
      }

      // TODO There are some ordering issues here, which we ignore for now
      // This does not fully respect the order between different agent types
      *this << "schedule.scheduleOnce(" << aLabel << ")";
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
    *this << ")NICOLA";
  }
}
/*
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
*/
void DMasonPrinter::print(const AST::Script &script) {
  inAgent = false; // Printing main simulation code

  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl
	<< "import java.util.List;" << nl << nl
	<< "import it.isislab.dmason.exception.DMasonException; "<< nl
	<< "import it.isislab.dmason.experimentals.tools.batch.data.EntryParam;" << nl
	<< "import it.isislab.dmason.experimentals.tools.batch.data.GeneralParam;" << nl
	<< "import it.isislab.dmason.sim.engine.DistributedMultiSchedule;" << nl
	<< "import it.isislab.dmason.sim.engine.DistributedState;" << nl
	<< "import it.isislab.dmason.sim.engine.RemotePositionedAgent;" << nl
	<< "import it.isislab.dmason.sim.field.DistributedField;" << nl
	<< "import it.isislab.dmason.sim.field.DistributedField2D;" << nl
	<< "import it.isislab.dmason.sim.field.continuous.DContinuousGrid2D;" << nl
	<< "import it.isislab.dmason.sim.field.continuous.DContinuousGrid2DFactory;"<< nl << nl
        << "public class Sim extends DistributedState<Double2D> {" << indent << nl;

  for (const AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  // TODO Replace dummy granularity
  if (script.envDecl && script.envDecl->sizeExpr) {
    //const AST::CallExpression &expr =
      //*dynamic_cast<const AST::CallExpression *>(&*script.envDecl->sizeExpr);
    *this << "DContinuousGrid2D env;"  << nl
    	  << "String topicPrefix;"  << nl
     	  << "int MODE;"  << nl
    	  << "public double gridWidth;" << nl
    	  << "public double gridHeight;" << nl << nl;
       //printVecCtorArgs(*this, expr);
  }

  *this << "public Sim(GeneralParam params,String prefix) {" << indent << nl
        << "super(params, new DistributedMultiSchedule<Double2D>(),prefix,params.getConnectionType());" << nl
	<< "this.MODE=params.getMode();" << nl
	<< "this.topicPrefix=prefix;" << nl
	<< "gridWidth=params.getWidth();" << nl
	<< "gridHeight=params.getHeight();" << nl
 	<< outdent << nl << "}" << nl << nl;

  *this << "public Sim(GeneralParam params,List<EntryParam<String, Object>> simParams,String prefix) {" << indent << nl
        << "super(params, new DistributedMultiSchedule<Double2D>(),prefix,params.getConnectionType());" << nl
        << "this.MODE=params.getMode();" << nl
        << "this.topicPrefix=prefix;" << nl
        << "gridWidth=params.getWidth();" << nl
        << "gridHeight=params.getHeight();" << nl  
	<< "for (EntryParam<String, Object> entryParam : simParams) {" << indent << nl
 	<< "try { " << nl
 		<< "this.getClass().getDeclaredField(entryParam.getParamName()).set(this, entryParam.getParamValue());"  << outdent << nl
 	<< "} catch (IllegalArgumentException e) {e.printStackTrace();}" << nl
	<< "catch (SecurityException e) {e.printStackTrace();}" << nl
 	<< "catch (IllegalAccessException e) {e.printStackTrace();}" << nl
	<< "catch (NoSuchFieldException e) {e.printStackTrace();}" << nl
	<< "}" << nl
 	<< "for (EntryParam<String, Object> entryParam : simParams) {" << indent << nl
 	<< "try {" << nl
		<< "System.out.println(this.getClass().getDeclaredField(entryParam.getParamName()).get(this));"  << outdent<< nl
 	<< "} catch (IllegalArgumentException e) {e.printStackTrace();}" << nl
 	<< "catch (SecurityException e) {e.printStackTrace();}" << nl
 	<< "catch (IllegalAccessException e) {e.printStackTrace();} " << nl
 	<< "catch (NoSuchFieldException e) {e.printStackTrace();}}" << nl
       << outdent << nl << "}" << nl << nl;


  *this << "public void start() {" << indent << nl
        << "super.start();" << nl
	<<"try { "<< indent << nl
	<<"env = DContinuousGrid2DFactory.createDContinuous2D(8.0,gridWidth, gridHeight,this,"<< nl
	<<"	super.AOI,TYPE.pos_i,TYPE.pos_j,super.rows,super.columns,MODE,\"env\", topicPrefix,false);" << nl
	<<"init_connection();"<< outdent << nl
	<<"} catch (DMasonException e) { e.printStackTrace();}" << nl << nl;

  // TODO This needs to be split in start + end
  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  if (mainFunc) {
    *this << *mainFunc->stmts;
  }

  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isStep && !decl->isMain()) {
      *this << nl << nl << *decl;
    }
  }

  *this << outdent << nl << "}" << nl
	<<"public DistributedField<Double2D> getField() {" << indent << nl
        <<"return env;" << nl
        <<"}" << outdent << nl
        <<"public void addToField(RemotePositionedAgent<Double2D> rm, Double2D loc) {"<< indent << nl
        <<"env.setObjectLocation(rm, loc);" << nl
        <<"}"<< outdent << nl
	<<"public SimState getState() {"<< indent << nl
        <<"return this;"<< nl
        <<"}"<< outdent << nl
	<< "public static void main(String[] args) {" << indent
        << nl << "doLoop(Sim.class, args);"
        << nl << "System.exit(0);"
        << outdent << nl << "}";

  *this << outdent << nl << "}";

}

}

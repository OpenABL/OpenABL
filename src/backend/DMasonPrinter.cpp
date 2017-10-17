#include "DMasonPrinter.hpp"

namespace OpenABL {

void DMasonPrinter::printLocalTestCode() {
	*this << "import it.isislab.dmason.experimentals.tools.batch.data.EntryParam;" << nl
	<< "import it.isislab.dmason.experimentals.tools.batch.data.GeneralParam;" << nl
	<< "import it.isislab.dmason.sim.engine.DistributedState;" << nl
	<< "import it.isislab.dmason.sim.field.DistributedField2D;" << nl
	<< "import it.isislab.dmason.util.connection.ConnectionType;" << nl
	<< "import java.util.ArrayList;"   << nl
	<< "public class LocalTestSim {" << indent << nl
	<< "private static int numSteps = 3000; //only graphicsOn=false" << nl
	<< "private static int rows = 2; //number of rows" << nl
	<< "private static int columns = 2; //number of columns" << nl
	<< "private static int AOI=10; //max distance" << nl
	<< "private static int NUM_AGENTS=20000; //number of agents" << nl
	<< "private static int WIDTH=200; //field width" << nl
	<< "private static int HEIGHT=200; //field height" << nl
	<< "private static String ip=\"127.0.0.1\"; //ip of activemq" << nl
	<< "private static String port=\"61616\"; //port of activemq" << nl
	<< "private static String topicPrefix=\"SIM-NAME\"; //unique string to identify topics for this simulation " << nl
	<< "private static int MODE = DistributedField2D.UNIFORM_PARTITIONING_MODE;" << nl
	<< "public static void main(String[] args) {" << indent << nl
	<< "	class worker extends Thread {" << indent << nl
	<< "		private DistributedState ds;" << nl
	<< "		public worker(DistributedState ds) {" << nl
	<< "			this.ds=ds;" << nl
	<< "			ds.start();" << nl
	<< "		}" << nl
	<< "		public void run() {" << nl
	<< "			int i=0;" << nl
	<< "			while(i!=numSteps)" << nl
	<< "			{" << nl
	<< "				System.out.println(i);" << nl
	<< "				ds.schedule.step(ds);" << nl
	<< "				i++;" << nl
	<< "			}" << nl
	<< "			System.exit(0);" << nl
	<< "		}" << outdent << nl
	<< "	}" << nl
	<< "	ArrayList<worker> myWorker = new ArrayList<worker>();" << nl
	<< "	for (int i = 0; i < rows; i++) {" << nl
	<< "		for (int j = 0; j < columns; j++) {" << nl
	<< "			GeneralParam genParam = new GeneralParam(WIDTH, HEIGHT, AOI, rows,columns,NUM_AGENTS, MODE,ConnectionType.pureActiveMQ); " << nl
	<< "			genParam.setI(i);" << nl
	<< "			genParam.setJ(j);" << nl
	<< "			genParam.setIp(ip);" << nl
	<< "			genParam.setPort(port);" << nl
	<< "			ArrayList<EntryParam<String, Object>> simParams=new ArrayList<EntryParam<String, Object>>();" << nl
	<< "			Sim sim = new Sim(genParam,simParams,topicPrefix); " << nl
	<< "			worker a = new worker(sim);" << nl
	<< "			myWorker.add(a);" << nl
	<< "		}" << nl
	<< "	}" << nl
	<< "		for (worker w : myWorker) {" << nl
	<< "			w.start();" << nl
	<< "		}" << nl
	<< "	}" << nl
	<< "}" << outdent << nl;
}
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
void DMasonPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.isStep) {
    const AST::Param &param = decl.stepParam();
    AST::AgentDeclaration &agent = decl.stepAgent();
    AST::AgentMember *posMember = agent.getPositionMember();

    currentInVar = param.var->id;
    currentOutVar = param.outVar->id;

    // Use a leading "_" to avoid clashes with existing methods like "step()"
    *this << "public void _" << decl.name << "(SimState state) {" << indent << nl
          << "Sim _sim = (Sim) state;" << nl
          << "State " << *param.var << " = getInState();" << nl
          << "State " << *param.outVar << " = getOutState();" << nl
          << *param.var << "." << posMember->name << " = _sim.env.getObjectLocation(this);" << nl
          << "prepareOutState();"
          << *decl.stmts;
    if (posMember) {
      if (agent.usesRuntimeRemoval) {
        assert(0); // TODO
        *this << nl << "if (_isDead) {" << indent << nl
              << "_sim.env.remove(this);"
              << outdent << nl << "} else {" << indent
              << nl << "_sim.env.setObjectLocation(this, getOutState()." << posMember->name << ");"
              << nl << "_sim.schedule.scheduleOnceIn(1.0, this);"
              << nl << "swapStates();"
              << outdent << nl << "}";
      } else {
        *this << nl << "try {" << indent
              << nl << "this.setPos(" << *param.outVar << "." << posMember->name << ");"
              << nl << "_sim.env.setDistributedObjectLocation("
              << *param.outVar << "." << posMember->name << ", this, _sim);"
              << nl << "swapStates();"
              << outdent << nl << "} catch (DMasonException e) { e.printStackTrace(); }";
      }
    }
    *this << outdent << nl << "}";
    
    currentInVar.reset();
    currentOutVar.reset();
  } else {
    Mason2Printer::print(decl);
  }
}

void DMasonPrinter::printAgentImports() {
  *this << "import sim.engine.*;" << nl
        << "import sim.util.*;" << nl
	      << "import it.isislab.dmason.exception.DMasonException;" << nl
	      << "import it.isislab.dmason.sim.engine.DistributedState;" << nl;
}
void DMasonPrinter::printAgentExtends(const AST::AgentDeclaration &decl) {
  *this << " extends Remote" << decl.name;
}
void DMasonPrinter::printAgentExtraCode(const AST::AgentDeclaration &decl) {
  // DMason needs default constructor
  *this << "public " << decl.name << "() { }" << nl;
}
void DMasonPrinter::printAgentExtraCtorArgs() {
  *this << "DistributedState<Double2D> sm, ";
}
void DMasonPrinter::printAgentExtraCtorCode() {
  *this << "super(sm);" << nl;
}
void DMasonPrinter::printStepDefaultCode(const AST::FunctionDeclaration &decl) {
  const AST::AgentDeclaration &stepAgent = decl.stepAgent();
  const AST::AgentMember *posMember = stepAgent.getPositionMember();

  *this << "try {" << indent
        << nl << "this.setPos(getInState()." << posMember->name << ");"
        << nl << "((Sim) state).env.setDistributedObjectLocation("
        << "getInState()." << posMember->name << ", this, state);"
        << outdent << nl << "} catch (DMasonException e) { e.printStackTrace(); }" << nl;
}

void DMasonPrinter::print(const AST::CallExpression &expr) {
  const std::string &name = expr.name;
  if (expr.isCtor()) {
    Mason2Printer::print(expr);
  } else if (expr.isBuiltin()) {
    if (name == "add") {
      std::string aLabel = makeAnonLabel();
      std::string pLabel = makeAnonLabel();
      const AST::Expression &arg = expr.getArg(0);
      Type type = arg.type;
      AST::AgentDeclaration *agent = type.getAgentDecl();
      AST::AgentMember *posMember = agent->getPositionMember();
      *this << type << " " << aLabel << " = " << arg << ";" << nl
            << "Double2D " << pLabel << " = " << aLabel
            << ".getInState()." << posMember->name << ";" << nl
            << "if (" << pLabel << ".x >= env.own_x && "
            << pLabel << ".x < env.own_x + env.my_width && "
            << pLabel << ".y >= env.own_y && "
            << pLabel << ".y < env.own_y + env.my_height) {" << indent << nl
            << aLabel << ".setPos(" << pLabel << ");" << nl
            << "env.setObjectLocation(" << aLabel << ", " << pLabel << ");" << nl
            << "schedule.scheduleOnce(" << aLabel << ");"
            << outdent << nl << "}" << nl;
    } else if (name == "save") {
      // TODO Handle save
      *this << "//save()";
    } else {
      Mason2Printer::print(expr);
    }
  } else {
    Mason2Printer::print(expr);
  }
}
void DMasonPrinter::print(const AST::AgentCreationExpression &expr) {
  AST::AgentDeclaration *agent = expr.type.getAgentDecl();

  *this << "new " << expr.name << "(this,";
  printCommaSeparated(*agent->members, [&](const AST::AgentMemberPtr &member) {
    auto it = expr.memberMap.find(member->name);
    assert(it != expr.memberMap.end());
    *this << *it->second;
  });
  *this <<")";
}
void DMasonPrinter::print(const AST::SimulateStatement &) {
  // Nothing
}

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

  *this << "DContinuousGrid2D env;"  << nl
      << "String topicPrefix;"  << nl
      << "int MODE;"  << nl
      << "public double gridWidth;" << nl
      << "public double gridHeight;" << nl << nl;

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
	<<"	super.AOI,TYPE.pos_i,TYPE.pos_j,super.rows,super.columns,MODE,\"env\", topicPrefix,true);" << nl
	<<"init_connection();"<< outdent << nl
	<<"} catch (DMasonException e) { e.printStackTrace();}" << nl << nl;

  // TODO This needs to be split in start + end
  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  if (mainFunc) {
    *this << *mainFunc->stmts;
  }

  *this << outdent << nl << "}" << nl;
  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isStep && !decl->isMain()) {
      *this << nl << nl << *decl;
    }
  }

  *this <<"public DistributedField<Double2D> getField() {" << indent << nl
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

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

#include <cmath>
#include "DMasonPrinter.hpp"

namespace OpenABL {

void DMasonPrinter::printLocalTestCode(const Config &config) {
  Value &envSize = script.envDecl->envSize;
  Value::Vec3 envSizeVec = envSize.extendToVec3().getVec3();
  int dim = envSize.isVec2() ? 2 : 3;
  int width = (int) ceil(envSizeVec.x);
  int height = (int) ceil(envSizeVec.y);
  int length = (int) ceil(envSizeVec.z);
  int maxDist = width > 10 ? 10 : width - 1; // TODO

  bool visualize = config.getBool("visualize", false);
  long gridRows = config.getInt("dmason.grid_rows", 2);
  long gridCols = config.getInt("dmason.grid_cols", 2);
  long gridLengths = config.getInt("dmason.grid_lengths", 1);

  const char *fieldPkg = dim == 3 ? "field3D" : "field";

  *this
    << "import it.isislab.dmason.experimentals.systemmanagement.utils.activemq.ActiveMQStarter;" << nl
    << "import it.isislab.dmason.experimentals.tools.batch.data.EntryParam;" << nl
    << "import it.isislab.dmason.experimentals.tools.batch.data.GeneralParam;" << nl
    << "import it.isislab.dmason.sim.engine.DistributedState;" << nl
    << "import it.isislab.dmason.sim." << fieldPkg << ".DistributedField" << dim << "D;" << nl
    << "import it.isislab.dmason.util.connection.ConnectionType;" << nl
    << "import java.util.ArrayList;" << nl
    << "import sim.display.Console;" << nl
    << nl
    << "public class LocalTestSim {" << indent << nl
    << "private static boolean graphicsOn = " << (visualize ? "true" : "false") << ";" << nl
    << "private static int rows = " << gridRows << "; // number of rows" << nl
    << "private static int columns = " << gridCols << "; // number of columns" << nl
    << "private static int lengths = " << gridLengths << "; // number of lengths" << nl
    << "private static int AOI = " << maxDist << "; // max distance" << nl
    << "private static int NUM_AGENTS=20000; //number of agents" << nl
    << "private static int WIDTH = " << width << "; // field width" << nl
    << "private static int HEIGHT = " << height << "; // field height" << nl
    << "private static int LENGTH = " << length << "; // field length" << nl
    << "private static String ip=\"127.0.0.1\"; //ip of activemq" << nl
    << "private static String port=\"61616\"; //port of activemq" << nl
    << "private static String topicPrefix=\"SIM-NAME\"; //unique string to identify topics for this simulation " << nl
    << "private static int MODE = DistributedField" << dim << "D.UNIFORM_PARTITIONING_MODE;" << nl
    << "public static void main(String[] args) {" << nl
    << "    ActiveMQStarter starter = new ActiveMQStarter();" << nl
    << "    starter.startActivemq();" << nl
    << "    System.setProperty(\"org.apache.activemq.SERIALIZABLE_PACKAGES\", \"*\");" << nl
    << "    class worker extends Thread {" << nl
    << "        private Sim sim;" << nl
    << "        public worker(Sim sim) {" << nl
    << "            this.sim = sim;" << nl
    << "            sim.start();" << nl
    << "        }" << nl
    << "        public void run() {" << nl
    << "            if (graphicsOn) {" << nl
    << "                while (true) sim.schedule.step(sim);" << nl
    << "            } else {" << nl
    << "                sim.run();" << nl
    << "            }" << nl
    << "            System.exit(0);" << nl
    << "        }" << nl
    << "    }" << nl
    << "    ArrayList<worker> myWorker = new ArrayList<worker>();" << nl
    << "    for (int i = 0; i < rows; i++) {" << nl
    << "        for (int j = 0; j < columns; j++) {" << nl
    << "            for (int z = 0; z < lengths; z++) {" << nl
    << "                GeneralParam genParam = new GeneralParam(" << nl
    << "                    WIDTH, HEIGHT, LENGTH, AOI," << nl
    << "                    rows, columns, lengths, NUM_AGENTS, MODE, ConnectionType.pureActiveMQ, /* is3D */ " << (dim == 3 ? "true" : "false") << nl
    << "                ); " << nl
    << "                genParam.setI(i);" << nl
    << "                genParam.setJ(j);" << nl
    << "                genParam.setZ(z);" << nl
    << "                genParam.setIp(ip);" << nl
    << "                genParam.setPort(port);" << nl
    << "                ArrayList<EntryParam<String, Object>> simParams=new ArrayList<EntryParam<String, Object>>();" << nl
    << "                if (graphicsOn && i == 0 && j == 0 && z == 0) {" << nl
    << "                    SimWithUI sim = new SimWithUI(genParam,simParams,topicPrefix);" << nl
    << "                    ((Console) sim.createController()).pressPause();" << nl
    << "                } else {" << nl
    << "                    Sim sim = new Sim(genParam,simParams,topicPrefix); " << nl
    << "                    worker a = new worker(sim);" << nl
    << "                    myWorker.add(a);" << nl
    << "                }" << nl
    << "            }" << nl
    << "        }" << nl
    << "    }" << nl
    << "        for (worker w : myWorker) {" << nl
    << "            w.start();" << nl
    << "        }" << nl
    << "    }" << nl
    << "}" << nl;
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

void DMasonPrinter::printUIExtraImports() {
  *this <<
  "import java.util.List;\n"
  "import it.isislab.dmason.experimentals.tools.batch.data.EntryParam;\n"
  "import it.isislab.dmason.experimentals.tools.batch.data.GeneralParam;\n";
}
void DMasonPrinter::printUICtors() {
  *this <<
  "    public SimWithUI(GeneralParam args, String prefix) {\n"
  "        super(new Sim(args, prefix));\n"
  "        name = String.valueOf(args.getI()) + \"\" + String.valueOf(args.getJ());\n"
  "    }\n"
  "    public SimWithUI(GeneralParam args, List<EntryParam<String, Object>> simParams, String prefix) {\n"
  "        super(new Sim(args, simParams, prefix));\n"
  "        name = String.valueOf(args.getI()) + \"\" + String.valueOf(args.getJ());\n"
  "    }\n";
}

void DMasonPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.isParallelStep()) {
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
        *this << nl << "if (_isDead) {" << indent << nl
              << "_sim.env.remove(this);" << nl
              << "return;"
              << outdent << nl << "}";
      }
      *this << nl << "try {" << indent
            << nl << "this.setPos(" << *param.outVar << "." << posMember->name << ");"
            << nl << "_sim.env.setDistributedObjectLocation("
            << *param.outVar << "." << posMember->name << ", this, _sim);"
            << nl << "swapStates();"
            << outdent << nl << "} catch (DMasonException e) { e.printStackTrace(); }";
    }
    *this << outdent << nl << "}";

    currentInVar.reset();
    currentOutVar.reset();
  } else {
    MasonPrinter::print(decl);
  }
}

void DMasonPrinter::printAgentImports() {
  *this << "import java.io.Serializable;" << nl
        << "import sim.engine.*;" << nl
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
  unsigned dim = script.envDecl->getEnvDimension();
  *this << "DistributedState<Double" << dim << "D> sm, ";
}
void DMasonPrinter::printAgentExtraCtorCode() {
  *this << "super(sm);" << nl;
}
void DMasonPrinter::printStepDefaultCode(const AST::AgentDeclaration &decl) {
  const AST::AgentMember *posMember = decl.getPositionMember();

  *this << "try {" << indent
        << nl << "this.setPos(getInState()." << posMember->name << ");"
        << nl << "((Sim) state).env.setDistributedObjectLocation("
        << "getInState()." << posMember->name << ", this, state);"
        << outdent << nl << "} catch (DMasonException e) { e.printStackTrace(); }" << nl;
}

void DMasonPrinter::print(const AST::CallExpression &expr) {
  const std::string &name = expr.name;
  if (expr.isCtor()) {
    MasonPrinter::print(expr);
  } else if (expr.isBuiltin()) {
    if (name == "add") {
      std::string aLabel = makeAnonLabel();
      std::string pLabel = makeAnonLabel();
      const AST::Expression &arg = expr.getArg(0);
      Type type = arg.type;
      AST::AgentDeclaration *agent = type.getAgentDecl();
      AST::AgentMember *posMember = agent->getPositionMember();
      unsigned dim = script.envDecl->getEnvDimension();
      bool isRuntimeAdd = !inMain;

      *this << type << " " << aLabel << " = " << arg << ";" << nl
            << *posMember->type << " " << pLabel << " = " << aLabel
            << ".getInState()." << posMember->name << ";" << nl;
      if (!isRuntimeAdd) {
        *this << "if (" << pLabel << ".x >= env.own_x && "
              << pLabel << ".x < env.own_x + env.my_width && "
              << pLabel << ".y >= env.own_y && "
              << pLabel << ".y < env.own_y + env.my_height";
        if (dim == 3) {
          *this << " && " << pLabel << ".z >= env.own_z && "
                << pLabel << ".z < env.own_z + env.my_length";
        }
        *this << ") {" << indent << nl;
      }
      *this << aLabel << ".setPos(" << pLabel << ");" << nl
            << getSimVarName() << ".env.setObjectLocation(" << aLabel << ", " << pLabel << ");" << nl
            << getSimVarName() << ".schedule.scheduleOnce(" << aLabel << ");";
      if (!isRuntimeAdd) {
        *this << outdent << nl << "}" << nl;
      }
    } else if (name == "count" || name == "sum") {
      // TODO
      *this << "0";
    } else if (name == "getLastExecTime") {
      // TODO
      *this << "0.0";
    } else if (name == "log_csv") {
      // TODO
      *this << "//log_csv()";
    } else if (name == "save") {
      // TODO Handle save
      *this << "//save()";
    } else {
      MasonPrinter::print(expr);
    }
  } else {
    MasonPrinter::print(expr);
  }
}
void DMasonPrinter::print(const AST::AgentCreationExpression &expr) {
  AST::AgentDeclaration *agent = expr.type.getAgentDecl();

  *this << "new " << expr.name << "(" << getSimVarName() << ", ";
  printCommaSeparated(*agent->members, [&](const AST::AgentMemberPtr &member) {
    auto it = expr.memberMap.find(member->name);
    assert(it != expr.memberMap.end());
    *this << *it->second;
  });
  *this <<")";
}

void DMasonPrinter::print(const AST::SimulateStatement &stmt) {
  *this << "int t = " << *stmt.timestepsExpr
        << " * " << stmt.stepFuncDecls.size() << ";" << nl
        << "for (int i = 0; i != t; i++) {" << indent << nl
        << "System.out.println(i);" << nl
        << "if (!this.schedule.step(this)) break;" << outdent << nl
        << "}";
}

void DMasonPrinter::print(const AST::Script &script) {
  inAgent = false; // Printing main simulation code
  unsigned dim = script.envDecl->getEnvDimension();
  const char *fieldPkg = dim == 3 ? "field3D" : "field";
  const char *continuousPkg = dim == 3 ? "continuous3D" : "continuous";

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
	<< "import it.isislab.dmason.sim." << fieldPkg << ".DistributedField" << dim << "D;" << nl
	<< "import it.isislab.dmason.sim." << fieldPkg << "." << continuousPkg
      << ".DContinuousGrid" << dim << "D;" << nl
	<< "import it.isislab.dmason.sim." << fieldPkg << "." << continuousPkg
      << ".DContinuousGrid" << dim << "DFactory;"<< nl << nl
  << "public class Sim extends DistributedState<Double" << dim << "D> {" << indent << nl;

  for (const AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }

  *this << "DContinuousGrid" << dim << "D env;"  << nl
      << "String topicPrefix;"  << nl
      << "int MODE;"  << nl
      << "public double gridWidth;" << nl
      << "public double gridHeight;" << nl
      << "public double gridLength;" << nl
      << nl;

  *this << "public Sim(GeneralParam params,String prefix) {" << indent << nl
        << "super(params, new DistributedMultiSchedule<Double" << dim << "D>(), prefix, params.getConnectionType());" << nl
	<< "this.MODE = params.getMode();" << nl
	<< "this.topicPrefix = prefix;" << nl
	<< "gridWidth = params.getWidth();" << nl
	<< "gridHeight = params.getHeight();" << nl
	<< "gridLength = params.getLenght();" << nl
 	<< outdent << nl << "}" << nl << nl;

  *this << "public Sim(GeneralParam params,List<EntryParam<String, Object>> simParams,String prefix) {" << indent << nl
        << "super(params, new DistributedMultiSchedule<Double" << dim << "D>(),prefix,params.getConnectionType());" << nl
        << "this.MODE = params.getMode();" << nl
        << "this.topicPrefix = prefix;" << nl
        << "gridWidth = params.getWidth();" << nl
        << "gridHeight = params.getHeight();" << nl
        << "gridLength = params.getLenght();" << nl
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
	<< "try { "<< indent << nl
	<< "env = DContinuousGrid" << dim << "DFactory.createDContinuous" << dim << "D(" << nl;
  if (dim == 3) {
    *this << "    8.0, gridWidth, gridHeight, gridLength, this," << nl
          << "    super.AOI, TYPE.pos_i, TYPE.pos_j, TYPE.pos_z," << nl
          << "    super.rows, super.columns, super.lenghts, MODE, \"env\", topicPrefix, true" << nl;
  } else {
    *this << "    8.0, gridWidth, gridHeight, this," << nl
          << "    super.AOI, TYPE.pos_i, TYPE.pos_j," << nl
          << "    super.rows, super.columns, MODE, \"env\", topicPrefix, true" << nl;
  }
  *this << ");" << nl
	<< "init_connection();"<< outdent << nl
	<< "} catch (DMasonException e) { e.printStackTrace();}" << nl << nl;

  AST::FunctionDeclaration *mainFunc = script.mainFunc;
  inMain = true;
  *this << mainFunc->getStmtsBeforeSimulate();
  inMain = false;

  *this << outdent << nl << "}" << nl << nl
        << "public void run() {" << indent << nl
        << *script.simStmt << outdent << nl
        << "}" << nl;

  // Print non-step, non-main functions
  for (const AST::FunctionDeclaration *decl : script.funcs) {
    if (!decl->isParallelStep() && !decl->isMain()) {
      *this << nl << nl << *decl;
    }
  }

  *this << nl
        <<"public DistributedField<Double" << dim << "D> getField() {" << indent << nl
        <<"return env;" << nl
        <<"}" << outdent << nl
        <<"public void addToField(RemotePositionedAgent<Double" << dim << "D> rm, Double" << dim << "D loc) {"<< indent << nl
        <<"env.setObjectLocation(rm, loc);" << nl
        <<"}"<< outdent << nl
	      <<"public SimState getState() {"<< indent << nl
        <<"return this;"<< nl
        <<"}"<< outdent << nl
	      << "public static void main(String[] args) {" << indent
        << nl << "doLoop(Sim.class, args);"
        << nl << "System.exit(0);"
        << outdent << nl << "}";
  *this << nl << "public static int getColor(Object obj) {" << indent
        << nl << "return 0;"
        << outdent << nl << "}"
        << nl << "public static int getSize(Object obj) {" << indent
        << nl << "return 1;"
        << outdent << nl << "}";

  *this << outdent << nl << "}";

}

}

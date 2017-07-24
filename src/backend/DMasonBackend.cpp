#include "Backend.hpp"
#include "DMasonPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

static std::string generateMainCode(AST::Script &script) {
  DMasonPrinter printer(script);
  printer.print(script);
  return printer.extractStr();
}

static std::string generateAgentCode(AST::Script &script, AST::AgentDeclaration &agent) {
  DMasonPrinter printer(script);
  printer.print(agent);
  return printer.extractStr();
}
static std::string generateStubAgentCode(AST::Script &script, AST::AgentDeclaration &agent) {
  DMasonPrinter printer(script);
  printer.printStubAgent(agent);
  return printer.extractStr();
}
static std::string generateLocalTestCode(AST::Script &script) {
  DMasonPrinter printer(script);
  printer.printLocalTestCode();
  return printer.extractStr();
}

void DMasonBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  if (script.envDecl) {
    if (script.envDecl->envDimension == 3) {
      throw NotSupportedError("3D environments are not supported by DMason");
    }
    for (double d : script.envDecl->envMin.getVec()) {
      if (d < 0) {
        throw NotSupportedError("Negative environment bounds are not supported by DMason");
      }
    }
  }

  writeToFile(outputDir + "/Sim.java", generateMainCode(script));
  writeToFile(outputDir + "/LocalTestSim.java", generateLocalTestCode(script));
  
  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(outputDir + "/Remote" + agent->name + ".java", generateStubAgentCode(script, *agent));
    writeToFile(outputDir + "/" + agent->name + ".java", generateAgentCode(script, *agent));
  }

  copyFile(assetDir + "/mason/Util.java", outputDir + "/Util.java");
  copyFile(assetDir + "/mason/build.sh", outputDir + "/build.sh");
  makeFileExecutable(outputDir + "/build.sh");
}

}

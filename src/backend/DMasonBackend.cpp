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

void DMasonBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  writeToFile(outputDir + "/Sim.java", generateMainCode(script));

  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(outputDir + "/Remote" + agent->name + ".java", generateStubAgentCode(script, *agent));
    writeToFile(outputDir + "/" + agent->name + ".java", generateAgentCode(script, *agent));
  }

  copyFile(assetDir + "/mason/Util.java", outputDir + "/Util.java");
  copyFile(assetDir + "/mason/build.sh", outputDir + "/build.sh");
  makeFileExecutable(outputDir + "/build.sh");
}

}

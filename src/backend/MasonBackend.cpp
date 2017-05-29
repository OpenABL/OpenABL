#include "Backend.hpp"
#include "MasonPrinter.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

static std::string generateMainCode(AST::Script &script) {
  MasonPrinter printer(script);
  printer.print(script);
  return printer.extractStr();
}

static std::string generateAgentCode(AST::Script &script, AST::AgentDeclaration &agent) {
  MasonPrinter printer(script);
  printer.print(agent);
  return printer.extractStr();
}

void MasonBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  copyFile(assetDir + "/mason/Util.java", outputDir + "/Util.java");
  writeToFile(outputDir + "/Sim.java", generateMainCode(script));

  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(outputDir + "/" + agent->name + ".java", generateAgentCode(script, *agent));
  }
}

}

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
    AST::Script &script, const BackendContext &ctx) {
  bool useFloat = ctx.config.getBool("use_float", false);
  if (useFloat) {
    throw BackendError("Floats are not supported by the Mason backend");
  }

  writeToFile(ctx.outputDir + "/Sim.java", generateMainCode(script));

  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(ctx.outputDir + "/" + agent->name + ".java", generateAgentCode(script, *agent));
  }

  copyFile(ctx.assetDir + "/mason/Util.java", ctx.outputDir + "/Util.java");
  copyFile(ctx.assetDir + "/mason/build.sh", ctx.outputDir + "/build.sh");
  copyFile(ctx.assetDir + "/mason/run.sh", ctx.outputDir + "/run.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");
}

}

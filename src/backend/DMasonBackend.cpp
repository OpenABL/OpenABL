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
    AST::Script &script, const BackendContext &ctx) {
  if (script.envDecl) {
    if (script.envDecl->envDimension == 3) {
      throw BackendError("3D environments are not supported by DMason");
    }
    for (double d : script.envDecl->envMin.getVec()) {
      if (d < 0) {
        throw BackendError("Negative environment bounds are not supported by DMason");
      }
    }
  }
  if (script.usesRuntimeRemoval || script.usesRuntimeAddition) {
    throw BackendError("DMason does not support dynamic add/remove yet");
  }

  bool useFloat = ctx.config.getBool("use_float", false);
  if (useFloat) {
    throw BackendError("Floats are not supported by the Mason backend");
  }

  writeToFile(ctx.outputDir + "/Sim.java", generateMainCode(script));
  writeToFile(ctx.outputDir + "/LocalTestSim.java", generateLocalTestCode(script));
  
  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(
        ctx.outputDir + "/Remote" + agent->name + ".java",
        generateStubAgentCode(script, *agent));
    writeToFile(
        ctx.outputDir + "/" + agent->name + ".java",
        generateAgentCode(script, *agent));
  }

  copyFile(ctx.assetDir + "/mason/Util.java", ctx.outputDir + "/Util.java");
  copyFile(ctx.assetDir + "/mason/build.sh", ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
}

}

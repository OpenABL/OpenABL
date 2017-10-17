#include "Backend.hpp"
#include "Mason2Printer.hpp"
#include "FileUtil.hpp"

namespace OpenABL {

static std::string generateMainCode(AST::Script &script) {
  Mason2Printer printer(script);
  printer.print(script);
  return printer.extractStr();
}

static std::string generateAgentCode(AST::Script &script, AST::AgentDeclaration &agent) {
  Mason2Printer printer(script);
  printer.print(agent);
  return printer.extractStr();
}

static std::string generateUICode(AST::Script &script) {
  Mason2Printer printer(script);
  printer.printUI();
  return printer.extractStr();
}

static std::string generateRunScript(bool visualize) {
  if (visualize) {
    return "java SimWithUI";
  } else {
    return "java Sim";
  }
}

void Mason2Backend::generate(
    AST::Script &script, const BackendContext &ctx) {
  bool useFloat = ctx.config.getBool("use_float", false);
  if (useFloat) {
    throw BackendError("Floats are not supported by the Mason backend");
  }

  bool visualize = ctx.config.getBool("visualize", false);

  writeToFile(ctx.outputDir + "/Sim.java", generateMainCode(script));
  writeToFile(ctx.outputDir + "/SimWithUI.java", generateUICode(script));

  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(ctx.outputDir + "/" + agent->name + ".java", generateAgentCode(script, *agent));
  }

  copyFile(ctx.assetDir + "/mason/Util.java", ctx.outputDir + "/Util.java");
  copyFile(ctx.assetDir + "/mason/build.sh", ctx.outputDir + "/build.sh");
  writeToFile(ctx.outputDir + "/run.sh", generateRunScript(visualize));
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");
}

}

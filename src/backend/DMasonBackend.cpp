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
static std::string generateLocalTestCode(AST::Script &script, bool visualize) {
  DMasonPrinter printer(script);
  printer.printLocalTestCode(visualize);
  return printer.extractStr();
}
static std::string generateUICode(AST::Script &script) {
  DMasonPrinter printer(script);
  printer.printUI();
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

  bool useFloat = ctx.config.getBool("use_float", false);
  if (useFloat) {
    throw BackendError("Floats are not supported by the Mason backend");
  }

  bool visualize = ctx.config.getBool("visualize", false);

  writeToFile(ctx.outputDir + "/Sim.java", generateMainCode(script));
  writeToFile(ctx.outputDir + "/SimWithUI.java", generateUICode(script));
  writeToFile(ctx.outputDir + "/LocalTestSim.java", generateLocalTestCode(script, visualize));

  for (AST::AgentDeclaration *agent : script.agents) {
    writeToFile(
        ctx.outputDir + "/Remote" + agent->name + ".java",
        generateStubAgentCode(script, *agent));
    writeToFile(
        ctx.outputDir + "/" + agent->name + ".java",
        generateAgentCode(script, *agent));
  }

  copyFile(ctx.assetDir + "/mason/Util.java", ctx.outputDir + "/Util.java");
  copyFile(ctx.assetDir + "/dmason/build.sh", ctx.outputDir + "/build.sh");
  copyFile(ctx.assetDir + "/dmason/run.sh", ctx.outputDir + "/run.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");
}

void DMasonBackend::initEnv(const BackendContext &ctx) {
  std::string dmasonDir = ctx.depsDir + "/dmason";
  if (directoryExists(dmasonDir)) {
    std::string jarPath = dmasonDir + "/DMASON-3.2.jar:";
    setenv("DMASON_JAR", jarPath.c_str(), true);

    std::string resourcesPath = dmasonDir + "/resources";
    setenv("DMASON_RESOURCES", resourcesPath.c_str(), true);
  }
}

}

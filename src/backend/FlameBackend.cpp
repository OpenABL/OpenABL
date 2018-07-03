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
#include "XmlUtil.hpp"
#include "FileUtil.hpp"
#include "FlameModel.hpp"
#include "FlamePrinter.hpp"
#include "FlameMainPrinter.hpp"

namespace OpenABL {

static XmlElems createXmlAgents(AST::Script &script, const FlameModel &model, bool useFloat) {
  XmlElems agents;
  for (const AST::AgentDeclaration *decl : script.agents) {
    XmlElems members, functions;

    auto unpackedMembers = FlameModel::getUnpackedMembers(*decl->members, useFloat, false);
    for (const FlameModel::Member &member : unpackedMembers) {
      members.push_back({ "variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    for (const FlameModel::Func &func : model.funcs) {
      if (func.agent != decl) {
        continue;
      }

      XmlElems inputs, outputs;

      if (!func.inMsgName.empty()) {
        inputs.push_back({ "input", {
          { "messageName", {{ func.inMsgName }} },
        }});
      }

      if (!func.outMsgName.empty()) {
        outputs.push_back({ "output", {
          { "messageName", {{ func.outMsgName }} },
        }});
      }

      functions.push_back({ "function", {
        { "name", {{ func.name }} },
        { "currentState", {{ func.currentState }} },
        { "nextState", {{ func.nextState }} },
        { "inputs", inputs },
        { "outputs", outputs },
      }});
    }

    agents.push_back({ "xagent", {
      { "name", {{ decl->name }} },
      { "memory", members },
      { "functions", functions },
    }});
  }
  return agents;
}

static XmlElems createXmlMessages(const FlameModel &model, bool useFloat) {
  XmlElems messages;
  for (const FlameModel::Message &msg : model.messages) {
    XmlElems variables;
    auto unpackedMembers = FlameModel::getUnpackedMembers(msg.members, useFloat, false);
    for (const FlameModel::Member &member : unpackedMembers) {
      variables.push_back({ "variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    messages.push_back({ "message", {
      { "name", {{ msg.name }} },
      { "variables", variables },
    }});
  }
  return messages;
}

static std::string createXmlModel(AST::Script &script, const FlameModel &model, bool useFloat) {
  XmlElems agents = createXmlAgents(script, model, useFloat);
  XmlElems messages = createXmlMessages(model, useFloat);
  XmlElem root("xmodel", {
    { "name", {{ "TODO" }} },
    { "version", {{ "01" }} },
    { "environment", {
      { "functionFiles", {
        { "file", {{ "functions.c" }} },
        { "file", {{ "libabl.c" }} },
      }}
    }},
    { "agents", agents },
    { "messages", messages },
  });
  root.setAttr("version", "2");
  root.setAttr("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
  root.setAttr("xsi:noNamespaceSchemaLocation", "http://flame.ac.uk/schema/xmml_v2.xsd");

  XmlWriter writer;
  return writer.serialize(root);
}

static std::string createFunctionsFile(
    AST::Script &script, const FlameModel &model, bool useFloat) {
  FlamePrinter printer(script, model, useFloat);
  printer.print(script);
  return printer.extractStr();
}

static std::string createMainFile(AST::Script &script, bool useFloat, bool parallel) {
  FlameMainPrinter printer(script,
    FlameMainPrinter::Params::createForFlame(useFloat, parallel));
  printer.print(script);
  return printer.extractStr();
}

static std::string createBuildRunner(bool useFloat) {
  if (useFloat) {
    return "gcc -O2 -std=c99 -DLIBABL_USE_FLOAT=1 runner.c libabl.c -lm -o runner";
  } else {
    return "gcc -O2 -std=c99 runner.c libabl.c -lm -o runner";
  }
}

void FlameBackend::generate(AST::Script &script, const BackendContext &ctx) {
  if (script.usesRuntimeRemoval || script.usesRuntimeAddition) {
    throw BackendError("Flame does not support dynamic add/remove");
  }

  bool useFloat = ctx.config.getBool("use_float", false);
  bool parallel = ctx.config.getBool("flame.parallel", false);

  FlameModel model = FlameModel::generateFromScript(script);

  writeToFile(ctx.outputDir + "/XMLModelFile.xml", createXmlModel(script, model, useFloat));
  writeToFile(ctx.outputDir + "/functions.c", createFunctionsFile(script, model, useFloat));
  writeToFile(ctx.outputDir + "/runner.c", createMainFile(script, useFloat, parallel));

  copyFile(ctx.assetDir + "/c/libabl.h", ctx.outputDir + "/libabl.h");
  copyFile(ctx.assetDir + "/c/libabl.c", ctx.outputDir + "/libabl.c");
  if (parallel) {
    copyFile(ctx.assetDir + "/flame/build-parallel.sh", ctx.outputDir + "/build.sh");
  } else {
    copyFile(ctx.assetDir + "/flame/build.sh", ctx.outputDir + "/build.sh");
  }
  copyFile(ctx.assetDir + "/flame/run.sh", ctx.outputDir + "/run.sh");
  writeToFile(ctx.outputDir + "/build_runner.sh", createBuildRunner(useFloat));
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/build_runner.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");

  createDirectory(ctx.outputDir + "/iterations");
}

void FlameBackend::initEnv(const BackendContext &ctx) {
  std::string flameDir = ctx.depsDir + "/flame";
  if (directoryExists(flameDir)) {
    std::string xparserDir = flameDir + "/xparser";
    setenv("FLAME_XPARSER_DIR", xparserDir.c_str(), true);

    std::string libmboardDir = flameDir + "/libmboard-install";
    setenv("LIBMBOARD_DIR", libmboardDir.c_str(), true);
  }
}

}

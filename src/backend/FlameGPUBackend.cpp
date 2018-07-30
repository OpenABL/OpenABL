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

#include <sstream>
#include <iomanip>
#include <cmath>
#include "Backend.hpp"
#include "FileUtil.hpp"
#include "XmlUtil.hpp"
#include "FlameModel.hpp"
#include "FlameGPUPrinter.hpp"
#include "FlameMainPrinter.hpp"

namespace OpenABL {

static std::string doubleToString(double d) {
  // Find a reasonable but accurate representation for the double
  // Seriously, C++? Seriously?
  for (int precision = 6; precision <= 17; precision++) {
    std::stringstream s;
    s << std::setprecision(precision) << d;
    std::string str = s.str();
    double d2 = std::stod(str);
    if (d == d2) {
      return str;
    }
  }
  assert(0);
}

static double roundToMultiple(double size, double radius) {
  return ceil(size / radius) * radius;
}

static XmlElems createXmlAgents(
    const AST::Script &script, const FlameModel &model, bool useFloat, long bufferSize) {
  XmlElems xagents;
  for (const AST::AgentDeclaration *decl : script.agents) {
    XmlElems members;
    auto unpackedMembers = FlameModel::getUnpackedMembers(*decl->members, useFloat, true);
    for (const FlameModel::Member &member : unpackedMembers) {
      members.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    // FlameGPU requires state names to be unique *across* agents
    std::string defaultState = decl->name + "_default";

    XmlElems functions;
    for (const FlameModel::Func &func : model.funcs) {
      if (func.agent != decl) {
        continue;
      }

      XmlElems inputs, outputs;

      if (!func.inMsgName.empty()) {
        inputs.push_back({ "gpu:input", {
          { "messageName", {{ func.inMsgName }} },
        }});
      }

      if (!func.outMsgName.empty()) {
        outputs.push_back({ "gpu:output", {
          { "messageName", {{ func.outMsgName }} },
          { "gpu:type", {{ "single_message" }} },
        }});
      }

      std::vector<XmlElem> fnElems {
        { "name", {{ func.name }} },
        { "currentState", {{ defaultState }} },
        { "nextState", {{ defaultState }} },
      };

      // FlameGPU does not allow <inputs> and <outputs> to be empty
      if (!inputs.empty()) {
        fnElems.push_back({ "inputs", inputs });
      }
      if (!outputs.empty()) {
        fnElems.push_back({ "outputs", outputs });
      }

      if (func.addedAgent) {
        fnElems.push_back({ "xagentOutputs", {
          { "gpu:xagentOutput", {
            { "xagentName", {{ func.addedAgent->name }} },
            { "state", {{ func.addedAgent->name + "_default" }} },
          }}
        }});
      }

      // FlameGPU is also very pedantic about order. These elements must
      // occur after inputs and outputs...
      bool usesRng = func.func && func.func->usesRng;
      fnElems.push_back({ "gpu:reallocate", {{ "false" }} });
      fnElems.push_back({ "gpu:RNG", {{ usesRng ? "true" : "false" }} });


      functions.push_back({ "gpu:function", fnElems });
    }

    xagents.push_back({ "gpu:xagent", {
      { "name", {{ decl->name }} },
      { "memory", members },
      { "functions", functions },
      { "states", {
        { "gpu:state", {
          { "name", {{ defaultState }} },
        }},
        { "initialState", {{ defaultState }} },
      }},
      { "gpu:type", {{ "continuous" }} },
      { "gpu:bufferSize", {{ std::to_string(bufferSize) }} },
    }});
  }
  return xagents;
}

static XmlElems createXmlMessages(
    const AST::Script &script, const FlameModel &model, bool useFloat, long bufferSize) {
  XmlElems messages;
  for (const FlameModel::Message &msg : model.messages) {
    XmlElems variables;
    auto unpackedMembers = FlameModel::getUnpackedMembers(msg.members, useFloat, true);
    for (const FlameModel::Member &member : unpackedMembers) {
      variables.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    const AST::EnvironmentDeclaration *envDecl = script.envDecl;
    assert(envDecl);

    const Value::Vec3 &min = envDecl->envMin.extendToVec3().getVec3();
    const Value::Vec3 &size = envDecl->envSize.extendToVec3().getVec3();
    double radius = envDecl->envGranularity.asFloat();

    // Adjust maximum environment bound so that the size is a multiple of the radius
    // Also make sure z dimension is at least one radius large, even if it's a 2D simulation
    // Both of these are FlameGPU requirements
    Value::Vec3 max = {
      roundToMultiple(size.x, radius) + min.x,
      roundToMultiple(size.y, radius) + min.y,
      size.z != 0 ? roundToMultiple(size.z, radius) + min.z : radius,
    };

    XmlElems partitioningInfo {
      { "gpu:radius", {{ doubleToString(radius) }} },
      { "gpu:xmin", {{ doubleToString(min.x) }} },
      { "gpu:xmax", {{ doubleToString(max.x) }} },
      { "gpu:ymin", {{ doubleToString(min.y) }} },
      { "gpu:ymax", {{ doubleToString(max.y) }} },
      { "gpu:zmin", {{ doubleToString(min.z) }} },
      { "gpu:zmax", {{ doubleToString(max.z) }} },
    };

    messages.push_back({ "gpu:message", {
      { "name", {{ msg.name }} },
      { "variables", variables },
      { "gpu:partitioningSpatial", partitioningInfo },
      { "gpu:bufferSize", {{ std::to_string(bufferSize) }} },
    }});
  }
  return messages;
}

static XmlElems createXmlLayers(const FlameModel &model) {
  XmlElems layers;
  for (const FlameModel::Func &func : model.funcs) {
    layers.push_back({ "layer", {
      { "gpu:layerFunction", {
        { "name", {{ func.name }} },
      }},
    }});
  }
  return layers;
}

static XmlElems createXmlEnv(AST::Script &script) {
  XmlElems env = {
    { "gpu:functionFiles", {
      { "file", {{ "functions.c" }} },
    }},
    { "gpu:initFunctions", {
      { "gpu:initFunction", {
        { "gpu:name", {{ "openabl_init" }} }
      }}
    }},
    { "gpu:exitFunctions", {
      { "gpu:exitFunction", {
        { "gpu:name", {{ "openabl_exit" }} }
      }}
    }}
  };

  AST::FunctionDeclaration *seqStep = script.simStmt->seqStepDecl;
  if (seqStep) {
    env.push_back({ "gpu:stepFunctions", {
      { "gpu:stepFunction", {
        { "gpu:name", {{ seqStep->name }} }
      }}
    }});
  }
  return env;
}

static std::string createXmlModel(
    AST::Script &script, const FlameModel &model, bool useFloat, long bufferSize) {
  XmlElems xagents = createXmlAgents(script, model, useFloat, bufferSize);
  XmlElems messages = createXmlMessages(script, model, useFloat, bufferSize);
  XmlElems layers = createXmlLayers(model);
  XmlElems environment = createXmlEnv(script);

  XmlElem root("gpu:xmodel", {
    { "name", {{ "TODO" }} },
    { "gpu:environment", environment },
    { "xagents", xagents },
    { "messages", messages },
    { "layers", layers },
  });
  root.setAttr("xmlns:gpu", "http://www.dcs.shef.ac.uk/~paul/XMMLGPU");
  root.setAttr("xmlns", "http://www.dcs.shef.ac.uk/~paul/XMML");
  XmlWriter writer;
  return writer.serialize(root);
}

static std::string createFunctionsFile(
    AST::Script &script, const FlameModel &model, bool useFloat) {
  FlameGPUPrinter printer(script, model, useFloat);
  printer.print(script);
  return printer.extractStr();
}

static std::string createMainFile(
    AST::Script &script, bool useFloat, bool visualize, bool profile) {
  FlameMainPrinter printer(script,
    FlameMainPrinter::Params::createForFlameGPU(useFloat, visualize, profile));
  printer.print(script);
  return printer.extractStr();
}

static std::string createBuildFile(bool visualize, bool profile) {
  std::string type = visualize ? "visualisation-ext" : "console-ext";
  std::string optionalProfile = profile ? " profile=1" : "";
  return "make " + type + optionalProfile + " && ./build_runner.sh";
}

static std::string createBuildRunner(bool useFloat) {
  if (useFloat) {
    return "gcc -O2 -std=c99 -DLIBABL_USE_FLOAT=1 runner.c libabl.c -lm -o runner";
  } else {
    return "gcc -O2 -std=c99 runner.c libabl.c -lm -o runner";
  }
}

static std::string createRunFile() {
  return "./runner";
}

void FlameGPUBackend::generate(AST::Script &script, const BackendContext &ctx) {
  bool useFloat = ctx.config.getBool("use_float", false);
  bool visualize = ctx.config.getBool("visualize", false);
  bool profile = ctx.config.getBool("profile", false);

  // TODO How to determine this value ???
  // For now just using an explicit configuration parameter
  long bufferSize = ctx.config.getInt("flamegpu.buffer_size", 1024);

  FlameModel model = FlameModel::generateFromScript(script);

  std::string assetDir = ctx.assetDir + "/flamegpu";
  std::string modelDir = ctx.outputDir + "/src/model";

  createDirectory(ctx.outputDir + "/src");
  createDirectory(ctx.outputDir + "/src/model");
  createDirectory(ctx.outputDir + "/src/dynamic");
  createDirectory(ctx.outputDir + "/src/visualisation");

  // Model and visualization files
  writeToFile(modelDir + "/XMLModelFile.xml",
      createXmlModel(script, model, useFloat, bufferSize));
  writeToFile(modelDir + "/functions.c", createFunctionsFile(script, model, useFloat));
  copyFile(assetDir + "/libabl_flamegpu.h", modelDir + "/libabl_flamegpu.h");
  copyFile(
      assetDir + "/visualisation.h",
      ctx.outputDir + "/src/visualisation/visualisation.h");

  copyFile(assetDir + "/Makefile", ctx.outputDir + "/Makefile");
  writeToFile(ctx.outputDir + "/runner.c", createMainFile(script, useFloat, visualize, profile));
  writeToFile(ctx.outputDir + "/build.sh", createBuildFile(visualize, profile));
  writeToFile(ctx.outputDir + "/build_runner.sh", createBuildRunner(useFloat));
  writeToFile(ctx.outputDir + "/run.sh", createRunFile());

  // These are required for runner.c
  copyFile(ctx.assetDir + "/c/libabl.h", ctx.outputDir + "/libabl.h");
  copyFile(ctx.assetDir + "/c/libabl.c", ctx.outputDir + "/libabl.c");

  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/build_runner.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");

  createDirectory(ctx.outputDir + "/iterations");
}

void FlameGPUBackend::initEnv(const BackendContext &ctx) {
  std::string flameGpuDir = ctx.depsDir + "/flamegpu";
  if (directoryExists(flameGpuDir)) {
    std::string realFlameGpuDir = flameGpuDir + "/FLAMEGPU";
    setenv("FLAMEGPU_DIR", realFlameGpuDir.c_str(), true);
  }
}

}

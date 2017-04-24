#include <sstream>
#include "Backend.hpp"
#include "FileUtil.hpp"
#include "XmlUtil.hpp"
#include "FlameModel.hpp"
#include "FlameGPUPrinter.hpp"

namespace OpenABL {

static FlameModel generateFlameModel(AST::Script &script) {
  FlameModel model;

  for (const AST::FunctionDeclaration *func : script.funcs) {
    if (!func->isStep()) {
      continue;
    }

    AST::AgentDeclaration &accessedAgent = *func->accessedAgent;
    const auto &accessedMembers = func->accessedMembers;

    AST::AgentDeclaration &stepAgent = *(*func->params)[0]->type->resolved.getAgentDecl();

    // Generate a new message with all the members accessed by this step function
    FlameModel::Message msg;
    msg.name = func->name + "_message";
    for (const AST::AgentMemberPtr &member : *accessedAgent.members) {
      if (accessedMembers.find(member->name) != accessedMembers.end()) {
        msg.members.push_back(&*member);
      }
    }
    model.messages.push_back(msg);

    // Add a function generating the message
    FlameModel::Func fnGen;
    fnGen.name = func->name + "_gen";
    fnGen.outMsgName = msg.name;
    fnGen.agent = &accessedAgent;
    model.funcs.push_back(fnGen);

    // Add the step function itself
    FlameModel::Func fnStep;
    fnStep.name = func->name;
    fnStep.inMsgName = msg.name;
    fnStep.agent = &stepAgent;
    fnStep.func = func;
    model.funcs.push_back(fnStep);
  }

  return model;
}

using KVPair = std::pair<std::string, std::string>;
using KVList = std::vector<KVPair>;

// Conversion to GPU types
static void pushMember(KVList &result, const std::string &name, Type type) {
  switch (type.getTypeId()) {
    case Type::INT32:
      result.push_back({ name, "int" });
      break;
    case Type::FLOAT32:
      result.push_back({ name, "float" });
      break;
    case Type::VEC2:
      result.push_back({ name + "_x", "float" });
      result.push_back({ name + "_y", "float" });
      break;
    case Type::VEC3:
      result.push_back({ name + "_x", "float" });
      result.push_back({ name + "_y", "float" });
      result.push_back({ name + "_z", "float" });
      break;
    default:
      assert(0);
      break;
  }
}

static KVList gpuAgentMembers(const AST::AgentMemberList &members) {
  KVList result;
  for (const AST::AgentMemberPtr &member : members) {
    pushMember(result, member->name, member->type->resolved);
  }

  return result;
}

static XmlElems createXmlAgents(AST::Script &script, const FlameModel &model) {
  XmlElems xagents;
  for (const AST::AgentDeclaration *decl : script.agents) {
    XmlElems members;
    for (const KVPair &member : gpuAgentMembers(*decl->members)) {
      members.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

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

      functions.push_back({ "gpu:function", {
        { "name", {{ func.name }} },
        { "currentState", {{ "default" }} },
        { "nextState", {{ "default" }} },
        { "inputs", inputs },
        { "outputs", outputs },
        { "gpu:reallocate", {{ "false" }} },
        { "gpu:RNG", {{ "false" }} },
      }});
    }

    xagents.push_back({ "gpu:xagent", {
      { "name", {{ decl->name }} },
      { "memory", members },
      { "functions", functions },
    }});
  }
  return xagents;
}

static KVList gpuMessageMembers(const std::vector<const AST::AgentMember *> &members) {
  KVList result;

  // We use the same field order as in the agent declaration
  for (const AST::AgentMember *member : members) {
    pushMember(result, member->name, member->type->resolved);
  }

  return result;
}

static XmlElems createXmlMessages(const FlameModel &model) {
  XmlElems messages;
  for (const FlameModel::Message &msg : model.messages) {
    XmlElems variables;
    for (const KVPair &member : gpuMessageMembers(msg.members)) {
      variables.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    messages.push_back({ "gpu:message", {
      { "name", {{ msg.name }} },
      { "variables", variables },
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

static std::string createXmlModel(AST::Script &script, const FlameModel &model) {
  XmlElems xagents = createXmlAgents(script, model);
  XmlElems messages = createXmlMessages(model);
  XmlElems layers = createXmlLayers(model);
  XmlElem root("gpu:model", {
    { "name", {{ "TODO" }} },
    { "gpu:environment", {
      { "gpu:functionFiles", {
        { "file", {{ "functions.c" }} },
      }}
    }},
    { "xagents", xagents },
    { "messages", messages },
    { "layers", layers },
  });
  root.setAttr("xmlns:gpu", "http://www.dcs.shef.ac.uk/~paul/XMMLGPU");
  root.setAttr("xmlns", "http://www.dcs.shef.ac.uk/~paul/XMML");
  XmlWriter writer;
  return writer.serialize(root);
}

static std::string createFunctionsFile(AST::Script &script, const FlameModel &model) {
  FlameGPUPrinter printer(script, model);
  printer.print(script);
  return printer.extractStr();
}

void FlameGPUBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  (void) assetDir;

  FlameModel model = generateFlameModel(script);

  createDirectory(outputDir + "/model");
  createDirectory(outputDir + "/dynamic");

  writeToFile(outputDir + "/model/XMLModelFile.xml", createXmlModel(script, model));
  writeToFile(outputDir + "/model/functions.c", createFunctionsFile(script, model));
}

}

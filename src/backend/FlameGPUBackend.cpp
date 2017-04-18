#include <sstream>
#include "Backend.hpp"
#include "FileUtil.hpp"
#include "XmlUtil.hpp"

namespace OpenABL {

using KVPair = std::pair<std::string, std::string>;
using KVList = std::vector<KVPair>;

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

static XmlElems createXmlAgents(AST::Script &script) {
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

    xagents.push_back({ "gpu:xagent", {
      { "name", {{ decl->name }} },
      { "memory", members },
      { "functions", functions },
    }});
  }
  return xagents;
}

static KVList gpuMessageMembers(
    const AST::AgentMemberList &members, const std::set<std::string> &accessed) {
  KVList result;

  // We use the same field order as in the agent declaration
  for (const AST::AgentMemberPtr &member : members) {
    if (accessed.find(member->name) != accessed.end()) {
      pushMember(result, member->name, member->type->resolved);
    }
  }

  return result;
}

static XmlElems createXmlMessages(AST::Script &script) {
  XmlElems messages;
  for (const AST::FunctionDeclaration *func : script.funcs) {
    if (!func->accessedAgent) {
      // Not a step function
      continue;
    }

    XmlElems variables;
    for (const KVPair &member : gpuMessageMembers(
          *func->accessedAgent->members, func->accessedMembers)) {
      variables.push_back({ "gpu:variable", {
        { "type", {{ member.second }} },
        { "name", {{ member.first }} },
      }});
    }

    messages.push_back({ "gpu:message", {
      { "name", {{ func->name + "_message" }} },
      { "variables", variables },
    }});
  }
  return messages;
}

static std::string createXmlModel(AST::Script &script) {
  XmlElems xagents = createXmlAgents(script);
  XmlElems messages = createXmlMessages(script);
  XmlElem root("gpu:model", {
    { "name", {{ "TODO" }} },
    { "gpu:environment", {
      { "gpu:functionFiles", {
        { "file", {{ "functions.c" }} },
      }}
    }},
    { "xagents", xagents },
    { "messages", messages },
    { "layers", {} },
  });
  root.setAttr("xmlns:gpu", "http://www.dcs.shef.ac.uk/~paul/XMMLGPU");
  root.setAttr("xmlns", "http://www.dcs.shef.ac.uk/~paul/XMML");
  XmlWriter writer;
  return writer.serialize(root);
}

void FlameGPUBackend::generate(
    AST::Script &script, const std::string &outputDir, const std::string &assetDir) {
  (void) assetDir;

  writeToFile(outputDir + "/XMLModelFile.xml", createXmlModel(script));
}

}

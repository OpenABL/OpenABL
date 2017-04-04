#include <sstream>
#include "Backend.hpp"
#include "FileUtil.hpp"
#include "XmlUtil.hpp"

namespace OpenABL {

using KVPair = std::pair<std::string, std::string>;
using KVList = std::vector<KVPair>;

KVList gpuAgentMembers(const AST::AgentMemberList &members) {
  KVList result;
  for (const AST::AgentMemberPtr &member : members) {
    const std::string &name = member->name;
    switch (member->type->resolved.getTypeId()) {
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

  return result;
}

static std::string createXmlModel(AST::Script &script) {
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

  XmlElem root("gpu:model", {
    { "name", {{ "TODO" }} },
    { "gpu:environment", {
      { "gpu:functionFiles", {
        { "file", {{ "functions.c" }} },
      }}
    }},
    { "xagents", xagents },
    { "messages", {} },
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

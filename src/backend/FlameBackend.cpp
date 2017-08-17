#include "Backend.hpp"
#include "XmlUtil.hpp"
#include "FileUtil.hpp"
#include "FlameModel.hpp"
#include "FlamePrinter.hpp"
#include "FlameMainPrinter.hpp"

namespace OpenABL {

static XmlElems createXmlAgents(AST::Script &script, const FlameModel &model) {
  XmlElems agents;
  for (const AST::AgentDeclaration *decl : script.agents) {
    XmlElems members, functions;

    auto unpackedMembers = FlameModel::getUnpackedMembers(*decl->members);
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

static XmlElems createXmlMessages(const FlameModel &model) {
  XmlElems messages;
  for (const FlameModel::Message &msg : model.messages) {
    XmlElems variables;
    auto unpackedMembers = FlameModel::getUnpackedMembers(msg.members);
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

static std::string createXmlModel(AST::Script &script, const FlameModel &model) {
  XmlElems agents = createXmlAgents(script, model);
  XmlElems messages = createXmlMessages(model);
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

static std::string createFunctionsFile(AST::Script &script, const FlameModel &model) {
  FlamePrinter printer(script, model);
  printer.print(script);
  return printer.extractStr();
}

static std::string createMainFile(AST::Script &script) {
  FlameMainPrinter printer(script, false);
  printer.print(script);
  return printer.extractStr();
}

void FlameBackend::generate(AST::Script &script, const BackendContext &ctx) {
  FlameModel model = FlameModel::generateFromScript(script);

  writeToFile(ctx.outputDir + "/XMLModelFile.xml", createXmlModel(script, model));
  writeToFile(ctx.outputDir + "/functions.c", createFunctionsFile(script, model));
  writeToFile(ctx.outputDir + "/runner.c", createMainFile(script));

  copyFile(ctx.assetDir + "/c/libabl.h", ctx.outputDir + "/libabl.h");
  copyFile(ctx.assetDir + "/c/libabl.c", ctx.outputDir + "/libabl.c");
  copyFile(ctx.assetDir + "/flame/build.sh", ctx.outputDir + "/build.sh");
  copyFile(ctx.assetDir + "/flame/run.sh", ctx.outputDir + "/run.sh");
  makeFileExecutable(ctx.outputDir + "/build.sh");
  makeFileExecutable(ctx.outputDir + "/run.sh");

  createDirectory(ctx.outputDir + "/iterations");
}

}

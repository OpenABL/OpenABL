#include "FlameModel.hpp"

namespace OpenABL {

FlameModel FlameModel::generateFromScript(AST::Script &script) {
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

// Conversion to unpacked types
static void pushMember(FlameModel::MemberList &result, const std::string &name, Type type) {
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

FlameModel::MemberList FlameModel::getUnpackedMembers(const AST::AgentMemberList &members) {
  FlameModel::MemberList result;
  for (const AST::AgentMemberPtr &member : members) {
    pushMember(result, member->name, member->type->resolved);
  }
  return result;
}

FlameModel::MemberList FlameModel::getUnpackedMembers(const std::vector<const AST::AgentMember *> &members) {
  FlameModel::MemberList result;
  for (const AST::AgentMember *member : members) {
    pushMember(result, member->name, member->type->resolved);
  }
  return result;
}


}

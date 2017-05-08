#include "FlameModel.hpp"

namespace OpenABL {

static std::string getStateName(unsigned n) {
  if (n == 0) {
    return "start";
  }

  return std::to_string(n);
}

FlameModel FlameModel::generateFromScript(AST::Script &script) {
  FlameModel model;
  if (!script.simStmt) {
    return model;
  }

  std::map<const AST::AgentDeclaration *, unsigned> numStates;
  auto stepFuncs = script.simStmt->stepFuncDecls;
  for (const AST::FunctionDeclaration *func : stepFuncs) {
    AST::AgentDeclaration &accessedAgent = *func->accessedAgent;
    const auto &accessedMembers = func->accessedMembers;
    AST::AgentDeclaration &stepAgent = func->stepAgent();

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
    unsigned &genState = numStates[&accessedAgent];
    fnGen.currentState = getStateName(genState++);
    fnGen.nextState = getStateName(genState);
    model.funcs.push_back(fnGen);

    // Add the step function itself
    FlameModel::Func fnStep;
    fnStep.name = func->name;
    fnStep.inMsgName = msg.name;
    fnStep.agent = &stepAgent;
    fnStep.func = func;
    unsigned &stepState = numStates[&stepAgent];
    fnStep.currentState = getStateName(stepState++);
    fnStep.nextState = getStateName(stepState);
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

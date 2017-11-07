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
    const auto &accessedMembers = func->accessedMembers;
    AST::AgentDeclaration &stepAgent = func->stepAgent();

    // Generate a new message with all the members accessed by this step function
    // If no members of other agents are accessed, we don't use the message
    FlameModel::Message msg;
    if (func->accessedAgent) {
      const AST::AgentDeclaration &accessedAgent = *func->accessedAgent;
      msg.name = func->name + "_message";
      for (const AST::AgentMemberPtr &member : *accessedAgent.members) {
        // The position member is always included, as it is necessary for the distance
        // comparison
        if (member->isPosition ||
            accessedMembers.find(member->name) != accessedMembers.end()) {
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
    }

    // Add the step function itself
    FlameModel::Func fnStep;
    fnStep.name = func->name;
    fnStep.inMsgName = msg.name;
    fnStep.agent = &stepAgent;
    fnStep.addedAgent = func->runtimeAddedAgent;
    fnStep.func = func;
    unsigned &stepState = numStates[&stepAgent];
    fnStep.currentState = getStateName(stepState++);
    fnStep.nextState = getStateName(stepState);
    model.funcs.push_back(fnStep);
  }

  return model;
}

// Conversion to unpacked types
static void pushMemberInfo(
    FlameModel::MemberList &result, const AST::AgentMember &member, bool useFloat, bool forGpu) {
  const std::string &name = member.name;
  Type type = member.type->resolved;

  // We support both float and double types
  const char *floatType = useFloat ? "float" : "double";

  // FlameGPU requires that the position members are always 3D with names x, y, z
  if (forGpu && member.isPosition) {
    result.push_back({ "x", floatType });
    result.push_back({ "y", floatType });
    result.push_back({ "z", floatType });
    // If we are in a 2D environment, technically we only need the "z" member in the
    // message type (because FlameGPU assumes it exists), but not in the agent type.
    // I'm including it in the agent anyway, both for simplicity and because I'm not
    // sure about the performance implications. (TODO)
    return;
  }

  switch (type.getTypeId()) {
    case Type::BOOL:
      // Flame/FlameGPU do not support booleans in agent members.
      // Rewrite to integer instead.
      result.push_back({ name, "int" });
      break;
    case Type::INT32:
      result.push_back({ name, "int" });
      break;
    case Type::FLOAT:
      result.push_back({ name, floatType });
      break;
    case Type::VEC2:
      result.push_back({ name + "_x", floatType });
      result.push_back({ name + "_y", floatType });
      break;
    case Type::VEC3:
      result.push_back({ name + "_x", floatType });
      result.push_back({ name + "_y", floatType });
      result.push_back({ name + "_z", floatType });
      break;
    default:
      assert(0);
      break;
  }
}

FlameModel::MemberList FlameModel::getUnpackedMembers(
    const AST::AgentMemberList &members, bool useFloat, bool forGpu) {
  FlameModel::MemberList result;
  for (const AST::AgentMemberPtr &member : members) {
    pushMemberInfo(result, *member, useFloat, forGpu);
  }
  return result;
}

FlameModel::MemberList FlameModel::getUnpackedMembers(
    const std::vector<const AST::AgentMember *> &members, bool useFloat, bool forGpu) {
  FlameModel::MemberList result;
  for (const AST::AgentMember *member : members) {
    pushMemberInfo(result, *member, useFloat, forGpu);
  }
  return result;
}


}

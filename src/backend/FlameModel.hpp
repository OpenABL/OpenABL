#pragma once

#include "AST.hpp"

namespace OpenABL {

struct FlameModel {
  // Name and type, used to represent members in unpacked
  // format (separate members for each component of a vector)
  using Member = std::pair<std::string, std::string>;
  using MemberList = std::vector<Member>;

  struct Message {
    std::string name;
    std::vector<const AST::AgentMember *> members;
  };
  struct Func {
    std::string name;
    std::string inMsgName;
    std::string outMsgName;
    std::string currentState;
    std::string nextState;
    const AST::AgentDeclaration *agent = nullptr;
    const AST::FunctionDeclaration *func = nullptr;
    const AST::AgentDeclaration *addedAgent = nullptr;
  };

  const Message *getMessageByName(const std::string &name) const {
    for (const Message &msg : messages) {
      if (msg.name == name) {
        return &msg;
      }
    }
    return nullptr;
  }

  static FlameModel generateFromScript(AST::Script &);

  static MemberList getUnpackedMembers(const AST::AgentMemberList &, bool useFloat, bool forGpu);
  static MemberList getUnpackedMembers(
      const std::vector<const AST::AgentMember *> &, bool useFloat, bool forGpu);

  std::vector<Message> messages;
  std::vector<Func> funcs;
};

}

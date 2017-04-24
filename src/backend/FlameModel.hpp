#pragma once

#include "AST.hpp"

namespace OpenABL {

struct FlameModel {
  struct Message {
    std::string name;
    std::vector<const AST::AgentMember *> members;
  };
  struct Func {
    std::string name;
    std::string inMsgName;
    std::string outMsgName;
    const AST::AgentDeclaration *agent = nullptr;
    const AST::FunctionDeclaration *func = nullptr;
  };

  const Message *getMessageByName(const std::string &name) const {
    for (const Message &msg : messages) {
      if (msg.name == name) {
        return &msg;
      }
    }
    return nullptr;
  }

  std::vector<Message> messages;
  std::vector<Func> funcs;
};

}

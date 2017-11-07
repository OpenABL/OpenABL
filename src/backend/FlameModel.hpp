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

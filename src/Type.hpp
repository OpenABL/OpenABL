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

#include <cassert>
#include <string>
#include <vector>

namespace OpenABL {

namespace AST {
  struct AgentDeclaration;
  struct AgentMember;
}

struct Type {
  enum TypeId {
    INVALID,
    VOID,
    BOOL,
    INT32,
    FLOAT,
    STRING,
    VEC2,
    VEC3,
    AGENT,
    ARRAY,
    AGENT_TYPE,   // Reference to the agent type itself
    AGENT_MEMBER, // Reference to a member of an agent type
    UNRESOLVED,   // Unresolved return type, computed from args
  };

  Type() : type{INVALID} {}
  Type(TypeId type)
    : type{type}, agent{nullptr}, member{nullptr}
  { assert(type != ARRAY); }

  Type(TypeId type, AST::AgentDeclaration *agent)
    : type{type}, agent{agent}, member{nullptr}
  { assert(type == AGENT || type == AGENT_TYPE); }

  Type(TypeId type, AST::AgentDeclaration *agent, AST::AgentMember *member)
    : type{type}, agent{agent}, member{member}
  { assert(type == AGENT_MEMBER); }

  Type(TypeId type, const Type &base)
    : type{type}, baseType{base.type},
      agent{base.agent}, member{base.member}
  { assert(type == ARRAY); }

  bool operator==(const Type &other) const {
    if (type != other.type) {
      return false;
    }
    if (type == ARRAY) {
      return getBaseType() == other.getBaseType();
    }
    return agent == other.agent && member == other.member;
  }
  bool operator!=(const Type &other) const {
    return !(*this == other);
  }

  bool isCompatibleWith(const Type &other) const {
    return isCompatibleWith(other, false);
  }

  bool isPromotableTo(const Type &other) const {
    return isCompatibleWith(other, true);
  }

  TypeId getTypeId() const { return type; }

  Type getBaseType() const {
    assert(isArray());
    if (baseType == AGENT) {
      return { AGENT, agent };
    } else {
      return { baseType };
    }
  }

  AST::AgentDeclaration *getAgentDecl() const {
    assert(canHaveAgent());
    return agent;
  }

  AST::AgentMember *getAgentMember() const {
    assert(isAgentMember());
    return member;
  }

  unsigned getVecLen() const {
    assert(isVec());
    return type == VEC2 ? 2 : 3;
  }

  std::vector<std::string> getVecMembers() const {
    assert(isVec());
    return type == VEC2 ? vec2Members : vec3Members;
  }

  bool isInvalid() const { return type == INVALID; }
  bool isVoid() const { return type == VOID; }
  bool isArray() const { return type == ARRAY; }
  bool isAgent() const { return type == AGENT; }
  bool isAgentType() const { return type == AGENT_TYPE; }
  bool isAgentMember() const { return type == AGENT_MEMBER; }
  bool isVec() const { return type == VEC2 || type == VEC3; }
  bool isVec2() const { return type == VEC2; }
  bool isVec3() const { return type == VEC3; }
  bool isNum() const { return type == INT32 || type == FLOAT; }
  bool isNumOrVec() const { return isNum() || isVec(); }
  bool isInt() const { return type == INT32; }
  bool isFloat() const { return type == FLOAT; }
  bool isBool() const { return type == BOOL; }
  bool isString() const { return type == STRING; }
  bool isUnresolved() const { return type == UNRESOLVED; }

  bool canHaveAgent() const {
    return isAgent() || isAgentType() || isAgentMember();
  }

  bool isGenericAgent() const {
    return canHaveAgent() && agent == nullptr;
  }
  bool isGenericAgentArray() const {
    return type == ARRAY && baseType == AGENT && agent == nullptr;
  }

  size_t calcHash() const {
    return std::hash<int>()(type)
         ^ std::hash<int>()(baseType)
         ^ std::hash<AST::AgentDeclaration *>()(agent)
         ^ std::hash<AST::AgentMember *>()(member);
  }

private:
  static const std::vector<std::string> vec2Members;
  static const std::vector<std::string> vec3Members;

  bool isCompatibleWith(const Type &other, bool allowPromotion) const {
    if (type != other.type) {
      if (allowPromotion && type == INT32) {
        // Integer to float promotion
        return other.type == FLOAT;
      }
      return false;
    }

    if (type == AGENT) {
      // nullptr indicates "any agent type"
      return agent == other.agent || other.agent == nullptr;
    }

    if (type == ARRAY) {
      return getBaseType().isCompatibleWith(other.getBaseType(), false);
    }

    return true;
  }

  TypeId type;
  // Base type for ARRAY type. We support simple arrays only, for now.
  TypeId baseType;

  // Agent declaration for AGENT, AGENT_TYPE and AGENT_MEMBER
  AST::AgentDeclaration *agent;
  // Agent member for AGENT_MEMBER
  AST::AgentMember *member;
};

std::ostream &operator<<(std::ostream &, const Type &);

}

namespace std {
  template<> struct hash<OpenABL::Type> {
    std::size_t operator()(const OpenABL::Type &t) const {
      return t.calcHash();
    }
  };
}

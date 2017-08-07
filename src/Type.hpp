#pragma once

#include <cassert>
#include <string>
#include <vector>

namespace OpenABL {

namespace AST {
  struct AgentDeclaration;
}

struct Type {
  enum TypeId {
    INVALID,
    VOID,
    BOOL,
    INT32,
    FLOAT32,
    STRING,
    VEC2,
    VEC3,
    AGENT,
    ARRAY,
  };

  Type() : type{INVALID} {}
  Type(TypeId type)
    : type{type}, agent{nullptr}
  { assert(type != ARRAY); }

  Type(TypeId type, AST::AgentDeclaration *agent)
    : type{type}, agent{agent}
  { assert(type == AGENT); }

  Type(TypeId type, const Type &base)
    : type{type}, baseType{base.type}, agent{base.agent}
  { assert(type == ARRAY); }

  bool operator==(const Type &other) const {
    if (type != other.type) {
      return false;
    }
    switch (type) {
      case AGENT:
        return agent == other.agent;
      case ARRAY:
        return getBaseType() == other.getBaseType();
      default:
        return true;
    }
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
    assert(isAgent());
    return agent;
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
  bool isVec() const { return type == VEC2 || type == VEC3; }
  bool isVec2() const { return type == VEC2; }
  bool isVec3() const { return type == VEC3; }
  bool isNum() const { return type == INT32 || type == FLOAT32; }
  bool isNumOrVec() const { return isNum() || isVec(); }
  bool isInt() const { return type == INT32; }
  bool isFloat() const { return type == FLOAT32; }
  bool isBool() const { return type == BOOL; }
  bool isString() const { return type == STRING; }

  bool isGenericAgent() const {
    return type == AGENT && agent == nullptr;
  }
  bool isGenericAgentArray() const {
    return type == ARRAY && baseType == AGENT && agent == nullptr;
  }

private:
  static const std::vector<std::string> vec2Members;
  static const std::vector<std::string> vec3Members;

  bool isCompatibleWith(const Type &other, bool allowPromotion) const {
    if (type != other.type) {
      if (allowPromotion && type == INT32) {
        // Integer to float promotion
        return other.type == FLOAT32;
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
  // Agent declaration for AGENT type
  AST::AgentDeclaration *agent;
};

std::ostream &operator<<(std::ostream &, const Type &);

}
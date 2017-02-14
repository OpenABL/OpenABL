#pragma once

#include <map>
#include <cassert>

namespace OpenABL {

namespace AST {
  struct AgentDeclaration;
}

// Globally unique variable id, to distinguish
// different variables that have the same name.
struct VarId {
  static VarId make() {
    return VarId { ++max_id };
  }

  bool operator<(const VarId &other) const {
    return id < other.id;
  }

  VarId() : id{0} {}

private:
  VarId(uint32_t id) : id{id} {}

  static uint32_t max_id;
  uint32_t id;
};

struct Type {
  enum TypeId {
    INVALID,
    VOID,
    BOOL,
    INT32,
    FLOAT32,
    VEC2,
    VEC3,
    AGENT,
    ARRAY,
  };

  Type(TypeId type)
    : type{type} { assert(type != AGENT && type != ARRAY); }
  Type(TypeId type, AST::AgentDeclaration *agent)
    : type{type}, agent{agent} { assert(type == AGENT); }
  Type(TypeId type, TypeId baseType)
    : type{type}, baseType{baseType} { assert(type == ARRAY); }

  bool operator==(const Type &other) const {
    if (type != other.type) {
      return false;
    }
    switch (type) {
      case AGENT:
        return agent == other.agent;
      case ARRAY:
        return baseType == other.baseType;
      default:
        return true;
    }
  }
  bool operator!=(const Type &other) const {
    return !(*this == other);
  }

  TypeId getTypeId() const { return type; }
  TypeId getBaseTypeId() const {
    assert(isArray());
    return baseType;
  }
  AST::AgentDeclaration *getAgentDecl() const {
    assert(isAgent());
    return agent;
  }

  bool isInvalid() const { return type == INVALID; }
  bool isArray() const { return type == ARRAY; }
  bool isAgent() const { return type == AGENT; }
  bool isVec() const { return type == VEC2 || type == VEC3; }
  bool isNum() const { return type == INT32 || type == FLOAT32; }
  bool isNumOrVec() const { return isNum() || isVec(); }
  bool isInt() const { return type == INT32; }
  bool isFloat() const { return type == FLOAT32; }

private:
  TypeId type;
  // Base type for ARRAY type. We support simple arrays only, for now.
  TypeId baseType;
  // Agent declaration for AGENT type
  AST::AgentDeclaration *agent;
};

std::ostream &operator<<(std::ostream &, Type);

struct ScopeEntry {
  Type type;
};

struct Scope {
  void add(VarId var, Type type) {
    vars.insert({ var, ScopeEntry { type } });
  }
  ScopeEntry &get(VarId var) {
    auto it = vars.find(var);
    return it->second;
  }

private:
  std::map<VarId, ScopeEntry> vars;
};

}

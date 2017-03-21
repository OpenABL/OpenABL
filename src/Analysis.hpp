#pragma once

#include <map>
#include <vector>
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

  bool isInvalid() const { return type == INVALID; }
  bool isArray() const { return type == ARRAY; }
  bool isAgent() const { return type == AGENT; }
  bool isVec() const { return type == VEC2 || type == VEC3; }
  bool isNum() const { return type == INT32 || type == FLOAT32; }
  bool isNumOrVec() const { return isNum() || isVec(); }
  bool isInt() const { return type == INT32; }
  bool isFloat() const { return type == FLOAT32; }

private:
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

struct FunctionSignature {
  bool isCompatibleWith(const std::vector<Type> &argTypes) const {
    if (argTypes.size() != paramTypes.size()) {
      return false;
    }

    for (size_t i = 0; i < argTypes.size(); i++) {
      if (!argTypes[i].isCompatibleWith(paramTypes[i])) {
        return false;
      }
    }

    return true;
  }

  std::string name;
  std::vector<Type> paramTypes;
  Type returnType;
};

struct BuiltinFunction {
  const FunctionSignature *getCompatibleSignature(const std::vector<Type> &argTypes) const {
    for (const FunctionSignature &sig : signatures) {
      if (sig.isCompatibleWith(argTypes)) {
        return &sig;
      }
    }

    return nullptr;
  }

  std::string name;
  std::vector<FunctionSignature> signatures;
};

struct BuiltinFunctions {
  std::map<std::string, BuiltinFunction> funcs;

  void add(const std::string &name, const std::string &sigName,
           std::vector<Type> argTypes, Type returnType) {
    funcs[name].signatures.push_back({ sigName, argTypes, returnType });
  }

  BuiltinFunction *getByName(const std::string &name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
      return nullptr;
    }
    return &it->second;
  }
};

}

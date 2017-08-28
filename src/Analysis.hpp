#pragma once

#include <map>
#include <vector>
#include <cassert>

#include "Type.hpp"

namespace OpenABL {

struct FunctionSignature;

namespace AST {
  struct AgentDeclaration;
  struct FunctionDeclaration;
  struct Expression;

  // TODO Move out of AST namespace
  enum class UnaryOp {
    MINUS,
    PLUS,
    LOGICAL_NOT,
    BITWISE_NOT,
  };

  // TODO Move out of AST namespace
  enum class BinaryOp {
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    BITWISE_AND,
    BITWISE_XOR,
    BITWISE_OR,
    SHIFT_LEFT,
    SHIFT_RIGHT,
    EQUALS,
    NOT_EQUALS,
    SMALLER,
    SMALLER_EQUALS,
    GREATER,
    GREATER_EQUALS,
    LOGICAL_AND,
    LOGICAL_OR,
    RANGE,
  };
}

// Globally unique variable id, to distinguish
// different variables that have the same name.
struct VarId {
  static VarId make() {
    return VarId { ++max_id };
  }

  bool operator==(const VarId &other) const {
    return id == other.id;
  }
  bool operator<(const VarId &other) const {
    return id < other.id;
  }

  VarId() : id{0} {}
  void reset() { id = 0; }

private:
  VarId(uint32_t id) : id{id} {}

  static uint32_t max_id;
  uint32_t id;
};

struct Value {
  struct Vec2 {
    double x;
    double y;

    bool operator==(const Vec2 &other) const {
      return x == other.x && y == other.y;
    }
    bool operator!=(const Vec2 &other) const { return !(*this == other); }
  };
  struct Vec3 {
    double x;
    double y;
    double z;

    bool operator==(const Vec3 &other) const {
      return x == other.x && y == other.y && z == other.z;
    }
    bool operator!=(const Vec3 &other) const { return !(*this == other); }
  };

  Value(const Value &other) {
    type = other.type;
    switch (type.getTypeId()) {
      case Type::INVALID:
        break;
      case Type::BOOL:
        bval = other.bval;
        break;
      case Type::INT32:
        ival = other.ival;
        break;
      case Type::FLOAT:
        fval = other.fval;
        break;
      case Type::STRING:
        str = other.str;
        break;
      case Type::VEC2:
        vec2 = other.vec2;
        break;
      case Type::VEC3:
        vec3 = other.vec3;
        break;
      default:
        assert(0);
    }
  }

  Value &operator=(const Value &other) {
    if (this != &other) {
      // Sorry
      this->~Value();
      new(this) Value(other);
    }
    return *this;
  }

  Value() {
    type = Type::INVALID;
  }

  Value(bool b) {
    type = Type::BOOL;
    bval = b;
  }

  Value(long i) {
    type = Type::INT32;
    ival = i;
  }

  Value(double f) {
    type = Type::FLOAT;
    fval = f;
  }

  Value(std::string s) {
    type = Type::STRING;
    str = s;
  }

  Value(double v1, double v2) {
    type = Type::VEC2;
    vec2 = { v1, v2 };
  }

  Value(double v1, double v2, double v3) {
    type = Type::VEC3;
    vec3 = { v1, v2, v3 };
  }

  bool isInvalid() const { return type.isInvalid(); }
  bool isValid() const { return !isInvalid(); }
  bool isBool() const { return type.isBool(); }
  bool isInt() const { return type.isInt(); }
  bool isFloat() const { return type.isFloat(); }
  bool isNum() const { return type.isNum(); }
  bool isVec() const { return type.isVec(); }
  bool isVec2() const { return type.isVec2(); }
  bool isVec3() const { return type.isVec3(); }

  Type getType() const {
    return type;
  }

  bool getBool() const {
    assert(type.isBool());
    return bval;
  }
  long getInt() const {
    assert(type.isInt());
    return ival;
  }
  double getFloat() const {
    assert(type.isFloat());
    return fval;
  }
  const std::string &getString() const {
    assert(type.isString());
    return str;
  }
  Vec2 getVec2() const {
    assert(type.isVec2());
    return vec2;
  }
  Vec3 getVec3() const {
    assert(type.isVec3());
    return vec3;
  }
  std::vector<double> getVec() const {
    if (isVec2()) {
      return { vec2.x, vec2.y };
    } else if (isVec3()) {
      return { vec3.x, vec3.y, vec3.z };
    } else {
      assert(0);
    }
  }

  double asFloat() const {
    if (isFloat()) {
      return fval;
    } else if (isInt()) {
      return (double) ival;
    } else {
      assert(0);
    }
  }

  Value toBoolExplicit() const {
    if (isBool()) {
      return *this;
    } else if (isInt()) {
      return (bool) getInt();
    } else if (isFloat()) {
      return (bool) getFloat();
    }
    return {};
  }

  Value toIntExplicit() const {
    if (isInt()) {
      return *this;
    } else if (isFloat()) {
      return (long) getFloat();
    } else if (isBool()) {
      return (long) getBool();
    }
    return {};
  }

  Value toFloatExplicit() const {
    if (isFloat()) {
      return *this;
    } else if (isInt()) {
      return (double) getInt();
    } else if (isBool()) {
      return (double) getBool();
    }
    return {};
  }

  Value toFloatImplicit() const {
    if (isFloat()) {
      return *this;
    } else if (isInt()) {
      return (double) getInt();
    }
    return {};
  }

  Value extendToVec3() const {
    if (isVec3()) {
      return *this;
    } else if (isVec2()) {
      return { vec2.x, vec2.y, 0 };
    } else {
      return {};
    }
  }

  ~Value() {
    if (type.isString()) {
      str.~basic_string();
    }
  }

  AST::Expression *toExpression() const;

  static Value fromString(const std::string &str);
  static Value calcUnaryOp(AST::UnaryOp op, const Value &val);
  static Value calcBinaryOp(AST::BinaryOp op, const Value &left, const Value &right);
  static Value calcBuiltinCall(const FunctionSignature &sig, const std::vector<Value> &args);

private:
  Type type;
  union {
    bool bval;
    long ival;
    double fval;
    std::string str;
    Vec2 vec2;
    Vec3 vec3;
  };
};

std::ostream &operator<<(std::ostream &s, const Value &value);

struct ScopeEntry {
  Type type;
  bool isConst;
  bool isGlobal;
  Value val;
};

struct Scope {
  void add(VarId var, Type type, bool isConst, bool isGlobal, Value val) {
    vars.insert({ var, ScopeEntry { type, isConst, isGlobal, val } });
  }
  bool has(VarId var) const {
    return vars.find(var) != vars.end();
  }
  const ScopeEntry &get(VarId var) const {
    auto it = vars.find(var);
    return it->second;
  }

private:
  std::map<VarId, ScopeEntry> vars;
};

struct FunctionSignature {
  FunctionSignature()
    : origName(""), name(""), paramTypes(), returnType(), decl(nullptr) {}
  FunctionSignature(const std::string &origName, const std::string &name,
                    const std::vector<Type> &paramTypes, Type returnType,
                    const AST::FunctionDeclaration *decl)
    : origName(origName), name(name),
      paramTypes(paramTypes), returnType(returnType), decl(decl) {}

  bool isCompatibleWith(const std::vector<Type> &argTypes) const {
    if (argTypes.size() != paramTypes.size()) {
      return false;
    }

    for (size_t i = 0; i < argTypes.size(); i++) {
      if (!argTypes[i].isPromotableTo(paramTypes[i])) {
        return false;
      }
    }

    return true;
  }

  bool isConflictingWith(const std::vector<Type> &newParamTypes) const {
    if (newParamTypes.size() != paramTypes.size()) {
      // Always allow arity overloading
      return false;
    }

    // For now we are very conservative and only allow overloads between float, float2 and float3.
    // This may be extended as necessary in the future.
    bool haveAllowedDiff = false, haveOtherDiff = false;
    for (size_t i = 0; i < newParamTypes.size(); i++) {
      Type newParamType = newParamTypes[i], paramType = paramTypes[i];
      if (newParamType != paramType) {
        if ((paramType.isVec() || paramType.isFloat())
            && (newParamType.isVec() || newParamType.isFloat())) {
          haveAllowedDiff = true;
        } else {
          haveOtherDiff = true;
        }
      }
    }

    return haveOtherDiff || !haveAllowedDiff;
  }

  // Concrete signature with any generic agent types replaced
  FunctionSignature getConcreteSignature(const std::vector<Type> &argTypes) const {
    std::vector<Type> newParamTypes;
    Type newReturnType;
    Type agentType = Type::AGENT;
    for (size_t i = 0; i < paramTypes.size(); i++) {
      Type type = paramTypes[i];
      if (type.isGenericAgent()) {
        agentType = argTypes[i];
        newParamTypes.push_back(argTypes[i]);
      } else if (type.isGenericAgentArray()) {
        agentType = argTypes[i].getBaseType();
        newParamTypes.push_back({ Type::ARRAY, argTypes[i].getBaseType() });
      } else {
        newParamTypes.push_back(type);
      }
    }

    if (returnType.isGenericAgent()) {
      newReturnType = agentType;
    } else if (returnType.isGenericAgentArray()) {
      newReturnType = { Type::ARRAY, agentType };
    } else {
      newReturnType = returnType;
    }

    return { origName, name, newParamTypes, newReturnType, decl };
  }

  std::string origName;
  std::string name;
  std::vector<Type> paramTypes;
  Type returnType;
  const AST::FunctionDeclaration *decl;
};

struct Function {
  const FunctionSignature *getCompatibleSignature(const std::vector<Type> &argTypes) const {
    for (const FunctionSignature &sig : signatures) {
      if (sig.isCompatibleWith(argTypes)) {
        return &sig;
      }
    }

    return nullptr;
  }

  // Get signature that conflicts for the purpose of overloading
  const FunctionSignature *getConflictingSignature(const std::vector<Type> &paramTypes) const {
    for (const FunctionSignature &sig : signatures) {
      if (sig.isConflictingWith(paramTypes)) {
        return &sig;
      }
    }
    return nullptr;
  }

  std::string name;
  std::vector<FunctionSignature> signatures;
};

struct FunctionList {
  std::map<std::string, Function> funcs;

  void add(FunctionSignature sig) {
    funcs[sig.origName].signatures.push_back(sig);
  }
  void add(const std::string &name, const std::string &sigName,
           std::vector<Type> argTypes, Type returnType,
           const AST::FunctionDeclaration *decl = nullptr) {
    add({ name, sigName, argTypes, returnType, decl });
  }
  void add(const std::string &name, std::vector<Type> argTypes, Type returnType,
           const AST::FunctionDeclaration *decl = nullptr) {
    add(name, name, argTypes, returnType, decl);
  }

  Function *getByName(const std::string &name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
      return nullptr;
    }
    return &it->second;
  }
};

}

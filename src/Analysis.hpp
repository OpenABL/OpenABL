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

#include <map>
#include <vector>
#include <functional>
#include <cassert>

#include "Type.hpp"
#include "Value.hpp"

namespace OpenABL {

// Globally unique variable id, to distinguish
// different variables that have the same name.
struct VarId {
  static VarId make() {
    return VarId { ++max_id };
  }

  bool operator==(const VarId &other) const {
    return id == other.id;
  }
  bool operator!=(const VarId &other) const {
    return id != other.id;
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
  static const unsigned MAIN_ONLY     = 1 << 0;
  static const unsigned STEP_ONLY     = 1 << 1;
  static const unsigned SEQ_STEP_ONLY = 1 << 2;

  static const unsigned MAIN_STEP_ONLY = MAIN_ONLY | STEP_ONLY;

  FunctionSignature()
    : origName(""), name(""), paramTypes(), returnType(), flags(0), decl(nullptr) {}
  FunctionSignature(const std::string &origName, const std::string &name,
                    const std::vector<Type> &paramTypes, Type returnType,
                    unsigned flags, const AST::FunctionDeclaration *decl)
    : origName(origName), name(name),
      paramTypes(paramTypes), returnType(returnType),
      flags(flags), decl(decl) {}

  bool isCompatibleWith(const std::vector<Type> &argTypes) const {
    if (customIsCompatibleWith) {
      return customIsCompatibleWith(argTypes);
    }

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

    // Don't allow overloading between bool, int and float
    bool haveDiff = false;
    for (size_t i = 0; i < newParamTypes.size(); i++) {
      Type newParamType = newParamTypes[i], paramType = paramTypes[i];
      if (newParamType != paramType) {
        haveDiff = true;
        if ((paramType.isBool() || paramType.isInt() || paramType.isFloat())
            && (newParamType.isBool() || newParamType.isInt() || newParamType.isFloat())) {
          return true;
        }
      }
    }

    return !haveDiff;
  }

  // Concrete signature with any generic agent types replaced
  FunctionSignature getConcreteSignature(const std::vector<Type> &argTypes) const;

  std::string origName;
  std::string name;
  std::vector<Type> paramTypes;
  Type returnType;
  unsigned flags;
  const AST::FunctionDeclaration *decl;
  std::function<bool(const std::vector<Type> &)> customIsCompatibleWith = {};
  std::function<FunctionSignature(const std::vector<Type> &)> customGetConcreteSignature = {};
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
           std::vector<Type> argTypes, Type returnType, unsigned flags = 0) {
    add({ name, sigName, argTypes, returnType, flags, nullptr });
  }
  void add(const std::string &name, std::vector<Type> argTypes,
           Type returnType, unsigned flags = 0) {
    add(name, name, argTypes, returnType, flags);
  }

  Function *getByName(const std::string &name) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
      return nullptr;
    }
    return &it->second;
  }
};

/* Supported types of reductions across all agents */
enum class ReductionKind {
  COUNT_TYPE,
  COUNT_MEMBER,
  SUM_MEMBER,
};

using ReductionInfo = std::pair<ReductionKind, Type>;

}

namespace std {
  template<> struct hash<OpenABL::ReductionInfo> {
    size_t operator()(const OpenABL::ReductionInfo &t) const {
      return std::hash<int>()(static_cast<int>(t.first))
           ^ std::hash<OpenABL::Type>()(t.second);
    }
  };
}

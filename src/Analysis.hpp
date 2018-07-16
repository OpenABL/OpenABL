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

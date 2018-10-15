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

#include <iostream>
#include "Type.hpp"
#include "AST.hpp"

namespace OpenABL {

const std::vector<std::string> Type::vec2Members { "x", "y" };
const std::vector<std::string> Type::vec3Members { "x", "y", "z" };

static const char *getTypeIdStr(Type::TypeId t) {
  switch (t) {
    case Type::INVALID: return "INVALID";
    case Type::VOID: return "void";
    case Type::BOOL: return "bool";
    case Type::INT32: return "int";
    case Type::FLOAT: return "float";
    case Type::STRING: return "string";
    case Type::VEC2: return "float2";
    case Type::VEC3: return "float3";
    default: return nullptr;
  }
}

static void printType(std::ostream &s, Type t) {
  if (t.isArray()) {
    printType(s, t.getBaseType());
    s << "[]";
  } else if (t.isAgent()) {
    auto agent = t.getAgentDecl();
    if (agent) {
      s << agent->name;
    } else {
      s << "agent";
    }
  } else if (t.isAgentType()) {
    s << "agentType";

    AST::AgentDeclaration *decl = t.getAgentDecl();
    if (decl) {
      s << "{" << decl->name << "}";
    }
  } else if (t.isAgentMember()) {
    s << "agentMember";

    AST::AgentDeclaration *decl = t.getAgentDecl();
    AST::AgentMember *member = t.getAgentMember();
    if (decl) {
      s << "{" << decl->name << "." << member->name << "}";
    }
  } else {
    s << getTypeIdStr(t.getTypeId());
  }
}

std::ostream &operator<<(std::ostream &s, const Type &t) {
  printType(s, t);
  return s;
}

}

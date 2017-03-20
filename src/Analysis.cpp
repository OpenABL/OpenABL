#include <iostream>
#include "Analysis.hpp"
#include "AST.hpp"

namespace OpenABL {

uint32_t VarId::max_id;

static const char *getTypeIdStr(Type::TypeId t) {
  switch (t) {
    case Type::INVALID: return "INVALID";
    case Type::VOID: return "void";
    case Type::BOOL: return "bool";
    case Type::INT32: return "int";
    case Type::FLOAT32: return "float";
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
  } else {
    s << getTypeIdStr(t.getTypeId());
  }
}

std::ostream &operator<<(std::ostream &s, const Type &t) {
  printType(s, t);
  return s;
}

}
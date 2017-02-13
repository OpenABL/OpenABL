#include <iostream>
#include "Analysis.hpp"

namespace OpenABL {

uint32_t VarId::max_id;

static const char *getTypeIdStr(Type::TypeId t) {
  switch (t) {
    case Type::INVALID: return "INVALID";
    case Type::VOID: return "void";
    case Type::BOOL: return "bool";
    case Type::INT32: return "int";
    case Type::FLOAT32: return "float";
    case Type::VEC2: return "vec2";
    case Type::VEC3: return "vec3";
    case Type::AGENT: return "???"; // TODO
    case Type::ARRAY: return NULL;
  }
}

std::ostream &operator<<(std::ostream &s, Type t) {
  if (t.isArray()) {
    s << getTypeIdStr(t.getBaseTypeId()) << "[]";
  } else {
    s << getTypeIdStr(t.getTypeId());
  }
  return s;
}

}

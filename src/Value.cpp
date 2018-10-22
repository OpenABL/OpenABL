/* Copyright 2018 OpenABL Contributors
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

#include <cmath>
#include "Analysis.hpp"
#include "AST.hpp"

namespace OpenABL {

std::ostream &operator<<(std::ostream &s, const Value &value) {
  switch (value.getType().getTypeId()) {
    case Type::INVALID:
      s << "INVALID";
      break;
    case Type::BOOL:
      s << (value.getBool() ? "true" : "false");
      break;
    case Type::INT32:
      s << value.getInt();
      break;
    case Type::FLOAT:
      s << value.getFloat();
      break;
    case Type::VEC2:
    {
      auto v = value.getVec2();
      s << "(" << v.x << ", " << v.y << ")";
      break;
    }
    case Type::VEC3:
    {
      auto v = value.getVec3();
      s << "(" << v.x << ", " << v.y << ", " << v.z << ")";
      break;
    }
    case Type::STRING:
      s << "\"" << value.getString() << "\"";
      break;
    default:
      s << "UNKNOWN";
      break;
  }
  return s;
}

Value Value::fromString(const std::string &str) {
  // Boolean
  if (str == "true") {
    return true;
  }

  if (str == "false") {
    return false;
  }

  // Integer
  try {
    size_t pos;
    long lval = std::stoi(str, &pos);
    if (pos == str.length()) {
      return lval;
    }
  } catch (const std::logic_error &e) {
  }

  // Double
  try {
    size_t pos;
    double dval = std::stod(str, &pos);
    if (pos == str.length()) {
      return dval;
    }
  } catch (const std::logic_error &e) {
  }

  // Invalid
  return {};
}

Value Value::calcUnaryOp(AST::UnaryOp op, const Value &val) {
  switch (op) {
    case AST::UnaryOp::PLUS:
      if (val.isInt() || val.isFloat() || val.isVec()) {
        return val;
      }
      return {};
    case AST::UnaryOp::MINUS:
      if (val.isInt()) {
        return -val.getInt();
      } else if (val.isFloat()) {
        return -val.getFloat();
      } else if (val.isVec2()) {
        return { -val.vec2.x, -val.vec2.y };
      } else if (val.isVec3()) {
        return { -val.vec3.x, -val.vec3.y, -val.vec3.z };
      }
      return {};
    case AST::UnaryOp::LOGICAL_NOT:
      if (val.isBool()) {
        return !val.getBool();
      }
      return {};
    case AST::UnaryOp::BITWISE_NOT:
      if (val.isInt()) {
        return ~val.getInt();
      }
      return {};
  }
  assert(0);
}

static int compareNums(const Value &l, const Value &r) {
  if (l.isInt() && r.isInt()) {
    long lv = l.getInt(), rv = r.getInt();
    return lv > rv ? 1 : lv < rv ? -1 : 0;
  } else {
    double lv = l.asFloat(), rv = r.asFloat();
    return lv > rv ? 1 : lv < rv ? -1 : 0;
  }
}

Value Value::calcBinaryOp(AST::BinaryOp op, const Value &l, const Value &r) {
  switch (op) {
    case AST::BinaryOp::ADD:
      if (l.isInt() && r.isInt()) {
        return l.getInt() + r.getInt();
      }
      if (l.isNum() && r.isNum()) {
        return l.asFloat() + r.asFloat();
      }
      if (l.isVec2() && r.isVec2()) {
        return { l.vec2.x + r.vec2.x, l.vec2.y + r.vec2.y };
      }
      if (l.isVec3() && r.isVec3()) {
        return { l.vec3.x + r.vec3.x, l.vec3.y + r.vec3.y, l.vec3.z + r.vec3.z };
      }
      return {};
    case AST::BinaryOp::SUB:
      if (l.isInt() && r.isInt()) {
        return l.getInt() - r.getInt();
      }
      if (l.isNum() && r.isNum()) {
        return l.asFloat() - r.asFloat();
      }
      if (l.isVec2() && r.isVec2()) {
        return { l.vec2.x - r.vec2.x, l.vec2.y - r.vec2.y };
      }
      if (l.isVec3() && r.isVec3()) {
        return { l.vec3.x - r.vec3.x, l.vec3.y - r.vec3.y, l.vec3.z - r.vec3.z };
      }
      return {};
    case AST::BinaryOp::MUL:
      if (l.isInt() && r.isInt()) {
        return l.getInt() * r.getInt();
      }
      if (l.isNum() && r.isNum()) {
        return l.asFloat() * r.asFloat();
      }
      if (l.isVec2() && r.isNum()) {
        double f = r.asFloat();
        return { l.vec2.x * f, l.vec2.y * f };
      }
      if (l.isVec3() && r.isNum()) {
        double f = r.asFloat();
        return { l.vec3.x * f, l.vec3.y * f, l.vec3.z * f };
      }
      if (l.isNum() && r.isVec2()) {
        double f = l.asFloat();
        return { r.vec2.x * f, r.vec2.y * f };
      }
      if (l.isNum() && r.isVec3()) {
        double f = l.asFloat();
        return { r.vec3.x * f, r.vec3.y * f, r.vec3.z * f };
      }
      return {};
    case AST::BinaryOp::DIV:
      if (l.isInt() && r.isInt()) {
        return l.getInt() / r.getInt();
      }
      if (l.isNum() && r.isNum()) {
        return l.asFloat() / r.asFloat();
      }
      if (l.isVec2() && r.isNum()) {
        double f = r.asFloat();
        return { l.vec2.x / f, l.vec2.y / f };
      }
      if (l.isVec3() && r.isNum()) {
        double f = r.asFloat();
        return { l.vec3.x / f, l.vec3.y / f, l.vec3.z / f };
      }
      return {};
    case AST::BinaryOp::MOD:
      if (l.isInt() && r.isInt()) {
        return l.getInt() % r.getInt();
      }
      if (l.isNum() && r.isNum()) {
        return fmod(l.asFloat(), r.asFloat());
      }
      return {};
    case AST::BinaryOp::BITWISE_OR:
      if (l.isInt() && r.isInt()) {
        return l.getInt() | r.getInt();
      }
      return {};
    case AST::BinaryOp::BITWISE_AND:
      if (l.isInt() && r.isInt()) {
        return l.getInt() & r.getInt();
      }
      return {};
    case AST::BinaryOp::BITWISE_XOR:
      if (l.isInt() && r.isInt()) {
        return l.getInt() ^ r.getInt();
      }
      return {};
    case AST::BinaryOp::SHIFT_LEFT:
      if (l.isInt() && r.isInt()) {
        return l.getInt() << r.getInt();
      }
      return {};
    case AST::BinaryOp::SHIFT_RIGHT:
      if (l.isInt() && r.isInt()) {
        return l.getInt() >> r.getInt();
      }
      return {};
    case AST::BinaryOp::EQUALS:
    case AST::BinaryOp::NOT_EQUALS:
      if (l.isVec2() && r.isVec2()) {
        return op == AST::BinaryOp::EQUALS ? l.vec2 == r.vec2 : l.vec2 != r.vec2;
      }
      if (l.isVec3() && r.isVec3()) {
        return op == AST::BinaryOp::EQUALS ? l.vec2 == r.vec2 : l.vec2 != r.vec2;
      }
      /* break missing intentionally */
    case AST::BinaryOp::SMALLER:
    case AST::BinaryOp::SMALLER_EQUALS:
    case AST::BinaryOp::GREATER:
    case AST::BinaryOp::GREATER_EQUALS:
    {
      if (!l.isNum() || !r.isNum()) {
        return {};
      }

      int cmp = compareNums(l, r);
      switch (op) {
        case AST::BinaryOp::EQUALS: return cmp == 0;
        case AST::BinaryOp::NOT_EQUALS: return cmp != 0;
        case AST::BinaryOp::SMALLER: return cmp < 0;
        case AST::BinaryOp::SMALLER_EQUALS: return cmp <= 0;
        case AST::BinaryOp::GREATER: return cmp > 0;
        case AST::BinaryOp::GREATER_EQUALS: return cmp >= 0;
        default: assert(0); return {};
      }
    }
    case AST::BinaryOp::LOGICAL_OR:
    case AST::BinaryOp::LOGICAL_AND:
      if (!l.isBool() || !r.isBool()) {
        return {};
      }
      if (op == AST::BinaryOp::LOGICAL_OR) {
        return l.getBool() || r.getBool();
      } else {
        return l.getBool() && r.getBool();
      }
    case AST::BinaryOp::RANGE:
      return {};
  }
  assert(0);
}

Value Value::calcBuiltinCall(const FunctionSignature &sig, const std::vector<Value> &args) {
  if (args.size() == 1) {
    // Currently all constexpr functions take a single double argument
    if (!args[0].isNum()) {
      return {};
    }

    static std::map<std::string, double (*)(double)> funcs = {
      { "sin", sin },
      { "cos", cos },
      { "tan", tan },
      { "sinh", sinh },
      { "cosh", cosh },
      { "tanh", tanh },
      { "asin", asin },
      { "acos", acos },
      { "atan", atan },
      { "exp", exp },
      { "log", log },
      { "sqrt", sqrt },
      { "cbrt", cbrt },
      { "round", round },
    };

    auto it = funcs.find(sig.name);
    if (it == funcs.end()) {
      return {};
    }

    return it->second(args[0].asFloat());
  }

  if (args.size() == 2) {
    if (sig.name != "pow" || !args[0].isNum() || !args[1].isNum()) {
      return {};
    }

    return pow(args[0].asFloat(), args[1].asFloat());
  }

  return {};
}

Value Value::getSumIdentity(const Type &type) {
  switch (type.getTypeId()) {
    case Type::BOOL:
      // We define that bool reductions sum to int
      return {0l};
    case Type::INT32:
      return {0l};
    case Type::FLOAT:
      return {0.};
    case Type::VEC2:
      return {0., 0.};
    case Type::VEC3:
      return {0., 0., 0.};
    default:
      return {};
  }
}

static inline AST::Expression *withType(AST::Expression *expr, Type t) {
  expr->type = t;
  return expr;
}

static inline AST::Expression *makeFloatLiteral(double value) {
  return withType(new AST::FloatLiteral(value, location{}), Type::FLOAT);
}

AST::Expression *Value::toExpression() const {
  switch (type.getTypeId()) {
    case Type::INVALID:
      return nullptr;
    case Type::BOOL:
      return withType(new AST::BoolLiteral(bval, location{}), type);
    case Type::INT32:
      return withType(new AST::IntLiteral(ival, location{}), type);
    case Type::FLOAT:
      return withType(new AST::FloatLiteral(fval, location{}), type);
    case Type::STRING:
      return withType(new AST::StringLiteral(str, location{}), type);
    case Type::VEC2:
    {
      auto *args = new AST::ExpressionList();
      args->emplace_back(makeFloatLiteral(vec2.x));
      args->emplace_back(makeFloatLiteral(vec2.y));
      auto *call = new AST::CallExpression("float2", args, location{});
      call->kind = AST::CallExpression::Kind::CTOR;
      call->type = type;
      return call;
    }
    case Type::VEC3:
    {
      auto *args = new AST::ExpressionList();
      args->emplace_back(makeFloatLiteral(vec3.x));
      args->emplace_back(makeFloatLiteral(vec3.y));
      args->emplace_back(makeFloatLiteral(vec3.z));
      auto *call = new AST::CallExpression("float3", args, location{});
      call->kind = AST::CallExpression::Kind::CTOR;
      call->type = type;
      return call;
    }
    default:
      assert(0);
      return nullptr;
  }
}

}

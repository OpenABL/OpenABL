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

#pragma once

#include <map>
#include <vector>
#include <cassert>

#include "Type.hpp"

namespace OpenABL {

struct FunctionSignature;

namespace AST {
  struct AgentDeclaration;
  struct AgentMember;
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
  static Value getSumIdentity(const Type &type);

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

}

#include "Analysis.hpp"
#include "AST.hpp"

namespace OpenABL {

uint32_t VarId::max_id;

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

static inline AST::Expression *withType(AST::Expression *expr, Type t) {
  expr->type = t;
  return expr;
}

static inline AST::Expression *makeFloatLiteral(double value) {
  return withType(new AST::FloatLiteral(value, location{}), Type::FLOAT32);
}

AST::Expression *Value::toExpression() const {
  switch (type.getTypeId()) {
    case Type::INVALID:
      return nullptr;
    case Type::BOOL:
      return withType(new AST::BoolLiteral(bval, location{}), type);
    case Type::INT32:
      return withType(new AST::IntLiteral(ival, location{}), type);
    case Type::FLOAT32:
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
      auto *call = new AST::CallExpression("float2", args, location{});
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

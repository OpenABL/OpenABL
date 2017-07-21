#include "Analysis.hpp"
#include "AST.hpp"

namespace OpenABL {

uint32_t VarId::max_id;

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
      args->emplace_back(makeFloatLiteral(vec2.first));
      args->emplace_back(makeFloatLiteral(vec2.second));
      auto *call = new AST::CallExpression("float2", args, location{});
      call->kind = AST::CallExpression::Kind::CTOR;
      call->type = type;
      return call;
    }
    case Type::VEC3:
    {
      auto *args = new AST::ExpressionList();
      args->emplace_back(makeFloatLiteral(std::get<0>(vec3)));
      args->emplace_back(makeFloatLiteral(std::get<1>(vec3)));
      args->emplace_back(makeFloatLiteral(std::get<2>(vec3)));
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

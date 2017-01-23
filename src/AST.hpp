#pragma once

#include <memory>

namespace OpenABL {
namespace AST {

struct Node {
};

struct Expression : public Node {
};

using ExpressionPtr = std::unique_ptr<Expression>;

struct BinaryOpExpression : public Expression {
  enum class Op {
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
  };

  Op op;
  ExpressionPtr left;
  ExpressionPtr right;

  BinaryOpExpression(Op op, Expression *left, Expression *right)
    : op{op}, left{left}, right{right} {}
};

struct Statement : public Node {
};

}
}

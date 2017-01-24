#pragma once

#include <memory>
#include "location.hh"

namespace OpenABL {
namespace AST {

using Location = OpenABL::location;

struct Node {
  Location loc;

  Node(Location loc) : loc{loc} {}
};

struct Expression : public Node {
  Expression(Location loc) : Node{loc} {}
};

using ExpressionPtr = std::unique_ptr<Expression>;

struct VarExpression : public Expression {
  std::string name;

  VarExpression(std::string name, Location loc)
    : Expression{loc}, name{name} {}
};

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
};

struct BinaryOpExpression : public Expression {
  BinaryOp op;
  ExpressionPtr left;
  ExpressionPtr right;

  BinaryOpExpression(BinaryOp op, Expression *left, Expression *right, Location loc)
    : Expression{loc}, op{op}, left{left}, right{right} {}
};

struct AssignOpExpression : public Expression {
  BinaryOp op; // Note: Not all binary ops are allowed here!
  ExpressionPtr left;
  ExpressionPtr right;

  AssignOpExpression(BinaryOp op, Expression *left, Expression *right, Location loc)
    : Expression{loc}, op{op}, left{left}, right{right} {}
};

struct AssignExpression : public Expression {
  ExpressionPtr left;
  ExpressionPtr right;

  AssignExpression(Expression *left, Expression *right, Location loc)
    : Expression{loc}, left{left}, right{right} {}
};

struct Statement : public Node {
  Statement(Location loc) : Node{loc} {}
};

using StatementPtr = std::unique_ptr<Statement>;
using StatementList = std::vector<StatementPtr>;
using StatementListPtr = std::unique_ptr<StatementList>;

struct ExpressionStatement : public Statement {
  ExpressionPtr expr;

  ExpressionStatement(Expression *expr, Location loc)
    : Statement{loc}, expr{expr} {}
};

struct Type : public Node {
  std::string name;

  Type(std::string name, Location loc)
    : Node{loc}, name{name} {}
};

using TypePtr = std::unique_ptr<Type>;

struct Param : public Node {
  TypePtr type;
  std::string name;

  Param(Type *type, std::string name, Location loc)
    : Node{loc}, type{type}, name{name} {}
};

using ParamPtr = std::unique_ptr<Param>;
using ParamList = std::vector<ParamPtr>;
using ParamListPtr = std::unique_ptr<ParamList>;

struct FunctionDeclaration : public Node {
  TypePtr returnType;
  std::string name;
  ParamListPtr params;
  StatementListPtr stmts;

  FunctionDeclaration(Type *returnType, std::string name,
                      ParamList *params, StatementList *stmts, Location loc)
    : Node{loc}, returnType{returnType}, name{name}, params{params}, stmts{stmts} {}
};

}
}

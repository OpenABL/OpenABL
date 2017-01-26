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

/* Top-level declaration */
struct Declaration : public Node {
  Declaration(Location loc) : Node{loc} {}
};

using DeclarationPtr = std::unique_ptr<Declaration>;
using DeclarationList = std::vector<DeclarationPtr>;
using DeclarationListPtr = std::unique_ptr<DeclarationList>;

struct FunctionDeclaration : public Declaration {
  TypePtr returnType;
  std::string name;
  ParamListPtr params;
  StatementListPtr stmts;

  FunctionDeclaration(Type *returnType, std::string name,
                      ParamList *params, StatementList *stmts, Location loc)
    : Declaration{loc}, returnType{returnType}, name{name}, params{params}, stmts{stmts} {}
};

struct AgentMember : public Node {
  bool isPosition;
  TypePtr type;
  std::string name;

  AgentMember(bool isPosition, Type *type, std::string name, Location loc)
    : Node{loc}, isPosition{isPosition}, type{type}, name{name} {}
};

using AgentMemberPtr = std::unique_ptr<AgentMember>;
using AgentMemberList = std::vector<AgentMemberPtr>;
using AgentMemberListPtr = std::unique_ptr<AgentMemberList>;

struct AgentDeclaration : public Declaration {
  std::string name;
  AgentMemberListPtr members;

  AgentDeclaration(std::string name, AgentMemberList *members, Location loc)
    : Declaration{loc}, name{name}, members{members} {}
};

/* AST root node */
struct Script : public Node {
  DeclarationListPtr declarations;

  Script(DeclarationList *declarations, Location loc)
    : Node{loc}, declarations{declarations} {}
};

}
}

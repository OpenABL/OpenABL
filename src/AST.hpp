#pragma once

#include <memory>
#include "location.hh"

namespace OpenABL {
namespace AST {

using Location = OpenABL::location;

struct Type;
using TypePtr = std::unique_ptr<Type>;

struct Node {
  Location loc;

  Node(Location loc) : loc{loc} {}
};

struct Expression : public Node {
  Expression(Location loc) : Node{loc} {}
};

using ExpressionPtr = std::unique_ptr<Expression>;
using ExpressionList = std::vector<ExpressionPtr>;
using ExpressionListPtr = std::unique_ptr<ExpressionList>;

struct Literal : public Expression {
  Literal(Location loc) : Expression{loc} {}
};

using LiteralPtr = std::unique_ptr<Literal>;

struct BoolLiteral : public Literal {
  bool value;

  BoolLiteral(bool value, Location loc)
    : Literal{loc}, value{value} {}
};

struct IntLiteral : public Literal {
  long value;

  IntLiteral(long value, Location loc)
    : Literal{loc}, value{value} {}
};

struct FloatLiteral : public Literal {
  double value;

  FloatLiteral(double value, Location loc)
    : Literal{loc}, value{value} {}
};

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
  RANGE,
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

enum class UnaryOp {
  MINUS,
  PLUS,
  LOGICAL_NOT,
  BITWISE_NOT,
};

struct UnaryOpExpression : public Expression {
  UnaryOp op;
  ExpressionPtr expr;

  UnaryOpExpression(UnaryOp op, Expression *expr, Location loc)
    : Expression{loc}, op{op}, expr{expr} {}
};

struct AssignExpression : public Expression {
  ExpressionPtr left;
  ExpressionPtr right;

  AssignExpression(Expression *left, Expression *right, Location loc)
    : Expression{loc}, left{left}, right{right} {}
};

struct Arg : public Node {
  ExpressionPtr expr;
  ExpressionPtr outExpr;

  Arg(Expression *expr, Expression *outExpr, Location loc)
    : Node{loc}, expr{expr}, outExpr{outExpr} {}
};

using ArgPtr = std::unique_ptr<Arg>;
using ArgList = std::vector<ArgPtr>;
using ArgListPtr = std::unique_ptr<ArgList>;

struct CallExpression : public Expression {
  std::string name;
  ArgListPtr args;

  CallExpression(std::string name, ArgList *args, Location loc)
    : Expression{loc}, name{name}, args{args} {}
};

struct MemberAccessExpression : public Expression {
  ExpressionPtr expr;
  std::string member;

  MemberAccessExpression(Expression *expr, std::string member, Location loc)
    : Expression{loc}, expr{expr}, member{member} {}
};

struct TernaryExpression : public Expression {
  ExpressionPtr condExpr;
  ExpressionPtr ifExpr;
  ExpressionPtr elseExpr;

  TernaryExpression(Expression *condExpr, Expression *ifExpr, Expression *elseExpr, Location loc)
    : Expression{loc}, condExpr{condExpr}, ifExpr{ifExpr}, elseExpr{elseExpr} {}
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

struct BlockStatement : public Statement {
  StatementListPtr stmts;

  BlockStatement(StatementList *stmts, Location loc)
    : Statement{loc}, stmts{stmts} {}
};

struct VarDeclarationStatement : public Statement {
  TypePtr type;
  std::string name;
  ExpressionPtr initializer;

  VarDeclarationStatement(Type *type, std::string name, Expression *initializer, Location loc)
    : Statement{loc}, type{type}, name{name}, initializer{initializer} {}
};

struct IfStatement : public Statement {
  ExpressionPtr condExpr;
  StatementPtr ifStmt;
  StatementPtr elseStmt;

  IfStatement(Expression *condExpr, Statement *ifStmt, Statement *elseStmt, Location loc)
    : Statement{loc}, condExpr{condExpr}, ifStmt{ifStmt}, elseStmt{elseStmt} {}
};

struct ForStatement : public Statement {
  bool isParallel;
  TypePtr type;
  std::string var;
  std::string outVar; // TODO std::optional
  ExpressionPtr expr;
  StatementPtr stmt;

  ForStatement(bool isParallel, Type *type, std::string var, std::string ourVar,
               Expression *expr, Statement *stmt, Location loc)
    : Statement{loc}, isParallel{isParallel}, type{type},
      var{var}, outVar{outVar}, expr{expr}, stmt{stmt} {}
};

struct Type : public Node {
  std::string name;

  Type(std::string name, Location loc)
    : Node{loc}, name{name} {}
};

struct Param : public Node {
  TypePtr type;
  std::string name;
  // TODO std::optional would be nice here. Import compat header?
  std::string outName;

  Param(Type *type, std::string name, std::string outName, Location loc)
    : Node{loc}, type{type}, name{name}, outName{outName} {}
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
  bool isInteract;
  TypePtr returnType;
  std::string name;
  ParamListPtr params;
  StatementListPtr stmts;

  FunctionDeclaration(bool isInteract, Type *returnType, std::string name,
                      ParamList *params, StatementList *stmts, Location loc)
    : Declaration{loc}, isInteract{isInteract}, returnType{returnType},
      name{name}, params{params}, stmts{stmts} {}
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

struct ConstDeclaration : public Declaration {
  TypePtr type;
  std::string name;
  LiteralPtr value;

  ConstDeclaration(Type *type, std::string name, Literal *value, Location loc)
    : Declaration{loc}, type{type}, name{name}, value{value} {}
};

/* AST root node */
struct Script : public Node {
  DeclarationListPtr declarations;

  Script(DeclarationList *declarations, Location loc)
    : Node{loc}, declarations{declarations} {}
};

}
}

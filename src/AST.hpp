#pragma once

#include <memory>
#include <set>
#include "Analysis.hpp"
#include "location.hh"

namespace OpenABL {

struct Printer;

namespace AST {

using Location = OpenABL::location;

struct Type;
using TypePtr = std::unique_ptr<Type>;

struct Visitor;

struct Node {
  Location loc;

  Node(Location loc) : loc{loc} {}

  virtual void accept(Visitor &) = 0;
  virtual void print(Printer &) const = 0;
  virtual ~Node() {}
};

struct Var : public Node {
  std::string name;
  VarId id;

  Var(std::string name, Location loc)
    : Node{loc}, name{name} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

using VarPtr = std::unique_ptr<Var>;

struct Expression : public Node {
  OpenABL::Type type;

  Expression(Location loc)
    : Node{loc} {}
};

using ExpressionPtr = std::unique_ptr<Expression>;
using ExpressionList = std::vector<ExpressionPtr>;
using ExpressionListPtr = std::unique_ptr<ExpressionList>;

struct Literal : public Expression {
  Literal(Location loc) : Expression{loc} {}

  void accept(Visitor &);
  void print(Printer &) const;
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

struct StringLiteral : public Literal {
  std::string value;

  StringLiteral(std::string value, Location loc)
    : Literal{loc}, value{value} {}
};

struct VarExpression : public Expression {
  VarPtr var;

  VarExpression(Var *var, Location loc)
    : Expression{loc}, var{var} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

enum class UnaryOp {
  MINUS,
  PLUS,
  LOGICAL_NOT,
  BITWISE_NOT,
};

static inline const char *getUnaryOpSigil(UnaryOp op) {
  switch (op) {
    case UnaryOp::MINUS:       return "-";
    case UnaryOp::PLUS:        return "+";
    case UnaryOp::LOGICAL_NOT: return "!";
    case UnaryOp::BITWISE_NOT: return "~";
  }
  assert(0);
}

struct UnaryOpExpression : public Expression {
  UnaryOp op;
  ExpressionPtr expr;

  UnaryOpExpression(UnaryOp op, Expression *expr, Location loc)
    : Expression{loc}, op{op}, expr{expr} {}

  void accept(Visitor &);
  void print(Printer &) const;
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

static inline const char *getBinaryOpSigil(BinaryOp op) {
  switch (op) {
    case BinaryOp::ADD:            return "+";
    case BinaryOp::SUB:            return "-";
    case BinaryOp::MUL:            return "*";
    case BinaryOp::DIV:            return "/";
    case BinaryOp::MOD:            return "%";
    case BinaryOp::BITWISE_AND:    return "&";
    case BinaryOp::BITWISE_XOR:    return "^";
    case BinaryOp::BITWISE_OR:     return "|";
    case BinaryOp::SHIFT_LEFT:     return "<<";
    case BinaryOp::SHIFT_RIGHT:    return ">>";
    case BinaryOp::EQUALS:         return "==";
    case BinaryOp::NOT_EQUALS:     return "!=";
    case BinaryOp::SMALLER:        return "<";
    case BinaryOp::SMALLER_EQUALS: return "<=";
    case BinaryOp::GREATER:        return ">";
    case BinaryOp::GREATER_EQUALS: return ">=";
    case BinaryOp::LOGICAL_AND:    return "&&";
    case BinaryOp::LOGICAL_OR:     return "||";
    case BinaryOp::RANGE:          return "..";
  }
  assert(0);
}

struct BinaryOpExpression : public Expression {
  BinaryOp op;
  ExpressionPtr left;
  ExpressionPtr right;

  BinaryOpExpression(BinaryOp op, Expression *left, Expression *right, Location loc)
    : Expression{loc}, op{op}, left{left}, right{right} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct Arg : public Node {
  ExpressionPtr expr;
  ExpressionPtr outExpr;

  Arg(Expression *expr, Expression *outExpr, Location loc)
    : Node{loc}, expr{expr}, outExpr{outExpr} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

using ArgPtr = std::unique_ptr<Arg>;
using ArgList = std::vector<ArgPtr>;
using ArgListPtr = std::unique_ptr<ArgList>;

struct CallExpression : public Expression {
  enum class Kind {
    USER,    // Call to user function
    BUILTIN, // Call to builtin function
    CTOR,    // Call to type constructor / cast
  };

  std::string name;
  ArgListPtr args;

  // Populated during analysis
  Kind kind;
  FunctionSignature calledSig;

  CallExpression(std::string name, ArgList *args, Location loc)
    : Expression{loc}, name{name}, args{args}, kind{Kind::USER} {}

  void accept(Visitor &);
  void print(Printer &) const;

  bool isBuiltin() const { return kind == Kind::BUILTIN; }
  bool isCtor() const { return kind == Kind::CTOR; }

  const AST::Arg &getArg(size_t n) const {
    return *(*args)[n];
  }
};

struct MemberAccessExpression : public Expression {
  ExpressionPtr expr;
  std::string member;

  MemberAccessExpression(Expression *expr, std::string member, Location loc)
    : Expression{loc}, expr{expr}, member{member} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct TernaryExpression : public Expression {
  ExpressionPtr condExpr;
  ExpressionPtr ifExpr;
  ExpressionPtr elseExpr;

  TernaryExpression(Expression *condExpr, Expression *ifExpr, Expression *elseExpr, Location loc)
    : Expression{loc}, condExpr{condExpr}, ifExpr{ifExpr}, elseExpr{elseExpr} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct MemberInitEntry : public Node {
  std::string name;
  ExpressionPtr expr;

  MemberInitEntry(std::string name, Expression *expr, Location loc)
    : Node{loc}, name{name}, expr{expr} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

using MemberInitEntryPtr = std::unique_ptr<MemberInitEntry>;
using MemberInitList = std::vector<MemberInitEntryPtr>;
using MemberInitListPtr = std::unique_ptr<MemberInitList>;

struct AgentCreationExpression : public Expression {
  std::string name;
  MemberInitListPtr members;

  // Computed during analysis
  std::map<std::string, Expression *> memberMap;

  AgentCreationExpression(std::string name, MemberInitList *members, Location loc)
    : Expression{loc}, name{name}, members{members} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct NewArrayExpression : public Expression {
  TypePtr elemType;
  ExpressionPtr sizeExpr;

  NewArrayExpression(Type *elemType, Expression *sizeExpr, Location loc)
    : Expression{loc}, elemType{elemType}, sizeExpr{sizeExpr} {}

  void accept(Visitor &);
  void print(Printer &) const;
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

  void accept(Visitor &);
  void print(Printer &) const;
};

struct AssignStatement : public Statement {
  ExpressionPtr left;
  ExpressionPtr right;

  AssignStatement(Expression *left, Expression *right, Location loc)
    : Statement{loc}, left{left}, right{right} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct AssignOpStatement : public Statement {
  BinaryOp op; // Note: Not all binary ops are allowed here!
  ExpressionPtr left;
  ExpressionPtr right;

  AssignOpStatement(BinaryOp op, Expression *left, Expression *right, Location loc)
    : Statement{loc}, op{op}, left{left}, right{right} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct BlockStatement : public Statement {
  StatementListPtr stmts;

  BlockStatement(StatementList *stmts, Location loc)
    : Statement{loc}, stmts{stmts} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct VarDeclarationStatement : public Statement {
  TypePtr type;
  VarPtr var;
  ExpressionPtr initializer;

  VarDeclarationStatement(Type *type, Var *var, Expression *initializer, Location loc)
    : Statement{loc}, type{type}, var{var}, initializer{initializer} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct IfStatement : public Statement {
  ExpressionPtr condExpr;
  StatementPtr ifStmt;
  StatementPtr elseStmt;

  IfStatement(Expression *condExpr, Statement *ifStmt, Statement *elseStmt, Location loc)
    : Statement{loc}, condExpr{condExpr}, ifStmt{ifStmt}, elseStmt{elseStmt} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct WhileStatement : public Statement {
  ExpressionPtr expr;
  StatementPtr stmt;

  WhileStatement(Expression *expr, Statement *stmt, Location loc)
    : Statement{loc}, expr{expr}, stmt{stmt} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct ForStatement : public Statement {
  enum class Kind {
    NORMAL, // For loop over an array          for (Agent agent : agents)
    RANGE,  // For loop over an integer range  for (int t : 0 .. t_max)
    NEAR,   // For loop over nearby agents     for (Agent nx : near(agent, radius))
  };

  TypePtr type;
  VarPtr var;
  ExpressionPtr expr;
  StatementPtr stmt;

  // Populated during analysis
  Kind kind;

  ForStatement(Type *type, Var *var, Expression *expr, Statement *stmt, Location loc)
    : Statement{loc}, type{type}, var{var}, expr{expr}, stmt{stmt}, kind{Kind::NORMAL} {}

  void accept(Visitor &);
  void print(Printer &) const;

  bool isRange() const { return kind == Kind::RANGE; }
  std::pair<Expression &, Expression &> getRange() const {
    assert(isRange());
    BinaryOpExpression *op = dynamic_cast<BinaryOpExpression *>(&*expr);
    return { *op->left, *op->right };
  }

  bool isNear() const { return kind == Kind::NEAR; }
  CallExpression &getNearCall() const {
    assert(isNear());
    return *dynamic_cast<CallExpression *>(&*expr);
  }
  Expression &getNearAgent() const { return *(*getNearCall().args)[0]->expr; }
  Expression &getNearRadius() const { return *(*getNearCall().args)[1]->expr; }
};

struct FunctionDeclaration;

using IdentList = std::vector<std::string>;
using IdentListPtr = std::unique_ptr<IdentList>;

struct SimulateStatement : public Statement {
  ExpressionPtr timestepsExpr;
  IdentListPtr stepFuncs;

  std::vector<FunctionDeclaration *> stepFuncDecls;

  SimulateStatement(Expression *timestepsExpr, IdentList *stepFuncs, Location loc)
    : Statement{loc}, timestepsExpr{timestepsExpr}, stepFuncs{stepFuncs} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct ReturnStatement : public Statement {
  ExpressionPtr expr;

  ReturnStatement(Expression *expr, Location loc)
    : Statement{loc}, expr{expr} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct Type : public Node {
  OpenABL::Type resolved;

  Type(Location loc) : Node{loc} {}
};

struct SimpleType : public Type {
  std::string name;

  SimpleType(std::string name, Location loc)
    : Type{loc}, name{name} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct ArrayType : public Type {
  TypePtr type;

  ArrayType(Type *type, Location loc)
    : Type{loc}, type{type} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct Param : public Node {
  TypePtr type;
  VarPtr var;
  VarPtr outVar;

  Param(Type *type, Var *var, Var *outVar, Location loc)
    : Node{loc}, type{type}, var{var}, outVar{outVar} {}

  void accept(Visitor &);
  void print(Printer &) const;
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

  // The following members are for step functions only
  bool isStep = false;
  // The type of the agent that is interacted with (type of agent
  // in for-near loop)
  AgentDeclaration *accessedAgent = nullptr;
  // Which members of the agent that we interact with are accessed
  std::set<std::string> accessedMembers;

  FunctionDeclaration(Type *returnType, std::string name,
                      ParamList *params, StatementList *stmts, Location loc)
    : Declaration{loc}, returnType{returnType},
      name{name}, params{params}, stmts{stmts} {}

  bool isMain() const { return name == "main"; }

  AST::Param &stepParam() const {
    assert(isStep);
    return *(*this->params)[0];
  }

  AST::AgentDeclaration &stepAgent() const {
    assert(isStep);
    return *stepParam().type->resolved.getAgentDecl();
  }

  void accept(Visitor &);
  void print(Printer &) const;
};

struct AgentMember : public Node {
  bool isPosition;
  TypePtr type;
  std::string name;

  AgentMember(bool isPosition, Type *type, std::string name, Location loc)
    : Node{loc}, isPosition{isPosition}, type{type}, name{name} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

using AgentMemberPtr = std::unique_ptr<AgentMember>;
using AgentMemberList = std::vector<AgentMemberPtr>;
using AgentMemberListPtr = std::unique_ptr<AgentMemberList>;

struct AgentDeclaration : public Declaration {
  std::string name;
  AgentMemberListPtr members;

  AgentDeclaration(std::string name, AgentMemberList *members, Location loc)
    : Declaration{loc}, name{name}, members{members} {}

  void accept(Visitor &);
  void print(Printer &) const;

  AgentMember *getPositionMember() const {
    for (AgentMemberPtr &member : *members) {
      if (member->isPosition) {
        return &*member;
      }
    }
    return nullptr;
  }
};

struct ConstDeclaration : public Declaration {
  TypePtr type;
  VarPtr var;
  ExpressionPtr expr;

  ConstDeclaration(Type *type, Var *var, Expression *expr, Location loc)
    : Declaration{loc}, type{type}, var{var}, expr{expr} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct EnvironmentDeclaration : public Declaration {
  MemberInitListPtr members;

  // Populated during analysis
  const Expression *sizeExpr = nullptr;

  EnvironmentDeclaration(MemberInitList *members, Location loc)
    : Declaration{loc}, members{members} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

/* AST root node */
struct Script : public Node {
  DeclarationListPtr decls;

  // Populated during analysis
  Scope scope;
  std::vector<AgentDeclaration *> agents;
  std::vector<ConstDeclaration *> consts;
  std::vector<FunctionDeclaration *> funcs;
  SimulateStatement *simStmt = nullptr;
  FunctionDeclaration *mainFunc = nullptr;

  const EnvironmentDeclaration *envDecl = nullptr;

  Script(DeclarationList *decls, Location loc)
    : Node{loc}, decls{decls} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

}
}

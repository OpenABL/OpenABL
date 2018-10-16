/* Copyright 2017 OpenABL Contributors
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

#include <memory>
#include <set>
#include <unordered_set>
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

struct FunctionDeclaration;

struct CallExpression : public Expression {
  enum class Kind {
    USER,    // Call to user function
    BUILTIN, // Call to builtin function
    CTOR,    // Call to type constructor / cast
  };

  std::string name;
  ExpressionListPtr args;

  // Populated during analysis
  Kind kind;
  FunctionSignature calledSig;
  // Only if user function is called
  const FunctionDeclaration *calledFunc = nullptr;

  CallExpression(std::string name, ExpressionList *args, Location loc)
    : Expression{loc}, name{name}, args{args}, kind{Kind::USER} {}

  void accept(Visitor &);
  void print(Printer &) const;

  bool isBuiltin() const { return kind == Kind::BUILTIN; }
  bool isCtor() const { return kind == Kind::CTOR; }

  size_t getNumArgs() const {
    return (*args).size();
  }

  const AST::Expression &getArg(size_t n) const {
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

struct EnvironmentAccessExpression : public Expression {
  std::string member;

  EnvironmentAccessExpression(std::string member, Location loc)
    : Expression{loc}, member{member} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct ArrayAccessExpression : public Expression {
  ExpressionPtr arrayExpr;
  ExpressionPtr offsetExpr;

  ArrayAccessExpression(Expression *arrayExpr, Expression *offsetExpr, Location loc)
    : Expression{loc}, arrayExpr{arrayExpr}, offsetExpr{offsetExpr} {}

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

  const AST::Expression &getExprFor(const std::string &memberName) const {
    auto it = memberMap.find(memberName);
    assert(it != memberMap.end());
    return *it->second;
  }
};

struct ArrayInitExpression : public Expression {
  ExpressionListPtr exprs;

  ArrayInitExpression(ExpressionList *exprs, Location loc)
    : Expression{loc}, exprs{exprs} {}

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
  const Expression &getNearAgent() const { return getNearCall().getArg(0); }
  const Expression &getNearRadius() const { return getNearCall().getArg(1); }
};

using IdentList = std::vector<std::string>;
using IdentListPtr = std::unique_ptr<IdentList>;

struct SimulateStatement : public Statement {
  ExpressionPtr timestepsExpr;
  IdentListPtr stepFuncs;

  // Populated during analysis
  std::vector<FunctionDeclaration *> stepFuncDecls;
  FunctionDeclaration *seqStepDecl = nullptr;

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

struct BreakStatement : public Statement {
  BreakStatement(Location loc)
    : Statement{loc} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct ContinueStatement : public Statement {
  ContinueStatement(Location loc)
    : Statement{loc} {}

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
  enum Kind {
    NORMAL,
    STEP,
    SEQ_STEP,
  };

  TypePtr returnType;
  std::string name;
  ParamListPtr params;
  StatementListPtr stmts;
  Kind kind;

  FunctionSignature sig;
  // The following members are for step functions only
  // The type of the agent that is interacted with (type of agent
  // in for-near loop)
  const AgentDeclaration *accessedAgent = nullptr;
  // Which members of the agent that we interact with are accessed
  std::set<std::string> accessedMembers;
  // Whether runtime removal is used in this step function
  bool usesRuntimeRemoval = false;
  // The agent type added at runtime, if any
  const AgentDeclaration *runtimeAddedAgent = nullptr;
  // FlameGPU needs to know whether an RNG is used
  bool usesRng = false;

  FunctionDeclaration(Type *returnType, std::string name,
                      ParamList *params, StatementList *stmts, Kind kind, Location loc)
    : Declaration{loc}, returnType{returnType},
      name{name}, params{params}, stmts{stmts}, kind{kind} {}

  bool isMain() const { return name == "main"; }

  bool isParallelStep() const {
    return kind == STEP;
  }
  bool isSequentialStep() const {
    return kind == SEQ_STEP;
  }
  bool isAnyStep() const {
    return isParallelStep() || isSequentialStep();
  }

  AST::Param &stepParam() const {
    assert(isParallelStep());
    return *(*this->params)[0];
  }

  AST::AgentDeclaration &stepAgent() const {
    assert(isParallelStep());
    return *stepParam().type->resolved.getAgentDecl();
  }

  // These two functions are for use with main(). They retrieve
  // simulation setup and teardown code.
  std::vector<const Statement *> getStmtsBeforeSimulate() const {
    std::vector<const Statement *> result;
    for (const StatementPtr &stmt : *stmts) {
      if (dynamic_cast<const SimulateStatement *>(&*stmt)) {
        break;
      }
      result.push_back(&*stmt);
    }
    return result;
  }
  std::vector<const Statement *> getStmtsAfterSimulate() const {
    std::vector<const Statement *> result;
    bool collect = false;
    for (const StatementPtr &stmt : *stmts) {
      if (dynamic_cast<const SimulateStatement *>(&*stmt)) {
        collect = true;
      } else if (collect) {
        result.push_back(&*stmt);
      }
    }
    return result;
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

  bool usesRuntimeRemoval = false;

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
  bool isArray;
  bool isParam; // Can be specified on CLI

  ConstDeclaration(Type *type, Var *var, Expression *expr, bool isArray, bool isParam, Location loc)
    : Declaration{loc}, type{type}, var{var}, expr{expr}, isArray{isArray}, isParam{isParam} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

struct EnvironmentDeclaration : public Declaration {
  MemberInitListPtr members;

  // Populated during analysis
  Value envMin;
  Value envMax;
  Value envSize;
  Value envGranularity;
  int envDimension = -1;

  EnvironmentDeclaration(MemberInitList *members, Location loc)
    : Declaration{loc}, members{members} {}

  bool hasEnvDimension() const {
    return envDimension != -1;
  }
  unsigned getEnvDimension() const {
    return envDimension;
  }

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
  std::unordered_set<ReductionInfo> reductions;
  std::set<std::string> params;
  SimulateStatement *simStmt = nullptr;
  FunctionDeclaration *mainFunc = nullptr;
  EnvironmentDeclaration *envDecl = nullptr;
  bool usesRuntimeRemoval = false;
  bool usesRuntimeAddition = false;
  bool usesLogging = false;
  bool usesTiming = false;
  bool usesRuntimeAdditionAtDifferentPos = false;

  Script(DeclarationList *decls, Location loc)
    : Node{loc}, decls{decls} {}

  void accept(Visitor &);
  void print(Printer &) const;
};

}
}

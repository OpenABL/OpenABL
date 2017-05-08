#pragma once

#include <sstream>
#include <type_traits>
#include <cassert>
#include "AST.hpp"

namespace OpenABL {

struct Printer {
  Printer() : str{}, indentLevel{0}, nextAnonLabel{0} {}

  std::string extractStr() {
    return str.str();
  }

  virtual void print(AST::Var &) = 0;
  virtual void print(AST::Literal &) = 0;
  virtual void print(AST::VarExpression &) = 0;
  virtual void print(AST::UnaryOpExpression &) = 0;
  virtual void print(AST::BinaryOpExpression &) = 0;
  virtual void print(AST::Arg &) = 0;
  virtual void print(AST::CallExpression &) = 0;
  virtual void print(AST::MemberAccessExpression &) = 0;
  virtual void print(AST::TernaryExpression &) = 0;
  virtual void print(AST::MemberInitEntry &) = 0;
  virtual void print(AST::AgentCreationExpression &) = 0;
  virtual void print(AST::NewArrayExpression &) = 0;
  virtual void print(AST::ExpressionStatement &) = 0;
  virtual void print(AST::AssignStatement &) = 0;
  virtual void print(AST::AssignOpStatement &) = 0;
  virtual void print(AST::BlockStatement &) = 0;
  virtual void print(AST::VarDeclarationStatement &) = 0;
  virtual void print(AST::IfStatement &) = 0;
  virtual void print(AST::ForStatement &) = 0;
  virtual void print(AST::SimulateStatement &) = 0;
  virtual void print(AST::ReturnStatement &) = 0;
  virtual void print(AST::SimpleType &) = 0;
  virtual void print(AST::ArrayType &) = 0;
  virtual void print(AST::Param &) = 0;
  virtual void print(AST::FunctionDeclaration &) = 0;
  virtual void print(AST::AgentMember &) = 0;
  virtual void print(AST::AgentDeclaration &) = 0;
  virtual void print(AST::ConstDeclaration &) = 0;
  virtual void print(AST::Script &) = 0;

  static Printer &indent(Printer &p) {
    p.indentLevel++;
    return p;
  }
  static Printer &outdent(Printer &p) {
    assert(p.indentLevel > 0);
    p.indentLevel--;
    return p;
  }
  static Printer &nl(Printer &p) {
    p << "\n" << std::string(4 * p.indentLevel, ' ');
    return p;
  }

  Printer &operator <<(AST::Node &node) {
    node.print(*this); return *this;
  }

  Printer &operator <<(Printer &(*fn)(Printer &)) {
    fn(*this); return *this;
  }

  template<typename T>
  Printer &operator <<(std::vector<std::unique_ptr<T>> &nodes) {
    for (const auto &node : nodes) {
      *this << nl << *node;
    }
    return *this;
  }

  template<typename T>
  typename std::enable_if<!std::is_convertible<T, AST::Node &>::value, Printer &>::type
  operator <<(T &&a) {
    str << a; return *this;
  }

  std::string makeAnonLabel() {
    return "_var" + std::to_string(nextAnonLabel++);
  }

private:
  std::stringstream str;
  uint32_t indentLevel;
  uint32_t nextAnonLabel;
};

}

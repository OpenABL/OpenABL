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

  virtual void print(const AST::Var &) = 0;
  virtual void print(const AST::Literal &) = 0;
  virtual void print(const AST::VarExpression &) = 0;
  virtual void print(const AST::UnaryOpExpression &) = 0;
  virtual void print(const AST::BinaryOpExpression &) = 0;
  virtual void print(const AST::CallExpression &) = 0;
  virtual void print(const AST::MemberAccessExpression &) = 0;
  virtual void print(const AST::ArrayAccessExpression &) = 0;
  virtual void print(const AST::TernaryExpression &) = 0;
  virtual void print(const AST::MemberInitEntry &) = 0;
  virtual void print(const AST::AgentCreationExpression &) = 0;
  virtual void print(const AST::ArrayInitExpression &) = 0;
  virtual void print(const AST::NewArrayExpression &) = 0;
  virtual void print(const AST::ExpressionStatement &) = 0;
  virtual void print(const AST::AssignStatement &) = 0;
  virtual void print(const AST::AssignOpStatement &) = 0;
  virtual void print(const AST::BlockStatement &) = 0;
  virtual void print(const AST::VarDeclarationStatement &) = 0;
  virtual void print(const AST::IfStatement &) = 0;
  virtual void print(const AST::WhileStatement &) = 0;
  virtual void print(const AST::ForStatement &) = 0;
  virtual void print(const AST::SimulateStatement &) = 0;
  virtual void print(const AST::ReturnStatement &) = 0;
  virtual void print(const AST::BreakStatement &) = 0;
  virtual void print(const AST::ContinueStatement &) = 0;
  virtual void print(const AST::SimpleType &) = 0;
  virtual void print(const AST::Param &) = 0;
  virtual void print(const AST::FunctionDeclaration &) = 0;
  virtual void print(const AST::AgentMember &) = 0;
  virtual void print(const AST::AgentDeclaration &) = 0;
  virtual void print(const AST::ConstDeclaration &) = 0;
  virtual void print(const AST::EnvironmentDeclaration &) = 0;
  virtual void print(const AST::Script &) = 0;

  template<typename T, typename Fn>
  void printCommaSeparated(const std::vector<T> &vec, Fn fn) {
    bool first = true;
    for (const T &elem : vec) {
      if (!first) {
        *this << ", ";
      }
      first = false;

      fn(elem);
    }
  }

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

  Printer &operator <<(const AST::Node &node) {
    node.print(*this); return *this;
  }

  Printer &operator <<(Printer &(*fn)(Printer &)) {
    fn(*this); return *this;
  }

  template<typename T>
  Printer &operator <<(const std::vector<std::unique_ptr<T>> &nodes) {
    for (const auto &node : nodes) {
      *this << nl << *node;
    }
    return *this;
  }

  template<typename T>
  Printer &operator <<(const std::vector<T *> &nodes) {
    for (const auto *node : nodes) {
      *this << nl << *node;
    }
    return *this;
  }

  /* We do not use universal forwarding here, because it shadows other overloads way too easily.
   * Instead we explicitly specify all supported operations, as necessary. */

  Printer &operator <<(char s) { str << s; return *this; }
  Printer &operator <<(int32_t i) { str << i; return *this; }
  Printer &operator <<(uint32_t i) { str << i; return *this; }
  Printer &operator <<(int64_t i) { str << i; return *this; }
  Printer &operator <<(uint64_t i) { str << i; return *this; }
  Printer &operator <<(double f) { str << f; return *this; }
  Printer &operator <<(const char *s) { str << s; return *this; }
  Printer &operator <<(const std::string &s) { str << s; return *this; }

  Printer &operator <<(const Value &v) { str << v; return *this; }
  Printer &operator <<(Type t) { str << t; return *this; }

  std::string makeAnonLabel() {
    return "_var" + std::to_string(nextAnonLabel++);
  }

private:
  std::stringstream str;
  uint32_t indentLevel;
  uint32_t nextAnonLabel;
};

}

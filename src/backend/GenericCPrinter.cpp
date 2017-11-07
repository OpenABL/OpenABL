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

#include <string>
#include "GenericCPrinter.hpp"

namespace OpenABL {

bool GenericCPrinter::isSpecialBinaryOp(
    AST::BinaryOp op, const AST::Expression &left, const AST::Expression &right) {
  Type l = left.type, r = right.type;
  if (op == AST::BinaryOp::MOD && !(l.isInt() && r.isInt())) {
    return true;
  }
  return l.isVec() || r.isVec();
}

void GenericCPrinter::printSpecialBinaryOp(
    AST::BinaryOp op, const AST::Expression &left, const AST::Expression &right) {
  Type l = left.type;
  Type r = right.type;
  if (l.isVec() || r.isVec()) {
    Type v = l.isVec() ? l : r;
    *this << "float" << v.getVecLen() << "_";
    switch (op) {
      case AST::BinaryOp::ADD: *this << "add"; break;
      case AST::BinaryOp::SUB: *this << "sub"; break;
      case AST::BinaryOp::DIV: *this << "div_scalar"; break;
      case AST::BinaryOp::MUL: *this << "mul_scalar"; break;
      case AST::BinaryOp::EQUALS: *this << "equals"; break;
      case AST::BinaryOp::NOT_EQUALS: *this << "not_equals"; break;
      default:
        assert(0);
    }
    *this << "(" << left << ", " << right << ")";
    return;
  }

  if (op == AST::BinaryOp::MOD && !(l.isInt() && r.isInt())) {
    *this << "fmod(" << left << ", " << right << ")";
    return;
  }

  assert(0);
}

void GenericCPrinter::print(const AST::UnaryOpExpression &expr) {
  Type t = expr.expr->type;
  if (t.isVec()) {
    if (expr.op == AST::UnaryOp::PLUS) {
      // Nothing to do
      *this << *expr.expr;
    } else if (expr.op == AST::UnaryOp::MINUS) {
      // Compile to multiplication by -1.0
      AST::FloatLiteral negOne(-1.0, AST::Location());
      printSpecialBinaryOp(AST::BinaryOp::MUL, *expr.expr, negOne);
    } else {
      assert(0);
    }
    return;
  }

  GenericPrinter::print(expr);
}

void GenericCPrinter::print(const AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var
        << (decl.isArray ? "[]" : "")
        << " = ";
  if (!decl.type->resolved.isVec()) {
    *this << *decl.expr << ";";
  } else if (const auto *call = dynamic_cast<const AST::CallExpression *>(&*decl.expr)) {
    // This would usually generate a floatN_create() call, which is not legal in a constant
    // initializer. Generate an explicit initializer expression instead.
    *this << "{";
    printCommaSeparated(*call->args, [&](const AST::ExpressionPtr &arg) {
      *this << *arg;
    });
    *this << "};";
  } else {
    assert(0);
  }
}

}

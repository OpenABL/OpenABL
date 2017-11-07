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

#include "AST.hpp"
#include "GenericPrinter.hpp"

namespace OpenABL {

struct GenericCPrinter : public GenericPrinter {
  using GenericPrinter::print;

  GenericCPrinter(AST::Script &script)
    : GenericPrinter(script, false) {}

  virtual void print(const AST::UnaryOpExpression &);
  virtual void print(const AST::ConstDeclaration &);

  virtual void printSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
  virtual bool isSpecialBinaryOp(
      const AST::BinaryOp, const AST::Expression &, const AST::Expression &);
};

}

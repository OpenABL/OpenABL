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
#include "CPrinter.hpp"

namespace OpenABL {

void CPrinter::printType(Type type) {
  if (type.isArray()) {
    // Print only the base type
    *this << type.getBaseType();
  } else if (type.isAgent()) {
    *this << type.getAgentDecl()->name << '*';
  } else if (type.isFloat()) {
    *this << (useFloat ? "float" : "double");
  } else {
    *this << type;
  }
}

static CPrinter &operator<<(CPrinter &s, Type type) {
  s.printType(type);
  return s;
}

static void printStorageType(CPrinter &s, Type type) {
  if (type.isAgent()) {
    // Don't use a pointer
    s << type.getAgentDecl()->name;
  } else {
    s.printType(type);
  }
}

static bool typeRequiresStorage(Type type) {
  return type.isArray() || type.isAgent();
}

static void printTypeCtor(CPrinter &p, const AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    size_t numArgs = expr.args->size();
    p << "float" << t.getVecLen() << "_"
      << (numArgs == 1 ? "fill" : "create");
    p << "(";
    p.printArgs(expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *(*expr.args)[0];
  }
}
void CPrinter::print(const AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else {
    const FunctionSignature &sig = expr.calledSig;
    if (sig.name == "add") {
      AST::AgentDeclaration *agent = sig.paramTypes[0].getAgentDecl();
      *this << "*DYN_ARRAY_PLACE(&agents.agents_" << agent->name
            << ", " << agent->name << ") = " << *(*expr.args)[0];
      return;
    } else if (sig.name == "save") {
      *this << "save(&agents, agents_info, " << *(*expr.args)[0] << ", SAVE_JSON)";
      return;
    }

    *this << sig.name << "(";
    printArgs(expr);
    *this << ")";
  }
}
void CPrinter::print(const AST::MemberInitEntry &entry) {
  *this << "." << entry.name << " = " << *entry.expr << ",";
}
void CPrinter::print(const AST::AgentCreationExpression &expr) {
  *this << "(" << expr.name << ") {" << indent
        << *expr.members << outdent << nl << "}";
}
void CPrinter::print(const AST::NewArrayExpression &expr) {
  *this << "DYN_ARRAY_CREATE_FIXED(";
  printStorageType(*this, expr.elemType->resolved);
  *this << ", " << *expr.sizeExpr << ")";
}

void CPrinter::print(const AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    *this << *expr.expr << "->" << expr.member;
  } else {
    GenericPrinter::print(expr);
  }
}

void CPrinter::print(const AST::AssignStatement &expr) {
  if (expr.right->type.isAgent()) {
    // Agent assignments are interpreted as copies, not reference assignments
    *this << "*" << *expr.left << " = *" << *expr.right << ";";
  } else {
    GenericPrinter::print(expr);
  }
}
void CPrinter::print(const AST::VarDeclarationStatement &stmt) {
  Type type = stmt.type->resolved;
  if (typeRequiresStorage(type)) {
    // Type requires a separate variable for storage.
    // This makes access to it consistent lateron
    std::string sLabel = makeAnonLabel();
    printStorageType(*this, type);
    *this << " " << sLabel;
    if (stmt.initializer) {
      *this << " = ";
      if (stmt.initializer->type.isAgent()) {
        // Agent assignments are interpreted as copies, not reference assignments
        *this << "*";
      }
      *this << *stmt.initializer;
    }
    *this << ";" << nl;
    *this << type << " " << *stmt.var << " = &" << sLabel << ";";
  } else {
    GenericPrinter::print(stmt);
  }
}

static void printRangeFor(CPrinter &p, const AST::ForStatement &stmt) {
  std::string eLabel = p.makeAnonLabel();
  auto range = stmt.getRange();
  p << "for (int " << *stmt.var << " = " << range.first
    << ", " << eLabel << " = " << range.second << "; "
    << *stmt.var << " < " << eLabel << "; ++" << *stmt.var << ") " << *stmt.stmt;
}

void CPrinter::print(const AST::ForStatement &stmt) {
  if (stmt.isRange()) {
    printRangeFor(*this, stmt);
    return;
  } else if (stmt.isNear()) {
    const AST::Expression &agentExpr = stmt.getNearAgent();
    const AST::Expression &radiusExpr = stmt.getNearRadius();

    AST::AgentDeclaration *agentDecl = stmt.type->resolved.getAgentDecl();
    AST::AgentMember *posMember = agentDecl->getPositionMember();
    const char *dist_fn = posMember->type->resolved == Type::VEC2
      ? "dist_float2" : "dist_float3";

    std::string eLabel = makeAnonLabel();
    std::string iLabel = makeAnonLabel();

    // For now: Print normal loop with radius check
    *this << "for (size_t " << iLabel << " = 0; "
          << iLabel << " < agents.agents_" << agentDecl->name << ".len; "
          << iLabel << "++) {"
          << indent << nl << *stmt.type << " " << *stmt.var
          << " = DYN_ARRAY_GET(&agents.agents_" << agentDecl->name << ", ";
    printStorageType(*this, stmt.type->resolved);
    *this << ", " << iLabel << ");" << nl
          << "if (" << dist_fn << "(" << *stmt.var << "->" << posMember->name << ", "
          << agentExpr << "->" << posMember->name << ") > " << radiusExpr
          << ") continue;" << nl
          << *stmt.stmt << outdent << nl << "}";
    return;
  }

  // Normal range-based for loop
  std::string eLabel = makeAnonLabel();
  std::string iLabel = makeAnonLabel();

  *this << stmt.expr->type << " " << eLabel << " = " << *stmt.expr << ";" << nl
        << "for (size_t " << iLabel << " = 0; "
        << iLabel << " < " << eLabel << "->len; "
        << iLabel << "++) {"
        << indent << nl << *stmt.type << " " << *stmt.var
        << " = DYN_ARRAY_GET(" << eLabel << ", ";
  printStorageType(*this, stmt.type->resolved);
  *this << ", " << iLabel << ");" << nl
        << *stmt.stmt << outdent << nl << "}";
}
void CPrinter::print(const AST::SimulateStatement &stmt) {
  std::string tLabel = makeAnonLabel();
  *this << "for (int " << tLabel << " = 0; "
        << tLabel << " < " << *stmt.timestepsExpr << "; "
        << tLabel << "++) {" << indent << nl
        << "dyn_array tmp;";

  for (AST::FunctionDeclaration *stepFunc : stmt.stepFuncDecls) {
    const AST::Param &param = *(*stepFunc->params)[0];
    Type type = param.type->resolved;

    std::ostringstream s;
    s << "agents.agents_" << type;
    std::string bufName = s.str();
    s << "_dbuf";
    std::string dbufName = s.str();

    std::string iLabel = makeAnonLabel();
    std::string inLabel = makeAnonLabel();
    std::string outLabel = makeAnonLabel();

    *this << nl << "if (!" << dbufName << ".values) {" << indent
          << nl << dbufName << " = DYN_ARRAY_COPY_FIXED(" << type
          << ", &" << bufName << ");"
          << outdent << nl << "}" << nl
          << "#pragma omp parallel for" << nl
          << "for (size_t " << iLabel << " = 0; "
          << iLabel << " < " << bufName << ".len; "
          << iLabel << "++) {" << indent << nl
          << type << " *" << inLabel
          << " = DYN_ARRAY_GET(&" << bufName << ", " << type
          << ", " << iLabel << ");" << nl
          << type << " *" << outLabel
          << " = DYN_ARRAY_GET(&" << dbufName << ", " << type
          << ", " << iLabel << ");" << nl
          << stepFunc->name << "(" << inLabel << ", "
          << outLabel << ");" << outdent << nl << "}" << nl
          << "tmp = " << bufName << ";" << nl
          << bufName << " = " << dbufName << ";" << nl
          << dbufName << " = tmp;";
  }

  *this << outdent << nl << "}";
  // TODO Cleanup memory
}

void CPrinter::print(const AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

static void printTypeIdentifier(CPrinter &p, Type type) {
  switch (type.getTypeId()) {
    case Type::BOOL: p << "TYPE_BOOL"; break;
    case Type::INT32: p << "TYPE_INT"; break;
    case Type::FLOAT: p << "TYPE_FLOAT"; break;
    case Type::STRING: p << "TYPE_STRING"; break;
    case Type::VEC2: p << "TYPE_FLOAT2"; break;
    case Type::VEC3: p << "TYPE_FLOAT3"; break;
    default: assert(0);
  }
}
void CPrinter::print(const AST::AgentDeclaration &decl) {
  *this << "typedef struct {" << indent
        << *decl.members << outdent << nl
        << "} " << decl.name << ";" << nl;

  // Runtime type information
  *this << "static const type_info " << decl.name << "_info[] = {" << indent << nl;
  for (AST::AgentMemberPtr &member : *decl.members) {
    *this << "{ ";
    printTypeIdentifier(*this, member->type->resolved);
    *this << ", offsetof(" << decl.name << ", " << member->name
          << "), \"" << member->name << "\", "
          << (member->isPosition ? "true" : "false") << " }," << nl;
  }
  *this << "{ TYPE_END, sizeof(" << decl.name << "), NULL }" << outdent << nl << "};" << nl;
}

void CPrinter::print(const AST::FunctionDeclaration &decl) {
  if (decl.isMain()) {
    // Return result code from main()
    *this << "int main() {" << indent << *decl.stmts << nl
          << "return 0;" << outdent << nl << "}";
    return;
  }

  GenericPrinter::print(decl);
}

void CPrinter::print(const AST::Script &script) {
  *this << "#include \"libabl.h\"" << nl << nl;

  // First declare all agents
  for (AST::AgentDeclaration *decl : script.agents) {
    *this << *decl << nl;
  }

  // Create structure to store agents
  *this << "struct agent_struct {" << indent;
  for (AST::AgentDeclaration *decl : script.agents) {
    *this << nl << "dyn_array agents_" << decl->name << ";";
    *this << nl << "dyn_array agents_" << decl->name << "_dbuf;";
  }
  *this << outdent << nl << "};" << nl
        << "struct agent_struct agents;" << nl;

  // Create runtime type information for this structure
  *this << "static const agent_info agents_info[] = {" << indent << nl;
  for (AST::AgentDeclaration *decl : script.agents) {
    *this << "{ " << decl->name << "_info, "
          << "offsetof(struct agent_struct, agents_" << decl->name
          << "), \"" << decl->name << "\" }," << nl;
  }
  *this << "{ NULL, 0, NULL }" << outdent << nl << "};" << nl << nl;

  // Then declare everything else
  for (AST::ConstDeclaration *decl : script.consts) {
    *this << *decl << nl;
  }
  for (AST::FunctionDeclaration *decl : script.funcs) {
    *this << *decl << nl;
  }
}

}

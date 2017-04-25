#include <string>
#include "CPrinter.hpp"

namespace OpenABL {

static void printType(Printer &s, Type type) {
  if (type.isArray()) {
    s << "dyn_array*";
  } else if (type.isAgent()) {
    s << type.getAgentDecl()->name << '*';
  } else {
    s << type;
  }
}

static CPrinter &operator<<(CPrinter &s, Type type) {
  printType(s, type);
  return s;
}

static void printStorageType(Printer &s, Type type) {
  if (type.isArray()) {
    s << "dyn_array";
  } else {
    s << type;
  }
}

static bool typeRequiresStorage(Type type) {
  return type.isArray() || type.isAgent();
}
static void printBinaryOp(CPrinter &p, AST::BinaryOp op,
                          AST::Expression &left, AST::Expression &right) {
  Type l = left.type;
  Type r = right.type;
  if (l.isVec() || r.isVec()) {
    Type v = l.isVec() ? l : r;
    p << (v.getTypeId() == Type::VEC2 ? "float2_" : "float3_");
    switch (op) {
      case AST::BinaryOp::ADD: p << "add"; break;
      case AST::BinaryOp::SUB: p << "sub"; break;
      case AST::BinaryOp::DIV: p << "div_scalar"; break;
      case AST::BinaryOp::MUL:
        p << "mul_scalar";
        if (r.isVec()) {
          // Normalize to right multiplication
          // TODO Move into analysis
          p << "(" << right << ", " << left << ")";
          return;
        }
        break;
      default:
        assert(0);
    }
    p << "(" << left << ", " << right << ")";
    return;
  }

  p << "(" << left << " " << AST::getBinaryOpSigil(op) << " " << right << ")";
}
void CPrinter::print(AST::UnaryOpExpression &expr) {
  Type t = expr.expr->type;
  if (t.isVec()) {
    if (expr.op == AST::UnaryOp::PLUS) {
      // Nothing to do
      *this << *expr.expr;
    } else if (expr.op == AST::UnaryOp::MINUS) {
      // Compile to multiplication by -1.0
      AST::FloatLiteral negOne(-1.0, AST::Location());
      printBinaryOp(*this, AST::BinaryOp::MUL, *expr.expr, negOne);
    } else {
      assert(0);
    }
    return;
  }

  GenericCPrinter::print(expr);
}
void CPrinter::print(AST::BinaryOpExpression &expr) {
  printBinaryOp(*this, expr.op, *expr.left, *expr.right);
}
void CPrinter::print(AST::AssignExpression &expr) {
  if (expr.right->type.isAgent()) {
    // Agent assignments are interpreted as copies, not reference assignments
    *this << "(*" << *expr.left << " = *" << *expr.right << ")";
    // TODO This can't be used as a return value. Forbid expression use of assignments?
  } else {
    *this << "(" << *expr.left << " = " << *expr.right << ")";
  }
}
static void printTypeCtor(CPrinter &p, AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    size_t numArgs = expr.args->size();
    if (t.getTypeId() == Type::VEC2) {
      p << (numArgs == 1 ? "float2_fill" : "float2_create");
    } else {
      p << (numArgs == 1 ? "float3_fill" : "float3_create");
    }
    p << "(";
    p.printArgs(expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *(*expr.args)[0];
  }
}
static void printBuiltin(CPrinter &p, AST::CallExpression &expr) {
  const FunctionSignature &sig = expr.calledSig;
  if (sig.name == "add") {
    AST::AgentDeclaration *agent = sig.paramTypes[0].getAgentDecl();
    p << "*DYN_ARRAY_PLACE(&agents.agents_" << agent->name
      << ", " << agent->name << ") = " << *(*expr.args)[0];
    return;
  } else if (sig.name == "save") {
    p << "save(&agents, agents_info, " << *(*expr.args)[0] << ")";
    return;
  }

  p << sig.name;
  p << "(";
  p.printArgs(expr);
  p << ")";
}
void CPrinter::print(AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    printBuiltin(*this, expr);
  } else {
    *this << expr.name << "(";
    this->printArgs(expr);
    *this << ")";
  }
}
void CPrinter::print(AST::MemberAccessExpression &expr) {
  if (expr.expr->type.isAgent()) {
    *this << *expr.expr << "->" << expr.member;
  } else {
    *this << *expr.expr << "." << expr.member;
  }
}
void CPrinter::print(AST::TernaryExpression &expr) {
  *this << "(" << *expr.condExpr << " ? " << *expr.ifExpr << " : " << *expr.elseExpr << ")";
}
void CPrinter::print(AST::MemberInitEntry &entry) {
  *this << "." << entry.name << " = " << *entry.expr << ",";
}
void CPrinter::print(AST::AgentCreationExpression &expr) {
  *this << "(" << expr.name << ") {" << indent
        << *expr.members << outdent << nl << "}";
}
void CPrinter::print(AST::NewArrayExpression &expr) {
  *this << "DYN_ARRAY_CREATE_FIXED(";
  printStorageType(*this, expr.elemType->resolved);
  *this << ", " << *expr.sizeExpr << ")";
}

void CPrinter::print(AST::AssignOpStatement &stmt) {
  *this << *stmt.left << " = ";
  printBinaryOp(*this, stmt.op, *stmt.left, *stmt.right);
  *this << ";";
}
void CPrinter::print(AST::VarDeclarationStatement &stmt) {
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
    GenericCPrinter::print(stmt);
  }
}

static void printRangeFor(CPrinter &p, AST::ForStatement &stmt) {
  std::string eLabel = p.makeAnonLabel();
  auto range = stmt.getRange();
  p << "for (int " << *stmt.var << " = " << range.first
    << ", " << eLabel << " = " << range.second << "; "
    << *stmt.var << " < " << eLabel << "; ++" << *stmt.var << ") " << *stmt.stmt;
}

void CPrinter::print(AST::ForStatement &stmt) {
  if (stmt.isRange()) {
    printRangeFor(*this, stmt);
    return;
  } else if (stmt.isNear()) {
    AST::Expression &agentExpr = stmt.getNearAgent();
    AST::Expression &radiusExpr = stmt.getNearRadius();

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
void CPrinter::print(AST::SimulateStatement &stmt) {
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

    *this << nl << "if (!" << dbufName << ".values) { "
          << dbufName << " = DYN_ARRAY_CREATE_FIXED(" << type
          << ", " << bufName << ".len); }" << nl
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
    // TODO Semantics: At which point should be double buffer switch occur?
  }

  *this << outdent << nl << "}";
  // TODO Cleanup memory
}
void CPrinter::print(AST::ReturnStatement &stmt) {
  if (stmt.expr) {
    *this << "return " << *stmt.expr << ";";
  } else {
    *this << "return;";
  }
}
void CPrinter::print(AST::SimpleType &type) {
  *this << type.resolved;
}
void CPrinter::print(AST::ArrayType &type) {
  *this << type.resolved;
}
void CPrinter::print(AST::Param &param) {
  *this << *param.type << " " << *param.var;
  if (param.outVar) {
    *this << ", " << *param.type << " " << *param.outVar;
  }
}
void CPrinter::print(AST::FunctionDeclaration &decl) {
  if (decl.returnType) {
    *this << *decl.returnType;
  } else {
    *this << "void";
  }
  *this << " " << decl.name << "(";
  bool first = true;
  for (const AST::ParamPtr &param : *decl.params) {
    if (!first) *this << ", ";
    first = false;
    *this << *param;
  }
  *this << ") {" << indent;
  *this << *decl.stmts << outdent << nl << "}";
}
void CPrinter::print(AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}

static void printTypeIdentifier(CPrinter &p, Type type) {
  switch (type.getTypeId()) {
    case Type::BOOL: p << "TYPE_BOOL"; break;
    case Type::INT32: p << "TYPE_INT"; break;
    case Type::FLOAT32: p << "TYPE_FLOAT"; break;
    case Type::STRING: p << "TYPE_STRING"; break;
    case Type::VEC2: p << "TYPE_FLOAT2"; break;
    case Type::VEC3: p << "TYPE_FLOAT3"; break;
    default: assert(0);
  }
}
void CPrinter::print(AST::AgentDeclaration &decl) {
  *this << "typedef struct {" << indent
        << *decl.members << outdent << nl
        << "} " << decl.name << ";" << nl;

  // Runtime type information
  *this << "static const type_info " << decl.name << "_info[] = {" << indent << nl;
  for (AST::AgentMemberPtr &member : *decl.members) {
    *this << "{ ";
    printTypeIdentifier(*this, member->type->resolved);
    *this << ", offsetof(" << decl.name << ", " << member->name
          << "), \"" << member->name << "\" }," << nl;
  }
  *this << "{ TYPE_END, sizeof(" << decl.name << "), NULL }" << outdent << nl << "};" << nl;
}
void CPrinter::print(AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var << " = " << *decl.expr << ";";
}
void CPrinter::print(AST::Script &script) {
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

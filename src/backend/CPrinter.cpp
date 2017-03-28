#include <string>
#include "CPrinter.hpp"

namespace OpenABL {

static void printStringLiteral(Printer &p, const std::string &str) {
  p << '"';
  for (char c : str) {
    if (c == '"' || c == '\\') {
      p << '\\' << c;
    } else {
      p << c;
    }
  }
  p << '"';
}

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

void CPrinter::print(AST::Var &var) {
  *this << var.name;
}
void CPrinter::print(AST::Literal &lit) {
  if (AST::IntLiteral *ilit = dynamic_cast<AST::IntLiteral *>(&lit)) {
    *this << ilit->value;
  } else if (AST::FloatLiteral *flit = dynamic_cast<AST::FloatLiteral *>(&lit)) {
    *this << flit->value;
  } else if (AST::BoolLiteral *blit = dynamic_cast<AST::BoolLiteral *>(&lit)) {
    *this << blit->value;
  } else if (AST::StringLiteral *slit = dynamic_cast<AST::StringLiteral *>(&lit)) {
    printStringLiteral(*this, slit->value);
  } else {
    assert(0);
  }
}
void CPrinter::print(AST::VarExpression &expr) {
  *this << *expr.var;
}
void CPrinter::print(AST::UnaryOpExpression &expr) {
  *this << "(" << AST::getUnaryOpSigil(expr.op) << *expr.expr << ")";
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
void CPrinter::print(AST::BinaryOpExpression &expr) {
  printBinaryOp(*this, expr.op, *expr.left, *expr.right);
}
void CPrinter::print(AST::AssignOpExpression &expr) {
  *this << "(" << *expr.left << " = ";
  printBinaryOp(*this, expr.op, *expr.left, *expr.right);
  *this << ")";
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
void CPrinter::print(AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}

static void printArgs(CPrinter &p, AST::CallExpression &expr) {
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) p << ", ";
    first = false;
    p << *arg;
  }
}
static void printTypeCtor(CPrinter &p, AST::CallExpression &expr) {
  Type t = expr.type;
  if (t.isVec()) {
    if (t.getTypeId() == Type::VEC2) {
      p << "float2_create";
    } else {
      p << "float3_create";
    }
    p << "(";
    printArgs(p, expr);
    p << ")";
  } else {
    p << "(" << t << ") " << *(*expr.args)[0];
  }
}
static void printBuiltin(CPrinter &p, AST::CallExpression &expr) {
  const FunctionSignature &sig = expr.calledSig;
  p << sig.name;
  p << "(";
  printArgs(p, expr);
  if (sig.name == "save") {
    // Pass runtime type information
    AST::AgentDeclaration *agent = sig.paramTypes[0].getBaseType().getAgentDecl();
    p << ", " << agent->name << "_info";
  }
  p << ")";
}
void CPrinter::print(AST::CallExpression &expr) {
  if (expr.isCtor()) {
    printTypeCtor(*this, expr);
  } else if (expr.isBuiltin()) {
    printBuiltin(*this, expr);
  } else {
    *this << expr.name << "(";
    printArgs(*this, expr);
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
void CPrinter::print(AST::NewArrayExpression &expr) {
  *this << "DYN_ARRAY_CREATE_FIXED(";
  printStorageType(*this, expr.elemType->resolved);
  *this << ", " << *expr.sizeExpr << ")";
}
void CPrinter::print(AST::ExpressionStatement &stmt) {
  *this << *stmt.expr << ";";
}
void CPrinter::print(AST::BlockStatement &stmt) {
  *this << "{" << indent << *stmt.stmts << outdent << nl << "}";
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
    *this << *stmt.type << " " << *stmt.var;
    if (stmt.initializer) {
      *this << " = " << *stmt.initializer;
    }
    *this << ";";
  }
}
void CPrinter::print(AST::IfStatement &stmt) {
  *this << "if (" << *stmt.condExpr << ") " << *stmt.ifStmt;
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
    AST::Expression &arrayExpr = stmt.getNearArray();
    AST::Expression &agentExpr = stmt.getNearAgent();
    AST::Expression &radiusExpr = stmt.getNearRadius();

    AST::AgentDeclaration *agentDecl = stmt.type->resolved.getAgentDecl();
    AST::AgentMember *posMember = agentDecl->getPositionMember();
    const char *dist_fn = posMember->type->resolved == Type::VEC2
      ? "dist_float2" : "dist_float3";

    std::string eLabel = makeAnonLabel();
    std::string iLabel = makeAnonLabel();

    // For now: Print normal loop with radius check
    *this << stmt.expr->type << " " << eLabel << " = " << arrayExpr << ";" << nl
          << "for (size_t " << iLabel << " = 0; "
          << iLabel << " < " << eLabel << "->len; "
          << iLabel << "++) {"
          << indent << nl << *stmt.type << " " << *stmt.var
          << " = DYN_ARRAY_GET(" << eLabel << ", ";
    printStorageType(*this, stmt.type->resolved);
    *this << ", " << iLabel << ");" << nl
          << "if (" << dist_fn << "(" << *stmt.var << "->" << posMember->name << ", "
          << agentExpr << "->" << posMember->name << ") > " << radiusExpr
          << ") continue;" << nl
          << *stmt.stmt << outdent << nl << "}";
    return;
  }

  // TODO special loops (ranges, near)
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
void CPrinter::print(AST::ParallelForStatement &stmt) {
  std::string iLabel = makeAnonLabel();

  *this << "if (!double_buf) { double_buf_storage = DYN_ARRAY_CREATE_FIXED(";
  printStorageType(*this, stmt.type->resolved);
  *this << ", " << *stmt.expr << "->len); "
        << "double_buf = &double_buf_storage; }" << "\n"
        << "#pragma omp parallel for" << nl
        << "for (size_t " << iLabel << " = 0; "
        << iLabel << " < " << *stmt.expr << "->len; "
        << iLabel << "++) {" << indent << nl
        << *stmt.type << " " << *stmt.var
        << " = DYN_ARRAY_GET(" << *stmt.expr << ", ";
  printStorageType(*this, stmt.type->resolved);
  *this << ", " << iLabel << ");" << nl
        << *stmt.type << " " << *stmt.outVar
        << " = DYN_ARRAY_GET(double_buf, ";
  printStorageType(*this, stmt.type->resolved);
  *this << ", " << iLabel << ");" << nl
        << *stmt.stmt << outdent << nl << "}" << nl
        << "{ dyn_array* tmp = " << *stmt.expr << "; "
        << *stmt.expr << " = double_buf; double_buf = tmp; }";
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
  if (decl.name == "main") {
    // TODO Make this more generic
    *this << nl << "dyn_array double_buf_storage; dyn_array* double_buf = NULL;";
  }
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
  *this << "const type_info " << decl.name << "_info[] = {" << indent << nl;
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
  for (const AST::DeclarationPtr &decl : *script.decls) {
    *this << *decl << nl;
  }
}

}

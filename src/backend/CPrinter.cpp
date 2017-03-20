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
void CPrinter::print(AST::BinaryOpExpression &expr) {
  Type l = expr.left->type;
  Type r = expr.right->type;
  if (l.isVec() || r.isVec()) {
    Type v = l.isVec() ? l : r;
    *this << (v.getTypeId() == Type::VEC2 ? "float2_" : "float3_");
    switch (expr.op) {
      case AST::BinaryOp::ADD: *this << "add"; break;
      case AST::BinaryOp::SUB: *this << "sub"; break;
      case AST::BinaryOp::DIV: *this << "div_scalar"; break;
      case AST::BinaryOp::MUL:
        *this << "mul_scalar";
        if (r.isVec()) {
          // Normalize to right multiplication
          // TODO Move into analysis
          *this << "(" << *expr.right << ", " << *expr.left << ")";
          return;
        }
        break;
      default:
        assert(0);
    }
    *this << "(" << *expr.left << ", " << *expr.right << ")";
    return;
  }

  *this << "(" << *expr.left << " " << AST::getBinaryOpSigil(expr.op)
        << " " << *expr.right << ")";
}
void CPrinter::print(AST::AssignOpExpression &expr) {
  *this << "(" << *expr.left << " " << AST::getBinaryOpSigil(expr.op)
        << "= " << *expr.right << ")";
}
void CPrinter::print(AST::AssignExpression &expr) {
  *this << "(" << *expr.left << " = " << *expr.right << ")";
}
void CPrinter::print(AST::Arg &arg) {
  *this << *arg.expr;
  if (arg.outExpr) {
    *this << ", " << *arg.outExpr;
  }
}
void CPrinter::print(AST::CallExpression &expr) {
  *this << expr.name << "(";
  bool first = true;
  for (const AST::ArgPtr &arg : *expr.args) {
    if (!first) *this << ", ";
    first = false;
    *this << *arg;
  }
  *this << ")";
}
void CPrinter::print(AST::MemberAccessExpression &expr) {
  *this << *expr.expr << "." << expr.member;
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
      *this << " = " << *stmt.initializer;
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
  AST::BinaryOpExpression *range = stmt.getRangeExpr();
  p << "for (int " << *stmt.var << " = " << *range->left
    << ", " << eLabel << " = " << *range->right << "; "
    << *stmt.var << " < " << eLabel << "; ++" << *stmt.var << ") " << *stmt.stmt;
}

void CPrinter::print(AST::ForStatement &stmt) {
  if (stmt.isRange()) {
    printRangeFor(*this, stmt);
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
  // TODO
  *this << "#pragma omp parallel for" << nl << "// TODO " << *stmt.stmt;
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
  *this << ") {" << indent << *decl.stmts << outdent << nl << "}";
}
void CPrinter::print(AST::AgentMember &member) {
  *this << *member.type << " " << member.name << ";";
}
void CPrinter::print(AST::AgentDeclaration &decl) {
  *this << "struct " << decl.name
        << " {" << indent << *decl.members << outdent << nl << "};";
}
void CPrinter::print(AST::ConstDeclaration &decl) {
  *this << *decl.type << " " << *decl.var << " = " << *decl.value << ";";
}
void CPrinter::print(AST::Script &script) {
  *this << "#include \"libabl.h\"" << nl << nl;
  for (const AST::DeclarationPtr &decl : *script.decls) {
    *this << *decl << nl;
  }
}

}

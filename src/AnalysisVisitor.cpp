#include "AnalysisVisitor.hpp"
#include "ErrorHandling.hpp"

namespace OpenABL {

static Type tryResolveNameToSimpleType(const std::string &name) {
    if (name == "void") {
      return { Type::VOID };
    } else if (name == "bool") {
      return { Type::BOOL };
    } else if (name == "int") {
      return { Type::INT32 };
    } else if (name == "float") {
      return { Type::FLOAT32 };
    } else if (name == "string") {
      return { Type::STRING };
    } else if (name == "float2") {
      return { Type::VEC2 };
    } else if (name == "float3") {
      return { Type::VEC3 };
    } else {
      return { Type::INVALID };
    }
}

Type AnalysisVisitor::resolveAstType(AST::Type &type) {
  if (AST::SimpleType *t = dynamic_cast<AST::SimpleType *>(&type)) {
    Type st = tryResolveNameToSimpleType(t->name);
    if (!st.isInvalid()) {
      return st;
    } else {
      // Agent type
      auto it = agents.find(t->name);
      if (it == agents.end()) {
        err << "Unknown type \"" << t->name << "\"" << type.loc;
        return { Type::INVALID };
      }
      return { Type::AGENT, it->second };
    }
  } else if (AST::ArrayType *t = dynamic_cast<AST::ArrayType *>(&type)) {
    Type baseType = resolveAstType(*t->type);
    if (baseType.isArray()) {
      err << "Arrays may only be nested to one level" << type.loc;
      return { Type::INVALID };
    }

    return { Type::ARRAY, baseType };
  } else {
    assert(0);
  }
}

VarId AnalysisVisitor::declareVar(std::string name, Type type) {
  auto it = varMap.find(name);
  if (it != varMap.end()) {
    std::cout << "Cannot redeclare " << name << std::endl;
    // TODO migrate to errstream
    // TODO We need more specific location information for identifiers
  }

  VarId id = VarId::make();
  varMap.insert({ name, id });
  scope.add(id, type);
  return id;
}

void AnalysisVisitor::pushVarScope() {
  varMapStack.push(varMap);
};
void AnalysisVisitor::popVarScope() {
  assert(!varMapStack.empty());
  varMap = varMapStack.top();
  varMapStack.pop();
};

void AnalysisVisitor::enter(AST::Var &) {};
void AnalysisVisitor::enter(AST::Literal &) {};
void AnalysisVisitor::enter(AST::UnaryOpExpression &) {};
void AnalysisVisitor::enter(AST::BinaryOpExpression &) {};
void AnalysisVisitor::enter(AST::AssignOpExpression &) {};
void AnalysisVisitor::enter(AST::AssignExpression &) {};
void AnalysisVisitor::enter(AST::Arg &) {};
void AnalysisVisitor::enter(AST::CallExpression &) {};
void AnalysisVisitor::enter(AST::MemberAccessExpression &) {};
void AnalysisVisitor::enter(AST::TernaryExpression &) {};
void AnalysisVisitor::enter(AST::NewArrayExpression &) {};
void AnalysisVisitor::enter(AST::ExpressionStatement &) {};
void AnalysisVisitor::enter(AST::IfStatement &) {};
void AnalysisVisitor::enter(AST::AgentMember &) {};
void AnalysisVisitor::enter(AST::Script &) {};
void AnalysisVisitor::enter(AST::VarExpression &) {};
void AnalysisVisitor::enter(AST::ConstDeclaration &) {};
void AnalysisVisitor::enter(AST::VarDeclarationStatement &) {};
void AnalysisVisitor::enter(AST::Param &) {};
void AnalysisVisitor::leave(AST::Var &) {};
void AnalysisVisitor::leave(AST::UnaryOpExpression &) {};
void AnalysisVisitor::leave(AST::AssignOpExpression &) {};
void AnalysisVisitor::leave(AST::AssignExpression &) {};
void AnalysisVisitor::leave(AST::Arg &) {};
void AnalysisVisitor::leave(AST::TernaryExpression &) {};
void AnalysisVisitor::leave(AST::ExpressionStatement &) {};
void AnalysisVisitor::leave(AST::IfStatement &) {};
void AnalysisVisitor::leave(AST::SimpleType &) {};
void AnalysisVisitor::leave(AST::ArrayType &) {};
void AnalysisVisitor::leave(AST::AgentMember &) {};
void AnalysisVisitor::leave(AST::AgentDeclaration &) {};
void AnalysisVisitor::leave(AST::Script &) {};

void AnalysisVisitor::enter(AST::FunctionDeclaration &decl) {
  pushVarScope();

  auto it = funcs.find(decl.name);
  if (it != funcs.end()) {
    err << "Redeclaration of function named \"" << decl.name << "\"" << decl.loc;
    return;
  }

  funcs.insert({ decl.name, &decl });
};
void AnalysisVisitor::leave(AST::FunctionDeclaration &) {
  popVarScope();
};
void AnalysisVisitor::enter(AST::BlockStatement &) {
  pushVarScope();
};
void AnalysisVisitor::leave(AST::BlockStatement &) {
  popVarScope();
};

void AnalysisVisitor::enter(AST::SimpleType &type) {
  type.resolved = resolveAstType(type);
};
void AnalysisVisitor::enter(AST::ArrayType &type) {
  type.resolved = resolveAstType(type);
};

void AnalysisVisitor::leave(AST::ConstDeclaration &decl) {
  decl.var->id = declareVar(decl.var->name, decl.type->resolved);
};
void AnalysisVisitor::leave(AST::VarDeclarationStatement &decl) {
  decl.var->id = declareVar(decl.var->name, decl.type->resolved);
  if (decl.initializer) {
    Type declType = decl.type->resolved;
    Type initType = decl.initializer->type;
    if (declType != initType) {
      err << "Trying to assign value of type " << initType
          << " to variable of type " << declType << decl.initializer->loc;
    }
  }
};

void AnalysisVisitor::leave(AST::Param &param) {
  param.var->id = declareVar(param.var->name, param.type->resolved);
  if (param.outVar) {
    param.outVar->id = declareVar(param.outVar->name, param.type->resolved);
  }
}

void AnalysisVisitor::enter(AST::ForStatement &stmt) {
  pushVarScope();

  Type type = resolveAstType(*stmt.type); // Not resolved yet
  stmt.var->id = declareVar(stmt.var->name, type);
};
void AnalysisVisitor::leave(AST::ForStatement &) {
  popVarScope();
};

void AnalysisVisitor::enter(AST::ParallelForStatement &stmt) {
  pushVarScope();

  Type type = resolveAstType(*stmt.type); // Not resolved yet
  stmt.var->id = declareVar(stmt.var->name, type);
  stmt.outVar->id = declareVar(stmt.outVar->name, type);
};
void AnalysisVisitor::leave(AST::ParallelForStatement &) {
  popVarScope();
};

void AnalysisVisitor::enter(AST::AgentDeclaration &decl) {
  auto it = agents.find(decl.name);
  if (it != agents.end()) {
    err << "Redefinition of agent " << decl.name << decl.loc;
    return;
  }

  agents.insert({ decl.name, &decl });
};

void AnalysisVisitor::leave(AST::VarExpression &expr) {
  AST::Var &var = *expr.var;
  auto it = varMap.find(var.name);
  if (it == varMap.end()) {
    err << "Use of undeclared variable " << var.name << expr.loc;
    return;
  }

  var.id = it->second;
  expr.type = scope.get(var.id).type;
}

void AnalysisVisitor::leave(AST::Literal &lit) {
  if (AST::IntLiteral *ilit = dynamic_cast<AST::IntLiteral *>(&lit)) {
    lit.type = { Type::INT32 };
  } else if (AST::FloatLiteral *flit = dynamic_cast<AST::FloatLiteral *>(&lit)) {
    lit.type = { Type::FLOAT32 };
  } else if (AST::BoolLiteral *blit = dynamic_cast<AST::BoolLiteral *>(&lit)) {
    lit.type = { Type::BOOL };
  } else if (AST::StringLiteral *slit = dynamic_cast<AST::StringLiteral *>(&lit)) {
    lit.type = { Type::STRING };
  } else {
    assert(0);
  }
};

static bool promoteToFloat(Type l, Type r) {
  return (l.isFloat() && r.isInt()) || (r.isFloat() && l.isInt());
}

static Type getBinaryOpType(AST::BinaryOp op, Type l, Type r) {
  switch (op) {
    case AST::BinaryOp::ADD:
    case AST::BinaryOp::SUB:
      if (!l.isNumOrVec() || !r.isNumOrVec()) {
        return { Type::INVALID };
      }
      if (l != r) {
        if (promoteToFloat(l, r)) {
          return { Type::FLOAT32 };
        } else {
          return { Type::INVALID };
        }
      }
      return l;
    case AST::BinaryOp::MUL:
    case AST::BinaryOp::DIV:
      if (!l.isNumOrVec() || !r.isNumOrVec()) {
        return { Type::INVALID };
      }
      if (l.isVec() && r.isVec()) {
        return { Type::INVALID };
      }
      if (l.isVec()) {
        return l;
      }
      if (r.isVec()) {
        if (op == AST::BinaryOp::DIV) {
          return { Type::INVALID };
        }
        return r;
      }
      if (l != r) {
        if (promoteToFloat(l, r)) {
          return { Type::FLOAT32 };
        } else {
          return { Type::INVALID };
        }
      }
      return l;
    case AST::BinaryOp::MOD:
      if (!l.isInt() || !r.isInt()) {
        return { Type::INVALID };
      }
      return { Type::INT32 };
    case AST::BinaryOp::SMALLER:
    case AST::BinaryOp::SMALLER_EQUALS:
    case AST::BinaryOp::GREATER:
    case AST::BinaryOp::GREATER_EQUALS:
      if (!l.isNum() || !r.isNum()) {
        return { Type::INVALID };
      }
      return { Type:: BOOL };
    case AST::BinaryOp::RANGE:
      if (!l.isInt() || !r.isInt()) {
        return { Type::INVALID };
      }
      return { Type::ARRAY, Type::INT32 };
    default:
      return { Type::INVALID };
  }
}

void AnalysisVisitor::leave(AST::BinaryOpExpression &expr) {
  expr.type = getBinaryOpType(expr.op, expr.left->type, expr.right->type);
  if (expr.type.isInvalid()) {
    err << "Type mismatch (" << expr.left->type << " "
        << getBinaryOpSigil(expr.op) << " " << expr.right->type << ")" << expr.loc;
  }
};

void AnalysisVisitor::leave(AST::NewArrayExpression &expr) {
  Type elemType = expr.elemType->resolved;
  if (elemType.isArray()) {
    err << "Cannot instantiate nested array type" << expr.elemType->loc;
    return;
  }

  expr.type = { Type::ARRAY, elemType };
};

static AST::AgentMember *findAgentMember(AST::AgentDeclaration &decl, const std::string &name) {
  for (AST::AgentMemberPtr &member : *decl.members) {
    if (member->name == name) {
      return &*member;
    }
  }
  return nullptr;
}

void AnalysisVisitor::leave(AST::MemberAccessExpression &expr) {
  Type type = expr.expr->type;
  if (type.isVec()) {
    // TODO
  } else if (type.isAgent()) {
    AST::AgentDeclaration *agent = type.getAgentDecl();
    AST::AgentMember *member = findAgentMember(*agent, expr.member);
    if (!member) {
      err << "Agent has no member \"" << expr.member << "\"" << expr.loc;
      return;
    }

    expr.type = member->type->resolved;
  } else {
    err << "Can only access members on agent or vector type" << expr.loc;
    return;
  }
};

void AnalysisVisitor::leave(AST::CallExpression &expr) {
  Type t = tryResolveNameToSimpleType(expr.name);
  if (!t.isInvalid()) {
    // This is a type constructor / cast
    // TODO: Represent this in the AST and handle it in the backend
    expr.type = t;
    return;
  }

  auto it = funcs.find(expr.name);
  if (it == funcs.end()) {
    err << "Call to unknown function \"" << expr.name << "\"" << expr.loc;
    return;
  }
};

}

#include "AnalysisVisitor.hpp"
#include "ErrorHandling.hpp"

#define SKIP_INVALID(type) do { \
  if (type.isInvalid()) { return; } \
} while (0)

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

void AnalysisVisitor::declareVar(AST::Var &var, Type type, bool isConst, bool isGlobal) {
  auto it = varMap.find(var.name);
  if (it != varMap.end()) {
    err << "Cannot redeclare variable \"" << var.name << "\"" << var.loc;
  }

  var.id = VarId::make();
  varMap.insert({ var.name, var.id });
  scope.add(var.id, type, isConst, isGlobal);
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
void AnalysisVisitor::enter(AST::Arg &) {};
void AnalysisVisitor::enter(AST::CallExpression &) {};
void AnalysisVisitor::enter(AST::MemberAccessExpression &) {};
void AnalysisVisitor::enter(AST::TernaryExpression &) {};
void AnalysisVisitor::enter(AST::MemberInitEntry &) {};
void AnalysisVisitor::enter(AST::AgentCreationExpression &) {};
void AnalysisVisitor::enter(AST::NewArrayExpression &) {};
void AnalysisVisitor::enter(AST::ExpressionStatement &) {};
void AnalysisVisitor::enter(AST::AssignStatement &) {};
void AnalysisVisitor::enter(AST::AssignOpStatement &) {};
void AnalysisVisitor::enter(AST::IfStatement &) {};
void AnalysisVisitor::enter(AST::SimulateStatement &) {};
void AnalysisVisitor::enter(AST::ReturnStatement &) {};
void AnalysisVisitor::enter(AST::AgentMember &) {};
void AnalysisVisitor::enter(AST::Script &) {};
void AnalysisVisitor::enter(AST::VarExpression &) {};
void AnalysisVisitor::enter(AST::VarDeclarationStatement &) {};
void AnalysisVisitor::enter(AST::Param &) {};
void AnalysisVisitor::enter(AST::EnvironmentDeclaration &) {};
void AnalysisVisitor::leave(AST::Var &) {};
void AnalysisVisitor::leave(AST::Arg &) {};
void AnalysisVisitor::leave(AST::MemberInitEntry &) {};
void AnalysisVisitor::leave(AST::ExpressionStatement &) {};
void AnalysisVisitor::leave(AST::IfStatement &) {};
void AnalysisVisitor::leave(AST::SimpleType &) {};
void AnalysisVisitor::leave(AST::ArrayType &) {};
void AnalysisVisitor::leave(AST::AgentMember &) {};
void AnalysisVisitor::leave(AST::AgentDeclaration &) {};

void AnalysisVisitor::enter(AST::FunctionDeclaration &decl) {
  pushVarScope();

  auto it = funcs.find(decl.name);
  if (it != funcs.end()) {
    err << "Redeclaration of function named \"" << decl.name << "\"" << decl.loc;
    return;
  }

  funcs.insert({ decl.name, &decl });
  currentFunc = &decl;

  script.funcs.push_back(&decl);

  if (decl.isMain()) {
    script.mainFunc = &decl;
  }
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

bool isConstantExpression(const AST::Expression &expr) {
  if (dynamic_cast<const AST::Literal *>(&expr)) {
    return true;
  } else if (dynamic_cast<const AST::UnaryOpExpression *>(&expr)) {
    return true;
  }
  // TODO Other expressions
  return false;
}

void AnalysisVisitor::leave(AST::ConstDeclaration &decl) {
  declareVar(*decl.var, decl.type->resolved, true, true);

  if (!isConstantExpression(*decl.expr)) {
    err << "Initializer of global constant must be a constant expression" << decl.expr->loc;
    return;
  }

  Type declType = decl.type->resolved;
  Type exprType = decl.expr->type;
  if (!exprType.isPromotableTo(declType)) {
    err << "Trying to assign value of type " << exprType
        << " to global of type " << declType << decl.expr->loc;
  }
};

void AnalysisVisitor::leave(AST::EnvironmentDeclaration &decl) {
  if (script.envDecl) {
    err << "Script can only contain a single environment specification" << decl.loc;
    return;
  }

  for (const AST::MemberInitEntryPtr &member : *decl.members) {
    if (member->name == "size") {
      const AST::Expression &sizeExpr = *member->expr;
      if (!sizeExpr.type.isVec()) {
        err << "Environment size must be float2 or float3" << decl.loc;
        return;
      }
      decl.sizeExpr = &sizeExpr;
    } else {
      err << "Unknown environment member \"" << member->name << "\"" << member->loc;
      return;
    }
  }

  script.envDecl = &decl;
};

void AnalysisVisitor::leave(AST::VarDeclarationStatement &decl) {
  declareVar(*decl.var, decl.type->resolved, false, false);
  if (decl.initializer) {
    Type declType = decl.type->resolved;
    Type initType = decl.initializer->type;
    SKIP_INVALID(declType);
    SKIP_INVALID(initType);

    if (!initType.isPromotableTo(declType)) {
      err << "Trying to assign value of type " << initType
          << " to variable of type " << declType << decl.initializer->loc;
    }
  }
};

void AnalysisVisitor::leave(AST::Param &param) {
  // If an in -> out structgure is used, "in" is immutable
  bool isImmutable = param.outVar != nullptr;
  declareVar(*param.var, param.type->resolved, isImmutable, false);
  if (param.outVar) {
    declareVar(*param.outVar, param.type->resolved, false, false);
  }
}

void AnalysisVisitor::enter(AST::ForStatement &stmt) {
  pushVarScope();

  Type declType = resolveAstType(*stmt.type); // Not resolved yet
  declareVar(*stmt.var, declType, true, false);

  // Handle for-near loops early, as we want to collect member accesses
  if (AST::CallExpression *call = dynamic_cast<AST::CallExpression *>(&*stmt.expr)) {
    if (call->name == "near") {
      if (!declType.isAgent()) {
        err << "Type specified in for-near loop is not an agent" << stmt.type->loc;
        return;
      }

      if (currentFunc->accessedAgent) {
        // TODO relax?
        err << "Multiple for-near loops in a single step function" << stmt.loc;
        return;
      }

      AST::AgentDeclaration *agent = declType.getAgentDecl();
      if (!agent->getPositionMember()) {
        err << "Cannot use for-near loop on agent without position member" << stmt.loc;
        return;
      }

      currentFunc->accessedAgent = agent;
      stmt.kind = AST::ForStatement::Kind::NEAR;

      // Collect member accesses on this variable
      collectAccessVar = stmt.var->id;
      return;
    }
  }
};
void AnalysisVisitor::leave(AST::ForStatement &stmt) {
  popVarScope();

  if (stmt.isNear()) {
    // Already handled, only disable member collection
    collectAccessVar.reset();
    return;
  }

  Type declType = stmt.type->resolved;
  Type exprType = stmt.expr->type;
  if (!exprType.isArray()) {
    err << "Can only use for with array type, received " << exprType << stmt.expr->loc;
    return;
  }

  if (!exprType.getBaseType().isCompatibleWith(declType)) {
    err << "For expression type " << exprType
        << " not compatible with declared " << declType << stmt.expr->loc;
    return;
  }

  // TODO enforce these are only used in for() loops
  if (AST::BinaryOpExpression *op = dynamic_cast<AST::BinaryOpExpression *>(&*stmt.expr)) {
    if (op->op == AST::BinaryOp::RANGE) {
      stmt.kind = AST::ForStatement::Kind::RANGE;
      return;
    }
  }

  stmt.kind = AST::ForStatement::Kind::NORMAL;
};

bool isStepFunction(const AST::FunctionDeclaration &fn) {
    if (fn.params->size() != 1) {
      return false;
    }

    const AST::Param &param = *(*fn.params)[0];
    if (!param.outVar) {
      return false;
    }

    if (!param.type->resolved.isAgent()) {
      return false;
    }

    return true;
}

void AnalysisVisitor::leave(AST::SimulateStatement &stmt) {
  if (script.simStmt) {
    err << "Script can only contain a single simulate statement" << stmt.loc;
    return;
  }

  if (!stmt.timestepsExpr->type.isPromotableTo(Type::INT32)) {
    err << "Number of timesteps must be an integer, "
        << stmt.timestepsExpr->type << " given" << stmt.timestepsExpr->loc;
    return;
  }

  for (const std::string &name : *stmt.stepFuncs) {
    auto it = funcs.find(name);
    if (it == funcs.end()) {
      err << "Unknown step function \"" << name << "\"" << stmt.loc;
      return;
    }

    AST::FunctionDeclaration &fn = *it->second;
    if (!isStepFunction(fn)) {
      err << "Function \"" << name << "\" is not a step function" << stmt.loc;
      return;
    }

    fn.isStep = true;
    stmt.stepFuncDecls.push_back(&fn);
  }

  script.simStmt = &stmt;
};

void AnalysisVisitor::leave(AST::ReturnStatement &stmt) {
  Type returnType = currentFunc->returnType->resolved;
  if (returnType.isVoid()) {
    if (stmt.expr) {
      err << "Cannot return value from void function" << stmt.expr->loc;
      return;
    }
  } else {
    if (!stmt.expr) {
      err << "Return from non-void function must specify value" << stmt.loc;
      return;
    }

    if (!stmt.expr->type.isPromotableTo(returnType)) {
      err << "Trying to return " << stmt.expr->type
          << " from function with return type " << returnType << stmt.expr->loc;
      return;
    }
  }
};

static bool isConst(const Scope &scope, AST::Expression &expr) {
  if (auto var = dynamic_cast<AST::VarExpression *>(&expr)) {
    const ScopeEntry &entry = scope.get(var->var->id);
    return entry.isConst;
  } else if (auto access = dynamic_cast<AST::MemberAccessExpression *>(&expr)) {
    return isConst(scope, *access->expr);
  } else {
    // This should only be called on "variable-like" expressions
    assert(0);
  }
}

void AnalysisVisitor::leave(AST::AssignStatement &stmt) {
  if (isConst(scope, *stmt.left)) {
    err << "Trying to assign to immutable variable" << stmt.left->loc;
    return;
  }

  // TODO Check type compatibility
};

void AnalysisVisitor::leave(AST::AssignOpStatement &stmt) {
  if (isConst(scope, *stmt.left)) {
    err << "Trying to assign to immutable variable" << stmt.left->loc;
    return;
  }

  // TODO Check type compatibility
};

void AnalysisVisitor::enter(AST::AgentDeclaration &decl) {
  auto it = agents.find(decl.name);
  if (it != agents.end()) {
    err << "Redefinition of agent " << decl.name << decl.loc;
    return;
  }

  agents.insert({ decl.name, &decl });
  script.agents.push_back(&decl);
};

void AnalysisVisitor::enter(AST::ConstDeclaration &decl) {
  script.consts.push_back(&decl);
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
  if (dynamic_cast<AST::IntLiteral *>(&lit)) {
    lit.type = { Type::INT32 };
  } else if (dynamic_cast<AST::FloatLiteral *>(&lit)) {
    lit.type = { Type::FLOAT32 };
  } else if (dynamic_cast<AST::BoolLiteral *>(&lit)) {
    lit.type = { Type::BOOL };
  } else if (dynamic_cast<AST::StringLiteral *>(&lit)) {
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
    case AST::BinaryOp::EQUALS:
    case AST::BinaryOp::NOT_EQUALS:
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

static Type getUnaryOpType(AST::UnaryOp op, Type t) {
  switch (op) {
    case AST::UnaryOp::PLUS:
    case AST::UnaryOp::MINUS:
      if (!t.isNumOrVec()) {
        return { Type::INVALID };
      }
      return t;
    case AST::UnaryOp::LOGICAL_NOT:
      if (t != Type::BOOL) {
        return { Type::INVALID };
      }
      return t;
    case AST::UnaryOp::BITWISE_NOT:
      if (t != Type::INT32) {
        return { Type::INVALID };
      }
      return t;
  }
  assert(0);
}

void AnalysisVisitor::leave(AST::BinaryOpExpression &expr) {
  expr.type = getBinaryOpType(expr.op, expr.left->type, expr.right->type);
  if (expr.type.isInvalid()) {
    err << "Type mismatch (" << expr.left->type << " "
        << getBinaryOpSigil(expr.op) << " " << expr.right->type << ")" << expr.loc;
    return;
  }

  // Normalize scalar * vector to vector * scalar
  if (expr.op == AST::BinaryOp::MUL && expr.right->type.isVec()) {
    std::swap(expr.left, expr.right);
  }
};

void AnalysisVisitor::leave(AST::UnaryOpExpression &expr) {
  expr.type = getUnaryOpType(expr.op, expr.expr->type);
  if (expr.type.isInvalid()) {
    err << "Type mismatch: Applying unary operator \"" << getUnaryOpSigil(expr.op)
        << "\" to " << expr.expr->type << expr.loc;
  }
};

void AnalysisVisitor::leave(AST::TernaryExpression &expr) {
  Type ifType = expr.ifExpr->type;
  Type elseType = expr.elseExpr->type;
  if (ifType.isPromotableTo(elseType)) {
    expr.type = elseType;
  } else if (elseType.isPromotableTo(ifType)) {
    expr.type = ifType;
  } else {
    err << "Branches of ternary operator have divergent types "
        << ifType << " and " << elseType << expr.loc;
  }
};

static AST::AgentMember *findAgentMember(AST::AgentDeclaration &decl, const std::string &name) {
  for (AST::AgentMemberPtr &member : *decl.members) {
    if (member->name == name) {
      return &*member;
    }
  }
  return nullptr;
}

void AnalysisVisitor::leave(AST::AgentCreationExpression &expr) {
  auto it = agents.find(expr.name);
  if (it == agents.end()) {
    err << "Unknown agent type \"" << expr.name << "\"" << expr.loc;
    return;
  }

  AST::AgentDeclaration &agent = *it->second;
  for (AST::MemberInitEntryPtr &entry : *expr.members) {
    AST::AgentMember *member = findAgentMember(agent, entry->name);
    if (!member) {
      err << "Agent has no member \"" << entry->name << "\"" << entry->loc;
      return;
    }

    Type memberType = member->type->resolved;
    Type exprType = entry->expr->type;
    if (!exprType.isPromotableTo(memberType)) {
      err << "Trying to initialize member of type " << memberType
          << " from expression of type " << exprType << entry->expr->loc;
      return;
    }

    expr.memberMap.insert({ entry->name, &*entry->expr });
  }

  // Make sure all members were initialized
  for (AST::AgentMemberPtr &member : *agent.members) {
    if (expr.memberMap.find(member->name) == expr.memberMap.end()) {
      err << "Agent member \"" << member->name
          << "\" has not been initialized" << expr.loc;
      return;
    }
  }

  expr.type = { Type::AGENT, &agent };
};

void AnalysisVisitor::leave(AST::NewArrayExpression &expr) {
  Type elemType = expr.elemType->resolved;
  if (elemType.isArray()) {
    err << "Cannot instantiate nested array type" << expr.elemType->loc;
    return;
  }

  expr.type = { Type::ARRAY, elemType };
};

static bool isValidVecMember(Type type, const std::string &name) {
  if (name == "x" || name == "y") {
    return true;
  }
  if (type.getTypeId() == Type::VEC3 && name == "z") {
    return true;
  }
  return false;
}

void AnalysisVisitor::leave(AST::MemberAccessExpression &expr) {
  const std::string &name = expr.member;
  Type type = expr.expr->type;
  if (type.isVec()) {
    if (!isValidVecMember(type, name)) {
      err << "Vector has no member \"" << name << "\"" << expr.loc;
      return;
    }

    expr.type = Type::FLOAT32;
  } else if (type.isAgent()) {
    AST::AgentDeclaration *agent = type.getAgentDecl();
    AST::AgentMember *member = findAgentMember(*agent, name);
    if (!member) {
      err << "Agent has no member \"" << name << "\"" << expr.loc;
      return;
    }

    expr.type = member->type->resolved;
  } else {
    err << "Can only access members on agent or vector type" << expr.loc;
    return;
  }

  // Record member access, if necessary
  if (AST::VarExpression *varExpr = dynamic_cast<AST::VarExpression *>(&*expr.expr)) {
    if (collectAccessVar == varExpr->var->id) {
      currentFunc->accessedMembers.insert(name);
    }
  }
};

static std::vector<Type> getArgTypes(AST::CallExpression &expr) {
  std::vector<Type> types;
  for (AST::ArgPtr &arg : *expr.args) {
    types.push_back(arg->expr->type);
    // TODO out types?
  }
  return types;
}

static bool areAllPromotableTo(const std::vector<Type> &types, Type type) {
  for (Type t : types) {
    if (!t.isPromotableTo(type)) {
      return false;
    }
  }
  return true;
}

static bool isTypeCtorValid(Type t, const std::vector<Type> &argTypes) {
  size_t numArgs = argTypes.size();
  switch (t.getTypeId()) {
    case Type::VOID:
      return false;
    case Type::BOOL:
    case Type::INT32:
    case Type::FLOAT32:
      return numArgs == 1; // TODO
    case Type::STRING:
      return numArgs == 1 && argTypes[0] == Type::STRING;
    case Type::VEC2:
      if (numArgs != 1 && numArgs != 2) {
        return false;
      }
      return areAllPromotableTo(argTypes, Type::FLOAT32);
    case Type::VEC3:
      if (numArgs != 1 && numArgs != 3) {
        return false;
      }
      return areAllPromotableTo(argTypes, Type::FLOAT32);
    default:
      assert(0);
  }
}

static void printArgs(ErrorStream &err, const std::vector<Type> &argTypes) {
  bool first = true;
  err << "(";
  for (const Type &argType : argTypes) {
    if (!first) err << ", ";
    err << argType;
    first = false;
  }
  err << ")";
}

void AnalysisVisitor::leave(AST::CallExpression &expr) {
  std::vector<Type> argTypes = getArgTypes(expr);

  Type t = tryResolveNameToSimpleType(expr.name);
  if (!t.isInvalid()) {
    if (!isTypeCtorValid(t, argTypes)) {
      err << "Type constructor called with invalid arguments: " << expr.name;
      printArgs(err, argTypes);
      err << expr.loc;
      return;
    }

    expr.kind = AST::CallExpression::Kind::CTOR;
    expr.type = t;
    return;
  }

  BuiltinFunction *f = builtins.getByName(expr.name);
  if (f) {
    const FunctionSignature *sig = f->getCompatibleSignature(argTypes);
    if (!sig) {
      err << "Builtin function called with invalid arguments: " << expr.name;
      printArgs(err, argTypes);
      err << expr.loc;
      return;
    }

    expr.kind = AST::CallExpression::Kind::BUILTIN;
    expr.calledSig = sig->getConcreteSignature(argTypes);
    expr.type = expr.calledSig.returnType;
    return;
  }

  auto it = funcs.find(expr.name);
  if (it == funcs.end()) {
    err << "Call to unknown function \"" << expr.name << "\"" << expr.loc;
    return;
  }

  // TODO check sig, verify, handle
  expr.kind = AST::CallExpression::Kind::USER;
};

void AnalysisVisitor::leave(AST::Script &script) {
  for (AST::AgentDeclaration *agent : script.agents) {
    AST::AgentMember *member = agent->getPositionMember();
    if (member) {
      if (!script.envDecl || !script.envDecl->sizeExpr) {
        err << "An environment { size } declaration is required to use position members"
            << member->loc;
        return;
      }
      if (member->type->resolved != script.envDecl->sizeExpr->type) {
        err << "Dimensionality of position member does not match environment dimension"
            << member->loc;
        return;
      }
    }
  }
};

}

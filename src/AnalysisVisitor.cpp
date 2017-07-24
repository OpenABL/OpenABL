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
  } else {
    assert(0);
  }
}

void AnalysisVisitor::declareVar(
    AST::Var &var, Type type, bool isConst, bool isGlobal, Value val) {
  auto it = varMap.find(var.name);
  if (it != varMap.end()) {
    const ScopeEntry &entry = scope.get(it->second);
    // Allow a local variable to shadow a global variable, but nothing else
    if (!entry.isGlobal || isGlobal) {
      err << "Cannot redeclare variable \"" << var.name << "\"" << var.loc;
      return;
    }
  }

  var.id = VarId::make();
  varMap.insert({ var.name, var.id });
  scope.add(var.id, type, isConst, isGlobal, val);
}

void AnalysisVisitor::pushVarScope() {
  varMapStack.push(varMap);
};
void AnalysisVisitor::popVarScope() {
  assert(!varMapStack.empty());
  varMap = varMapStack.top();
  varMapStack.pop();
};

bool promoteTo(AST::ExpressionPtr &expr, Type type) {
  if (expr->type == type) {
    return true;
  }

  if (expr->type.isInt() && type.isFloat()) {
    if (const auto *lit = dynamic_cast<AST::IntLiteral *>(&*expr)) {
      // Convert to float literal
      expr.reset(new AST::FloatLiteral((double) lit->value, lit->loc));
    } else {
      // Insert cast expression
      auto *origExpr = expr.release();
      auto *args = new AST::ExpressionList();
      args->emplace_back(origExpr);
      auto *cast = new AST::CallExpression("float", args, origExpr->loc);
      cast->kind = AST::CallExpression::Kind::CTOR;
      cast->type = Type::FLOAT32;
      expr.reset(cast);
    }
    return true;
  }

  return false;
}

void AnalysisVisitor::enter(AST::Var &) {}
void AnalysisVisitor::enter(AST::Literal &) {}
void AnalysisVisitor::enter(AST::UnaryOpExpression &) {}
void AnalysisVisitor::enter(AST::BinaryOpExpression &) {}
void AnalysisVisitor::enter(AST::CallExpression &) {}
void AnalysisVisitor::enter(AST::MemberAccessExpression &) {}
void AnalysisVisitor::enter(AST::ArrayAccessExpression &) {}
void AnalysisVisitor::enter(AST::TernaryExpression &) {}
void AnalysisVisitor::enter(AST::MemberInitEntry &) {}
void AnalysisVisitor::enter(AST::AgentCreationExpression &) {}
void AnalysisVisitor::enter(AST::ArrayInitExpression &) {}
void AnalysisVisitor::enter(AST::NewArrayExpression &) {}
void AnalysisVisitor::enter(AST::ExpressionStatement &) {}
void AnalysisVisitor::enter(AST::AssignStatement &) {}
void AnalysisVisitor::enter(AST::AssignOpStatement &) {}
void AnalysisVisitor::enter(AST::IfStatement &) {}
void AnalysisVisitor::enter(AST::WhileStatement &) {}
void AnalysisVisitor::enter(AST::SimulateStatement &) {}
void AnalysisVisitor::enter(AST::ReturnStatement &) {}
void AnalysisVisitor::enter(AST::AgentMember &) {}
void AnalysisVisitor::enter(AST::Script &) {}
void AnalysisVisitor::enter(AST::VarExpression &) {}
void AnalysisVisitor::enter(AST::VarDeclarationStatement &) {}
void AnalysisVisitor::enter(AST::Param &) {}
void AnalysisVisitor::enter(AST::EnvironmentDeclaration &) {}
void AnalysisVisitor::leave(AST::Var &) {}
void AnalysisVisitor::leave(AST::MemberInitEntry &) {}
void AnalysisVisitor::leave(AST::ArrayInitExpression &) {}
void AnalysisVisitor::leave(AST::ExpressionStatement &) {}
void AnalysisVisitor::leave(AST::SimpleType &) {}
void AnalysisVisitor::leave(AST::AgentMember &) {}
void AnalysisVisitor::leave(AST::AgentDeclaration &) {}

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

static bool isConstantExpression(const AST::Expression &expr) {
  if (dynamic_cast<const AST::Literal *>(&expr)) {
    return true;
  } else if (dynamic_cast<const AST::UnaryOpExpression *>(&expr)) {
    return true;
  } else if (auto *arr = dynamic_cast<const AST::ArrayInitExpression *>(&expr)) {
    for (const AST::ExpressionPtr &expr : *arr->exprs) {
      if (!isConstantExpression(*expr)) {
        return false;
      }
    }
    return true;
  }

  // TODO Other expressions
  return false;
}

Value AnalysisVisitor::evalExpression(const AST::Expression &expr) {
  // Literals
  if (auto *blit = dynamic_cast<const AST::BoolLiteral *>(&expr)) {
    return { blit->value };
  }
  if (auto *ilit = dynamic_cast<const AST::IntLiteral *>(&expr)) {
    return { ilit->value };
  }
  if (auto *flit = dynamic_cast<const AST::FloatLiteral *>(&expr)) {
    return { flit->value };
  }
  if (auto *slit = dynamic_cast<const AST::StringLiteral *>(&expr)) {
    return { slit->value };
  }

  // Access to named constants
  if (auto *var = dynamic_cast<const AST::VarExpression *>(&expr)) {
    VarId id = var->var->id;
    if (!scope.has(id)) {
      return {};
    }

    ScopeEntry entry = scope.get(id);
    return entry.val;
  }

  // Casts and type constructors
  if (auto *call = dynamic_cast<const AST::CallExpression *>(&expr)) {
    if (!call->isCtor()) {
      return {};
    }

    switch (call->type.getTypeId()) {
      case Type::BOOL:
        return evalExpression(call->getArg(0)).toBoolExplicit();
      case Type::INT32:
        return evalExpression(call->getArg(0)).toIntExplicit();
      case Type::FLOAT32:
        return evalExpression(call->getArg(0)).toFloatExplicit();
      case Type::VEC2:
        if (call->getNumArgs() == 1) {
          Value v = evalExpression(call->getArg(0)).toFloatImplicit();
          if (v.isInvalid()) {
            return {};
          }
          return { v.getFloat(), v.getFloat() };
        } else {
          Value v1 = evalExpression(call->getArg(0)).toFloatImplicit();
          Value v2 = evalExpression(call->getArg(1)).toFloatImplicit();
          if (v1.isInvalid() || v2.isInvalid()) {
            return {};
          }
          return { v1.getFloat(), v2.getFloat() };
        }
      case Type::VEC3:
        if (call->getNumArgs() == 1) {
          Value v = evalExpression(call->getArg(0)).toFloatImplicit();
          if (v.isInvalid()) {
            return {};
          }
          return { v.getFloat(), v.getFloat(), v.getFloat() };
        } else {
          Value v1 = evalExpression(call->getArg(0)).toFloatImplicit();
          Value v2 = evalExpression(call->getArg(1)).toFloatImplicit();
          Value v3 = evalExpression(call->getArg(1)).toFloatImplicit();
          if (v1.isInvalid() || v2.isInvalid() || v3.isInvalid()) {
            return {};
          }
          return { v1.getFloat(), v2.getFloat(), v3.getFloat() };
        }
      default:
        return {};
    }
  }

  // Unary expression
  if (auto *unary = dynamic_cast<const AST::UnaryOpExpression *>(&expr)) {
    Value v = evalExpression(*unary->expr);
    if (v.isInvalid()) {
      return {};
    }
    return Value::calcUnaryOp(unary->op, v);
  }

  // Binary expression
  if (auto *binary = dynamic_cast<const AST::BinaryOpExpression *>(&expr)) {
    Value l = evalExpression(*binary->left);
    Value r = evalExpression(*binary->right);
    if (l.isInvalid() || r.isInvalid()) {
      return {};
    }
    return Value::calcBinaryOp(binary->op, l, r);
  }
  return {};
}

static bool handleArrayInitializer(ErrorStream &err, AST::Expression &expr, Type elemType) {
  const auto *init = dynamic_cast<const AST::ArrayInitExpression *>(&expr);
  if (!init) {
    err << "Array must be initialized using array initializer" << expr.loc;
    return false;
  }

  for (AST::ExpressionPtr &expr : *init->exprs) {
    if (!promoteTo(expr, elemType)) {
      err << "Element of type " << expr->type
          << " inside initializer for array of type " << elemType << expr->loc;
      return false;
    }
  }

  expr.type = { Type::ARRAY, elemType };
  return true;
}

void AnalysisVisitor::leave(AST::ConstDeclaration &decl) {
  Value val;
  if (decl.isArray) {
    Type elemType = decl.type->resolved;
    decl.type->resolved = { Type::ARRAY, elemType };

    if (!handleArrayInitializer(err, *decl.expr, elemType)) {
      return;
    }

    if (!isConstantExpression(*decl.expr)) {
      err << "Initializer of global constant must be a constant expression" << decl.expr->loc;
      return;
    }

    val = {};
  } else {
    val = evalExpression(*decl.expr);
    if (val.isInvalid()) {
      err << "Initializer of global constant must be a constant expression" << decl.expr->loc;
      return;
    }
  }

  Type declType = decl.type->resolved;
  if (!promoteTo(decl.expr, declType)) {
    err << "Trying to assign value of type " << decl.expr->type
        << " to global of type " << declType << decl.expr->loc;
    return;
  }

  auto it = params.find(decl.var->name);
  if (it != params.end()) {
    // Parameter was overwritten on the CLI
    if (!decl.isParam) {
      err << "Only constants marked with \"param\" can be specified as parameters"
          << decl.var->loc;
      return;
    }

    val = Value::fromString(it->second);
    if (val.isInvalid()) {
      err << "Value \"" << it->second << "\" provided for parameter \"" << decl.var->name
          << "\" could not parsed" << decl.var->loc;
      return;
    }

    if (!val.getType().isPromotableTo(declType)) {
      err << "Provided parameter \"" << decl.var->name << "\" is of type " << val.getType()
          << ", but " << declType << " expected" << decl.var->loc;
      return;
    }

    // Update initializer expression, as that's what the pretty printers are using right now
    decl.expr.reset(val.toExpression());
  }

  declareVar(*decl.var, decl.type->resolved, true, true, val);
};

void AnalysisVisitor::leave(AST::EnvironmentDeclaration &decl) {
  if (script.envDecl) {
    err << "Script can only contain a single environment specification" << decl.loc;
    return;
  }

  for (const AST::MemberInitEntryPtr &member : *decl.members) {
    if (member->name == "min" || member->name == "max") {
      const AST::Expression &boundsExpr = *member->expr;
      Value v = evalExpression(boundsExpr);
      if (v.isInvalid()) {
        err << "Environment bounds must be a constant expression" << boundsExpr.loc;
        return;
      }
      if (!boundsExpr.type.isVec()) {
        err << "Environment bounds must be float2 or float3" << boundsExpr.loc;
        return;
      }
      if (member->name == "min") {
        decl.envMin = v;
      } else {
        decl.envMax = v;
      }
    } else {
      err << "Unknown environment member \"" << member->name << "\"" << member->loc;
      return;
    }
  }

  if (decl.envMax.isValid()) {
    decl.envDimension = decl.envMax.getType().getVecLen();
    if (decl.envMin.isValid()) {
      if (decl.envMax.getType() != decl.envMin.getType()) {
        err << "min and max environment bounds must have the same type" << decl.loc;
        return;
      }
    } else {
      // Default environment minimum
      if (decl.envDimension == 2) {
        decl.envMin = { 0, 0 };
      } else {
        decl.envMin = { 0, 0, 0 };
      }
    }

    decl.envSize = Value::calcBinaryOp(AST::BinaryOp::SUB, decl.envMax, decl.envMin);

    for (double coord : decl.envSize.getVec()) {
      if (coord < 0) {
        err << "Environment minimum should be smaller or equal than the maximum"
            << decl.loc;
        return;
      }
    }
  }

  script.envDecl = &decl;
};

void AnalysisVisitor::leave(AST::VarDeclarationStatement &decl) {
  declareVar(*decl.var, decl.type->resolved, false, false, {});
  if (decl.initializer) {
    Type declType = decl.type->resolved;
    Type initType = decl.initializer->type;
    SKIP_INVALID(declType);
    SKIP_INVALID(initType);

    if (!promoteTo(decl.initializer, declType)) {
      err << "Trying to assign value of type " << initType
          << " to variable of type " << declType << decl.initializer->loc;
    }
  }
};

void AnalysisVisitor::leave(AST::Param &param) {
  // If an in -> out structgure is used, "in" is immutable
  bool isImmutable = param.outVar != nullptr;
  declareVar(*param.var, param.type->resolved, isImmutable, false, {});
  if (param.outVar) {
    declareVar(*param.outVar, param.type->resolved, false, false, {});
  }
}

void AnalysisVisitor::enter(AST::ForStatement &stmt) {
  pushVarScope();

  Type declType = resolveAstType(*stmt.type); // Not resolved yet
  declareVar(*stmt.var, declType, true, false, {});

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

void AnalysisVisitor::leave(AST::IfStatement &stmt) {
  Type t = stmt.condExpr->type;
  if (!t.isBool()) {
    err << "if() condition must be bool, but received " << t << stmt.condExpr->loc;
    return;
  }
}

void AnalysisVisitor::leave(AST::WhileStatement &stmt) {
  Type t = stmt.expr->type;
  if (!t.isBool()) {
    err << "while() condition must be bool, but received " << t << stmt.expr->loc;
    return;
  }
}

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

  if (!stmt.timestepsExpr->type.isInt()) {
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

    if (!promoteTo(stmt.expr, returnType)) {
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

static bool promoteBinary(AST::ExpressionPtr &left, AST::ExpressionPtr &right) {
  return promoteTo(left, right->type) || promoteTo(right, left->type);
}

static Type getBinaryOpType(
    AST::BinaryOp op, AST::ExpressionPtr &left, AST::ExpressionPtr &right) {
  Type l = left->type;
  Type r = right->type;
  switch (op) {
    case AST::BinaryOp::ADD:
    case AST::BinaryOp::SUB:
      if (!l.isNumOrVec() || !r.isNumOrVec()) {
        return { Type::INVALID };
      }
      if (l != r) {
        if (promoteBinary(left, right)) {
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
        promoteTo(right, Type::FLOAT32);
        return l;
      }
      if (r.isVec()) {
        if (op == AST::BinaryOp::DIV) {
          return { Type::INVALID };
        }
        promoteTo(left, Type::FLOAT32);
        return r;
      }
      if (l != r) {
        if (promoteBinary(left, right)) {
          return { Type::FLOAT32 };
        } else {
          return { Type::INVALID };
        }
      }
      return l;
    case AST::BinaryOp::MOD:
    case AST::BinaryOp::BITWISE_OR:
    case AST::BinaryOp::BITWISE_AND:
    case AST::BinaryOp::BITWISE_XOR:
    case AST::BinaryOp::SHIFT_LEFT:
    case AST::BinaryOp::SHIFT_RIGHT:
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
    case AST::BinaryOp::LOGICAL_OR:
    case AST::BinaryOp::LOGICAL_AND:
      if (!l.isBool() || !r.isBool()) {
        return { Type::INVALID };
      }
      return { Type:: BOOL };
    case AST::BinaryOp::RANGE:
      if (!l.isInt() || !r.isInt()) {
        return { Type::INVALID };
      }
      return { Type::ARRAY, Type::INT32 };
  }
  assert(0);
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
  SKIP_INVALID(expr.left->type);
  SKIP_INVALID(expr.right->type);

  expr.type = getBinaryOpType(expr.op, expr.left, expr.right);
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
  SKIP_INVALID(expr.expr->type);

  expr.type = getUnaryOpType(expr.op, expr.expr->type);
  if (expr.type.isInvalid()) {
    err << "Type mismatch: Applying unary operator \"" << getUnaryOpSigil(expr.op)
        << "\" to " << expr.expr->type << expr.loc;
  }
};

void AnalysisVisitor::leave(AST::TernaryExpression &expr) {
  Type ifType = expr.ifExpr->type;
  Type elseType = expr.elseExpr->type;
  SKIP_INVALID(ifType);
  SKIP_INVALID(elseType);

  if (promoteTo(expr.ifExpr, elseType)) {
    expr.type = elseType;
  } else if (promoteTo(expr.elseExpr, ifType)) {
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
    if (!promoteTo(entry->expr, memberType)) {
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

void AnalysisVisitor::leave(AST::ArrayAccessExpression &expr) {
  Type arrayT = expr.arrayExpr->type;
  if (!arrayT.isArray()) {
    err << "Can only index into arrays" << expr.arrayExpr->loc;
    return;
  }

  Type offsetT = expr.offsetExpr->type;
  if (!offsetT.isInt()) {
    err << "Array offset must be an integer" << expr.offsetExpr->loc;
    return;
  }

  expr.type = arrayT.getBaseType();
}

static std::vector<Type> getArgTypes(const AST::CallExpression &expr) {
  std::vector<Type> types;
  for (AST::ExpressionPtr &arg : *expr.args) {
    types.push_back(arg->type);
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
    {
      if (numArgs != 1) {
        return false;
      }

      Type argType = argTypes[0];
      return argType.isBool() || argType.isNum();
    }
    case Type::STRING:
      // Don't allow explicitly string casts
      return false;
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

    // FlameGPU needs to know
    if (expr.name == "random") {
      currentFunc->usesRng = true;
    }
    return;
  }

  auto it = funcs.find(expr.name);
  if (it == funcs.end()) {
    err << "Call to unknown function \"" << expr.name << "\"" << expr.loc;
    return;
  }

  const AST::FunctionDeclaration *decl = it->second;
  if (expr.args->size() != decl->params->size()) {
    err << "Function " << decl->name << " expects " << decl->params->size()
        << " parameters, but " << expr.args->size() << " were given" << expr.loc;
    return;
  }

  size_t num = expr.args->size();
  for (size_t i = 0; i < num; i++) {
    AST::ExpressionPtr &arg = (*expr.args)[i];
    const AST::Param &param = *(*decl->params)[i];
    if (!promoteTo(arg, param.type->resolved)) {
      err << "Argument " << i << " to function " << decl->name << " has type "
          << arg->type << " but " << param.type->resolved << " expected" << arg->loc;
      return;
    }
  }

  expr.kind = AST::CallExpression::Kind::USER;
  expr.type = decl->returnType->resolved;
  expr.calledFunc = decl;

  // We might be using an RNG indirectly
  if (decl->usesRng) {
    currentFunc->usesRng = true;
  }
};

void AnalysisVisitor::leave(AST::Script &script) {
  for (AST::AgentDeclaration *agent : script.agents) {
    AST::AgentMember *member = agent->getPositionMember();
    if (member) {
      if (!script.envDecl || !script.envDecl->hasEnvDimension()) {
        err << "An environment { } declaration is required to use position members"
            << member->loc;
        return;
      }
      if (member->type->resolved.getVecLen() != script.envDecl->getEnvDimension()) {
        err << "Dimensionality of position member does not match environment dimension"
            << member->loc;
        return;
      }
    }
  }

  if (!script.mainFunc) {
    err << "Script must have a main function" << AST::Location{};
    return;
  }
};

}

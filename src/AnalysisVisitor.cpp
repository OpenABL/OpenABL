#include "AnalysisVisitor.hpp"

namespace OpenABL {

Type AnalysisVisitor::resolveAstType(AST::Type &type) {
  if (AST::SimpleType *t = dynamic_cast<AST::SimpleType *>(&type)) {
    if (t->name == "void") {
      return { Type::VOID };
    } else if (t->name == "bool") {
      return { Type::BOOL };
    } else if (t->name == "int") {
      return { Type::INT32 };
    } else if (t->name == "float") {
      return { Type::FLOAT32 };
    } else if (t->name == "vec2") {
      return { Type::VEC2 };
    } else if (t->name == "vec3") {
      return { Type::VEC3 };
    } else {
      // TODO check existence
      return { Type::AGENT };
    }
  } else if (AST::ArrayType *t = dynamic_cast<AST::ArrayType *>(&type)) {
    Type baseType = resolveAstType(*t->type);
    if (baseType.isArray()) {
      std::cout << "Arrays may only be nested to one level" << std::endl;
      return { Type::INVALID };
    }

    return { Type::ARRAY, baseType.getTypeId() };
  } else {
    assert(0);
  }
}

VarId AnalysisVisitor::declareVar(std::string name, Type type) {
  auto it = varMap.find(name);
  if (it != varMap.end()) {
    std::cout << "Cannot redeclare " << name << std::endl;
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
void AnalysisVisitor::enter(AST::ExpressionStatement &) {};
void AnalysisVisitor::enter(AST::IfStatement &) {};
void AnalysisVisitor::enter(AST::SimpleType &) {};
void AnalysisVisitor::enter(AST::ArrayType &) {};
void AnalysisVisitor::enter(AST::AgentMember &) {};
void AnalysisVisitor::enter(AST::AgentDeclaration &) {};
void AnalysisVisitor::enter(AST::Script &) {};
void AnalysisVisitor::enter(AST::VarExpression &) {};
void AnalysisVisitor::leave(AST::Var &) {};
void AnalysisVisitor::leave(AST::Literal &) {};
void AnalysisVisitor::leave(AST::UnaryOpExpression &) {};
void AnalysisVisitor::leave(AST::AssignOpExpression &) {};
void AnalysisVisitor::leave(AST::AssignExpression &) {};
void AnalysisVisitor::leave(AST::Arg &) {};
void AnalysisVisitor::leave(AST::CallExpression &) {};
void AnalysisVisitor::leave(AST::MemberAccessExpression &) {};
void AnalysisVisitor::leave(AST::TernaryExpression &) {};
void AnalysisVisitor::leave(AST::ExpressionStatement &) {};
void AnalysisVisitor::leave(AST::VarDeclarationStatement &) {};
void AnalysisVisitor::leave(AST::IfStatement &) {};
void AnalysisVisitor::leave(AST::SimpleType &) {};
void AnalysisVisitor::leave(AST::ArrayType &) {};
void AnalysisVisitor::leave(AST::Param &) {};
void AnalysisVisitor::leave(AST::AgentMember &) {};
void AnalysisVisitor::leave(AST::AgentDeclaration &) {};
void AnalysisVisitor::leave(AST::ConstDeclaration &) {};
void AnalysisVisitor::leave(AST::Script &) {};

void AnalysisVisitor::enter(AST::FunctionDeclaration &) {
  pushVarScope();
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

void AnalysisVisitor::enter(AST::ConstDeclaration &decl) {
  Type type = resolveAstType(*decl.type);
  decl.var->id = declareVar(decl.var->name, type);
};

void AnalysisVisitor::enter(AST::VarDeclarationStatement &decl) {
  decl.var->id = declareVar(decl.var->name, resolveAstType(*decl.type));
}

void AnalysisVisitor::enter(AST::Param &param) {
  Type type = resolveAstType(*param.type);
  param.var->id = declareVar(param.var->name, type);
  if (param.outVar) {
    param.outVar->id = declareVar(param.outVar->name, type);
  }
}

void AnalysisVisitor::enter(AST::ForStatement &stmt) {
  pushVarScope();

  Type type = resolveAstType(*stmt.type);
  stmt.var->id = declareVar(stmt.var->name, type);
};
void AnalysisVisitor::leave(AST::ForStatement &) {
  popVarScope();
};

void AnalysisVisitor::enter(AST::ParallelForStatement &stmt) {
  pushVarScope();

  Type type = resolveAstType(*stmt.type);
  stmt.var->id = declareVar(stmt.var->name, type);
  stmt.outVar->id = declareVar(stmt.outVar->name, type);
};
void AnalysisVisitor::leave(AST::ParallelForStatement &) {
  popVarScope();
};

void AnalysisVisitor::leave(AST::VarExpression &expr) {
  AST::Var &var = *expr.var;
  auto it = varMap.find(var.name);
  if (it == varMap.end()) {
    std::cout << "Use of undeclared variable " << var.name << std::endl;
    return;
  }

  var.id = it->second;
  expr.type = scope.get(var.id).type;
}

static Type getBinaryOpType(AST::BinaryOp op, Type l, Type r) {
  switch (op) {
    case AST::BinaryOp::ADD:
    case AST::BinaryOp::SUB:
    case AST::BinaryOp::MUL:
    case AST::BinaryOp::DIV:
      if (l != r) {
        // Int to float promotion
        if (l.isFloat() && r.isInt()) {
          r = l;
        } else if (r.isFloat() && l.isInt()) {
          l = r;
        } else {
          std::cout << "Type mismatch (" << l << " "
                    << getBinaryOpSigil(op) << " " << r << ")" << std::endl;
          return { Type::INVALID };
        }
      }
      return l;
    case AST::BinaryOp::MOD:
      if (!l.isInt() || !r.isInt()) {
        std::cout << "Modulus requires integer operands" << std::endl;
        return { Type::INVALID };
      }
      return { Type::INT32 };
    default:
      return { Type::INVALID };
  }
}

void AnalysisVisitor::leave(AST::BinaryOpExpression &expr) {
  expr.type = getBinaryOpType(expr.op, expr.left->type, expr.right->type);
};

}

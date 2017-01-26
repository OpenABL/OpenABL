%skeleton "lalr1.cc"
%locations
%define api.token.constructor
%define api.value.type variant
%define api.namespace {OpenABL}
%define parser_class_name {Parser}
%define parse.error verbose
%param { OpenABL::ParserContext &ctx }

%code requires
{
#include "AST.hpp"

namespace OpenABL {
class ParserContext;
}

}

%{

#include "ParserContext.hpp"

using namespace OpenABL::AST;

OpenABL::Parser::symbol_type yylex(OpenABL::ParserContext &ctx);

%}

%token
  END

  AGENT
  INTERACT
  FOR
  POSITION
  PFOR

  ADD
  SUB
  MUL
  DIV
  MOD
  BITWISE_AND
  BITWISE_XOR
  BITWISE_OR
  SHIFT_LEFT
  SHIFT_RIGHT
  NOT
  QM
  DOT
  COMMA
  COLON
  SEMI
  LPAREN
  RPAREN
  LBRACKET
  RBRACKET
  LBRACE
  RBRACE
  ASSIGN
  ARROW
  EQUALS
  NOT_EQUALS
  SMALLER_EQUALS
  GREATER_EQUALS
  LOGICAL_AND
  LOGICAL_OR
  ADD_ASSIGN
  SUB_ASSIGN
  MUL_ASSIGN
  DIV_ASSIGN
  MOD_ASSIGN
  BITWISE_AND_ASSIGN
  BITWISE_XOR_ASSIGN
  BITWISE_OR_ASSIGN
  SHIFT_LEFT_ASSIGN
  SHIFT_RIGHT_ASSIGN

  INT
  FLOAT
  IDENTIFIER
;

/* Precedence following C */
%left MUL DIV MOD
%left ADD SUB
%left SHIFT_LEFT SHIFT_RIGHT
%left SMALLER SMALLER_EQUALS GREATER GREATER_EQUALS
%left EQUALS NOT_EQUALS
%left BITWISE_AND
%left BITWISE_XOR
%left BITWISE_OR
%left LOGICAL_AND
%left LOGICAL_OR
/* TODO: Ternary */
%left ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN BITWISE_AND_ASSIGN BITWISE_XOR_ASSIGN BITWISE_OR_ASSIGN SHIFT_LEFT_ASSIGN SHIFT_RIGHT_ASSIGN

%type <std::string> IDENTIFIER;
%type <bool> opt_position;
%type <OpenABL::AST::Type *> type;
%type <OpenABL::AST::AgentMember *> agent_member;
%type <OpenABL::AST::AgentMemberList *> agent_member_list;
%type <OpenABL::AST::Param *> param;
%type <OpenABL::AST::ParamList *> param_list;
%type <OpenABL::AST::StatementList *> statement_list;
%type <OpenABL::AST::Declaration *> declaration func_decl agent_decl;
%type <OpenABL::AST::DeclarationList *> declaration_list;
%type <OpenABL::AST::Expression *> expression;
%type <OpenABL::AST::Statement *> statement;

%%

start: declaration_list { ctx.script = new Script($1, @$); }

declaration_list: %empty { $$ = new DeclarationList(); }
                | declaration_list declaration { $1->emplace_back($2); $$ = $1; };

declaration: agent_decl
           | func_decl;

agent_decl: AGENT IDENTIFIER LBRACE agent_member_list RBRACE
		      { $$ = new AgentDeclaration($2, $4, @$); };

agent_member_list: %empty { $$ = new AgentMemberList(); }
				 | agent_member_list agent_member { $1->emplace_back($2); $$ = $1; };

opt_position: %empty { $$ = false; }
			| POSITION { $$ = true; }

agent_member: opt_position type IDENTIFIER SEMI { $$ = new AgentMember($1, $2, $3, @$); };

func_decl: type IDENTIFIER LPAREN param_list RPAREN LBRACE statement_list RBRACE
             { $$ = new FunctionDeclaration($1, $2, $4, $7, @$); };

param_list: %empty { $$ = new ParamList(); }
          | param_list param { $1->emplace_back($2); $$ = $1; };

param: type IDENTIFIER { $$ = new Param($1, $2, @$); };

type: IDENTIFIER { $$ = new Type($1, @$); };

statement_list: %empty { $$ = new StatementList(); }
              | statement_list statement { $1->emplace_back($2); $$ = $1; };

statement: expression SEMI { $$ = new ExpressionStatement($1, @$); }
         ;

expression: IDENTIFIER { $$ = new VarExpression($1, @$); }
          | LPAREN expression RPAREN { $$ = $2; }

          | expression ADD expression
              { $$ = new BinaryOpExpression(BinaryOp::ADD, $1, $3, @$); }
          | expression SUB expression
              { $$ = new BinaryOpExpression(BinaryOp::SUB, $1, $3, @$); }
          | expression MUL expression
              { $$ = new BinaryOpExpression(BinaryOp::MUL, $1, $3, @$); }
          | expression DIV expression
              { $$ = new BinaryOpExpression(BinaryOp::DIV, $1, $3, @$); }
          | expression MOD expression
              { $$ = new BinaryOpExpression(BinaryOp::MOD, $1, $3, @$); }
          | expression BITWISE_AND expression
              { $$ = new BinaryOpExpression(BinaryOp::BITWISE_AND, $1, $3, @$); }
          | expression BITWISE_XOR expression
              { $$ = new BinaryOpExpression(BinaryOp::BITWISE_XOR, $1, $3, @$); }
          | expression BITWISE_OR expression
              { $$ = new BinaryOpExpression(BinaryOp::BITWISE_OR, $1, $3, @$); }
          | expression SHIFT_LEFT expression
              { $$ = new BinaryOpExpression(BinaryOp::SHIFT_LEFT, $1, $3, @$); }
          | expression SHIFT_RIGHT expression
              { $$ = new BinaryOpExpression(BinaryOp::SHIFT_RIGHT, $1, $3, @$); }
          | expression EQUALS expression
              { $$ = new BinaryOpExpression(BinaryOp::EQUALS, $1, $3, @$); }
          | expression NOT_EQUALS expression
              { $$ = new BinaryOpExpression(BinaryOp::NOT_EQUALS, $1, $3, @$); }
          | expression SMALLER expression
              { $$ = new BinaryOpExpression(BinaryOp::SMALLER, $1, $3, @$); }
          | expression SMALLER_EQUALS expression
              { $$ = new BinaryOpExpression(BinaryOp::SMALLER_EQUALS, $1, $3, @$); }
          | expression GREATER expression
              { $$ = new BinaryOpExpression(BinaryOp::GREATER, $1, $3, @$); }
          | expression GREATER_EQUALS expression
              { $$ = new BinaryOpExpression(BinaryOp::GREATER_EQUALS, $1, $3, @$); }
          | expression LOGICAL_AND expression
              { $$ = new BinaryOpExpression(BinaryOp::LOGICAL_AND, $1, $3, @$); }
          | expression LOGICAL_OR expression
              { $$ = new BinaryOpExpression(BinaryOp::LOGICAL_OR, $1, $3, @$); }

          | expression ASSIGN expression
              { $$ = new AssignExpression($1, $3, @$); }
          | expression ADD_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::ADD, $1, $3, @$); }
          | expression SUB_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::SUB, $1, $3, @$); }
          | expression MUL_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::MUL, $1, $3, @$); }
          | expression DIV_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::DIV, $1, $3, @$); }
          | expression MOD_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::MOD, $1, $3, @$); }
          | expression BITWISE_AND_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::BITWISE_AND, $1, $3, @$); }
          | expression BITWISE_XOR_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::BITWISE_XOR, $1, $3, @$); }
          | expression BITWISE_OR_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::BITWISE_OR, $1, $3, @$); }
          | expression SHIFT_LEFT_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::SHIFT_LEFT, $1, $3, @$); }
          | expression SHIFT_RIGHT_ASSIGN expression
              { $$ = new AssignOpExpression(BinaryOp::SHIFT_RIGHT, $1, $3, @$); }
          ;
%%

void OpenABL::Parser::error(const OpenABL::location &loc, const std::string &message) {
  std::cerr << "Parse error: " << message << std::endl;
}

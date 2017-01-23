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
namespace OpenABL {
class ParserContext;
}
}

%{

#include "AST.hpp"
#include "ParserContext.hpp"

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
  SHIFT_LEFT
  SHIFT_RIGHT
  LOGICAL_AND
  LOGICAL_OR

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
%left ASSIGN PLUS_ASSIGN MINUS_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN BITWISE_AND_ASSIGN BITWISE_XOR_ASSIGN BITWISE_OR_ASSIGN SHIFT_LEFT_ASSIGN SHIFT_RIGHT_ASSIGN

%%

start: declaration_list;

declaration_list: %empty
				| declaration_list declaration;

declaration: agent_decl
		   | func_decl;

agent_decl: AGENT IDENTIFIER LBRACE RBRACE;

func_decl: type IDENTIFIER LPAREN param_list RPAREN LBRACE statement_list RBRACE;

param_list: %empty
		  | param_list param;

param: type IDENTIFIER
	 | type IDENTIFIER ARROW IDENTIFIER;

type: IDENTIFIER;

statement_list: %empty
			  | statement_list statement;

statement: expression SEMI;

expression: IDENTIFIER
		  | LPAREN expression RPAREN

		  | expression ADD expression
		  | expression SUB expression
		  | expression MUL expression
		  | expression DIV expression
		  | expression MOD expression
		  | expression BITWISE_AND expression
		  | expression BITWISE_XOR expression
		  | expression BITWISE_OR expression
		  | expression SHIFT_LEFT expression
		  | expression SHIFT_RIGHT expression
		  | expression EQUALS expression
		  | expression NOT_EQUALS expression
		  | expression SMALLER expression
		  | expression SMALLER_EQUALS expression
		  | expression GREATER expression
		  | expression GREATER_EQUALS expression
		  | expression LOGICAL_AND expression
		  | expression LOGICAL_OR expression

		  | expression ASSIGN expression
		  | expression PLUS_ASSIGN expression
		  | expression MINUS_ASSIGN expression
		  | expression MUL_ASSIGN expression
		  | expression DIV_ASSIGN expression
		  | expression MOD_ASSIGN expression
		  | expression BITWISE_AND_ASSIGN expression
		  | expression BITWISE_XOR_ASSIGN expression
		  | expression BITWISE_OR_ASSIGN expression
		  | expression SHIFT_LEFT_ASSIGN expression
		  | expression SHIFT_RIGHT_ASSIGN expression;
%%

void OpenABL::Parser::error(const OpenABL::location &loc, const std::string &message) {
}

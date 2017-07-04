%skeleton "lalr1.cc"
%locations
%expect 0
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
struct ParserContext;
}

}

%{

#include "ParserContext.hpp"

using namespace OpenABL::AST;

OpenABL::Parser::symbol_type yylex(OpenABL::ParserContext &ctx);

%}

%token
  END 0 "end of file"

  AGENT
  ELSE
  ENVIRONMENT
  IF
  FOR
  NEW
  POSITION
  RETURN
  SIMULATE
  WHILE

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
  BITWISE_NOT
  QM
  DOT
  DOTDOT
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

  BOOL
  INT
  FLOAT
  STRING
  IDENTIFIER
;

/* Precedence following C */
%left ASSIGN ADD_ASSIGN SUB_ASSIGN MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN BITWISE_AND_ASSIGN BITWISE_XOR_ASSIGN BITWISE_OR_ASSIGN SHIFT_LEFT_ASSIGN SHIFT_RIGHT_ASSIGN
%right QM COLON
%left LOGICAL_OR
%left LOGICAL_AND
%left BITWISE_OR
%left BITWISE_XOR
%left BITWISE_AND
%left EQUALS NOT_EQUALS
%left SMALLER SMALLER_EQUALS GREATER GREATER_EQUALS
%nonassoc DOTDOT /* TODO: What is the right precedence for ranges? */
%left SHIFT_LEFT SHIFT_RIGHT
%left ADD SUB
%left MUL DIV MOD
%right NOT BITWISE_NOT
%left LBRACKET
%left DOT

/* Handling dangling else */
%left NO_ELSE
%left ELSE

%type <std::string> IDENTIFIER STRING;
%type <bool> BOOL;
%type <long> INT;
%type <double> FLOAT;

%type <bool> opt_position is_array;
%type <OpenABL::AST::Var *> var;
%type <OpenABL::AST::Literal *> literal;
%type <OpenABL::AST::Type *> type;
%type <OpenABL::AST::AgentMember *> agent_member;
%type <OpenABL::AST::AgentMemberList *> agent_member_list;
%type <OpenABL::AST::Param *> param;
%type <OpenABL::AST::ParamList *> param_list non_empty_param_list;
%type <OpenABL::AST::Arg *> arg;
%type <OpenABL::AST::ArgList *> arg_list non_empty_arg_list;
%type <OpenABL::AST::StatementList *> statement_list;
%type <OpenABL::AST::MemberInitEntry *> member_init_entry;
%type <OpenABL::AST::MemberInitList *> member_init_list non_empty_member_init_list;
%type <OpenABL::AST::Declaration *> declaration func_decl agent_decl const_decl env_decl;
%type <OpenABL::AST::DeclarationList *> declaration_list;
%type <OpenABL::AST::IdentList *> ident_list;
%type <OpenABL::AST::Expression *> expression array_initializer initializer;
%type <OpenABL::AST::ExpressionList *> expression_list;
%type <OpenABL::AST::Statement *> statement;

%%

start: declaration_list { ctx.script = new Script($1, @$); }

declaration_list: %empty { $$ = new DeclarationList(); }
                | declaration_list declaration { $1->emplace_back($2); $$ = $1; };

declaration: agent_decl { $$ = $1; }
           | func_decl { $$ = $1; }
           | const_decl { $$ = $1; }
		   | env_decl { $$ = $1; }
           ;

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
          | non_empty_param_list { $$ = $1; };

non_empty_param_list: param { $$ = new ParamList(); $$->emplace_back($1); }
                    | non_empty_param_list COMMA param { $1->emplace_back($3); $$ = $1; };

var: IDENTIFIER { $$ = new Var($1, @$); }

param: type var { $$ = new Param($1, $2, nullptr, @$); }
     | type var ARROW var { $$ = new Param($1, $2, $4, @$); };

is_array: %empty { $$ = false; }
		| LBRACKET RBRACKET { $$ = true; };

opt_comma: %empty | COMMA;

expression_list: expression { $$ = new ExpressionList(); $$->emplace_back($1); }
			   | expression_list COMMA expression { $1->emplace_back($3); $$ = $1; }

array_initializer: LBRACE expression_list opt_comma RBRACE
				     { $$ = new ArrayInitExpression($2, @$); };

initializer: expression { $$ = $1; }
		   | array_initializer { $$ = $1; };

const_decl: type var is_array ASSIGN initializer SEMI
              { $$ = new ConstDeclaration($1, $2, $5, $3, @$); };

env_decl: ENVIRONMENT LBRACE member_init_list RBRACE
		    { $$ = new EnvironmentDeclaration($3, @$); };

literal: BOOL { $$ = new BoolLiteral($1, @$); }
       | INT { $$ = new IntLiteral($1, @$); }
       | FLOAT { $$ = new FloatLiteral($1, @$); }
       | STRING { $$ = new StringLiteral($1, @$); }
       ;

type: IDENTIFIER { $$ = new SimpleType($1, @$); };

statement_list: %empty { $$ = new StatementList(); }
              | statement_list statement { $1->emplace_back($2); $$ = $1; };

ident_list: IDENTIFIER { $$ = new IdentList(); $$->push_back($1); }
		  | ident_list COMMA IDENTIFIER { $1->push_back($3); $$ = $1; };

statement: expression SEMI { $$ = new ExpressionStatement($1, @$); }
         | LBRACE statement_list RBRACE { $$ = new BlockStatement($2, @$); }
         | type var SEMI
             { $$ = new VarDeclarationStatement($1, $2, nullptr, @$); }
         | type var ASSIGN expression SEMI
             { $$ = new VarDeclarationStatement($1, $2, $4, @$); }
         | IF LPAREN expression RPAREN statement %prec NO_ELSE
             { $$ = new IfStatement($3, $5, nullptr, @$); }
         | IF LPAREN expression RPAREN statement ELSE statement
             { $$ = new IfStatement($3, $5, $7, @$); }
         | FOR LPAREN type var COLON expression RPAREN statement
             { $$ = new ForStatement($3, $4, $6, $8, @$); }
         | WHILE LPAREN expression RPAREN statement
             { $$ = new WhileStatement($3, $5, @$); }
         | RETURN expression SEMI
             { $$ = new ReturnStatement($2, @$); }
         | RETURN SEMI
             { $$ = new ReturnStatement(nullptr, @$); }
         | SIMULATE LPAREN expression RPAREN LBRACE ident_list RBRACE
             { $$ = new SimulateStatement($3, $6, @$); }

         | expression ASSIGN expression SEMI
             { $$ = new AssignStatement($1, $3, @$); }
         | expression ADD_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::ADD, $1, $3, @$); }
         | expression SUB_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::SUB, $1, $3, @$); }
         | expression MUL_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::MUL, $1, $3, @$); }
         | expression DIV_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::DIV, $1, $3, @$); }
         | expression MOD_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::MOD, $1, $3, @$); }
         | expression BITWISE_AND_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::BITWISE_AND, $1, $3, @$); }
         | expression BITWISE_XOR_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::BITWISE_XOR, $1, $3, @$); }
         | expression BITWISE_OR_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::BITWISE_OR, $1, $3, @$); }
         | expression SHIFT_LEFT_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::SHIFT_LEFT, $1, $3, @$); }
         | expression SHIFT_RIGHT_ASSIGN expression SEMI
             { $$ = new AssignOpStatement(BinaryOp::SHIFT_RIGHT, $1, $3, @$); }
         ;

arg: expression { $$ = new Arg($1, nullptr, @$); }
   | expression ARROW expression { $$ = new Arg($1, $3, @$); };

arg_list: %empty { $$ = new ArgList(); }
        | non_empty_arg_list { $$ = $1; };

non_empty_arg_list: arg { $$ = new ArgList(); $$->emplace_back($1); }
                  | non_empty_arg_list COMMA arg { $1->emplace_back($3); $$ = $1; };

optional_comma: %empty
              | COMMA;

member_init_entry: IDENTIFIER COLON expression { $$ = new MemberInitEntry($1, $3, @$); };

member_init_list: %empty { $$ = new MemberInitList(); }
                | non_empty_member_init_list optional_comma { $$ = $1; };

non_empty_member_init_list: member_init_entry
                              { $$ = new MemberInitList(); $$->emplace_back($1); }
                          | non_empty_member_init_list COMMA member_init_entry
                              { $1->emplace_back($3); $$ = $1; };

expression: var { $$ = new VarExpression($1, @$); }
          | literal { $$ = $1; }
          | LPAREN expression RPAREN { $$ = $2; }
          | IDENTIFIER LPAREN arg_list RPAREN { $$ = new CallExpression($1, $3, @$); }
          | IDENTIFIER LBRACE member_init_list RBRACE
              { $$ = new AgentCreationExpression($1, $3, @$); }
          | expression DOT IDENTIFIER { $$ = new MemberAccessExpression($1, $3, @$); }
          | expression LBRACKET expression RBRACKET { $$ = new ArrayAccessExpression($1, $3, @$); }
          | expression QM expression COLON expression
              { $$ = new TernaryExpression($1, $3, $5, @$); }
          | NEW type LBRACKET expression RBRACKET { $$ = new NewArrayExpression($2, $4, @$); }
    
          | NOT expression           { $$ = new UnaryOpExpression(UnaryOp::LOGICAL_NOT, $2, @$); }
          | BITWISE_NOT expression   { $$ = new UnaryOpExpression(UnaryOp::BITWISE_NOT, $2, @$); }
          | ADD expression %prec NOT { $$ = new UnaryOpExpression(UnaryOp::PLUS, $2, @$); }
          | SUB expression %prec NOT { $$ = new UnaryOpExpression(UnaryOp::MINUS, $2, @$); }

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
          | expression DOTDOT expression
              { $$ = new BinaryOpExpression(BinaryOp::RANGE, $1, $3, @$); }
          ;
%%

void OpenABL::Parser::error(const OpenABL::location &loc, const std::string &message) {
  std::cerr << "Parse error: " << message << " on line " << loc.begin.line << std::endl;
}

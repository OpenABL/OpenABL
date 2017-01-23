%option noyywrap nounput noinput batch
%x ST_COMMENT

%{
#include "ParserContext.hpp"
#include "Parser.hpp"

using OpenABL::Parser;

#define YY_DECL \
  OpenABL::Parser::symbol_type yylex(OpenABL::ParserContext &ctx)

static OpenABL::location loc;
%}

DNUM [0-9]+
HNUM 0[xX][0-9a-fA-F]+
FLOAT_BASE {DNUM}"."{DNUM}?|"."{DNUM}
FLOAT {FLOAT_BASE}([eE][+-]?{DNUM}+)?
ID [a-zA-Z_][a-zA-Z0-9_]*

%%

"agent"    { return Parser::make_AGENT(loc); }
"interact" { return Parser::make_INTERACT(loc); }
"for"      { return Parser::make_FOR(loc); }
"position" { return Parser::make_POSITION(loc); }
"pfor"     { return Parser::make_PFOR(loc); }

{DNUM}  { return Parser::make_INT(loc); }
{HNUM}  { return Parser::make_INT(loc); }
{FLOAT} { return Parser::make_FLOAT(loc); }
{ID}    { return Parser::make_IDENTIFIER(loc); }

"+" { return Parser::make_ADD(loc); }
"-" { return Parser::make_SUB(loc); }
"*" { return Parser::make_MUL(loc); }
"/" { return Parser::make_DIV(loc); }
"%" { return Parser::make_MOD(loc); }
"&" { return Parser::make_BITWISE_AND(loc); }
"^" { return Parser::make_BITWISE_XOR(loc); }
"|" { return Parser::make_BITWISE_OR(loc); }
"=" { return Parser::make_ASSIGN(loc); }
"!" { return Parser::make_NOT(loc); }
"?" { return Parser::make_QM(loc); }
"." { return Parser::make_DOT(loc); }
"," { return Parser::make_COMMA(loc); }
":" { return Parser::make_COLON(loc); }
";" { return Parser::make_SEMI(loc); }
"<" { return Parser::make_SMALLER(loc); }
">" { return Parser::make_GREATER(loc); }
"(" { return Parser::make_LPAREN(loc); }
")" { return Parser::make_RPAREN(loc); }
"[" { return Parser::make_LBRACKET(loc); }
"]" { return Parser::make_RBRACKET(loc); }
"{" { return Parser::make_LBRACE(loc); }
"}" { return Parser::make_RBRACE(loc); }
"->" { return Parser::make_ARROW(loc); }
"==" { return Parser::make_EQUALS(loc); }
"!=" { return Parser::make_NOT_EQUALS(loc); }
"<=" { return Parser::make_SMALLER_EQUALS(loc); }
">=" { return Parser::make_GREATER_EQUALS(loc); }
"<<" { return Parser::make_SHIFT_LEFT(loc); }
">>" { return Parser::make_SHIFT_RIGHT(loc); }
"&&" { return Parser::make_LOGICAL_AND(loc); }
"||" { return Parser::make_LOGICAL_OR(loc); }
"+=" { return Parser::make_PLUS_ASSIGN(loc); }
"-=" { return Parser::make_MINUS_ASSIGN(loc); }
"*=" { return Parser::make_MUL_ASSIGN(loc); }
"/=" { return Parser::make_DIV_ASSIGN(loc); }
"%=" { return Parser::make_MOD_ASSIGN(loc); }
"&=" { return Parser::make_BITWISE_AND_ASSIGN(loc); }
"^=" { return Parser::make_BITWISE_XOR_ASSIGN(loc); }
"|=" { return Parser::make_BITWISE_OR_ASSIGN(loc); }
"<<=" { return Parser::make_SHIFT_LEFT_ASSIGN(loc); }
">>=" { return Parser::make_SHIFT_RIGHT_ASSIGN(loc); }

"//".* { /* ignore */ }

"/*" { BEGIN(ST_COMMENT); }
<ST_COMMENT>[^*\n]+    { /* ignore */ }
<ST_COMMENT>"*"+[^/\n] { /* ignore */ }
<ST_COMMENT>\n         { /* ignore */ }
<ST_COMMENT>"*/"       { BEGIN(INITIAL); }

[ \t\r]+ { /* ignore */ }
\n { /* ignore */ }

<<EOF>> { return Parser::make_END(loc); }

%%

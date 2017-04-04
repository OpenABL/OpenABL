%option noyywrap nounput noinput batch
%x ST_COMMENT

%{
#include "ParserContext.hpp"
#include "Parser.hpp"

using OpenABL::Parser;

#define YY_DECL \
  OpenABL::Parser::symbol_type yylex(OpenABL::ParserContext &ctx)

static OpenABL::location loc;
/* Run each time a pattern matches */
#define YY_USER_ACTION loc.columns (yyleng);

static std::string parseString(const char *str, size_t len) {
  std::string result;
  result.reserve(len - 2);
  for (size_t i = 1; i < len - 1; i++) {
    char c = str[i];
    if (c == '\\') {
      char n = str[++i];
      // TODO validate n
      result.push_back(n);
    } else {
      result.push_back(c);
    }
  }
  return result;
}

%}

DNUM [0-9]+
HNUM 0[xX][0-9a-fA-F]+
FLOAT_BASE {DNUM}"."{DNUM}?|"."{DNUM}
FLOAT {FLOAT_BASE}([eE][+-]?{DNUM}+)?
ID [a-zA-Z_][a-zA-Z0-9_]*

%%

%{
/* Run on each yylex call */
loc.step();
%}

"agent"    { return Parser::make_AGENT(loc); }
"else"     { return Parser::make_ELSE(loc); }
"if"       { return Parser::make_IF(loc); }
"interact" { return Parser::make_INTERACT(loc); }
"for"      { return Parser::make_FOR(loc); }
"new"      { return Parser::make_NEW(loc); }
"position" { return Parser::make_POSITION(loc); }
"return"   { return Parser::make_RETURN(loc); }
"simulate" { return Parser::make_SIMULATE(loc); }

true  { return Parser::make_BOOL(true, loc); }
false { return Parser::make_BOOL(false, loc); }

{DNUM} {
  // TODO: Overflow check
  return Parser::make_INT(std::stol(yytext, nullptr, 10), loc);
}
{HNUM} {
  // TODO: Overflow check
  return Parser::make_INT(std::stol(yytext, nullptr, 16), loc);
}
{FLOAT} {
  return Parser::make_FLOAT(std::stod(yytext, nullptr), loc);
}

\"(([^"]|\\.)*)\" {
  return Parser::make_STRING(parseString(yytext, yyleng), loc);
}

{ID}    { return Parser::make_IDENTIFIER(yytext, loc); }

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
"~" { return Parser::make_BITWISE_NOT(loc); }
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
".." { return Parser::make_DOTDOT(loc); }
"->" { return Parser::make_ARROW(loc); }
"==" { return Parser::make_EQUALS(loc); }
"!=" { return Parser::make_NOT_EQUALS(loc); }
"<=" { return Parser::make_SMALLER_EQUALS(loc); }
">=" { return Parser::make_GREATER_EQUALS(loc); }
"<<" { return Parser::make_SHIFT_LEFT(loc); }
">>" { return Parser::make_SHIFT_RIGHT(loc); }
"&&" { return Parser::make_LOGICAL_AND(loc); }
"||" { return Parser::make_LOGICAL_OR(loc); }
"+=" { return Parser::make_ADD_ASSIGN(loc); }
"-=" { return Parser::make_SUB_ASSIGN(loc); }
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
<ST_COMMENT>[^*\n]+
<ST_COMMENT>"*"+[^/\n]
<ST_COMMENT>\n+        { loc.lines(yyleng); }
<ST_COMMENT>"*/"       { loc.step(); BEGIN(INITIAL); }

[ \t\r]+ { loc.step(); }
\n+ { loc.lines(yyleng); }

<<EOF>> { return Parser::make_END(loc); }

%%

namespace OpenABL {

void ParserContext::initLexer() {
  yyin = file;
}

}

#ifndef _LEXER_HH
#define _LEXER_HH

#include "util.hh"
#include <iostream>
#include <fstream>
#include <vector>
using std::vector;

namespace Lexer {
enum TokenType {
  TC,
  IFP,
  FORALL,
  EXISTS,
  LPAREN,
  RPAREN,
  LBRACK,
  RBRACK,

  AND,
  OR,
  NOT,
  IFF,
  IMPLIES,
  XOR,

  LT,
  LTE,
  EQUALS,
  NEQUALS,

  TRUE,
  FALSE,
  MULT,
  PLUS,
  MINUS,
  COMMA,
  COLON,

  REDFIND,
  QUOTE,
  LBRACE,
  RBRACE,
  ASSIGN,
  NEW,
  REDUC,
  BQUERY,
  QUERY,
  STRUC,
  VOCAB,
  LOAD,
  LOADSTRING,
  SAVE,
  SIZE,
  DRAW,
  MACE,

  NUMBER,
  IDENTIFIER,
  QUOTED,

  NEWLINE,
  PERIOD,
  ERROR,
};

struct Token {
  Token(TokenType t, int l, int c): type(t), line(l), col(c) { }
  Token(int number, int l, int c): type(NUMBER), integer(number), line(l), col(c) { }
  Token(TokenType t, string s, int l, int c): type(t), data(s), line(l), col(c) { }

  bool operator==(const TokenType rhs) { return type == rhs; }

  const char* c_str() const { return data.c_str(); }
  string toString() const;

  TokenType type;
  int integer;
  string data;

  // for debugging / errors
  int line = -1;
  int col = -1;
};

vector<Token> lexString(const string &input);
}
#endif


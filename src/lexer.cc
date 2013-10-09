#include <string>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <vector>
#include "util.hh"
using std::string;
using std::vector;

#define returnAsString(tt) case tt: return #tt
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

  CONSTANT,
  NUMBER,
  VAR,
  PRED,
  FILENAME,

  NEWLINE,
  PERIOD,
  ERROR,
};

static const char* tokenTypeToString(TokenType tt) {
  switch(tt) {
    returnAsString(TC);
    returnAsString(IFP);
    returnAsString(FORALL);
    returnAsString(EXISTS);
    returnAsString(LPAREN);
    returnAsString(RPAREN);
    returnAsString(LBRACK);
    returnAsString(RBRACK);
    returnAsString(AND);
    returnAsString(OR);
    returnAsString(NOT);
    returnAsString(IFF);
    returnAsString(IMPLIES);
    returnAsString(XOR);
    returnAsString(LT);
    returnAsString(LTE);
    returnAsString(EQUALS);
    returnAsString(NEQUALS);
    returnAsString(TRUE);
    returnAsString(FALSE);
    returnAsString(MULT);
    returnAsString(PLUS);
    returnAsString(MINUS);
    returnAsString(COMMA);
    returnAsString(COLON);
    returnAsString(REDFIND);
    returnAsString(QUOTE);
    returnAsString(LBRACE);
    returnAsString(RBRACE);
    returnAsString(ASSIGN);
    returnAsString(NEW);
    returnAsString(REDUC);
    returnAsString(BQUERY);
    returnAsString(QUERY);
    returnAsString(STRUC);
    returnAsString(VOCAB);
    returnAsString(LOAD);
    returnAsString(LOADSTRING);
    returnAsString(SAVE);
    returnAsString(SIZE);
    returnAsString(DRAW);
    returnAsString(MACE);
    returnAsString(CONSTANT);
    returnAsString(NUMBER);
    returnAsString(VAR);
    returnAsString(PRED);
    returnAsString(FILENAME);
    returnAsString(NEWLINE);
    returnAsString(PERIOD);
    returnAsString(ERROR);
  }
  return "UNKNOWN";
}

struct LexerCharAction {
  char c;
  TokenType type;
};

struct LexerStringAction {
  const string s;
  TokenType type;
};

// Try to match big things first
static vector<LexerStringAction> StringActions = {
  {"TC", TC},
  {"TransitiveClosure", TC},
  {"IFP", IFP},
  {"\\A", FORALL},
  {"\\forall", FORALL},
  {"\\E", EXISTS},
  {"\\exists", EXISTS},
  {"<->", IFF},
  {"iff", IFF},
  {"->", IMPLIES},
  {"<=", LTE},
  {"!=", NEQUALS},
  {"\\t", TRUE},
  {"\\f", FALSE},
  {"redfind", REDFIND},
  {"is", ASSIGN},
  {":=", ASSIGN},
  {"new", NEW},
  {"reduction", REDUC},
  {"query", QUERY},
  {"structure", STRUC},
  {"vocabulary", VOCAB},
  {"loadstring", LOADSTRING},
  {"load", LOAD},
  {"save", SAVE},
  {"size", SIZE},
  {"draw", DRAW},
  {"mace", MACE},
  {"(", LPAREN},
  {")", RPAREN},
  {"[", LBRACK},
  {"]", RBRACK},
  {"&", AND},
  {"|", OR},
  {"~", NOT},
  {"!", NOT},
  {"^", XOR},
  {"<", LT},
  {"=", EQUALS},
  {"*", MULT},
  {"+", PLUS},
  {"-", MINUS},
  {",", COMMA},
  {".", PERIOD},
  {":", COLON}
};

struct Token {
  Token(TokenType t): type(t) { }
  Token(int number): type(NUMBER), integer(number) { }
  Token(TokenType t, string s): type(t), data(s) { }

  string toString() {
    if(type == NUMBER) {
      return stringf("NUMBER(%d)", integer);
    }
    if(data.size()) {
      return string(tokenTypeToString(type))+"("+data+")";
    }
    return tokenTypeToString(type);
  }

  TokenType type;
  int integer;
  string data;
};

class LexerStream {
  public:
    explicit LexerStream(const char *fn) : fp(fn), is(fp) { }
    explicit LexerStream(std::istream &i) : is(i) { }
    bool good() const { return is.good() || buffer.size(); }

    void skip(int n) {
      buffer = buffer.substr(n);
    }
    int peek() {
      if(!ensure(1))
        return -1;
      return buffer[0];
    }
    bool peekEquals(const string &ref) {
      if(!ensure(ref.size()))
        return false;
      for(int i=0; i<ref.size(); i++) {
        if(buffer[i] != ref[i])
          return false;
      }
      return true;
    }
    int next() {
      if(!ensure(1))
        return -1;
      char result = buffer[0];
      buffer = buffer.substr(1);
      return result;
    }
    bool ensure(int n) {
      if(n < buffer.size()) return true;
      int delta = (n - buffer.size());
      
      // read into a tmp buffer
      vector<char> tmp;
      tmp.reserve(4096);
      is.read(&tmp[0], 4096);
      int readAmt = is.gcount();

      // append to string what we can
      buffer.append(&tmp[0], readAmt);

      // return 
      return readAmt >= delta;
    }
  private:
    string buffer;
    std::ifstream fp;
    std::istream &is;
};

static Token fileName(LexerStream &input) {
  assert(input.peek() == '\"');
  input.next();
  string str;
  while(input.peek() != '\"' && input.peek() != -1) {
    str += (char) input.next();
  }
  return Token(FILENAME, str);
}

static void ignoreWhitespace(LexerStream &input) {
  while(input.peek() == ' ' || input.peek() == '\t')
    input.next();
}

Token nextToken(LexerStream &input) {
  // handle whitespace, end of file, quotes
  switch(input.peek()) {
    case '\"':
      return fileName(input);
    case -1:
      return NEWLINE;
    case '\n':
      input.skip(1);
      return NEWLINE;
    case '\t': case ' ':
      ignoreWhitespace(input);
      return nextToken(input);
    default: break;
  }

  // table driven lexing
  for(auto action : StringActions) {
    const string &id = action.s;
    if(input.peekEquals(id)) {
      input.skip(id.size());
      return action.type;
    }
  }

  return ERROR;
}


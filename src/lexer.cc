#include <string>
#include <cassert>
#include <cstdio>
#include <vector>
#include "lexer.hh"
#include "util.hh"
using std::string;
using std::vector;

using namespace Lexer;

/**
 * toString functionality starts here
 */
#define returnAsString(tt) case tt: return #tt
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
    returnAsString(NUMBER);
    returnAsString(QUOTED);
    returnAsString(IDENTIFIER);
    returnAsString(NEWLINE);
    returnAsString(PERIOD);
    returnAsString(ERROR);
  }
  return "UNKNOWN";
}
string Token::toString() const {
  if(type == NUMBER) {
    return stringf("NUMBER(%d)", integer);
  }
  if(data.size()) {
    return string(tokenTypeToString(type))+"("+data+")";
  }
  return tokenTypeToString(type);
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
  {"{", LBRACE},
  {"}", RBRACE},
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

bool LexerStream::good() const {
  return is.good() || buffer.size();
}
void LexerStream::skip(int n) {
  buffer = buffer.substr(n);
  col += n;
}
int LexerStream::peek() {
  if(!ensure(1))
    return -1;
  return buffer[0];
}
bool LexerStream::peekEquals(const string &ref) {
  if(!ensure(ref.size()))
    return false;
  for(size_t i=0; i<ref.size(); i++) {
    if(buffer[i] != ref[i])
      return false;
  }
  return true;
}
int LexerStream::next() {
  if(!ensure(1))
    return -1;
  char result = buffer[0];
  if(result == '\n') {
    line ++;
    col = 0;
  }
  buffer = buffer.substr(1);
  return result;
}
bool LexerStream::ensure(size_t n) {
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

static Token quoted(LexerStream &input) {
  assert(input.peek() == '\"');
  int line = input.line;
  int col = input.col;
  input.next();
  string str;
  while(input.peek() != '\"' && input.peek() != -1) {
    str += (char) input.next();
  }
  return {QUOTED, str, line, col};
}

static void ignoreWhitespace(LexerStream &input) {
  while(input.peek() == ' ' || input.peek() == '\t')
    input.next();
}

static Token identifier(LexerStream &input) {
  // must be an identifier
  string id;
  while(input.peek() != -1) {
    char next = input.peek();
    if(isalpha(next) || isdigit(next)) {
      id += input.next();
    } else break;
  }

  // if we started with a digit, we should be a number
  assert(!isdigit(id[0]));

  return {IDENTIFIER, id, input.line, input.col};
}

static Token number(LexerStream &input) {
  // must be an identifier
  string num;
  while(input.peek() != -1 && isdigit(input.peek())) {
    num += input.next();
  }
  int data = atoi(num.c_str());
  return {data, input.line, input.col};
}

Token Lexer::nextToken(LexerStream &input) {
  // handle whitespace, end of file, quotes
  switch(input.peek()) {
    case '\"':
      return quoted(input);
    case -1:
      return {NEWLINE, input.line, input.col};
    case '\n':
      input.skip(1);
      return {NEWLINE, input.line, input.col};
    case '\t': case ' ':
      ignoreWhitespace(input);
      return nextToken(input);
    default:
      break;
  }

  // table driven lexing
  for(auto action : StringActions) {
    const string &id = action.s;
    if(input.peekEquals(id)) {
      input.skip(id.size());
      return {action.type, input.line, input.col};
    }
  }

  if(isdigit(input.peek())) {
    return number(input);
  } else {
    return identifier(input);
  }
}


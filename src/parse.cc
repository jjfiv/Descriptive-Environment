// ISC LICENSE

#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include <string.h>
#include "protos.h"
#include "lexer.hh"

/**
 * Global variables
 */
Node *parsetree;
Node *cmdtree;

Node *node(int l, Node *left, Node *right)
{
  Node *n= (Node*) malloc(sizeof(Node));
  if (!n)
    return 0;
  n->l = left;
  n->r = right;
  n->label = l;
  n->data = 0;
  return n;
}

Node *inode(int l, int value)
{
  Node *n= (Node*) malloc(sizeof(Node));
  if (!n)
    return 0;
  n->label = l;
  n->data = (int*) malloc(sizeof(int));
  *((int* )(n->data)) = value; /* backwards compat, TODO:remove */
  n->ndata = value;
  n->ival = NULL;
  n->l = n->r = 0;
  return n;
}

Node *snode(int l, char *string)
{
  Node *n= (Node*) malloc(sizeof(Node));
  if (!n)
    return 0;
  n->label = l;
  n->data = dupstr(string);
  n->l = n->r = 0;
  n->ival = NULL;
  return n;
}

Node *qnode(int l, Node *vl, Node *restr, Node *form)
{
  Node *n;
  Node *vn;
  if (!(n=  (Node*) malloc(sizeof(Node))))
    return 0;
  if (!(vn=  (Node*) malloc(sizeof(Node))))
  {
    free(n);
    return 0;
  }

  n->label = l;
  n->l = vn;
  n->r = form;
  n->data = 0;

  vn->label=VLR;
  vn->l = vl;
  vn->r = restr;
  vn->data = 0;
  return n;
}

Node *vlnode(int l, char *var, Node *next)
{
  Node *n= (Node*) malloc(sizeof(Node));

  if (!n)
    return 0;
  n->label = l;
  n->l = 0;
  n->r = next; 
  n->data = dupstr(var);

  return n;
}

Node *arg_node(int l, Node *args)
{
  Node *n= (Node*) malloc(sizeof(Node));

  if (!n)
    return 0;
  n->label = l;
  n->l = args;
  n->r = 0;
  n->data = 0;
  return n;
}

Node *fournode(int lab, Node *ll, Node *lr, Node *rl, Node *rr)
{
  Node *n;
  Node *l;
  Node *r;

  if (!(n= (Node*) malloc(sizeof(Node))))
    return 0;

  if (!(l= (Node*) malloc(sizeof(Node))))
  {
    free(n);
    return 0;
  }
  if (!(r= (Node*) malloc(sizeof(Node))))
  {
    free(n); free(l);
    return 0;
  }

  n->label=lab;
  n->l=l;
  n->r=r;
  n->data = 0;

  l->label = FNODEIL;
  l->l = ll;
  l->r = lr;
  l->data = 0;

  r->label = FNODEIR;
  r->l = rl;
  r->r = rr;
  r->data = 0;

  return n;
}

void yyerror(const char *s)
{
  printf("%s\n",s);
}

/*
// subvector
vector<Token> subv(const vector<Token> &in, int start) {
  return vector<Token>(in.begin()+start, in.end());
}

static Node *mkNode() {
  return (Node*) malloc(sizeof(Node));
}

static Node* identifier(const Token &id) {
  return snode(ID, id.data.c_str());
}

using namespace Lexer;
Node* newParse(vector<Token> input) {
  //assign_cmd
  if(input[0] == Lexer::IDENTIFIER &&
     input[1] == Lexer::ASSIGN) {
    return node(ASSIGN, identifier(input[0]), cmdexpr(subv(input, 2)));
  }
  
  //struc_ex_cmd
  if(input[0] == Lexer::IDENTIFIER &&
     input[1] == Lexer::PERIOD &&
     input[3] == Lexer::LPAREN) {
    return node(EXPRED, identifier(input[0]),
        vlnode(PRED, input[2].c_str(),
        relargs(subv(input,4)));
  }

  if(input[0] == Lexer::IDENTIFIER &&
     input[1] == Lexer::PERIOD &&
     input[2] == Lexer::IDENTIFIER) {
    return node(EXPREDALL, identifier(input[0]), identifier(input[2]));
  }

  if(input[0] == Lexer::IDENTIFIER &&
     input[1] == Lexer::ASSIGN) {
  return node(ASSIGN, identifier(input[0])
  }

  return nullptr;
}
*/

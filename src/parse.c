// ISC LICENSE

#include <stdlib.h>
#include <stdio.h>
#include "parse.h"
#include <string.h>
#include "protos.h"

/**
 * Global variables
 */
struct node *parsetree;
struct node *cmdtree;

struct node *node(int l, struct node *left, struct node *right)
{
  struct node *n=malloc(sizeof(struct node));
  if (!n)
    return 0;
  n->l = left;
  n->r = right;
  n->label = l;
  n->data = 0;
  return n;
}

struct node *inode(int l, int value)
{
  struct node *n=malloc(sizeof(struct node));
  if (!n)
    return 0;
  n->label = l;
  n->data = malloc(sizeof(int));
  *((int* )(n->data)) = value; /* backwards compat, TODO:remove */
  n->ndata = value;
  n->ival = NULL;
  n->l = n->r = 0;
  return n;
}

struct node *snode(int l, char *string)
{
  struct node *n=malloc(sizeof(struct node));
  if (!n)
    return 0;
  n->label = l;
  n->data = dupstr(string);
  n->l = n->r = 0;
  n->ival = NULL;
  return n;
}

struct node *qnode(int l, struct node *vl, struct node *restr, struct node *form)
{
  struct node *n;
  struct node *vn;
  if (!(n= malloc(sizeof(struct node))))
    return 0;
  if (!(vn= malloc(sizeof(struct node))))
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

struct node *vlnode(int l, char *var, struct node *next)
{
  struct node *n=malloc(sizeof(struct node));

  if (!n)
    return 0;
  n->label = l;
  n->l = 0;
  n->r = next; 
  n->data = dupstr(var);

  return n;
}

struct node *arg_node(int l, struct node *args)
{
  struct node *n=malloc(sizeof(struct node));

  if (!n)
    return 0;
  n->label = l;
  n->l = args;
  n->r = 0;
  n->data = 0;
  return n;
}

struct node *fournode(int lab, struct node *ll, struct node *lr, struct node *rl, struct node *rr)
{
  struct node *n;
  struct node *l;
  struct node *r;

  if (!(n=malloc(sizeof(struct node))))
    return 0;

  if (!(l=malloc(sizeof(struct node))))
  {
    free(n);
    return 0;
  }
  if (!(r=malloc(sizeof(struct node))))
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

int yyerror(char *s)
{
  printf("%s\n",s);
  return 1;
}


//ISC LICENSE

#ifndef _PARSE_H
#define _PARSE_H
/* parse.h
 * Skip Jordan
 * Header for parse.c
 *
 * chj	10/19/06	Created.
 * chj  11/14/06	Fix for byacc-1.9 compatibility.
 */

extern int yyparse();

typedef struct Node {
  struct Node *l;
  struct Node *r;
  void *data;
  int *ival;
  int ndata;
  int label;
} Node;

extern Node *parsetree;
extern Node *cmdtree;

Node* node(int, Node*, Node*);
Node* inode(int, int);
Node* snode(int, char *);
Node* vlnode(int, char*, Node *);
Node* qnode(int, Node*, Node*, Node*);
Node* arg_node(int, Node *);
Node* fournode(int, Node*, Node*, Node*, Node*);

#endif


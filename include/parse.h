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

extern int yylex(void);

struct node {
  struct node *l;
  struct node *r;
  void *data;
  int *ival;
  int ndata;
  int label;
};

extern struct node *parsetree;
extern struct node *cmdtree;

struct node *node(int, struct node*, struct node*);
struct node *inode(int, int);
struct node *snode(int, char *);
struct node *vlnode(int, char*, struct node *);
struct node *qnode(int, struct node*, struct node*, struct node*);
struct node *arg_node(int, struct node *);
struct node *fournode(int, struct node*, struct node*, struct node*, struct node*);

#endif


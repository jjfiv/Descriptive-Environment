// ISC LICENSE
/* redtypes.h
 * Skip Jordan
 *
 * Type definitions for reduction-finding.  
 * chj	 4/5/12		created.
 */

#ifndef DEREDTYPESH
#define DEREDTYPESH

#include "minisat.h"
#include "types.h"
#include "parse.h"

#ifdef REDFIND_CUDD
#include "stdio.h" /* cudd.h needs stdio.h ... */
#include "cudd.h"
#endif

#ifdef REDFIND_DEBUG2
#define red_debug1(a) printf(a)
#define red_debug2(a,b) printf(a,b)
#define red_debug3(a,b,c) printf(a,b,c)
#else
#define red_debug1(a)
#define red_debug2(a,b)
#define red_debug3(a,b,c)
#endif

typedef struct TCDef {
  int num;
  int tup_arity;
  int size;
  Node *tc_node;
  Interp *interp;
  minisat_Lit *lits; /* lits indexed by tuple cindices */
  
  struct TCDef *next;
} TCDef;

typedef struct RedTupleElement {
  int type; /* 0==var (x-a-b), 1==constant, name is cons_names[a] */
  int a;    /* type 2, a natural (value a), only for cbvars */
  int b;
} RedTupleElement;

typedef struct RedTuple {
  RedTupleElement *data;
  int arity;
  int num_cons;
  char **cons_names;
} RedTuple;

typedef struct Example {
  Structure *a;
  int p1; /* whether this example has p1 */
} Example;

typedef struct RedBVarList {
  char *pos;
  char *neg;
  minisat_Var posVar;
  minisat_Var negVar;
  minisat_Lit posLit;
  minisat_Lit negLit;
#ifdef REDFIND_CUDD
  DdNode *posDdn;
  DdNode *negDdn;
#endif
  struct RedBVarList *next;
} RedBVarList;

typedef struct ExRelation {
  char *var;
  char *relname;
  RedTuple *tup;
  RedBVarList *bv;
} ExRelation;

typedef struct RedRelForms {
  int c;
  int arity;
  char *name;
  char **forms; /* forms forms[0]...forms[c-1], for clauses 1,...,c */
  struct RedRelForms *next;
} RedRelForms;

typedef struct ConsBVars {
  int num;
  char **lits;
  struct ConsBVars *next;
} ConsBVars;

typedef struct RedSearch {
  Environment *hash;
  minisat_solver *solver;
#ifdef REDFIND_CUDD
  DdManager *ddm;
  DdNode *cur_root;
#endif
  RedBVarList *bvars;
  RedBVarList *cbvars; /* Boolean variables for constants */
  RedBVarList *tbvars; /* temporary Boolean variables for TC */
  List *used_exrel;
  TCDef *tc[3]; /* 0: p1 in getex,
                         * 1: p2 in getex,
                         * 2: p2 in redfind
                         */
  const BQuery *p1;
  const BQuery *p2;
  int abort;
  int k;
  int c;
  int n;
  int n1;
  int n2;
  int num_ex;
  int num_tc;
  int outsize;
} RedSearch;

typedef struct RedHypot {
  RedSearch *rsearch;
  RedRelForms *relforms;
  List *consforms;
} RedHypot;


#endif /* DEREDTYPESH */


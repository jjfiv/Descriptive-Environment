/*
Copyright (c) 2012, Charles Jordan <skip@alumni.umass.edu>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
/* redtypes.h
 * Skip Jordan
 *
 * Type definitions for reduction-finding.  
 * chj	 4/5/12		created.
 */

#ifndef DEREDTYPESH
#define DEREDTYPESH

#include "minisat.h"

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

struct redsearch {
	struct env *hash;
	struct minisat_solver_t *solver;
#ifdef REDFIND_CUDD
	DdManager *ddm;
	DdNode *cur_root;
#endif
	struct red_bvarlist *bvars;
	struct red_bvarlist *cbvars; /* Boolean variables for constants */
	struct red_bvarlist *tbvars; /* temporary Boolean variables for TC */
	struct list *used_exrel;
	struct tc_def *tc[3]; /* 0: p1 in getex,
			       * 1: p2 in getex,
			       * 2: p2 in redfind
			       */
	const struct bquery *p1;
	const struct bquery *p2;
	int abort;
	int k;
	int c;
	int n;
	int n1;
	int n2;
	int num_ex;
	int num_tc;
	int outsize;
	};

struct tc_def {
	int num;
	int tup_arity;
	int size;
	struct tc_def *next;
	struct node *tc_node;
	struct interp *interp;
	minisat_Lit *lits; /* lits indexed by tuple cindices */
	};

struct ex_rel {
	char *var;
	char *relname;
	struct red_tuple *tup;
	struct red_bvarlist *bv;
	};

struct example {
        struct structure *a;
        int p1; /* whether this example has p1 */
};

struct red_tuple {
        struct red_tuple_element *data;
        int arity;
        int num_cons;
        char **cons_names;
        };

struct red_tuple_element {
        int type; /* 0==var (x-a-b), 1==constant, name is cons_names[a] */
        int a;    /* type 2, a natural (value a), only for cbvars */
        int b;
        };

struct red_bvarlist {
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
	struct red_bvarlist *next;
	};

struct red_hypot {
	struct redsearch *rsearch;
	struct red_relforms *relforms;
	struct list *consforms;
	};

struct red_relforms {
	int c;
	int arity;
	char *name;
	char **forms; /* forms forms[0]...forms[c-1], for clauses 1,...,c */
	struct red_relforms *next;
	};

struct cons_bvars {
	int num;
	char **lits;
	struct cons_bvars *next;
	};
#endif /* DEREDTYPESH */

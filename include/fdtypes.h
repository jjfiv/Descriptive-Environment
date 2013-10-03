/*
Copyright (c) 2013, Charles Jordan <skip@alumni.umass.edu>

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
/* fdtypes.h
 * Skip Jordan
 *
 * Type definitions for formula distance.  
 * chj   1/25/13         created.
 */

#ifndef FDTYPESH
#define FDTYPESH

#include <stdio.h> /* cudd.h needs stdio.h ... */
#include "cudd.h"

struct fd_bvarlist {
	int clause; /* what clause, 0..c-1 */
	int type; /* 0: eq, 1: succ, 2: <, 3: pred */
	int arity; /* 2 for eq, succ, <, arity of pred for pred */

	int binvars[2]; /* for monadic,binary only: binvars[i]=0 if not a var,
			 * otherwise, binvers[i] is # of var (1...k)
			 */
	char **consargs; /* names of constants, 0...arity-1 */
			 /* free consargs, not consargs[i] */
	int *varargs;    /* see binvars, used if arity>2 */
	char *predname; /* predname if type==3, don't free */
	DdNode *posDdn;
	DdNode *negDdn;

	struct fd_bvarlist *next;
	};

struct fd_quniv {
	int varnum; /* 1 ... k */
	/* this node goes to false if existential, true if universal */
	DdNode *qDdn;
	struct fd_quniv *next;
	};

struct fdistance {
	DdManager *ddm;
	struct fd_bvarlist *fdvars;
	struct fd_quniv *qvars;
	struct structure *s1;
	struct structure *s2;

	int numvars;
	int k;
	int c;
	int ex;
	};

#endif

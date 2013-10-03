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
/* fd.c
 * Skip Jordan
 * Things related to formula distance.
 *
 * chj	 1/24/13	created
 */

#include "parse.h"
#include "types.h"
#include "protos.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* CUDD */
#include "util.h"
#include "cudd.h"

#include "fdtypes.h"
#include "fdprotos.h"

#include "math.h" /* pow */

#define FD_DEBUG2 /* check zero references before Cudd_Quit */
#define FD_DEBUG3  /* define to put a dot of the BDD in /tmp/disting.dot */
			/* and output variable meanings on stdout */
#define FD_ZDDCHECK  /* define to convert the BDD to a ZDD and */
			  /* put it in /tmp/zdd_disting.dot */

/* print distance between s1,s2. k variables, c clauses, ex==1 if
 * existential only
 */
int fdistance(struct structure *s1, struct structure *s2, int k,
	      int c, int ex)
{
	struct fdistance *fdist = fd_init(s1,s2,k,c,ex);
	DdNode *s1form, *s2form, *res;
	DdManager *ddm = fdist->ddm;
	double r1, r2;
	int nvars;
#ifdef FD_ZDDCHECK
	DdNode *zdd;
#endif

	if (!fdist)
		return 0;

	ddm = fdist->ddm;
	nvars = fdist->numvars;

	s1form = fd_getforms(fdist, s1);
	s2form = fd_getforms(fdist, s2);

	res = Cudd_bddXor(ddm, s1form, s2form);
	Cudd_Ref(res);
	Cudd_RecursiveDeref(ddm, s1form);
	Cudd_RecursiveDeref(ddm, s2form);

	s1form = NULL;
	s2form = NULL;

	r1 = (Cudd_CountPathsToNonZero(res)/Cudd_CountPath(res));
	r2 = (Cudd_CountMinterm(ddm, res, nvars)/pow(2,nvars));

#ifdef FD_DEBUG3
	output_disting(fdist, res, "/tmp/disting.dot",0);
#endif
#ifdef FD_ZDDCHECK
	Cudd_zddVarsFromBddVars(ddm, 1);
	zdd = Cudd_zddPortFromBdd(ddm, res);
	Cudd_Ref(zdd);
	output_disting(fdist, zdd, "/tmp/zdd_disting.dot",1);
	Cudd_RecursiveDeref(ddm, zdd);
#endif

	Cudd_RecursiveDeref(ddm, res);
	fd_cleanup(fdist);
	printf(": %f  %f\n", r1, r2);

#if 0
	printf("Not implemented yet, sorry (%s,%s,%d,%d,%d)\n",
		s1->name,s2->name,k,c,ex);
#endif
	return 0;
}

/* type is 0 if bdd, 1 if zdd */
void output_disting(struct fdistance *fdist, DdNode *disting, char *fn, 
		    int type)
{
	FILE *f;
	struct fd_bvarlist *bv;
	struct fd_quniv *qv;
	char op;
	int i;
	int *va;
	char ops[4]={'=','=','<','\0'};

	f = fopen(fn,"w");
	if (!type)
		Cudd_DumpDot(fdist->ddm, 1, &disting, NULL, NULL, f);
	else
		Cudd_zddDumpDot(fdist->ddm,1,&disting,NULL,NULL,f);
	fclose(f);

	for (qv=fdist->qvars; qv; qv=qv->next)
		printf("q: %d	%d\n",qv->qDdn->index, qv->varnum);
	for (bv=fdist->fdvars; bv; bv=bv->next)
	{
		printf("bv: %d (clause %d) ",bv->posDdn->index,bv->clause);
		op = ops[bv->type];
		switch (bv->type)
		{
			case 0:
			case 1:
			case 2:
				if (bv->binvars[0])
					printf("x%d", bv->binvars[0]);
				else
					printf("%s", bv->consargs[0]);
				printf("%c",op);
				if (bv->binvars[1])
                                        printf("x%d", bv->binvars[1]);
                                else
                                        printf("%s", bv->consargs[1]);
				if (bv->type==1)
					printf("+1");
				break;
			case 3:
				printf("%s(",bv->predname);
				if (bv->arity<=2)
					va=bv->binvars;
				else
					va=bv->varargs;
				for (i=0; i<bv->arity; i++)
				{
					if (i)
						printf(",");
					if (va[i])
						printf("x%d",va[i]);
					else
						printf("%s",bv->consargs[i]);
				}
				printf(")");
				break;
		}

		printf("\tneg:%d\n",bv->negDdn->index);
	}

	return;
}

/* make the set of formulas in our class that are satisfied by s */
/* should replace the next_tuple by splitting into two cases and or
 * result
 */
DdNode *fd_getforms(struct fdistance *fdist, struct structure *s)
{
	int i, k=fdist->k;
	int *qmask=NULL;
	int *vals;
	DdNode *res=NULL, *tmp1, *tmp2, *tmp3;
	DdManager *ddm = fdist->ddm;
	int max=2; /* over both existential and univ */
	struct interp *interp = new_interp(s);
	if (fdist->ex) 
		max=1; /* just existential */

	/* probably should split range of qmasks in half each step,
	 * minimize number of big bddOrs
	 */
	vals = malloc(sizeof(int)*k); /* values for x1...xk */
	for (i=0; i<k; i++)
		vals[i]=-1;
	while ((qmask=next_tuple(qmask, k, max)))
	{
		tmp1 = fd_qmask_dd(fdist, k, qmask);
		tmp2 = fd_getsatforms(fdist, interp, s, qmask, vals, 0, 0, 
				      s->size-1);
		tmp3 = Cudd_bddAnd(ddm, tmp1, tmp2);
		Cudd_Ref(tmp3);
		Cudd_RecursiveDeref(ddm, tmp1);
		Cudd_RecursiveDeref(ddm, tmp2);
		if (!res)
			res=tmp3;
		else
		{
			tmp1 = Cudd_bddOr(ddm, res, tmp3);
			Cudd_Ref(tmp1);
			Cudd_RecursiveDeref(ddm, res);
			Cudd_RecursiveDeref(ddm, tmp3);
			res = tmp1;
		}
	}
	free_interp(interp);
	return res;
}

/* return the DD corresponding to ranging quantifier q (0<q<k)
 * between n1<=val<=n2, qmask[q]==0 if existential else universal,
 * variables with id < q are in interp.
 * splits range in HALF at each step unless n1==n2
 */
DdNode *fd_getsatforms(struct fdistance *fdist, struct interp *interp,
		       struct structure *s, int *qmask, int *vals, int q,
		       int n1, int n2)
{
	int mid;
	int qtype;
	int oldval;
	int c=fdist->c;
	DdNode *dd1, *dd2, *res;
	DdManager *ddm=fdist->ddm;
	int k=fdist->k;

	if (n1<n2)
	{
		qtype=qmask[q];
		mid = (n2+n1)>>1;
		dd1 = fd_getsatforms(fdist, interp, s, qmask, vals, q,
				     n1, mid);
		dd2 = fd_getsatforms(fdist, interp, s, qmask, vals, q,
				     mid+1, n2);
		if (qtype==0)
		{
			/* existential */
			res = Cudd_bddOr(ddm, dd1, dd2);
			Cudd_Ref(res);
			Cudd_RecursiveDeref(ddm, dd1);
			Cudd_RecursiveDeref(ddm, dd2);
			return res;
		}
		/* universal */
		res = Cudd_bddAnd(ddm, dd1, dd2);
		Cudd_Ref(res);
		Cudd_RecursiveDeref(ddm, dd1);
		Cudd_RecursiveDeref(ddm, dd2);
		return res;
	}

	/* n1 = n2, so add to the interp and call on the next var,
	 * unless q=k-1 (last one), in which case we get the forms
	 */
	oldval = vals[q];
	vals[q] = n1;
	if (q<k-1)
	{
		res = fd_getsatforms(fdist, interp, s, qmask, vals, q+1,
				     0, s->size-1);
		vals[q]=oldval;
		return res;
	}

	/* yay, we have all the quantifiers fixed now */
	res = fd_getallsatforms(fdist, interp, s, vals, 0, c-1);
	vals[q]=oldval;
	return res;
}


/* vals[i] is value of xi+1. */
/* we split clause range in half at each step */
DdNode *fd_getallsatforms(struct fdistance *fdist, struct interp *interp,
			  struct structure *s, int *vals, int c1, int c2)
{
	int mid;
	DdNode *tmp1, *tmp2, *res;
	DdManager *ddm=fdist->ddm;

	if (c2>c1)
	{
		mid = (c1+c2)>>1;
		tmp1 = fd_getallsatforms(fdist,interp,s,vals,c1,mid);
		tmp2 = fd_getallsatforms(fdist,interp,s,vals,mid+1,c2);
		res = Cudd_bddOr(ddm, tmp1, tmp2);
		Cudd_Ref(res);
		Cudd_RecursiveDeref(ddm, tmp1);
		Cudd_RecursiveDeref(ddm, tmp2);
		return res;
	}
	else
		return fd_realgetsatforms(fdist,interp,s,vals,c1);
}

/* vals[i] is value of variable x(i+1) */
/* we do clause c, return
 * (or over all variables representing things in this clause we satisfy) AND
 * NOT (or over all variables representing things in this clause that we don't)
 */
DdNode *fd_realgetsatforms(struct fdistance *fdist, struct interp *interp,
			   struct structure *s, int *vals, int c)
{
	DdManager *ddm=fdist->ddm;
	DdNode *satforms=Cudd_ReadLogicZero(ddm), 
	       *unsatforms=Cudd_ReadLogicZero(ddm);
	DdNode *tmp1, *tmp2, *tmp3;
	struct fd_bvarlist *vars;

	Cudd_Ref(satforms);
	Cudd_Ref(unsatforms);

	for (vars=fdist->fdvars; vars; vars=vars->next)
	{
		if (c!=vars->clause)
			continue;
		if (fd_issat(s,interp,vals,vars))
		{
			/* add pos to satforms, neg to unsatforms */
			tmp1 = vars->posDdn;
			tmp2 = vars->negDdn;
		}
		else
		{
			/* add neg to satforms, pos to unsatforms */
			tmp1 = vars->negDdn;
			tmp2 = vars->posDdn;
		}

		tmp3 = Cudd_bddOr(ddm,satforms,tmp1);
		Cudd_Ref(tmp3);
		Cudd_RecursiveDeref(ddm,satforms);
		satforms = tmp3;

		tmp3 = Cudd_bddOr(ddm,unsatforms,tmp2);
		Cudd_Ref(tmp3);
		Cudd_RecursiveDeref(ddm,unsatforms);
		unsatforms = tmp3;
	}

	tmp1 = Cudd_Not(unsatforms);
	tmp2 = Cudd_bddAnd(ddm, satforms, tmp1);
	Cudd_Ref(tmp2);
	Cudd_RecursiveDeref(ddm,satforms);
	Cudd_RecursiveDeref(ddm,unsatforms);
	return tmp2;
}

/* fd_issat(s,vals,vars)
 * returns 1 if (s,vals) satisfies the atomic represented by vars,
 * otherwise 0
 */
int fd_issat(struct structure *s, struct interp *interp, int *vals, 
	     struct fd_bvarlist *vars)
{
	int left, right;
	if (vars->type==3)
		return fd_issatpred(s,interp,vals,vars);

	left = fd_geteval(interp,vals,vars,0);
	right = fd_geteval(interp,vals,vars,1);

	switch (vars->type)
	{
		case 0: /* equality */
			return (left==right);
		case 1: /* succ */
			return (right==left+1);
		case 2: /* < */
			return (left<right);
		default:
			printf("Unknown variable type %d\n",vars->type);
			return 1;
	}
}

/* fd_issatpred
 * return 1 if (s,vals) satisfies the atomic represented by vars,
 * which is a relation
 * return 0 otherwise
 */
int fd_issatpred(struct structure *s, struct interp *interp, int *vals, 
		 struct fd_bvarlist *vars)
{
	int arity=vars->arity;
	int *args=malloc(sizeof(int)*arity);
	int i;
	struct relation *rel = get_relation(vars->predname, interp, s);
	int tupnum;

	for (i=0; i<arity; i++)
		args[i]=fd_geteval(interp, vals,vars,i);

	tupnum = tuple_cindex(args, arity, s->size);
	return rel->cache[tupnum];
}

/* fd_geteval(vals,vars,index)
 * 
 * evaluate the "index-th" argument (either variable or constant) using
 * vals.
 * index must be < vars->arity
 */
int fd_geteval(struct interp *interp, int *vals, struct fd_bvarlist *vars, int index)
{
	int arity=vars->arity;
	int bvi;

	if (index>=arity)
	{
		printf("Index out of bound\n");
		return 0;
	}

	if (arity<=2)
	{
		bvi=vars->binvars[index];
		if (bvi) /* var */
			return vals[bvi-1];
		else /* constant */
			return get_interp_value(vars->consargs[index],interp);
	}
	/* not binary */
	bvi=vars->varargs[index];
	if (bvi) /* var */
		return vals[bvi-1];
	else /* constant */
		return get_interp_value(vars->consargs[index],interp);
}

/* fd_init(s1,s2,k,c,ex)
 * Sets up the fdistance structure (inits CUDD, sets up variable
 * lists, etc).
 */
struct fdistance *fd_init(struct structure *s1, struct structure *s2,
			  int k, int c, int ex)
{
	struct fdistance *fdist;
	struct relation *rel;
	if (s1->vocab!=s2->vocab)
	{
		printf("fd??: Different vocabularies.\n");
		return NULL;
	}
	if (k<1)
	{
		printf("fd??: k must be positive\n");
		return NULL;
	}
	if (c<1)
	{
		printf("fd??: c must be positive\n");
		return NULL;
	}
	if (ex<0 || ex>1)
	{
		printf("fd??: ex must be (0,1)\n");
		return NULL;
	}

	fdist = malloc(sizeof(struct fdistance));
	fdist->ddm = Cudd_Init(0,0,CUDD_UNIQUE_SLOTS,CUDD_CACHE_SLOTS,0);
	fdist->s1 = s1;
	fdist->s2 = s2;
	fdist->k = k;
	fdist->c = c;
	fdist->ex = ex;

	fdist->qvars = fd_initqvars(fdist);	
	fdist->fdvars = fd_initfdvars(fdist);
	fdist->numvars = fd_countvars(fdist->fdvars);
	if (fdist->ex)
		fdist->numvars += fd_countqvars(fdist->qvars);

	for (rel=s1->rels; rel; rel=rel->next)
		fill_relcache(rel, s1);
	for (rel=s2->rels; rel; rel=rel->next)
		fill_relcache(rel, s2);

	return fdist;
}

int fd_countqvars(struct fd_quniv *vars)
{
	struct fd_quniv *tmp;
	int count=0;
	for (tmp=vars; tmp; tmp=tmp->next)
		count++;
	return count;
}

int fd_countvars(struct fd_bvarlist *vars)
{
	int count=0;
	struct fd_bvarlist *tmp;

	for (tmp=vars; tmp; tmp=tmp->next)
		count+=2;

	return count;
}

struct fd_quniv *fd_initqvars(struct fdistance *fdist)
{
	struct fd_quniv *qv1, *qv2=NULL;
	int k, i;
	DdManager *ddm = fdist->ddm;
	k=fdist->k;

	for (i=1; i<=k; i++)
	{
		qv1 = malloc(sizeof(struct fd_quniv));
		qv1->varnum = i;
		qv1->next = qv2;
		qv1->qDdn = Cudd_bddNewVar(ddm);
		qv2=qv1;
	}

	return qv1;
}

struct fd_bvarlist *fd_initfdvars(struct fdistance *fdist)
{
	struct fd_bvarlist *vars=NULL;
	struct constant *cons;
	struct relation *rel;
	int i,k=fdist->k,c=fdist->c;
	int v1, v2;
	int use_succ=1;
	int use_lt=0;
	int use_eq=0;
	struct structure *s1=fdist->s1;

	/* group Boolean variables by clause first */
	for (i=0; i<c; i++)
	{
		/* add cons=cons etc? */

		for (v1=1; v1<=k; v1++)
		{
			for (v2=v1+1; v2<=k; v2++)
			{
				/* add v1=v2 and v1!=v2 */
				if (use_eq)
					vars = fd_fdaddlist(fdist, vars, i,0, 2,
							    v1, v2, NULL, NULL);
				/* add v1=v2+1 and v2=v1+1 */
				if (use_succ)
				{
					vars=fd_fdaddlist(fdist, vars, i, 1, 2,
							  v1, v2, NULL, NULL);
					vars=fd_fdaddlist(fdist, vars, i, 1, 2,
							  v2, v1, NULL, NULL);
				}

				/* add v1<v2 and v2<v1 */
				if (use_lt)
				{
					vars=fd_fdaddlist(fdist, vars, i, 2, 2,
							  v1, v2, NULL, NULL);
					vars=fd_fdaddlist(fdist, vars, i, 2, 2,
							  v2, v1, NULL, NULL);
				}
			}

			for (cons=s1->cons; cons; cons=cons->next)
				vars=fd_fdaddveqc(fdist, vars, i, v1, cons);
		}

		for (rel=s1->rels; rel; rel=rel->next)
			vars = fd_add_predvars(fdist, i, vars, rel);
	}

	return vars;
}

/* add var = cons */
struct fd_bvarlist *fd_fdaddveqc(struct fdistance *fdist,
                                  struct fd_bvarlist *vars, int clause,
                                  int var, struct constant *cons)
{
	struct fd_bvarlist *nbv = malloc(sizeof(struct fd_bvarlist));
	nbv->clause=clause;
	nbv->type = 0;
	nbv->arity = 2;
	nbv->binvars[0]=var;
	nbv->binvars[1]=0;

	nbv->consargs=malloc(sizeof(char *)*2);
	nbv->consargs[0]=NULL;
	nbv->consargs[1]=cons->name;

	nbv->varargs = NULL;
	nbv->predname=NULL;

	nbv->posDdn = Cudd_bddNewVar(fdist->ddm);
        nbv->negDdn = Cudd_bddNewVar(fdist->ddm);
        nbv->next = vars;
	return nbv;
}


/* fd_fdaddlist(fdist, vars, clause, type, arity, bv0, bv1, consargs, varargs)
 * adds this one to the fd_bvarlist vars
 */
struct fd_bvarlist *fd_fdaddlist(struct fdistance *fdist, 
				 struct fd_bvarlist *vars, int clause, 
				 int type, int arity, int bv0, int bv1, 
				 char **consargs, int *varargs)
{
	struct fd_bvarlist *nbv = malloc(sizeof(struct fd_bvarlist));
	nbv->clause = clause;
	nbv->type = type;
	nbv->arity = arity;
	nbv->binvars[0]=bv0;
	nbv->binvars[1]=bv1;
	nbv->consargs = consargs;
	nbv->varargs = varargs;
	nbv->predname = NULL;

	nbv->posDdn = Cudd_bddNewVar(fdist->ddm);
	nbv->negDdn = Cudd_bddNewVar(fdist->ddm);
	nbv->next = vars;
	return nbv;
}

/* fd_add_predvars(fdist, vars, rel)
 * return variables for rel->name[tup] to vars, for all
 * tuples over variables and constant symbols of correct arity.
 */
/* based on redfind.c:red_add_pred(...) */
struct fd_bvarlist *fd_add_predvars(struct fdistance *fdist,
				    int clause,
				    struct fd_bvarlist *vars,
				    struct relation *rel)
{
	int k=fdist->k;
	struct constant *cons=fdist->s1->cons;
	int *tuple=NULL;
	struct fd_bvarlist *ret=vars;
	int arity=rel->arity;
	char *name = rel->name;
	int numcons=0;

	for (cons=fdist->s1->cons; cons; cons=cons->next)
		numcons++;
	cons=fdist->s1->cons;

	while ((tuple=next_tuple(tuple, arity, k+numcons)))
	{
		/* tuple[i] between 0...k-1 means x_tup+1,
		 * tuple[i]>=k means constant tup-k
		 */
		ret=fd_addpredvar(fdist, ret, tuple, arity, cons, name, clause);
	}
	return ret;
}

/* add the fd_bvarlist for name(tuple) to vars and return it.
 * tuple is arity arity, 0<=tuple[i]<=k-1 means variable x_(tuple[i]+1)
 * tuple[i]>=k means constant #tuple[i]-k (k=cons,k+1=cons->next,...)
 */
struct fd_bvarlist *fd_addpredvar(struct fdistance *fdist,
				  struct fd_bvarlist *vars, const int *tuple,
				  int arity, struct constant *cons, 
				  char *name, int clause)
{
	struct fd_bvarlist *ret=malloc(sizeof(struct fd_bvarlist));
	int i, index, cflag=0;
	int k=fdist->k;
	struct constant *tmpcons;
	DdManager *ddm=fdist->ddm;

	for (i=0; i<arity; i++)
		if (tuple[i]>=k)
		{
			cflag=1;
			break;
		}

	ret->next=vars;
	ret->clause=clause;
	ret->type=3;
	ret->arity=arity;
	ret->predname=name;
	if (!cflag)
	{
		ret->consargs=NULL;
		if (arity<=2)
			ret->varargs=NULL;
	}
	else
		ret->consargs=malloc(sizeof(char *)*arity);
	if (arity>2)
		ret->varargs = malloc(sizeof(int)*arity);

	for (i=0; i<arity; i++)
	{
		if (tuple[i]<k)
		{
			if (arity<=2)
			{
				ret->binvars[i]=tuple[i]+1;
				if (cflag)
					ret->consargs[i]=NULL;
				continue;
			}
			ret->varargs[i]=tuple[i]+1;
			if (cflag)
				ret->consargs[i]=NULL;
			continue;
		}
		/* otherwise we're a constant */
		if (arity<=2)
			ret->binvars[i]=0;
		else
			ret->varargs[i]=0;

		index=tuple[i]-k;
		for (tmpcons=cons; index; tmpcons=tmpcons->next)
			index--;
		ret->consargs[i]=tmpcons->name;
	}

	ret->posDdn = Cudd_bddNewVar(ddm);
	ret->negDdn = Cudd_bddNewVar(ddm);
	return ret;
}

/* qmask is a k-ary array. qmask[i]=0 if var x+1 is existential,
 * qmask[i]=1 if universal.  return a BDD representing the prefix
 */
DdNode *fd_qmask_dd(struct fdistance *fdist, int k, int *qmask)
{
	int i;
	struct fd_quniv *var;
	DdNode *res=NULL;
	DdNode *tmp, *tmp2;
	DdManager *ddm = fdist->ddm;

	DdNode **varnodes = malloc(sizeof(DdNode *)*k);
	for (var=fdist->qvars; var; var=var->next)
		varnodes[var->varnum-1]=var->qDdn;
	
	for (i=0; i<k; i++)
	{
		tmp = varnodes[i];
		if (!qmask[i])
		{
			tmp2=Cudd_Not(tmp);
			tmp=tmp2;
		}
		if (!res)
		{
			Cudd_Ref(tmp);
			res=tmp;
			continue;
		}
		tmp2 = Cudd_bddAnd(ddm, res, tmp);
		Cudd_Ref(tmp2);
		Cudd_RecursiveDeref(ddm, res);
		res = tmp2;
	}

	free(varnodes);
	return res;
}

/* free / cleanup everything in fdist that's not needed elsewhere */
void fd_cleanup(struct fdistance *fdist)
{
	struct fd_bvarlist *fdb1, *fdb2;
	struct fd_quniv *fqv1, *fqv2;
#ifdef FD_DEBUG2
	int checkz;
#endif

	for (fdb1=fdist->fdvars; fdb1; fdb1=fdb2)
	{
		fdb2=fdb1->next;
		free(fdb1->consargs);
		free(fdb1->varargs);
		free(fdb1);
	}

	for (fqv1=fdist->qvars; fqv1; fqv1=fqv2)
	{
		fqv2=fqv1->next;
		free(fqv1);
	}

#ifdef FD_DEBUG2
	checkz = Cudd_CheckZeroRef(fdist->ddm);
	if (checkz)
		printf("\n\n    Cudd_CheckZeroRef: %d\n\n",checkz);
#endif
	Cudd_Quit(fdist->ddm);

	free(fdist);
	
	return;
}



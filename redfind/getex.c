/*
Copyright (c) 2012-2013, Charles Jordan <skip@alumni.umass.edu>

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
/* getex.c
 * Skip Jordan
 *
 * This includes the example-finding code for reduction-finding.  It is
 * based on the ideas in
 * Crouch, Immerman, Moss: Finding Reductions Automatically, 
 * in Fields of Logic and Computation, 2010, Springer, 181 - 200.
 *
 * chj	 4/11/12	created
 * chj	 7/13/12	changes for fast interpretations
 * chj	 7/17/12	debugging output for looking at formulas
 * chj	11/06/12	add support for "x=max" literals
 * chj	11/16/12	support for x=y+1 (REDFIND_SUCC)
 * chj	02/19/13	ex_improve_example
 */

#include "types.h"
#include "redtypes.h"
#include "protos.h"
#include "redprotos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "parse.h"
#include "minisat.h"
#include <sys/time.h>

#ifndef REDFIND_RANDEX
#define REDFIND_RANDEX 1
#endif
#ifndef RF_altex
#define RF_altex 0
#endif
#ifndef RF_minex
#define RF_minex 0
#endif

#define INIT_COMMAND(s) \
        bufstate = yy_scan_string(s); \
        yyparse(); \
        do_cmd(cmdtree->l); \
        yy_delete_buffer(bufstate)

/* returns the next example.  rsearch contains the satisfying assignment
 * giving a candidate reduction. 
 */
struct example *get_next_example(struct redsearch *rsearch)
{
	/* initialize solver */
	struct minisat_solver_t *solver = minisat_new();
	struct env *env = ex_inithash();
	struct red_bvarlist *bv = ex_initbvarlist(rsearch,solver,env);
	struct example *ex;
	int res;
	char *phi=NULL;
	char *tmp=NULL;
	struct tc_def *tcd;

	/* add requirements that constants have exactly one definition */
	red_debug1("\n      -- Formula for counter-example --\n");
	ex_initsolver(solver,env,rsearch);

	phi = ex_makeform(rsearch,bv);

	bv = ex_addtcvars(rsearch->tc[0], solver, bv, env);
	bv = ex_addtcvars(rsearch->tc[1], solver, bv, env);

	red_debug2("&(%s)",phi);
	res = limboole(phi,solver,env);
	free(phi);

	for (tcd=rsearch->tc[0]; tcd; tcd=tcd->next)
	{
		tmp = ex_maketcdef(rsearch, tcd, bv);
		red_debug2("&(%s)",tmp);
		res = limboole(tmp,solver,env);
		free(tmp);
		tmp=NULL;
	}
	for (tcd=rsearch->tc[1]; tcd; tcd=tcd->next)
	{
		tmp = ex_make_part2_tcdef(rsearch, tcd, bv);
		red_debug2("&(%s)",tmp);
		res = limboole(tmp, solver, env);
		free(tmp);
		tmp=NULL;
	}
	
	ex_freetcd(rsearch->tc[0]);
	ex_freetcd(rsearch->tc[1]);
	rsearch->tc[0]=NULL;
	rsearch->tc[1]=NULL;

	red_debug1("\n\n      Calling minisat_solve for example.... ");
#ifdef REDFIND_DEBUG2
	fflush(stdout);
#endif
	res = minisat_solve(solver, 0, NULL);
	red_debug1("minisat_solve is done\n\n");

	/* FREE STUFF LIKE bvars! */
	if (res == minisat_l_False)
	{
		hash_free_nodes(env->id_hash);
        	hash_destroy(env->id_hash);
		free(env);
		minisat_delete(solver);
		red_freebvars(bv);
		return NULL;
	}
	else
	{
#if (REDFIND_RANDEX>0)
		res = ex_randomize_example(solver,rsearch,bv);
#endif
#ifdef REDFIND_EXIMPROVE
		res = ex_improve_example(solver,rsearch,bv);
#endif
		ex = ex_getsatex(solver,rsearch,env);
	}

	hash_free_nodes(env->id_hash);
        hash_destroy(env->id_hash);
	minisat_delete(solver);
	free(env);

	red_freebvars(bv);

	return ex;
}

/* the solver has a counter-example.  here, we run a few extra times with assumptions,
 * saying "it's not exactly that counter-example" and choose one of the resulting
 * counter-examples randomly (a poor approximation of stochastic finite learning
 */
int ex_randomize_example(struct minisat_solver_t *solver, struct redsearch *rsearch, struct red_bvarlist *bv)
{
	minisat_Lit *assumps;
	int num_lits=0;
	int res;
	struct red_bvarlist *tmp=bv;
	struct timeval ts;
	int len;	
	int num=0; /* which example we want to use. if there aren't this many, we use the last */
	int curnum;
	int i;

	gettimeofday(&ts, NULL);
	srandom(ts.tv_usec);

	num = random()%REDFIND_RANDEX;
	if (num==0)
		return minisat_l_True;

	for (tmp=bv; tmp; tmp=tmp->next)
	{
		len=strlen(tmp->pos);
		if (len>3 && tmp->pos[0]=='T' && tmp->pos[1]=='C' &&
		    isdigit(tmp->pos[2]))
			continue;
		num_lits++;
	}

	assumps = malloc(sizeof(minisat_Lit)*num);
	for (i=0;i<num;i++)
		assumps[i]=minisat_mkLit(minisat_newVar(solver));

	for (curnum=0; curnum<num; curnum++)
	{
		ex_forbidthisexample(rsearch,solver, assumps[curnum], bv);
		res = minisat_solve(solver, curnum+1, assumps);
		if (res == minisat_l_False)
		{
#ifdef REDFIND_DEBUG
			printf("   deb: wanted the %dth counter-example, but only had %d.\n",num+1,curnum+1);
#endif
			break;
		}
	}

	free(assumps);
	return minisat_l_True;
}

/* add a clause saying that if assumpLit is assumed to be true, the
   counter-example is not exactly the one that is currently the solver's
   satisfying assignment.
*/
void ex_forbidthisexample(struct redsearch *rsearch, struct minisat_solver_t *solver, minisat_Lit assumpLit, struct red_bvarlist *bv)
{
	struct red_bvarlist *tmp;
	int len;

	minisat_addClause_begin(solver);
	minisat_addClause_addLit(solver, minisat_negate(assumpLit));
	for (tmp=bv; tmp; tmp=tmp->next)
	{
		len = strlen(tmp->pos);
		if (len>3 && tmp->pos[0]=='T' && tmp->pos[1]=='C' &&
                    isdigit(tmp->pos[2]))
                        continue;
		if (tmp->pos && red_minisat_truevar(rsearch,solver,tmp,1))
			minisat_addClause_addLit(solver,
						 minisat_negate(tmp->posLit));
		else if (tmp->pos)
			minisat_addClause_addLit(solver, tmp->posLit);
	}
	minisat_addClause_commit(solver);

	return;
}


/* the solver has a counter-example.  here, we try to minimize (or
 * maximize, or alternate) the number of Boolean variables that are 
 * true (i.e., a simple, greedy attempt at a "simple" counter-example
 */
int ex_improve_example(struct minisat_solver_t *solver, 
		       struct redsearch *rsearch,
		       struct red_bvarlist *bv)
{
	struct red_bvarlist *tmp;

	int i;
	minisat_Lit *all_lits;
	minisat_Lit *assumps;
	int num_assumps=0;
	int num_lits=0;
	int res;
	int len;

	for (tmp=bv; tmp; tmp=tmp->next)
	{
		if (islower(tmp->pos[0]))
			continue; /* ignore constants */
		len=strlen(tmp->pos);
		if (len>3 && tmp->pos[0]=='T' && tmp->pos[1]=='C' && 
		    isdigit(tmp->pos[2]))
			continue; /* ignore TC */
		num_lits++;

	}

	all_lits = malloc(sizeof(minisat_Lit)*num_lits);
	assumps = malloc(sizeof(minisat_Lit)*num_lits);

	for (tmp=bv,i=0; tmp; tmp=tmp->next)
	{
		len = strlen(tmp->pos);
		if (len>3 && tmp->pos[0]=='T' && tmp->pos[1]=='C' && 
                    isdigit(tmp->pos[2]))
                        continue; /* ignore TC */
		if (!islower(tmp->pos[0]))
		{
			if (RF_minex || (RF_altex && !(rsearch->num_ex&1)))
				all_lits[i++]=minisat_negate(tmp->posLit);
			else
				all_lits[i++]=tmp->posLit;
		}
	}

	for (i=0; i<num_lits; i++)
	{
		assumps[num_assumps++]=all_lits[i];
		res = minisat_solve(solver, num_assumps, assumps);
		if (res == minisat_l_False)
		{
			/* no counter-example with this false */
			num_assumps--;
			continue;
		}
	}

	free(all_lits);
	free(assumps);
		
	return res;
}

/* ex_make_part2_tcdef(...)
 * Almost identical to red_maketcform and ex_maketcdef, should be combined */
char *ex_make_part2_tcdef(struct redsearch *rsearch, struct tc_def *tcd, 
			  struct red_bvarlist *bv)
{
	char *res=NULL, *tmp=NULL;
        int *tuple=NULL;
        int i, arity=tcd->tup_arity;
        int darity = arity<<1;
        int outsize=rsearch->outsize;
        int max = de_pow(outsize, arity)-1;

        eval_init_form(tcd->tc_node, tcd->interp, NULL);

	/* first, distance <= 1 */
        while ((tuple=next_tuple(tuple, darity, outsize)))
        {
                tmp = ex_p2_tcp1_dist1(rsearch, tcd, tuple, bv);
                res = red_conjunc(rsearch, res, tmp);
        }

        for (i=2; ;i=(i<<1))
        {
                while ((tuple=next_tuple(tuple,darity, outsize)))
                {
                        tmp = ex_tcp1_disti(rsearch, tcd,tuple, i);
                        res = red_conjunc(rsearch, res, tmp);
                }
                if (i>=max)
                {
                        while ((tuple=next_tuple(tuple, darity, outsize)))
                        {
                                tmp = ex_tcp1_final(tcd, tuple, i);
                                res = red_conjunc(rsearch, res, tmp);
                        }
                        break;
                }
	}

        return res;
}

/* just like ex_tcp1_dist1 and red_tcp1_dist1, should be combined */
char *ex_p2_tcp1_dist1(struct redsearch *rsearch, struct tc_def *tcd, 
		       int *tuple, struct red_bvarlist *ebv)
{
	int tup_arity=tcd->tup_arity;
        int *tupa=tuple, *tupb=tuple+tup_arity;
        int flag=0;

        char *tmp1, *res, *tmp2;
        struct node *tcform;
        struct node *tcargs;
        struct node *tmp;
        struct interp *interp;

        int i;

        for (i=0; i<tup_arity; i++)
                if (tupa[i]!=tupb[i])
                {
                        flag=1;
                        break;
                }

        tmp1 = ex_printtcvar(tcd->num, 1, tupa, tupb, tup_arity);

	if (!flag)
                return tmp1;

        tcform = tcd->tc_node->l->r;
        tcargs = tcd->tc_node->l->l;
        interp = tcd->interp;

        tmp = tcargs;
        for (i=0; ;tmp=tmp->r)
        {
                *(tmp->l->l->ival)=tupa[i++];
                if (++i<tup_arity)
                        *(tmp->l->r->ival)=tupa[i];
                else
                        break;
        }
        if (tup_arity&1) /* odd arity means we split one */
        {
                *(tmp->l->r->ival)=tupb[0];
                i=1;
        }
        else
                i=0;

	for (tmp=tmp->r; ;tmp=tmp->r)
        {
                if (!tmp)
                        break;
                *(tmp->l->l->ival)=tupb[i];
                if (++i<tup_arity)
                        *(tmp->l->r->ival)=tupb[i];
                else
                        break;
        }

	tmp2 = ex_p2part_rec(rsearch, tcform, interp, ebv);

        res = malloc(sizeof(char)*(strlen(tmp1)+strlen(tmp2)+8));
        sprintf(res,"(%s<->(%s))",tmp1,tmp2);
        free(tmp1);
        free(tmp2);
        return res;
}

/* free tc */
void ex_freetcd(struct tc_def *tc)
{
	struct tc_def *tcd;
	struct tc_def *next;

	for (tcd=tc; tcd; tcd=next)
	{
		next = tcd->next;
		if (tcd->interp) /* BDD version may leave NULL */
			free_interp(tcd->interp);
		free(tcd->lits);
		free(tcd);
	}

	return;
}

/* return a SAT formula defining the transitive closure represented by tcd */
char *ex_maketcdef(struct redsearch *rsearch, struct tc_def *tcd,
		   struct red_bvarlist *ebv)
{
	char *res=NULL;
	char *tmp=NULL;
	int *tuple=NULL;
	int i, arity=tcd->tup_arity;
	int darity = arity<<1;
	int size = tcd->size;
	int max = de_pow(size, arity)-1;

	eval_init_form(tcd->tc_node, tcd->interp, NULL);

	/* first, distance <=1 */
	while ((tuple=next_tuple(tuple, darity, size)))
	{
		tmp = ex_tcp1_dist1(rsearch, tcd, tuple, ebv);
		res = red_conjunc(rsearch, res, tmp);

	}

	for (i=2; ;i=(i<<1))
	{
		while ((tuple=next_tuple(tuple,darity, size)))
		{
			tmp = ex_tcp1_disti(rsearch, tcd,tuple, i);
			res = red_conjunc(rsearch, res, tmp);
		}
		if (i>=max)
		{
			while ((tuple=next_tuple(tuple, darity, size)))
			{
				tmp = ex_tcp1_final(tcd, tuple, i);
				res = red_conjunc(rsearch, res, tmp);
			}
			break;
		}
	}

	return res;
}

/* define the initial TC{num}[1]_[tuple] , */
/* tuple is (x-,y-), where each is arity tcd->tup_arity.
 * this is true if x-=y-, and otherwise we substitute into the
 * formula that we're taking the TC of
 */
char *ex_tcp1_dist1(struct redsearch *rsearch, struct tc_def *tcd, int *tuple,
		    struct red_bvarlist *ebv)
{
	int tup_arity=tcd->tup_arity;
	int *tupa=tuple, *tupb=tuple+tup_arity;
	int flag=0;

	char *tmp1, *res, *tmp2;
	struct node *tcform;
	struct node *tcargs;
	struct node *tmp;
	struct interp *interp;

	int i;

	for (i=0; i<tup_arity; i++)
		if (tupa[i]!=tupb[i])
		{
			flag=1;
			break;
		}

	tmp1 = ex_printtcvar(tcd->num, 1, tupa, tupb, tup_arity);

	if (!flag)
		return tmp1;

	tcform = tcd->tc_node->l->r;
	tcargs = tcd->tc_node->l->l;
	interp = tcd->interp;

	tmp = tcargs;
	for (i=0; ;tmp=tmp->r)
	{
		*(tmp->l->l->ival)=tupa[i++];
		if (++i<tup_arity)
			*(tmp->l->r->ival)=tupa[i];
		else
			break;
	}
	if (tup_arity&1) /* odd arity means we split one */
	{
		*(tmp->l->r->ival)=tupb[0];
		i=1;
	}
	else
		i=0;

	for (tmp=tmp->r; ;tmp=tmp->r)
	{
		if (!tmp)
			break;
		*(tmp->l->l->ival)=tupb[i];
		if (++i<tup_arity)
			*(tmp->l->r->ival)=tupb[i];
		else
			break;
	}

	tmp2 = ex_p1rec(rsearch, tcform, interp, ebv);

	res = malloc(sizeof(char)*(strlen(tmp1)+strlen(tmp2)+8));
	sprintf(res,"(%s<->(%s))",tmp1,tmp2);
	free(tmp1);
	free(tmp2);
	return res;
}

/* tuple is arity (tcd->arity<<1) */
/* return "(TC{num}_[tuple]<->TC{num}[d]_[tuple])" */
char *ex_tcp1_final(struct tc_def *tcd, int *tuple, int d)
{
	char *res;

	int num=tcd->num;
	int i, arity=(tcd->tup_arity<<1);
	int len = 3+numdigits(num)+8+numdigits(num)+7+numdigits(d)+
		  ((arity-1)<<1);
	
	for (i=0; i<arity; i++)
		len+=((int)numdigits(tuple[i])<<1);

	res = malloc(sizeof(char)*len);
	sprintf(res,"(TC%d_",num);

	ex_printtupc(res+strlen(res),tuple,arity);
	sprintf(res+strlen(res),"<->TC%d[%d]_",num,d);
	ex_printtupc(res+strlen(res),tuple,arity);
	strcat(res,")");
	return res;
}

/* return the formula 
(TC{num}[d]_[tuple]<->(\Bigvee_z (TC{num}[d/2]_[a,z]&TC{num}[d/2]_[z,b])))
defining TC recursively
 */
/* d is a power of 2 (d>=2) */
char *ex_tcp1_disti(struct redsearch *rsearch,
		    struct tc_def *tcd, int *tuple, int d)
{
	char *res, *tmp, *tmp1, *tmp2;
	int arity=tcd->tup_arity;
	int n=tcd->size;
	int *tupa=tuple, *tupb=tuple+arity, *tupz=NULL;
	int i;
	int flag=0;
	int len;

	tmp = ex_printtcvar(tcd->num, d, tupa, tupb, arity);

	res = malloc(sizeof(char)*(strlen(tmp)+6));
	sprintf(res, "(%s<->(", tmp);

	for (i=0; i<arity; i++)
		if (tupa[i]!=tupb[i])
			break;
	if (i==arity)
	{
		free(res);
		return tmp; /* reflexive, so tupa==tupb is always included */
	}

	free(tmp);

	while ((tupz=next_tuple(tupz, arity, n)))
	{
		tmp1 = ex_printtcvar(tcd->num, (d>>1), tupa, tupz, arity);
		tmp2 = ex_printtcvar(tcd->num, (d>>1), tupz, tupb, arity);
		tmp1 = red_conjunc(rsearch, tmp1, tmp2);
		tmp2 = malloc(sizeof(char)*(strlen(tmp1)+3));
		sprintf(tmp2,"(%s)",tmp1);
		free(tmp1);
		tmp1=tmp2;
		if (flag)
		{
			res = red_disjunc(rsearch, res, tmp1);
			continue;
		}
		len = strlen(res);
		tmp2 = realloc(res, sizeof(char)*(len+strlen(tmp1)+2));
		if (!tmp2)
		{
			printf("r??: No memory\n");
			free(tmp1);
			free(res);
			return NULL;
		}
		strcat(tmp2,tmp1);
		flag=1;
		free(tmp1);
		res=tmp2;
	}		
	
	tmp2 = realloc(res, sizeof(char)*(strlen(res)+3));
	if (!tmp2)
	{
		printf("r??: No memory\n");
		free(res);
		return NULL;
	}

	strcat(tmp2, "))");
	return tmp2;
}

/* Returns "TC{num}[d]_[tupa-tupb]", where tupa and tupb are each arity arity */char *ex_printtcvar(int num, int d, int *tupa, int *tupb, int arity)
{
	char *res;
	int len2=2+numdigits(num)+3+numdigits(d)+2+(arity<<1);
	int i;
	int len;

	for (i=0; i<arity; i++)
		len2 += numdigits(tupa[i])+numdigits(tupb[i]);

	res = malloc(sizeof(char)*len2);
	sprintf(res, "TC%d[%d]_", num, d);

	len=strlen(res);

	ex_printtupc(res+len,tupa,arity);
	len = strlen(res);
	ex_printtupc(res+len-1,tupb,arity);
	res[len-1] = '-';

	return res;
}

/* Add the SAT variables for this tc_def to bv and the hash */
struct red_bvarlist *ex_addtcvars(struct tc_def *tcd,
				  struct minisat_solver_t *solver,
				  struct red_bvarlist *bvars, struct env *env)
{
	struct tc_def *tc;
	struct red_bvarlist *bv=bvars;

	for (tc=tcd; tc; tc=tc->next)
		bv = ex_addtcvars_1(tc, solver, bv, env);
	return bv;
}

struct red_bvarlist *ex_addtcvars_1(struct tc_def *tcd, 
				  struct minisat_solver_t *solver,
				  struct red_bvarlist *bvars, struct env *env)
{
	struct red_bvarlist *bv=bvars;
	int num_tuples;
	int *tuple=NULL;
	int arity = (tcd->tup_arity<<1);
	char *varname;
	int size = tcd->size;
	int i;
	int max;
	int len, len2;

	num_tuples = de_pow(size,arity);
	tcd->lits = malloc(sizeof(minisat_Lit)*num_tuples);
	
	for (i=0; i<num_tuples; i++)
	{
		tcd->lits[i]=minisat_newLit(solver);
		/*minisat_setFrozen(solver, minisat_var(tcd->lits[i]), 
				  minisat_l_True);*/
	}
	i=0;
	while ((tuple=next_tuple(tuple, arity, size)))
	{
		bv = ex_addtcvar(tcd, env, bv, tuple, tcd->lits[i]);
		i++;
	}

	if (i!=num_tuples)
		printf("r??: ERROR!\n");

	/* now we need the ones that say "distance <= i" */

	max = de_pow(size, tcd->tup_arity)-1;
	len2 = 8+numdigits(tcd->num)+numdigits(max<<1)+arity*numdigits(size)+arity;

	for (i=1; ; i=(i<<1))
	{
		while ((tuple = next_tuple(tuple,arity,size)))
		{
			varname = malloc(sizeof(char)*len2);
                	sprintf(varname,"TC%d[%d]_",tcd->num,i);
                	len = strlen(varname);
			ex_printtupc(varname+len,tuple,arity);
			bv = ex_addtolist(bv, varname, solver, env);
		}

		if (i>=max && i>=2)
			break;
	}
	return bv;
}

/* print the tuple as [tuple[0]-tuple[1]-...-tuple[arity-1]] */
void ex_printtupc(char *str, int *tuple, int arity)
{
	int i;
	str[0]='[';
	str[1]='\0';

	for (i=0; i<arity; i++)
	{
		if (i)
			strcat(str,"-");
		sprintf(str+strlen(str),"%d",tuple[i]);
	}
	strcat(str,"]");

	return;
}

/* add "TC{tcd->num}_[tuple]" to the bvarlist and hash as lit */
struct red_bvarlist *ex_addtcvar(struct tc_def *tcd, struct env *env,
				 struct red_bvarlist *bv,
				 int *tuple, minisat_Lit lit)
{
	char *tmp;
	int i, l;
	int arity=(tcd->tup_arity<<1);
	int num=tcd->num;
	int len=6+arity-1+numdigits(num);

	for (i=0; i<arity; i++)
		len+=numdigits(tuple[i]);

	tmp=malloc(sizeof(char)*len);
	sprintf(tmp,"TC%d_[",num);

	for (i=0; i<arity; i++)
	{
		l=strlen(tmp);
		if (i)
			sprintf(tmp+l,"-%d",tuple[i]);
		else
			sprintf(tmp+l,"%d",tuple[i]);
	}
	strcat(tmp,"]");

	return ex_addlittolist(bv, tmp, lit, env);
}

struct red_bvarlist *ex_addlittolist(struct red_bvarlist *bvars, char *tmp, 
				     minisat_Lit lit, struct env *env)
{
	struct red_bvarlist *list=malloc(sizeof(struct red_bvarlist));
	list->pos=tmp;
	list->neg=NULL;
	list->posVar=minisat_var(lit);
	list->posLit = lit;
	list->next = bvars;
	hash_alloc_insert(env->id_hash, tmp, list);
	return list;
}

/* add clauses for each constant having exactly one value */
/* also assert TRUE is true and FALSE is false */
void ex_initsolver(struct minisat_solver_t *solver, struct env *env,
		   struct redsearch *rsearch)
{
	int n=rsearch->n;
	struct vocab *voc=rsearch->p1->voc;
	struct cons_symbol *cons;
	int i, j, len, base=numdigits(n);
	char *tmp, *tmp2, *name;
	minisat_Lit lit, lit2;

	minisat_addClause_begin(solver);
	minisat_addClause_addLit(solver, red_getposLit(env, "TRUE"));
	minisat_addClause_commit(solver);
	minisat_addClause_begin(solver);
	minisat_addClause_addLit(solver, minisat_negate(red_getposLit(env, "FALSE")));
	minisat_addClause_commit(solver);
#ifdef REDFIND_DEBUG2
	red_debug1("TRUE&!FALSE");
#endif
	for (cons=voc->cons_symbols; cons; cons=cons->next)
	{
		len = base+strlen(cons->name)+3;
		tmp = malloc(sizeof(char)*len);
		tmp2= malloc(sizeof(char)*len);
		name = cons->name;

#ifdef REDFIND_DEBUG2
		red_debug1("&(");
#endif
		/* at least one of the variables must be true */
		minisat_addClause_begin(solver);
		for (i=0; i<n; i++)
		{
			sprintf(tmp,"%sis%d",name,i);
			lit = red_getposLit(env, tmp);
			minisat_addClause_addLit(solver, lit);
#ifdef REDFIND_DEBUG2
			if (i>0)
				red_debug2("|%s",tmp);
			else
				red_debug2("%s",tmp);
#endif
		}
		minisat_addClause_commit(solver);
		red_debug1(")");

		/* for all pairs, at least one is false */
		for (i=0; i<n; i++)
		{
			for (j=i+1; j<n; j++)
			{
				minisat_addClause_begin(solver);
				sprintf(tmp,"%sis%d",name,i);
				sprintf(tmp2,"%sis%d",name,j);
                        	lit = red_getposLit(env, tmp);
				lit2= red_getposLit(env, tmp2);
				lit = minisat_negate(lit);
				lit2= minisat_negate(lit2);
				red_debug3("&(!%s|!%s)",tmp,tmp2);
				minisat_addClause_addLit(solver, lit);
				minisat_addClause_addLit(solver, lit2);
				minisat_addClause_commit(solver);
			}
		}
		free(tmp);
		free(tmp2);
	}
#ifdef REDFIND_DEBUG2
	if (voc->cons_symbols)
		red_debug1(" & ");
#endif
	return;
}

struct env *ex_inithash(void)
{
	struct env *env = malloc(sizeof(struct env));
	env->id_hash = hash_create(RED_EXMAXVARS,(hash_comp_t)strcmp,0);
	env->next_id = 0;
	return env;
}

struct red_bvarlist *ex_initbvarlist(struct redsearch *rsearch, 
				     struct minisat_solver_t *solver,
				     struct env *env)
{
	int j;
	char *tmp;
	struct red_bvarlist *bvars=NULL;
	int len;
	struct rel_symbol *rel;
	struct cons_symbol *cons;
	int n=rsearch->n;
	struct vocab *voc=rsearch->p1->voc;

	bvars = ex_addtolist(bvars, dupstr("TRUE"), solver, env);
	bvars = ex_addtolist(bvars, dupstr("FALSE"), solver, env);

	/* Constants cis0,...,cismax */
	for (cons=voc->cons_symbols; cons; cons=cons->next)
		for (j=0; j<n; j++)
		{
			len = strlen(cons->name)+2+numdigits(j)+1;
			tmp = malloc(sizeof(char)*len);
			sprintf(tmp,"%sis%d",cons->name,j);
			bvars = ex_addtolist(bvars, tmp, solver, env);
			/* TODO check bv */
		}

	/* Relations, R[0-0-0],...,R[max-max-max] */
	for (rel=voc->rel_symbols; rel; rel=rel->next)
		bvars = ex_add_pred(solver, env, bvars, rel, n);

	return bvars;
}

struct red_bvarlist *ex_add_pred(struct minisat_solver_t *solver, 
				 struct env *env, struct red_bvarlist *bvars, 
				 struct rel_symbol *rel, int n)
{
	char *tmp;
	int *tuple=NULL;
	char *relname=rel->name;
	int a=rel->arity;
	int baselen;
	int i, len;
	char *ts;
	struct red_bvarlist *bv=bvars;

	baselen = strlen(relname)+2+1+(a-1);
	while ((tuple=next_tuple(tuple, a, n)))
	{
		len=baselen;
		for (i=0; i<a; i++)
			len+=numdigits(tuple[i]);
		ts=malloc(sizeof(char)*len);
		tmp = malloc(sizeof(char)*len);
		sprintf(ts,"%s[",relname);
		for (i=0; i<a; i++)
		{
			if (i)
				strcat(ts,"-");
			sprintf(tmp,"%d",tuple[i]);
			strcat(ts,tmp);
		}
		strcat(ts,"]");
		free(tmp);
		bv = ex_addtolist(bv, ts, solver, env);
		/* check bv!=NULL */
	}

	return bv;
}

/* add tmp (and its negation) to the list, and the hash */
struct red_bvarlist *ex_addtolist(struct red_bvarlist *bvars, char *tmp,
                     struct minisat_solver_t *solver, struct env *hash)
{
        struct red_bvarlist *list=malloc(sizeof(struct red_bvarlist));
	int len=strlen(tmp);

        /* TODO check those mallocs */

	list->pos=tmp;
	list->neg=NULL;
        list->posVar=minisat_newVar(solver);

	if (!(len>3 && tmp[0]=='T' && tmp[1]=='C' && isdigit(tmp[2])))
		minisat_setFrozen(solver, list->posVar, minisat_l_True);
        list->posLit=minisat_mkLit(list->posVar);
        list->next = bvars;

        hash_alloc_insert(hash->id_hash, tmp, list);
        return list;
}

/* Returns phi, which is "\phi_{w\models p1}\oplus\phi_{r(w)\models p2}"
 */
char *ex_makeform(struct redsearch *rsearch, struct red_bvarlist *ebv)
{
	char *phi1 = ex_p1part(rsearch,ebv);
	char *phi2;
	char *res;

	/* TODO check return!=NULL */	
	phi2 = ex_p2part(rsearch,ebv);

	res = malloc((sizeof(char))*(strlen(phi1)+strlen(phi2)+9));
	sprintf(res,"(%s)<->!(%s)",phi1,phi2);
	free(phi1);
	free(phi2);
	return res;
}

char *ex_p1part(struct redsearch *rsearch, struct red_bvarlist *ebv)
{
	/* make an interpretation and call the internal, recursive fct */
	struct interp *interp = new_interp(NULL);
	char *res;
	eval_init_form(rsearch->p1->form,interp,NULL);
	res=ex_p1rec(rsearch,rsearch->p1->form,interp,ebv);
	free_interp(interp);
	return res;
}

char *ex_p1rec(struct redsearch *rsearch, struct node *form, 
	       struct interp *interp, struct red_bvarlist *ebv)
{
	char *tmpl, *tmpr, *res;
	char *dummyvar = ebv->pos;

	switch (form->label)
	{
		case TRUE:
			res = malloc(sizeof(char)*5);
			sprintf(res,"TRUE");
			return res;
		case FALSE:
			res = malloc(sizeof(char)*6);
			sprintf(res, "FALSE");
			return res;
		case NOT:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
			res = malloc(sizeof(char)*(strlen(tmpl)+4));
			sprintf(res,"!(%s)",tmpl);
			free(tmpl);
			return res;
		case AND:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
			tmpr = ex_p1rec(rsearch,form->r,interp,ebv);
			res = malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+6));
			sprintf(res,"(%s)&(%s)",tmpl,tmpr);
			free(tmpl);
			free(tmpr);
			return res;
		case OR:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
                        tmpr = ex_p1rec(rsearch,form->r,interp,ebv);
                        res = malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+6));
                        sprintf(res,"(%s)|(%s)",tmpl,tmpr);
                        free(tmpl);
                        free(tmpr);
                        return res;
		case IMPLIES:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
			tmpr = ex_p1rec(rsearch,form->r,interp,ebv);
			res = malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+7));
			sprintf(res,"(%s)->(%s)",tmpl,tmpr);
			free(tmpl);
			free(tmpr);
			return res;
		case IFF:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
                        tmpr = ex_p1rec(rsearch,form->r,interp,ebv);
                        res = malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+8));
                        sprintf(res,"(%s)<->(%s)",tmpl,tmpr);
                        free(tmpl);
                        free(tmpr);
                        return res;
		case XOR:
			tmpl = ex_p1rec(rsearch,form->l,interp,ebv);
                        tmpr = ex_p1rec(rsearch,form->r,interp,ebv);
                        res = malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+9));
                        sprintf(res,"(%s)<->!(%s)",tmpl,tmpr);
                        free(tmpl);
                        free(tmpr);
                        return res;
		case EXISTS:
			return ex_p1rec_exists(rsearch,form,interp,ebv);
		case FORALL:
			return ex_p1rec_forall(rsearch,form,interp,ebv);
		case PRED:
			return ex_p1rec_pred(rsearch,form,interp);
		case EQUALS:
			return ex_p1rec_eq(rsearch, form, interp);
		case NEQUALS:
			tmpl = ex_p1rec_eq(rsearch, form, interp);
			tmpr = malloc(sizeof(char)*(strlen(tmpl)+4));
			sprintf(tmpr,"!(%s)",tmpl);
			free(tmpl);
			return tmpr;
		case LT:
			/* return ex_p1rec_lt(rsearch,form,interp); */
		case LTE:
			/* return ex_p1rec_lte(rsearch,form,interp); */
		case TC:
			return ex_p1rec_tc(rsearch,form,interp);
		default:
			rsearch->abort=1;
			printf("r01: Sorry, reduction finding for this formula is not implemented.  Results will be incorrect.\n");
			res=malloc(sizeof(char)*((strlen(dummyvar)<<1)+3));
			sprintf(res,"%s|!%s",dummyvar,dummyvar);
			return res;
	}
}

/* this needs to re-use old tc_def if the interpretation hasn't changed
 * significantly and the node is the same
 */
/* we need to make sure no nested TC */
/* we need to enforce no interpreted vars inside tcform */
char *ex_p1rec_tc(struct redsearch *rsearch, struct node *form,
		  struct interp *interp)
{
	struct tc_def *tcd;
	struct node *tcargs, *relargs, *tmpnode;
	int tup_arity;
	int arity;

	struct red_tuple *rtup;
	int *cons_mask;
	char **cons_names;
	int *tuple=NULL;
	int *val;
	int i, n=rsearch->n;
	int num_tups;

	char *res=NULL, *tmp;
	struct tc_def *tcdtemp;

	tcargs = form->l->l;
	relargs = form->r;
	for (tup_arity=0,tmpnode=tcargs; tmpnode; tup_arity++)
        	tmpnode=tmpnode->r;

	num_tups = de_pow(rsearch->n, tup_arity);
	num_tups = num_tups*num_tups;

	tcd=NULL;
	/* should check interps, etc. but we're
	 * strict, see red_checkp1p2(...)
	 */
	for (tcdtemp=rsearch->tc[0];tcdtemp;tcdtemp=tcdtemp->next)
                if (tcdtemp->tc_node==form)
                {
                        tcd=tcdtemp;
                        break;
                }
        if (!tcd)
	{
		tcd = malloc(sizeof(struct tc_def));
		tcd->num = rsearch->num_tc++;
		tcd->tc_node = form;
		tcd->lits = NULL;
		tcd->size = rsearch->n;
		tcd->next = rsearch->tc[0];
		tcd->tup_arity = tup_arity;
		tcd->interp = dup_interp(interp);
	
		rsearch->tc[0]=tcd;
	}

	tmp = malloc(sizeof(char)*(4+numdigits(tcd->num)));
	sprintf(tmp,"TC%d_",tcd->num);

	arity = (tup_arity << 1);
	/* I want to do much like ex_p2_lfpf_pform , iter over the tuples
	 * consistent with the constants -- can reuse most of that here
	 */
	rtup = red_rf_argstotup(rsearch, relargs, arity, 
				rsearch->p1->voc, n, interp, NULL);
	cons_mask = malloc(sizeof(int)*arity);
	val = malloc(sizeof(int)*arity);
	cons_names = malloc(sizeof(char *)*arity);
        for (i=0; i<arity; i++)
                cons_names[i]=NULL;

	ex_rtup_to_consmask(rtup, arity, cons_names, cons_mask, val);

	while ((tuple = ex_nextpf_tuple(tuple, arity, n, val, cons_mask)))
		res = ex_p2_lfpf_join(rsearch, res, tmp, tuple, arity, 
				      cons_mask, cons_names);

	free(cons_mask);
	free(cons_names);
	free(val);
	free(tmp);

	return res;
}

/* looking at rtup, init cons_names, cons_mask, val like ex_pf_tupconsmask
 * would, if it took a red_tuple instead of a char *
 */
/* done by hacking such a char * and using ex_pf_tupconsmask :-( */
void ex_rtup_to_consmask(struct red_tuple *rtup, int arity, char **cons_names, 
			 int *cons_mask, int *val)
{
	struct red_tuple_element *rte;
	char *vlit;
	int i;
	int len=0;

	for (i=0; i<arity; i++)
	{
		rte = rtup->data+i;
		switch (rte->type)
		{
			case 0:
				printf("r??: Variable in ex_rtup_to_consmask not yet interpreted.\n");
				/* rsearch->abort=1; */
				break;
			case 1:
				len+=strlen(rtup->cons_names[rte->a]);
				break;
			case 2:
				len+=numdigits(rte->a);
				break;
			default:
				printf("r??: Unreachable\n");
				break;
		}
	}

	len += 2+(arity-1); /* ]\0 and separators */
	vlit = malloc(sizeof(char)*len);
	vlit[0]='\0';

	for (i=0; i<arity; i++)
        {
                rte = rtup->data+i;
		if (i!=0)
			strcat(vlit,".");
                switch (rte->type)
                {
                        case 0:                                printf("r??: Variable in ex_rtup_to_consmask not yet interpreted.\n");  
                                /* rsearch->abort=1; */
                                break;
                        case 1:
				strcat(vlit, rtup->cons_names[rte->a]);
                                break;
                        case 2:
				sprintf(vlit+strlen(vlit),"%d",rte->a);
                                break;
                        default:
                                printf("r??: Unreachable\n");
                                break;
                }
        }

	strcat(vlit,"]");
	ex_pf_tupconsmask(vlit,strlen(vlit),NULL,arity,cons_mask,cons_names,val);
	free(vlit);
	return;
}

char *ex_p1rec_eq(struct redsearch *rsearch, struct node *form, 
		  struct interp *interp)
{
	int leftinc_c=0;  /* constants are complicated, we need to use bvars*/
        int rightinc_c=0; /* if there are no constants on a side, we can just*/
                          /* teval it, which is much easier */
	int left=0;
        int right=0;
        leftinc_c = term_has_cons(form->l);
        rightinc_c = term_has_cons(form->r);

	if (!leftinc_c && !rightinc_c)
        {
                left = teval(form->l, interp, NULL);
                right = teval(form->r, interp, NULL);
                if (left==right)
                        return ex_make_rf_form_true();
                return ex_make_rf_form_false();
        }

	if (leftinc_c && !rightinc_c)
        {
                right = teval(form->r, interp, NULL);
                return ex_p1make_teqi(rsearch, form->l, right);
        }

        if (!leftinc_c && rightinc_c)
        {
                left = teval(form->l, interp, NULL);
                return ex_p1make_teqi(rsearch, form->r, left);
        }

	else /* the trickiest in general */
		return ex_p1make_teqt(rsearch, form);
}

/* we have term=term, for terms with constants */
/* For now we assume that the terms are single constant symbols. */
char *ex_p1make_teqt(struct redsearch *rsearch, struct node *form)
{
	char *leftc, *rightc;
        struct node *left, *right;
	int n=rsearch->n;
	int i;
	int base;

	char *res=NULL, *tmp;

	left=form->l;
	right=form->r;

	if (left->label!=CONSTANT || right->label!=CONSTANT)
        {
		rsearch->abort=1;
                printf("r02: Reduction finding with non-atomic terms containing constants is not yet supported.\n");
                return ex_make_rf_form_true();
	}

	leftc = (char *)left->data;
	rightc = (char *)right->data;

	if (!strcmp(leftc,rightc))
		return ex_make_rf_form_true();

	base = strlen(leftc)+2+1+strlen(rightc)+2+1+2; /* (cis%d&cis%d) without
							* length of %d */
	for (i=0; i<n; i++)
	{
		tmp = malloc(sizeof(char)*(base+(numdigits(i)*2)));
		sprintf(tmp, "(%sis%d&%sis%d)", leftc, i, rightc, i);
		res = red_disjunc(rsearch, res, tmp);
	}

	return res;
}

/* we have term=value, for some term with constants */
/* For now we assume that term is a single constant symbol. */
char *ex_p1make_teqi(struct redsearch *rsearch, struct node *term, int val)
{
	char *name;
	char *res;

	if (term->label!=CONSTANT)
        {
		rsearch->abort=1;
                printf("r03: Sorry, complicated terms containing constants are not supported yet.\n");
                return ex_make_rf_form_true();
        }

	name = (char *)term->data;
	res=malloc(sizeof(char)*(strlen(name)+2+numdigits(val)+1));
	sprintf(res,"%sis%d",name,val);
	return res;
}

/* Sentence, so all terms can be teval'd or contain constants.
 */
char *ex_p1rec_pred(struct redsearch *rsearch, struct node *form,
                    struct interp *interp)
{
	struct node *relargs=form->r;
	char *relname = (char *)form->data;
	struct vocab *voc=rsearch->p1->voc;
	char *res;

	int n=rsearch->n;
	struct rel_symbol *rs;
	int arity;
	struct red_tuple *rtup;
	int i;

	for (rs=voc->rel_symbols; rs; rs=rs->next)
		if (!strcmp(rs->name, relname))
			break;

	if (!rs)
	{
		rsearch->abort=1;
		printf("r04: redfind can't evaluate %s\n",relname);
		return ex_make_rf_form_true();
	}

	arity=rs->arity;

	rtup = red_rf_argstotup(rsearch,relargs,arity, voc, n, interp, relname);

	for (i=0; i<arity; i++)
	{
		if ((rtup->data+i)->type==1)
		{
			printf("r??: Sorry, constants in predicates not supported yet.\n");
			rsearch->abort=1;
			red_freertup(rtup);
			return ex_make_rf_form_true();
		}
	}
	if (!rtup)
		return ex_make_rf_form_false();

	res = ex_p1make_predform(/*rsearch,*/ relname, rtup); 
	red_freertup(rtup);
	return res;
}

/* Return the formula for relname(rtup) */
char *ex_p1make_predform(/*struct redsearch *rsearch,*/ 
			 char *relname, 
		         struct red_tuple *rtup)
{
	char *res, *tmp;
	int len;

	len = red_tupstrlen(rtup);
	tmp = ex_printtup(rtup, len);
	
	len += strlen(relname)+3;
	res = malloc(sizeof(char)*(len));
	sprintf(res,"%s%s",relname,tmp);
	free(tmp);

	return res;
}


char *ex_printtup(const struct red_tuple *tuple, int len)
{
	int i;
        int arity=tuple->arity;
        char **cnames = tuple->cons_names;
        struct red_tuple_element *el=tuple->data;
        char *tmp=malloc(sizeof(char)*(len+1));
        char *tmp2=malloc(sizeof(char)*(len+1));
        if (!tmp)
        {
                printf("r05: No memory.\n");
                return 0;
        }
        if (!tmp2)
        {
                printf("r06: No memory.\n");
                return 0;
        }
        tmp[0]='[';
        tmp[1]='\0';

	for (i=0; i<arity; i++)
        {
                if (el[i].type==1)
                {
                        strcat(tmp,cnames[el[i].a]);
                        strcat(tmp,"-");
                }
                else /* assert (el[i].type==2) */
                {
                        sprintf(tmp2,"%d-",el[i].a);
                        strcat(tmp,tmp2);
                }
        }
        tmp[strlen(tmp)-1]=']';
        free(tmp2);
        return tmp;
}

/* \forall x:\phi === !\exists x:!\phi */
char *ex_p1rec_forall(struct redsearch *rsearch, struct node *form,
		      struct interp *interp, struct red_bvarlist *ebv)
{
	char *tmp, *ret;
	struct node *not = node(NOT, form->r, 0);
        if (!not)
                return NULL;
        form->r=not;
        form->label = EXISTS;
	tmp = ex_p1rec_exists(rsearch,form,interp,ebv);
	form->r = not->l;
        form->label = FORALL;
        free(not);

	ret = malloc(sizeof(char)*(strlen(tmp)+4));
	sprintf(ret,"!(%s)",tmp);
	free(tmp);
	return ret;
}

/* return the formula, using a disjunction over the possible values of the
 * variable.
 */
/* for each tuple of the variables, we include a clause "(restr&phi)" if
 * restr exists, and "(phi)" otherwise.
 */
char *ex_p1rec_exists(struct redsearch *rsearch, struct node *form, 
		      struct interp *interp, struct red_bvarlist *ebv)
{
	char **varnames;
	int size=rsearch->n;
	int arity=0;
        int i;
        /* int tmpi; */
	int res=0;
	char *tmpl, *tmpr, *ret=NULL, *tmp;

        struct node *restr = form->l->r;
        struct node *varlist = form->l->l;
        struct node *phi = form->r;
        struct node *tnode=varlist;
        int *first;
	int *old_values;
	int **values;

	while (tnode)
        {
                arity++;
                tnode = tnode->r;
        }

	old_values = malloc(arity * sizeof(int));
	values = malloc(arity*sizeof(int *));
	varnames = malloc(arity*sizeof(char *));

        tnode = varlist;

        for (i=0; i<arity; i++)
        {
                varnames[i]=tnode->data;
		values[i] = tnode->ival;
		old_values[i] = *(values[i]);
		*(values[i])=0;
                tnode = tnode->r;
        }

	first = values[0];
	*first = -1;

        /* tmpi = arity-1; */

	while (1)
        {
            (*first)++;
            if (*first>=size)
            {
                    *first=0;
		    res=0;
                    for (i=1; i<arity; i++)
                    {
			    res = ++(*values[i]);
                            if (res < size)
                                    break;
                            res = *(values[i]) = 0;
                    }
                    if (!res && i==arity)
                            break;
            }
	    if (restr)
		tmpl = ex_p1rec(rsearch,restr,interp,ebv);
	    tmpr = ex_p1rec(rsearch,phi,interp,ebv);

	    if (restr)
	    {
		if (!ret)
		{
			ret=malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+8));
			sprintf(ret,"((%s)&(%s))",tmpl,tmpr);
			free(tmpl);
			free(tmpr);
			continue;
		}
		tmp=malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+
					 strlen(ret)+9));
		sprintf(tmp,"%s|((%s)&(%s))",ret,tmpl,tmpr);
		free(tmpl);
		free(tmpr);
		free(ret);
		ret=tmp;
		continue;
	    }
	    else
	    {
		if (!ret)
		{
			ret=tmpr;
			continue;
		}
		tmp = malloc(sizeof(char)*(strlen(tmpr)+strlen(ret)+4));
		sprintf(tmp,"%s|(%s)",ret,tmpr);
		free(tmpr);
		free(ret);
		ret=tmp;
		continue;
	    }
       }
	for (i=0; i<arity; i++)
		*(values[i])=old_values[i];
	free(values);
       /* free(vartuple); */
       free(old_values);
       free(varnames);
       return ret;
}

struct example *get_any_example(int n, const struct bquery *p1)
{
        struct structure *ex;
        struct example *e;
        struct rel_symbol *rs;
        struct cons_symbol *cs;
        struct vocab *voc = p1->voc;
        struct id *hash_data;
        struct hnode_t *hnode;
        char *inp;
        void *bufstate;
        struct interp *interp;
        char name[6];
        char c;
	int i, max, arity;
	struct relation *rel;

        int len=0,t;
        for (c='A'; c<='Z'; c++)
        {
                for (t=0; t<999; t++)
                {
                        sprintf(name,"%c%d",c,t);
                        hnode = hash_lookup(cur_env->id_hash, name);
                        if (!hnode)
				break;
                }
                if (!hnode)
                        break;
        }

        len = 4+2+3+1+10+strlen(voc->name)+1+numdigits(n)+1;
        /* A???:=new structure{vocab,n,*/
        for (rs=voc->rel_symbols; rs; rs=rs->next)
                len+=strlen(rs->name)+1+numdigits(rs->arity)+7;
                /* NAME:arity is \f, */
        for (cs=voc->cons_symbols; cs; cs=cs->next)
                len+=strlen(cs->name)+4; /*NAME:=0,*/
        len+=4; /*}.\n\0*/

        inp = malloc(sizeof(char)*len);

        sprintf(inp,"%s:=new structure{%s,%d",name,voc->name,n);
        for (rs=voc->rel_symbols; rs; rs=rs->next)
        {
                t=strlen(inp);
                sprintf(inp+t,",%s:%d is \\f",rs->name,rs->arity);
        }
	for (cs=voc->cons_symbols; cs; cs=cs->next)
        {
                t=strlen(inp);
                sprintf(inp+t,",%s:=0",cs->name);
        }
        strcat(inp,"}.\n");

        INIT_COMMAND(inp);
        free(inp);

        hnode = hash_lookup(cur_env->id_hash, name);
        hash_data = (struct id*)hnode_get(hnode);

        ex = (struct structure *)hash_data->def;

        hash_delete_free(cur_env->id_hash, hnode);
        /* free(hash_data->name); */
        free(hash_data);

        e = malloc(sizeof(struct example));
        e->a=ex;

        interp = new_interp(ex);
	e->p1=eval(p1->form, interp, ex);
        free_interp(interp);

	/* we need to make all caches full */
	for (rel=ex->rels; rel; rel=rel->next)
	{
		arity = rel->arity;
		max = de_pow(n,arity);
		for (i=0; i<max; i++)
			rel->cache[i]=0;
	}

        return e;
}

/* solver has a satisfying assignment representing a counter-example.
 * Return it as a (struct example *)
 */
struct example *ex_getsatex(struct minisat_solver_t *solver,
			    struct redsearch *rsearch,
			    struct env *env)
{
	struct vocab *voc=rsearch->p1->voc;
	struct structure *ex;
	struct example *e;
	struct rel_symbol *rs;
	struct relation *rel;
	struct cons_symbol *cs;
	struct constant *cons;
	int i,n=rsearch->n;
	int *tuple;
	int a;
	char c;
	struct interp *interp;

	void *bufstate;
	char *inp;
	char name[6];

	char *relname, *consname;
	struct id *hash_data;
        struct hnode_t *hnode;

        int len=0,t;
        for (c='A'; c<='Z'; c++)
        {
                for (t=0; t<999; t++)
                {
                        sprintf(name,"%c%d",c,t);
                        hnode = hash_lookup(cur_env->id_hash, name);
                        if (!hnode)
                                break;
                }
                if (!hnode)
                        break;
        }

	len = 4+2+3+1+10+strlen(voc->name)+1+numdigits(n)+1;
        /* A???:=new structure{vocab,n,*/
        for (rs=voc->rel_symbols; rs; rs=rs->next)
                len+=strlen(rs->name)+1+numdigits(rs->arity)+7;
                /* NAME:arity is \f, */
        for (cs=voc->cons_symbols; cs; cs=cs->next)
                len+=strlen(cs->name)+4; /*NAME:=0,*/
        len+=4; /*}.\n\0*/

        inp = malloc(sizeof(char)*len);

        sprintf(inp,"%s:=new structure{%s,%d",name,voc->name,n);
        for (rs=voc->rel_symbols; rs; rs=rs->next)
        {
                t=strlen(inp);
                sprintf(inp+t,",%s:%d is \\f",rs->name,rs->arity);
        }
        for (cs=voc->cons_symbols; cs; cs=cs->next)
        {
                t=strlen(inp);
                sprintf(inp+t,",%s:=0",cs->name);
        }
        strcat(inp,"}.\n");

	INIT_COMMAND(inp);
        free(inp);

        hnode = hash_lookup(cur_env->id_hash, name);
        hash_data = (struct id*)hnode_get(hnode);

        ex = (struct structure *)hash_data->def;

        hash_delete_free(cur_env->id_hash, hnode);
        /* free(hash_data->name); */
        free(hash_data);

        e = malloc(sizeof(struct example));
        e->a=ex;

	for (rs=voc->rel_symbols,i=0; rs; rs=rs->next)
	{
		a=rs->arity;
		relname = rs->name;
		rel = get_relation(relname, NULL, ex);
		tuple = NULL;
		i=0;
		while ((tuple=next_tuple(tuple,a,n)))
			rel->cache[i++]=ex_getrelval(solver,env,relname,a,tuple);
	}

	for (cs=voc->cons_symbols; cs; cs=cs->next)
	{
		consname = cs->name;
		cons = get_constant(consname, ex);
		cons->value = ex_getconsval(solver,env,consname,n);
	}

	interp = new_interp(ex);
	free_tc_caches(rsearch->p1->form);
        e->p1=eval(rsearch->p1->form, interp, ex);
	free_tc_caches(rsearch->p1->form);
        free_interp(interp);

	/* TODO check for any other frees and check all mallocs */
	return e;
}

/* return the value of consname in the satisfying assignment of solver */
int ex_getconsval(struct minisat_solver_t *solver, struct env *env, 
		  const char *consname, int n)
{
	int i,res=0;
	minisat_Lit lit;
	char *varname=malloc(sizeof(char)*(strlen(consname)+numdigits(n-1)+1));
	for (i=0; i<n; i++)
	{
		sprintf(varname,"%sis%d",consname,i);
		lit = red_getposLit(env,varname);
		if (minisat_modelValue_Lit(solver,lit)==minisat_l_True)
		{
			res=i;
			break;
		}
	}
	free(varname);
	return res;
}

/* Returns the value (0,1) of relname[tuple] in the satisfying assignment */ 
int ex_getrelval(struct minisat_solver_t *solver, struct env *env,
		 const char *relname, int a, int *tuple)
{
	char *varname;
	char *tmp, *tmp2;
	int len;
	int i;
	minisat_Lit lit;

	len = strlen(relname)+3+(a-1);
	for (i=0; i<a; i++)
		len+=numdigits(tuple[i]);
	varname = malloc(sizeof(char)*len);
	tmp = malloc(sizeof(char)*len);
	sprintf(varname,"%s[",relname);
	for (i=0; i<a; i++)
	{
		if (i!=0)
			strcat(varname,"-");
		sprintf(tmp,"%s%d",varname,tuple[i]);
		tmp2=varname;
		varname=tmp;
		tmp=tmp2;
	}
	free(tmp);
	strcat(varname,"]");
	
	lit = red_getposLit(env,varname);
	free(varname);
        return (minisat_modelValue_Lit(solver,lit)==minisat_l_True);
}

void print_bvars(struct red_bvarlist *bv, struct minisat_solver_t *solver,
		 struct env *env)
{
	int flag=0;
	struct red_bvarlist *b;

	for (b=bv; b; b=b->next)
	{
		printf(" %s    %d",b->pos,b->posLit);
		if (env)
			printf("\t%d", red_getposLit(env, b->pos));
		if (flag)
			printf("\t%d", minisat_modelValue_Lit(solver,
					       red_getposLit(env,b->pos)));
		if (b->neg)
			printf("\t%s    %d",b->neg,b->negLit);
		printf("\n");
	}
	return;
}

/* return (in Limboole format) the formula \phi_{r(w)\in p2}, where
 * r is given by the satisying assignment of rsearch->solver, and
 * our Boolean variables give w.
 */
char *ex_p2part(struct redsearch *rsearch, struct red_bvarlist *ebv)
{
	struct interp *interp = new_interp(NULL);
	struct node *form = rsearch->p2->form;
	char *res;

	interp = add_symb_to_interp(interp, "max", rsearch->outsize-1);
	eval_init_form(form,interp,NULL);

	res = ex_p2part_rec(rsearch, form, interp, ebv);
	free_interp(interp);
	return res;
}

char *ex_p2part_rec(struct redsearch *rsearch, struct node *form,
		    struct interp *interp, struct red_bvarlist *ebv)
{
	char *tmp1, *tmp2, *tmp3;


	switch (form->label)
	{
		case TRUE:
			tmp1 = malloc(sizeof(char)*5);
			sprintf(tmp1,"TRUE");
			return tmp1;
		case FALSE:
			tmp1 = malloc(sizeof(char)*6);
			sprintf(tmp1, "FALSE");
			return tmp1;
		case NOT:
			tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
			tmp2 = malloc(sizeof(char)*(strlen(tmp1)+4));
			sprintf(tmp2, "!(%s)",tmp1);
			free(tmp1);
			return tmp2;
		case AND:
			tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
			tmp2 = ex_p2part_rec(rsearch, form->r, interp, ebv);
			return red_binform(rsearch, tmp1, tmp2, "&");
		case OR:
			tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
                        tmp2 = ex_p2part_rec(rsearch, form->r, interp, ebv);
                        return red_binform(rsearch, tmp1, tmp2, "|");
		case IMPLIES:
                        tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
                        tmp2 = ex_p2part_rec(rsearch, form->r, interp, ebv);
                        return red_binform(rsearch, tmp1, tmp2, "->");
		case IFF:
                        tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
                        tmp2 = ex_p2part_rec(rsearch, form->r, interp, ebv);
                        return red_binform(rsearch, tmp1, tmp2, "<->");
		case XOR:
			tmp1 = ex_p2part_rec(rsearch, form->l, interp, ebv);
			tmp2 = ex_p2part_rec(rsearch, form->r, interp, ebv);
			tmp3 = malloc(sizeof(char)*(strlen(tmp2)+4));
			sprintf(tmp3, "!(%s)", tmp2);
			free(tmp2);
			return red_binform(rsearch, tmp1, tmp3, "<->");

		case PRED:
			return ex_p2part_pred(rsearch, form, interp);

		case EXISTS:
			return ex_p2part_exists(rsearch, form, interp, ebv);
		case FORALL:
			return ex_p2part_forall(rsearch, form, interp, ebv);
		case EQUALS:
			return ex_p2part_equals(rsearch, form, interp);
		case NEQUALS:
			tmp1 = ex_p2part_equals(rsearch, form, interp);
			tmp2 = malloc(sizeof(char)*(strlen(tmp1)+4));
			sprintf(tmp2,"!(%s)",tmp1);
			free(tmp1);
			return tmp2;

		case TC:
			return ex_p2part_tc(rsearch, form, interp);

		default:
			rsearch->abort=1;
			printf("r07: Reduction-finding not implemented on such formulas.\n");
			return ex_make_rf_form_true();
	}
}

/* ex_p2part_tc(...)
 * Returns the TC variable TC%d_[TUP]
 * this needs to re-use old tc_def, etc., same as ex_p1rec_tc and
 * make_rf_tc.  Based on those; similar parts should be combined
 */
char *ex_p2part_tc(struct redsearch *rsearch, struct node *form,
		   struct interp *interp)
{
	struct tc_def *tcd;
        struct node *tcargs, *relargs, *tmpnode;
        int tup_arity;
        int arity;

        struct red_tuple *rtup;
        int *cons_mask;
        char **cons_names;
        int *tuple = NULL;
        int *val;
        int i, outsize=rsearch->outsize;
        int num_tups;

        char *res=NULL, *tmp;

	struct tc_def *tcdtemp;

        tcargs = form->l->l;
        relargs = form->r;
        for (tup_arity=0, tmpnode=tcargs; tmpnode; tup_arity++)
                tmpnode=tmpnode->r;

	 num_tups = de_pow(rsearch->n, tup_arity);
        num_tups = num_tups*num_tups;

	tcd=NULL;
	/* should check interps, etc.
	 * but we are strict (see red_checkp1p2(...))
	 */
	for (tcdtemp=rsearch->tc[1];tcdtemp;tcdtemp=tcdtemp->next)
                if (tcdtemp->tc_node==form)
                {
                        tcd=tcdtemp;
                        break;
                }
        if (!tcd)
	{
	        tcd = malloc(sizeof(struct tc_def));
        	tcd->num = rsearch->num_tc++;
        	tcd->tc_node = form;
        	tcd->lits = NULL;
        	tcd->size = outsize;
        	tcd->next = rsearch->tc[1];
        	tcd->tup_arity = tup_arity;
        	tcd->interp = dup_interp(interp);

		rsearch->tc[1]=tcd;
	}

        tmp = malloc(sizeof(char)*(4+numdigits(tcd->num)));
        sprintf(tmp,"TC%d_",tcd->num);

        arity = (tup_arity << 1);
        /* I want to do much like ex_p2_lfpf_pform , iter over the tuples
         * consistent with the constants -- can reuse most of that here
         */
        rtup = red_rf_argstotup(rsearch, relargs, arity,
                                rsearch->p2->voc, outsize, interp, NULL);
        cons_mask = malloc(sizeof(int)*arity);
        val = malloc(sizeof(int)*arity);
        cons_names = malloc(sizeof(char *)*arity);
        for (i=0; i<arity; i++)
                cons_names[i]=NULL;

        ex_rtup_to_consmask(rtup, arity, cons_names, cons_mask, val);

#if 0
	for (i=0; i<arity; i++)
	{
		if (cons_names[i])
		{
			printf("r??: Sorry, constants in TC args not supported yet\n");
			rsearch->abort=1;
			free(cons_mask);
			free(cons_names);
			free(val);
			free(tmp);
			return ex_make_rf_form_true();
		}
	}
#endif

	while ((tuple = ex_nextpf_tuple(tuple, arity, outsize, val, cons_mask)))
                res = ex_p2_tc_join(rsearch, res, tmp, tuple, arity,
                                    cons_mask, cons_names, interp);

        free(cons_mask);
        free(cons_names);
        free(val);
        free(tmp);

        return res;
}

/* ex_p2_tc_join(...)
 * Like red_p2_lfpf_join, based on ex_p2_lfpf_join, common bits should be 
 * combined.
 */
char *ex_p2_tc_join(struct redsearch *rsearch, char *res,
                    const char *relname, const int *tuple, int arity,
                    const int *cons_mask, char **cons_names,
		    struct interp *interp)
{
char *tmp, *tmp1;
        int len, i, maxc=0;
        char **cons_defs=malloc(sizeof(char *)*arity);
#if 0
        int j, k=rsearch->k, *ctup=malloc(sizeof(int)*k);
#endif
        struct node *fakenode;

        for (i=0; i<arity; i++)
                cons_defs[i]=NULL;

        len = 2+strlen(relname)+3+(arity-1);

        for (i=0; i<arity; i++)
        {
                len+=numdigits(tuple[i]);
                if (cons_mask[i]>maxc)
                {
                        maxc=cons_mask[i];
                        fakenode = node(EQUALS, snode(CONSTANT, cons_names[i]),
                                        inode(NUMBER, tuple[i]));
                        cons_defs[i] = ex_p2part_equals(rsearch, fakenode, 
							interp);
                        free(fakenode->l->data);
                        free(fakenode->r->data);
			free(fakenode->l);
                        free(fakenode->r);
                        free(fakenode);
                        fakenode = NULL;
                        len+=4+strlen(cons_defs[i]);
                }
        }
        /* len is enough for the new part of the formula */
        tmp = malloc(sizeof(char)*len);
        tmp1 = malloc(sizeof(char)*len);
        sprintf(tmp,"(%s[",relname);
        maxc=0;

	for (i=0; i<arity; i++)
        {
                if (i)
                        sprintf(tmp1,"-%d",tuple[i]);
                else
                        sprintf(tmp1,"%d",tuple[i]);
                strcat(tmp,tmp1);
        }
        strcat(tmp,"]");

        for (i=0; i<arity; i++)
                if (cons_mask[i]>maxc)
                {
                        maxc=cons_mask[i];
                        sprintf(tmp1, "&(%s)", cons_defs[i]);
#if 0
			sprintf(tmp1,"&cons_%s_[",cons_names[i]);
                        ctup = cindex_to_tuple(ctup,tuple[i],k,rsearch->n);
                        for (j=0; j<k; j++)
                        {
                                if (j)
                                        sprintf(tmp1+strlen(tmp1),".%d",ctup[j]);
                                else
                                        sprintf(tmp1+strlen(tmp1),"%d",ctup[j]);
                        }
#endif
                        strcat(tmp,tmp1);
                }

        strcat(tmp,")");
        free(tmp1);

        return red_disjunc(rsearch, res, tmp);
}

/* based on ex_p1rec_exists.
 * return the formula, using a disjunction over the possible values of the
 * variable.
 */
/* for each tuple of the variables, we include a clause "(restr&phi)" if
 * restr exists, and "(phi)" otherwise.
 */
char *ex_p2part_exists(struct redsearch *rsearch, struct node *form,
		       struct interp *interp, struct red_bvarlist *ebv)
{
	char **varnames;
        int size=rsearch->outsize;
        int arity=0;
        int i;
        /* int tmpi; */
        int res=0;
        char *tmpl, *tmpr, *ret=NULL, *tmp;

        struct node *restr = form->l->r;
        struct node *varlist = form->l->l;
        struct node *phi = form->r;
        struct node *tnode=varlist;
        int *first;
	int *old_values;
	int **values;

	while (tnode)
        {
                arity++;
                tnode = tnode->r;
        }

	old_values = malloc(sizeof(int)*arity);
        varnames = malloc(arity*sizeof(char *));
	values = malloc(sizeof(int *)*arity);
        
	tnode = varlist;

        for (i=0; i<arity; i++)
        {
                varnames[i]=tnode->data;
		values[i]=tnode->ival;
		old_values[i]=*(values[i]);
		*(values[i])=0;
                tnode = tnode->r;
        }

	first = values[0];
	*first = -1;
        /* tmpi = arity-1; */

        while (1)
        {
            (*first)++;
            if (*first>=size)
            {
                    *first=0;
		    res=0;
                    for (i=1; i<arity; i++)
                    {
                            res=++(*values[i]);
                            if (res < size)
                                    break;
                            res=(*values[i]) = 0;
                    }
                    if (!res && i==arity)
                            break;
            }

	    if (restr)
                tmpl = ex_p2part_rec(rsearch,restr,interp,ebv);
            tmpr = ex_p2part_rec(rsearch,phi,interp,ebv);

            if (restr)
            {
                if (!ret)
                {
                        ret=malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+8));
                        sprintf(ret,"((%s)&(%s))",tmpl,tmpr);
                        free(tmpl);
                        free(tmpr);
                        continue;
                }
                tmp=malloc(sizeof(char)*(strlen(tmpl)+strlen(tmpr)+
                                         strlen(ret)+9));
                sprintf(tmp,"%s|((%s)&(%s))",ret,tmpl,tmpr);
                free(tmpl);
                free(tmpr);
                free(ret);
                ret=tmp;
                continue;
            }
	    else
            {
                if (!ret)
                {
/*                        ret=tmpr; */
			ret = malloc(sizeof(char)*(strlen(tmpr)+3));
			sprintf(ret,"(%s)",tmpr);
			free(tmpr);
                        continue;
                }
                tmp = malloc(sizeof(char)*(strlen(tmpr)+strlen(ret)+4));
                sprintf(tmp,"%s|(%s)",ret,tmpr);
                free(tmpr);
                free(ret);
                ret=tmp;
                continue;
            }
       }
       for (i=0; i<arity; i++)
		*values[i]=old_values[i];
       free(values);
       free(old_values);
       /* free(vartuple); */
       free(varnames);
       return ret;
}

/* \forall x:\phi === !\exists x:!\phi */
/* based on ex_p1rec_forall, which is based on eval_forall */
char *ex_p2part_forall(struct redsearch *rsearch, struct node *form,
		       struct interp *interp, struct red_bvarlist *ebv)
{
	char *tmp, *ret;
        struct node *not = node(NOT, form->r, 0);
        if (!not)
                return NULL;
        form->r=not;
        form->label = EXISTS;
        tmp = ex_p2part_exists(rsearch,form,interp,ebv);
        form->r = not->l;
        form->label = FORALL;
        free(not);

        ret = malloc(sizeof(char)*(strlen(tmp)+4));
        sprintf(ret,"!(%s)",tmp);
        free(tmp);
        return ret;
}

/* Assume form is an EQUALS node.  return the relevant formula
 * r(w)\models p2, where r is the reduction given in the satisfying
 * assignment of rsearch->solver (needed if the formula contains
 * constants), and w is the counter-example we're looking for.
 */
/* Based on ex_p1rec_eq, */
char *ex_p2part_equals(struct redsearch *rsearch, struct node *form,
		       struct interp *interp)
{
	struct node *leftn = form->l;
	struct node *rightn = form->r;
	int leftinc_c=0;  /* constants are complicated, we need to use bvars*/
        int rightinc_c=0; /* if there are no constants on a side, we can just*/
                          /* teval it, which is much easier */
        int left=0;
        int right=0;
        leftinc_c = term_has_cons(leftn);
        rightinc_c = term_has_cons(rightn);

	if (!leftinc_c && !rightinc_c)
        {
                left = teval(leftn, interp, NULL);
                right = teval(rightn, interp, NULL);
                if (left==right)
                        return ex_make_rf_form_true();
                return ex_make_rf_form_false();
        }

	if (leftinc_c && !rightinc_c)
        {
                right = teval(rightn, interp, NULL);
                return ex_p2make_teqi(rsearch, leftn, right);
        }

        if (!leftinc_c && rightinc_c)
        {
                left = teval(leftn, interp, NULL);
                return ex_p2make_teqi(rsearch, rightn, left);
        }

	rsearch->abort=1;
	printf("r08: Sorry, constants equal to constants is not implemented yet.\n");
	rsearch->abort=1;
	return ex_make_rf_form_true();
	/* return ex_p2make_teqt(rsearch, form, interp, ebv); */
}

/* we have term=value, for some term with constants.
 * for now, we assume that term is a single constant symbol.
 */
char *ex_p2make_teqi(struct redsearch *rsearch, struct node *term, int value)
{
	char *name;
	char *var;
	struct red_tuple *rtup=NULL;
	int *xtup;
	int hascons=0;
	int k=rsearch->k;
	int n=rsearch->n;
	int i;
	struct red_tuple_element *el;
	char *nm;
	int len;
	char *res=NULL, *tmp;
	
	if (term->label!=CONSTANT)
	{
		rsearch->abort=1;
		printf("r09: Sorry, complicated terms with constants are not supported yet.\n");
		rsearch->abort=1;
		return ex_make_rf_form_true();
	}


	xtup = malloc(sizeof(int)*k);
	xtup = cindex_to_tuple(xtup, value, k, rsearch->n);
	name = (char *)term->data;

	/* name = value in the image of the reduction if cons_%s_[A,B,...],
	 * where [A,B,...] is cindex_to_tuple(value), or some equivalent
	 * tuple (i.e., if cisA, then cons_%s_[c,B,...] is also okay.
	 */

	var = ex_get_redcons_truevar(rsearch, name);
	rtup = red_nextconstuple(rsearch, rtup, k, n, 
				 rsearch->p1->voc->cons_symbols);
	rtup = ex_getconstuple(var, rtup);

	for (i=0; i<k; i++)
	{
		el = rtup->data+i;
		if (el->type==1)
		{
			hascons++;
			continue;
		}
		if (el->type==2)
		{
			if (el->a!=xtup[i])
			{
				red_freertup(rtup);
				free(xtup);
				return ex_make_rf_form_false();
			}
			continue;
		}
		rsearch->abort=1;
		red_freertup(rtup);
		free(xtup);
		printf("r10: Invalid element in a cbvar tuple\n");
		return ex_make_rf_form_false();
	}

	if (!hascons)
	{
		red_freertup(rtup);
		free(xtup);
		return ex_make_rf_form_true();
	}

	for (i=0; i<k; i++)
        {
                el = rtup->data+i;
		if (el->type!=1)
			continue;
		nm = rtup->cons_names[el->a];
		len = strlen(nm)+3+numdigits(xtup[i]);
		tmp = malloc(sizeof(char)*len);
		sprintf(tmp, "%sis%d", nm, xtup[i]);
		res = red_conjunc(rsearch,res,tmp);
	}

	tmp = malloc(sizeof(char)*(strlen(res)+3));
	sprintf(tmp,"(%s)",res);
	free(res);
	res=tmp;
	return res;
}

/* form is a PRED node.  Return the relevant formula (r(w)\models p2,
 * where r is the reduction given in the satisfying assignment of
 * rsearch->solver, and w is the counter-example we're looking for.
 */
char *ex_p2part_pred(struct redsearch *rsearch, struct node *form, 
		     struct interp *interp)
{
	struct node *relargs=form->r;
        char *relname = (char *)form->data;
        struct vocab *voc=rsearch->p2->voc;

	char *res;
	char *tmp;
	int outsize = rsearch->outsize;

	struct red_tuple *rtup;
        int arity;
        struct rel_symbol *rs;

        for (rs=voc->rel_symbols; rs; rs=rs->next)
                if (!strcmp(rs->name, relname))
                        break;

        if (!rs)
        {
		rsearch->abort=1;
                printf("r11: redfind can't evaluate %s\n",relname);
                return ex_make_rf_form_true();
        }

	arity = rs->arity;

	rtup = red_rf_argstotup(rsearch,relargs,arity,rsearch->p2->voc, outsize,
				interp, relname);

	if (!rtup)
		return ex_make_rf_form_false();

	res = ex_rf_getrelform(rsearch, relname, rtup);
	tmp = malloc(sizeof(char)*(strlen(res)+3));
	sprintf(tmp,"(%s)",res);
	free(res);
	res=tmp;
	red_freertup(rtup);
	return res;
}

char *ex_rf_getrelform(struct redsearch *rsearch, char *relname,
		       struct red_tuple *rtup)
{
	int c=rsearch->c;
	int i;
	char *res=NULL, *tmp;

	for (i=1; i<=c; i++)
	{
		tmp = ex_rf_getrf_clause(rsearch,relname,rtup,i);
		res = red_disjunc(rsearch,res,tmp);
	}

	return res;
}

/* return (%s), where the string is the formula corresponding to
 * clause cl of the reduction (rsearch->solver) for relname(rtup).
 */
char *ex_rf_getrf_clause(struct redsearch *rsearch, char *relname,
			 struct red_tuple *rtup,int cl)
{
	struct red_bvarlist *bv;
	struct red_bvarlist **clause;
	int *signs;
	struct red_tuple_element *el;
	int arity=rtup->arity;
	int k=rsearch->k;
	int **xtup;
	int i, j, n=rsearch->n;
	int clen;
	int namelen = strlen(relname);
	struct minisat_solver_t *rsolver = rsearch->solver;

	char *res, *tmp;

	xtup = malloc(sizeof(int *)*arity);

	for (i=0; i<arity; i++)
		xtup[i]=malloc(sizeof(int)*k);

	for (i=0; i<arity; i++)
	{
		el = rtup->data+i;
		if (el->type!=2)
		{
			rsearch->abort=1;
			printf("r12: Sorry, this formula isn't supported yet.\n");
			for (j=0; j<arity; j++)
				free(xtup[j]);
			free(xtup);
			tmp= ex_make_rf_form_true();
			res = malloc(sizeof(char)*(strlen(tmp)+3));
                        sprintf(res,"(%s)",tmp);
                        free(tmp);
                        return res;
		}
		xtup[i] = cindex_to_tuple(xtup[i], el->a, k, n);
	}

	clen = 0;
	/* count how many literals are in clause i */
	for (bv=rsearch->bvars; bv; bv=bv->next)
	{
		for (j=0; j<namelen; j++)
			if (bv->pos[j+4]!=relname[j])
				break;
		if (j!=namelen || bv->pos[j+4]!='_')
			continue;	
			
		if (atoi(bv->pos+4+namelen+7)!=cl)
			continue;
		if (minisat_modelValue_Lit(rsolver, bv->posLit)==minisat_l_True 
			||
		    minisat_modelValue_Lit(rsolver, bv->negLit)==minisat_l_True)
			clen++;
	}

	if (clen == 0)
	{
		for (j=0; j<arity; j++)
                        free(xtup[j]);
                free(xtup);
		tmp = ex_make_rf_form_false();
		/* tmp = ex_make_rf_form_true(rsearch, ebv); */
		res = malloc(sizeof(char)*(strlen(tmp)+3));
                sprintf(res,"(%s)",tmp);
                free(tmp);
                return res;
	}
	signs = malloc(sizeof(int)*clen);
	clause = malloc(sizeof(struct red_bvarlist *)*clen);
	i=0;
	for (bv=rsearch->bvars; bv; bv=bv->next)
        {
		for (j=0; j<namelen; j++)
                        if (bv->pos[j+4]!=relname[j])
                                break;
                if (j!=namelen || bv->pos[j+4]!='_')
                        continue;
                if (atoi(bv->pos+4+namelen+7)!=cl)
                        continue;
                if (minisat_modelValue_Lit(rsolver, bv->posLit)==minisat_l_True)
		{
			clause[i]=bv;
			signs[i++]=1;
		}
		else if (minisat_modelValue_Lit(rsolver, bv->negLit)==minisat_l_True)
		{
			clause[i]=bv;
			signs[i++]=-1;
		}
	}
	
	/* now we have the clause */
	res = NULL;
	for (j=0; j<clen; j++)
	{
		/* include () around tmp */
		tmp = ex_p2_predform_getlitform(rsearch, xtup, clause[j],
						signs[j]);
		res = red_conjunc(rsearch, res, tmp);
	}

	for (j=0; j<arity; j++)
		free(xtup[j]);
	free(xtup);
	free(signs);
	free(clause);

	tmp = malloc(sizeof(char)*(strlen(res)+3));
	sprintf(tmp,"(%s)",res);

	free(res);
	res=tmp;

	return res;
}

/* return (%s) where the string is the formula corresponding to whether
 * r(w)\models this literal (pos if sign==1, neg if sign==-1) when the
 * x-i-j assignments are given by xtup
 */
char *ex_p2_predform_getlitform(struct redsearch *rsearch, int **xtup, 
				struct red_bvarlist *litbv, int sign)
{
	char *lit = litbv->pos;
	char *tmp, *res;
	int i,j;
	int llen = strlen(lit);

	j=0;
	for (i=0; i<llen; i++)
		if (lit[i]=='_')
			if (j++>0)
				break;
	lit+=i+1;

	if (lit[0]=='T' && lit[1]=='\0')
	{
		if (sign==1)
		{
			tmp = ex_make_rf_form_true();
			res = malloc(sizeof(char)*(strlen(tmp)+3));
			sprintf(res,"(%s)",tmp);
			free(tmp);
			return res;
		}
		else
		{
			tmp = ex_make_rf_form_false();
			res = malloc(sizeof(char)*(strlen(tmp)+3));
                        sprintf(res,"(%s)",tmp);
                        free(tmp);
                        return res;
		}
	}
	else if (lit[0]=='e' && lit[1]=='q' && lit[2]=='[')
	{
		tmp = ex_p2_lfpf_eqform(rsearch, lit, xtup);
		i = strlen(tmp)+3;
		if (sign==-1)
		{
			i+=3;
			res = malloc(sizeof(char)*i);
			sprintf(res,"(!(%s))",tmp);
			free(tmp);
			return res;
		}
		res = malloc(sizeof(char)*i);
		sprintf(res,"(%s)",tmp);
		free(tmp);
		return res;
	}
	else
	{
		tmp = ex_p2_lfpf_pform(rsearch, lit,xtup,rsearch->n);
		i = strlen(tmp)+3;
		if (sign==-1)
		{
			i+=3;
                        res = malloc(sizeof(char)*i);
                        sprintf(res,"(!(%s))",tmp);
                        free(tmp);
                        return res;
                }
                res = malloc(sizeof(char)*i);
                sprintf(res,"(%s)",tmp);
                free(tmp);
                return res;
	}
	return NULL;
	/* unreachable */
}

/* lit is eq[CONS...] */
/* return the corresponding Boolean formula */
/* note that, redfind currently guarantees that eq[CONS...] implies
 * lit is eq[CONS.CONS]
 */
char *ex_p2_lfpf_eqform_c(struct redsearch *rsearch, char *lit)
{
	int i, n=rsearch->n;
	char *tmp=NULL, *cons1, *cons2, *res=NULL;
	int len1, len2, len, llen=strlen(lit);
	int dlen=0;

	for (i=3; i<llen; i++)
	{
		if (lit[i]!='.')
			continue;
		len1 = i-3;
		break;
	}

	cons1=malloc(sizeof(char)*(len1+1));
	strncpy(cons1,lit+3,len1);
	cons1[len1]='\0';

	for (i++; i<llen; i++)
	{
		if (lit[i]!=']')
			continue;
		len2 = i - len1 - 3 - 1;
		break;
	}

	cons2 = malloc(sizeof(char)*(len2+1));
	strncpy(cons2,lit+3+len1+1,len2);
	cons2[len2]='\0';

	for (i=0; i<n; i++)
		dlen+=numdigits(i);
	dlen = (dlen<<1);
	len = n*(7+len1+len2+1)+dlen; /* (cis%d&cis%d)|... */
	res=malloc(sizeof(char)*(len+1));
	tmp = malloc(sizeof(char)*(8+((int)numdigits(n-1)<<1)+len1+len2));
	res[0]='\0';

	for (i=0; i<n; i++)
	{
		sprintf(tmp, "(%sis%d&%sis%d)|",cons1,i,cons2,i);
		strcat(res,tmp);
	}
	res[strlen(res)-1]='\0';

	free(cons1);
	free(cons2);
	free(tmp);
	return res;
}

/* lit is eq[...] */
/* return the corresponding Boolean formula */
char *ex_p2_lfpf_eqform(struct redsearch *rsearch, char *lit, int **xtup)
{
	int v1=-1, v2=-1;
	int a1, a2, k1, k2;
	int i;
	int llit=strlen(lit);
	int llen;
	char *var;
	char *vt;
	int succ = 0;

	if (lit[3]=='x')
	{
		sscanf(lit,"eq[x-%d-%d",&k1,&a1);
		v1 = xtup[a1-1][k1-1];
	}
	else
		return ex_p2_lfpf_eqform_c(rsearch,lit);

	for (i=0; i<llit; i++)
		if (lit[i]=='.')
			break;

	if (lit[i+1]=='+')
	{
		succ=1;
		i++;
	}

	if (isdigit(lit[i+1]))
		v2=atoi(lit+i+1);
	else if(lit[i+1]=='x')
	{
		sscanf(lit+i+1,"x-%d-%d",&k2,&a2);
		v2 = xtup[a2-1][k2-1];
	}
	else
	{
		llen = strlen(lit+i+1); /* length of constant name+]*/
		llen += 2+numdigits(v1);
		var = malloc(sizeof(char)*llen);
		vt  = malloc(sizeof(char)*llen);
		strcpy(var,lit+i+1);
		var[strlen(var)-1]='\0';
		if (!strcmp(var,"max"))
		{
			free(var);
			free(vt);
			var = malloc(sizeof(char)*6);
			if (v1==rsearch->n-1)
				sprintf(var, "TRUE");
			else
				sprintf(var, "FALSE");
			return var;
		}

		sprintf(vt,"%sis%d",var,v1);
		free(var);
		var=vt;
		return var;
	}

	if (v1==(v2+succ))
		return ex_make_rf_form_true();
	else
		return ex_make_rf_form_false();
}

char *ex_make_rf_form_true(void)
{
	char *res=malloc(sizeof(char)*5);
	sprintf(res,"TRUE");
	return res;
}

char *ex_make_rf_form_false(void)
{
	char *res=malloc(sizeof(char)*6);
	sprintf(res,"FALSE");
	return res;
}

/* lit is RELNAME[I.J....] */
/* where relname is a relation of p1, and the I, J... are
 * constants (of p1) or variables x-k1-a1.
 *
 * Return the corresponding Boolean formula.  Currently only variables are
 * supported.
 */
char *ex_p2_lfpf_pform(struct redsearch *rsearch, char *lit, int **xtup, int n)
{
	int i,j;
	int llen=strlen(lit);
	char *relname;
	char *vlit;
	int *cons_mask;
	char **cons_names;
	int *val;
	int *tuple=NULL;
	int arity;
	char *res=NULL;

	for (i=0; i<llen; i++)
                if (lit[i]=='[')
                        break;

        relname=malloc(sizeof(char)*(i+1));
        for (j=0; j<i; j++)
                relname[j]=lit[j];
        relname[j]='\0';

        vlit = lit+i+1;
        llen = strlen(vlit);

	arity = ex_pf_tuparity(vlit, llen);
	cons_mask = malloc(sizeof(int)*arity);
	val = malloc(sizeof(int)*arity);
	cons_names = malloc(sizeof(char *)*arity);
	for (i=0; i<arity; i++)
		cons_names[i]=NULL;

	/* get the mask of constants, names of constants */
	ex_pf_tupconsmask(vlit, llen, xtup, arity, cons_mask, cons_names, val);

	while ((tuple = ex_nextpf_tuple(tuple, arity, n, val, cons_mask)))
		res = ex_p2_lfpf_join(rsearch, res, relname, tuple, arity, 
				      cons_mask, cons_names);

	free(cons_mask);
	free(relname);
	for (i=0; i<arity; i++)
		free(cons_names[i]);
	free(cons_names);

	return res;
}

/* vlit is the TUP part of R[TUP , including the closing ], with elements
 * separated by '.'. and llen==strlen(vlit).
 * Return the arity.
 */
int ex_pf_tuparity(char *vlit, int llen)
{
	int arity=0, i;
	for (i=0; i<llen; i++)
		if (vlit[i]=='.' || vlit[i]==']')
			arity++;
	return arity;
}

/* vlit is the TUP part of R[TUP , including the closing ], with arity-many
 * elements seperated by '.' and llen==strlen(vlit).
 * Initialize cons_mask, cons_names and val, where cons_mask[i]==0 if the i'th
 * element of vlit is not a constant, and cons_mask[j]==cons_mask[k] iff
 * elements j and k are the same constant (unless cons_mask[j]==cons_mask[k]==0,
 * in which case they are both variables but may be different.
 * cons_names[i] is the name of the constant in element i, or NULL if
 * that element is not a constant.  These names are strpy'ed, so should be freed
 * when they are not needed.
 * val[i] is the value of the variable in element i (from xtup), or 0 if
 * element i is a constant.
 * Note: the "new" non-zero elements of cons_mask must be non-decreasing
 * left-to-right.
 */
void ex_pf_tupconsmask(char *vlit, int llen, int **xtup, int arity,
		       int *cons_mask, char **cons_names, int *val)
{
	int i,j,c=0,len;
	int ki,ai;
	char old;

	for (i=0; i<llen; i++)
        {
		if (isdigit(vlit[i]))
		{
			val[c] = atoi(vlit+i);
			cons_names[c]=NULL;
			c++;
			while (vlit[i]!='.' && vlit[i]!=']')
                                i++;
                        continue;
		}
                else if (vlit[i]=='x')
                {
			sscanf(vlit+i,"x-%d-%d",&ki,&ai);
                        val[c] = xtup[ai-1][ki-1];
			cons_names[c]=NULL;
			c++;
			while (vlit[i]!='.' && vlit[i]!=']')
				i++;
			continue;
		}
                else
                {
			for (j=i+1; j<llen; j++)
			{
				old = vlit[j];
				if (old!='.' && old!=']')
					continue;
				len = j-i;
				vlit[j]='\0';
				cons_names[c]=malloc(sizeof(char)*(len+1));
				strcpy(cons_names[c],vlit+i);
				val[c]=0;
				c++;
				vlit[j]=old;
				i=j; /* loop will increment */
				break;
			}
		}
	}

	for (i=0; i<arity; i++)
		cons_mask[i]=0;
	/* now we have cons_names, and cons_mask==0 for non-constants */

	c=1;
	for (i=0; i<arity; i++)
	{
		if (!cons_names[i])
			continue;
		if (cons_names[i] && cons_mask[i]==0)
			cons_mask[i]=c++;
		for (j=i+1; j<arity; j++)
		{
			if (!cons_names[j])
				continue;
			if (!strcmp(cons_names[i],cons_names[j]))
			{ /* same string, both use cons_mask[i] */
				cons_mask[j]=cons_mask[i];
			}
			else
				/* first time we've seen cons_names[j] */
				cons_mask[j]=c++;
			/* note that we may have "holes" on things like
			 * (c,d,d,e), resulting in (1,2,2,4),
			 * but we satisfy requirements.
			 */
		}
	}
	/* done */
	return;
}

/* return the next tuple in an enumeration of tuples consistent with the
 * following.
 *  - arity is arity
 *  - if cons_mask[i]==0, then tuple[i]==val[i]
 *  - if cons_mask[i]>0 and cons_mask[i]==cons_mask[j], then
 *       tuple[i]==tuple[j].
 *  - if cons_mask[i]>0, then 0<=tuple[i]<=n-1
 * If tuple is NULL, return the first in the enumeration.  If it is the last,
 * free it and return NULL.
 */
int *ex_nextpf_tuple(int *tuple, int arity, int n, int *val, int *cons_mask)
{
	int *res;
	int i, j;
	int maxc=0; /* largest cons_mask[i] we've seen */

	if (tuple==NULL)
	{
		res = malloc(sizeof(int)*arity);
		for (i=0; i<arity; i++)
		{
			if (!cons_mask[i])
				res[i]=val[i];
			else /* if cons_mask[i], val[i]==0, so we could */
			     /* always do res[i]=val[i] */
				res[i]=0; /* all constants=0 is okay */
		}
		return res;
	}

	/* otherwise we have a tuple, need to try to increment it */

	for (i=0; i<arity; i++)
	{
		if (cons_mask[i]<=maxc)
			continue; /* already decided not to increment this one*/
		maxc=cons_mask[i];
		if (++tuple[i]>=n)
			tuple[i]=0;
		for (j=i+1; j<arity; j++)
			if (cons_mask[i]==cons_mask[j])
				tuple[j]=tuple[i];
		if (tuple[i]) /* done! */
			return tuple;
	}
	/* no luck, we had the last */
	free(tuple);
	return NULL;
}
/* ex_p2_lfpf_join.
 * return res|(relname[tuple]&cons_names=tuple)
 * (if res==NULL, just return the second part)
 * Here, cons_names=tuple refers to the conjunction
 * cisj&disk... for c,d in cons_names with corresponding values given in tuple
 */
/* we should do relname[tuple] with constants in tuple, and later say
 * relname[tuple]<-> this current version, to abbreviate/share.
 */
char * ex_p2_lfpf_join(struct redsearch *rsearch, 
		       char *res, const char *relname, const int *tuple, 
		       int arity, const int *cons_mask, char **cons_names)
{
	char *tmp, *tmp1;
	int len, i, maxc=0;

	len = 2+strlen(relname)+3+(arity-1);

	for (i=0; i<arity; i++)
	{
		len+=numdigits(tuple[i]);
		if (cons_mask[i]>maxc)
		{
			maxc=cons_mask[i];
			len+=1+strlen(cons_names[i])+2+numdigits(tuple[i]);
			/* &cisv */
		}
	}
	/* len is enough for the new part of the formula */
	tmp = malloc(sizeof(char)*len);
	tmp1 = malloc(sizeof(char)*len);
	sprintf(tmp,"(%s[",relname);
	maxc=0;

	for (i=0; i<arity; i++)
	{
		if (i)
			sprintf(tmp1,"-%d",tuple[i]);
		else
			sprintf(tmp1,"%d",tuple[i]);
		strcat(tmp,tmp1);
	}
	strcat(tmp,"]");

	for (i=0; i<arity; i++)
		if (cons_mask[i]>maxc)
                {
                        maxc=cons_mask[i];
			sprintf(tmp1,"&%sis%d",cons_names[i],tuple[i]);
			strcat(tmp,tmp1);
		}

	strcat(tmp,")");
	free(tmp1);

	return red_disjunc(rsearch, res, tmp);
}

/* return the cbvar defining constant name that is true in the satisfying
 * assignment of rsearch->solver.
 */
char *ex_get_redcons_truevar(struct redsearch *rsearch, const char *name)
{
	struct red_bvarlist *cbv;
	struct minisat_solver_t *solver=rsearch->solver;

	for (cbv=rsearch->cbvars; cbv; cbv=cbv->next)
		if (ex_is_crelvar(cbv->pos,name) && 
		    minisat_modelValue_Lit(solver,cbv->posLit)==minisat_l_True)
			return cbv->pos;

	return NULL; /* unreachable in normal circumstances */
}

/* vname is of the form "cons_%s_[TUP]" -- is the %s == cname ? */
int ex_is_crelvar(char *vname, const char *cname)
{
	char *t;
	int i, len, len2;
	t=vname+5;
	len=strlen(t), len2=strlen(cname);
	for (i=0; i<len && i<len2; i++)
		if (t[i]!=cname[i])
			return 0;
	return (i==len2);
}
	
/* Modify rtup to correspond to the arguments in the cbvar var */
/* we assume the vocabulary (cons_names) and arity match */
struct red_tuple *ex_getconstuple(char *var, struct red_tuple *rtup)
{
	char *tup;
	int i,j,t,vlen=strlen(var);
	int len;
	int num;
	char old;
	struct red_tuple_element *el;

	for (i=0; i<vlen; i++)
		if (var[i]=='[')
			break;
	tup = var+i+1;
	len = vlen-i-1;
	
	/* TODO can be done much like red_cf_gettup(char *tup, int k, int len)*/
	for (num=0,i=0,j=0; i<len; i++)
	{
		if (isdigit(tup[i]))
		{
			el = rtup->data+num;
			el->type = 2;
			el->a=atoi(tup+i);
			for (j=i+1; j<len; j++)
				if (tup[j]=='.'||tup[j]==']')
				{
					i=j;
					break;
				}
			num++;
			continue;
		}
		else /* constant */
		{
			for (j=i+1; j<len; j++)
			{
				if (!(tup[j]=='.'||tup[j]==']'))
					continue;
                        	el = rtup->data+num;
                        	old=tup[j];
                        	tup[j]='\0';
				for (t=0; t<rtup->num_cons; t++)
					if (!strcmp(rtup->cons_names[t],tup+i))
						break;
				if (t>=rtup->num_cons)
				{
					printf("r13: Uhoh, unknown constant\n");
					return rtup;
				}
				el->type = 1;
				el->a = t;
				tup[j]=old;
				i=j;
				num++;
				break;
			}
		}
	}
	return rtup;
}	

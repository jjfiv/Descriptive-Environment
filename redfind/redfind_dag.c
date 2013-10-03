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
/* redfind_dag.c
 * Skip Jordan
 *
 * This is intended to completely replace redfind.c.
 *
 * This uses SAT solvers to search for a reduction between properties
 * given as Boolean queries.  It is based on the ideas in
 * Crouch, Immerman, Moss: Finding Reductions Automatically, 
 * in Fields of Logic and Computation, 2010, Springer, 181 - 200.
 * 
 * chj	 4/3/12		created
 * chj	 7/13/12	changes for fast interpretations
 * chj	 7/17/12	debugging output for looking at formulas
 * chj	11/06/12	adding x=max, options for few variables
 * chj  11/12/12	support searching range of counter-example sizes
 * chj	11/16/12	support for x=y+1 (REDFIND_SUCC)
 * chj	 2/26/13	red_improve_hyp
 * chj	 3/01/13	always look for small counter-examples first (OLDRANGE)
 * chj	 3/12/13	do formulas as dags, not trees
 */

#include "types.h"
#include "redtypes.h"
#include "protos.h"
#include "redprotos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "parse.h"
#include <ctype.h> /* isdigit() */

#define INIT_COMMAND(s) \
        bufstate = yy_scan_string(s); \
        yyparse(); \
        do_cmd(cmdtree->l); \
        yy_delete_buffer(bufstate)

/* Searches for a k-ary quantifier-free reduction from
 * p1 to p2.  We only look for reductions where all formulas
 * are in CNF, and each formula must be a conjunction of at
 * most c disjunctions.
 */
int redfind(const struct bquery *p1, const struct bquery *p2, 
            int k, int c, int n1, int n2)
{
	struct example *ex=NULL;
	struct red_hypot *hypot=NULL;
	struct redsearch *rsearch = NULL;
	int done = 0;

	if (red_checkp1p2(p1,p2))
		return -1;

	rsearch = red_init_rsearch(p1,p2,k,c,n1,n2);
#ifdef REDFIND_DEBUG
	printf("  ==== Reduction finder ====\n");
#endif
			
	set_init_form(rsearch); /* phi_1 */

	red_constant_forms(rsearch);

	ex = get_any_example(n1,p1);

#ifdef REDFIND_DEBUG
	red_printex(ex,rsearch);
#endif

	while (!done)
	{
		if (rsearch->abort)
		{
			printf("r14: Aborted reduction search due to previous errors\n");
			if (ex)
				free_ex(ex);
			ex=NULL;
			/* also free things related to hypot and the solver,
			 * bvars, cbvars, etc. */
			break;
		}

		if (hypot)
			red_freehyp(hypot);
			
		hypot = get_next_hypothesis(rsearch, ex);

		free_ex(ex);
		if (!hypot)
		{
			printf("r00: No reduction in this search space.\n");
			/* ALSO NEED TO FREE OTHER THINGS */
			if (rsearch->used_exrel)
				red_free_exrel(rsearch->hash, 
						rsearch->used_exrel);
			red_freersearch(rsearch);
			return -1;
		}

#ifdef REDFIND_DEBUG
                printf("\n      -- Next candidate reduction --\n");
                red_printhyp(hypot);
                printf("\n");
#endif
	
		rsearch->used_exrel = red_free_exrel(rsearch->hash,
						     rsearch->used_exrel);
#ifndef REDFIND_OLDRANGE
		rsearch->n=n1;
		rsearch->outsize=de_pow(n1,k);
#endif
		ex = get_next_example(rsearch);
		while (!ex && rsearch->n<rsearch->n2)
		{
			rsearch->n++;
			rsearch->outsize=de_pow(rsearch->n,rsearch->k);
#ifdef REDFIND_DEBUG
			printf("\n         Increasing size to %d\n",
				rsearch->n);
#endif
			ex = get_next_example(rsearch);
		}
		if (!ex)
			break;
		rsearch->num_ex++;
#ifdef REDFIND_DEBUG
                red_printex(ex,rsearch);
#endif
	}

#ifdef REDFIND_DEBUG
	printf("No counter-examples found.\n");
#endif
	red_printhyp(hypot);
	red_freersearch(rsearch);
	/* add_reduction(p1, p2, hypot); */

	red_freehyp(hypot);

	return 1;
}

/* the solver has a hypothesis.  here, we try to minimize the number
 * of Boolean variables that are true (i.e., a simple, greedy
 * attempt at a "simple" counter-example
 */
/* this first try just looks at what atoms occur in the clauses,
 * trying to remove them (starting with succ) without adding new atoms.
 * later, we'll also try another strategy: removing all succ first (even if
 * we have to add something), then others, etc.
 */
void red_improve_hyp(struct redsearch *rsearch)
{
	minisat_Lit *all_lits;
	minisat_Lit *assumps;
	int *succ;
	int *truev;
	int num_assumps=0;
	int num_lits=0;
	int res;
	int i,len,j,atnum;
	struct minisat_solver_t *solver=rsearch->solver;
	char *str;

	struct red_bvarlist *tmp, *bv=rsearch->bvars;

	red_debug1("        Candidate found, simplifying...");

	for (tmp=bv; tmp; tmp=tmp->next)
		num_lits+=2;

	assumps = malloc(sizeof(minisat_Lit)*num_lits);
	all_lits = malloc(sizeof(minisat_Lit)*num_lits);
	truev = malloc(sizeof(int)*num_lits);
	succ = malloc(sizeof(int)*(num_lits>>1));

	j=atnum=0;
	for (tmp=bv; tmp; tmp=tmp->next)
	{
		succ[j]=0;
		truev[atnum]=1;
#ifdef RED_IGNALL
		atnum++;
		truev[atnum++]=1;
		j++;
		continue;
#elif defined(RED_IGNTF)
		str=tmp->pos;
		len=strlen(tmp->pos);
		if (str[len-2]=='_' && str[len-1]=='T')
		{
			atnum++;
			truev[atnum++]=1;
			j++;
			continue;
		}
#endif
		if (!red_minisat_truevar(rsearch, solver, tmp, -1))
		{
			truev[atnum]=0;
			assumps[num_assumps++]=minisat_negate(tmp->negLit);
		}
		atnum++;
		truev[atnum]=1;
		if (!red_minisat_truevar(rsearch, solver, tmp, 1))
		{
			truev[atnum]=0;
			assumps[num_assumps++]=minisat_negate(tmp->posLit);
		}
		atnum++;

		str=tmp->pos;
		len=strlen(str);
		for (i=0; i<len; i++)
		{
			if (str[i]=='+')
			{
				succ[j]=1;
				break;
			}
		}
		j++;
	}
	/* now, the initial num_assumps in assumps say that nothing will be
	 * added.  we have succ[j] if the j'th in bv is a successor,
	 * atnum and atnum+1 in truev[ ] represent if atnum>>1 is occurring
	 * (atnum is neg, atnum+1 is pos).  So, we want to try disabling
	 * things that occur, starting with successor.
	 * we add to all_lits, all literals that occur starting with successor.
	 */

	i=j=atnum=0;
	for (tmp=bv; tmp; tmp=tmp->next)
	{
		if (!succ[j])
		{
			atnum+=2;
			j++;
			continue;
		}
		/* okay, it's a successor, add the neg and pos to all_lits
		 * if they occur
		 */
		if (truev[atnum++]) /* negLit occurs */
			all_lits[i++]=minisat_negate(tmp->negLit);
		if (truev[atnum++]) /* posLit occurs */
			all_lits[i++]=minisat_negate(tmp->posLit);
		j++;
	}
	/* now, add non-succ */
	j=atnum=0;
	for (tmp=bv; tmp; tmp=tmp->next)
	{
		if (succ[j])
		{
			atnum+=2;
			j++;
			continue;
		}
		/* okay, not a successor, add the neg and pos if they occur */
		if (truev[atnum++])
			all_lits[i++]=minisat_negate(tmp->negLit);
		if (truev[atnum++])
			all_lits[i++]=minisat_negate(tmp->posLit);
		j++;
	}

	num_lits = i; /* now, num_lits is number occurring */

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
	free(truev);
	free(succ);

	return;
}

/* check p1,p2 -- return 0 if they look okay, otherwise non-zero */
/* Currently, only checks for nested TC and TC that have free variables
 * bound by outer quantifiers
 */
int red_checkp1p2(const struct bquery *p1, const struct bquery *p2)
{
	int res=0;

	res = red_checknestedtc(p1->form, 0);
	res |= red_checknestedtc(p2->form, 0);

	if (res)
		return res;

	res |= red_checkfreetcvars(p1->form, p1->voc);
	res |= red_checkfreetcvars(p2->form, p2->voc);
	return res;
}

/* return 0 if no TC has free variables (including those bound by outer
 * quantifiers.  Can assume no nested TC
 */
int red_checkfreetcvars(struct node *form, struct vocab *voc)
{
	struct list *list, *tmp;
	switch (form->label)
	{
		case TRUE:
                case FALSE:
                case EQUALS:
                case NEQUALS:
                case PRED:
                        return 0;
		case NOT:
			return red_checkfreetcvars(form->l, voc);
		case AND:
                case OR:
                case IMPLIES:
                case IFF:
                case XOR:
			return (red_checkfreetcvars(form->l, voc) ||
			       red_checkfreetcvars(form->r, voc));
		case EXISTS:
		case FORALL:
			return (red_checkfreetcvars(form->r, voc) ||
			       (form->l->r && 
				red_checkfreetcvars(form->l->r, voc)));
		case TC:
			list = d_free_var(form->l->r, voc);
			list = remove_tcargs(list, form->l->l);
			if (!list)
				return 0;
			while (list)
			{
				tmp=list->next;
				free(list);
				list=tmp;
			}
			return 1;
		default:
			return 0;
	}
}

/* return 0 if there are no nested tc, 1 otherwise. depth is 1 if we're in
 * a TC currently */
int red_checknestedtc(struct node *form, int depth)
{
	switch (form->label)
	{
		case TRUE:
		case FALSE:
		case EQUALS:
		case NEQUALS:
		case PRED:
			return 0;
		case NOT:
			return red_checknestedtc(form->l,depth);
		case AND:
		case OR:
		case IMPLIES:
		case IFF:
		case XOR:
			return (red_checknestedtc(form->l,depth) ||
				red_checknestedtc(form->r,depth));
		case EXISTS:
		case FORALL:
			return (red_checknestedtc(form->r, depth) ||
				(form->l->r && 
				 red_checknestedtc(form->l->r,depth)));
		case TC:
			if (depth)
			{
				printf("r??: Sorry, redfind doesn't support nested TC\n");
				return 1;
			}
			return red_checknestedtc(form->l->r, depth+1);
		default:
			printf("r??: Unknown node when checking for nested TC\n");
			return 1;
	}
}

#ifdef REDFIND_DEBUG
/* debugging output, print the current example */
void red_printex(struct example *ex, struct redsearch *rsearch)
{
	struct structure *str = ex->a;
	struct relation *rel;
	int arity, size;
	int tuple_num;
	struct interp *interp;
	int *tuple=NULL;
	int res;
	char *output=NULL;
	struct constant *cons;

	printf("   -- Example %d --\n",rsearch->num_ex);

	for (rel=str->rels; rel; rel=rel->next)
	{
		output=NULL;
		arity = rel->arity;
		size = str->size;
		if (rel->cache)
			tuple_num = 0;
		else
			tuple_num = -1;
		interp = new_interp(str);

		if (rel->parse_cache)
			eval_init_form(rel->parse_cache, interp, str);
		printf("Relation %s:\n",rel->name);
		while ((tuple = next_tuple(tuple, arity, size)))
		{
			if (tuple_num>=0 && ((res=rel->cache[tuple_num++])>-1))
	                {
	                        if (res)
	                                output = add_tup_to_output(output,
	       	                                            tuple, arity, size);
	                        continue;
	                }

	                interp = fake_add_tup_to_interp(interp, tuple, arity);
	                if ((res=eval_rec(rel->parse_cache, interp, str)))
	                        output = add_tup_to_output(output, tuple, arity,
							   size);
	                /* interp = free_remove_tup(interp, arity); */
	                if (tuple_num>=0)
	                        rel->cache[tuple_num-1]=res;
		}
		free_interp(interp);

	 	if (output)
	        {
	                printf(":%s}\n",output);
	        	free(output);
	        }
	        else
                	printf(":{}\n");
	}

	for (cons=str->cons; cons; cons=cons->next)
		printf(" %s is %d",cons->name, cons->value);

	printf("\n");
	return;
}
#endif

/* Frees everything in rsearch that is not used outside of redfind.c */
void red_freersearch(struct redsearch *rsearch)
{
	red_freetruefalse(rsearch);
	hash_free_nodes(rsearch->hash->id_hash);
	hash_destroy(rsearch->hash->id_hash);
	free(rsearch->hash);
	rsearch->hash=NULL;

	if (rsearch->solver)
		minisat_delete(rsearch->solver);
	rsearch->solver = NULL;

	red_freebvars(rsearch->bvars);
	red_freebvars(rsearch->cbvars);

	free(rsearch);
}

/* get the bvarlist for TRUE and FALSE and free them */
void red_freetruefalse(struct redsearch *rsearch)
{
	struct hnode_t *hnode;
        struct red_bvarlist *bv;
        hnode = hash_lookup(rsearch->hash->id_hash, "T");
        bv = (struct red_bvarlist *)hnode_get(hnode);

	free(bv);
	      /* we allocated T&F together, so we only free once.
 	       * annoying but whatever.
	       */
}

/* free everything not needed elsewhere.
 * note that we are called on rsearch->bvars, rsearch->cbvars and
 * also on the result of ex_initbvarlist.
 */
void red_freebvars(struct red_bvarlist *bvars)
{
	struct red_bvarlist *bv1, *bv2;

	for (bv1=bvars; bv1; bv1=bv2)
	{
		bv2=bv1->next;
		if (bv1->pos)
			free(bv1->pos);
		if (bv1->neg)
			free(bv1->neg);
		free(bv1);
	}
	return;
}

void red_printhyp(struct red_hypot *hyp)
{
	char *cmd = red_hypottocmd(hyp->rsearch,hyp);
	printf(": %s",cmd);
	free(cmd);
	return;
}

struct redsearch *red_init_rsearch(const struct bquery *p1, 
				   const struct bquery *p2, int k, int c, 
				   int n1, int n2)
{
	/* TODO check mallocs and return errors */
	struct redsearch *rsearch = malloc(sizeof(struct redsearch));
	rsearch->hash = malloc(sizeof(struct env));
	rsearch->hash->id_hash = hash_create(RED_MAXVARS,(hash_comp_t)strcmp,0);
	rsearch->hash->next_id = 0;
	rsearch->num_ex = 0;
	rsearch->abort=0;
	rsearch->used_exrel = NULL;
	rsearch->solver = minisat_new();
        rsearch->p1 = p1;
        rsearch->p2 = p2;
        rsearch->k = k;
        rsearch->c = c;
        rsearch->n1 = n1;
        rsearch->n = n1;
        rsearch->n2 = n2;
        rsearch->outsize = de_pow(n1,k);
	rsearch->bvars = red_makebvars(rsearch, p1, p2, k, c, n2);
	rsearch->cbvars = red_makecbvars(rsearch, p1, p2, k, n2);
	rsearch->tbvars = NULL;
	rsearch->tc[0]=rsearch->tc[1]=rsearch->tc[2] = NULL;
	rsearch->num_tc = 0;
	init_truefalse_rsearch(rsearch);
	return rsearch;
}

void init_truefalse_rsearch(struct redsearch *rsearch)
{
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *env=rsearch->hash;
	struct red_bvarlist *list = malloc(sizeof(struct red_bvarlist)<<1);

	list->pos = "T";
	list->neg = NULL;
	list->posVar=minisat_newVar(solver);
	minisat_setFrozen(solver,list->posVar, minisat_l_True);
	list->posLit=minisat_mkLit(list->posVar);
	list->next = NULL;

	hash_alloc_insert(env->id_hash, "T", list);
	list++;
	
	list->pos = "F";
	list->neg = NULL;
	list->posVar=minisat_newVar(solver);
	minisat_setFrozen(solver, list->posVar, minisat_l_True);
	list->posLit=minisat_mkLit(list->posVar);
	list->next = NULL;

	hash_alloc_insert(env->id_hash, "F", list);

	minisat_addClause_begin(solver);
        minisat_addClause_addLit(solver, (list-1)->posLit);
        minisat_addClause_commit(solver);
        minisat_addClause_begin(solver);
        minisat_addClause_addLit(solver, minisat_negate(list->posLit));
        minisat_addClause_commit(solver);
	return;
}

/* Note that each constant needs exactly one definition */
int red_constant_forms(struct redsearch *rsearch)
{
	const struct bquery *p1=rsearch->p1, *p2=rsearch->p2;
	struct cons_symbol *p1cons=p1->voc->cons_symbols;
	struct cons_symbol *p2cons;
	struct red_tuple *tuple1=NULL, *tuple2=NULL;
	minisat_Lit lit;
	struct minisat_solver_t *solver=rsearch->solver;
	int k=rsearch->k;
	int n=rsearch->n;
	int i,j;
	struct env *hash=rsearch->hash;

	/* need at least one true */
	for (p2cons=p2->voc->cons_symbols; p2cons; p2cons=p2cons->next)
	{
		minisat_addClause_begin(solver);
		while ((tuple1=red_nextconstuple(rsearch,tuple1,k,n,p1cons)))
		{
			lit = red_getconsbvarlit(hash, p2cons->name, tuple1);
			minisat_addClause_addLit(solver,lit);
		}
		minisat_addClause_commit(solver);
	}

	/* at most one true (for each pair, at least one false) */
	for (p2cons=p2->voc->cons_symbols; p2cons; p2cons=p2cons->next)
	{
		for (tuple1=red_nextconstuple(rsearch,tuple1,k,n,p1cons),i=0; 
		     tuple1;
		     tuple1=red_nextconstuple(rsearch,tuple1,k,n,p1cons),i++)
		{
		       for (tuple2=red_nextconstuple(rsearch,tuple2,k,n,p1cons),
			            j=0;
			    tuple2; 
			    tuple2=red_nextconstuple(rsearch,tuple2,k,n,p1cons),
				    j++)
			{
				if (i==j)
					continue;
				minisat_addClause_begin(solver);
				lit = red_getconsbvarlit(hash, p2cons->name,
							 tuple1);
				minisat_addClause_addLit(solver,
							 minisat_negate(lit));
				lit = red_getconsbvarlit(hash, p2cons->name,
							 tuple2);
				minisat_addClause_addLit(solver,
							 minisat_negate(lit));
				minisat_addClause_commit(solver);
			}
		}
	}

	return 1;
}

/* returns the minisat_Lit for cons_cname_[tuple] */
minisat_Lit red_getconsbvarlit(struct env *env, char *cname, 
			       struct red_tuple *tuple)
{
	char *varname = red_getconsbvarname(cname, tuple);
	minisat_Lit lit = red_getposLit(env, varname);
	free(varname);
	return lit;
}

/* returns "cons_cname_[tuple]" */
char *red_getconsbvarname(const char *consname, struct red_tuple *tuple)
{
	int len,i,arity=tuple->arity;
	char *res, *tmp;
	struct red_tuple_element *el;

	len = 5+strlen(consname)+2+2; /* "cons_%s_[" and "]\0" */
	for (i=0; i<arity; i++)
        {
                el=tuple->data+i;
                if (el->type==2) /* natural */
                        len+=numdigits(el->a)+1; /* "%d," */
                else /* assert el->type==1 */
                        len+=strlen(tuple->cons_names[el->a])+1;
        }

        res = malloc(sizeof(char)*len);
        tmp = malloc(sizeof(char)*len);

        sprintf(res,"cons_%s_[",consname);

	for (i=0; i<arity; i++)
        {
                if (i!=0)
                        strcat(res,".");
                el=tuple->data+i;
                if (el->type==2)
                        sprintf(tmp,"%d",el->a);
                else /* assert el->type == 1 */
                        sprintf(tmp,"%s",tuple->cons_names[el->a]);
                strcat(res,tmp);
        }
        free(tmp);

        strcat(res,"]");

	return res;
}

/* Adds phi_1 to the solver */
int set_init_form(struct redsearch *rsearch)
{
	struct red_bvarlist *t;
	struct minisat_solver_t *solver=rsearch->solver;
	struct rel_symbol *rel;
	int i, c=rsearch->c;

	red_debug1("\n      -- Initial formula for candidate reductions ---\n");
#ifdef REDFIND_DEBUG2
        red_debug1("T&!F&");
#endif
	for (t=rsearch->bvars; t; t=t->next)
	{
		minisat_addClause_begin(solver);
		minisat_addClause_addLit(solver,minisat_negate(t->posLit));
		minisat_addClause_addLit(solver,minisat_negate(t->negLit));
		minisat_addClause_commit(solver);
#ifdef REDFIND_DEBUG2
		if (t!=rsearch->bvars)
			red_debug1("&");
		red_debug3("(!%s|!%s)",t->pos,t->neg);
#endif
	}

	for (rel=rsearch->p2->voc->rel_symbols; rel; rel=rel->next)
		for (i=1; i<=c; i++)
			red_clausei_notempty(rsearch, rel->name, i);
	red_debug1("\n\n");
	return 1;
}

/* require that form{relname}_clause{cl} is not empty */
void red_clausei_notempty(struct redsearch *rsearch, char *relname, int cl)
{
	struct red_bvarlist *t;
	struct minisat_solver_t *solver=rsearch->solver;
	int i;
	minisat_addClause_begin(solver);

	for (t=rsearch->bvars; t; t=t->next)
	{
		if (!red_relform_relevantvar(t, relname))
			continue;
		i = red_getclause(t);
		if (i!=cl)
			continue;
		minisat_addClause_addLit(solver, t->posLit);
		minisat_addClause_addLit(solver, t->negLit);
	}
	minisat_addClause_commit(solver);
	return;
}

/* get_next_hypothesis: Returns a (candidate) reduction r that
 * maps structures from the vocabulary of p1 to that of p2, such that
 * A\models p1 <-> r(A)\models p2, for examples A\in ex
 */
/* We construct a propositional formula such that a satisfying assignment
 * gives us such a (candidate) reduction, and then use SAT solvers.
 */
/* We assume that the SAT solver is ready for us to add only the new clause(s)
 * for (phi_{ex\models p1)<->phi_{r(w1)\models p2}) (i.e., that things are
 * being done incrementally.  Note that we know whether ex\models p1,
 * so this simplifies to phi_{r(w1)\models p2} or its negation.
 */
/* Note that one can use red_hypottored() to convert this red_hypot to a
 * (struct reduction) if needed.
 */
struct red_hypot *get_next_hypothesis(struct redsearch *rsearch,
		  const struct example *ex)
{
	struct node *phi;
	struct node *tmp, *tmp1=NULL;
	/* our CNF conversion adds the formula to the solver! */
	struct red_hypot *r;
	minisat_bool res;

	red_debug1("\n      Preparing reduction formula... ");
#ifdef REDFIND_DEBUG2
	fflush(stdout);
#endif
	phi = make_redform(rsearch, ex);

	if (!ex->p1)
	{
		tmp = red_makenode(NOT, phi, 0);
		phi = tmp;
	}

        if (rsearch->tc[2])
                tmp1 = red_maketcform(rsearch, ex, rsearch->tc[2]);


	tmp = make_predform(rsearch, ex);
	if (tmp)	
		phi = red_binform(rsearch, phi, tmp, "&");
	tmp=NULL;

	red_debug1("done.\n");
	if (rsearch->tc[2])
		red_debug3("      -- New part of hypothesis formula --\n(%s)&(%s)\n\n",
			   phi,tmp1);
	else
	{
		red_debug2("      -- New part of hypothesis formula --\n%s\n\n",
			   phi);
	}
	red_docnf(phi,rsearch->solver,rsearch->hash);
	red_freenode(phi);
	if (tmp1)
	{
		red_docnf(tmp1,rsearch->solver,rsearch->hash);
		red_freenode(tmp1);
	}
	/* TODO: check for errors */

	red_debug1("\n      Calling minisat_solve for hypothesis.... ");
#ifdef REDFIND_DEBUG2
	fflush(stdout);
#endif
	res = minisat_solve(rsearch->solver, 0, NULL);
	red_debug1("minisat_solve is done\n\n");

   	if (rsearch->tc[2])
                free_rsearch_tc2(rsearch);

	if (res == minisat_l_False)
		return NULL;

#ifdef REDFIND_REDIMPROVE
                red_improve_hyp(rsearch);
#endif

	r = red_getsatred(rsearch);
	return r;
}

#ifdef REDFIND_DEBUG
int red_countbvars(struct red_bvarlist *bv)
{
	int i=0;
	struct red_bvarlist *t;
	for (t=bv; t; t=t->next)
		i++;
	return i;
}
#endif

/* Remove the variables in rsearch->tbvars from rsearch->hash and
 * free everything in rsearch->tbvars
 * Also frees rsearch->tc[2] and sets that and tbvars to NULL;
 */
void free_rsearch_tc2(struct redsearch *rsearch)
{
	struct red_bvarlist *next, *tmp;
	struct env *env = rsearch->hash;
	struct hnode_t *hnode;

	for (tmp=rsearch->tbvars; tmp; tmp=next)
	{	
		next = tmp->next;
		hnode = hash_lookup(env->id_hash, tmp->pos);
		if (hnode)
			hash_delete_free(env->id_hash, hnode);
		free (tmp->pos);
		free(tmp);
	}
	rsearch->tbvars = NULL;

	ex_freetcd(rsearch->tc[2]);
	rsearch->tc[2]=NULL;
	return;
}

/* Define the initial TC{num}[1]_[tuple] */
/* based on ex_tcp1_dist1 with small changes for using in get_next_hypothesis
 * Only change is calling make_rf_rec, should be combined.
 */
/* tuple is (x-,y-), where each is arity tcd->tup_arity.
 * this is true if x-=y-, and otherwise we substitute into the
 * formula that we're taking the TC of
 */
struct node *red_tcp1_dist1(struct redsearch *rsearch, struct tc_def *tcd,
			    int *tuple, const struct example *ex)
{
	int tup_arity=tcd->tup_arity;
        int *tupa=tuple, *tupb=tuple+tup_arity;
        int flag=0;

	char *tmp1s, *tmp2s;
        struct node *tmp1, *res, *tmp2;
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

        tmp1s = ex_printtcvar(tcd->num, 1, tupa, tupb, tup_arity);
	tmp1 = snode(VAR,tmp1s);
	free(tmp1s);

        if (!flag)
                return tmp1;

	tcform = tcd->tc_node->l->r;
        tcargs = tcd->tc_node->l->l;
        interp = tcd->interp;

        tmp = tcargs;
        for (i=0; ;tmp=tmp->r)
        {
                *(tmp->l->l->ival)=tupa[i]; /* was i++ TODO CHECK */
                if (++i<tup_arity)
                        *(tmp->l->r->ival)=tupa[i++];
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
                        *(tmp->l->r->ival)=tupb[i++];
                else
                        break;
        }

	tmp2 = make_rf_rec(rsearch, ex, tcform, interp);

	res = malloc(sizeof(char)*(strlen(tmp1)+strlen(tmp2)+8));
        sprintf(res,"(%s<->(%s))",tmp1,tmp2);
        free(tmp1);
        free(tmp2);
        return res;
}

/* Returns the formula defining the TC in tcd */
/* Almost identical to ex_maketcdef -- should be combined */
char *red_maketcform(struct redsearch *rsearch, const struct example *ex,
		     struct tc_def *tc)
{
	char *res=NULL, *tmp=NULL;
	int *tuple=NULL;
	int i, arity;
	int darity;
	int outsize=rsearch->outsize;
	int max;
	struct tc_def *tcd;

	rsearch->tbvars = ex_addtcvars(tc, rsearch->solver, 
				       rsearch->tbvars, rsearch->hash);
	for (tcd=tc; tcd; tcd=tcd->next)
	{
		arity=tcd->tup_arity;
		darity = arity<<1;
		max = de_pow(outsize,arity)-1;
		eval_init_form(tcd->tc_node, tcd->interp, NULL);

		/* first, distance <= 1 */
		while ((tuple=next_tuple(tuple, darity, outsize)))
       	 	{
       	         	tmp = red_tcp1_dist1(rsearch, tcd, tuple,ex);
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

	}
        return res;
}

/* Returns the formula defining used variables out%d_E[a.b.c...]
 * in terms of the bvars,cbvars
 */
char *make_predform(struct redsearch *rsearch, const struct example *ex)
{
	struct list *list;
	struct ex_rel *er;
	char *res=NULL, *tmp;

	for (list=rsearch->used_exrel; list; list=list->next)
	{
		er = (struct ex_rel *)list->data;
		tmp = make_pf_exrel(rsearch, ex, er);
		if (!res)
			res=tmp;
		else
			res = red_conjunc(rsearch, res,tmp);
	}

	return res;
}

/* return (er->var <-> (c1t1 | c1t2 | c1t3 | c2t1 | c2t2 | c2t3 | ... ))
 * where ci is the clause, and ti are lits that would make 
 * er->relname[er->tuple] true. (so citj are Boolean variables)
 */
char *make_pf_exrel(struct redsearch *rsearch, const struct example *ex, 
		    struct ex_rel *er)
{
	struct cons_bvars *cons_bvars=red_getconslits(rsearch,ex,er);
	struct cons_bvars *incons_bvars=red_getinconslits(rsearch, ex, er);
	int i, c=rsearch->c;
	char *res=NULL, *res2=NULL, *tmp=NULL, *tmp1 = NULL;

	cons_bvars = red_cons_addlit(cons_bvars, "T");
	incons_bvars = red_cons_addlit(incons_bvars, "notT");

	for (i=1; i<=c; i++)
	{
		res = red_rf_pred_inconsclause(rsearch,i, cons_bvars, er);
		res2= red_rf_pred_inconsclause(rsearch,i,incons_bvars,er);
		tmp1 = malloc(sizeof(char)*(strlen(res)+strlen(res2)+5));
		sprintf(tmp1, "(%s&!%s)",res,res2);
		free(res);
		free(res2);
		tmp = red_disjunc(rsearch, tmp, tmp1);
	}

	tmp1 = malloc(sizeof(char)*(strlen(er->var)+strlen(tmp)+8));
	sprintf(tmp1, "(%s<->(%s))",er->var,tmp);
	free(tmp);
	res=tmp1;
	red_freecbvars(cons_bvars);
	red_freecbvars(incons_bvars);
	return res;
}

void red_freecbvars(struct cons_bvars *cb)
{
	struct cons_bvars *t, *tn;
	int i, num;

	for (t=cb; t; t=tn)
	{
		tn=t->next;
		num=t->num;
		for (i=0; i<num; i++)
			free(t->lits[i]);
		free(t->lits);
		free(t);
	}
	return;
}

/* the solver in rsearch has a satisfying assignment.
 * Return the reduction it represents.
 */
struct red_hypot *red_getsatred(struct redsearch *rsearch)
{
	struct rel_symbol *rel;
	struct cons_symbol *cons;
	struct red_hypot *hyp = malloc(sizeof(struct red_hypot));
	/* TODO check malloc */
	hyp->rsearch = rsearch;
	hyp->relforms = NULL;
	hyp->consforms = NULL;

	for (rel=rsearch->p2->voc->rel_symbols; rel; rel=rel->next)
		hyp->relforms = red_addrelform(hyp->relforms, rel, rsearch);

	for (cons=rsearch->p2->voc->cons_symbols; cons; cons=cons->next)
		hyp->consforms = red_addconsform(hyp->consforms, cons, rsearch);

	return hyp;
}

/* returns the DE command for red_hypot */
char *red_hypottocmd(struct redsearch *rsearch, struct red_hypot *red)
{
	char c;
        struct hnode_t *hnode;
        char *inp;
	char *voc1=rsearch->p1->voc->name;
	char *voc2=rsearch->p2->voc->name;
	struct red_relforms *rf;
	int tt;
	char *tmpinp;
	char *tmp;
	int k=rsearch->k;
	char phi0[]="\\t"; /* Currently, we assume phi0==\t, as this
			    * simplifies the map between new elements of the
			    * universe and k-tuples
			    */
	char **relforms;
	char name[5];
	struct list *cf;

        int len=0,t;
        for (c='A'; c<='Z'; c++)
        {
                for (tt=0; tt<999; tt++)
                {
                        sprintf(name,"%c%d",c,tt);
                        hnode = hash_lookup(cur_env->id_hash, name);
                        if (!hnode)
                                break;
                }
                if (!hnode)
                        break;
        }
	sprintf(name,"%c%d",c,tt);

	len=4+2+3+1+10+strlen(voc1)+1+strlen(voc2)+1+numdigits(k)+1+
	    strlen(phi0)+1; /* A???:=new reduction{voc1,voc2,k,phi0, */

	t=0;
	for (rf=red->relforms; rf; rf=rf->next)
		t++;

	relforms = malloc(sizeof(char *)*t);
	for (t=0,rf=red->relforms; rf; t++,rf=rf->next)
	{
		if (rf->c)
			tmp = red_rftodef(rf);
		else
		{
			tmp=malloc(sizeof(char)*3);
			sprintf(tmp,"\\t");
		}
		len+=strlen(rf->name)+1+numdigits(rf->arity)+4+
		     strlen(tmp)+1; /* NAME:arity is tmp, */
		relforms[t]=tmp;
	}

	for (cf=red->consforms; cf; cf=cf->next)
		len+=strlen((char *)cf->data)+1; /* c is \phi, */

	len+=4; /* }.\n\0 */

	inp = malloc(sizeof(char)*len);
	tmpinp = malloc(sizeof(char)*len);

	sprintf(inp,"%s:=new reduction{%s,%s,%d,%s",name,voc1,voc2,k,phi0);
	for (t=0,rf=red->relforms; rf; t++,rf=rf->next)
	{
		sprintf(tmpinp,",%s:%d is %s",rf->name,rf->arity,relforms[t]);
		free(relforms[t]);
		strcat(inp,tmpinp);
	}
	free(relforms);
	free(tmpinp);

	for (cf=red->consforms; cf; cf=cf->next)
	{
		strcat(inp,",");
		strcat(inp,(char *)cf->data);
	}
	strcat(inp,"}.\n");
	return inp;
}

/* red is a red_hypot.  Return it as a (struct reduction), where additional
 * parameters are from rsearch.
 */
struct reduction *red_hypottored(struct redsearch *rsearch,
                                 struct red_hypot *red)
{
	struct id *hash_data;
        struct hnode_t *hnode;
        void *bufstate;
	char *inp = red_hypottocmd(rsearch,red);
	char name[5];
	struct reduction *res;
	char c;
	int tt;
	for (c='A'; c<='Z'; c++)
        {
                for (tt=0; tt<999; tt++)
                {
                        sprintf(name,"%c%d",c,tt);
                        hnode = hash_lookup(cur_env->id_hash, name);
                        if (!hnode)
                                break;
                }
                if (!hnode)
                        break;
        }
        sprintf(name,"%c%d",c,tt);

	INIT_COMMAND(inp);
	free(inp);

	hnode = hash_lookup(cur_env->id_hash, name);
        hash_data = (struct id*)hnode_get(hnode);

	res = (struct reduction *)hash_data->def;

	hash_delete_free(cur_env->id_hash, hnode);
        /* free(hash_data->name); */
        free(hash_data);
	return res;
}

/* Return phi_{r(w1)\models p2} */
char *make_redform(struct redsearch *rsearch, const struct example *ex)
{
	struct interp *interp=new_interp(NULL);
	struct node *form=rsearch->p2->form;
	char *res;
	interp = add_symb_to_interp(interp, "max", rsearch->outsize-1);

	eval_init_form(form, interp, NULL);
	res = make_rf_rec(rsearch, ex, form, interp);
	free_interp(interp);
	return res;
}

char *make_rf_rec(struct redsearch *rsearch, const struct example *ex,
		  struct node *form, struct interp *interp)
{
	char *tmp1, *tmp2, *tmp3;
	switch (form->label)
	{
		case TRUE:
			return dupstr("T");
		case FALSE:
			return dupstr("F");

		/* Boolean connectives */
		case NOT:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
			tmp2 = malloc(sizeof(char)*(strlen(tmp1)+4));
			sprintf(tmp2,"!(%s)",tmp1);
			free(tmp1);
			return tmp2;
		case AND:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
			tmp2 = make_rf_rec(rsearch, ex, form->r, interp);
			return red_binform(rsearch, tmp1, tmp2, "&");
		case OR:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
			tmp2 = make_rf_rec(rsearch, ex, form->r, interp);
			return red_binform(rsearch, tmp1, tmp2, "|");
		case IMPLIES:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
                        tmp2 = make_rf_rec(rsearch, ex, form->r, interp);
			return red_binform(rsearch, tmp1, tmp2, "->");
		case IFF:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
                        tmp2 = make_rf_rec(rsearch, ex, form->r, interp);
			return red_binform(rsearch, tmp1, tmp2, "<->");
		case XOR:
			tmp1 = make_rf_rec(rsearch, ex, form->l, interp);
                        tmp2 = make_rf_rec(rsearch, ex, form->r, interp);
			tmp3 = malloc(sizeof(char)*(strlen(tmp2)+4));
			sprintf(tmp3, "!(%s)", tmp2);
			free(tmp2);
			return red_binform(rsearch, tmp1, tmp3, "<->");

		/* quantifiers */
		case EXISTS:
			return make_rf_exists(rsearch,ex,form,interp);
		case FORALL:
			return make_rf_forall(rsearch,ex,form,interp);

		/* equals, nequals */
		case EQUALS:
			return make_rf_eq(rsearch,ex,form,interp);
		case NEQUALS:
			tmp1 = make_rf_eq(rsearch,ex,form,interp);
			tmp2 = malloc(sizeof(char)*(strlen(tmp1)+4));
			sprintf(tmp2,"!(%s)",tmp1);
			free(tmp1);
			return tmp2;

		case PRED:
			return make_rf_pred(rsearch,form,interp);

		case TC:
			return make_rf_tc(rsearch, form, interp, ex);

		default:
			rsearch->abort=1;
			printf("r15: Sorry, basic functionality only.\n");
			return NULL;
	}
	return NULL; /* unreachable */
}

void red_freertup(struct red_tuple *tup)
{
	free(tup->data);
	free(tup->cons_names);
	free(tup);
	return;
}

/* make_rf_tc(...)
 * Returns the TC variable TC%d_[TUP]
 */
/* if TUP contains a constant, we return the OR over possible values of the 
 * constant */
/* this needs to re-use old tc_def if the interpretation hasn't changed
 * significantly and the node is the same
 */
/* we need to make sure no nested TC */
/* we need to enforce no interpreted vars inside tcform */
/* based on ex_p1rec_tc */
char *make_rf_tc(struct redsearch *rsearch, struct node *form,
		 struct interp *interp, const struct example *ex)
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
	struct tc_def *tcdtemp;

	char *res=NULL, *tmp;

	tcargs = form->l->l;
	relargs = form->r;
	for (tup_arity=0, tmpnode=tcargs; tmpnode; tup_arity++)
		tmpnode=tmpnode->r;

	num_tups = de_pow(rsearch->n, tup_arity);
	num_tups = num_tups*num_tups;
	
	tcd=NULL;

	/* should check interps, etc. but see red_checkp1p2(...) */
	for (tcdtemp=rsearch->tc[2];tcdtemp;tcdtemp=tcdtemp->next)
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
       		tcd->next = rsearch->tc[2];
        	tcd->tup_arity = tup_arity;
        	tcd->interp = dup_interp(interp);

		rsearch->tc[2]=tcd;
	}

	tmp = malloc(sizeof(char)*(4+numdigits(tcd->num)));
        sprintf(tmp,"TC%d_",tcd->num);

        arity = (tup_arity << 1);
        /* I want to do much like ex_p2_lfpf_pform , iter over the tuples
         * consistent with the constants -- can reuse most of that here
         */
        rtup = red_rf_argstotup(rsearch, relargs, arity, /* was p1->voc */
                                rsearch->p2->voc, outsize, interp, NULL);
        cons_mask = malloc(sizeof(int)*arity);
        val = malloc(sizeof(int)*arity);
        cons_names = malloc(sizeof(char *)*arity);
        for (i=0; i<arity; i++)
                cons_names[i]=NULL;

        ex_rtup_to_consmask(rtup, arity, cons_names, cons_mask, val);

	while ((tuple = ex_nextpf_tuple(tuple, arity, outsize, val, cons_mask)))
                res = red_p2_lfpf_join(rsearch, res, tmp, tuple, arity,
                                       cons_mask, cons_names, ex, interp);

        free(cons_mask);
        free(cons_names);
	free(val);
        free(tmp);

        return res;
}

/* red_p2_lfpf_join.
 * Basically the same as ex_p2_lfpf_join, but with the constant SAT variables 
 * used by get_next_hypothesis
 * return res|(relname[tuple]&cons_names=tuple)
 * (if res==NULL, just return the second part)
 * Here, cons_names=tuple refers to the conjunction
 * cons_c_[tuple]&cons_d_[tuple]... for c,d in cons_names with 
 * corresponding values given in tuple
 * Particularly annoying are the cases where c is defined in terms of
 * constants.
 */
/* we should do relname[tuple] with constants in tuple, and later say
 * relname[tuple]<-> this current version, to abbreviate/share.
 */
char *red_p2_lfpf_join(struct redsearch *rsearch, char *res, 
		       const char *relname, const int *tuple, int arity,
		       const int *cons_mask, char **cons_names,
		       const struct example *ex, struct interp *interp)
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
			cons_defs[i] = make_rf_eq(rsearch, ex, fakenode, interp);
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

/* make_rf_pred(...)
 * Returns the variable for out%d_REL[TUP] */
char *make_rf_pred(struct redsearch *rsearch,
		   struct node *form, struct interp *interp)
{
	struct node *relargs=form->r;
	char *relname = (char *)form->data;
	struct vocab *voc=rsearch->p2->voc;

	char *res;
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
		printf("r16: redfind can't evaluate %s\n",relname);
		return dupstr("T");
	}

	arity = rs->arity;

	rtup = red_rf_argstotup(rsearch,relargs, arity,rsearch->p2->voc,outsize,
				interp,relname);
	if (!rtup)
		return dupstr("F");

	res=red_rf_getrelvar(rsearch, relname, rtup);

	red_add_exrel(rsearch, res, relname, rtup);
	return res;
}

/* add this to the list of variables we need to define */
/* also add it to rsearch->hash */
void red_add_exrel(struct redsearch *rsearch, char *var, char *relname,
		   struct red_tuple *rtup)
{
	struct list *list=malloc(sizeof(struct list));
	struct ex_rel *er=malloc(sizeof(struct ex_rel));
	struct red_bvarlist *bv=malloc(sizeof(struct red_bvarlist));

	if (hash_lookup(rsearch->hash->id_hash, var))
		return; /* we've already added this one */

	list->data=er;
	list->next=rsearch->used_exrel;
	rsearch->used_exrel=list;

	er->var = dupstr(var);
	er->relname=relname;
	er->tup = rtup;
	er->bv = bv;

	bv->pos=er->var;
	bv->neg=NULL;
	bv->posVar=minisat_newVar(rsearch->solver);
	/*minisat_setFrozen(rsearch->solver, bv->posVar, minisat_l_True);*/
	/* these variables don't need to be frozen */
	bv->posLit=minisat_mkLit(bv->posVar);
	bv->negVar=0;
	bv->negLit=0;
	bv->next=NULL;

	hash_alloc_insert(rsearch->hash->id_hash, er->var, bv);

	return;
}

struct list *red_free_exrel(struct env *hash, struct list *used_exrel)
{
	struct list *tmp, *tmpn;
	struct ex_rel *er;
	struct red_bvarlist *bv;
	struct hnode_t *hnode;

	for (tmp=used_exrel; tmp; tmp=tmpn)
	{
		tmpn = tmp->next;
		er = (struct ex_rel *)tmp->data;
		red_freertup(er->tup);
		bv = er->bv;
		hnode = hash_lookup(hash->id_hash, bv->pos);
		hash_delete_free(hash->id_hash, hnode);
		free(bv);
		free(er->var);
		free(er);
		
		free(tmp);
	}

	return NULL;
}
/* returns out%d_{relname}[tup] */
char *red_rf_getrelvar(struct redsearch *rsearch, const char *relname, 
		       struct red_tuple *tup)
{
	int num_ex=rsearch->num_ex;
	char *res, *tmp;
	int len;

	len = red_tupstrlen(tup);
	tmp = red_printtup(rsearch, tup, len);
	res=malloc(sizeof(char)*(len+3+numdigits(num_ex)+1+strlen(relname)+1));
	sprintf(res,"out%d_%s%s",num_ex,relname,tmp);
	free(tmp);
	return res;
}

/* returns the tuple of relargs as a red_tuple, using the interpretation to
 * teval everything that is not a constant.
 * arity should be arity, and should be checked.  arguments>=outsize are
 * invalid, return NULL (same for <0)
 */
struct red_tuple *red_rf_argstotup(struct redsearch *rsearch,
				   struct node *relargs, int arity, 
				   struct vocab *voc, int outsize, 
				   struct interp *interp, char *relname)
{
	int i,j,num_cons;
	struct red_tuple *tup;
	struct red_tuple_element *el;
	struct cons_symbol *cons, *cs=voc->cons_symbols;
	struct node *ra, *ral;

	tup = malloc(sizeof(struct red_tuple));
	tup->arity=arity;
	tup->data = malloc(sizeof(struct red_tuple_element)*arity);
	for (i=0, cons=cs; cons; cons=cons->next,i++);
	if (i)
		tup->cons_names = malloc(sizeof(char *)*i);
	else
		tup->cons_names = NULL;
	tup->num_cons=num_cons=i;

	for (i=0, cons=cs; cons; cons=cons->next, i++)
		tup->cons_names[i]=cons->name;

	for (i=0,ra=relargs; ra; i++,ra=ra->r)
	{
		if (i>=arity)
		{
			rsearch->abort=1;
			printf("r17: Wrong arity for %s\n",relname);
			red_freertup(tup);
			return NULL;
		}
		el = tup->data+i;
		ral=ra->l;
		if (ral->label==CONSTANT)
		{
			for (j=0; j<num_cons; j++)
			{
				if (!strcmp(ral->data, tup->cons_names[j]))
				{
					el->type=1;
					el->a=j;
					break;
				}
			}
			if (j>=num_cons)
			{
				rsearch->abort=1;
				printf("r18: Invalid constant %s\n",
				       (char *)ra->data);
				red_freertup(tup);
				return NULL;
			}
		}
		else if (term_has_cons(ral))
		{
			rsearch->abort=1;
			printf("r19: Reduction finding doesn't support non-atomic terms with constants.\n");
			red_freertup(tup);
			return NULL;
		}
		else
		{
			el->type = 2;
			el->a=teval(ral, interp, NULL);

			if (el->a<0 || el->a>=outsize)
			{
				red_freertup(tup);
				return NULL;
			}
		}
	}
	return tup;
}

/* return a single cons_bvars with all (except F) literals inconsistent
 * with ex and the (output) predicate in ex_rel being true.
 * TODO: this should be combined with red_getconslits, which it almost
 * duplicates.
 */
struct cons_bvars *red_getinconslits(struct redsearch *rsearch, 
				   const struct example *ex, struct ex_rel *er)
{
        int num_cons=0;
        char **lits;
        struct cons_bvars *res;
        int i=0;
        int a1, a2, k1, k2;
        struct red_tuple *tup=er->tup;
        int k=rsearch->k, n=rsearch->n, arity=tup->arity;
        struct red_tuple_element *el1, *el2;
        int v1;
        int v2;
        int *ktup1=malloc(sizeof(int)*k), *ktup2=malloc(sizeof(int)*k);
        int len;
        struct constant *cons = ex->a->cons, *tc, *tc2;
        int d;

        num_cons = red_getnumconslits(rsearch, er);
        lits = malloc(sizeof(char *)*num_cons);

        res = malloc(sizeof(struct cons_bvars));
        res->num = num_cons;
        res->lits = lits;
        res->next = NULL;

        for (a1=1; a1<=arity; a1++)
        {
                el1 = tup->data+(a1-1);
		ktup1 = cindex_to_tuple(ktup1, el1->a, k, n);
		for (k1=1; k1<=k; k1++)
		{
			v1=ktup1[k1-1];
                	for (a2=1; a2<=arity; a2++)
                	{
                        	el2=tup->data+(a2-1);
				ktup2 = cindex_to_tuple(ktup2, el2->a, k, n);
                                for (k2=1; k2<=k; k2++)
                                {
                                        if (a1==a2 && k1==k2)
                                                continue;
					v2=ktup2[k2-1];
                                        len=12+numdigits(a1)+
                                               numdigits(a2)+numdigits(k1)+
                                               numdigits(k2);
					if (k1<k2 || (k1==k2&&a1<a2))
					{
                                            if (v1==v2)
                                                    len+=3;
                                            lits[i]=malloc(sizeof(char)*len);
                                            if (v1!=v2)
                                                    sprintf(lits[i],"eq[x-%d-%d.x-%d-%d]",k1,a1,k2,a2);
                                            else
                                                    sprintf(lits[i],"noteq[x-%d-%d.x-%d-%d]",k1,a1,k2,a2);
                                            i++;
					}
#ifdef REDFIND_SUCC
					if (v1!=v2+1)
						len = len+1;
					else
						len = len+4;
					lits[i] = malloc(sizeof(char)*len);
					if (v1!=v2+1)
						sprintf(lits[i],"eq[x-%d-%d.+x-%d-%d]",k1,a1,k2,a2);
					else
						sprintf(lits[i],"noteq[x-%d-%d.+x-%d-%d]",k1,a1,k2,a2);
					i++;
#endif
                                }
			}
			/* we did pairs of vars, now x-k1-a1=d */
                        for (d=0; d<n; d++)
                        {
                        	len=9+numdigits(k1)+numdigits(a1)
                                     +numdigits(d);
                                if (v1==d)
                                        len+=3;
                                lits[i]=malloc(sizeof(char)*len);
                                if (v1!=d)
                                        sprintf(lits[i],"eq[x-%d-%d.%d]",k1,a1,d);
                                else
                                        sprintf(lits[i],"noteq[x-%d-%d.%d]",k1,a1,d);
                                i++;
                        }

			/* now x-k1-a1=max */
#if !defined(REDFIND_FEWVARS) || defined(REDFIND_FV_0max)
			len = 9+numdigits(k1)+numdigits(a1)+3;
			if (v1==n-1)
				len+=3;
			lits[i]=malloc(sizeof(char)*len);
			if (v1!=n-1)
				sprintf(lits[i],"eq[x-%d-%d.max]",k1,a1);
			else
				sprintf(lits[i],"noteq[x-%d-%d.max]",k1,a1);
			i++;
#endif

			/* now x-k1-a1=c */
                        for (tc=cons; tc; tc=tc->next)
                        {
                                len=9+numdigits(k1)+numdigits(a1)
                                     +strlen(tc->name);
                                if (v1==tc->value)
                                        len+=3;
                                lits[i]=malloc(sizeof(char)*len);
                                if (v1!=tc->value)
                                        sprintf(lits[i],"eq[x-%d-%d.%s]",k1,a1,
                                                        tc->name);
                                else
                                        sprintf(lits[i],"noteq[x-%d-%d.%s]",k1,
                                                a1,tc->name);
                                i++;
                        }

                        
                }
        }

	/* now we've done veqv, veqc, veqi.  We still need ceqc and
         * those from add_pred
         */
        for (tc=cons; tc; tc=tc->next)
        {
                for (tc2=cons; tc2; tc2=tc2->next)
                {
                        if (tc==tc2)
                                continue;
                        len=6+strlen(tc->name)+strlen(tc2->name);
                        if (tc->value==tc2->value)
                                len+=3;
                        lits[i]=malloc(sizeof(char)*len);
                        if (tc->value!=tc2->value)
                                sprintf(lits[i],"eq[%s.%s]",tc->name,
                                                            tc2->name);
                        else
                                sprintf(lits[i],"noteq[%s.%s]",tc->name,
                                                tc2->name);
                        i++;
                }
        }

	free(ktup1);
        free(ktup2);

	/* now we just have the variables from add_pred */

	i = red_addincons_pred(rsearch, ex, er, lits, i);

	return res;
}

/* add the variables from add_pred, starting at lit[i], return last i+1 */
int red_addincons_pred(struct redsearch *rsearch, const struct example *ex,
                     const struct ex_rel *er, char **lits, int j)
{
	struct red_tuple *rtup;
        struct red_tuple *ertup=er->tup;
        struct cons_symbol *cs=rsearch->p1->voc->cons_symbols;
        struct constant *tc, *ac=ex->a->cons;
        struct relation *rel;
        struct red_tuple_element *el, *el1;

        int arity=ertup->arity;
        int n=rsearch->n;
        int k=rsearch->k;
        int i=j;
        int *reltup;
        int t;
        int ra;
        int *ktup=malloc(sizeof(int)*k);
        int va,vk;
        int flag;
        int tuplen, len;
        char *ts;

        rtup = NULL;
        cs = rsearch->p1->voc->cons_symbols;
        ac = ex->a->cons;
        for (rel=ex->a->rels; rel; rel=rel->next)
        {
                ra = rel->arity;
                reltup = malloc(sizeof(int)*ra);

		while ((rtup = red_nexttuple(rsearch, rtup, ra, k, arity, cs)))
                {
                        for (t=0; t<ra; t++)
                        {
                                el=rtup->data+t;
                                if (el->type==1)
                                {
                                        for (tc=ac; tc; tc=tc->next)
                                        {
                                                if (!strcmp(tc->name,
                                                     rtup->cons_names[el->a]))
                                                {
                                                        reltup[t]=tc->value;
                                                        break;
                                                }
                                        }
                                }
                                else
                                {
                                        vk = el->a;
                                        va = el->b;
                                        el1 = ertup->data+(va-1);
					ktup = cindex_to_tuple(ktup,el1->a,k,n);
                                        reltup[t]=ktup[vk-1];
                                }
                        }
                        /* now we have reltup. if ex\not\models rel(reltup),
                         * add rel[rtup] as lits[i], otherwise add the not.
                         */
			flag = rel->cache[tuple_cindex(reltup, rel->arity, n)];
                        tuplen = red_tupstrlen(rtup);
                        len = strlen(rel->name)+tuplen+1;
                        if (flag)
                                len+=3;
                        lits[i]=malloc(sizeof(char)*len);
                        ts = red_printtup(rsearch, rtup, tuplen);
                        if (flag)
                                sprintf(lits[i],"not%s%s",rel->name,ts);
                        else
                                sprintf(lits[i],"%s%s",rel->name,ts);
                        i++;
                        free(ts);
                }
                free(reltup);
        }

        free(ktup);
        return i;
}

/* return a single cons_bvars with all (except T) literals consistent with
 * ex and the (output) predicate in ex_rel being true.
 */
struct cons_bvars *red_getconslits(struct redsearch *rsearch, 
				   const struct example *ex, struct ex_rel *er)
{
	int num_cons=0;
	char **lits;
	struct cons_bvars *res;
	int i=0;
	int a1, a2, k1, k2;
	struct red_tuple *tup=er->tup;
	int k=rsearch->k, n=rsearch->n, arity=tup->arity;
	struct red_tuple_element *el1, *el2;
	int v1;
	int v2;
	int *ktup1=malloc(sizeof(int)*k), *ktup2=malloc(sizeof(int)*k);
	int len;
	struct constant *cons = ex->a->cons, *tc, *tc2;
	int d;

	num_cons = red_getnumconslits(rsearch, er);
	lits = malloc(sizeof(char *)*num_cons);

	res = malloc(sizeof(struct cons_bvars));
	res->num = num_cons;
	res->lits = lits;
	res->next = NULL;

	for (a1=1; a1<=arity; a1++)
	{
		el1 = tup->data+(a1-1);
		ktup1 = cindex_to_tuple(ktup1, el1->a, k, n);
		for (k1=1; k1<=k; k1++)
		{
			v1=ktup1[k1-1];
			for (a2=1; a2<=arity; a2++)
			{
				el2=tup->data+(a2-1);
				ktup2 = cindex_to_tuple(ktup2, el2->a, k, n);
				for (k2=1; k2<=k; k2++)
				{
					if (a1==a2 && k1==k2)
						continue;
					v2=ktup2[k2-1];
					len=12+numdigits(a1)+
					    numdigits(a2)+numdigits(k1)+
					    numdigits(k2);
					if (k1<k2 || (k1==k2&&a1<a2))
                                        {
					    if (v1!=v2)
				   		   len+=3;
					    lits[i]=malloc(sizeof(char)*len);
					    if (v1==v2)
					  	   sprintf(lits[i],"eq[x-%d-%d.x-%d-%d]",k1,a1,k2,a2);
					    else
						   sprintf(lits[i],"noteq[x-%d-%d.x-%d-%d]",k1,a1,k2,a2);
					    i++;
					}
#ifdef REDFIND_SUCC
                                        if (v1==v2+1)
                                                len = len+1;
                                        else
                                                len = len+4;
                                        lits[i] = malloc(sizeof(char)*len);
                                        if (v1==v2+1)
                                                sprintf(lits[i],"eq[x-%d-%d.+x-%d-%d]",k1,a1,k2,a2);
                                        else
                                                sprintf(lits[i],"noteq[x-%d-%d.+x-%d-%d]",k1,a1,k2,a2);
                                        i++;
#endif
				}
			}

			/* we did pairs of vars, now x-k1-a1=d */
			for (d=0; d<n; d++)
			{
				len=9+numdigits(k1)+numdigits(a1)
				     +numdigits(d);
				if (v1!=d)
					len+=3;
				lits[i]=malloc(sizeof(char)*len);
				if (v1==d)
					sprintf(lits[i],"eq[x-%d-%d.%d]",k1,a1,d);
				else
					sprintf(lits[i],"noteq[x-%d-%d.%d]",k1,a1,d);
				i++;
			}

                        /* now x-k1-a1=max */
#if !defined(REDFIND_FEWVARS) || defined(REDFIND_FV_0max)
                        len = 9+numdigits(k1)+numdigits(a1)+3;
                        if (v1!=n-1)
                                len+=3;
                        lits[i]=malloc(sizeof(char)*len);
                        if (v1==n-1)
                                sprintf(lits[i],"eq[x-%d-%d.max]",k1,a1);
                        else
                                sprintf(lits[i],"noteq[x-%d-%d.max]",k1,a1);
                        i++;
#endif

			/* now x-k1-a1=c */
			for (tc=cons; tc; tc=tc->next)
			{
				len=9+numdigits(k1)+numdigits(a1)
				     +strlen(tc->name);
				if (v1!=tc->value)
					len+=3;
				lits[i]=malloc(sizeof(char)*len);
				if (v1==tc->value)
					sprintf(lits[i],"eq[x-%d-%d.%s]",k1,a1,
							tc->name);
				else
					sprintf(lits[i],"noteq[x-%d-%d.%s]",k1,
						a1,tc->name);
				i++;
			}
	
	
		}
	}

	/* now we've done veqv, veqc, veqi.  We still need ceqc and
 	 * those from add_pred
	 */
	for (tc=cons; tc; tc=tc->next)
	{
		for (tc2=cons; tc2; tc2=tc2->next)
		{
			if (tc==tc2)
				continue;
			len=6+strlen(tc->name)+strlen(tc2->name);
			if (tc->value!=tc2->value)
				len+=3;
			lits[i]=malloc(sizeof(char)*len);
			if (tc->value==tc2->value)
				sprintf(lits[i],"eq[%s.%s]",tc->name,
							    tc2->name);
			else
				sprintf(lits[i],"noteq[%s.%s]",tc->name,
						tc2->name);
			i++;
		}
	}

	free(ktup1);
	free(ktup2);
	/* now we just have the variables from add_pred */
	i = red_addcons_pred(rsearch, ex, er, lits, i);

	return res;
}

/* add the variables from add_pred, starting at lit[i], return last i+1 */
int red_addcons_pred(struct redsearch *rsearch, const struct example *ex, 
		     const struct ex_rel *er, char **lits, int j)
{
	struct red_tuple *rtup;
	struct red_tuple *ertup=er->tup;
	struct cons_symbol *cs=rsearch->p1->voc->cons_symbols;
	struct constant *tc, *ac=ex->a->cons;
	struct relation *rel;
	struct red_tuple_element *el, *el1;

	int arity=ertup->arity;
	int n=rsearch->n;
	int k=rsearch->k;
	int i=j;
	int *reltup;
	int t;
	int ra;
	int *ktup=malloc(sizeof(int)*k);
	int va,vk;
	int flag;
	int tuplen, len;
	char *ts;
	
	rtup = NULL;
        cs = rsearch->p1->voc->cons_symbols;
	ac = ex->a->cons;
        for (rel=ex->a->rels; rel; rel=rel->next)
        {
		ra = rel->arity;
		reltup = malloc(sizeof(int)*ra);

		while ((rtup = red_nexttuple(rsearch, rtup, ra, k, arity, cs)))
		{
			for (t=0; t<ra; t++)
			{
				el=rtup->data+t;
				if (el->type==1)
				{
					for (tc=ac; tc; tc=tc->next)
					{
						if (!strcmp(tc->name,
						     rtup->cons_names[el->a]))
						{
							reltup[t]=tc->value;
							break;
						}
					}
				}
				else
				{
					vk = el->a;
					va = el->b;
					el1 = ertup->data+(va-1);
					ktup = cindex_to_tuple(ktup, el1->a,k,
							       n);
					reltup[t]=ktup[vk-1];
				}
			}
			/* now we have reltup. if ex\models rel(reltup),
			 * add rel[rtup] as lits[i], otherwise add the not.
			 */
			flag = rel->cache[tuple_cindex(reltup, rel->arity, n)];
			tuplen = red_tupstrlen(rtup);
			len = strlen(rel->name)+tuplen+1;
			if (!flag)
				len+=3;
			lits[i]=malloc(sizeof(char)*len);
			ts = red_printtup(rsearch, rtup, tuplen);
			if (!flag)
				sprintf(lits[i],"not%s%s",rel->name,ts);
			else
				sprintf(lits[i],"%s%s",rel->name,ts);
			i++;
			free(ts);
		}
		free(reltup);
	}

	free(ktup);
	return i;
}

int red_getnumconslits(struct redsearch *rsearch, struct ex_rel *er)
{
	int num_cons=0;
	int a;
	struct red_tuple *tup=er->tup;
	int k=rsearch->k, n=rsearch->n, arity=tup->arity;
	int numbcons;
	struct cons_symbol *cs;
	struct rel_symbol *rs;

	/* first, count how many are consistent, so we can allocate lits */

	/* arity-ary rel, k-ary reduc, so k*arity many vars. For each,
	 * x-i-j=m is consistent for one value m (given by cindex_to_tuple)
	 * and x-i-j!=m is consistent for n-1 values m (the others),
	 * so k*arity*n consistent (not)x-i-j=m.
	 */
	num_cons+=k*arity*n;

	/* for each of (k*arity)^2-(k*arity) many distinct pairs of variables, 
	 * either they're equal or they're not
	 */
	num_cons += ((k*arity)*(k*arity-1))>>1;
#ifdef REDFIND_SUCC
	/* for each distinct pair of variables, x=y+1 or not */
	num_cons+=(k*arity)*(k*arity-1);
#endif

	/* for each of (numb_cons)*k*arity constant (of p1->voc!)-variable
	 * pairs, either they're equal or they're not
	 */
	for (cs=rsearch->p1->voc->cons_symbols,numbcons=0; cs; 
	     cs=cs->next,numbcons++);
	num_cons+=(k*arity)*numbcons;

	/* for each pair of (distinct) 
	 * p1->voc constants, either they're equal in ex or not */
	num_cons+=numbcons*numbcons-numbcons;

	/* for each rel_symbol, add all possible tuples (either true or not is
							 true)
	 */
	for (rs=rsearch->p1->voc->rel_symbols; rs; rs=rs->next)
	{
		a=rs->arity;
		num_cons+=de_pow(k*arity+numbcons, a); /* (k*arity+numbcons)^a
							* many tuples 
							*/
	}

#if !defined(REDFIND_FEWVARS) || defined(REDFIND_FV_0max)
	num_cons += k*arity; /* for each var, =max or not */
#endif
	return num_cons; /* hopefully no overflow :-) */
}

struct cons_bvars *red_cons_addlit(struct cons_bvars *cb, const char *c)
{
	struct cons_bvars *res=malloc(sizeof(struct cons_bvars));
	int i;
	res->num=cb->num+1;
	res->next=cb->next;
	res->lits=malloc(sizeof(char *)*res->num);
	for (i=0; i<res->num-1; i++)
		res->lits[i]=cb->lits[i];
	res->lits[i]=malloc(sizeof(char)*(strlen(c)+1));
	sprintf(res->lits[i],"%s",c);
	free(cb->lits);
	free(cb);
	return res;
}
struct cons_bvars *red_cons_addtrue(struct cons_bvars *cb)
{
	struct cons_bvars *res=malloc(sizeof(struct cons_bvars));
	res->num=1;
	res->next=cb;
	res->lits=malloc(sizeof(char *));
	res->lits[0]=malloc(sizeof(char)*2);
	sprintf(res->lits[0],"T");
	return res;
}

/* We're making a disjunction of Boolean variables inconsistent with
 * relname(tuple), and form is what we have so far.  Add the
 * form{relname}_clause{i}_* part to form.
 * If form is NULL, return the initial form.
 */
char *red_rf_pred_inconsclause(struct redsearch *rsearch, int cl, 
			       struct cons_bvars *cb, struct ex_rel *er)
{
	char *relname=er->relname;
	int len=0;
        char *tmp, *res=NULL;
        int i, num=cb->num;
        char **lits = cb->lits;
        int rln=strlen(relname);
        int ncl = numdigits(cl);

	for (i=0; i<num; i++)
        {
                len = strlen(lits[i])+4+rln+1+6+ncl+1+1;
                tmp = malloc(sizeof(char)*len);
                sprintf(tmp,"form%s_clause%d_%s",relname,cl,lits[i]);
                res = red_disjunc(rsearch, res,tmp);
        }

	tmp=malloc(sizeof(char)*(strlen(res)+3));
        sprintf(tmp,"(%s)",res);
        free(res);
        res=tmp;
        return res;
}
/* We're making the formula for relname(tuple), and form is what we
 * have so far.  Add the form{relname}_clause{i}_* part to form.
 * If form is NULL, return the initial form
 */
char *red_rf_pred_claused(struct redsearch *rsearch, char *form, int i,
			  struct cons_bvars *cons_bvars, struct ex_rel *er)
{
	struct cons_bvars *cb;
	char *tmp, *res=NULL;

	for (cb=cons_bvars; cb; cb=cb->next)
	{
		tmp = red_rf_pred_cbform(rsearch, er->relname, i, cb);
		res = red_disjunc(rsearch, res, tmp);
	}

	res = red_disjunc(rsearch, form, res);
	return res;	
}

char *red_rf_pred_cbform(struct redsearch *rsearch, char *relname, int cl, 
			 struct cons_bvars *cb)
{
	int len=0;
	char *tmp, *res=NULL;
	int i, num=cb->num;
	char **lits = cb->lits;
	int rln=strlen(relname);
	int ncl = numdigits(cl);

	for (i=0; i<num; i++)
	{
		len = strlen(lits[i])+4+rln+1+6+ncl+1+1;
		tmp = malloc(sizeof(char)*len);
		sprintf(tmp,"form%s_clause%d_%s",relname,cl,lits[i]);
		res = red_conjunc(rsearch, res,tmp);
	}

	tmp=malloc(sizeof(char)*(strlen(res)+3));
	sprintf(tmp,"(%s)",res);
	free(res);
	res=tmp;
	return res;
}

char *make_rf_eq(struct redsearch *rsearch, const struct example *ex,
		 struct node *form, struct interp *interp)
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
			return dupstr("T");
		return dupstr("F"); 
	}

	if (leftinc_c && !rightinc_c)
	{
		right = teval(form->r, interp, NULL);
		return make_rf_teqi(rsearch, ex, form->l, right);
	}

	if (!leftinc_c && rightinc_c)
	{
		left = teval(form->l, interp, NULL);
		return make_rf_teqi(rsearch, ex, form->r, left);
	}

	else /* the trickiest */
		return make_rf_teqt(rsearch, form);
}

/* Return the formula corresponding to form->l=form->r, where both are terms.
 * For now, we assume that both are single constant symbols.
 */
char *make_rf_teqt(struct redsearch *rsearch,
		   struct node *form)
{
	char *leftc, *rightc;
	struct node *left, *right;
	struct cons_symbol *p1cons;
	struct red_tuple *tuple=NULL;
	int k=rsearch->k;
	int n=rsearch->n;
	char *res=NULL;

	left=form->l;
	right=form->r;

	if (left->label!=CONSTANT || right->label!=CONSTANT)
	{
		rsearch->abort=1;
		printf("r21: Reduction finding with non-atomic terms containing constants is not yet supported.\n");
		return dupstr("T"); 
	}

	leftc=(char *)left->data;
	rightc=(char *)right->data;

	if (!strcmp(leftc,rightc))
		return dupstr("T"); 

	p1cons=rsearch->p1->voc->cons_symbols;
	while ((tuple=red_nextconstuple(rsearch,tuple,k,n,p1cons)))
		res=red_form_addconstuple(res, tuple,leftc,rightc);

	return res;
}

/* return form|(cons_leftc_tuple & cons_rightc_tuple) */
/* if form is null, omit the first '|' */
/* free the old form */
char *red_form_addconstuple(char *form, struct red_tuple *tuple, 
			    char *leftc, char *rightc)
{
	char *tmp1, *tmp2, *tmp3, *res;

	tmp1 = red_getconsbvarname(leftc, tuple);
	tmp2 = red_getconsbvarname(rightc, tuple);

	tmp3 = malloc(sizeof(char)*(strlen(tmp1)+strlen(tmp2)+4));
	sprintf(tmp3,"(%s&%s)",tmp1,tmp2);
	if (!form)
		return tmp3;
	res = malloc(sizeof(char)*(strlen(form)+strlen(tmp3)+2));
	sprintf(res,"%s|%s",form,tmp3);
	free(tmp3);
	free(form);
	return res;
}

/* Return the formula corresponding to (term=val) using bvars.
 * For now, we assume that term is a single constant symbol.
 */
char *make_rf_teqi(struct redsearch *rsearch, const struct example *ex, 
		   struct node *term, int val)
{
	char *name;
	int k=rsearch->k;
	int n=rsearch->n;
	int *tuple;
	char *res;
	struct red_tuple *tup;

	if (term->label!=CONSTANT)
	{
		rsearch->abort=1;
		printf("r22: Sorry, complicated terms containing constants are not supported yet.\n");
		return dupstr("T");
	}

	tuple = malloc(sizeof(int)*k);
	tuple = cindex_to_tuple(tuple, val, k, n);
	name = (char *)term->data;

	/* now, we have name = tuple.
	 * However, it is possible that some constants (from ex)
	 * share the values of parts of tuple (i.e., s=[0,0] can be
	 * satisfied by a reduction sending s<-[s,s] if s=0 in this
	 * example.
	 * So, we have to do a disjunction over tuple and all possible
 	 * equivalent tuples with parts replaced by constants.
	 */

	res = NULL;
	tup = NULL;

	while ((tup = red_getnext_compat_tuple(tup, tuple, ex, k)))
		res = red_rf_addconsclause(res, name, tup);

	free(tuple);
	return res;
}

/* add cons_name_[tup] to the disjunction form and return it.
 * if form is NULL, return a suitable form.
 */
char *red_rf_addconsclause(char *form, const char *name, struct red_tuple *tup)
{
	char *res;
	char *tmp;

	tmp = red_getconsbvarname(name, tup);
	if (!form)
		return tmp;
	
	res = malloc(sizeof(char)*(strlen(tmp)+strlen(form)+2));
	sprintf(res,"%s|%s",form,tmp);
	free(tmp);
	return res;
}

/* returns the next red_tuple in an enumeration of tuples over integers and
 * constant symbols of ex that are consistent with the (integer) tuple tuple.
 * if rtup is NULL, returns the first in an enumeration.  If rtup is the last
 * in the enumeration, frees it and returns NULL.
 */
struct red_tuple *red_getnext_compat_tuple(struct red_tuple *rtup, int *tuple,
					const struct example *ex, int k)
{
	struct red_tuple *res;
	struct red_tuple_element *el;
	int i,j,flag;
	struct constant *cons;

	if (!rtup)
	{
		res = malloc(sizeof(struct red_tuple));
		res->arity=k;
		res->data = malloc(sizeof(struct red_tuple_element)*k);
		for (i=0,el=res->data; i<k; i++,el++)
		{
			el->type = 2;
			el->a = tuple[i];
		}

		for (cons=ex->a->cons,i=0; cons; cons=cons->next,i++);
		res->num_cons=i;
		if (i==0)
		{
			res->cons_names = NULL;
			return res;
		}
		res->cons_names=malloc(sizeof(char *)*i);
		for (j=0,cons=ex->a->cons; j<i; j++,cons=cons->next)
			res->cons_names[j]=cons->name;
		return res;
	}
	
	/* otherwise rtup is a red_tuple, we need to increment it */

	if (rtup->num_cons==0)
	{
		free(rtup->data);
		free(rtup->cons_names);
		free(rtup);
		return NULL;
	}

	el = rtup->data;
	for (i=k-1; i>=0; i--) /* find a spot to increment */
	{
		cons=NULL;
		if (el[i].type==1 && el[i].a==rtup->num_cons-1)
		{
			el[i].type=2;
			el[i].a=tuple[i];
			continue;
		}
		else
		{
			/* we need to check if there's another constant
			 * compatible with el[i]
			 */
			/* there are better ways of doing this, but it's not
			 * critical.
			 */
			flag=0;
			for (cons=ex->a->cons,j=0; cons; cons=cons->next,j++)
			{
				if (cons->value!=tuple[i])
					continue;
				if (el[i].type==1 && el[i].a>=j)
				{
					/* already used this constant */
					continue;
				}
				flag=1;
				break;
			}
			if (flag) /* we found one */
				break;
			/* otherwise we need to check the next place */
			el[i].type=2;
                        el[i].a=tuple[i];
			continue;
		}
	}

	if (i==-1)
	{
		free(rtup->data);
                free(rtup->cons_names);
                free(rtup);
                return NULL;
        }

	/* otherwise we found a spot to increment */
	assert(flag==1);
	assert(cons->value==tuple[i]);
	el[i].type=1;
	el[i].a=j;
	return rtup;
}

/* form is a term.  Return 1 if it has any constant symbols, 0 otherwise */
int term_has_cons(struct node *form)
{
	switch (form->label)
	{
		case MULT:
		case PLUS:
		case MINUS:
			return (term_has_cons(form->l)||term_has_cons(form->r));
		case CONSTANT:
			return 1;
		case VAR:
		case NUMBER:
			return 0;
		default:
			return 0;
	}
}

/* \forall x:\phi === !\exists x:!\phi */
/* based on ex_p1rec_forall, which is based on eval_forall */
char *make_rf_forall(struct redsearch *rsearch, const struct example *ex,
		     struct node *form, struct interp *interp)
{
	char *tmp, *ret;
        struct node *not = node(NOT, form->r, 0);
        if (!not)
                return NULL;
        form->r=not;
        form->label = EXISTS;
        tmp = make_rf_exists(rsearch,ex,form,interp);
        form->r = not->l;
        form->label = FORALL;
        free(not);

        ret = malloc(sizeof(char)*(strlen(tmp)+4));
        sprintf(ret,"!(%s)",tmp);
        free(tmp);
        return ret;
}
/* based on ex_p1rec_exists, which is based on eval_exists */
char *make_rf_exists(struct redsearch *rsearch, const struct example *ex,
		     struct node *form, struct interp *interp)
{
	/* NOTE WE ASSUME THAT phi0 is \t! */
	char **varnames;
	int size=rsearch->outsize;
        int arity=0;
        int i;
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

        varnames = malloc(arity*sizeof(char *));
	old_values = malloc(arity*sizeof(int));
	values = malloc(arity*sizeof(int *));
        tnode = varlist;

        for (i=0; i<arity; i++)
        {
                varnames[i]=tnode->data;
		values[i]=tnode->ival;
		old_values[i]=*values[i];
		*values[i]=0;
                tnode = tnode->r;
        }

	first = values[0];
	*first = -1;

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
                            res=*values[i] = 0;
                    }
                    if (!res && i==arity)
                            break;
            }
            if (restr)
		tmpl = make_rf_rec(rsearch,ex,restr,interp);
            tmpr = make_rf_rec(rsearch,ex,phi,interp);

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
		*values[i]=old_values[i];
       free(values);
       free(old_values);
       /* free(vartuple); */
       free(varnames);
       return ret;
}

/* Returns form1|form2 and frees form1, form2.
 * If !form1, returns form2 
 */
char *red_disjunc(struct redsearch *rsearch, char *form1, char *form2)
{
	int len1, len2, i, j;
        char *tmp;
        if (!form1)
                return form2;
        len1 = strlen(form1);
	len2 = strlen(form2);
        tmp = realloc(form1,sizeof(char)*(len1+len2+2));
        if (!tmp)
        {
                rsearch->abort=1;
                printf("r24: No memory\n");
                free(form2);
                return form1;
        }
        tmp[len1]='|';
	/* like strcat but faster */
        for (i=len1+1,j=0; j<=len2; i++,j++)
                tmp[i] = form2[j];      
        free(form2);
        return tmp;
}

/* Returns form1&form2 and frees form1, form2.
 * If !form1, returns form2 
 */
char *red_conjunc(struct redsearch *rsearch, char *form1, char *form2)
{
        int len1, len2, i,j;
        char *tmp;
        if (!form1)
                return form2;
	len1 = strlen(form1);
	len2 = strlen(form2);
        tmp = realloc(form1,sizeof(char)*(len1+len2+2));
        if (!tmp)
        {
		rsearch->abort=1;
                printf("r24: No memory\n");
                free(form2);
                return form1;
        }
	tmp[len1]='&';
	/* like strcat but faster */
	for (i=len1+1,j=0; j<=len2; i++,j++)
		tmp[i] = form2[j];
        free(form2);
        return tmp;
}

/* Returns (form1)op(form2) and frees form1,form2 */
char *red_binform(struct redsearch *rsearch, char *form1, char *form2, const char *op)
{
	int len;
	char *tmp;
	len = strlen(form1)+strlen(form2)+5+strlen(op); /* ()op() and \0 */
	tmp = malloc(sizeof(char)*len);
	if (!tmp)
	{
		rsearch->abort=1;
		printf("r25: No memory\n");
		free(form2);
		return form1;
	}
	sprintf(tmp,"(%s)%s(%s)",form1,op,form2);
	free(form1);
	free(form2);
	return tmp;
}

/* This returns the Boolean variables, named form{name}_clause%d_%s.
 * and form{name}_clause%d_not%s, where the last string gives the
 * meaning.  rsearch is not yet fully initialized.
 * Also adds TRUE and FALSE.
 */
/* 
 * and something for constants, etc.
 */
struct red_bvarlist *red_makebvars(struct redsearch *rsearch,
				   const struct bquery *p1, 
				   const struct bquery *p2,
				   int k, int c, int n2)
{
	int j;
	int n=n2;
	char *tmp;
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *hash=rsearch->hash;
	struct red_bvarlist *bvars=NULL, *bv;
	int base, numl;
	struct rel_symbol *rel, *rel2;
	struct cons_symbol *cons1, *cons2;
	char *name;
	int a_i, vk, va;

	for (rel=p2->voc->rel_symbols; rel; rel=rel->next)
	{
		name = rel->name;
		base = (6+strlen(name)+1+6); 
		base++; /* closing \0 */
                /* form{name}_clause */
		a_i=rel->arity;
		for (j=1; j<=c; j++)
		{
			numl=sizeof(char)*(numdigits(j)+1); /* j_ */
			/* make Boolean variables formN_clausei_* */

			/* T */
			tmp = malloc(sizeof(char)*(base+numl+1));
			if (!tmp)
			{
				rsearch->abort=1;
				printf("r26: No memory.\n");
				return 0;
			}
			sprintf(tmp,"form%s_clause%d_T",name,j);

			bv = red_addtolist(bvars, tmp, solver, hash);
			if (!bv)
			{
				rsearch->abort=1;
				printf("r27: No memory.\n");
				free(tmp);
				return 0;
			}
			bvars = bv;

			/* k-ary reduction and a_i-ary rel, so variables
			 * "x-vk-va"
			 */
			for (vk=1; vk<=k; vk++)
				for (va=1; va<=a_i; va++)
				{
					/* add x-vk-va=i for 0<=i<n */
					bvars = red_add_veqi(rsearch,
							     bvars, name, j,
							     n, vk, va, 
							     base+numl);
					/* TODO check return value */

					/* add x-vk-va=x for other vars x */
					/* also ads x-vk-va=x+1 for other x*/
					bvars = red_add_veqv(rsearch,
							     bvars, name, j, 
							     k, a_i, vk, va,
							     base+numl);

					/* add x-vk-va=c for constants c */
					bvars = red_add_veqc(rsearch,
							bvars, name, j,
							p1->voc->cons_symbols,
							vk,va,base+numl);

				}
		/* TODO only need to start cons2 at cons1->next! */
			for (cons1=p1->voc->cons_symbols; cons1; 
			     cons1=cons1->next)
				for (cons2=p1->voc->cons_symbols; cons2;
				     cons2=cons2->next)
				{
					if (cons1==cons2)
						continue;
					bvars = red_add_ceqc(rsearch,
							     bvars,name,j,
							     cons1->name,
							     cons2->name,
							     base+numl);
				}
			for (rel2=p1->voc->rel_symbols; rel2; rel2=rel2->next)
				bvars = red_add_pred(rsearch, bvars, rel2, 
						     j, name, a_i, k,
					     p1->voc->cons_symbols,base+numl);
		}
	}

	return bvars;
}

/* add form{name}_clause{c}_eq[x-vk-va.x-I-J] for 1<=I<=k and 1<=J<=a_i,
 * omitting the case where I=vk&J=va.
 */
struct red_bvarlist *red_add_veqv(struct redsearch *rsearch,
				  struct red_bvarlist *bvars, 
				  const char *name, int c,
			  	  int k, int a_i, int vk, int va, int len)
{
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *hash = rsearch->hash;
	char *tmp;
	int i,j,a,b,q=numdigits(vk)+numdigits(va);
	struct red_bvarlist *bv=bvars;

	for (i=1; i<=k; i++)
	{
		a=numdigits(i)+q;
		for (j=1; j<=a_i; j++)
		{
			/* variables always equal themselves */
			/* only need one of (x-k1-a1=x-k2-a2) (x-k2-a2=x-k1-a1)
			 * so keep the one s.t. (k1<k2 | (k1=k2 & a1<a2)) */
			if (i==vk && j==va)
				continue; /* variables always equal themselves*/
			b=numdigits(i);
			if (vk<i || (vk==i && va<j))
			{
				tmp = malloc(sizeof(char)*(len+11+a+b));
				if (!tmp)
                		{
                        		red_abort_nomem(rsearch, bvars);
                        		return 0;
                		}
                		sprintf(tmp,"form%s_clause%d_eq[x-%d-%d.x-%d-%d]",name,
					c,vk,va,i,j);
				bv = red_addtolist(bv, tmp, solver, hash);
                		if (!bv)
                		{
					rsearch->abort=1;
                        		printf("r28: No memory.\n");
                        		return 0;
                		}
			}
#ifdef REDFIND_SUCC
			tmp = malloc(sizeof(char)*(len+11+a+b+1));
			if (!tmp)
			{
				red_abort_nomem(rsearch,bvars);
				return 0;
			}
			sprintf(tmp, "form%s_clause%d_eq[x-%d-%d.+x-%d-%d]",name,
				c,vk,va,i,j);
			bv = red_addtolist(bv, tmp, solver, hash);
			if (!bv)
			{
				rsearch->abort=1;
				printf("r??: No memory.\n");
				return 0;
			}
#endif
		}
	}
	
	return bv;
}

/* add form{name}_clause{c}_eq[x-vk-va.i] for 0<=i<n */
struct red_bvarlist *red_add_veqi(struct redsearch *rsearch,
			  	  struct red_bvarlist *bvars, const char *name, 
			  	  int c, int n, int vk, int va, int len)
{
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *hash=rsearch->hash;
	char *tmp;
	int i,a;
	int n1=rsearch->n1;
	struct red_bvarlist *bv=bvars;
	for (i=0; i<n; i++)
	{
		a=numdigits(i); 
		tmp = malloc(sizeof(char)*(len+8+a));
		if (!tmp)
                {
                	red_abort_nomem(rsearch, bvars);
                        return 0;
                }
		sprintf(tmp,"form%s_clause%d_eq[x-%d-%d.%d]", name,c,vk,va,i);
		bv = red_addtolist(bv, tmp, solver, hash);
                if (!bv)
              	{
			rsearch->abort=1;
                        printf("r29: No memory.\n");
                        return 0;
                }
		if (i>=n1)
		{
			minisat_addClause_begin(solver);
                 	minisat_addClause_addLit(solver, 
				minisat_negate(bv->posLit));
                 	minisat_addClause_commit(solver);
                 	minisat_addClause_begin(solver);
                 	minisat_addClause_addLit(solver, 
				minisat_negate(bv->negLit));
                 	minisat_addClause_commit(solver);
		}
#ifdef REDFIND_FEWVARS /* tell MiniSat not to use the x=i literals in reductions */
#ifdef REDFIND_FV_0max /* but if set, we want to use x=0 and x=max */
		if (i==0)
			continue;
#endif
		 minisat_addClause_begin(solver);
                 minisat_addClause_addLit(solver, minisat_negate(bv->posLit));
                 minisat_addClause_commit(solver);
                 minisat_addClause_begin(solver);
                 minisat_addClause_addLit(solver, minisat_negate(bv->negLit));
                 minisat_addClause_commit(solver);
#endif
         }
         return bv;
}

/* add form{name}_clause{c}_eq[x-vk-va.c] for constants c */
struct red_bvarlist *red_add_veqc(struct redsearch *rsearch,
			struct red_bvarlist *bvars, const char *name, int c,
			  struct cons_symbol *cons, int vk, int va, int len)
{
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *hash=rsearch->hash;
	char *tmp;
	struct cons_symbol *ct;
	int a=numdigits(vk)+numdigits(va);
	struct red_bvarlist *bv=bvars;

	for (ct=cons; ct; ct=ct->next)
	{
		tmp=malloc(sizeof(char)*(len+9+a+strlen(ct->name)));
		if (!tmp)
		{
			red_abort_nomem(rsearch, bv);
			return 0;
		}
		sprintf(tmp,"form%s_clause%d_eq[x-%d-%d.%s]",name,c,
			    vk, va, ct->name);
		bv = red_addtolist(bv, tmp, solver, hash);
		if (!bv)
		{
			rsearch->abort=1;
			printf("r30: No memory.\n");
			return 0;
		}
	}

/* and x=max */
#if !defined(REDFIND_FEWVARS) || defined(REDFIND_FV_0max)
        tmp = malloc(sizeof(char)*(len+9+a+3));
	if (!tmp)
        {
        	red_abort_nomem(rsearch, bv);
        	return 0;
        }
	sprintf(tmp, "form%s_clause%d_eq[x-%d-%d.max]",name,c,vk,va);
	bv = red_addtolist(bv, tmp, solver, hash);
	if (!bv)
	{
		rsearch->abort=1;
		printf("r30: No memory (max).\n");
		return 0;
	}
#endif
	return bv;
}

/* add form{name}_clause{c}_eq[c1.c2] */
struct red_bvarlist *red_add_ceqc(struct redsearch *rsearch,
			  struct red_bvarlist *bvars, const char *name, int c, 
			  const char *c1, const char *c2, int len)
{
	struct minisat_solver_t *solver=rsearch->solver;
        struct env *hash=rsearch->hash;
	char *tmp = malloc(sizeof(char)*(len+5+strlen(c1)+strlen(c2)));
	if (!tmp)
	{
		red_abort_nomem(rsearch, bvars);
		return 0;
	}
	sprintf(tmp, "form%s_clause%d_eq[%s.%s]",name,c,c1,c2);
	return red_addtolist(bvars, tmp, solver, hash);
}

/* add all Boolean variables "form{name}_clause{c}_{rel}[A1.A2.....Aa_r]"
 * where each Aj is either a variable x-vk-va (vk<=k, va<=a_i) or a constant.
 */
/* len is the length of "form{pred}_clause{c}_"+1. */
struct red_bvarlist 
	    *red_add_pred(struct redsearch *rsearch,
			  struct red_bvarlist *bvars, 
			  const struct rel_symbol *rel, int c, const char *name,
			  int a_i, int k, struct cons_symbol *cons, int len)
{
	char *tmp;
	struct minisat_solver_t *solver=rsearch->solver;
	struct env *hash=rsearch->hash;
	struct red_tuple *tuple=NULL;
	int tuplen;
	int size;
	int arity=rel->arity;
	char *relname = rel->name;
	char *ts;
	struct red_bvarlist *bv=bvars;

	while ((tuple=red_nexttuple(rsearch, tuple, arity, k, a_i, cons)))
	{
		tuplen = red_tupstrlen(tuple);
		size = len+strlen(name)+strlen(relname)+tuplen;
		tmp = malloc(sizeof(char)*size);
		if (!tmp)
		{
			red_abort_nomem(rsearch, bv);
        	        return 0;
        	}
		ts = red_printtup(rsearch, tuple,tuplen);
		/* TODO check ts==NULL and abort */
		sprintf(tmp,"form%s_clause%d_%s%s",name,c,relname,ts);
		free(ts);
		bv = red_addtolist(bv, tmp, solver, hash);
                if (!bv)
                {
			if (tuple)
				red_freertup(tuple);
			rsearch->abort=1;
                        printf("r31: No memory.\n");
                        return 0;
                }	
	}
	return bv;
}

/* return char * to [A.B.C...] for tuple (the string is len characters
 * long, add 1 for closing \0
 */
char *red_printtup(struct redsearch *rsearch, const struct red_tuple *tuple,
		   int len)
{
	int i;
	int arity=tuple->arity;
	char **cnames = tuple->cons_names;
	struct red_tuple_element *el=tuple->data;
	char *tmp=malloc(sizeof(char)*(len+1));
	char *tmp2=malloc(sizeof(char)*(len+1));
	if (!tmp)
	{
		rsearch->abort=1;
		printf("r32: No memory.\n");
		return 0;
	}
	if (!tmp2)
	{
		rsearch->abort=1;
		printf("r33: No memory.\n");
		return 0;
	}
	tmp[0]='[';
	tmp[1]='\0';

	for (i=0; i<arity; i++)
	{
		if (el[i].type==1)
		{
			strcat(tmp,cnames[el[i].a]);
			strcat(tmp,".");
		}
		else if (el[i].type==2)
		{
			sprintf(tmp2,"%d.",el[i].a);
			strcat(tmp,tmp2);
		}
		else
		{
			sprintf(tmp2,"x-%d-%d.",el[i].a,el[i].b);
			strcat(tmp,tmp2);
		}
	}
	tmp[strlen(tmp)-1]=']';
	free(tmp2);
	return tmp;
}

/* the number of characters in [A.B.C...] when printing tuple */
int red_tupstrlen(const struct red_tuple *tuple)
{
	int i,len=1,arity=tuple->arity;
	struct red_tuple_element *el=tuple->data;
	for (i=0; i<arity; i++)
	{
		if (el[i].type==1)
			len+=1+strlen(tuple->cons_names[el[i].a]);
		else if (el[i].type==2)
			len+=numdigits(el[i].a)+1;
		else /* x-a-b. */
			len+=1+3+numdigits(el[i].a)+numdigits(el[i].b);
	}
	/* we counted the closing ] as a period after the last element */
	return len;
}

/* modify tuple to be the next arity-ary tuple in some fixed enumeration of
 * [A1.A2.....Aa_r]
 * where each Aj is either a variable x-vk-va (vk<=k, va<=a_i) or a constant
 * (given in cons).
 * If tuple is NULL, make it the first tuple.  If tuple is the last tuple,
 * free it and return NULL.
 */ 
struct red_tuple *red_nexttuple(struct redsearch *rsearch,
				struct red_tuple *tuple, int arity, int k,
				int a_i, struct cons_symbol *cons)
{
	struct red_tuple *tup;
	struct red_tuple_element *el;
	int i;
	struct cons_symbol *ct;
	if (!tuple)
	{
		tup = malloc(sizeof(struct red_tuple));
		if (!tup)
		{
			rsearch->abort=1;
			printf("r34: No memory\n");
			return 0;
		}
		for (i=0,ct=cons;ct;i++,ct=ct->next);
		if (i)
		{
			tup->cons_names=malloc(sizeof(char *)*i);
			if (!tup->cons_names)
			{
				rsearch->abort=1;
				printf("r35: No memory\n");
				free(tup);
				return 0;
			}
		}
		else
			tup->cons_names=0;
		el=malloc(sizeof(struct red_tuple_element)*arity);
		if (!el)
		{
			rsearch->abort=1;
			printf("r36: No memory\n");
			if (tup->cons_names)
				free(tup->cons_names);
			free(tup);
			return 0;
		}
		tup->data=el;
		for (i=0,ct=cons;ct;i++,ct=ct->next)
			tup->cons_names[i]=ct->name;
		tup->num_cons=i;
		tup->arity=arity;
		for (i=0; i<arity; i++) /* must be at least one var because
					 * no 0-ary reductions and no 0-ary
					 * predicate.
					 */
		{
			el[i].type=0;
			el[i].a=el[i].b=1;
		}
		return tup;
	}

	el = tuple->data;
	/* otherwise, tuple is a tuple and we need to get the next one. */
	for (i=arity-1; i>=0; i--) /* find a spot to increment */
	{
		if ((el[i].type==1 && el[i].a==tuple->num_cons-1) ||
		    (tuple->num_cons==0 && el[i].type==0 && el[i].a==k && 
		     el[i].b==a_i))
		{
			el[i].type=0;
			el[i].a=el[i].b=1;
			continue;
		}
		else
			break;
	}

	if (i==-1)
	{
		free(tuple->data);
		free(tuple->cons_names);
		free(tuple);
		return NULL;
	}

	/* otherwise we found a spot to increment */
	if (el[i].type == 1)
	{
		el[i].a++;
		return tuple;
	}

	if (el[i].a==k && el[i].b==a_i)
	{
		el[i].type = 1;
		el[i].a=0;
		return tuple;
	}

	if (el[i].b==a_i)
	{
		el[i].a++;
		el[i].b=1;
		return tuple;
	}

	el[i].b++;
	return tuple;
}

/* Ran out of memory, give an error and free everything in v */
int red_abort_nomem(struct redsearch *rsearch, struct red_bvarlist *v)
{
	struct red_bvarlist *t=v, *n;
	while (t)
	{
		n=t->next;
		free(t->pos);
		free(t->neg);
		free(t);
		t=n;
	}
	rsearch->abort=1;
	printf("r37: No memory (abort).\n");
	return 0;
}

/* add tmp (and its negation) to the list, and the hash */
struct red_bvarlist *red_addtolist(struct red_bvarlist *bvars, char *tmp, 
	      	     struct minisat_solver_t *solver, struct env *hash)
{
        char *negname=malloc(sizeof(char)*(strlen(tmp)+4));
        struct red_bvarlist *list=malloc(sizeof(struct red_bvarlist));
        int i;

        /* TODO check those mallocs */
        strcpy(negname,tmp);
        for (i=strlen(negname); ; i--) /* start at the \0 */
        {
                if (negname[i]!='_')
                        negname[i+3]=negname[i];
                else
                {
                        negname[i+1]='n';
                        negname[i+2]='o';
                        negname[i+3]='t';
                        break;
                }
        }

        list->pos=tmp;
        list->neg=negname;
        list->posVar=minisat_newVar(solver);
        minisat_setFrozen(solver, list->posVar, minisat_l_True);
        list->posLit=minisat_mkLit(list->posVar);
        list->negVar=minisat_newVar(solver);
        minisat_setFrozen(solver, list->negVar, minisat_l_True);
        list->negLit=minisat_mkLit(list->negVar);
        list->next = bvars;

        hash_alloc_insert(hash->id_hash, tmp, list);
        hash_alloc_insert(hash->id_hash, negname, list);
        return list;
}

/* Doesn't free the vocab because that's used elsewhere */
int free_ex(struct example *ex)
{
	struct structure *a=ex->a;
	struct relation *r1, *r2;
	struct constant *c1, *c2;
	free(ex);
	for (r1=a->rels; r1; r1=r2)
	{
		r2=r1->next;
		free(r1->name);
		free(r1->cache);
		/* we leak the r1->parse_cache I guess */
		free(r1);
	}
	for (c1=a->cons; c1; c1=c2)
	{
		c2=c1->next;
		free(c1->name);
		free(c1);
	}
	free(a->name);
	free(a);
	return 1;
}	

/* return 1 iff varname is a negvar */
int red_isnegvar(const char *varname)
{
	int i,len=strlen(varname);
	int flag=0;
	for (i=0; i<len; i++)
		if (varname[i]=='_')
			if (flag++>0)
				break;
	if (flag!=2 || (len-i)<5)
		return 0;

	if (varname[i+1]=='n' && varname[i+2]=='o' && varname[i+3]=='t')
		return 1;
	return 0;
}

minisat_Lit red_getnegLit(struct env *env, const char *name)
{
        struct hnode_t *hnode;
        struct red_bvarlist *bv;
        hnode = hash_lookup(env->id_hash, name);
        bv = (struct red_bvarlist *)hnode_get(hnode);
        return bv->negLit;
}

minisat_Lit red_getposLit(struct env *env, const char *name)
{
        struct hnode_t *hnode;
	struct red_bvarlist *bv;
	hnode = hash_lookup(env->id_hash, name);
        bv = (struct red_bvarlist *)hnode_get(hnode);
	return bv->posLit;
}

/* add the formula defining rel (given in the satisfying assignment of
 * rsearch->solver to the list of relforms and return the new list of
 * relforms.
 */
struct red_relforms *red_addrelform(struct red_relforms *relforms, 
			    struct rel_symbol *rel, 
			    struct redsearch *rsearch)
{
	struct red_relforms *newr=NULL;
	struct red_bvarlist *bvars=rsearch->bvars;
	char *relname = rel->name;
	struct minisat_solver_t *solver=rsearch->solver;

	int c=rsearch->c;
	int k=rsearch->k;
	/* int n=rsearch->n; */
	
	for (bvars=rsearch->bvars; bvars; bvars=bvars->next)
		if (red_relform_relevantvar(bvars, relname))
		{
			if (red_minisat_truevar(rsearch, solver, bvars, 1))
				newr = red_relform_addlit(newr, bvars, rel,1,c,k);
			if (red_minisat_truevar(rsearch, solver,bvars,-1))
				newr = red_relform_addlit(newr,bvars,rel,-1,c,k);
		}

	if (newr)
	{
		newr->next = relforms;
		newr->c=c;
		newr->arity=rel->arity;
		return newr;
	}

	else
	{
		newr = malloc(sizeof(struct red_relforms));
		newr->name = malloc(sizeof(char)*(strlen(rel->name)+1));
		sprintf(newr->name,"%s",rel->name);
		newr->c=0;
		newr->forms = NULL;
		newr->arity=rel->arity;
		newr->next = relforms;
		return newr;
	}
	/* unreachable */
}

/* returns 1 if bvars is a variable for the formula defining relname */
/* and 0 otherwise. */
int red_relform_relevantvar(struct red_bvarlist *bvars, const char *relname)
{
	char *varname = bvars->pos;
	int i, len=strlen(varname), lenr=strlen(relname);
	int j;

	/* varnames start with form%s_ */
	for (j=0,i=4; i<len && j<lenr; i++,j++)
	{
		if (varname[i]=='_')
		{
			break;
		}
		else if (varname[i]!=relname[j])
			return 0;
	}
	if (j!=lenr || varname[i]!='_')
		return 0;
	return 1;
}

/* returns 1 if bvar->pos [if sign==1] or bvar->neg [if sign==-1] is true
 * in the minisat model, and 0 otherwise 
 */
int red_minisat_truevar(struct redsearch *rsearch,
			struct minisat_solver_t *solver, 
			struct red_bvarlist *bvars, int sign)
{
	minisat_Lit lit;
	if (sign==1)
		lit=bvars->posLit;
	else if (sign==-1)
		lit=bvars->negLit;
	else
	{
		rsearch->abort=1;
		printf("r38: Invalid sign in red_minisat_truevar.\n");
		return 0;
	}

	return ((minisat_modelValue_Lit(solver, lit)==minisat_l_True));
}

/* add the bit from bvars->pos [if sign==1] or bvars->neg [if sign==-1] to the
 * relevant clause from rform.  If newr is NULL, create a new one.
 * Note that the length of the clauses may need to be increased.  If a new
 * newr is created, initialize all clauses (to NULL).
 */
struct red_relforms *red_relform_addlit(struct red_relforms *rform, 
				        struct red_bvarlist *bvars,
				        struct rel_symbol *rel, int sign, int c,
					int k)
{
	struct red_relforms *newr = rform;
	char *newlit;
	int clause = red_getclause(bvars);
	int i;
	
	if (!newr)
	{
		newr = malloc(sizeof(struct red_relforms));
		newr->c = c;
		newr->name = rel->name;
		newr->forms = malloc(sizeof(char *)*c);
		for (i=0; i<c; i++)
			newr->forms[i]=NULL;
	}

	newlit = red_relform_getlit(bvars,k); /* get the literal to add */
	newr->forms[clause-1]=red_relform_realaddlit(newlit, newr->forms[clause-1],
						   sign);

	return newr;
}

/* add the literal newlit (negated if sign==-1)
 * to oldclause, and return the new clause.
 * the clause is a conjunction
 * free oldclause if needed
 */
char *red_relform_realaddlit(char *newlit, char *oldclause, int sign)
{
	char *newclause;
	int len;

	if (!oldclause)
	{
		len = strlen(newlit)+1;
		if (sign==-1)
		{
			len++;
			newclause = malloc(sizeof(char)*len);
			sprintf(newclause,"~%s",newlit);
			return newclause;
		}
		else
		{
			newclause = malloc(sizeof(char)*len);
                        sprintf(newclause,"%s",newlit);
                        return newclause;
		}
	}
	len=strlen(newlit)+strlen(oldclause)+2;
	if (sign==-1)
		len++;
	newclause = malloc(sizeof(char)*len);
	
	if (sign==-1)
		sprintf(newclause,"%s&~%s",oldclause,newlit);
	else
		sprintf(newclause,"%s&%s",oldclause,newlit);
	free(oldclause);
	return newclause;
}

/* bv->pos is of the form form%s_clause%d_....
 * Return the %d.
 */
int red_getclause(struct red_bvarlist *bv)
{
	char *t=bv->pos;
	int i, len=strlen(t);
	for (i=0; i<len; i++)
	{
		if (t[i]=='_')
			break;
	}

	/* i is currently at the '_' */
	i+=7; /* now at the start of the number */
	return atoi(t+i);
}

/* return the atomic formula (in DE syntax) in the variable bv->pos.
 * bv->pos is "form%s_clause%d_LIT" but LIT needs to be converted.
 */
char *red_relform_getlit(struct red_bvarlist *bv, int k)
{
	int i, j=0, len;
	char *lit=bv->pos;
	char *ret;
	len=strlen(lit);
	for (i=0; i<len && j<2; i++)
	{
		if (lit[i]=='_')
			j++;
	}
	lit+=i; /* now, the literal starts at lit */

	/* T */
	if (lit[0]=='T' && lit[1]=='\0')
	{
		ret = malloc(sizeof(char)*3);
		sprintf(ret,"\\t");
		return ret;
	}

	/* eq */
	if (lit[0]=='e' && lit[1]=='q')
		return red_getlit_eq(lit, k);

	else /* add a sanity check */
		return red_getlit_pred(lit, k);	
}

/* lit is the literal part of a Boolean variable (i.e., it is the LIT part of
 * form%s_clause%d_LIT.  It is a predicate literal (e.g., E[x-1-2,s]) or
 * something.
 * Return the DE version of it (converting variable names!)
 */
char *red_getlit_pred(char *lit, int k)
{
	int i,j,len=strlen(lit);
	char *ret=malloc(sizeof(char)*len);
	int first=1;

	for (i=0; i<len; i++)
	{
		if (lit[i]=='[')
			break;
		ret[i]=lit[i];
	}
	ret[i]='(';
	ret[i+1]='\0';

	i++; 
	
	do {
		if (!first)
			ret = red_pred_concat(ret,',');
		else
			first=0;
		for (j=i+1; j<len; j++)
		{
			if (lit[j]=='.')
			{
				lit[j]='\0';
				ret=red_pred_concatterm(ret, lit+i, k);
				lit[j]='.';
				i=j+1;
				break;
			}
			else if (lit[j]==']')
			{
				lit[j]='\0';    
                                ret=red_pred_concatterm(ret, lit+i, k);
                                lit[j]=']';
                                i=j+1;
                                break;
                        }
		}
		if (j<len && lit[j]==']')
			break;
	} while (i<len);

	ret = red_pred_concat(ret,')');

	return ret;
}

/* add this term (after converting it to DE syntax if needed */
char *red_pred_concatterm(char *old, char *lit, int k)
{
	char *newterm=red_getterm(lit,k);
	char *tmp = malloc(sizeof(char)*(strlen(old)+strlen(newterm)+1));
	sprintf(tmp,"%s%s",old,newterm);
	free(old);
	free(newterm);
	return tmp;
}

/* add that char and a \0 */
char *red_pred_concat(char *old, char newchar)
{
	int len = strlen(old);
	char *tmp = realloc(old, sizeof(char)*(len+2));
	if (!tmp)
	{
		printf("r39: Unable to realloc an extra byte.\n");
		return NULL;
	}
	
	tmp[len]=newchar;
	tmp[len+1]='\0';
	return tmp;
}

char *red_getlit_eq(const char *lit, int k)
{
	char *left, *right;
	char *tmp;
	int i, len=strlen(lit);
	int succ = 0;

	tmp = malloc(sizeof(char)*len);

	strcpy(tmp,lit+3);
	for (i=0; i<len; i++)
	{
		if (tmp[i]=='.')
		{
			tmp[i]='\0';
			break;
		}
	}

	if (tmp[i+1]=='+')
	{
		succ = 1;
		i++;
	}

	left = red_getterm(tmp,k);
	right = red_getterm(tmp+i+1,k);
	free(tmp);

	tmp = malloc(sizeof(char)*(strlen(left)+1+strlen(right)+1+(succ<<1)));
	if (!succ)
		sprintf(tmp,"%s=%s",left,right);
	else
		sprintf(tmp,"%s=%s+1",left,right);
	free(left);
	free(right);

	return tmp;
}

char *red_getterm(char *lit, int k)
{
	char *tmp;
	int i,j;
	int len;
	char c;

	if (isdigit(lit[0]))
	{
		i = atoi(lit);
		tmp = malloc(sizeof(char)*(1+numdigits(i)));
		sprintf(tmp,"%d",i);
		return tmp;
	}
	if (lit[0]!='x') /* all our variables are x-vk-va */
	{
		len=strlen(lit);
		tmp = malloc(sizeof(char)*(len+1));
		strcpy(tmp,lit);
		c = tmp[len-1];
		if (c==']')
			tmp[len-1]='\0';
		return tmp;
	}
	/* else we're a variable */
	sscanf(lit,"x-%d-%d",&i,&j);

	i = i + (j-1)*k;

	tmp = malloc(sizeof(char)*(2+numdigits(i)));
	sprintf(tmp,"x%d",i);
	return tmp;
}

/* return a bvarlist of the Boolean variables for constants.
 * These are of the form "cons_%s_[A,B,...]" where the %s is the constant name
 * and [A,B,...]\in {0,1,...,n-1,p1_cons}^k.
 * The neg parts of the bvarlist are all NULL.
 */
struct red_bvarlist *red_makecbvars(struct redsearch *rsearch,
				    const struct bquery *p1,
			 	    const struct bquery *p2, int k, int n2)
{
	struct minisat_solver_t *solver = rsearch->solver;
	struct env *hash = rsearch->hash;
	struct cons_symbol *cons;
	struct cons_symbol *p1cons = p1->voc->cons_symbols;
	struct red_tuple *tuple=NULL;
	struct red_bvarlist *cbvar=NULL;
	int n=n2;

	for (cons=p2->voc->cons_symbols; cons; cons=cons->next)
		while ((tuple=red_nextconstuple(rsearch,tuple, k, n, p1cons)))
			cbvar=red_addconsbvar(rsearch, cbvar,solver,hash,
					      cons->name, tuple);

	return cbvar;
}

/* modify tuple to be the next arity-ary tuple in some fixed enumeration of
 * [A1.A2.....Aa_r]
 * where each Aj is either a natural 0<=Aj<=n-1 or a constant from p1cons.
 *
 * If tuple is NULL, make it the first tuple.  If tuple is the last tuple,
 * free it and return NULL.
 */
struct red_tuple *red_nextconstuple(struct redsearch *rsearch, 
				    struct red_tuple *tuple, int k, int n, 
				    struct cons_symbol *p1cons)
{
	struct red_tuple *tup;
        struct red_tuple_element *el;
	struct cons_symbol *ct;
        int i;

	if (!tuple)
	{
		tup = malloc(sizeof(struct red_tuple));
                if (!tup)
                {
			rsearch->abort=1;
                        printf("r40: No memory\n");
                        return 0;
                }
		for (i=0,ct=p1cons;ct;i++,ct=ct->next);
                if (i)
                {
                        tup->cons_names=malloc(sizeof(char *)*i);
                        if (!tup->cons_names)
                        {
				rsearch->abort=1;
                                printf("r41: No memory\n");
                                free(tup);
                                return 0;
                        }
                }
                else
                        tup->cons_names=0;
		el=malloc(sizeof(struct red_tuple_element)*k);
                if (!el)
                {
			rsearch->abort=1;
                        printf("r42: No memory\n");
                        if (tup->cons_names)
                                free(tup->cons_names);
                        free(tup);
                        return 0;
                }
                tup->data=el;
                for (i=0,ct=p1cons;ct;i++,ct=ct->next)
                        tup->cons_names[i]=ct->name;
                tup->num_cons=i;
                tup->arity=k;
                for (i=0; i<k; i++) /* 0 guaranteed to exist */
                {
                        el[i].type=2;
                        el[i].a=el[i].b=0;
                }
		return tup;
        }

	el = tuple->data;
        /* otherwise, tuple is a tuple and we need to get the next one. */
        for (i=k-1; i>=0; i--) /* find a spot to increment */
        {
                if ((el[i].type==1 && el[i].a==tuple->num_cons-1) ||
                    (tuple->num_cons==0 && el[i].type==2 && el[i].a==n-1))
                {
                        el[i].type=2;
                        el[i].a=el[i].b=0;
                        continue;
                }
                else
                        break;
        }

	if (i==-1)
        {
                free(tuple->data);
                free(tuple->cons_names);
                free(tuple);
                return NULL;
        }

        /* otherwise we found a spot to increment */
	if (el[i].type == 1)
        {
                el[i].a++;
                return tuple;
        }

	if (el[i].a==n-1)
	{
		el[i].type = 1;
		el[i].a=0;
		return tuple;
	}

	el[i].a++;
	return tuple;
}


/* add cons_%s_[...] */ 
struct red_bvarlist *red_addconsbvar(struct redsearch *rsearch,
				     struct red_bvarlist *cbvars, 
				     struct minisat_solver_t *solver, 
				     struct env *hash, char *consname,
                                     struct red_tuple *tuple)
{
	char *res;
	struct red_bvarlist *list=malloc(sizeof(struct red_bvarlist));
	int i, arity=tuple->arity;
	int n1=rsearch->n1;

	res = red_getconsbvarname(consname, tuple);

	list->pos=res;
	list->neg=NULL;
	list->posVar=minisat_newVar(solver);
	minisat_setFrozen(solver, list->posVar, minisat_l_True);
	list->posLit=minisat_mkLit(list->posVar);
	list->negVar=0;
	list->negLit=0;
	list->next=cbvars;

	hash_alloc_insert(hash->id_hash, res, list);

	/* disable cons assignments to too-large values */
	for (i=0; i<arity; i++)
	{
		if ((tuple->data+i)->a>=n1)
		{
			minisat_addClause_begin(solver);
                 	minisat_addClause_addLit(solver, 
						 minisat_negate(list->posLit));
                 	minisat_addClause_commit(solver);
		}
	}
	return list;
}

/* return the defining formula for constant cons in DE syntax, using the
 * cbvars in rsearch.
 */
struct list *red_addconsform(struct list *cf, struct cons_symbol *cons,
			     struct redsearch *rsearch)
{
	struct red_bvarlist *bv;
	struct list *list=malloc(sizeof(struct list));
	int k=rsearch->k;
	struct minisat_solver_t *solver = rsearch->solver;
	minisat_Lit lit;

	for (bv=rsearch->cbvars; bv; bv=bv->next)
	{
		lit=bv->posLit;
		if (red_cf_relevant(cons, bv) && 
		    (minisat_modelValue_Lit(solver,lit)==minisat_l_True))
		{
			list->next = cf;
			list->data = red_cf_getconsform(cons,bv,k);
			return list;
		}
	}

	/* uh oh */
	rsearch->abort=1;
	printf("r43: No definition found for %s.\n",cons->name);
	return cf;
}

/* returns 1 if this variable is for this constant, otherwise 0 */
int red_cf_relevant(struct cons_symbol *cons, struct red_bvarlist *bv)
{
	int i,j, tlen, clen;
	char *tmp=bv->pos;
	char *cname=cons->name;
	tlen = strlen(tmp);
	clen = strlen(cname);

	for (i=5,j=0; i<tlen && j<clen; i++,j++)
		if (tmp[i]!=cname[j])
			return 0;
	if (j==clen && i<tlen && tmp[i]=='_')
		return 1;
	return 0;
}

/* returns the DE-formula defining cons->name corresponding to bv */
/* For example "cons->name is (x1=0 & x2=2...&xk=0)" */
char *red_cf_getconsform(struct cons_symbol *cons, struct red_bvarlist *bv, 
			 int k)
{
	char *res, *cname=cons->name;
	char *tmp;
	int len, i, j;
	char *tup=bv->pos;
	int bl = strlen(tup);
	
	len=strlen(cname)+4+2+(k-1)+1+(k<<1); 
		/*"cname is (" and ")\0", all '&', all 'x' and all '=' */

	/* the 1,...,k for x1,...,xk */
	for (i=1; i<=k; i++)
		len+=numdigits(i);

	for (i=0,j=0; i<bl; i++)
		if (tup[i]=='_')
			if (++j>=2)
				break;
	tup=tup+i+1; /* tup now starts at the LIT of cons_c_LIT */

	len += red_cf_vallen(tup,k);

	res = malloc(sizeof(char)*len);
	sprintf(res,"%s is (",cname);

	tmp=red_cf_gettup(tup,k,len);
	strcat(res,tmp);
	free(tmp);

	strcat(res,")");

	return res;
}

/* tup is [A.B.C...] - return the length needed to display the components
 * (no need for connectives, etc.
 * A,B,C... are constants or naturals, not variables 
 */
/* note that they are already strings */
int red_cf_vallen(char *tup, int k)
{
	int len=strlen(tup);
	return (len-(k-1)-2); /* no "[]" and no '.' (k-1 of those) */
}

/* tup is [A.B.C...] - return "x1=A&x2=B&x3=C&x4=D...".
 * k is k and len is an upperbound on the length of this string.
 */
char *red_cf_gettup(char *tup, int k, int len)
{
	char *res = malloc(sizeof(char)*len);
	char *tmp = malloc(sizeof(char)*len);
	int i,j=1,num;
	int tlen = strlen(tup);
	char old;
	res[0]='\0';

	for (num=1,i=1; i<tlen; i++)
	{
		if (tup[i]=='.'||tup[i]==']')
		{
			old = tup[i];
			tup[i]='\0';
			if (num!=1)
				sprintf(tmp,"&x%d=%s",num,tup+j);
			else
				sprintf(tmp,"x%d=%s",num,tup+j);
			j=i+1;
			tup[i]=old;
			strcat(res,tmp);
			if (num++>k)
			{
#ifdef DEBUG
				printf("r44: Broken tuple: %s\n",tup);
#endif
				break;
			}
		}
	}
	free(tmp);
	return res;
}

/* return the formula in rf (i.e., a conjunction of its clauses) */
char *red_rftodef(struct red_relforms *rf)
{
	char *res;
	char *tmp;
	int len;
	int i;
	int c=rf->c;
	char **forms = rf->forms;

	len = (c-1) + (c<<1)+1; /* '&' and '(',')' and '\0' */
	for (i=0; i<c; i++)
		len+=strlen(forms[i]);

	res = malloc(sizeof(char)*len);
	tmp = malloc(sizeof(char)*len);

	res[0]='\0';
	for (i=0; i<c; i++)
	{
		sprintf(tmp,"(%s)",forms[i]);
		if (i!=0)
			strcat(res,"|");
		strcat(res,tmp);
	}

	free(tmp);
	return res;
}

/* free the relforms and consforms, but not hyp->rsearch */
int red_freehyp(struct red_hypot *hyp)
{
	struct red_relforms *rf, *rfn;
	struct list *cf, *cfn;
	int i,c;

	for (cf=hyp->consforms; cf; cf=cfn)
	{
		cfn=cf->next;
		free(cf->data);
		free(cf);
	}

	for (rf=hyp->relforms; rf; rf=rfn)
	{
		rfn=rf->next;
		/* don't free rf->name, it's used elsewhere */
		c=rf->c;
		for (i=0; i<c; i++)
			free(rf->forms[i]);
		free(rf);
	}

	free(hyp);
	return 1;
}

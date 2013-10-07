// ISC LICENSE
/* eval.c
 * Skip Jordan
 * Evaluates SOE formulas.  At the moment, only propositional. :P
 *
 * chj	10/23/06	created
 * chj  11/17/06	evaluate quantifiers
 * chj  11/23/06	evaluate transitive closure, cache relations
 * chj	12/03/06	allow arithmetic in terms to be full 32bits
 * chj  12/11/06        eval_exists loop trimming and bugfix
 * chj	12/4/11		xor
 * chj	 7/11/12	faster interpretations by cheating
 * chj	 7/13/12	faster teval and eval by avoiding last recursion
 */

#include "parse.h"
#include "protos.h"
#include <stdlib.h>
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

/* setlr is an optimization for teval and eval_rec, we try to avoid recursing
 * since many terms are simple
 */
#define setlr() \
  do {\
    forml = form->l; \
    formr = form->r; \
    if (forml->label==VAR || forml->label==CONSTANT) \
      l = *(forml->ival);\
    else \
      l = teval(forml,interp,struc);\
    if (formr->label==VAR || formr->label==CONSTANT)\
      r = *(formr->ival);\
    else \
      r = teval(formr,interp,struc); \
  } while(0)

/* Node *parse(char *formula)
 *
 * Returns the root of an AST representing formula.
 * Returns null if this is not possible.
 */

/* 
   Node *parse(char *formula)
   {
   return NULL;

   }
   */

/* initiate interp to have one element for each variable and constant symbol
 * that we need to interpret in form.  For each node, set a pointer to the 
 * value in the interpretation.
 * Then we just look up the value directly, without finding the symbol in the
 * interpretation.
 * This changes acceptable usage of interpretations.
 * The external API is generally to use eval(), which does everything normally.
 * However, eval_init_form is fairly slow, and eval_rec() is much faster than
 * eval().  If the user wants to evaluate the same formula repeatedly, changing
 * values in the interpretation each time, the following must be done:
 * 1) On the first call, use eval() and don't have any symbol names correspond
 *    to multiple entries in the interpretation.
 * 2) On subsequent calls, update the values (interp->symbols->value)
 *    in the interpretation but do not add/remove/change anything else in
 *    the interpretation.  Then, use eval_rec() instead of eval().
 *
 * Note that "max" is handled specially -- it's added to the interpretation
 * with the value of the given structure.  So, if you change the structure,
 * re-use the interpretation and call eval_rec(), it will use the old value
 * of max unless you change it in the interpretation.
 *
 * Note that a formula is essentially "reserved for use" by eval_init_form -
 * calling eval() on it with a different interpretation/structure will remove
 * the fast interpretation, making future calls to eval_rec() use the new
 * interpretation (so be careful with parallel/interleaving calls to eval_rec).
 */
void eval_init_form(Node *form, Interp *interp, const Structure *struc) {
  InterpSymbol *is;
  char *name;

  switch (form->label)
  {
    case TRUE:
    case FALSE:
    case NUMBER:
      return;
    case LT:
    case LTE:
    case NEQUALS:
    case EQUALS:
    case AND:
    case OR:
    case IMPLIES:
    case IFF:
    case XOR:
    case MULT:
    case PLUS:
    case MINUS:
      eval_init_form(form->r, interp, struc);
    case NOT:
      eval_init_form(form->l, interp, struc);
      return;
    case CONSTANT:
    case VAR:
      name = (char *)form->data;
      for (is=interp->symbols; is; is=is->next)
        if (!strcmp(is->name,name))
        {
          form->ival=&(is->value);
          return;
        }
      is = (InterpSymbol*) malloc(sizeof(InterpSymbol));
      is->name=dupstr(name);
      is->value=-2;
      if (!strcmp("max",name))
        is->value = struc->size-1;
      form->ival=&(is->value);
      is->next = interp->symbols;
      interp->symbols = is;
      return;
    case EXISTS:
    case FORALL:
      eval_init_form_q(form,interp,struc);
      return;
    case PRED:
      eval_init_form_pred(form,interp,struc);
      return;
    case TC:
      eval_init_form_tc(form,interp,struc);
      return;
    case SOE:
      eval_init_form_soe(form,interp,struc);
      return;
    case IFP:
    default:
      printf("61: Unknown node preparing fast interp\n");
      exit(0);
  }
}

/* form is \forall or \exists, handle it */
void eval_init_form_q(Node *form, Interp *interp, const Structure *struc) {
  Node *varlist = form->l->l;
  Node *restr = form->l->r;
  Node *phi = form->r;
  Node *tnode;
  char *name;

  InterpSymbol *is;

  for (tnode=varlist; tnode; tnode=tnode->r)
  {
    name = (char*) tnode->data;
    for (is=interp->symbols; is; is=is->next)
      if (!strcmp(is->name,name))
      {
        tnode->ival=&(is->value);
        break;
      }
    if (is)
      continue;
    is = (InterpSymbol*) malloc(sizeof(InterpSymbol));
    is->name=dupstr(name);
    is->value=-2;
    tnode->ival=&(is->value);
    is->next = interp->symbols;
    interp->symbols = is;
  }

  if (restr)
    eval_init_form(restr, interp, struc);
  eval_init_form(phi, interp, struc);

  return;
}

void eval_init_form_tc(Node *form, Interp *interp, const Structure *struc) {
  Node *tcargs = form->l->l;
  Node *tcform = form->l->r;
  Node *relargs = form->r;
  Node *tmp;

  for (tmp=tcargs; tmp; tmp=tmp->r)
  {
    eval_init_form(tmp->l->l, interp, struc);
    if (tmp->l->r)
      eval_init_form(tmp->l->r, interp, struc);
  }
  eval_init_form(tcform, interp, struc);
  for (tmp=relargs; tmp; tmp=tmp->r)
    eval_init_form(tmp->l, interp, struc);
  return;
}

void eval_init_form_pred(Node *form, Interp *interp, const Structure *struc) {
  Node *relargs=form->r;
  Node *tnode;
  Relation *rel;
  int i, a;

  for (tnode=relargs; tnode; tnode=tnode->r)
    eval_init_form(tnode->l, interp, struc);

  rel = get_relation((const char*) form->data, interp, struc);
  if (!rel) /* SO variable */
    return;

  if (rel->parse_cache)
    eval_init_form(rel->parse_cache, interp, struc);

  a=rel->arity;
  for (i=1; i<=a; i++)
    if (get_xi_interp_value(i,interp)==-1)
      add_xi_interp(i,interp,-2);
  return;
}

void eval_init_form_soe(Node *form, Interp *interp, const Structure *struc) {
  Node *restr=form->l->r;
  Node *phi = form->r;

  if (restr)
    eval_init_form(restr, interp, struc);
  eval_init_form(phi, interp, struc);
  return;
}

/* int eval(Node *form)
 * 
 * Evaluates the formula form.
 * Returns 1 iff true, 0 iff false.
 *
 * External interface to the evaluator (sets up a fast interpretation and
 * calls eval_rec)
 */

int eval(Node *form, Interp *interp, const Structure *struc) {
  char *fv;
  eval_init_form(form, interp, struc);
  fv = free_var_fast(form,interp,struc);
  if (fv)
  {
    printf("??: Symbol %s occurs free in formula.\n",fv);
    return 0;
  }
  return eval_rec(form, interp, struc);
}

int eval_rec(Node *form, Interp *interp, const Structure *struc) {
  int l,r;
  Node *forml, *formr;
#if 0 /* this really should never happen */
  if (!form)
  {
    return 0;
  }
#endif
  switch (form->label)
  {
    case TRUE:
      return 1;
    case FALSE:
      return 0;
    case LT:
      setlr();
      return l<r;
    case LTE:
      setlr();
      return l<=r;
    case NEQUALS:
      setlr();
      return l!=r;
    case EQUALS:
      setlr();
      return l==r;

    case NOT:
      return !eval_rec(form->l, interp, struc);
    case AND:
      return eval_rec(form->l, interp, struc) && eval_rec(form->r, interp, struc);
    case OR:
      return eval_rec(form->l, interp, struc) || eval_rec(form->r, interp, struc);

    case IMPLIES:
      /* a->b <-> ~a | b */
      return ((!eval_rec(form->l, interp, struc))||(eval_rec(form->r, interp, struc)));
    case IFF:
      /* a<->b is ~(a xor b) */
      return !(eval_rec(form->l, interp, struc) ^ eval_rec(form->r, interp, struc));
    case XOR:
      return eval_rec(form->l,interp,struc) ^ eval_rec(form->r,interp,struc);
    case PRED:
      return eval_pred(form,interp,struc);

    case EXISTS:
      return eval_exists(form,interp,struc);

    case FORALL:
      return eval_forall(form,interp,struc);
    case TC:
      return eval_tc(form,interp,struc);
    case IFP:
      return eval_ifp(form,interp,struc);
    case SOE:
      return eval_soe(form, interp, struc);

    default:
      printf("5: Unknown logical node (%d)\n",form->label);
      if (form->l)
        return eval_rec(form->l, interp, struc);
      if (form->r)
        return eval_rec(form->r, interp, struc);
      return -1;
  }
}

/* Evaluate transitive closure with Warshall's Algorithm *
 * Note that this is reflexive.
 */
int eval_tc(Node *form, Interp *interp, const Structure *struc) {
  int *tc_cache;
  int num_args;
  int tup_arity;
  int size;
  int csize;
  int *tup1=NULL;
  int *tup2=NULL;
  int res;
  Node *tmp;
  Node *tcargs;
  Node *tcform;
  Node *relargs;
  char **t1names;
  char **t2names;
  int i, j, ii;
  int num_tuples;
  int *old_values1, *old_values2;
  int **values1, **values2;

  size = struc->size;
  tcargs = form->l->l;
  tcform = form->l->r;
  relargs = form->r;

  for (tup_arity=0,tmp=tcargs; tmp; tup_arity++)
    tmp=tmp->r;
  for (i=0, tmp=relargs; tmp; i++)
    tmp = tmp->r;
  num_args = tup_arity<<1; /* parser does it with qnode */
  if (i!=num_args)
  {
    printf("14:TC of arity %d given %d arguments\n",num_args,i);
    return -1;
  }
  num_tuples = de_pow(size,tup_arity);

  if (form->data) /* check for a cache */
  {
    tup1 = next_tuple(NULL,tup_arity, size);
    tup2 = next_tuple(NULL,tup_arity, size);
    tmp = relargs;

    for (i=0; i<tup_arity; i++,tmp=tmp->r)
      tup1[i] = teval(tmp->l,interp,struc);
    for (i=0; i<tup_arity; i++,tmp=tmp->r)
      tup2[i] = teval(tmp->l,interp,struc);

    i = tuple_cindex(tup1, tup_arity, size);
    j = tuple_cindex(tup2, tup_arity, size);

    res = ((int *)(form->data))[i*num_tuples+j];

    free(tup1);
    free(tup2);

    if (res!=-1)
      return res;
  }

  csize = num_tuples * num_tuples;
  tc_cache =(int*) malloc(csize * sizeof(int));
  t1names = (char**) malloc(tup_arity * sizeof(char **));
  t2names = (char**) malloc(tup_arity * sizeof(char **));
  old_values1 = (int*) malloc(tup_arity * sizeof(int));
  old_values2 = (int*) malloc(tup_arity * sizeof(int));
  values1 = (int**) malloc(tup_arity * sizeof(int **));
  values2 = (int**) malloc(tup_arity * sizeof(int **));
  /* TODO check all those pointers :P */
  tmp = tcargs;
  for (i=0; ;tmp=tmp->r)
  {
    values1[i] = tmp->l->l->ival;
    old_values1[i]=*(values1[i]);
    t1names[i] = (char*) tmp->l->l->data;
    if (++i<tup_arity)
    {
      t1names[i] = (char*) tmp->l->r->data;
      values1[i] = tmp->l->r->ival;
      old_values1[i]=*(values1[i]);
      i++; /* chj new CHECK */
    }
    else
      break;
  }
  if (tup_arity&1) /* odd arity means we split one */
  {
    t2names[0]=(char*) tmp->l->r->data;
    values2[0]=tmp->l->r->ival;
    old_values2[0]=*(values2[0]);
    i=1;
  }
  else
    i=0;
  for (tmp=tmp->r; ;tmp=tmp->r)
  {
    if (!tmp)
      break;
    values2[i] = tmp->l->l->ival;
    old_values2[i]=*(values2[i]);
    t2names[i] = (char*) tmp->l->l->data;
    if (++i<tup_arity)
    {
      values2[i] = tmp->l->r->ival;
      old_values2[i] = *(values2[i]);
      t2names[i] = (char*) tmp->l->r->data;
      i++;
    }
    else
      break;
  }

  /* okay, now we initialize the cache */
  /* this is a bit scary. */
  /* tup_arity is usually very small, so
   * the for loops should be very short */
  i=j=-1;
  while ((tup1=next_tuple(tup1, tup_arity, size)))
  {
    i++;
    for (ii=0; ii<tup_arity; ii++)
      *(values1[ii])=tup1[ii];
    j=-1;
    while ((tup2=next_tuple(tup2, tup_arity, size)))
    {
      j++;
      if (i==j) /* reflexive! */
      {
        tc_cache[i*num_tuples+j]=1;
        continue;
      }
      for (ii=0; ii<tup_arity; ii++)
        *(values2[ii]) = tup2[ii];
      res = eval_rec(tcform, interp, struc);
      tc_cache[i*num_tuples+j] = res;
    }
  }
  for (ii=0; ii<tup_arity; ii++)
  {
    *(values1[ii])=old_values1[ii];
    *(values2[ii])=old_values2[ii];
  }

  /* wow, look at that.  hope that graph is sparse. */
#if 0	
  for (i=0; i<num_tuples; i++)
    for (j=0; j<num_tuples; j++)
#endif
      for (j=0; j<num_tuples; j++)
        for (i=0; i<num_tuples; i++)
          if (tc_cache[i*num_tuples+j]==1)
            for (ii=0; ii<num_tuples; ii++)
              if (tc_cache[j*num_tuples+ii]==1)
                tc_cache[i*num_tuples+ii] = 1;

  tup1 = next_tuple(NULL,tup_arity, size);
  tup2 = next_tuple(NULL,tup_arity, size);
  tmp = relargs;

  for (i=0; i<tup_arity; i++,tmp=tmp->r)
    tup1[i] = teval(tmp->l,interp,struc);
  for (i=0; i<tup_arity; i++,tmp=tmp->r)
    tup2[i] = teval(tmp->l,interp,struc);

  i = tuple_cindex(tup1, tup_arity, size);
  j = tuple_cindex(tup2, tup_arity, size);

  res = tc_cache[i*num_tuples+j];

  if (form->data)
    free(form->data);
  form->data = tc_cache;

  free(tup1);
  free(tup2);
  free(t1names);
  free(t2names);
  free(old_values1);
  free(old_values2);
  free(values1);
  free(values2);

  return res;
}

/* Nonsense body to remove warnings of unused parameters */
/* Of course, implement IFP later */
int eval_ifp(Node *form, Interp *interp, const Structure *struc) {
  if (form && interp && struc)
    return 0;
  return 1;
}

/* We begin by guessing 0-, then 0-...1, then 0-...10, etc.
 * Yay for exponential time.
 */
int eval_soe(Node *form, Interp *interp, const Structure *struc) {
  char *varname=(char*) form->l->l->l->l->data;
  int res=0;
  int size = struc->size;
  int arity=*(int *)(form->l->l->l->r->data);
  int *cache;
  int i;

  Node *restr=form->l->r;
  Node *phi = form->r;
  Relation *sov;

  int tc_size=de_pow(size, arity);
  cache = (int*) malloc(tc_size * sizeof(int));
  for (i=0; i<tc_size; i++) 
    cache[i]=0;
  sov = (Relation*) malloc(sizeof(Relation));
  sov->name = varname;
  sov->arity = arity;
  sov->next = interp->rel_symbols;;
  sov->cache = cache;
  sov->parse_cache = 0;
  interp->rel_symbols=sov;
  if (restr)		
    do {
      if ((res=(eval_rec(restr,interp,struc))))
        res = eval_rec(phi, interp, struc);
      for (i=tc_size-1; i>=0; i--)
      {
        if ((cache[i]=cache[i]^1))
          break;
      }
    } while (!res && !(i==-1 && cache[i+1]==0));
  else
    do {
      res = eval_rec(phi, interp, struc);
      for (i=tc_size-1; i>=0; i--)
      {
        if ((cache[i]=cache[i]^1))
          break;
      }
    } while (!res && !(i==-1 && cache[i+1]==0));
  free(cache);
  interp->rel_symbols = sov->next;
  free(sov);
  return res;
}

int eval_forall(Node *form, Interp *interp, const Structure *struc) {
  Node *nnot = node(NOT, form->r, 0);
  int res=0;
  if (!nnot)
    return -1;
  form->r=nnot;
  form->label = EXISTS;

  res = !eval_exists(form,interp,struc);

  form->r = nnot->l;
  form->label = FORALL;
  free(nnot);

  return res;
}

int eval_exists(Node *form, Interp *interp, const Structure *struc) {
  char **varnames;
  int size=struc->size;
  int arity=0;
  int i;
  int res=0;

  Node *restr = form->l->r;
  Node *varlist = form->l->l;
  Node *phi = form->r;
  Node *tnode=varlist;
  int *first;
  int *old_values;
  int **values;

  while (tnode)
  {
    arity++;
    tnode = tnode->r;
  }

  old_values = (int*) malloc(arity*(sizeof(int)+sizeof(int *)+sizeof(char *)));
  values = (int **)(old_values+arity);
  varnames = (char **)(values+arity);

  tnode = varlist;

  for (i=0; i<arity; i++)
  {
    varnames[i]=(char*) tnode->data;
    values[i]=tnode->ival;
    old_values[i]=*(values[i]);
    *(values[i])=0;
    tnode = tnode->r;
  }

  first = values[0];
  *first = -1;

  if (restr)
    while (1)
    {
      (*first)++;
      if (*first>=size)
      {
        *first=0;
        for (i=1; i<arity; i++)
        {
          res = ++(*values[i]);
          if (res < size)
            break;
          res=(*values[i]) = 0;
        }
        if (!res && i==arity)
          break;
      }

      res = eval_rec(restr, interp, struc);
      if (res)
        res = eval_rec(phi,interp,struc);

      /* TODO res==-1 error catching */

      if (res) /* found one */
      {
        for (i=0; i<arity; i++)
          *(values[i])=old_values[i];
        /* free(vartuple); */
        free(old_values);
        /* free(values); */
        /* free(varnames); */
        return 1;
      }
    }
  else /* !restr */
    while (1)
    {
      (*first)++;
      if (*first>=size)
      {
        *first=0;
        for (i=1; i<arity; i++)
        {
          res = ++(*values[i]);
          if (res < size)
            break;
          res=(*values[i]) = 0;
        }
        if (!res && i==arity)
          break;
      }

      res = eval_rec(phi,interp,struc);

      /* TODO res==-1 error catching */

      if (res) /* found one */
      {
        for (i=0; i<arity; i++)
          *(values[i])=old_values[i];
        /* free(values); */
        free(old_values);
        /* free(varnames); */
        return 1;
      }
    }


  for (i=0; i<arity; i++)
    *(values[i])=old_values[i];
  /* free(values); */
  free(old_values);
  /* free(varnames); */
  return 0;
}

int eval_pred(Node *form, Interp *interp, const Structure *struc) {
  Relation *rel;
  int arity, i, res;
  int *tup;
  int size;
  int num;
  Node *relargs;
  int *old_values;	
  int **values;

  /* get the relation definition, in struc if a real predicate,
   * in interp if it's a local variable that's part of a fixed-point,
   * or so formula.
   */
  rel = get_relation((char*) form->data, interp, struc);
  if (!rel)
  {
    printf("4:Relation symbol %s is undefined\n",(char *)form->l->data);
    return -1;
  }
  arity = rel->arity;
  tup = (int*) malloc(arity * sizeof(int));
  relargs = form->r;
  size = struc->size;

  for (i=0; relargs && i<arity; i++)
  {
    tup[i] = teval(relargs->l, interp, struc);
    if (tup[i]>=struc->size || tup[i]<0)
    {
#ifdef DEBUG
      printf("d: %dth argument to %s out of range\n",i,rel->name);
#endif
      free(tup);
      return 0; /* out of range means false      */
      /* TODO should still check arity */
    }
    relargs = relargs->r;
  }
  if (relargs || i!=arity)
  {
    printf("6: Relation symbol %s used with incorrect arity\n",rel->name);
    free(tup);
    return -1;
  }

  if (rel->cache)
  {
    num = tuple_cindex(tup, arity, size);
    if (rel->cache[num]>=0)
    {
      free(tup);
      return rel->cache[num];
    }
  }

  old_values = (int*) malloc((sizeof(int)+sizeof(int *))*arity);
  /* instead of two small mallocs, cast for clarity */
  values = (int **)(old_values+arity);

  for (i=0; i<arity; i++)
  {
    values[i] = get_xi_ival(i+1,interp);
    old_values[i] = *(values[i]);
    *(values[i]) = tup[i];
  }

  res = eval_rec(rel->parse_cache, interp, struc);

  for (i=0; i<arity; i++)
    *(values[i]) = old_values[i];

  if (rel->cache)
  {
    num = tuple_cindex(tup, arity, size);
    rel->cache[num]=res;
  }
  free(tup);
  free(old_values);
  return res;
}

int teval(Node *form, Interp *interp, const Structure *struc) {
  int l, r;
  Node *forml, *formr;
#if 0 /* this really should never happen */
  char *name;
  int value;
  if (!form)
    return -1;
#endif
  switch (form->label)
  {
    case CONSTANT:
    case VAR:
      return *(form->ival);
#if 0 /* none of that should ever happen */
      name = (char *)form->data;
      /* max, but NULL struc may be used in redfind.
       * we'll add it to the interpretation in that case.
       */
      if (form->ival)
        return *(form->ival);
      if (!strcmp(name,"max") && struc)
        return struc->size-1;
      value = get_interp_value(name,interp);
      if (value<0)
      {
        printf("9:Symbol %s not in interpretation.\n",name);
        return -1;
      }
      return value;
#endif
    case NUMBER:
      return form->ndata;

    case MULT:
      setlr();
      return l*r;
    case PLUS:
      setlr();
      return l+r;
    case MINUS:
      setlr();
      return l-r;
    default:
      return -1;
  }

}

/* free the TC caches in form */
void free_tc_caches(Node *form) {
  switch (form->label)
  {
    case NOT:
      free_tc_caches(form->l);
      return;
    case AND:
    case OR:
    case IMPLIES:
    case IFF:
    case XOR:
      free_tc_caches(form->l);
      free_tc_caches(form->r);
      return;
    case EXISTS:
    case FORALL:
    case SOE:
      if (form->l->r)
        free_tc_caches(form->l->r);
      free_tc_caches(form->r);
      return;
    case TC:
      free(form->data);
      form->data = 0;
      return;
    case TRUE:
    case FALSE:
    case LT:
    case LTE:
    case NEQUALS:
    case EQUALS:
    case PRED:
    case IFP:
    default:
      return;
  }
}



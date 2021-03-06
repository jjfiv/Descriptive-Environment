// ISC LICENSE
/* check.c
 * Various functions to check if formulas are okay.
 */

#include "protos.h"
#include "types.h"
#include "parse.h"
#include <string.h>
#include <stdlib.h>

/* Returns a pointer to a STATIC (don't free) copy of a free-variable
 * in form (when constants from vocab don't count), or NULL if none exist.
 */
char *free_var(Node *form, Vocabulary *vocab) {
  List *list=d_free_var(form,vocab), *tmp;
  char *name;
  if (!list)
    return NULL;
  name=(char*) list->data;
  while (list)
  {
    tmp=list->next;
    free(list);
    list=tmp;
  }
  return name;
}

/* we use the fast interpretations from eval_init_form to check for free
 * variables without strcmp
 */
char *free_var_fast(Node *form, Interp *interp, const Structure *struc) {
  char *c;
  Node *relargs;
  int *iv;
  Relation *rel;

  switch (form->label)
  {
    case TRUE:
    case FALSE:
    case NUMBER:
      return NULL;
    case CONSTANT:
    case VAR:
      iv=form->ival;
      if (iv)
      {
        if (*iv>=0 || !strcmp((const char*) form->data,"max"))
          return NULL;
        return (char*) form->data;
      }
    case LT:
    case LTE:
    case NEQUALS:
    case EQUALS:
    case AND:
    case OR:
    case IMPLIES:
    case IFF:
    case MULT:
    case PLUS:
    case MINUS:
      c=free_var_fast(form->r,interp,struc);
      if (c)
        return c;
      /* else fall through to check the left */
    case NOT:
      return free_var_fast(form->l, interp, struc);
    case PRED:
      /* should also check correct number of arguments */
      rel = get_relation((const char*) form->data, interp, struc);
      if (!rel)
        return (char*) form->data;
      for (relargs = form->r; relargs; relargs=relargs->r)
      {
        c = free_var_fast(relargs->l, interp, struc);
        if (c)
          return c;
      }
      return NULL;
    case EXISTS:
    case FORALL:
      return free_var_fastq(form,interp,struc);
    case TC:
      return free_var_fasttc(form,interp,struc);
    case SOE:
      return free_var_fastsoe(form,interp,struc);
    case IFP:
    default:
      return NULL;
  }
  return NULL; /* unreachable */
}

char *free_var_fastq(Node *form, Interp *interp, const Structure *struc) {
  int arity=0,i;
  Node *vl=form->l->l;
  Node *tn;
  Node *restr = form->l->r;
  Node *phi = form->r;
  char *c=NULL;

  for (tn=vl; tn; tn=tn->r)
    arity++;
  int *old_values = (int*) malloc(arity*sizeof(int));
  int **values = (int**) malloc(arity*sizeof(int *));

  for (tn=vl,i=0; tn; tn=tn->r,i++)
  {
    values[i]=tn->ival;
    old_values[i]=*(values[i]);
    *(values[i])=0;
  }

  if (restr)
    c=free_var_fast(restr,interp,struc);
  if (!c)
    c=free_var_fast(phi,interp,struc);

  for (i=0; i<arity; i++)
    *(values[i])=old_values[i];

  free(values);
  free(old_values);
  return c;
}

char *free_var_fasttc(Node *form, Interp *interp, const Structure *struc) {
  char *c=NULL;
  Node *tmp, *tcargs, *tcform, *relargs;
  int arity, i;

  tcargs = form->l->l;
  tcform = form->l->r;
  relargs = form->r;

  for (tmp=relargs; tmp; tmp=tmp->r)
  {
    c=free_var_fast(tmp->l,interp,struc);
    if (c)
      return c;
  }

  arity=0;
  for (tmp=tcargs; tmp; tmp=tmp->r)
  {
    arity++;
    if (tmp->l->r)
      arity++;
  }
  int *old_values = (int*) malloc(sizeof(int)*arity);
  int **values = (int**) malloc(sizeof(int *)*arity);

  for (tmp=tcargs,i=0; tmp; tmp=tmp->r)
  {
    values[i]=tmp->l->l->ival;
    old_values[i]=*(values[i]);
    *values[i] = 1;
    i++;
    if (tmp->l->r)
    {
      values[i]=tmp->l->r->ival;
      old_values[i]=*(values[i]);
      *values[i]=1;
      i++;
    }
  }
  c = free_var_fast(tcform,interp,struc);
  for (tmp=tcargs,i=0; tmp; tmp=tmp->r)
  {
    *values[i]=old_values[i];
    i++;
    if (tmp->l->r)
    {
      *values[i]=old_values[i];
      i++;
    }
  }
  free(values);
  free(old_values);
  return c;
}

char *free_var_fastsoe(Node *form, Interp *interp, const Structure *struc) {
  char *varname=(char*)form->l->l->l->l->data;
  char *c=NULL;
  int arity =*(int *)form->l->l->l->r->data;
  Node *restr=form->l->r;
  Node *phi = form->r;
  Relation *sov = (Relation*) malloc(sizeof(Relation));
  sov->name = varname;
  sov->arity = arity;
  sov->next = interp->rel_symbols;
  sov->parse_cache=NULL;
  interp->rel_symbols=sov;

  if (restr)
    c=free_var_fast(restr,interp,struc);
  if (!c)
    c=free_var_fast(phi, interp,struc);

  interp->rel_symbols=sov->next;
  free(sov);
  return c;
}

/* returns a list of the free variables in form */
List *d_free_var(Node *form, Vocabulary *vocab) {
  List *tmp, *tmp2, *tmp3;
  Node *args, *args2, *tn;
  char *vn;
  ConsSymbol *cons;
  RelationSymbol *rel; 
  switch (form->label)
  {
    case TRUE:
    case FALSE:
    case NUMBER:
      return NULL;
    case CONSTANT:
    case VAR:
      if (!strcmp((const char*)form->data,"max"))
        return 0; /* max is a constant */
      for (cons=vocab->cons_symbols;cons;cons=cons->next)
        if (!strcmp(cons->name,(const char*)form->data))
          return 0;
      /* not a constant in the vocabulary, so free here*/
      tmp = (List*) malloc(sizeof(List));
      if (!tmp)
      {
        err("43: No memory\n");
        return 0;
      }
      tmp->next=NULL;
      tmp->data = form->data;
      return tmp;
    case NOT:
      return d_free_var(form->l, vocab);
    case LT:
    case LTE:
    case NEQUALS:
    case EQUALS:
    case AND:
    case OR:
    case IMPLIES:
    case IFF:
    case MULT:
    case PLUS:
    case MINUS:
      tmp = d_free_var(form->l, vocab);
      tmp2 = d_free_var(form->r, vocab);
      return join_lists(tmp,tmp2);
#if 0
      if (!tmp)
        return tmp2;
      if (!tmp2)
        return tmp;
      tmp3=tmp;
      while (tmp3->next)
        tmp3=tmp3->next;
      tmp3->next=tmp2;
      return tmp;
#endif
    case PRED:
      /* should also check correct number of arguments */
      for (rel = vocab->rel_symbols; rel; rel=rel->next)
        if (!strcmp(rel->name,(const char*) form->data))
          break;
      if (!rel) /* free occurence */
      {
        tmp = (List*) malloc(sizeof(List));
        if (!tmp)
        {
          err("44: No memory.\n");
          return 0;
        }
        tmp->data=form->data;
        tmp->next=NULL;
      }
      else
        tmp=NULL;
      args = form->r;
      while (args)
      {
        tmp2 = d_free_var(args->l, vocab);
        tmp=join_lists(tmp,tmp2);
        args = args->r;
      }
      return tmp;
    case EXISTS:
    case FORALL:
      args = form->l->l;
      tmp = NULL;
      tmp2 = NULL;
      tmp3 = NULL;
      tn = form->l->r; /* restriction */
      if (tn)
        tmp2=d_free_var(tn,vocab);
      tmp3 = d_free_var(form->r,vocab); /* formula */

      tmp = join_lists(tmp2, tmp3);
      tmp = remove_args(tmp, args); /*remove bound variables*/

      return tmp;
    case TC:
      args = form->l->l; /* these are the TC args, bound in
                          * tcform
                          */
      args2 = form->r; /* these are the external arguments, 
                        * need to add free occurences of 
                        * things here 
                        */
      tmp = tmp2 = tmp3 = NULL;
      tmp2 = d_free_var(form->l->r, vocab);

      tmp = remove_tcargs(tmp2,args); /* TC args are bound */

      while (args2)
      {
        tmp2 = d_free_var(args2->l, vocab);
        tmp = join_lists(tmp,tmp2);
        args2=args2->r;
      }
      return tmp;
    case SOE:
      vn = (char*) form->l->l->l->l->data; /* SO variable name */
      tn = form->l->r; /* restriction */
      tmp2=NULL;
      if (tn)
        tmp2 = d_free_var(tn, vocab);
      tmp3 = d_free_var(form->r, vocab);
      tmp = join_lists(tmp2,tmp3);
      for (tmp2=tmp,tmp3=NULL; tmp2; tmp2=tmp2->next)
      {
        if (strcmp((const char*)tmp2->data,vn))
        {
          tmp3=tmp2;
          continue;
        }
        /* otherwise we found one to remove */
        if (tmp3)
          tmp3->next=tmp2->next;
        else /* tmp2 == tmp */
          tmp=tmp2->next;
        free(tmp2);
        break; /* break is here because we are
                * assuming that each variable occurs
                * at most ONCE in the list.
                * Otherwise, we would need to keep
                * checking, and change the update
                * tmp2=tmp2->next to be a
                * tmp2=tmp4, where tmp4=tmp2->next
                * happens before the free.
                */
      }
      return tmp;
    case IFP:
    default:
      return 0; /* no checking unsupported things */
  }
  return 0; /* should be unreachable */
}

/* join list1 and list2, with data being null-terminated strings,
 * removing any duplicates (freeing those Lists but not the strings)
 * however, we can assume no entries occur twice (or more) in the same list
 * Note: list1 and list2 are modified - they should be forgotten (anything
 * not freed will be in the returned list).
 */
List *join_lists(List *list1, List *list2) {
  List *done=NULL, *tmp1, *tmp2, *tmp3, *s2, *prev;
  if (!list1)
    return list2;
  if (!list2)
    return list1;
  s2=list2;
  for (tmp1=list1; tmp1;)
  {
    char *t= (char*) tmp1->data;
    for (tmp2=s2,prev=NULL;tmp2;)
    {
      tmp3=tmp2->next;
      if (strcmp(t,(const char*)tmp2->data))
      {
        prev=tmp2;
        tmp2=tmp3;
        continue;
      }
      if (prev)
        prev->next=tmp2->next;
      else
        s2=tmp2->next;
      free(tmp2);
      tmp2=tmp3; /* prev stays the same */
    }
    tmp3=tmp1->next;
    tmp1->next=done;
    done=tmp1;
    tmp1=tmp3;
  }
  for (tmp1=done; tmp1; tmp1=tmp1->next)
    if (!tmp1->next)
    {
      tmp1->next = s2;
      break;
    }
  return done;
}

/* list is a singly-linked list of variable names (null-terminated strings).
 * we want to remove any occurences of the variables in args
 * which is the varlist from a first-order quantifier.
 * list gets clobbered (but data in it doesn't)
 */
List *remove_args(List *list, Node *args) {
  char *name;
  List *tmp, *done=NULL, *tmp3;
  Node *cur;
  int flag;

  for (tmp=list; tmp;)
  {
    flag = 0;
    name = (char *)tmp->data;
    for (cur=args; cur; cur=cur->r)
      if (!strcmp(name,(const char*) cur->data))
      {
        flag=1;
        break;
      }
    tmp3=tmp->next;
    if (!flag)
    {
      tmp->next = done;
      done = tmp;
    }
    else /* if (flag), clobber tmp */
      free(tmp);
    tmp=tmp3;
  }

  return done;
}

/* remove the TC arguments from list, because they're bound.
 * Based on eval_tc
 * Mangles list, just use the return.
 */
List *remove_tcargs(List *list, Node *tcargs) {
  Node *tmp;
  List *ret;
  int tup_arity, i;

  ret = list;

  for (tup_arity=0,tmp=tcargs; tmp; tup_arity++)
    tmp=tmp->r;

  tmp = tcargs;

  for (i=0; ;tmp=tmp->r)
  {
    ret = list_remove(ret, (char*) tmp->l->l->data);
    i++; 
    if (++i<tup_arity)
      ret = list_remove(ret, (char*) tmp->l->r->data);
    else
      break;
  }
  if (tup_arity&1) /* odd arity means we split one */
  {
    ret = list_remove(ret, (char*) tmp->l->r->data);
    i=1;
  }
  else
    i=0;
  for (tmp=tmp->r; ;tmp=tmp->r)
  {
    if (!tmp)
      break;
    ret = list_remove(ret, (char*) tmp->l->l->data);
    if (++i<tup_arity)
      ret = list_remove(ret, (char*) tmp->l->r->data);
    else
      break;
  }

  return ret;
}

/* remove key from list */
List* list_remove(List *list, char *key) {
  char *name;
  List *tmp, *done=NULL, *tmp3;

  for (tmp=list; tmp;)
  {
    name = (char *)tmp->data;
    tmp3=tmp->next;
    if (strcmp(name,key))
    {
      tmp->next = done;
      done = tmp;
    }
    else /* clobber tmp */
      free(tmp);
    tmp=tmp3;
  }

  return done;
}


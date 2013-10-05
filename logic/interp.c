// ISC LICENSE
/* interp.c
 * Skip Jordan
 *
 * Utility functions for interpretations.
 * chj	11/16/06	created
 * chj  12/11/06        bug fix to off-by-one error in add_tup_to_interp
 * chj   4/18/12        replace trpow&get_order for trivial performance benefit
 * chj	 7/13/12	changes for fast interpretations
 */

#include "types.h"
#include "protos.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

struct interp *new_interp(const struct structure *struc)
{
  struct interp *interp = malloc(sizeof(struct interp));
  struct interp_symbol *symb;
  struct constant *cons;
  if (!interp)
    return 0;
  interp->rel_symbols = NULL;
  if (!struc)
  {
    interp->symbols = NULL;
    return interp;
  }
  cons = struc->cons;

  if (!cons)
  {
    interp->symbols = NULL;
    return interp;
  }
  symb = malloc(sizeof(struct interp_symbol));
  interp->symbols = symb;
  while (cons)
  {
    symb->value= cons->value;
    symb->name = dupstr(cons->name);
    cons = cons->next;
    if (cons)
    {
      symb->next=malloc(sizeof(struct interp_symbol));
      symb=symb->next;
    }
  }
  symb->next = NULL;

  return interp;
}

/* return the value of x{i} in interp */
int get_xi_interp_value(int i, struct interp *interp)
{
  struct interp_symbol *is = interp->symbols;
  char *name;

  for (is=interp->symbols; is; is=is->next)
  {
    name=is->name;
    if (name[0]!='x' || !isdigit(name[1]))
      continue;
    if (atoi(name+1)==i)
      return is->value;
  }

  return -1;
}

int *get_xi_ival(int i, struct interp *interp)
{
  struct interp_symbol *is = interp->symbols;
  char *name;

  for (is=interp->symbols; is; is=is->next)
  {
    name=is->name;
    if (name[0]!='x' || !isdigit(name[1]))
      continue;
    if (atoi(name+1)==i)
      return &(is->value);
  }
  return NULL;
}

void add_xi_interp(int i, struct interp *interp, int val)
{
  char *name=malloc(sizeof(char)*(2+numdigits(i)));
  sprintf(name,"x%d",i);
  add_symb_to_interp(interp, name, val);
  free(name);
  return;
}


int get_interp_value(const char *name, const struct interp *interp)
{
  struct interp_symbol *is = interp->symbols;

  while (is && strcmp(is->name, name))
    is = is->next;

  if (is)
    return is->value;

  return -1;
}

/* the tuple (x1,...,xarity) is probably already in the interp.  
 * Take it out and reinsert at the head of the list in reverse order, seting
 * values to tup.  Add things not there (should not happen).
 */
struct interp *fake_add_tup_to_interp(struct interp *interp, int *tup,
    int arity)
{
  char *name = malloc(sizeof(char)+2+numdigits(arity));
  struct interp *ret=interp;
  int i;

  for (i=1; i<=arity; i++)
  {
    sprintf(name,"x%d",i);
    ret = fake_add_symb_to_interp(ret, name, tup[i-1]);
  }

  return ret;
}

/* the symbol is probably already in the interp.  Take it out and reinsert at
 * the head of the list, setting the value to value.
 * If it's not already in there, add it to the start (should not happen) 
 */
struct interp *fake_add_symb_to_interp(struct interp *interp, const char *symb,
    const int value)
{
  struct interp_symbol *is, *pre=NULL;
  for (is=interp->symbols; is; is=is->next)
  {
    if (!strcmp(symb, is->name))
    {
      if (!pre)
      {
        is->value = value;
        return interp;
      }
      pre->next=is->next;
      is->next=interp->symbols;
      interp->symbols=is;
      is->value = value;
      return interp;
    }
    pre=is;
  }
  /* should be unreachable */
  printf("D: Adding new symbol %s(=%d) to interpretation\n",
      symb, value);
  return add_symb_to_interp(interp, symb, value);
}

struct interp *add_symb_to_interp(struct interp *interp, const char *symb, 
    const int value)
{
  struct interp_symbol *is = malloc(sizeof(struct interp_symbol));
  if (!is)
  {
    return 0;
  }
  is->next = interp->symbols;
  is->name = dupstr(symb);
  is->value = value;
  interp->symbols = is;

  return interp;
}


/* TODO priority 2 for performance on 11/29/06 */
/* tup is an array of arity integers representing I(x1), I(x2)...I(xarity).
 * add (x1, I(x1))... to the interpretation.
 * We add them to the start of the singly linked list, where they will
 * shadow any duplicate names, and can be easily removed.
 */
struct interp *add_tup_to_interp(struct interp *interp, const int *tup, const int arity)
{
  struct interp_symbol *is;
  char *vname;
  int order, i;

  if (arity<9)
    order=1;
  else if(arity<99)
    order=2;
  else
    order=get_order(arity+1);

  vname = malloc((order+1+1)*sizeof(char)); /* x+i+\0 */

  for (i=0; i<arity; i++)
  {
    is = malloc(sizeof(struct interp_symbol));
    is->next = interp->symbols;
    interp->symbols = is;

    is->value = tup[i];
    sprintf(vname, "x%d", i+1);
    is->name = dupstr(vname);
  }

  return interp;
}

/* returns the number of digits in the decimal representation of arity */
/* incredibly slow, but if anyone gives me relations of much higher order
 * than 100, they're probably trying to hack us anyways :P
 */
int get_order(int arity)
{
  if (arity<10)
    return 1;
  else if (arity<100)
    return 2;
  return floor(log10(arity))+1;
}

/* return a duplicate copy of the interpretation */
/* ignores rel_symbols :-/ */
struct interp *dup_interp(struct interp *interp)
{
  struct interp_symbol *is, *nis;
  struct interp *n;

  n = new_interp(NULL);
  for (is=interp->symbols; is; is=is->next)
  {
    nis = malloc(sizeof(struct interp_symbol));
    nis->name = dupstr(is->name);
    nis->value = is->value;
    nis->next = n->symbols;
    n->symbols = nis;
  }

  return n;
}

/* remove the first arity tuples from interp, freeing all memory */
struct interp *free_remove_tup(struct interp *interp, int arity)
{
  struct interp_symbol *is;
  struct interp_symbol *nis;
  int i;

  is = interp->symbols;
  for (i=0; i<arity; i++)
  {
    assert(is);
    nis = is->next;
    free(is->name);
    free(is);
    is=nis;
  }
  interp->symbols = is;

  return interp;
}

void free_interp(struct interp *interp)
{
  int i=0;
  struct interp_symbol *is = interp->symbols;

  while (is)
  {
    is = is->next;
    i++;
  }

  interp = free_remove_tup(interp, i);

  free(interp);

  return;
}

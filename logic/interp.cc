// ISC LICENSE
// Utility functions for interpretations.

#include "types.h"
#include "protos.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

Interp *new_interp(const Structure *struc) {
  Interp *interp = (Interp*) calloc(1, sizeof(Interp));
  InterpSymbol *symb;

  if (!interp) return nullptr;
  if (!struc || !struc->cons) return interp;

  for(Constant *cons = struc->cons; cons; cons = cons->next) {
    InterpSymbol* symb = (InterpSymbol*) malloc(sizeof(InterpSymbol));
    
    symb->value = cons->value;
    symb->name = dupstr(cons->name);
    
    // add to list of InterpSymbols on Interp
    symb->next = interp->symbols;
    interp->symbols = symb;
  }

  return interp;
}

/* return the value of x{i} in interp */
int get_xi_interp_value(int i, Interp *interp) {
  for (InterpSymbol *is=interp->symbols; is; is=is->next) {
    const char *name=is->name;
    if (name[0] != 'x' || !isdigit(name[1]))
      continue;
    if (atoi(name+1)==i)
      return is->value;
  }

  return -1;
}

int *get_xi_ival(int i, Interp *interp)
{
  InterpSymbol *is = interp->symbols;
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

void add_xi_interp(int i, Interp *interp, int val)
{
  char *name=(char*) malloc(sizeof(char)*(2+numdigits(i)));
  sprintf(name,"x%d",i);
  add_symb_to_interp(interp, name, val);
  free(name);
  return;
}


int get_interp_value(const char *name, const Interp *interp) {
  InterpSymbol *is = interp->symbols;

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
Interp *fake_add_tup_to_interp(Interp *interp, int *tup, int arity) {
  char *name = (char*) malloc(sizeof(char)+2+numdigits(arity));
  Interp *ret=interp;
  int i;

  for (i=1; i<=arity; i++) {
    sprintf(name,"x%d",i);
    ret = fake_add_symb_to_interp(ret, name, tup[i-1]);
  }

  return ret;
}

/* the symbol is probably already in the interp.  Take it out and reinsert at
 * the head of the list, setting the value to value.
 * If it's not already in there, add it to the start (should not happen) 
 */
Interp *fake_add_symb_to_interp(Interp *interp, const char *symb, const int value)
{
  InterpSymbol *is, *pre=NULL;
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

Interp *add_symb_to_interp(Interp *interp, const char *symb, 
    const int value)
{
  InterpSymbol *is = (InterpSymbol*) malloc(sizeof(InterpSymbol));
  if (!is) {
    return 0;
  }
  is->next = interp->symbols;
  is->name = dupstr(symb);
  is->value = value;
  interp->symbols = is;

  return interp;
}


/* TODO priority 2 for performance on 11/29/06 */
/* tup is an array of arity integers representing I(x1), I(x2)...I(x_n).
 * add (x1, I(x1))... to the interpretation.
 * We add them to the start of the singly linked list, where they will
 * shadow any duplicate names, and can be easily removed.
 */
Interp *add_tup_to_interp(Interp *interp, const int *tup, const int arity)
{
  int order = get_order(arity+1);

  char* vname = (char*) malloc((order+1+1)*sizeof(char)); /* x+i+\0 */

  for (int i=0; i<arity; i++) {
    InterpSymbol *is = (InterpSymbol*) malloc(sizeof(InterpSymbol));
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
Interp *dup_interp(Interp *interp)
{
  InterpSymbol *is, *nis;
  Interp *n;

  n = new_interp(NULL);
  for (is=interp->symbols; is; is=is->next)
  {
    nis = (InterpSymbol*) malloc(sizeof(InterpSymbol));
    nis->name = dupstr(is->name);
    nis->value = is->value;
    nis->next = n->symbols;
    n->symbols = nis;
  }

  return n;
}

/* remove the first k tuples from interp, freeing all memory */
Interp *free_remove_tup(Interp *interp, int k)
{
  InterpSymbol *is;
  InterpSymbol *nis;
  int i;

  is = interp->symbols;
  for (i=0; i<k; i++) {
    assert(is);
    nis = is->next;
    free(is->name);
    free(is);
    is=nis;
  }
  interp->symbols = is;

  return interp;
}

void free_interp(Interp *interp) {
  int i=0;
  InterpSymbol *is = interp->symbols;

  while (is) {
    is = is->next;
    i++;
  }

  interp = free_remove_tup(interp, i);

  free(interp);

  return;
}

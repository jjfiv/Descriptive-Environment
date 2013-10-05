// ISC LICENSE
/* reduc.c
 * Functions for applying reductions and queries.
 */

#include "types.h"
#include "parse.h"
#include "protos.h"
#include "stdlib.h"

/* We make maps from k-tuples to naturals and naturals to k-tuples for
 * the structure induced by reduc on struc.
 */

/* We have a choice.  Either we malloc both caches at the start and loop once,
 * probably wasting memory because we only have an upper bound on the number
 * of tuples in the universe at that point, or we malloc the tup_to_nat cache,
 * fill that, count how many we used, malloc a proper-sized nat_to_tup cache,
 * and then build that cache from the tuple cache.
 * At the moment I do the first, \t is probably common as a formula defining
 * the k-tuples in |I(A)|, in which case our upper bound is also exact,
 * and this is only a temporary cache anyways, it's thrown away once the induced
 * structure is assigned to an id.
 */
ReductionMap *make_rmap(const Reduction *reduc, Structure *struc)
{
  Node *univ_form;
  Interp *interp;
  int k, cindex, size, i, res=0;
  int *tuple=0;
  int num_tuples;
  ReductionMap *rmap;
  int tmp, j;
  InterpSymbol *is;
  InterpSymbol *base;
  int *first;

  univ_form = reduc->universe_form;
  k = reduc->k;
  size = struc->size;
  /* TODO check mallocs */
  num_tuples = trpow(size,k);
  interp = new_interp(struc);

  rmap = malloc(sizeof(ReductionMap));
  rmap->nat_to_tup = malloc(num_tuples*sizeof(int));
  rmap->tup_to_nat = malloc(num_tuples*sizeof(int));

  tuple = next_tuple(tuple, k, size);
  interp = add_tup_to_interp(interp,tuple,k);
  interp->symbols->value = -1;
  free(tuple);
  base = interp->symbols->next;
  first = &(interp->symbols->value);
  tmp = k - 1;

  i = 0;
  cindex = -1;
  while (1)
  {
    (*first)++;
    if (*first>=size)
    {
      *first = 0;
      res = 0;
      for (j=0, is=base; j<tmp; j++,is=is->next)
      {
        res=++is->value;
        if (is->value < size)
          break;
        res = is->value = 0;
      }
      if (!res && j==tmp)
        break;
    }

    res = eval(univ_form,interp,struc);
    cindex++;
    rmap->tup_to_nat[cindex]=-1;

    if (res)
    {
      rmap->nat_to_tup[i] = cindex;
      rmap->tup_to_nat[cindex]=i;
      i++;
    }
  }

  rmap->size = i;
  free_interp(interp);

  return rmap;
}

/*
Copyright (c) 2006-2012, Charles Jordan <skip@alumni.umass.edu>

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
/* reduc.c
 * Skip Jordan
 *
 * Functions for applying reductions and queries.
 * chj	11/26/06	created
 * chj  12/7/06		trimmed loop for performance
 * chj	6/12/12		bugfix in make_rmap
 */

#include "types.h"
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
struct reduc_map *make_rmap(const struct reduction *reduc, 
			    struct structure *struc)
{
	struct node *univ_form;
	struct interp *interp;
	int k, cindex, size, i, res=0;
	int *tuple=0;
	int num_tuples;
	struct reduc_map *rmap;
	int tmp, j;
	struct interp_symbol *is;
	struct interp_symbol *base;
	int *first;

	univ_form = reduc->universe_form;
	k = reduc->k;
	size = struc->size;
	/* TODO check mallocs */
	num_tuples = trpow(size,k);
	interp = new_interp(struc);

	rmap = malloc(sizeof(struct reduc_map));
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

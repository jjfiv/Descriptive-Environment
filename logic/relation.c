// ISC LICENSE
/* relation.c
 * Skip Jordan
 *
 * Various functions related to relations
 * chj 11/16/06	created
 * chj	1/28/13	fill_relcache
 */

#include "protos.h"
#include "types.h"
#include <string.h>
#include <stdio.h>

Relation *get_relation(const char *name, const Interp *inter, const Structure *struc)
{
  /* ignore inter for now :-P */
  Relation *rel=0;

  if (inter)
    rel = inter->rel_symbols;

  while (rel && strcmp(rel->name, name))
    rel = rel->next;
  if (rel || !struc)
    return rel;
  rel = struc->rels;
  while (rel && strcmp(rel->name, name))
    rel=rel->next;
  return rel;
}

/* doesn't combine with fast interpretations -- use OUTSIDE of
 * eval/eval_rec only
 */
void fill_relcache(Relation *rel, Structure *str)
{
  int arity, size;
  int *tuple=NULL;
  Interp *interp;
  int tuple_num, res;

  arity = rel->arity;
  size = str->size;

  if (rel->cache)
    tuple_num = 0;
  else
  {
    printf("??: Fixme, no cache reserved for %s.\n",rel->name);
    return;
  }

  interp = new_interp(str);

  if (rel->parse_cache)
    eval_init_form(rel->parse_cache, interp, str);

  while ((tuple = next_tuple(tuple, arity, size)))
  {
    if ((res=rel->cache[tuple_num++])>-1)
      continue;

    interp = fake_add_tup_to_interp(interp, tuple, arity);
    res=eval_rec(rel->parse_cache, interp, str);
    if (tuple_num>=0)
      rel->cache[tuple_num-1]=res;
  }

  free_interp(interp);
  return;
}


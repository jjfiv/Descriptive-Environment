/*
   Copyright (c) 2006-2013, Charles Jordan <skip@alumni.umass.edu>

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

struct relation *get_relation(const char *name, const struct interp *inter,
    const struct structure *struc)
{
  /* ignore inter for now :-P */
  struct relation *rel=0;

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
void fill_relcache(struct relation *rel, struct structure *str)
{
  int arity, size;
  int *tuple=NULL;
  struct interp *interp;
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

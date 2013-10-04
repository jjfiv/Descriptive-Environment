/*
   Copyright (c) 2006-2011, Charles Jordan <skip@alumni.umass.edu>

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
/* tuple.c
 * Skip Jordan
 *
 * Functions for dealing with tuples
 * chj	11/17/06	created
 * chj  11/23/06	tuple_cindex for relation caches
 * chj	12/4/06		make_next_tuple and cindex_to_tuple
 */

#include "protos.h"
#include "types.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* returns the next tuple contained in the reduction map of tuples to ints.  
 * cindex is the cindex of the CURRENT tuple, it should also be incremented
 * if tuple is null, malloc it and return the first.  if tuple is the max,
 * free it and return NULL.
 */
int *make_next_tuple(int *tuple, const struct reduc_map *rmap, 
    const int arity, const int size, int *cindex, 
    const int cindexarity, const int k)
{
  int i;
  int p=0;
  int rsize=rmap->size;
  int *nat_to_tup=rmap->nat_to_tup;
  int *tup_to_nat=rmap->tup_to_nat;
  int zero = nat_to_tup[0];
  /*int *cindex = *pcindex;*/
  if (!tuple)
  {
    tuple = malloc(arity*sizeof(int));
    /* cindex = malloc(cindexarity*sizeof(int));*/
    for (i=0; i<cindexarity; i++)
      cindex[i]=zero;
    i=-1;
  }
  else
  {
    for (i=cindexarity-1; i>=0; i--)
    {
      p = tup_to_nat[cindex[i]];		 
      p++;
      if (p>=rsize)
      {
        cindex[i]=zero;
      }
      else
      {
        cindex[i]=nat_to_tup[p];
        break;
      }
    }
    if (p>=rsize) /* we did the last tuple already, free and ret */
    {
      free(tuple);
      return 0;
    }
  }
  if (i==-1)
    i++;
  p=(i)*k;
  for (; i<cindexarity; i++, p+=k)
  {
    cindex_to_tuple(tuple+p, cindex[i], k, size);
  }
  return tuple;
}

/* the inverse of tuple_cindex */
int *cindex_to_tuple(int *tuple, const int cindex, const int arity, 
    const int size)
{
  int i, p=trpow(size,arity-1), c=cindex;
  for (i=0; i<arity; i++)
  {
    tuple[i] = c / p;
    c = c % p;
    p = p / size;
  }
  return tuple;
}

/* returns the index of tuple tup in a fixed ordering of tuples of the given
 * arity in a universe of the given size.  Used for caches.
 * For performance, it's important that <0,1> follows <0,0>, etc.
 * This ordering MUST be the SAME as next_tuple, or things WILL
 * explode.
 */
int tuple_cindex(const int *tup, const int arity, const int size)
{
  int i;
  int p=0;
  for (i=0; i<arity; i++)
    p = size * p + tup[i];
  return p;
}

/* note that the contents of tuple are over-written, and returned.
 * frees tuple and returns NULL if the last was the last. :P
 * bad semantics, but hey.
 */
int *next_tuple(int *tuple, const int arity, const int size)
{
  int i;

  if (!tuple)
  {
    tuple=malloc(arity * sizeof(int));
    for (i=0; i<arity; i++)
      tuple[i] = 0;
    return tuple;
  }
  for (i=arity-1; i>=0; i--)
  {
    if (++tuple[i]>=size)
      tuple[i]=0;
    else
      break;
  }

  if (i==-1 && tuple[i+1] == 0)
  {
    free(tuple);
    return NULL;
  }

  return tuple;
}

/* kills and frees output */

char *add_tup_to_output(char *output, const int *tuple, const int arity, const int size)
{
  char *new_output;
  char *buf;
  int length;
  int i;
  int j;
  int k;

  length = (output?strlen(output):1); /* one for a '{' */

  for (i=0; i<arity; i++)
  {
    length+=get_order(tuple[i]);
  }

  length += 3; /* for '(' and ')' and '\0' */
  length += arity - 1; /* for commas :P */
  if (output)
    length+=2; /* for a comma and space before the tuple */

  new_output = malloc(length * sizeof(char));

  if (!output)
  {
    new_output[0]='{';
    i=1;
  }
  else
  {
    for (i=0; output[i]; i++)
      new_output[i]=output[i];
    new_output[i++]=',';
    new_output[i++]=' ';
  }

  new_output[i++] = '(';
  buf = malloc((get_order(size)+1)*sizeof(char));
  for (j=0; j<arity; j++)
  {
    sprintf(buf, "%d", tuple[j]);
    for (k=0; buf[k]; k++)
      new_output[i++] = buf[k];
    if (j+1<arity)
      new_output[i++] = ',';
  }
  new_output[i++] = ')';
  new_output[i++] = '\0';

  free(buf);
  if (output)
    free(output);
  return new_output;
}

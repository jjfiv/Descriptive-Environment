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
/* types.h
 * Skip Jordan
 *
 * Basic type definitions.
 * chj 	11/13/06	created.
 */

#ifndef DETYPESH
#define DETYPESH

#include "hash.h"

#define DEBUG

#define MAX_IDS 1024 /* maximum number of total ids allowed :-( */

#ifndef assert
#ifdef DEBUG
#define assert(x) if (!(x)) printf("Failed assertion!\n");
#else
#define assert(x);
#endif
#endif
struct interp {
	struct interp_symbol *symbols;
	struct relation *rel_symbols;
	};

struct interp_symbol {
	struct interp_symbol *next;
	char *name;
	int value;
	};

struct structure {
	struct relation *rels;
	struct constant *cons;
	struct vocab *vocab;  /* really the vocab can be implied   */
			      /* but we need the vocab for queries */
	char *name;
	int size;
	int id;	
	};

struct relation {
	char *name;
   /*	char *formula;   We may not know the formula :( */
	struct node *parse_cache;
	/* struct something cache of tuples */
	int *cache;
	struct relation *next;
	int arity;
	};

struct reduction {
	char *name;
	struct vocab *from_vocab;
	struct vocab *to_vocab;
	struct node *universe_form;
	int k;
	struct relation *relforms;
	struct consform *consforms;
	};

struct bquery {
	char *name;
	struct vocab *voc;
	struct node *form;
	};

struct reduc_map {
	int size;
	int *nat_to_tup; /* array where a[i] is the cindex of the ith tuple */
	int *tup_to_nat; /* array where a[cindex] is the number of the tuple */
	};

struct consform {
	char *name;
	struct node *parse_cache;
	/* int arity; */
	struct consform *next;
	};

struct constant {
	char *name;
  /*	char *formula; */
	struct node *parse_cache;
	struct constant *next;
	int value;
	};

struct vocab {
	struct rel_symbol  *rel_symbols;
	struct cons_symbol *cons_symbols;
	char *name;
	int id;
	};

struct rel_symbol {
	char *name;
	int arity;
	struct rel_symbol *next;
	};

struct cons_symbol {
	char *name;
	struct cons_symbol *next;
	};

struct id {
	char *name;
	void *def;
	int type;
	};

struct env {
	struct hash_t *id_hash;
	int next_id;
	};

struct list {
	void *data;
	struct list *next;
	};

extern struct env *cur_env;

#endif /* DETYPESH */

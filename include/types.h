// ISC LICENSE
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
typedef struct interp {
  struct interp_symbol *symbols;
  struct relation *rel_symbols;
} Interp;

typedef struct interp_symbol {
  struct interp_symbol *next;
  char *name;
  int value;
} InterpSymbol;

typedef struct structure {
  struct relation *rels;
  struct constant *cons;
  struct vocab *vocab;  /* really the vocab can be implied   */
  /* but we need the vocab for queries */
  char *name;
  int size;
  int id;	
} Structure;

typedef struct relation {
  char *name;
  /*	char *formula;   We may not know the formula :( */
  struct node *parse_cache;
  /* struct something cache of tuples */
  int *cache;
  struct relation *next;
  int arity;
} Relation;

typedef struct reduction {
  char *name;
  struct vocab *from_vocab;
  struct vocab *to_vocab;
  struct node *universe_form;
  int k;
  struct relation *relforms;
  struct consform *consforms;
} Reduction;

typedef struct bquery {
  char *name;
  struct vocab *voc;
  struct node *form;
} BQuery;

typedef struct reduc_map {
  int size;
  int *nat_to_tup; /* array where a[i] is the cindex of the ith tuple */
  int *tup_to_nat; /* array where a[cindex] is the number of the tuple */
} ReductionMap;

typedef struct consform {
  char *name;
  struct node *parse_cache;
  /* int arity; */
  struct consform *next;
} ConsForm;

typedef struct constant {
  char *name;
  /*	char *formula; */
  struct node *parse_cache;
  struct constant *next;
  int value;
} Constant;

typedef struct vocab {
  struct rel_symbol  *rel_symbols;
  struct cons_symbol *cons_symbols;
  char *name;
  int id;
} Vocabulary;

typedef struct rel_symbol {
  char *name;
  int arity;
  struct rel_symbol *next;
} RelationSymbol;

typedef struct cons_symbol {
  char *name;
  struct cons_symbol *next;
} ConsSymbol;

typedef struct id {
  char *name;
  void *def;
  int type;
} Identifier;

typedef struct env {
  struct hash_t *id_hash;
  int next_id;
} Environment;

typedef struct list {
  void *data;
  struct list *next;
} List;

extern Environment *cur_env;

#endif /* DETYPESH */

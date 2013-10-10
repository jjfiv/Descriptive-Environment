// ISC LICENSE
/* types.h
 * Skip Jordan
 *
 * Basic type definitions.
 * chj 	11/13/06	created.
 */

#ifndef DETYPESH
#define DETYPESH

#include "parse.h"
#include "hash.h"
#include <assert.h>

#define MAX_IDS 1024 /* maximum number of total ids allowed :-( */

typedef struct RelationSymbol {
  char *name;
  int arity;
  struct RelationSymbol *next;
} RelationSymbol;

typedef struct ConsSymbol {
  char *name;
  struct ConsSymbol *next;
} ConsSymbol;

typedef struct Vocabulary {
  RelationSymbol  *rel_symbols;
  ConsSymbol *cons_symbols;
  char *name;
  int id;
} Vocabulary;

typedef struct Relation {
  char *name;
  /*	char *formula;   We may not know the formula :( */
  Node *parse_cache;
  /* struct something cache of tuples */
  int *cache;
  struct Relation *next;
  int arity;
} Relation;

typedef struct Constant {
  char *name;
  /*	char *formula; */
  Node *parse_cache;
  struct Constant *next = nullptr;
  int value;
} Constant;

typedef struct ReductionMap {
  int size;
  int *nat_to_tup; /* array where a[i] is the cindex of the ith tuple */
  int *tup_to_nat; /* array where a[cindex] is the number of the tuple */
} ReductionMap;

typedef struct ConsForm {
  char *name;
  Node *parse_cache;
  /* int arity; */
  struct ConsForm *next;
} ConsForm;

typedef struct Identifier {
  char *name = nullptr;
  void *def = nullptr;
  int type = -1;
} Identifier;

typedef struct Environment {
  struct hash_t *id_hash = nullptr;
  int next_id = 0;
} Environment;

typedef struct List {
  void *data = nullptr;
  struct List *next = nullptr;
} List;

typedef struct InterpSymbol {
  struct InterpSymbol *next = nullptr;
  char *name = nullptr;
  int value = 0;
} InterpSymbol;

typedef struct Interp {
  InterpSymbol *symbols = nullptr;
  Relation *rel_symbols = nullptr;
} Interp;

typedef struct Structure {
  Relation *rels;
  Constant *cons;
  Vocabulary *vocab;  /* really the vocab can be implied   */
  /* but we need the vocab for queries */
  char *name;
  int size;
  int id;	
} Structure;

typedef struct Reduction {
  char *name;
  Vocabulary *from_vocab;
  Vocabulary *to_vocab;
  Node *universe_form;
  int k;
  Relation *relforms;
  ConsForm *consforms;
} Reduction;

typedef struct BQuery {
  char *name;
  Vocabulary *voc;
  Node *form;
} BQuery;

extern Environment *cur_env;

#endif /* DETYPESH */

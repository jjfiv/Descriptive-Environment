// ISC LICENSE
#ifndef _DE_PROTOS_H
#define _DE_PROTOS_H

#include "types.h"
#include "parse.h"
#include <cstdio> /* for FILE * */
#include <string>
#include <sstream>
#include <iostream>
using namespace std;
#include "soe_parse.tab.hh"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string (const char *yy_str  );
extern void yy_delete_buffer (YY_BUFFER_STATE  b );

#define DEBUG 1

#if DEBUG
#define err printf
#else
#define err(...)
#endif

#define numdigits(i) (i==0?1:floor(log10(i))+1)

/* env.c */
void runCommand(Environment*, string cmd);
void init_env(void);
string gensym(Environment *env);
Identifier* getBinding(Environment*, string name);
void removeBinding(Environment*, string name);
Identifier* makeBinding(Environment*, char *name, int type, void* data);
BQuery* getBQuery(Environment*, string name);
Structure* getStructure(Environment*, string name);
Vocabulary* getVocab(Environment *env, string name);
Reduction* getReduction(Environment *env, string name);

/* cmd.c */
void command_loop(void);
int do_cmd_str(const char *, size_t);
int do_cmd(Environment *, Node *);
int do_assign_command(Environment *, Node *);
int do_vocab_assign(Environment *, Node *);
int do_struc_assign(Environment *, Node *);
int do_reduc_assign(Environment *, Node *);
int do_expred_command(Environment *, Node *);
int do_excons_command(Environment *, Node *);
int do_listtuple_command(Environment *, Node *);
int do_apply_assign(Environment *, Node *);
int do_bquery_assign(Environment *, Node *);
int do_abquery_command(Environment *, Node *);
int do_minisat_query(const Structure *);
int do_threecolorsat_query(const Structure *);
int do_minisat2_query(const Structure *);
int do_threecolor_sat2_query(const Structure *);
int do_mace(Environment *, Node *);
int do_fd(Environment *, Node *);
int do_redfind(Environment *, Node *);

/* file.c */
int do_draw_command(Environment *, Node *);
int do_load(Environment *, Node *);
int do_save_command(Environment *, Node *);
int save_struc(Structure *, FILE *);
int save_tuple_line(Relation *, int *, FILE *);
int save_voc_inner(Vocabulary *, FILE *);
char *loadstring_getdec(char *, int, Vocabulary *);
int do_loadassign(Environment *, Node *);
void loadstring_convert(Structure *, int, char *);

/* help.c */
int do_help(void);

/* check.c */
char *free_var(Node *, Vocabulary *);
char *free_var_fast(Node *, Interp *, const Structure *);
char *free_var_fastq(Node *, Interp *, const Structure *);
char *free_var_fasttc(Node *, Interp *, const Structure *);
char *free_var_fastsoe(Node *, Interp *, const Structure *);
List *d_free_var(Node *, Vocabulary *);
List *join_lists(List *, List *);
List *remove_args(List *, Node *);
List *remove_tcargs(List *, Node *);
List *list_remove(List *, char *);

/* eval.c */
int eval(Node *, Interp *, const Structure *);
int eval_rec(Node *, Interp *, const Structure *);
int teval(Node *, Interp *, const Structure *);
int eval_pred(Node *, Interp *, const Structure *);
int eval_exists(Node *, Interp *, const Structure *);
int eval_forall(Node *, Interp *, const Structure *);
int eval_tc(Node *, Interp *, const Structure *);
int eval_ifp(Node *, Interp *, const Structure *);
int eval_soe(Node *, Interp *, const Structure *);
void eval_init_form(Node *, Interp *, const Structure *);
void eval_init_form_q(Node *, Interp *, const Structure *);
void eval_init_form_tc(Node *, Interp *, const Structure *);
void eval_init_form_pred(Node *, Interp *, const Structure *);
void eval_init_form_soe(Node *, Interp *, const Structure *);
void free_tc_caches(Node *);

/* interp.c */
Interp *new_interp(const Structure *struc);
Interp *free_remove_tup(Interp *, int);
Interp *dup_interp(Interp *);
int get_order(int);
Interp *fake_add_tup_to_interp(Interp *, int *, int);
Interp *add_tup_to_interp(Interp *, const int *, const int);

int get_interp_value(const char *, const Interp *);
int get_xi_interp_value(int, Interp *);
Interp *remove_xi_interp(int, Interp *);
int *get_xi_ival(int, Interp *);
void free_interp(Interp *);
Interp *fake_add_symb_to_interp(Interp *, const char *, const int);
void add_xi_interp(int, Interp *, int);
Interp *add_symb_to_interp(Interp *, const char *, const int);

/* tuple.c */
int *make_next_tuple(int *, const ReductionMap *, const int, const int,
    int *, const int, const int);
int tuple_cindex(const int *, const int, const int);
int *next_tuple(int *, const int, const int);
char *add_tup_to_output(char *, const int *, const int, const int);
int *cindex_to_tuple(int *, const int, const int, const int);

/* relation.c */
Relation *get_relation(const char *, const Interp *, const Structure *);
void fill_relcache(Relation *, Structure *);

/* constant.c */
Constant *get_constant(const char *, const Structure *);

/* reduc.c */
ReductionMap *make_rmap(const Reduction *, Structure *);

/* redfind.c */
int redfind(Environment *, const BQuery *,const BQuery *,int,int,int,int);

/* usemace.c */
int usemace(Environment*, Node *, Vocabulary *, char *, int);
int get_mace_model_size(FILE *);
int print_mace(FILE *, Node *);
int t_print_mace(FILE *, Node *);
int make_mace_model(Structure *, FILE *);
int need_arithmetic(Node *);

/* util.c */
char *dupstr(const char *);
long de_pow(int x, short p);
List *free_list(List *);
string simplify(const string &);
string stringf(const char *fmt, ...);
string temporaryFileName();

#endif


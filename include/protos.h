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
void runCommand(string cmd);
void init_env(void);
string gensym(Environment *env);
Identifier* getBinding(Environment*, string name);
void removeBinding(Environment*, string name);
BQuery* getBQuery(Environment*, string name);
Structure* getStructure(Environment*, string name);
Vocabulary* getVocab(Environment *env, string name);
Reduction* getReduction(Environment *env, string name);

/* cmd.c */
extern void command_loop(void);
extern int do_cmd_str(const char *, size_t);
extern int do_cmd(Environment *, Node *);
extern int do_assign_command(Environment *, Node *);
extern int do_vocab_assign(Environment *, Node *);
extern int do_struc_assign(Environment *, Node *);
extern int do_reduc_assign(Environment *, Node *);
extern int do_expred_command(Environment *, Node *);
extern int do_excons_command(Environment *, Node *);
extern int do_listtuple_command(Environment *, Node *);
extern int do_apply_assign(Environment *, Node *);
extern int do_bquery_assign(Environment *, Node *);
extern int do_abquery_command(Environment *, Node *);
extern int do_minisat_query(const Structure *);
extern int do_threecolorsat_query(const Structure *);
extern int do_minisat2_query(const Structure *);
extern int do_threecolor_sat2_query(const Structure *);
extern int do_mace(Environment *, Node *);
extern int do_fd(Environment *, Node *);
extern int do_redfind(Environment *, Node *);

/* file.c */
extern int do_draw_command(Environment *, Node *);
extern int do_load(Environment *, Node *);
extern int do_save_command(Environment *, Node *);
extern int save_struc(Structure *, FILE *);
extern int save_tuple_line(Relation *, int *, FILE *);
extern int save_voc_inner(Vocabulary *, FILE *);
extern char *loadstring_getdec(char *, int, Vocabulary *);
extern int do_loadassign(Environment *, Node *);
extern void loadstring_convert(Structure *, int, char *);

/* help.c */
extern int do_help(void);

/* check.c */
extern char *free_var(Node *, Vocabulary *);
extern char *free_var_fast(Node *, Interp *, const Structure *);
extern char *free_var_fastq(Node *, Interp *, const Structure *);
extern char *free_var_fasttc(Node *, Interp *, 
    const Structure *);
extern char *free_var_fastsoe(Node *, Interp *, 
    const Structure *);
extern List *d_free_var(Node *, Vocabulary *);
extern List *join_lists(List *, List *);
extern List *remove_args(List *, Node *);
extern List *remove_tcargs(List *, Node *);
extern List *list_remove(List *, char *);

/* eval.c */
extern int eval(Node *, Interp *, const Structure *);
extern int eval_rec(Node *, Interp *, const Structure *);
extern int teval(Node *, Interp *, const Structure *);
extern int eval_pred(Node *, Interp *, const Structure *);
extern int eval_exists(Node *, Interp *, const Structure *);
extern int eval_forall(Node *, Interp *, const Structure *);
extern int eval_tc(Node *, Interp *, const Structure *);
extern int eval_ifp(Node *, Interp *, const Structure *);
extern int eval_soe(Node *, Interp *, const Structure *);
extern void eval_init_form(Node *, Interp *, 
    const Structure *);
extern void eval_init_form_q(Node *, Interp *,
    const Structure *);
extern void eval_init_form_tc(Node *, Interp *,
    const Structure *);
extern void eval_init_form_pred(Node *, Interp *,
    const Structure *);
extern void eval_init_form_soe(Node *, Interp *,
    const Structure *);
extern void free_tc_caches(Node *);

/* interp.c */
extern Interp *new_interp(const Structure *struc);
extern Interp *free_remove_tup(Interp *, int);
extern Interp *dup_interp(Interp *);
extern int get_order(int);
extern Interp *fake_add_tup_to_interp(Interp *, int *, int);
extern Interp *add_tup_to_interp(Interp *, const int *, const int);

extern int get_interp_value(const char *, const Interp *);
extern int get_xi_interp_value(int, Interp *);
extern Interp *remove_xi_interp(int, Interp *);
extern int *get_xi_ival(int, Interp *);
extern void free_interp(Interp *);
extern Interp *fake_add_symb_to_interp(Interp *, const char *,
    const int);
extern void add_xi_interp(int, Interp *, int);
extern Interp *add_symb_to_interp(Interp *, const char *,
    const int);

/* tuple.c */
int *make_next_tuple(int *, const ReductionMap *, const int, const int,
    int *, const int, const int);
int tuple_cindex(const int *, const int, const int);
int *next_tuple(int *, const int, const int);
char *add_tup_to_output(char *, const int *, const int, const int);
int *cindex_to_tuple(int *, const int, const int, const int);

/* relation.c */
extern Relation *get_relation(const char *, const Interp *, const Structure *);
extern void fill_relcache(Relation *, Structure *);

/* constant.c */
extern Constant *get_constant(const char *, const Structure *);

/* reduc.c */
extern ReductionMap *make_rmap(const Reduction *, Structure *);

/* redfind.c */
extern int redfind(const BQuery *,const BQuery *,int,int,int,int);

/* usemace.c */
extern int usemace(Environment*, Node *, Vocabulary *, char *, int);
extern int get_mace_model_size(FILE *);
extern int print_mace(FILE *, Node *);
extern int t_print_mace(FILE *, Node *);
extern int make_mace_model(Structure *, FILE *);
extern int need_arithmetic(Node *);

/* util.c */
char *dupstr(const char *);
long de_pow(int x, short p);
List *free_list(List *);
string simplify(const string &);
string stringf(const char *fmt, ...);
string temporaryFileName();

#endif


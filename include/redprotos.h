// ISC LICENSE
#ifndef _RED_PROTOS_H
#define _RED_PROTOS_H

/* redprotos.h
 * Skip Jordan
 *
 * Prototypes for reduction-finding.
 * chj	 4/5/12		created
 */

#include "types.h"
#include "parse.h"
#include "redtypes.h"
#include "minisat.h"
#include <string>
using std::string;

#ifndef RED_MAXVARS
#define RED_MAXVARS 65536
#endif
#ifndef RED_EXMAXVARS
#define RED_EXMAXVARS 16493
#endif


/* getex.c */
Example *get_any_example(Environment*, int, const BQuery *);
Example *get_next_example(RedSearch *);
char *ex_p2_lfpf_eqform(RedSearch *, char *, int **);
char *ex_p2_lfpf_eqform_c(RedSearch *, char *);
char *ex_p2part_rec(RedSearch *, Node *, Interp *, RedBVarList *);
char *ex_p2part_equals(RedSearch *,Node *,Interp *);
int ex_is_crelvar(char *, const char *);
char *ex_make_rf_form_true(void);
char *ex_make_rf_form_false(void);
char *ex_p2make_teqi(RedSearch*,Node*, int);
RedTuple *ex_getconstuple(char *, RedTuple *);
char *ex_get_redcons_truevar(RedSearch *, const char *);
char *ex_p1rec_eq(RedSearch *, Node *, Interp *);
char *ex_p2part_pred(RedSearch *, Node *, Interp *);
char *ex_p2part_exists(RedSearch *, Node *, Interp *, RedBVarList *);
char *ex_p2part_forall(RedSearch *, Node *, Interp *, RedBVarList *);
char *ex_p1rec_tc(RedSearch *, Node *, Interp *);
RedBVarList *ex_addtcvar(TCDef *, Environment *, RedBVarList *, int *, minisat_Lit);
RedBVarList *ex_addlittolist(RedBVarList *, char *, minisat_Lit, Environment *);
char *ex_maketcdef(RedSearch *, TCDef *, RedBVarList *);
char *ex_make_part2_tcdef(RedSearch *, TCDef *, RedBVarList *);
char *ex_tcp1_dist1(RedSearch *, TCDef *, int *, RedBVarList *);
char *ex_p2_tcp1_dist1(RedSearch *, TCDef *, int *, RedBVarList *);
char *ex_tcp1_disti(RedSearch *, TCDef *, int *, int);
char *ex_tcp1_final(TCDef *, int *, int);
char *ex_printtcvar(int, int, int *, int *, int);
void ex_freetcd(TCDef *);
RedBVarList *ex_addtcvars(TCDef *, minisat_solver *, RedBVarList *, Environment *);
RedBVarList *ex_addtcvars_1(TCDef *, minisat_solver *, RedBVarList *, Environment *);
char *ex_p2_lfpf_pform(RedSearch *, char *, int **, int);
char *ex_p2_tc_join(RedSearch *, char *, const char *, const int *, int, const int *, char **, Interp *);
int ex_pf_tuparity(char *, int);
int *ex_nextpf_tuple(int *, int, int, int *, int *);
void ex_pf_tupconsmask(char *, int, int **, int, int *, char **, int *);
void ex_rtup_to_consmask(RedTuple *, int, char **, int *, int *);
char *ex_p2_lfpf_join(RedSearch *, char *, const char *, const int *, int, const int *, char **);
char *ex_rf_getrf_clause(RedSearch *, const char *, RedTuple *,int);
char *ex_rf_getrelform(RedSearch *, const char *, RedTuple *);
char *ex_p2_predform_getlitform(RedSearch *, int **, RedBVarList *, int);
char *ex_printtup(const RedTuple *, int);
void ex_printtupc(char *, int *, int);
char *ex_p1make_predform(char *, RedTuple *);
char *ex_p1make_teqi(RedSearch *, Node *, int);
char *ex_p1make_teqt(RedSearch *, Node *);
char *ex_p1rec_pred(RedSearch *, Node *, Interp *);
char *ex_p1rec_exists(RedSearch *, Node *,Interp *, RedBVarList *);
char *ex_p1rec_forall(RedSearch *, Node *,Interp *, RedBVarList *);
char *ex_p1rec(RedSearch *, Node *, Interp *, RedBVarList *ebv);
char *ex_p1part(RedSearch *, RedBVarList *);
char *ex_p2part(RedSearch *, RedBVarList *);
char *ex_p2part_tc(RedSearch *, Node *, Interp *);
char *ex_makeform(RedSearch *, RedBVarList *);
void ex_initsolver(minisat_solver *, Environment *env, RedSearch *);
Environment *ex_inithash(void);
void init_truefalse_rsearch(RedSearch *);
void red_freetruefalse(RedSearch *);
RedBVarList *ex_initbvarlist(RedSearch *, minisat_solver *, Environment *);
RedBVarList *ex_add_pred(minisat_solver *, Environment *, RedBVarList *, RelationSymbol *, int);
RedBVarList *ex_addtolist(RedBVarList *, char *, minisat_solver *, Environment *); extern Example *ex_getsatex(minisat_solver *,RedSearch *, Environment *);
int ex_getconsval(minisat_solver *, Environment *, const char *, int);
int ex_getrelval(minisat_solver *, Environment *, const char *, int, int *);
int ex_improve_example(minisat_solver *, RedSearch *, RedBVarList *);
void ex_forbidthisexample(RedSearch *, minisat_solver *, minisat_Lit, RedBVarList *);
int ex_randomize_example(minisat_solver *, RedSearch *, RedBVarList *);

/* limboole.c */
int limboole (char *, minisat_solver *, Environment *);

/* redfind.c */
/* the external interface, redfind(...), declared in protos.h */
RedHypot *get_next_hypothesis(RedSearch *, const Example *);
void red_freebvars(RedBVarList *);
void red_freecbvars(ConsBVars *);
void red_freersearch(RedSearch *);
void red_improve_hyp(RedSearch *);
RedBVarList *red_makebvars(RedSearch *, const BQuery *, const BQuery *, int, int, int);

#ifdef REDFIND_DEBUG
void red_printex(Example *, RedSearch *);
#endif

int red_checkp1p2(const BQuery *, const BQuery *);
int red_checkfreetcvars(Node *, Vocabulary *);
int red_checknestedtc(Node *, int);

RedBVarList *red_add_veqv(RedSearch *, RedBVarList *, const char *, int, int, int, int, int, int); 
RedBVarList *red_add_veqi(RedSearch *, RedBVarList *, const char *, int, int, int, int, int); 
RedBVarList *red_add_veqc(RedSearch *, RedBVarList *, const char *, int, ConsSymbol *, int, int, int);
RedBVarList *red_add_ceqc(RedSearch *, RedBVarList *, const char *, int, const char *, const char *, int); extern RedBVarList *red_add_pred(RedSearch *, RedBVarList *, const RelationSymbol *, int, const char *, int, int, ConsSymbol *, int);
char *red_printtup(RedSearch *, const RedTuple *, int);
int red_tupstrlen(const RedTuple *);
RedTuple *red_nexttuple(RedSearch *, RedTuple *, int, int, int, ConsSymbol *);
char *red_rf_getrelvar(RedSearch *rsearch, const char *relname, RedTuple *tup);
int red_abort_nomem(RedSearch *rsearch, RedBVarList *bvars);
minisat_solver *get_init_form(int, const BQuery *, const BQuery *, int, int, List *);
void free_rsearch_tc2(RedSearch *);
char *red_maketcform(RedSearch *, const Example *, TCDef *);
RedSearch *red_init_rsearch(const BQuery *, const BQuery *, int, int, int, int);
int set_init_form(RedSearch *);
RedHypot *red_getsatred(RedSearch *);
int free_ex(Example *);
int red_relform_relevantvar(RedBVarList *, const char *);
int red_minisat_truevar(RedSearch *, minisat_solver *, RedBVarList *, int);
void red_clausei_notempty(RedSearch *, char *, int);
int red_getclause(RedBVarList *);
RedRelForms *red_addrelform(RedRelForms *, RelationSymbol *rel, RedSearch *rsearch);
List *red_addconsform(List *, ConsSymbol *, RedSearch *rsearch);
Reduction *red_hypottored(Environment*, RedSearch *rsearch, RedHypot *hyp);
int red_isnegvar(const char *);
minisat_Lit red_getposLit(Environment *, const char *);
minisat_Lit red_getnegLit(Environment *, const char *);
int red_getclause(RedBVarList *);
char *red_relform_getlit(RedBVarList *, int);
char *red_getlit_eq(const char *, int);
char *red_getterm(char *, int);
char *red_pred_concatterm(char *, char *, int);
char *red_pred_concat(char *, char);
char *red_getlit_pred(char *, int);
int red_constant_forms(RedSearch *);
char *red_getconsbvarname(const char *, RedTuple *);
RedBVarList *red_makecbvars(RedSearch *, const BQuery *, const BQuery *, int, int);
RedTuple *red_nextconstuple(RedSearch*,RedTuple *, int, int, ConsSymbol *);
char *red_relform_realaddlit(char *, char *, int);
int red_cf_relevant(ConsSymbol *, RedBVarList *);
int red_cf_vallen(char *,int);
int red_freehyp(RedHypot *);
string red_rftodef(RedRelForms *);
char *red_cf_gettup(char *, int, int);
void red_printhyp(Environment*, RedHypot *);
RedTuple *red_getnext_compat_tuple(RedTuple *, int *, const Example *, int);
char *red_binform(RedSearch *, char *, char *, const char *);
char *red_disjunc(RedSearch *, char *, char *);
char *red_conjunc(RedSearch *, char *, char *);
int term_has_cons(Node *);
char *red_rf_pred_claused(RedSearch *rsearch, char *, int, ConsBVars *, ExRelation *);
ConsBVars *red_cons_addlit(ConsBVars *, const char *);
ConsBVars *red_cons_addtrue(ConsBVars *);
char *red_rf_pred_cbform(RedSearch*,char*,int,ConsBVars*);
RedTuple *red_rf_argstotup(RedSearch *rsearch, Node *,int,Vocabulary *,int, Interp *, const char *);
void red_freertup(RedTuple *);
char *make_pf_exrel(RedSearch *rsearch, const Example *ex, ExRelation *er);
int red_addcons_pred(RedSearch *, const Example *, const ExRelation *, char **, int);
int red_addincons_pred(RedSearch *, const Example *, const ExRelation *, char **, int);
int red_getnumconslits(RedSearch *, ExRelation *);
ConsBVars *red_getinconslits(RedSearch *, const Example *, ExRelation *);
ConsBVars *red_getconslits(RedSearch *, const Example *, ExRelation *);
char *make_predform(RedSearch*,const Example*);
List *red_free_exrel(Environment *env, List *);
void red_add_exrel(RedSearch *,char *,char *,RedTuple *);
string red_hypottocmd(RedSearch *, RedHypot *, string);
char *red_cf_getconsform(ConsSymbol *,RedBVarList *,int);
RedRelForms *red_relform_addlit(RedRelForms *, RedBVarList *, RelationSymbol *, int, int, int);
char *make_rf_tc(RedSearch *, Node *, Interp *, const Example *);
char *red_rf_addconsclause(char *, const char *, RedTuple *);
char *make_rf_teqt(RedSearch *, Node *);
RedBVarList *red_addtolist(RedBVarList *, char *, minisat_solver *, Environment *);
RedBVarList *red_addconsbvar(RedSearch *, RedBVarList *, minisat_solver *, Environment *, char *, RedTuple *);
char *red_rf_pred_inconsclause(RedSearch *,int, ConsBVars *, ExRelation *);
char *make_rf_exists(RedSearch *, const Example *, Node *, Interp *);
char *make_rf_forall(RedSearch *, const Example *, Node *, Interp *);
char *make_redform(RedSearch *, const Example *);
char *make_rf_rec(RedSearch *, const Example *, Node *, Interp *);
char *make_rf_pred(RedSearch *, Node *, Interp *);
minisat_Lit red_getconsbvarlit(Environment *, char *, RedTuple *);
char *make_rf_eq(RedSearch *, const Example *, Node *, Interp *);
char *make_rf_teqi(RedSearch *, const Example *, Node *, int);
char *red_form_addconstuple(char *, RedTuple *, char *, char *);
char *red_p2_lfpf_join(RedSearch *, char *, const char *, const int *, int, const int *, char **, const Example *, Interp *);

#endif


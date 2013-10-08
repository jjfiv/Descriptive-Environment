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
extern Example *get_any_example(int, const BQuery *);
extern Example *get_next_example(RedSearch *);
extern char *ex_p2_lfpf_eqform(RedSearch *, char *, int **);
extern char *ex_p2_lfpf_eqform_c(RedSearch *, char *);
extern char *ex_p2part_rec(RedSearch *, Node *, Interp *, RedBVarList *);
extern char *ex_p2part_equals(RedSearch *,Node *,Interp *);
extern int ex_is_crelvar(char *, const char *);
extern char *ex_make_rf_form_true(void);
extern char *ex_make_rf_form_false(void);
extern char *ex_p2make_teqi(RedSearch*,Node*, int);
extern RedTuple *ex_getconstuple(char *, RedTuple *);
extern char *ex_get_redcons_truevar(RedSearch *, const char *);
extern char *ex_p1rec_eq(RedSearch *, Node *, Interp *);
extern char *ex_p2part_pred(RedSearch *, Node *, Interp *);
extern char *ex_p2part_exists(RedSearch *, Node *, Interp *, RedBVarList *);
extern char *ex_p2part_forall(RedSearch *, Node *, Interp *, RedBVarList *);
extern char *ex_p1rec_tc(RedSearch *, Node *, Interp *);
extern RedBVarList *ex_addtcvar(TCDef *, Environment *, RedBVarList *, int *, minisat_Lit);
extern RedBVarList *ex_addlittolist(RedBVarList *, char *, minisat_Lit, Environment *);
extern char *ex_maketcdef(RedSearch *, TCDef *, RedBVarList *);
extern char *ex_make_part2_tcdef(RedSearch *, TCDef *, RedBVarList *);
extern char *ex_tcp1_dist1(RedSearch *, TCDef *, int *, RedBVarList *);
extern char *ex_p2_tcp1_dist1(RedSearch *, TCDef *, int *, RedBVarList *);
extern char *ex_tcp1_disti(RedSearch *, TCDef *, int *, int);
extern char *ex_tcp1_final(TCDef *, int *, int);
extern char *ex_printtcvar(int, int, int *, int *, int);
extern void ex_freetcd(TCDef *);
extern RedBVarList *ex_addtcvars(TCDef *, minisat_solver *, RedBVarList *, Environment *);
extern RedBVarList *ex_addtcvars_1(TCDef *, minisat_solver *, RedBVarList *, Environment *);
extern char *ex_p2_lfpf_pform(RedSearch *, char *, int **, int);
extern char *ex_p2_tc_join(RedSearch *, char *, const char *, const int *, int, const int *, char **, Interp *);
extern int ex_pf_tuparity(char *, int);
extern int *ex_nextpf_tuple(int *, int, int, int *, int *);
extern void ex_pf_tupconsmask(char *, int, int **, int, int *, char **, int *);
extern void ex_rtup_to_consmask(RedTuple *, int, char **, int *, int *);
extern char *ex_p2_lfpf_join(RedSearch *, char *, const char *, const int *, int, const int *, char **);
extern char *ex_rf_getrf_clause(RedSearch *, char *, RedTuple *,int);
extern char *ex_rf_getrelform(RedSearch *, char *, RedTuple *);
extern char *ex_p2_predform_getlitform(RedSearch *, int **, RedBVarList *, int);
extern char *ex_printtup(const RedTuple *, int);
extern void ex_printtupc(char *, int *, int);
extern char *ex_p1make_predform(char *, RedTuple *);
extern char *ex_p1make_teqi(RedSearch *, Node *, int);
extern char *ex_p1make_teqt(RedSearch *, Node *);
extern char *ex_p1rec_pred(RedSearch *, Node *, Interp *);
extern char *ex_p1rec_exists(RedSearch *, Node *,Interp *, RedBVarList *);
extern char *ex_p1rec_forall(RedSearch *, Node *,Interp *, RedBVarList *);
extern char *ex_p1rec(RedSearch *, Node *, Interp *, RedBVarList *ebv);
extern char *ex_p1part(RedSearch *, RedBVarList *);
extern char *ex_p2part(RedSearch *, RedBVarList *);
extern char *ex_p2part_tc(RedSearch *, Node *, Interp *);
extern char *ex_makeform(RedSearch *, RedBVarList *);
extern void ex_initsolver(minisat_solver *, Environment *env, RedSearch *);
extern Environment *ex_inithash(void);
extern void init_truefalse_rsearch(RedSearch *);
extern void red_freetruefalse(RedSearch *);
extern RedBVarList *ex_initbvarlist(RedSearch *, minisat_solver *, Environment *);
extern RedBVarList *ex_add_pred(minisat_solver *, Environment *, RedBVarList *, RelationSymbol *, int);
extern RedBVarList *ex_addtolist(RedBVarList *, char *, minisat_solver *, Environment *); extern Example *ex_getsatex(minisat_solver *,RedSearch *, Environment *);
extern int ex_getconsval(minisat_solver *, Environment *, const char *, int);
extern int ex_getrelval(minisat_solver *, Environment *, const char *, int, int *);
extern int ex_improve_example(minisat_solver *, RedSearch *, RedBVarList *);
extern void ex_forbidthisexample(RedSearch *, minisat_solver *, minisat_Lit, RedBVarList *);
extern int ex_randomize_example(minisat_solver *, RedSearch *, RedBVarList *);

/* limboole.c */
extern int limboole (char *, minisat_solver *, Environment *);

/* redfind.c */
/* the external interface, redfind(...), declared in protos.h */
/* extern int redfind(const BQuery *p1, const BQuery *p2,
   int k, int c, int n); */
extern RedHypot *get_next_hypothesis(RedSearch *, const Example *);
extern void red_freebvars(RedBVarList *);
extern void red_freecbvars(ConsBVars *);
extern void red_freersearch(RedSearch *);
extern void red_improve_hyp(RedSearch *);
extern RedBVarList *red_makebvars(RedSearch *, const BQuery *, const BQuery *, int, int, int);

#ifdef REDFIND_DEBUG
extern void red_printex(Example *, RedSearch *);
#endif

extern int red_checkp1p2(const BQuery *, const BQuery *);
extern int red_checkfreetcvars(Node *, Vocabulary *);
extern int red_checknestedtc(Node *, int);

extern RedBVarList *red_add_veqv(RedSearch *, RedBVarList *, const char *, int, int, int, int, int, int); 
extern RedBVarList *red_add_veqi(RedSearch *, RedBVarList *, const char *, int, int, int, int, int); 
extern RedBVarList *red_add_veqc(RedSearch *, RedBVarList *, const char *, int, ConsSymbol *, int, int, int);
extern RedBVarList *red_add_ceqc(RedSearch *, RedBVarList *, const char *, int, const char *, const char *, int); extern RedBVarList *red_add_pred(RedSearch *, RedBVarList *, const RelationSymbol *, int, const char *, int, int, ConsSymbol *, int);
extern char *red_printtup(RedSearch *, const RedTuple *, int);
extern int red_tupstrlen(const RedTuple *);
extern RedTuple *red_nexttuple(RedSearch *, RedTuple *, int, int, int, ConsSymbol *);
extern char *red_rf_getrelvar(RedSearch *rsearch, const char *relname, RedTuple *tup);
extern int red_abort_nomem(RedSearch *rsearch, RedBVarList *bvars);
extern minisat_solver *get_init_form(int, const BQuery *, const BQuery *, int, int, List *);
extern void free_rsearch_tc2(RedSearch *);
extern char *red_maketcform(RedSearch *, const Example *, TCDef *);
extern RedSearch *red_init_rsearch(const BQuery *, const BQuery *, int, int, int, int);
extern int set_init_form(RedSearch *);
extern RedHypot *red_getsatred(RedSearch *);
extern int free_ex(Example *);
extern int red_relform_relevantvar(RedBVarList *, const char *);
extern int red_minisat_truevar(RedSearch *, minisat_solver *, RedBVarList *, int);
extern void red_clausei_notempty(RedSearch *, char *, int);
extern int red_getclause(RedBVarList *);
extern RedRelForms *red_addrelform(RedRelForms *, RelationSymbol *rel, RedSearch *rsearch);
extern List *red_addconsform(List *, ConsSymbol *, RedSearch *rsearch);
extern Reduction *red_hypottored(RedSearch *rsearch, RedHypot *hyp);
extern int red_isnegvar(const char *);
extern minisat_Lit red_getposLit(Environment *, const char *);
extern minisat_Lit red_getnegLit(Environment *, const char *);
extern int red_getclause(RedBVarList *);
extern char *red_relform_getlit(RedBVarList *, int);
extern char *red_getlit_eq(const char *, int);
extern char *red_getterm(char *, int);
extern char *red_pred_concatterm(char *, char *, int);
extern char *red_pred_concat(char *, char);
extern char *red_getlit_pred(char *, int);
extern int red_constant_forms(RedSearch *);
extern char *red_getconsbvarname(const char *, RedTuple *);
extern RedBVarList *red_makecbvars(RedSearch *, const BQuery *, const BQuery *, int, int);
extern RedTuple *red_nextconstuple(RedSearch*,RedTuple *, int, int, ConsSymbol *);
extern char *red_relform_realaddlit(char *, char *, int);
extern int red_cf_relevant(ConsSymbol *, RedBVarList *);
extern int red_cf_vallen(char *,int);
extern int red_freehyp(RedHypot *);
extern char *red_rftodef(RedRelForms *);
extern char *red_cf_gettup(char *, int, int);
extern void red_printhyp(RedHypot *);
extern RedTuple *red_getnext_compat_tuple(RedTuple *, int *, const Example *, int);
extern char *red_binform(RedSearch *, char *, char *, const char *);
extern char *red_disjunc(RedSearch *, char *, char *);
extern char *red_conjunc(RedSearch *, char *, char *);
extern int term_has_cons(Node *);
extern char *red_rf_pred_claused(RedSearch *rsearch, char *, int, ConsBVars *, ExRelation *);
extern ConsBVars *red_cons_addlit(ConsBVars *, const char *);
extern ConsBVars *red_cons_addtrue(ConsBVars *);
extern char *red_rf_pred_cbform(RedSearch*,char*,int,ConsBVars*);
extern RedTuple *red_rf_argstotup(RedSearch *rsearch, Node *,int,Vocabulary *,int, Interp *, char *);
extern void red_freertup(RedTuple *);
extern char *make_pf_exrel(RedSearch *rsearch, const Example *ex, ExRelation *er);
extern int red_addcons_pred(RedSearch *, const Example *, const ExRelation *, char **, int);
extern int red_addincons_pred(RedSearch *, const Example *, const ExRelation *, char **, int);
extern int red_getnumconslits(RedSearch *, ExRelation *);
extern ConsBVars *red_getinconslits(RedSearch *, const Example *, ExRelation *);
extern ConsBVars *red_getconslits(RedSearch *, const Example *, ExRelation *);
extern char *make_predform(RedSearch*,const Example*);
extern List *red_free_exrel(Environment *env, List *);
extern void red_add_exrel(RedSearch *,char *,char *,RedTuple *);
extern string red_hypottocmd(RedSearch *, RedHypot *, string);
extern char *red_cf_getconsform(ConsSymbol *,RedBVarList *,int);
extern RedRelForms *red_relform_addlit(RedRelForms *, RedBVarList *, RelationSymbol *, int, int, int);
extern char *make_rf_tc(RedSearch *, Node *, Interp *, const Example *);
extern char *red_rf_addconsclause(char *, const char *, RedTuple *);
extern char *make_rf_teqt(RedSearch *, Node *);
extern RedBVarList *red_addtolist(RedBVarList *, char *, minisat_solver *, Environment *);
extern RedBVarList *red_addconsbvar(RedSearch *, RedBVarList *, minisat_solver *, Environment *, char *, RedTuple *);
extern char *red_rf_pred_inconsclause(RedSearch *,int, ConsBVars *, ExRelation *);
extern char *make_rf_exists(RedSearch *, const Example *, Node *, Interp *);
extern char *make_rf_forall(RedSearch *, const Example *, Node *, Interp *);
extern char *make_redform(RedSearch *, const Example *);
extern char *make_rf_rec(RedSearch *, const Example *, Node *, Interp *);
extern char *make_rf_pred(RedSearch *, Node *, Interp *);
extern minisat_Lit red_getconsbvarlit(Environment *, char *, RedTuple *);
extern char *make_rf_eq(RedSearch *, const Example *, Node *, Interp *);
extern char *make_rf_teqi(RedSearch *, const Example *, Node *, int);
extern char *red_form_addconstuple(char *, RedTuple *, char *, char *);
extern char *red_p2_lfpf_join(RedSearch *, char *, const char *, const int *, int, const int *, char **, const Example *, Interp *);

#endif


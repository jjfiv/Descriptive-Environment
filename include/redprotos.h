#ifndef _RED_PROTOS_H
#define _RED_PROTOS_H

/*
   Copyright (c) 2012, Charles Jordan <skip@alumni.umass.edu>

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

/* redprotos.h
 * Skip Jordan
 *
 * Prototypes for reduction-finding.
 * chj	 4/5/12		created
 */

#include "types.h"
#include "redtypes.h"
#include "minisat.h"

/* getex.c */
extern struct example *get_any_example(int, const struct bquery *);
extern struct example *get_next_example(struct redsearch *);
extern char *ex_p2_lfpf_eqform(struct redsearch *, char *, int **);
extern char *ex_p2_lfpf_eqform_c(struct redsearch *, char *);
extern char *ex_p2part_rec(struct redsearch *, struct node *,
    struct interp *, struct red_bvarlist *);
extern char *ex_p2part_equals(struct redsearch *,struct node *,struct interp *);
extern int ex_is_crelvar(char *, const char *);
extern char *ex_make_rf_form_true(void);
extern char *ex_make_rf_form_false(void);
extern char *ex_p2make_teqi(struct redsearch*,struct node*, int);
extern struct red_tuple *ex_getconstuple(char *, struct red_tuple *);
extern char *ex_get_redcons_truevar(struct redsearch *, const char *);
extern char *ex_p1rec_eq(struct redsearch *, struct node *, struct interp *);
extern char *ex_p2part_pred(struct redsearch *, struct node *, struct interp *);
extern char *ex_p2part_exists(struct redsearch *, struct node *,
    struct interp *, struct red_bvarlist *);
extern char *ex_p2part_forall(struct redsearch *, struct node *,
    struct interp *, struct red_bvarlist *);
extern char *ex_p1rec_tc(struct redsearch *, struct node *, struct interp *);
extern struct red_bvarlist *ex_addtcvar(struct tc_def *, struct env *,
    struct red_bvarlist *, int *, minisat_Lit);
extern struct red_bvarlist *ex_addlittolist(struct red_bvarlist *,
    char *, minisat_Lit, struct env *);
extern char *ex_maketcdef(struct redsearch *, struct tc_def *, 
    struct red_bvarlist *);
extern char *ex_make_part2_tcdef(struct redsearch *, struct tc_def *,
    struct red_bvarlist *);
extern char *ex_tcp1_dist1(struct redsearch *, struct tc_def *, int *,
    struct red_bvarlist *);
extern char *ex_p2_tcp1_dist1(struct redsearch *, struct tc_def *, int *, 
    struct red_bvarlist *);
extern char *ex_tcp1_disti(struct redsearch *, struct tc_def *, int *, int);
extern char *ex_tcp1_final(struct tc_def *, int *, int);
extern char *ex_printtcvar(int, int, int *, int *, int);
extern void ex_freetcd(struct tc_def *);
extern struct red_bvarlist *ex_addtcvars(struct tc_def *,
    struct minisat_solver_t *,
    struct red_bvarlist *, struct env *);
extern struct red_bvarlist *ex_addtcvars_1(struct tc_def *,
    struct minisat_solver_t *,
    struct red_bvarlist *, struct env *);
extern char *ex_p2_lfpf_pform(struct redsearch *, char *, int **, int);
extern char *ex_p2_tc_join(struct redsearch *, char *, const char *, 
    const int *, int, const int *, char **,
    struct interp *);
extern int ex_pf_tuparity(char *, int);
extern int *ex_nextpf_tuple(int *, int, int, int *, int *);
extern void ex_pf_tupconsmask(char *, int, int **, int, int *, char **, int *);
extern void ex_rtup_to_consmask(struct red_tuple *, int, char **, int *, int *);
extern char *ex_p2_lfpf_join(struct redsearch *, char *, const char *, 
    const int *, int, const int *, char **);
extern char *ex_rf_getrf_clause(struct redsearch *, char *,
    struct red_tuple *,int);
extern char *ex_rf_getrelform(struct redsearch *, char *, struct red_tuple *);
extern char *ex_p2_predform_getlitform(struct redsearch *, int **, 
    struct red_bvarlist *, int);
extern char *ex_printtup(const struct red_tuple *, int);
extern void ex_printtupc(char *, int *, int);
extern char *ex_p1make_predform(/*struct redsearch *,*/
    char *, struct red_tuple *);
extern char *ex_p1make_teqi(struct redsearch *, struct node *, int);
extern char *ex_p1make_teqt(struct redsearch *, struct node *);
extern char *ex_p1rec_pred(struct redsearch *, struct node *, struct interp *);
extern char *ex_p1rec_exists(struct redsearch *, struct node *,struct interp *,
    struct red_bvarlist *);
extern char *ex_p1rec_forall(struct redsearch *, struct node *,struct interp *,
    struct red_bvarlist *);
extern char *ex_p1rec(struct redsearch *, struct node *, struct interp *,
    struct red_bvarlist *ebv);
extern char *ex_p1part(struct redsearch *, struct red_bvarlist *);
extern char *ex_p2part(struct redsearch *, struct red_bvarlist *);
extern char *ex_p2part_tc(struct redsearch *, struct node *,
    struct interp *);
extern char *ex_makeform(struct redsearch *, struct red_bvarlist *);
extern void ex_initsolver(struct minisat_solver_t *, struct env *env,
    struct redsearch *);
extern struct env *ex_inithash(void);
extern void init_truefalse_rsearch(struct redsearch *);
extern void red_freetruefalse(struct redsearch *);
extern struct red_bvarlist *ex_initbvarlist(struct redsearch *, 
    struct minisat_solver_t *,
    struct env *);
extern struct red_bvarlist *ex_add_pred(struct minisat_solver_t *, struct env *,
    struct red_bvarlist *,
    struct rel_symbol *, int);
extern struct red_bvarlist *ex_addtolist(struct red_bvarlist *, char *,
    struct minisat_solver_t *, struct env *);
extern struct example *ex_getsatex(struct minisat_solver_t *,struct redsearch *,
    struct env *);
extern int ex_getconsval(struct minisat_solver_t *, struct env *, const char *, 
    int);
extern int ex_getrelval(struct minisat_solver_t *, struct env *, const char *, 
    int, int *);
extern int ex_improve_example(struct minisat_solver_t *, struct redsearch *,
    struct red_bvarlist *);
extern void ex_forbidthisexample(struct redsearch *, struct minisat_solver_t *, minisat_Lit, struct red_bvarlist *);
extern int ex_randomize_example(struct minisat_solver_t *, struct redsearch *, struct red_bvarlist *);

/* limboole.c */
extern int limboole (char *, struct minisat_solver_t *, struct env *);

/* redfind.c */
/* the external interface, redfind(...), declared in protos.h */
/* extern int redfind(const struct bquery *p1, const struct bquery *p2,
   int k, int c, int n); */
extern struct red_hypot *get_next_hypothesis(struct redsearch *,
    const struct example *);
extern void red_freebvars(struct red_bvarlist *);
extern void red_freecbvars(struct cons_bvars *);
extern void red_freersearch(struct redsearch *);
extern void red_improve_hyp(struct redsearch *);
extern struct red_bvarlist 
*red_makebvars(struct redsearch *, const struct bquery *, 
    const struct bquery *, int, int, int);
#ifdef REDFIND_DEBUG
extern void red_printex(struct example *, struct redsearch *);
#endif
extern int red_checkp1p2(const struct bquery *, const struct bquery *);
extern int red_checkfreetcvars(struct node *, struct vocab *);
extern int red_checknestedtc(struct node *, int);

extern struct red_bvarlist *red_add_veqv(struct redsearch *, struct red_bvarlist *, const char *, int, int, int, int, int, int); 

extern struct red_bvarlist *red_add_veqi(struct redsearch *, struct red_bvarlist *, const char *, int, int, int, int, int); 

extern struct red_bvarlist *red_add_veqc(struct redsearch *, struct red_bvarlist *, const char *, int, struct cons_symbol *, int, int, int);

extern struct red_bvarlist *red_add_ceqc(struct redsearch *, struct red_bvarlist *, const char *, int, const char *, const char *, int); extern struct red_bvarlist *red_add_pred(struct redsearch *, struct red_bvarlist *, const struct rel_symbol *, int, const char *, int, int, struct cons_symbol *, int);
extern char *red_printtup(struct redsearch *, const struct red_tuple *, int);
extern int red_tupstrlen(const struct red_tuple *);
extern struct red_tuple *red_nexttuple(struct redsearch *, struct red_tuple *, 
    int, int, int, struct cons_symbol *);
extern char *red_rf_getrelvar(struct redsearch *rsearch, const char *relname, struct red_tuple *tup);
extern int red_abort_nomem(struct redsearch *rsearch, struct red_bvarlist *bvars);
extern struct minisat_solver_t *get_init_form(int, const struct bquery *, const struct bquery *, int, int, struct list *);
extern void free_rsearch_tc2(struct redsearch *);
extern char *red_maketcform(struct redsearch *, const struct example *, struct tc_def *);
extern struct redsearch *red_init_rsearch(const struct bquery *, const struct bquery *, int, int, int, int);
extern int set_init_form(struct redsearch *);
extern struct red_hypot *red_getsatred(struct redsearch *);
extern int free_ex(struct example *);
extern int red_relform_relevantvar(struct red_bvarlist *, const char *);
extern int red_minisat_truevar(struct redsearch *, struct minisat_solver_t *, struct red_bvarlist *, int);
extern void red_clausei_notempty(struct redsearch *, char *, int);
extern int red_getclause(struct red_bvarlist *);
extern struct red_relforms *red_addrelform(struct red_relforms *, struct rel_symbol *rel, struct redsearch *rsearch);
extern struct list *red_addconsform(struct list *, struct cons_symbol *, struct redsearch *rsearch);
extern struct reduction *red_hypottored(struct redsearch *rsearch, struct red_hypot *hyp);
extern int red_isnegvar(const char *);
extern minisat_Lit red_getposLit(struct env *, const char *);
extern minisat_Lit red_getnegLit(struct env *, const char *);
extern int red_getclause(struct red_bvarlist *);
extern char *red_relform_getlit(struct red_bvarlist *, int);
extern char *red_getlit_eq(const char *, int);
extern char *red_getterm(char *, int);
extern char *red_pred_concatterm(char *, char *, int);
extern char *red_pred_concat(char *, char);
extern char *red_getlit_pred(char *, int);
extern int red_constant_forms(struct redsearch *);
extern char *red_getconsbvarname(const char *, struct red_tuple *);
extern struct red_bvarlist *red_makecbvars(struct redsearch *,
    const struct bquery *, const struct bquery *, int, int);
extern struct red_tuple *red_nextconstuple(struct redsearch*,struct red_tuple *,
    int, int, struct cons_symbol *);
extern char *red_relform_realaddlit(char *, char *, int);
extern int red_cf_relevant(struct cons_symbol *, struct red_bvarlist *);
extern int red_cf_vallen(char *,int);
extern int red_freehyp(struct red_hypot *);
extern char *red_rftodef(struct red_relforms *);
extern char *red_cf_gettup(char *, int, int);
extern void red_printhyp(struct red_hypot *);
extern struct red_tuple *red_getnext_compat_tuple(struct red_tuple *, int *,
    const struct example *, int);
extern char *red_binform(struct redsearch *, char *, char *, const char *);
extern char *red_disjunc(struct redsearch *, char *, char *);
extern char *red_conjunc(struct redsearch *, char *, char *);
extern int term_has_cons(struct node *);
extern char *red_rf_pred_claused(struct redsearch *rsearch, char *, int,
    struct cons_bvars *, struct ex_rel *);
extern struct cons_bvars *red_cons_addlit(struct cons_bvars *, const char *);
extern struct cons_bvars *red_cons_addtrue(struct cons_bvars *);
extern char *red_rf_pred_cbform(struct redsearch*,char*,int,struct cons_bvars*);
extern struct red_tuple *red_rf_argstotup(struct redsearch *rsearch,
    struct node *,int,struct vocab *,int,
    struct interp *, char *);
extern void red_freertup(struct red_tuple *);
extern char *make_pf_exrel(struct redsearch *rsearch, const struct example *ex,
    struct ex_rel *er);
extern int red_addcons_pred(struct redsearch *, const struct example *,
    const struct ex_rel *, char **, int);
extern int red_addincons_pred(struct redsearch *, const struct example *,
    const struct ex_rel *, char **, int);
extern int red_getnumconslits(struct redsearch *, struct ex_rel *);
extern struct cons_bvars *red_getinconslits(struct redsearch *,
    const struct example *, struct ex_rel *);
extern struct cons_bvars *red_getconslits(struct redsearch *, 
    const struct example *, struct ex_rel *);
extern char *make_predform(struct redsearch*,const struct example*);
extern struct list *red_free_exrel(struct env *env, struct list *);
extern void red_add_exrel(struct redsearch *,char *,char *,struct red_tuple *);
extern char *red_hypottocmd(struct redsearch *, struct red_hypot *);
extern char *red_cf_getconsform(struct cons_symbol *,struct red_bvarlist *,int);
extern struct red_relforms *red_relform_addlit(struct red_relforms *,
    struct red_bvarlist *,
    struct rel_symbol *, int, int, int);
extern char *make_rf_tc(struct redsearch *, struct node *, struct interp *,
    const struct example *);
extern char *red_rf_addconsclause(char *, const char *, struct red_tuple *);
extern char *make_rf_teqt(struct redsearch *, struct node *);
extern struct red_bvarlist *red_addtolist(struct red_bvarlist *, char *,
    struct minisat_solver_t *, struct env *);
extern struct red_bvarlist *red_addconsbvar(struct redsearch *,
    struct red_bvarlist *,
    struct minisat_solver_t *,
    struct env *, char *, struct red_tuple *);
extern char *red_rf_pred_inconsclause(struct redsearch *,int,
    struct cons_bvars *, struct ex_rel *);
extern char *make_rf_exists(struct redsearch *, const struct example *,
    struct node *, struct interp *);
extern char *make_rf_forall(struct redsearch *, const struct example *,
    struct node *, struct interp *);
extern char *make_redform(struct redsearch *, const struct example *);
extern char *make_rf_rec(struct redsearch *, const struct example *,
    struct node *, struct interp *);
extern char *make_rf_pred(struct redsearch *,
    struct node *, struct interp *);
extern minisat_Lit red_getconsbvarlit(struct env *, char *, struct red_tuple *);
extern char *make_rf_eq(struct redsearch *, const struct example *,
    struct node *, struct interp *);
extern char *make_rf_teqi(struct redsearch *, const struct example *,
    struct node *, int);
extern char *red_form_addconstuple(char *, struct red_tuple *, char *, char *);
extern char *red_p2_lfpf_join(struct redsearch *, char *, const char *,
    const int *, int, const int *, char **,
    const struct example *, struct interp *);

#endif


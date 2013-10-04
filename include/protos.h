/*
Copyright (c) 2013, John Foley <jfoley@cs.umass.edu>
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

#include "types.h"
#include <stdio.h> /* for FILE * */
#include "soe_parse.h"

#ifdef DEBUG
#define err printf
#else
#define err(a, b)
#endif

int yyparse(void);

#define numdigits(i) (i==0?1:floor(log10(i))+1)

/* cmd.c */
extern int command_loop(void);
extern int do_cmd(struct node *);
extern int do_assign_command(struct node *);
extern int do_vocab_assign(struct node *);
extern int do_struc_assign(struct node *);
extern int do_reduc_assign(struct node *);
extern int do_expred_command(struct node *);
extern int do_excons_command(struct node *);
extern int do_listtuple_command(struct node *);
extern int do_apply_assign(struct node *);
extern int do_bquery_assign(struct node *);
extern int do_abquery_command(struct node *);
extern int do_minisat_query(const struct structure *);
extern int do_threecolorsat_query(const struct structure *);
extern int do_minisat2_query(const struct structure *);
extern int do_threecolor_sat2_query(const struct structure *);
extern int do_mace(struct node *);
extern int do_fd(struct node *);
extern int do_redfind(struct node *);

/* file.c */
extern int do_draw_command(struct node *);
extern int do_load(struct node *);
extern int do_save_command(struct node *);
extern int save_struc(struct structure *, FILE *);
extern int save_tuple_line(struct relation *, int *, FILE *);
extern int save_voc_inner(struct vocab *, FILE *);
extern char *loadstring_getdec(char *, int, struct vocab *);
extern int do_loadassign(struct node *);
extern void loadstring_convert(struct structure *, int, char *);

/* help.c */
extern int do_help(const char *);

/* init.c */
extern int init_env(void);

/* check.c */
extern char *free_var(struct node *, struct vocab *);
extern char *free_var_fast(struct node *, struct interp *, 
			   const struct structure *);
extern char *free_var_fastq(struct node *, struct interp *, 
			    const struct structure *);
extern char *free_var_fasttc(struct node *, struct interp *, 
			     const struct structure *);
extern char *free_var_fastsoe(struct node *, struct interp *, 
			     const struct structure *);
extern struct list *d_free_var(struct node *, struct vocab *);
extern struct list *join_lists(struct list *, struct list *);
extern struct list *remove_args(struct list *, struct node *);
extern struct list *remove_tcargs(struct list *, struct node *);
extern struct list *list_remove(struct list *, char *);

/* eval.c */
extern int eval(struct node *, struct interp *, const struct structure *);
extern int eval_rec(struct node *, struct interp *, const struct structure *);
extern int teval(struct node *, struct interp *, const struct structure *);
extern int eval_pred(struct node *, struct interp *, const struct structure *);
extern int eval_exists(struct node *, struct interp *, const struct structure *);
extern int eval_forall(struct node *, struct interp *, const struct structure *);
extern int eval_tc(struct node *, struct interp *, const struct structure *);
extern int eval_ifp(struct node *, struct interp *, const struct structure *);
extern int eval_soe(struct node *, struct interp *, const struct structure *);
extern void eval_init_form(struct node *, struct interp *, 
			   const struct structure *);
extern void eval_init_form_q(struct node *, struct interp *,
			     const struct structure *);
extern void eval_init_form_tc(struct node *, struct interp *,
			      const struct structure *);
extern void eval_init_form_pred(struct node *, struct interp *,
				const struct structure *);
extern void eval_init_form_soe(struct node *, struct interp *,
			       const struct structure *);
extern void free_tc_caches(struct node *);

/* interp.c */
extern struct interp *new_interp(const struct structure *struc);
extern struct interp *free_remove_tup(struct interp *, int);
extern struct interp *dup_interp(struct interp *);
extern int get_order(int);
extern struct interp *fake_add_tup_to_interp(struct interp *, int *, int);
extern struct interp *add_tup_to_interp(struct interp *, const int *, const int);
/* trpow was a different implementation, but de_pow is equivalent and faster */
#define trpow(a,b) de_pow(a,b)
extern int get_interp_value(const char *, const struct interp *);
extern int get_xi_interp_value(int, struct interp *);
extern struct interp *remove_xi_interp(int, struct interp *);
extern int *get_xi_ival(int, struct interp *);
extern void free_interp(struct interp *);
extern struct interp *fake_add_symb_to_interp(struct interp *, const char *,
					      const int);
extern void add_xi_interp(int, struct interp *, int);
extern struct interp *add_symb_to_interp(struct interp *, const char *,
					 const int);

/* tuple.c */
int *make_next_tuple(int *, const struct reduc_map *, const int, const int,
		     int *, const int, const int);
int tuple_cindex(const int *, const int, const int);
int *next_tuple(int *, const int, const int);
char *add_tup_to_output(char *, const int *, const int, const int);
int *cindex_to_tuple(int *, const int, const int, const int);

/* relation.c */
extern struct relation *get_relation(const char *, const struct interp *, const struct structure *);
extern void fill_relcache(struct relation *, struct structure *);

/* constant.c */
extern struct constant *get_constant(const char *, const struct structure *);

/* reduc.c */
extern struct reduc_map *make_rmap(const struct reduction *, struct structure *);

/* y.tab.c */
extern int yyparse();

/* lex.yy.c */
extern void *yy_scan_string(const char *);
extern void yy_delete_buffer(void *);

/* redfind.c */
extern int redfind(const struct bquery *,const struct bquery *,int,int,int,int);

/* usemace.c */
extern int usemace(struct node *, struct vocab *, char *, int);
extern int get_mace_model_size(FILE *);
extern int print_mace(FILE *, struct node *);
extern int t_print_mace(FILE *, struct node *);
extern int make_mace_model(struct structure *, FILE *);
extern int need_arithmetic(struct node *);

/* util.c */
extern char *dupstr(const char *);
extern long de_pow(int x, short p);
extern struct list *free_list(struct list *);

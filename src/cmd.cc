// ISC LICENSE
#include "parse.h"
#include "types.h"
#include "protos.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
using namespace std;

extern "C" {
#define DE_MINISAT /* omit unused bits of MiniSat to avoid warnings */
#include "solver.h"
#include "minisat.h"
}

#include "lexer.hh"

void debug_lexer(string cmd) {
  for(auto tok : Lexer::lexString(cmd)) {
    cout << tok.toString() << " ";
  }
  cout << "\n";
}

void command_loop(void) {
  while(cin) {
    string cmd;
    // print prompt & flush
    cout << "> "; cout.flush();

    if(!getline(cin, cmd)) break;
    cmd = simplify(cmd);

    debug_lexer(cmd);

    if (cmd == "") continue;
    if (cmd == "quit") break;
    if (cmd == "help") { do_help(); continue; }

    runCommand(cur_env, cmd);
  }
}

/* Executes the command pointed to by command.
*/
int do_cmd(Environment* env, Node *command) {
  switch (command->label) {
    case ASSIGN:    return do_assign_command(env, command);
    case EXCONS:    return do_excons_command(env, command);
    case EXPRED:    return do_expred_command(env, command);
    case EXPREDALL: return do_listtuple_command(env, command);
    case ABQUERY:   return do_abquery_command(env, command);
    case SAVE:      return do_save_command(env, command);
    case DRAW:      return do_draw_command(env, command);
    case REDFIND:   return do_redfind(env, command);
    default: break;
  }
  printf("1: Unrecognized command (%d)\n",command->label);
  return 0;
}

/* int do_draw_command(Node *command) */
/* in file/file.c */

/* int do_save_command(Node *command) */
/* in file/file.c */

int do_abquery_command(Environment *env, Node *command)
{
  int res;
  Interp *interp;

  const string bname = (const char*) command->l->data;
  const string sname = (const char*) command->r->data;

  BQuery *bq = getBQuery(env, bname);
  if(!bq) return 0;
  Structure* struc = getStructure(env, sname);
  if(!struc) return 0;

  /* minisat is a builtin query using the Minisat-C solver */
  if(bname == "minisat") return do_minisat_query(struc);
  if(bname == "minisat2") return do_minisat2_query(struc);
  if(bname == "threecolorwithsat") return do_threecolorsat_query(struc);
  if(bname == "threecolorwithsat2") return do_threecolor_sat2_query(struc);

  // TODO bname == ?
  interp = new_interp(struc);
  res = eval(bq->form, interp, struc);
  free_interp(interp);

  /* can't keep the TC cache here */
  free_tc_caches(bq->form);
  if (res) {
    printf(":\\t\n");
    return 1;
  }
  printf(":\\f\n");
  return 0;
}

/* P:2 is x1=3&x4=0&x2=0&x3=x6&x5<3 
 * N:2 is x4=0 & E(x2,x3) & (x1=x5 & (x6=x2 | x6=x3)) & x1<3
 */
int do_threecolorsat_query(const Structure *struc)
{
  int i, i1, i2;
  int j;
  int n=struc->size;
  Interp *interp=new_interp(struc);
  int *iv;
  int *jv;
  Node *e;
  int ii, jj;
  int r;
  Relation *er=get_relation("E",NULL,struc);
  int *ec = er->cache;
  veci *lits=(veci*) malloc(sizeof(veci));
  lit *begin;
  solver *s=solver_new();
  veci_new(lits);
  e = er->parse_cache;

  i1=n;
  i2=n*n; /* CHJ IS 2n OKAY ? */
  /* P */
  for (i=0; i<n; i++)
  {
    veci_resize(lits,0);
    veci_push(lits, toLit(i));
    veci_push(lits, toLit(i+i1));
    veci_push(lits, toLit(i+i2));
    begin=veci_begin(lits);
    solver_addclause(s,begin,begin+veci_size(lits));
  }

  interp = add_symb_to_interp(interp, "x1", 0);
  interp = add_symb_to_interp(interp, "x2", 0);
  jv = &(interp->symbols->value);
  iv = &(interp->symbols->next->value);
  /* N */
  /* couple of tricks here. */
  /* first, we try to use the cache as much as possible.
   * second, if E(i,j), there's no reason to check E(j,i), which could,
   * especially with empty caches, help a lot.
   * next, we modify the current interpretation instead of adding and
   * deleting symbols.
   * finally, we only modify the interpretation when we really need to.
   * this means that the interpretation could be in a strange state,
   * but it is corrected before calling eval.
   */
  for (i=ii=0; i<n; i++,ii+=n)
  {
    *iv = i;
    for (j=jj=i; j<n; j++,jj+=n)
    {
      /* *jv = j; */
      /* a tiny optimization, we only need *jv=j if
       * ec[ii+j] is not -1, comma operator evaluates to RHS
       * and evaluates LHS first per ANSI.
       */
      r = ec[ii+j];
      if (r==1 || (r==-1 && (*jv=j,eval(e,interp,struc))))
      {
        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j)));
        veci_push(lits, lit_neg(toLit(i)));
        begin=veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits));

        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j+i1)));
        veci_push(lits, lit_neg(toLit(i+i1)));
        begin=veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits));

        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j+i2)));
        veci_push(lits, lit_neg(toLit(i+i2)));
        begin=veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits));
        continue;
      }
      if (i==j)
        continue;
      r = ec[jj+i];
      if (r==1 || (r==-1 && (*iv=j,*jv=i,eval(e,interp,struc))))
      {
        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j)));
        veci_push(lits, lit_neg(toLit(i)));
        begin = veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits));

        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j+i1)));
        veci_push(lits, lit_neg(toLit(i+i1)));
        begin=veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits)); 
        veci_resize(lits,0);
        veci_push(lits, lit_neg(toLit(j+i2)));
        veci_push(lits, lit_neg(toLit(i+i2)));
        begin=veci_begin(lits);
        solver_addclause(s,begin,begin+veci_size(lits));
      }
      *iv=i; /* *jv will be taken care of automatically */
    }
  }
#if 0
  printf("D:Reduction completed, executing SAT solver.\n");
#endif
  free_interp(interp);
  i = solver_solve(s,0,0);
  solver_delete(s);
  veci_delete(lits);
  printf(i==1?":\\t\n":":\\f\n");
  return i;
}

/* based on do_minisat_query */
int do_minisat2_query(const Structure *struc)
{
  int c, v;
  int size=struc->size;
  Interp *interp;
  int res;
  int i;
  Relation *pr = get_relation("P",NULL,struc);
  Relation *nr = get_relation("N",NULL,struc);
  Node *p=pr->parse_cache;
  Node *n=nr->parse_cache;
  int *pc = pr->cache;
  int *nc = nr->cache;
  int flag;

  minisat_solver *solver=minisat_new();
  minisat_Lit *lits = (minisat_Lit*) malloc(sizeof(minisat_Lit)*size);


  for (i=0; i<size; i++)
    lits[i]=minisat_newLit(solver);

  interp = new_interp(struc);
  interp = add_symb_to_interp(interp, "x2",0);
  interp = add_symb_to_interp(interp, "x1",0);
  i=-1;

  /* for each clause */
  for (c=0; c<size; c++)
  {
    flag=0;
    for (v=0; v<size; v++)
    {
      i++;
      interp->symbols->value = c;
      interp->symbols->next->value = v;
      if ((res=pc[i])==-1)
        res = eval(p,interp,struc);
      if (res)
      {
        if (!flag)
          minisat_addClause_begin(solver);
        flag=1;
        minisat_addClause_addLit(solver, lits[v]);
      }
      if ((res=nc[i])==-1)
        res = eval(n,interp,struc);
      if (res)
      {
        if (!flag)
          minisat_addClause_begin(solver);
        flag=1;
        minisat_addClause_addLit(solver, 
            minisat_negate(lits[v]));
      }
    }
    if (flag)
      minisat_addClause_commit(solver);
  }

  free_interp(interp);
  free(lits);
  res = minisat_solve(solver, 0, NULL);
  minisat_delete(solver);
  printf(res==1?":\\t\n":":\\f\n");
  return res;
}

/* based on do_threecolorsat_query */
/* P:2 is x1=3&x4=0&x2=0&x3=x6&x5<3 
 * N:2 is x4=0 & E(x2,x3) & (x1=x5 & (x6=x2 | x6=x3)) & x1<3
 */
int do_threecolor_sat2_query(const Structure *struc)
{
  int i, i1, i2;
  int j;
  int n=struc->size;
  Interp *interp=new_interp(struc);
  int *iv;
  int *jv;
  Node *e;
  int ii, jj;
  int r;
  Relation *er=get_relation("E",NULL,struc);
  int *ec = er->cache;
  int threen=3*n; /* 3n */
  minisat_Lit *lits=(minisat_Lit*) malloc(sizeof(minisat_Lit)*threen);
  minisat_solver *s=minisat_new();

  for (i=0; i<threen; i++)
    lits[i]=minisat_newLit(s);

  e = er->parse_cache;

  i1=n;
  i2=(n<<1); /* 2n */
  /* P */
  for (i=0; i<n; i++)
  {
    /* vertex i has a color */
    minisat_addClause_begin(s);
    minisat_addClause_addLit(s, lits[i]);
    minisat_addClause_addLit(s, lits[i+i1]);
    minisat_addClause_addLit(s, lits[i+i2]);
    minisat_addClause_commit(s);
  }

  interp = add_symb_to_interp(interp, "x1", 0);
  interp = add_symb_to_interp(interp, "x2", 0);
  jv = &(interp->symbols->value);
  iv = &(interp->symbols->next->value);
  /* couple of tricks here. */
  /* first, we try to use the cache as much as possible.
   * second, if E(i,j), there's no reason to check E(j,i), which could,
   * especially with empty caches, help a lot.
   * next, we modify the current interpretation instead of adding and
   * deleting symbols.
   * finally, we only modify the interpretation when we really need to.
   * this means that the interpretation could be in a strange state,
   * but it is corrected before calling eval.
   */
  for (i=ii=0; i<n; i++,ii+=n)
  {
    *iv = i;
    for (j=jj=i; j<n; j++,jj+=n)
    {
      /* *jv = j; */
      /* a tiny optimization, we only need *jv=j if
       * ec[ii+j] is not -1, comma operator evaluates to RHS
       * and evaluates LHS first per ANSI.
       */
      r = ec[ii+j];
      if (r==1 || (r==-1 && (*jv=j,eval(e,interp,struc))))
      {
        /* there is an edge (i,j) */
        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,
            minisat_negate(lits[i]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j]));
        minisat_addClause_commit(s);

        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,
            minisat_negate(lits[i+i1]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j+i1]));
        minisat_addClause_commit(s);

        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,
            minisat_negate(lits[i+i2]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j+i2]));
        minisat_addClause_commit(s);
        continue;
      }
      if (i==j)
        continue;
      r = ec[jj+i];
      if (r==1 || (r==-1 && (*iv=j,*jv=i,eval(e,interp,struc))))
      {
        /* there is an edge (i,j) */
        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,
            minisat_negate(lits[i]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j]));
        minisat_addClause_commit(s);

        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,
            minisat_negate(lits[i+i1]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j+i1]));
        minisat_addClause_commit(s);

        minisat_addClause_begin(s);
        minisat_addClause_addLit(s,                     
            minisat_negate(lits[i+i2]));
        minisat_addClause_addLit(s,
            minisat_negate(lits[j+i2]));
        minisat_addClause_commit(s);
      }
      *iv=i;
    }
  }

  free_interp(interp);
  i = minisat_solve(s,0,NULL);
  minisat_delete(s);
  free(lits);
  printf(i==1?":\\t\n":":\\f\n");
  return i;
}

/* TODO ALL MY SAT STUFF ASSUMES NO LIT OCCURS BOTH POS AND NEG IN SAME CLAUSE */
int do_minisat_query(const Structure *struc)
{
  int c, v;
  int size;
  Interp *interp;
  int res;
  int i;
  Relation *pr = get_relation("P",NULL,struc);
  Relation *nr = get_relation("N",NULL,struc);
  Node *p=pr->parse_cache;
  Node *n=nr->parse_cache;
  int *pc = pr->cache;
  int *nc = nr->cache;
  veci *lits=(veci*) malloc(sizeof(veci));
  lit *begin;
  solver *s = solver_new();
  size = struc->size;

  veci_new(lits);
  interp = new_interp(struc);
  interp = add_symb_to_interp(interp, "x2",0);
  interp = add_symb_to_interp(interp, "x1",0);
  i=-1;
  /* for each clause */
  for (c=0; c<size; c++)
  {
    veci_resize(lits,0);
    for (v=0; v<size; v++)
    {
      i++;
      interp->symbols->value = c;
      interp->symbols->next->value = v;
      if ((res=pc[i])==-1)
        res = eval(p,interp,struc);
      if (res)
      {
        veci_push(lits, toLit(v));
        continue;
      }
      if ((res=nc[i])==-1)
        res = eval(n,interp,struc);
      if (res)
      {
        veci_push(lits,lit_neg(toLit(v)));
      }
    }
    begin = veci_begin(lits);
    if (veci_size(lits)<=0)
      continue;
    if (!solver_addclause(s, begin, begin+veci_size(lits)))
    {
      printf(":\\f\n");
      veci_delete(lits);
      free_interp(interp);
      return 0;
    }
  }
  free_interp(interp);

  res = solver_solve(s,0,0);
  solver_delete(s);
  printf(res==1?":\\t\n":":\\f\n");
  return res;
}

int do_listtuple_command(Environment *env, Node *command) {
  Relation *rel;
  int arity, size;
  int *tuple=NULL;
  Interp *interp;
  char *output=NULL;
  int tuple_num, res;

  char* sname = (char*) command->l->data;
  char* rname = (char*) command->r->data;

  Structure *str = getStructure(env, sname);
  if(!str) return 0;

  rel = get_relation(rname, NULL, str);

  if (!rel)
  {
    printf("11: Relation %s does not exist in %s\n",rname,sname);
    return 0;
  }

  arity = rel->arity;
  size = str->size;

  if (rel->cache)
    tuple_num = 0;
  else
    tuple_num = -1;

  interp = new_interp(str);

  if (rel->parse_cache)
    eval_init_form(rel->parse_cache, interp, str);

  while ((tuple = next_tuple(tuple, arity, size)))
  {
    if (tuple_num>=0 && ((res=rel->cache[tuple_num++])>-1))
    {
      if (res)
        output = add_tup_to_output(output,
            tuple, arity, size);
      continue;
    }

    interp = fake_add_tup_to_interp(interp, tuple, arity);
    if ((res=eval_rec(rel->parse_cache, interp, str)))
      output = add_tup_to_output(output, tuple, arity,size);
    /* interp = free_remove_tup(interp, arity); */
    if (tuple_num>=0)
      rel->cache[tuple_num-1]=res;
  }

  free_interp(interp);

  if (output)
  {
    printf(":%s}\n",output);
    free(output);
  }
  else
    printf(":{}\n");
  return 1;
}

int do_expred_command(Environment *env, Node *command) {
  int value;
  char ans;
  Interp *inter;

  char* sname = (char*) command->l->data;
  char* rname = (char*) command->r->data;

  Structure *str = getStructure(env, sname);
  if(!str) return 0;

  inter = new_interp(str);
  value = eval(command->r, inter, str);
  free_interp(inter);

  if (value==-1)
  {
    printf("8: Error evaluating predicate %s in %s.\n",rname,sname);
    return 0;
  }

  ans = (value?'t':'f');
  printf(":\\%c\n",ans);

  return 1;
}

int do_excons_command(Environment *env, Node *command) {
  Constant *cons;
  char *sname;
  char *cname;
  Interp *interp;
  int value;

  sname = (char *)command->l->data;
  cname = (char *)command->r->data;

  Structure *str = getStructure(env, sname);
  if(!str) return 0;

  cons = get_constant(cname,str);
  if (!cons) {
    printf("33: Constant %s does not exist in %s\n",cname,sname);
    return 0;
  }

  interp = new_interp(str);

  /* done?: TODO check if this is right and free that interp */
  value = cons->value;
  if (value==-1)
    value=teval(cons->parse_cache, interp, str);

  free(interp);
  if (value == -1)
  {
    printf("7: Invalid constant definition for %s.%s\n",sname,cname);
    return 0;
  }

  printf(":%d\n",value);
  return 1;
}

int do_assign_command(Environment *env, Node *command) {
  char *new_name = (char*) command->l->data;

  if (getBinding(env, new_name)) {
    err("22: %s already exists\n", new_name);
    return 0;
  }

  switch (command->r->label) {
    case VOCAB:      return do_vocab_assign(env, command);
    case STRUC:      return do_struc_assign(env, command);
    case REDUC:      return do_reduc_assign(env, command);
    case APPLY:      return do_apply_assign(env, command);
    case BQUERY:     return do_bquery_assign(env, command);
    case LOAD:       return do_load(env, command);
    case LOADSTRING: return do_loadassign(env, command);
    case MACE:       return do_mace(env, command);
    default: return -1;
  }
}

/*This is a bit tricky.
 * Basically, the universe of the new structure will consist of
 * k-ary tuples of elements from the old universe, but the reduction
 * or query is allowed to restrict which tuples are included in a first-order
 * (or other, we don't restrict the user) way.  We remap these tuples back to
 * the naturals, so E(0,0) is possible (instead of E(<0,0,0,0,0...>,<0,0,0..>).
 */ 
int do_apply_assign(Environment *env, Node *command) {
  Relation *rel;
  Relation *newr;
  Relation *prevr;

  ConsForm *cons;
  Constant *newc;
  Constant *prevc;
  /*
     Node *form;
     Node *t;
     */
  int k, arity, i, res;
  int cind;
  int *cindex;
  int *tuple=0;
  int fl;
  /*	int num_tups; */
  int size, relsize;
  int **values;

  Interp *interp;
  int firstcall;
  ReductionMap *rmap;

  /* TODO check if new id exists */
  /* this should already be checked in do_assign_command above 
   * (chj 11/1/11)
   */
  Reduction *reduc = getReduction(env, (const char*) command->r->l->data);
  if(!reduc) return 0;
  Structure *ostruc = getStructure(env, (const char*) command->r->r->data);
  if(!ostruc) return 0;

  /* TODO CHECK TYPES ON VOCABULARIES */
  Structure *new_id = (Structure*) malloc(sizeof(Structure));
  rmap = make_rmap(reduc, ostruc);
  k = reduc->k;
  size = rmap->size;

  new_id->size = size;
  new_id->name = dupstr((const char*) command->l->data);
  new_id->cons = prevc = 0;
  new_id->rels = prevr = 0;
  new_id->vocab = reduc->to_vocab;
  cindex = 0;
  /* num_tups = de_pow(ostruc->size,k); */
  /* num_of_ktuples = de_pow(rmap->size,k); */

  /* this is a bit tricky, we want to get the relations.  We evaluate them on
   * a rel->arity'ary tuple of k-ary tuples, BUT only if these k-ary tuples are
   * ALL included in the new structure's universe.  Also, the cache of the
   * relation is indexed on the naturals from the rmap->tup_to_nat map of
   * cindices of k-tuples to naturals.  mmkay? chj
   */

  for (rel = reduc->relforms; rel; rel = rel->next)
  {
    firstcall=1;
    interp = new_interp(ostruc);
    arity = k*rel->arity;

    values = (int**) malloc(sizeof(int *)*arity);
    tuple = (int*) malloc(sizeof(int)*arity);
    for (i=0; i<arity; i++)
      tuple[i]=0;
    interp = add_tup_to_interp(interp, tuple, arity);
    for (i=0; i<arity; i++)
      values[i]=get_xi_ival(i+1, interp);
    free(tuple);
    tuple=NULL;

    newr = (Relation*) malloc(sizeof(Relation));
    newr->name = rel->name;
    newr->arity = rel->arity;
    newr->parse_cache = 0; /* TODO HACK THIS IN */
    relsize = de_pow(size, rel->arity);
    newr->cache = (int*) malloc(relsize*sizeof(int));
    relsize--;
    /* ci = malloc(rel->arity*sizeof(int)); */
    cind = -1;
    cindex = (int*) malloc(rel->arity*sizeof(int));
    while (cind<relsize)
    {
      /* tuple = next_tuple(tuple, arity, ostruc->size); */
      fl = 0;
      /*			cindex = next_tuple(cindex, rel->arity, num_tups);*/
      tuple = make_next_tuple(tuple, rmap, arity, 
          ostruc->size, cindex,
          rel->arity, k);
      for (i=0; i<rel->arity; i++)
        if (rmap->tup_to_nat[cindex[i]]==-1)
          fl=-1;
      if (fl==-1)
        continue;
      /*cindex = next_tuple(cindex, rel->arity, rmap->size);*/
      cind++;
      for (i=0; i<arity; i++)
        *(values[i])=tuple[i];
      if (!firstcall)
        res = eval_rec(rel->parse_cache, interp, ostruc);
      else
      {
        res = eval(rel->parse_cache, interp, ostruc);
        firstcall = 0;
      }
      newr->cache[cind]=res;

    }
    if (tuple)
    {
      free(tuple);
      tuple = 0;
    }
    if (cindex)
    {
      free(cindex);
      cindex = 0;
    }
    /* free(ci); */
    if (prevr)
    {
      prevr->next = newr;
      newr->next = 0;
      prevr = newr;
    }
    else
    {
      new_id->rels = prevr = newr;
      newr->next = 0;
    }
    free_interp(interp);
  }

  interp = new_interp(ostruc);
  values = (int**) malloc(sizeof(int *)*k);
  tuple = next_tuple(tuple,k,ostruc->size);
  interp = add_tup_to_interp(interp, tuple, k);
  for (i=0; i<k; i++)
    values[i]=get_xi_ival(i+1, interp);
  free(tuple);
  tuple=NULL;

  for (cons = reduc->consforms; cons; cons=cons->next)
  {
    newc = (Constant*) malloc(sizeof(Constant));
    newc->name = cons->name;
    newc->parse_cache = cons->parse_cache;
    newc->next = 0;
    cind = -1;
    arity = -1;
    firstcall = 1;
    while ((tuple=next_tuple(tuple,k,ostruc->size)))
    {
      arity++;
      if (rmap->tup_to_nat[arity]==-1)
        continue; 
      cind++;
      for (i=0; i<k; i++)
        *(values[i])=tuple[i];
      if (!firstcall)
        res = eval_rec(newc->parse_cache, interp, ostruc);
      else
      {
        res = eval(newc->parse_cache, interp, ostruc);
        firstcall = 0;
      }

      if (res)
      {
        newc->value = cind;
        free(tuple);
        tuple = 0;
        break;
      }
    }
    if (prevc)
    {
      prevc->next = newc;
      prevc = newc;
    }
    else
      new_id->cons = prevc = newc;
  }

  free(values);				
  free(interp);
  free(rmap->nat_to_tup);
  free(rmap->tup_to_nat);
  free(rmap);

  if(!makeBinding(env, new_id->name, STRUC, new_id))
    return 0;

  return 1;
}

int do_bquery_assign(Environment *env, Node *command) {
  const char *vname = (char*) command->r->l->data;

  Vocabulary *vocab = getVocab(env, vname);
  if(!vocab) return 0;

  char *name = dupstr((const char*) command->l->data);
  BQuery *new_id = (BQuery*) malloc(sizeof(BQuery));
  new_id->name = name;
  new_id->voc = vocab;
  new_id->form = command->r->r;	
  
  if(!makeBinding(env, name, BQUERY, new_id)) {
    return 0;
  }

  return 1;
}

/* TODO sanity checking to make sure the reduction defines a structure
 * of the correct vocab, etc. */
int do_reduc_assign(Environment *env, Node *command) {
  Node *cmdexpr = command->r;
  Node *t;
  char *fvname, *tvname;

  Reduction *new_id;
  int k;
  Relation *tmpr, *prevr=NULL;
  ConsForm *tmpc, *prevc=NULL;
  char *rel_name;
  int rel_arity;
  Node *rel_form;

  fvname = (char*) cmdexpr->l->l->l->data;
  tvname = (char*) cmdexpr->l->l->r->data;

  Vocabulary *from_vocab = getVocab(env, fvname);
  if(!from_vocab) return 0;
  Vocabulary *to_vocab = getVocab(env, tvname);
  if(!to_vocab) return 0;

  k = *(int *)(cmdexpr->r->l->data);
  t = cmdexpr->l->r;
  new_id = (Reduction*) malloc(sizeof(Reduction)); 
  if (!new_id)
  {
    err("Insufficient memory\n");
    return 0;
  }
  new_id->from_vocab = from_vocab;
  new_id->to_vocab = to_vocab;
  new_id->k = k;
  new_id->name = dupstr((const char*) command->l->data);
  new_id->universe_form = (cmdexpr->r->r);
  new_id->relforms = NULL;
  new_id->consforms = NULL;

  /* TODO really need to error check all this */

  while (t && t->label == CRRELDEF)
  {
    rel_name = (char*) t->l->l->data;
    rel_arity = *(int *)(t->l->r->data);
    rel_form = t->r->l;

    tmpr = (Relation*) malloc(sizeof(Relation));
    if (!prevr)
      new_id->relforms = tmpr;
    else
      prevr->next = tmpr;
    tmpr->name = dupstr((const char*) rel_name);
    tmpr->parse_cache=rel_form;
    tmpr->arity = rel_arity; /* or k*rel_arity is better? */
    tmpr->next = NULL;
    tmpr->cache = NULL;
    prevr = tmpr;
    t=t->r->r;
  }

  while (t && t->label == CRCONSDEF)
  {
    rel_name = (char*) t->l->l->data;
    rel_form = t->l->r;

    tmpc = (ConsForm*) malloc(sizeof(ConsForm));
    if (!prevc)
      new_id->consforms = tmpc;
    else
      prevc->next = tmpc;
    tmpc->name = dupstr((const char*) rel_name);
    tmpc->parse_cache = rel_form;
    tmpc->next = 0;
    prevc = tmpc;
    t = t->r;
  }

  if(!makeBinding(env, new_id->name, REDUC, new_id))
    return 0;

  return 1;
}

int do_vocab_assign(Environment *env, Node *command) {
  Node *assign_id = command->l;
  Node *cmdexpr = command->r;
  Node *t;

  Vocabulary * new_id;
  RelationSymbol *tmpr;
  RelationSymbol *prevr=NULL;
  ConsSymbol *tmpc;
  ConsSymbol *prevc=NULL;

  t = cmdexpr->l;

  new_id = (Vocabulary*) malloc(sizeof(Vocabulary));
  new_id ->name = dupstr((const char*) assign_id->data);
  new_id->id = env->next_id++;
  new_id->cons_symbols = 0;
  new_id->rel_symbols = 0;
  while (t && t->label == CVRELARG)
  {
    tmpr = (RelationSymbol*) malloc(sizeof(RelationSymbol));
    if (!prevr)
      new_id->rel_symbols = tmpr;
    else
      prevr->next = tmpr;
    tmpr->next = 0;
    tmpr->name = dupstr((const char*) t->l->l->data);      /* scary */
    tmpr->arity = *(int *)(t->l->r->data);   /* oh noes */
    prevr = tmpr;
    t=t->r;
  }
  while (t && t->label == CVCONSARG)
  {
    tmpc = (ConsSymbol*) malloc(sizeof(ConsSymbol));
    if (!prevc)
      new_id->cons_symbols = tmpc;
    else
      prevc->next=tmpc;
    tmpc->next = 0;
    tmpc->name = dupstr((const char*) t->l->data);
    prevc = tmpc;
    t=t->r;
  }

  if(!makeBinding(env, new_id->name, VOCAB, new_id))
    return 0;

  return 1;
}

/* TODO make sure that the structure definition matches the vocabulary given */

int do_struc_assign(Environment *env, Node *command) {
  Node *assign_id = command->l;
  Node *cmdexpr = command->r;
  Node *t;

  Structure *new_id;
  Relation *tmpr;
  Relation *prevr=0;
  Constant *tmpc;
  Constant *prevc=0;

  int *cache_pt;
  int size, cache_size, i;

  char *rel_name;
  int rel_arity;
  Node *rel_form;	

  t = cmdexpr->l;

  const char *vname = (const char*) t->l->l->data;
  Vocabulary *voc = getVocab(env, vname);
  if(!voc) return 0;

  if ((size=teval(t->l->r,new_interp(NULL),NULL))<0)
  {
    printf("3: Invalid size of universe\n");
    return 0;
  }

  assert(t->label==CSARGS);
  new_id = (Structure*) malloc(sizeof(Structure));
  new_id->name = dupstr((const char*) assign_id->data);
  new_id->id = env->next_id++;
  new_id->cons = 0;
  new_id->rels = 0;
  new_id->vocab = voc;
  new_id->size = size;

  t=t->r;

  while (t && t->label==CSRELDEF)
  {
    rel_name=(char*) t->l->l->data;
    rel_arity=*(int *)(t->l->r->data);
    rel_form=t->r->l;
    cache_size = de_pow(size, rel_arity);
    cache_pt = (int*) malloc(cache_size*sizeof(int));

    tmpr = (Relation*) malloc(sizeof(Relation));
    if (!prevr)
      new_id->rels=tmpr;
    else
      prevr->next=tmpr;
    tmpr->name=dupstr((const char*) rel_name);
    tmpr->parse_cache=rel_form;
    tmpr->arity=rel_arity;
    tmpr->next = 0;
    tmpr->cache = cache_pt;

    if (tmpr->cache)
      for (i=0; i<cache_size; i++)
        tmpr->cache[i] = -1;

    prevr = tmpr;

    t=t->r->r;
  }

  while (t && t->label == CSCONSDEF)
  {
    rel_name = (char*) t->l->l->data;
    rel_form = t->l->r;

    tmpc = (Constant*) malloc(sizeof(Constant));
    if (!prevc)
      new_id->cons = tmpc;
    else
      prevc->next = tmpc;

    tmpc->name = dupstr((const char*) rel_name);
    tmpc->parse_cache = rel_form;
    tmpc->next = 0;
    tmpc->value = 0;
    tmpc->value = teval(tmpc->parse_cache, new_interp(new_id),NULL);

    prevc = tmpc;
    t = t->r;
  }

  if(!makeBinding(env, new_id->name, STRUC, new_id))
    return 0;

  return 1;
}

/* redfind is in redfind/redfind.c */
int do_redfind(Environment *env, Node *command) {
  int k, c, n1, n2;

  if (!(command->r->l)) {
    k=1;
    c=1;
    n1=n2=3;
  } else {
    k=*(int *)command->r->l->data;
    c=*(int *)command->r->r->l->data;
    if (command->r->r->r->label != RF_RANGE)
      n1=n2=*(int *)command->r->r->r->data;
    else {
      n1 = *(int *)command->r->r->r->l->data;
      n2 = *(int *)command->r->r->r->r->data;
    }
  }

  char *p1name = (char*) command->l->l->data;
  char *p2name = (char*) command->l->r->data;
  BQuery *p1 = getBQuery(env, p1name);
  if(!p1) return 0;
  BQuery *p2 = getBQuery(env, p2name);
  if(!p2) return 0;

  /* okay, now we want to search for a reduction from p1 to p2. */
  return redfind(env, p1,p2,k,c,n1,n2);
}

/* usemace is in mace/usemace.c */
int do_mace(Environment *env, Node *command) {
  const char *vname = (char*) command->r->l->l->data;

  Vocabulary *vocab = getVocab(env, vname);
  if(!vocab) return 0;

  Node *form = command->r->l->r;
  char *freevar = free_var(form,vocab);
  if (freevar) {
    err("42: %s is free.\n",freevar);
    return 0;
  }

  int clock = 0;
  if (command->r->r)
    clock = command->r->r->ndata;
  return usemace(env, form, vocab, (char*) command->l->data, clock);
}


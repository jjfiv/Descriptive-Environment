/* limboole.c
 *
 * Does CNF conversion for propositional formulas.  We use it
 * to convert formulas generated in redfind.c for use with
 * SAT solvers.
 *
 * This is the CNF conversion from Limboole 0.2.  Nothing from
 * Limmat is included here.
 *
 * Limboole is available from http://fmv.jku.at/limboole/
 * As of 4/11/2012, the license is given as
 *
 * "Limboole is absolutely free software. You can modify and use it for 
 *  any purpose you like. However, note that Limmat has a BSD license and if 
 *  you generate a binary version of Limboole linked to Limmat you have to 
 *  obey the requirements of the license of Limmat."
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "redprotos.h"
#include "redtypes.h"
#include "minisat.h"

extern char *dupstr(const char *);

char *limboole_id = "$Id: limboole.c,v 1.15 2002/11/07 07:12:07 biere Exp $";

/*------------------------------------------------------------------------*/
/* These are the node types we support.  They are ordered in decreasing
 * priority: if a parent with type t1 has a child with type t2 and t1 > t2,
 * then pretty printing the parent requires parentheses.  See 'pp_aux' for
 * more details.
 */
enum Type
{
  VAR = 0,
  LP = 1,
  RP = 2,
  NOT = 3,
  AND = 4,
  OR = 5,
  IMPLIES = 6,
  IFF = 7,
  DONE = 8,
  ERROR = 9
};

/*------------------------------------------------------------------------*/

typedef enum Type Type;
typedef struct Node Node;
typedef union Data Data;

/*------------------------------------------------------------------------*/

union Data
{
  char *as_name;		/* variable data */
  Node *as_child[2];		/* operator data */
};

/*------------------------------------------------------------------------*/

struct Node
{
  Type type;
  int idx;			/* tsetin index */
  Node *next;			/* collision chain in hash table */
  Node *next_inserted;		/* chronological list of hash table */
  Data data;
};

/*------------------------------------------------------------------------*/

typedef struct Mgr Mgr;

struct Mgr
{
  unsigned nodes_size;
  unsigned nodes_count;
  int idx;
  Node **nodes;
  Node *first;
  Node *last;
  Node *root;
  char *buffer;
  char *name;
  unsigned buffer_size;
  unsigned buffer_count;
  int saved_char;
  int saved_char_is_valid;
  int last_y;
  int verbose;
  unsigned x;
  unsigned y;
  struct minisat_solver_t *solver;
  minisat_Lit *posLits;
  struct env *env;
  char *form;
  int pos;
  FILE *log;
  FILE *out;
  int close_in;
  int close_log;
  int close_out;
  Type token;
  unsigned token_x;
  unsigned token_y;
  Node **idx2node;
  int check_satisfiability;
  int dump;
};

/*------------------------------------------------------------------------*/

static unsigned
hash_var (Mgr * mgr, const char *name)
{
  unsigned res, tmp;
  const char *p;

  res = 0;

  for (p = name; *p; p++)
    {
      tmp = res & 0xf0000000;
      res <<= 4;
      res += *p;
      if (tmp)
	res ^= (tmp >> 28);
    }

  res &= (mgr->nodes_size - 1);
  assert (res < mgr->nodes_size);

  return res;
}

/*------------------------------------------------------------------------*/

static unsigned
hash_op (Mgr * mgr, Type type, Node * c0, Node * c1)
{
  unsigned res;

  res = (unsigned) type;
  res += 4017271 * (unsigned) c0;
  res += 70200511 * (unsigned) c1;

  res &= (mgr->nodes_size - 1);
  assert (res < mgr->nodes_size);

  return res;
}

/*------------------------------------------------------------------------*/

static unsigned
hash (Mgr * mgr, Type type, void *c0, Node * c1)
{
  if (type == VAR)
    return hash_var (mgr, (char *) c0);
  else
    return hash_op (mgr, type, c0, c1);
}

/*------------------------------------------------------------------------*/

static int
eq_var (Node * n, const char *str)
{
  return n->type == VAR && !strcmp (n->data.as_name, str);
}

/*------------------------------------------------------------------------*/

static int
eq_op (Node * n, Type type, Node * c0, Node * c1)
{
  return n->type == type && n->data.as_child[0] == c0
    && n->data.as_child[1] == c1;
}

/*------------------------------------------------------------------------*/

static int
eq (Node * n, Type type, void *c0, Node * c1)
{
  if (type == VAR)
    return eq_var (n, (char *) c0);
  else
    return eq_op (n, type, c0, c1);
}

/*------------------------------------------------------------------------*/

static Node **
find (Mgr * mgr, Type type, void *c0, Node * c1)
{
  Node **p, *n;
  unsigned h;

  h = hash (mgr, type, c0, c1);
  for (p = mgr->nodes + h; (n = *p); p = &n->next)
    if (eq (n, type, c0, c1))
      break;

  return p;
}

/*------------------------------------------------------------------------*/

static void
enlarge_nodes (Mgr * mgr)
{
  Node **old_nodes, *p, *next;
  unsigned old_nodes_size, h, i;

  old_nodes = mgr->nodes;
  old_nodes_size = mgr->nodes_size;
  mgr->nodes_size *= 2;
  mgr->nodes = (Node **) calloc (mgr->nodes_size, sizeof (Node *));

  for (i = 0; i < old_nodes_size; i++)
    {
      for (p = old_nodes[i]; p; p = next)
	{
	  next = p->next;
	  if (p->type == VAR)
	    h = hash_var (mgr, p->data.as_name);
	  else
	    h =
	      hash_op (mgr, p->type, p->data.as_child[0],
		       p->data.as_child[1]);
	  p->next = mgr->nodes[h];
	  mgr->nodes[h] = p;
	}
    }

  free (old_nodes);
}

/*------------------------------------------------------------------------*/

static void
insert (Mgr * mgr, Node * node)
{
  if (mgr->last)
    mgr->last->next_inserted = node;
  else
    mgr->first = node;
  mgr->last = node;
  mgr->nodes_count++;
}

/*------------------------------------------------------------------------*/

static Node *
var (Mgr * mgr, const char *str)
{
  Node **p, *n;

  if (mgr->nodes_size <= mgr->nodes_count)
    enlarge_nodes (mgr);

  p = find (mgr, VAR, (void *) str, 0);
  n = *p;
  if (!n)
    {
      n = (Node *) malloc (sizeof (*n));
      memset (n, 0, sizeof (*n));
      n->type = VAR;
      n->data.as_name = dupstr (str);

      *p = n;
      insert (mgr, n);
    }

  return n;
}

/*------------------------------------------------------------------------*/

static Node *
op (Mgr * mgr, Type type, Node * c0, Node * c1)
{
  Node **p, *n;

  if (mgr->nodes_size <= mgr->nodes_count)
    enlarge_nodes (mgr);

  p = find (mgr, type, c0, c1);
  n = *p;
  if (!n)
    {
      n = (Node *) malloc (sizeof (*n));
      memset (n, 0, sizeof (*n));
      n->type = type;
      n->data.as_child[0] = c0;
      n->data.as_child[1] = c1;

      *p = n;
      insert (mgr, n);
    }

  return n;
}

/*------------------------------------------------------------------------*/

static Mgr *
init (char *form, struct minisat_solver_t *solver)
{
  Mgr *res;

  res = (Mgr *) malloc (sizeof (*res));
  memset (res, 0, sizeof (*res));
  res->nodes_size = 2;
  res->nodes = (Node **) calloc (res->nodes_size, sizeof (Node *));
  res->buffer_size = 2;
  res->buffer = (char *) malloc (res->buffer_size);
  /* res->in = stdin; */
  res->form = form;
  res->pos = 0;
  res->log = stderr;
  res->out = stdout;
  res->solver=solver;
  return res;
}

static void
release (Mgr * mgr)
{
  Node *p, *next;

  for (p = mgr->first; p; p = next)
    {
      next = p->next_inserted;
      if (p->type == VAR)
	free (p->data.as_name);
      free (p);
    }


  if (mgr->close_out)
    fclose (mgr->out);
  if (mgr->close_log)
    fclose (mgr->log);

  free (mgr->idx2node);
  free (mgr->nodes);
  free (mgr->buffer);
  free (mgr->posLits);
  free (mgr);
}

/*------------------------------------------------------------------------*/

static void
print_token (Mgr * mgr)
{
  switch (mgr->token)
    {
    case VAR:
      fputs (mgr->buffer, mgr->log);
      break;
    case LP:
      fputc ('(', mgr->log);
      break;
    case RP:
      fputc (')', mgr->log);
      break;
    case NOT:
      fputc ('!', mgr->log);
      break;
    case AND:
      fputc ('&', mgr->log);
      break;
    case OR:
      fputc ('|', mgr->log);
      break;
    case IMPLIES:
      fputs ("->", mgr->log);
      break;
    case IFF:
      fputs ("<->", mgr->log);
      break;
    default:
      assert (mgr->token == DONE);
      fputs ("EOF", mgr->log);
      break;
    }
}

/*------------------------------------------------------------------------*/

static void
parse_error (Mgr * mgr, const char *fmt, ...)
{
  va_list ap;
  char *name;

  name = mgr->name ? mgr->name : "<stdin>";
  fprintf (mgr->log, "%s:%u:%u: ", name, mgr->token_x + 1, mgr->token_y);
  if (mgr->token == ERROR)
    fputs ("scan error: ", mgr->log);
  else
    {
      fputs ("parse error at '", mgr->log);
      print_token (mgr);
      fputs ("' ", mgr->log);
    }
  va_start (ap, fmt);
  vfprintf (mgr->log, fmt, ap);
  va_end (ap);
  fputc ('\n', mgr->log);
}

/*------------------------------------------------------------------------*/

static int
is_var_letter (int ch)
{
  if (isalnum (ch))
    return 1;

  switch (ch)
    {
    case '-':
    case '_':
    case '.':
    case '[':
    case ']':
    case '$':
    case '@':
    case '+':
      return 1;

    default:
      return 0;
    }
}

/*------------------------------------------------------------------------*/

static void
enlarge_buffer (Mgr * mgr)
{
  mgr->buffer_size *= 2;
  mgr->buffer = (char *) realloc (mgr->buffer, mgr->buffer_size);
}

/*------------------------------------------------------------------------*/

static int
next_char (Mgr * mgr)
{
  int res;

  mgr->last_y = mgr->y;

  if (mgr->saved_char_is_valid)
    {
      mgr->saved_char_is_valid = 0;
      res = mgr->saved_char;
    }
  else
    res = mgr->form[mgr->pos++];

  if (res == '\n')
    {
      mgr->x++;
      mgr->y = 0;
    }
  else
    mgr->y++;

  return res;
}

/*------------------------------------------------------------------------*/

static void
unget_char (Mgr * mgr, int ch)
{
  assert (!mgr->saved_char_is_valid);

  mgr->saved_char_is_valid = 1;
  mgr->saved_char = ch;

  if (ch == '\n')
    {
      mgr->x--;
      mgr->y = mgr->last_y;
    }
  else
    mgr->y--;
}

/*------------------------------------------------------------------------*/

static void
next_token (Mgr * mgr)
{
  int ch;

  mgr->token = ERROR;
  ch = next_char (mgr);

RESTART_NEXT_TOKEN:

  while (isspace ((int) ch))
    ch = next_char (mgr);

  if (ch == '%')
    {
      while ((ch = next_char (mgr)) != '\n' && ch != '\0')
	;

      goto RESTART_NEXT_TOKEN;
    }

  mgr->token_x = mgr->x;
  mgr->token_y = mgr->y;

  if (ch == '\0')
    mgr->token = DONE;
  else if (ch == '<')
    {
      if (next_char (mgr) != '-')
	parse_error (mgr, "expected '-' after '<'");
      else if (next_char (mgr) != '>')
	parse_error (mgr, "expected '>' after '-'");
      else
	mgr->token = IFF;
    }
  else if (ch == '-')
    {
      if (next_char (mgr) != '>')
	parse_error (mgr, "expected '>' after '-'");
      else
	mgr->token = IMPLIES;
    }
  else if (ch == '&')
    {
      mgr->token = AND;
    }
  else if (ch == '|')
    {
      mgr->token = OR;
    }
  else if (ch == '!' || ch == '~')
    {
      mgr->token = NOT;
    }
  else if (ch == '(')
    {
      mgr->token = LP;
    }
  else if (ch == ')')
    {
      mgr->token = RP;
    }
  else if (is_var_letter (ch))
    {
      mgr->buffer_count = 0;

      while (is_var_letter (ch))
	{
	  if (mgr->buffer_size <= mgr->buffer_count + 1)
	    enlarge_buffer (mgr);

	  mgr->buffer[mgr->buffer_count++] = ch;
	  ch = next_char (mgr);
	}

      unget_char (mgr, ch);
      mgr->buffer[mgr->buffer_count] = 0;

      if (mgr->buffer[mgr->buffer_count - 1] == '-')
	parse_error (mgr, "variable '%s' ends with '-'", mgr->buffer);
      else
	mgr->token = VAR;
    }
  else
    parse_error (mgr, "invalid character '%c'", ch);
}

/*------------------------------------------------------------------------*/

static Node *parse_expr (Mgr *);

/*------------------------------------------------------------------------*/

static Node *
parse_basic (Mgr * mgr)
{
  Node *child;
  Node *res;

  res = 0;

  if (mgr->token == LP)
    {
      next_token (mgr);
      child = parse_expr (mgr);
      if (mgr->token != RP)
	{
	  if (mgr->token != ERROR)
	    parse_error (mgr, "expected ')'");
	}
      else
	res = child;
      next_token (mgr);
    }
  else if (mgr->token == VAR)
    {
      res = var (mgr, mgr->buffer);
      next_token (mgr);
    }
  else if (mgr->token != ERROR)
    parse_error (mgr, "expected variable or '('");

  return res;
}

/*------------------------------------------------------------------------*/

static Node *
parse_not (Mgr * mgr)
{
  Node *child, *res;

  if (mgr->token == NOT)
    {
      next_token (mgr);
      child = parse_not (mgr);
      if (child)
	res = op (mgr, NOT, child, 0);
      else
	res = 0;
    }
  else
    res = parse_basic (mgr);

  return res;
}

/*------------------------------------------------------------------------*/

static Node *
parse_associative_op (Mgr * mgr, Type type, Node * (*lower) (Mgr *))
{
  Node *res, *child;
  int done;

  res = 0;
  done = 0;

  do
    {
      child = lower (mgr);
      if (child)
	{
	  res = res ? op (mgr, type, res, child) : child;
	  if (mgr->token == type)
	    next_token (mgr);
	  else
	    done = 1;
	}
      else
	res = 0;
    }
  while (res && !done);

  return res;
}


/*------------------------------------------------------------------------*/

static Node *
parse_and (Mgr * mgr)
{
  return parse_associative_op (mgr, AND, parse_not);
}

/*------------------------------------------------------------------------*/

static Node *
parse_or (Mgr * mgr)
{
  return parse_associative_op (mgr, OR, parse_and);
}

/*------------------------------------------------------------------------*/

static Node *
parse_implies (Mgr * mgr)
{
  Node *l, *r;

  if (!(l = parse_or (mgr)))
    return 0;
  if (mgr->token != IMPLIES)
    return l;
  next_token (mgr);
  if (!(r = parse_or (mgr)))
    return 0;

  return op (mgr, IMPLIES, l, r);
}

/*------------------------------------------------------------------------*/

static Node *
parse_iff (Mgr * mgr)
{
  return parse_associative_op (mgr, IFF, parse_implies);
}

/*------------------------------------------------------------------------*/

static Node *
parse_expr (Mgr * mgr)
{
  return parse_iff (mgr);
}

/*------------------------------------------------------------------------*/

static int
parse (Mgr * mgr)
{
  next_token (mgr);

  if (mgr->token == ERROR)
    return 0;

  if (!(mgr->root = parse_expr (mgr)))
    return 0;

  if (mgr->token == DONE)
    return 1;

  if (mgr->token != ERROR)
    parse_error (mgr, "expected operator or '\0'");

  return 0;
}

/*------------------------------------------------------------------------*/

static void
unit_clause (Mgr * mgr, minisat_Lit a)
{
	minisat_solver *s=mgr->solver;
	minisat_addClause_begin(s);
	minisat_addClause_addLit(s,a);
	minisat_addClause_commit(s);
	return;
}

/*------------------------------------------------------------------------*/

static void
binary_clause (Mgr * mgr, minisat_Lit a, minisat_Lit b)
{
	minisat_solver *s=mgr->solver;
	if (a==minisat_negate(b))
		return; /* don't add trivial clauses */
	minisat_addClause_begin(s);
        minisat_addClause_addLit(s,a);
	minisat_addClause_addLit(s,b);
        minisat_addClause_commit(s);
	return;
}

/*------------------------------------------------------------------------*/

static void
ternary_clause (Mgr * mgr, minisat_Lit a, minisat_Lit b, 
		minisat_Lit c)
{
	minisat_solver *s=mgr->solver;
	if (a==minisat_negate(b) || c==minisat_negate(b) || a==minisat_negate(c))
		return; /* don't add trivial clauses */
	minisat_addClause_begin(s);
        minisat_addClause_addLit(s,a);
        minisat_addClause_addLit(s,b);
	minisat_addClause_addLit(s,c);
        minisat_addClause_commit(s);
        return;
}

/*------------------------------------------------------------------------*/

static void
tsetin (Mgr * mgr)
{
  int num_clauses;
  Node *p;
  minisat_solver *solver = mgr->solver;
  minisat_Lit *posLits;
  minisat_Var nvar;
  char *tn;
  num_clauses = 0;
  for (p = mgr->first; p; p = p->next_inserted)
    {
      p->idx = ++mgr->idx;

      if (mgr->dump && p->type == VAR)
	fprintf (mgr->out, "c %d %s\n", p->idx, p->data.as_name);

      switch (p->type)
	{
	case IFF:
	  num_clauses += 4;
	  break;
	case OR:
	case AND:
	case IMPLIES:
	  num_clauses += 3;
	  break;
	case NOT:
	  num_clauses += 2;
	  break;
	default:
	  assert (p->type == VAR);
	  break;
	}
    }

  mgr->posLits = malloc(sizeof(minisat_Lit)*(mgr->idx+1));
  /* TODO check malloc */

  mgr->idx2node = (Node **) calloc (mgr->idx + 1, sizeof (Node *));
  for (p = mgr->first; p; p = p->next_inserted)
  {
    mgr->idx2node[p->idx] = p;
    if (p->type == VAR)
    {
	tn=p->data.as_name;
	if (!red_isnegvar(tn))
		mgr->posLits[p->idx]=red_getposLit(mgr->env,tn);
	else
		mgr->posLits[p->idx]=red_getnegLit(mgr->env,tn);
    }
    else
    {
	nvar = minisat_newVar(solver);
	/* minisat_setFrozen(solver, nvar, minisat_l_True); */
	mgr->posLits[p->idx]=minisat_mkLit(nvar);
    }
  }
  if (mgr->dump)
    fprintf (mgr->out, "p cnf %d %u\n", mgr->idx, num_clauses + 1);

  posLits=mgr->posLits;

  for (p = mgr->first; p; p = p->next_inserted)
    {
      switch (p->type)
	{
	case IFF:
	  ternary_clause(mgr, posLits[p->idx],
			 minisat_negate(posLits[p->data.as_child[0]->idx]),
			 minisat_negate(posLits[p->data.as_child[1]->idx]));
	  ternary_clause (mgr, posLits[p->idx], 
			  posLits[p->data.as_child[0]->idx],
			  posLits[p->data.as_child[1]->idx]);
	  ternary_clause (mgr, minisat_negate(posLits[p->idx]), 
			  minisat_negate(posLits[p->data.as_child[0]->idx]),
			  posLits[p->data.as_child[1]->idx]);
	  ternary_clause (mgr, minisat_negate(posLits[p->idx]), 
			  posLits[p->data.as_child[0]->idx],
			  minisat_negate(posLits[p->data.as_child[1]->idx]));
	  break;
	case IMPLIES:
	  binary_clause (mgr,posLits[p->idx],posLits[p->data.as_child[0]->idx]);
	  binary_clause (mgr, posLits[p->idx], 
			 minisat_negate(posLits[p->data.as_child[1]->idx]));
	  ternary_clause (mgr, minisat_negate(posLits[p->idx]),
			  minisat_negate(posLits[p->data.as_child[0]->idx]),
			  posLits[p->data.as_child[1]->idx]);
	  break;
	case OR:
	  binary_clause (mgr, posLits[p->idx], 
			 minisat_negate(posLits[p->data.as_child[0]->idx]));
	  binary_clause (mgr, posLits[p->idx], 
			 minisat_negate(posLits[p->data.as_child[1]->idx]));
	  ternary_clause (mgr, minisat_negate(posLits[p->idx]),
			  posLits[p->data.as_child[0]->idx], 
			  posLits[p->data.as_child[1]->idx]);
	  break;
	case AND:
	  binary_clause (mgr, minisat_negate(posLits[p->idx]), 
			 posLits[p->data.as_child[0]->idx]);
	  binary_clause (mgr, minisat_negate(posLits[p->idx]), 
			 posLits[p->data.as_child[1]->idx]);
	  ternary_clause (mgr, posLits[p->idx],
			  minisat_negate(posLits[p->data.as_child[0]->idx]),
			  minisat_negate(posLits[p->data.as_child[1]->idx]));
	  break;
	case NOT:
	  binary_clause (mgr, posLits[p->idx], 
			 posLits[p->data.as_child[0]->idx]);
	  binary_clause (mgr, minisat_negate(posLits[p->idx]), 
			 minisat_negate(posLits[p->data.as_child[0]->idx]));
	  break;
	default:
	  assert (p->type == VAR);
	  break;
	}
    }

  assert (mgr->root);

  unit_clause (mgr, posLits[mgr->root->idx]);
}

/*------------------------------------------------------------------------*/

int
limboole (char *form, struct minisat_solver_t *solver, 
	  struct env *env)
{
  int max_decisions;
  int error;
  Mgr *mgr;

  error = 0;
  max_decisions = -1;

  mgr = init (form,solver);
  mgr->env = env;

  error = !parse (mgr);

  if (!error)
  {
      tsetin (mgr);
  }

  release (mgr);

  return error != 0;
}

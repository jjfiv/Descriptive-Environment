/*
Copyright (c) 2011, Charles Jordan <skip@alumni.umass.edu>

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
/* usemace.c
 * Skip Jordan
 *
 * Uses the first-order model-finder Mace4 to find models.
 * This is unfortunately not currently portable.
 * LADR is GPLv2, which many interpret to mean that dynamic linking
 * requires us to be GPL.  So, I execute mace4 as a separate process,
 * giving me the freedom to license as I wish.
 * This isn't possible in strict C89, so we use popen for now.
 *
 * chj	12/1/11		created
 * chj	12/4/11		xor
 * chj	12/6/11		only set(arithmetic) if needed (optimization)
 */

#include "parse.h"
#include "types.h"
#include "protos.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define INIT_COMMAND(s) \
        bufstate = yy_scan_string(s); \
        yyparse(); \
        do_cmd(cmdtree->l); \
        yy_delete_buffer(bufstate)

/* Tries to find a model of form with vocabulary voc, using Mace4.
 * Returns 0 if we didn't find a model, -1 if there was some error,
 * and saves the found model as name otherwise.
 */
int usemace(struct node *form, struct vocab *voc, char *name, int clock)
{
	FILE *m;
	char *inp;
	struct structure *str;
	struct rel_symbol *rel;
	struct cons_symbol *cons;
	struct id *hash_data;
	struct hnode_t *hnode;
	int n, t, formlength, arith;
	void *bufstate;
	char *tfn;
	char *mace;
	FILE *tmp;

	/* TODO: check that form is FO, of vocabulary voc, and that
         * name doesn't exist
         */

	if (hash_lookup(cur_env->id_hash, name))
	{
		err("53: %s already exists\n",name);
		return 0;
	}

	tfn = tmpnam(NULL);
	tmp = fopen(tfn,"w");

	if (!tmp)
	{
		err("54: Can't open temporary file %s for writing\n",tfn);
		return 0;
	}

	arith = need_arithmetic(form);
	if (arith)
		fprintf(tmp, "set(arithmetic).\n");
	fprintf(tmp,"formulas(assumptions).\n");
	if (arith) /* max uses arith */
		fprintf(tmp,"-(exists x (x>max)).\n"); /* max is max */
	print_mace(tmp,form);
	fprintf(tmp,".\nend_of_list.\n");
	fclose(tmp);
	mace = malloc(sizeof(char)*16+strlen(tfn));
	if (clock<=0)
		clock = 30;
	sprintf(mace,"mace4 -t %d -f %s",clock,tfn);
	m = popen(mace,"r");
        if (!m)
        {
                err("45: Unable to execute mace4.\n");
		remove(tfn);
                return -1;
        }
	n = get_mace_model_size(m);
	if (n==-1)
	{
		err("46: Mace Error.\n");
		pclose(m);
		remove(tfn);
		return 0;
	}
	else if (n==-2)
	{
		err("47: Sorry, Mace4 didn't find a model.\n");
		pclose(m);
		remove(tfn);
		return 0;
	}
	
	formlength = strlen(name)+16+strlen(voc->name)+1+
                     numdigits(n)+1; /* name:=new structure{vocab,N,*/
	for (rel=voc->rel_symbols; rel; rel=rel->next)
		formlength += strlen(rel->name)+1+numdigits(rel->arity)+
			      7; /* NAME:arity is \f, */
	for (cons=voc->cons_symbols; cons; cons=cons->next)
		formlength += strlen(cons->name)+4; /* NAME:=0, */
	formlength += 4; /* }.\n\0 */
	
	inp = malloc(sizeof(char)*formlength);
	if (!inp)
	{
		err("48: No memory.\n");
		pclose(m);
		remove(tfn);
		return -1;
	}

	sprintf(inp,"%s:=new structure{%s,%d",name,voc->name,n);
	for (rel=voc->rel_symbols; rel; rel=rel->next)
	{
		t=strlen(inp);
		sprintf(inp+t,",%s:%d is \\f",rel->name,rel->arity);
	}
	for (cons=voc->cons_symbols; cons; cons=cons->next)
	{
		t=strlen(inp);
		sprintf(inp+t,",%s:=0",cons->name);
	}
	strcat(inp,"}.\n");

	INIT_COMMAND(inp);
	free(inp);

	hnode = hash_lookup(cur_env->id_hash, name);
	hash_data = (struct id*)hnode_get(hnode);

	str = (struct structure *)hash_data->def;

	make_mace_model(str,m);

	pclose(m);
	remove(tfn);
	return 1;
}

/* returns the size of the model found by Mace4, where m is
 * the stdout of Mace4.
 * Returns -1 if there was an error, -2 if no model was found.
 * stream is AFTER the model size (i.e., "interpretation( N" is read)
 * when exiting.
 */
int get_mace_model_size(FILE *m)
{
	int c;
	int n=-2;
	while ((c=getc(m))!=EOF)
	{
		if (c=='M' && (c=getc(m))=='O' && (c=getc(m))=='D' &&
                    (c=getc(m))=='E' && (c=getc(m))=='L')
			while ((c=getc(m))!=EOF)
				if (c=='i' && (c=getc(m))=='n' 
                                    && (c=getc(m))=='t' && (c=getc(m))=='e'
				    && (c=getc(m))=='r' && (c=getc(m))=='p'
				    && (c=getc(m))=='r' && (c=getc(m))=='e'
				    && (c=getc(m))=='t' && (c=getc(m))=='a'
				    && (c=getc(m))=='t' && (c=getc(m))=='i'
				    && (c=getc(m))=='o' && (c=getc(m))=='n'
				    && (c=getc(m))=='(')
				{
					fscanf(m,"%d",&n);
					return n;
				}
		else if (c=='E' && (c=getc(m))=='R' && (c=getc(m))=='R'
			 && (c=getc(m))=='O' && (c=getc(m))=='R')
			return -1;
	}
	return n;
}
	
/* Prints form in Mace4 format */
int print_mace(FILE *m, struct node *form)
{
	struct node *tnode, *restr, *varlist, *phi, *relargs;
	switch (form->label)
	{
		case TRUE:
			fprintf(m,"$T");
			return 1;
		case FALSE:
			fprintf(m,"$F");
			return 1;
		case LT:
			fprintf(m,"(");
			t_print_mace(m,form->l);
			fprintf(m,")<(");
			t_print_mace(m,form->r);
			fprintf(m,")");
			return 1;
		case LTE:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")<=(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case NEQUALS:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")!=(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case EQUALS:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")=(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case NOT:
			fprintf(m,"-(");
			print_mace(m,form->l);
			fprintf(m,")");
			return 1;
		case AND:
			fprintf(m,"(");
                        print_mace(m,form->l);
                        fprintf(m,")&(");
                        print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case OR:
			fprintf(m,"(");
                        print_mace(m,form->l);
                        fprintf(m,")|(");
                        print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case IMPLIES:
			fprintf(m,"(");
                        print_mace(m,form->l);
                        fprintf(m,")->(");
                        print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case IFF:
			fprintf(m,"(");
                        print_mace(m,form->l);
                        fprintf(m,")<->(");
                        print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case XOR:
			fprintf(m,"-(");
			print_mace(m,form->l);
			fprintf(m,")<->(");
			print_mace(m,form->r);
			fprintf(m,")");
			return 1;
		case PRED:
			fprintf(m,"%s(",(char *)form->data);
			for (relargs = form->r; relargs; relargs=relargs->r)
			{
				t_print_mace(m,relargs->l);
				if (relargs->r)
					putc(',',m);
			}
			fprintf(m,")");
			return 1;	
		case EXISTS:
			restr = form->l->r;
			varlist = form->l->l;
			phi = form->r;
			for (tnode = varlist; tnode; tnode=tnode->r)
				fprintf(m,"exists %s ",(char *)tnode->data);
			putc('(',m);
			if (restr)
			{
				putc('(',m);
				print_mace(m,restr);
				fprintf(m,")&");
			}
			putc('(',m);
			print_mace(m,phi);
			fprintf(m,"))");
			return 1;
		case FORALL:
			restr = form->l->r;
                        varlist = form->l->l;
                        phi = form->r;
                        for (tnode = varlist; tnode; tnode=tnode->r)
                                fprintf(m,"all %s ",(char *)tnode->data);
                        putc('(',m);
                        if (restr)
                        {
                                putc('(',m);
                                print_mace(m,restr);
                                fprintf(m,")->");
                        }
                        putc('(',m);
                        print_mace(m,phi);      
                        fprintf(m,"))");
                        return 1;
		default:
			err("49: Sorry, finding a model only supports FO\n");
			return -1;
	}
	return -1; /* unreachable */
}

/* prints the term form in Mace4 format */
int t_print_mace(FILE *m, struct node *form)
{

	switch (form->label)
	{
		case MULT:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")*(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case PLUS:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")+(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case MINUS:
			fprintf(m,"(");
                        t_print_mace(m,form->l);
                        fprintf(m,")+-(");
                        t_print_mace(m,form->r);
                        fprintf(m,")");
                        return 1;
		case CONSTANT:
		case VAR:
			fprintf(m,"%s",(char *)form->data);
			return 1;
		case NUMBER:
			fprintf(m,"%d",form->ndata);
			return 1;
		default:
			err("50: Unknown operand (%d) in t_print_mace\n",
			    form->label);
			return -1;
	}
	return -1; /* unreachable */
}

/* make the structure str like the model m, where the stream
 * starts at the comma after the universe size
 */
int make_mace_model(struct structure *str, FILE *m)
{
	int c;
	char *n;
	int i, j;
	int a, size;
	struct constant *cons;
	struct relation *rel;
	int *tuple=NULL;

	n=malloc(sizeof(char)*512);
	/* TODO check the malloc */
	while ((c=getc(m))!=EOF)
	{
		if (c=='f' && (c=getc(m))=='u' && (c=getc(m))=='n' && 
                    (c=getc(m))=='c' && (c=getc(m))=='t' && (c=getc(m))=='i' &&
                    (c=getc(m))=='o' && (c=getc(m))=='n' && (c=getc(m))=='(')
		{
			fscanf(m,"%511s [ %d",n,&j);
			/* note that n will end in a comma, and may have (_,_),
			 * etc. if it's not a constant.
			 */
			for (i=0; n[i]; i++)
			{
				if (n[i]=='(') /* we have no function symbols */
				{
					n[0]=0; /* so ignore it */
					break;
				}
				else if (n[i]==',') /* constant */
				{
					n[i]=0; /* drop trailing comma */
					break;
				}
			}
			if (!(n[0]))
				continue;
			cons = get_constant(n,str);
			if (!cons)
				continue; /* this constant is not in the
					   * vocabulary.  Probably, it's a
					   * meaningless constant added by
					   * Mace.  Otherwise, user error.
					   */
			cons->value = j;
		}
		if (c=='r' && (c=getc(m))=='e' && (c=getc(m))=='l' &&
                    (c=getc(m))=='a' && (c=getc(m))=='t' && (c=getc(m))=='i' &&
                    (c=getc(m))=='o' && (c=getc(m))=='n' && (c=getc(m))=='(')
                {
			fscanf(m,"%511s [",n); /* n ends with "(_,_),"/etc. */
			for (i=0,a=0; n[i]; i++)
			{
				if (n[i]=='(') /* name ends at first ( */
					n[i]=0;
				else if (n[i]=='_') /* count arguments */
					a++;
			}
			rel = get_relation(n, NULL, str);
			if (!rel)
			{
				printf("51: Relation %s not in vocabulary\n",
				       n);
				continue;
			}
			if (a!=rel->arity)
			{
				printf("52: Arity of relation %s does not match vocabulary\n",
				       n);
				continue;
			}

			size = str->size;
			tuple = next_tuple(NULL, a, size);
			
			fscanf(m," %d",&i);
			rel->cache[0]=i;
			j=1;
			while ((tuple = next_tuple(tuple,a,size)))
			{
				fscanf(m, " , %d",&i);
				rel->cache[j++]=i;
			}
		}
	}
	return 1;
}

int need_arithmetic(struct node *form)
{
	int ret;
	struct node *tmp;
	switch (form->label)
	{
		case TRUE:
		case FALSE:
		case VAR:
		case NUMBER:
			return 0;
		case CONSTANT: /* we use < to handle max */
			return !strcmp(form->data,"max");
		case MULT:
		case PLUS:
		case MINUS:
		case LT: /* mace4 requires arithmetic for these */
		case LTE:
			return 1;
		case NOT:
			return need_arithmetic(form->l);
		case NEQUALS:
		case EQUALS:
		case AND:
		case IMPLIES:
		case OR:
		case IFF:
		case XOR:
			return (need_arithmetic(form->l) || 
			        need_arithmetic(form->r));
		case EXISTS:
		case FORALL:
			tmp = form->l->r;
			if (tmp)
				return (need_arithmetic(tmp) ||
					need_arithmetic(form->r));
			return need_arithmetic(form->r);
		case PRED:
			ret = 0;
			for (tmp=form->r; tmp; tmp=tmp->r)
				ret = (ret || need_arithmetic(tmp->l));
			return ret;
		case TC:
			ret = 0;
			for (tmp=form->r; tmp; tmp=tmp->r)
				ret = (ret || need_arithmetic(tmp->l));
			return (ret || need_arithmetic(form->l->r));
		case SOE:
			tmp = form->l->r;
			if (tmp)
				return (need_arithmetic(tmp) ||
					need_arithmetic(form->r));
			return need_arithmetic(form->r);
		case IFP:
			ret = 0;
			for (tmp=form->r->r; tmp; tmp=tmp->r)
				ret = (ret || need_arithmetic(tmp->l));
			return (ret || need_arithmetic(form->l->r));
		default:
			err("55: Unknown logical node (%d)\n",form->label);
			return 0;
	}
	return 0; /* unreachable */
}

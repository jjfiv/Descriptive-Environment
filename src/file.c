/* ISC LICENSE
 * file.c
 *
 * File-related (load/save) functions.
 * DIMACS support is informed by asc2bin.c and bin2asc.c, which were
 * Written by Tamas Badics (badics@rutcor.rutgers.edu),
 * using the technique of Marcus Peinado.
 *
 * chj	11/01/11	created (load DIMACS coloring ascii format)
 * chj	11/03/11	save
 * chj	12/05/11	draw with Marco's de_draw
 * chj	 1/28/13	loadstring
 */

#define _POSIX_C_SOURCE 2

#include "parse.h"
#include "types.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "protos.h"
#include <math.h>
#include <ctype.h>

#define INIT_COMMAND(s) \
  bufstate = yy_scan_string(s); \
yyparse(); \
do_cmd(cmdtree->l); \
yy_delete_buffer(bufstate)

/* Calls Marco's program to draw a structure */
int do_draw_command(struct node *command)
{
  char *name=command->l->data, *tfn;
  FILE *tmp, *m;
  struct structure *str;
  char *exec;

  struct id *hash_data;
  struct hnode_t *hnode = hash_lookup(cur_env->id_hash, name);
  int len;

  if (!hnode)
  {
    err("56: Nonexistent object %s\n",name);
    return 0;
  }

  hash_data = (struct id*)hnode_get(hnode);

  if (hash_data->type!=STRUC)
  {
    err("57: %s is not a structure\n",name);
    return 0;
  }

  str = hash_data->def;

  tfn = tmpnam(NULL);
  tmp = fopen(tfn,"w");

  if (!tmp)
  {
    err("58: Can't open temporary file %s for writing\n",tfn);
    return 0;
  }

  len = strlen(tfn);
  exec = malloc(sizeof(char)*(len+8));

  if (!len)
  {
    err("59: No memory\n");
    fclose(tmp);
    return 0;
  }

  save_struc(str, tmp);

  sprintf(exec, "de_draw %s",tfn);
  m = popen(exec, "r");
  if (!m)
    err("60: Unable to execute de_draw\n");
  pclose(m);
  remove(tfn);
  return 0;
}

/* based on the do_*_assign from cmd/cmd.c */
/* Loads a file.  Initially, this loads DIMACS graph coloring files
 * (uncompressed, undirected graphs).  The intent is to extend it
 * to loading compressed DIMACS format, and then a new format for
 * structures, and eventually for sessions (text file is okay), etc.
 */
/* This is done by first creating an empty graph of the proper size, and
 * then adding the edges as they appear.
 */
int do_load(struct node *command)
{
  void *bufstate;
  char *assign_id = command->l->data;
  char *fn = command->r->data;
  /* fn has a leading and trailing \" that we want to remove,
   * so -2 length +1 for the \0
   */
  int  fnl=strlen(fn);
  char *filename=malloc(fnl-1);
  FILE *f;
  int  i,j,k;
  char *buf;
  char tmp[8];
  int c;
  char oc;
  int n,m, n_digits,buflen;
  struct structure *str;
  struct relation *rel;
  struct id *hash_data;
  struct hnode_t *hnode;

  strncpy(filename,fn+1,fnl-2); 
  filename[fnl-2]='\0';
#if 0
  printf("In do_load, loading %s as %s\n",filename,assign_id);
#endif
  f = fopen(filename, "r");
  if (!f)
  {
    err("29: Unable to open file.\n");
    free(filename);
    return -1;
  }

  for (oc='\0'; (c=fgetc(f))!= EOF; oc=c)
    if (oc=='\n' && c=='p')
      break;

  if(1 != fscanf(f,"%7s",tmp))
    return -1;

  if (strcmp(tmp,"edge"))
  {
    err("30: Invalid file.\n");
    free(filename);
    fclose(f);
    return -1;
  }

  if(2 != fscanf(f,"%d %d",&n,&m))
    return -1;

  if (n<0 || m<0)
  {
    err("31: Invalid file.\n");
    free(filename);
    fclose(f);
    return -1;
  }

  /* need n digits to represent n */
  n_digits=numdigits(n);

  buflen = strlen(assign_id)+22+(n_digits<<1)+19+4;
  buf = malloc(sizeof(char)*buflen);
  /* TODO check malloc */

  sprintf(buf,"%s:=new structure{graph,%d,E:2 is \\f,s:=0,t:=%d}.\n",assign_id,n,n-1);
  INIT_COMMAND(buf); /* make a structure with empty adjacency matrix,
                      * now we fill it.
                      */

  hnode = hash_lookup(cur_env->id_hash, assign_id);
  hash_data = (struct id*)hnode_get(hnode);

  str = (struct structure *)hash_data->def;
  rel = get_relation("E", NULL, str);

  for (i=0; i<n*n;)
    rel->cache[i++]=0;

  /* now add the edges */
  /* TODO BOUNDS check to make sure i,j<n and also that at most m edges.
  */
  for (k=0, oc='\0'; (c=fgetc(f))!= EOF; oc=c)
    if (oc=='\n' && c=='e')
    {
      if(2 != fscanf(f,"%d %d",&i,&j))
        return -1;
      if (i>n)
      {
#ifdef DEBUG
        printf("DEBUG: Warning, edge to invalid node %d"
            " changed to %d\n",i,n-1);
#endif
        i=n;
      }
      if (j>n)
      {
#ifdef DEBUG
        printf("DEBUG: Warning, edge to invalid node %d"
            " changed to %d\n",j,n-1);
#endif
        j=n;
      }
      k++;
      if (k>m)
      {
        err("32: Warning, inconsistent file (too many edges)\n");
        break;
      }
      rel->cache[(i-1)*n+j-1]=1;
      rel->cache[(j-1)*n+i-1]=1; /* undirected */
    }

  free(buf);
  return 1;
}

int do_save_command(struct node *command)
{
  char *id=command->l->data;
  char *fn=command->r->data;
  FILE *f;
  int fnl=strlen(fn);
  char *filename=malloc(fnl-1);
  struct id *hash_data;
  struct hnode_t *hnode;

  hnode = hash_lookup(cur_env->id_hash, id);

  if (!hnode)
  {
    printf("37: Nonexistent object %s\n",id);
    return 0;
  }

  hash_data = (struct id*)hnode_get(hnode);

  strncpy(filename,fn+1,fnl-2);
  filename[fnl-2]='\0';

  f = fopen(filename, "r");
  if (f)
  {
    fclose(f);
    err("38: File already exists.\n");
    free(filename);
    return -1;
  }

  switch (hash_data->type)
  {
    case STRUC:
      f = fopen(filename, "w");
      return save_struc((struct structure *)hash_data->def,
          f);
    default:
      err("39: Saving this object is not supported.\n");
      free(filename);
      return -1;
  }
  fclose(f);
  free(filename);
  return 1;
}

/* saves s to FILE f (already opened for writing) */
int save_struc(struct structure *s, FILE *f)
{
  struct constant *con;
  struct relation *relation;
  int *tuple=NULL;
  int a,n=s->size,res,tuple_num;
  struct interp *interp=new_interp(s);

  fprintf(f,"c de 1\nn %s\np de %d 0\n",s->name,n);
  save_voc_inner(s->vocab,f);

  for (relation=s->rels; relation; relation=relation->next)
  {
    a=relation->arity;
    if (relation->cache)
      tuple_num = 0;
    else
      tuple_num = -1;
    while ((tuple = next_tuple(tuple, a, n)))
    {
      if (tuple_num>=0 && ((res=relation->cache[tuple_num++])>-1))
      {
        if (res)
          save_tuple_line(relation, tuple,
              f);
        continue;
      }

      interp = add_tup_to_interp(interp, tuple, a);
      if ((res=eval(relation->parse_cache, interp, s)))
        save_tuple_line(relation, tuple, f);
      interp = free_remove_tup(interp, a);
      if (tuple_num>=0)
        relation->cache[tuple_num-1]=res;
    }
  }

  for (con=s->cons; con; con=con->next)
    fprintf(f,"%s %d\n",con->name,con->value);

  fclose(f);
  free_interp(interp);

  return 1;
}

int save_tuple_line(struct relation *relation, int *tuple, FILE *f)
{
  int i,a=relation->arity;
  fprintf(f,"%s ",relation->name);
  for (i=0; i<a; i++)
    fprintf(f,"%d ",tuple[i]);
  fputc('\n',f);
  return 1;
}

/* save voc to f with no header, f already open for writing */
int save_voc_inner(struct vocab *voc, FILE *f)
{
  struct cons_symbol *con;
  struct rel_symbol *rel;
  fprintf(f, "v %s\nv ",voc->name);
  for (rel = voc->rel_symbols; rel; rel=rel->next)
    fprintf(f,"%s:%d ",rel->name, rel->arity);
  fprintf(f,"\nv ");
  for (con = voc->cons_symbols; con; con=con->next)
    fprintf(f,"%s ",con->name);
  fputc('\n',f);
  return 1;
}	

/* based on do_load */
/* loads a file as a string, all characters must be alphabetic and
 * no difference between upper/lower-case.
 * The vocabulary is implicit, characters that occur are monadic predicates.
 */
int do_loadassign(struct node *command)
{
  void *bufstate;
  char *assign_id = command->l->data;
  char *fn=command->r->r->data;
  /* fn has a leading and trailing \" that we want to remove,
   * so -2 length +1 for the \0
   */
  char *vocname=command->r->l->data;
  int  fnl=strlen(fn);
  char *filename=malloc(fnl-1);
  FILE *f;
  int  i;
  char *buf;
  int c;
  struct structure *struc;
  struct vocab *voc;
  struct id *hash_data;
  struct hnode_t *hnode;
  int count=0;
  char *str;
  struct rel_symbol *rs;

  strncpy(filename,fn+1,fnl-2);
  filename[fnl-2]='\0';
#if 0
  printf("In do_loadstring, loading %s as %s\n",filename,assign_id);
#endif
  f = fopen(filename, "r");
  if (!f)
  {
    err("??: Unable to open file.\n");
    free(filename);
    return -1;
  }

  for (c='\0'; (c=fgetc(f)) != EOF;)
  {
    if (c=='\n')
      break;
    if (!isalpha(c))
    {
      printf("??: Invalid character (%c) in file.\n",c);
      fclose(f);
      return -1;
    }
    count++;
  }

  str = malloc(sizeof(char)*(count+1));
  rewind(f);

  for (i=0; i<=count; i++)
    str[i]=fgetc(f);

  fclose(f);

  hnode = hash_lookup(cur_env->id_hash, vocname);
  if (!hnode)
  {
    printf("??: Vocabulary %s doesn't exist.\n",vocname);
    free(str);
    return -1;
  }

  hash_data = (struct id*)hnode_get(hnode);
  if (hash_data->type != VOCAB)
  {
    printf("??: %s is not a vocabulary.\n",vocname);
    free(str);
    return -1;
  }

  voc = (struct vocab *)hash_data->def;

  if (voc->cons_symbols)
  {
    printf("??: loadstring doesn't support constants.\n");
    free(str);
    return -1;
  }
  if (!voc->rel_symbols)
  {
    printf("??: loadstring requires predicate symbols\n");
    free(str);
    return -1;
  }
  for (rs=voc->rel_symbols; rs; rs=rs->next)
  {
    if (rs->arity!=1)
    {
      printf("??: loadstring supports only monadic predicates.\n");
      free(str);
      return -1;
    }
  }

  buf = loadstring_getdec(assign_id,count,voc);
  INIT_COMMAND(buf); /* make a long, empty string */
  free(buf);

  hnode = hash_lookup(cur_env->id_hash, assign_id);
  hash_data = (struct id*)hnode_get(hnode);

  struc = (struct structure *)hash_data->def;

  loadstring_convert(struc,count,str);
  free(str);

  return 1;
}

/* return "assign_id is new structure{voc->name,count, ...}" */
char *loadstring_getdec(char *assign_id, int count, struct vocab *voc)
{
  int len;
  char *ret;
  struct rel_symbol *rs;

  len = strlen(assign_id)+18+strlen(voc->name)+1+numdigits(count)+1;

  for (rs=voc->rel_symbols; rs; rs=rs->next)
    len+=strlen(rs->name)+1+numdigits(rs->arity)+7;

  len+=4; /* }.\n\0 */

  ret = malloc(sizeof(char)*len);
  sprintf(ret,"%s is new structure{%s,%d,",assign_id,voc->name,count);

  for (rs=voc->rel_symbols; rs; rs=rs->next)
  sprintf(ret+strlen(ret),"%s:1 is \\f,",rs->name);

  sprintf(ret+strlen(ret)-1,"}.\n");
  return ret;
  }


void loadstring_convert(struct structure *struc, int count, char *str)
{
  struct relation *rel;
  int i;
  char c[2];

  c[1]='\0';
  for (i=0; i<count; i++)
  {
    c[0]=toupper(str[i]);
    for (rel=struc->rels; rel; rel=rel->next)
    {
      if (!strcmp(rel->name,c))
      {
        rel->cache[i]=1;
        break;
      }
    }
  }

  for (rel=struc->rels; rel; rel=rel->next)
    for (i=0; i<count; i++)
      if (rel->cache[i]==-1)
        rel->cache[i]=0;
  return;	
}








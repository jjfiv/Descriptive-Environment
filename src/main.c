// ISC LICENSE

#include "parse.h"
#include <stdio.h>
#include "protos.h"
#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <string.h>
#include "protos.h"
#include "types.h"
#include "parse.h"

static void init_command(const char *str) {
  void *bufstate = yy_scan_string(str);
  yyparse();
  do_cmd(cmdtree->l);
  yy_delete_buffer(bufstate);
}

static void init_env(void)
{
  printf("Welcome to the DescriptiveEnvironment (DE).\n");

  cur_env = malloc(sizeof(struct env));
  if (!cur_env)
    return 0;

  cur_env->id_hash = hash_create(MAX_IDS, (hash_comp_t)strcmp, 0);	
  cur_env->next_id=0;
  init_command("sat is new vocabulary{P:2, N:2}.\n");
  init_command("minisat is new bquery{sat, \\t}.\n");
  init_command("graph is new vocabulary{E:2,s,t}.\n");
  init_command("threecolorwithsat is new bquery{graph, \\t}.\n");
  init_command("minisat2 is new bquery{sat, \\t}.\n");
  init_command("threecolorwithsat2 is new bquery{graph, \\t}.\n");
}

int main(int argc, char **argv)
{
  init_env();
  command_loop();

  return 0;
}


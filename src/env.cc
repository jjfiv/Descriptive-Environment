// ISC LICENSE

#include "types.h"
#include "parse.h"
#include "protos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static Environment gbl_env;
Environment *cur_env = &gbl_env;

void runCommand(string cmd) {
  if(cmd == "" || cmd == "quit" || cmd == "help") 
    return;

  if(cmd.back() != '\n')
    cmd += '\n';

  // use string as command
  YY_BUFFER_STATE bufstate = yy_scan_string(cmd.c_str());
  yyparse();
  do_cmd(cur_env, cmdtree->l);
  yy_delete_buffer(bufstate);
}

void init_command(const char *str) {
  YY_BUFFER_STATE bufstate = yy_scan_string(str);
  yyparse();
  do_cmd(cur_env, cmdtree->l);
  yy_delete_buffer(bufstate);
}

void init_env(void)
{
  if (!cur_env)
    return;

  cur_env->id_hash = hash_create(MAX_IDS, (hash_comp_t)strcmp, 0);	
  cur_env->next_id=0;
  init_command("sat is new vocabulary{P:2, N:2}.\n");
  init_command("minisat is new bquery{sat, \\t}.\n");
  init_command("graph is new vocabulary{E:2,s,t}.\n");
  init_command("threecolorwithsat is new bquery{graph, \\t}.\n");
  init_command("minisat2 is new bquery{sat, \\t}.\n");
  init_command("threecolorwithsat2 is new bquery{graph, \\t}.\n");
}


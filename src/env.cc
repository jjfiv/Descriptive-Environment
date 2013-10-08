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
  int rc = yyparse();
  if(0 != rc) {
    cerr << "failed to parse: " << cmd;
    return;
  }
  do_cmd(cur_env, cmdtree->l);
  yy_delete_buffer(bufstate);
}

void init_env(void)
{
  if (!cur_env)
    return;

  cur_env->id_hash = hash_create(MAX_IDS, (hash_comp_t)strcmp, 0);	
  cur_env->next_id=0;
  runCommand("sat is new vocabulary{P:2, N:2}.\n");
  runCommand("minisat is new bquery{sat, \\t}.\n");
  runCommand("graph is new vocabulary{E:2,s,t}.\n");
  runCommand("threecolorwithsat is new bquery{graph, \\t}.\n");
  runCommand("minisat2 is new bquery{sat, \\t}.\n");
  runCommand("threecolorwithsat2 is new bquery{graph, \\t}.\n");
}

Identifier* getBinding(Environment *env, string name) {
  hnode_t *hd = hash_lookup(env->id_hash, name.c_str());
  if(!hd) return nullptr;
  return (Identifier*) hnode_get(hd);
}

BQuery* getBQuery(Environment *env, string name) {
  Identifier *id = getBinding(env, name);
  if(!id) {
    err("18: Query %s does not exist\n",name.c_str());
    return nullptr;
  }
  if(id->type != BQUERY) {
    err("19: %s is not a query\n",name.c_str());
    return nullptr;
  }
  return (BQuery *)id->def;
}

Structure* getStructure(Environment *env, string name) {
  Identifier *id = getBinding(env, name);
  if(!id) {
    err("20: Structure %s does not exist\n",name.c_str());
    return nullptr;
  }
  if(id->type != STRUC) {
    err("21: %s is not a structure\n",name.c_str());
    return nullptr;
  }
  return (Structure *)id->def;
}

Vocabulary* getVocab(Environment *env, string name) {
  Identifier *id = getBinding(env, name);
  if(!id) {
    err("22: Vocabulary %s does not exist\n",name.c_str());
    return nullptr;
  }
  if(id->type != VOCAB) {
    err("23: %s is not a vocabulary.\n",name.c_str());
    return nullptr;
  }
  return (Vocabulary *)id->def;
}

Reduction* getReduction(Environment *env, string name) {
  Identifier *id = getBinding(env, name);
  if(!id) {
    err("22: Reduction %s does not exist\n",name.c_str());
    return nullptr;
  }
  if(id->type != REDUC) {
    err("23: %s is not a reduction.\n",name.c_str());
    return nullptr;
  }
  return (Reduction *)id->def;
}

void removeBinding(Environment *env, string name) {
  hnode_t *hd = hash_lookup(env->id_hash, name.c_str());
  if(!hd) return;
  hash_delete_free(env->id_hash, hd);
}

// todo: make this safer
string gensym(Environment *env) {
  char nameBuf[32];
  for(char c='A'; c<='Z'; c++) {
    for(int t=0; t<999; t++) {
      sprintf(nameBuf,"%c%d",c,t);
      if(!getBinding(cur_env, nameBuf))
        return string(nameBuf);
    }
  }
  return "";
}

Identifier* makeBinding(Environment *env, char* name, int type, void *data) {
  Identifier *id = (Identifier*) malloc(sizeof(Identifier));
  if(!id) {
    cerr << "makeBinding::malloc error!\n";
    return nullptr;
  }
  id->name = name;
  id->def = data;
  id->type = type;

  if(!hash_alloc_insert(env->id_hash, id->name, id)) {
    // TODO hash_alloc_insert error
    cerr << "makeBinding::hash_alloc_insert error!\n";
    return nullptr;
  }

  return id;
}





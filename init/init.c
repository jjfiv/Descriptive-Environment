/*
Copyright (c) 2006-2012, Charles Jordan <skip@alumni.umass.edu>

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
/* init.c
 * Skip Jordan
 *
 * Initialize the environment.
 * chj	11/14/06	created
 * chj	 3/12/07	zchaff
 */

#include <stdlib.h>
#include <stdio.h>
#include "types.h"
#include <string.h>
#include "protos.h"
#include "types.h"
#include "parse.h"

#define INIT_COMMAND(s) \
	bufstate = yy_scan_string(s); \
	yyparse(); \
	do_cmd(cmdtree->l); \
	yy_delete_buffer(bufstate)


int init_env(void)
{
	void *bufstate;
	printf("Welcome to the DescriptiveEnvironment (DE).\n");

	cur_env = malloc(sizeof(struct env));
	if (!cur_env)
		return 0;

	cur_env->id_hash = hash_create(MAX_IDS, (hash_comp_t)strcmp, 0);	
	cur_env->next_id=0;
	INIT_COMMAND("sat is new vocabulary{P:2, N:2}.\n");
	INIT_COMMAND("minisat is new bquery{sat, \\t}.\n");
	INIT_COMMAND("graph is new vocabulary{E:2,s,t}.\n");
	INIT_COMMAND("threecolorwithsat is new bquery{graph, \\t}.\n");
#ifdef ZCHAFF
	INIT_COMMAND("zchaff is new bquery{sat, \\t}.\n");
	INIT_COMMAND("threecolorwithchaff is new bquery{graph, \\t}.\n");
#endif
#ifdef MINISAT2
	INIT_COMMAND("minisat2 is new bquery{sat, \\t}.\n");
	INIT_COMMAND("threecolorwithsat2 is new bquery{graph, \\t}.\n");
#endif
	
	return 1;
}

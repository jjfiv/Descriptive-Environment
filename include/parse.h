/*
Copyright (c) 2006-2011, Charles Jordan <skip@alumni.umass.edu>

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
/* parse.h
 * Skip Jordan
 * Header for parse.c
 *
 * chj	10/19/06	Created.
 * chj  11/14/06	Fix for byacc-1.9 compatibility.
 */

struct node {
	struct node *l;
	struct node *r;
	void *data;
	int *ival;
	int ndata;
	int label;
};

extern struct node *parsetree;
extern struct node *cmdtree;

struct node *node(int, struct node*, struct node*);
struct node *inode(int, int);
struct node *snode(int, char *);
struct node *vlnode(int, char*, struct node *);
struct node *qnode(int, struct node*, struct node*, struct node*);
struct node *arg_node(int, struct node *);
struct node *fournode(int, struct node*, struct node*, struct node*, struct node*);


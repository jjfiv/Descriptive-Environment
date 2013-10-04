/*
Copyright (c) 2013, John Foley <jfoley@cs.umass.edu>
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

#include <stdlib.h>
#include <stdio.h>

/* The user has just typed inp, which begins with help */
int do_help(const char *inp)
{
	if (!inp) return 1; /* quick hack to remove a warning on -Wextra */
	printf("\n"
"This is the Descriptive Environment (DE) described in\n"
"\"Experimental Descriptive Complexity\" (2012).\n"
"\n"
"The following are built-in:\n"
" - sat is a vocabulary, {P:2(c,v), N:2(c,v)}.\n"
" - graph is a vocabulary, {E:2, s, t}.\n"
" - minisat( ) is a boolean query on structures with\n"
"   vocabulary sat that uses the MiniSat SAT solver.\n"
" - threecolorwithsat is a boolean query on graphs using\n" 
"   a reduction to sat and the MiniSat SAT solver.\n"
"\n"
"Type \"quit\" to quit.\n"
"See the README, and the paper and file \"tests\" for examples.\n\n");
	return 1;
}

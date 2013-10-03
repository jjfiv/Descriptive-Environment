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
/* util.c
 * Skip Jordan
 *
 * Utility functions.
 * chj	11/16/06 created
 */

/* strdup implementation.  strdup is non-ANSI and a reserved name for string.h */

#include "types.h"
#include <string.h>
#include <stdlib.h>

char *dupstr(const char *inp)
{
	int len = strlen(inp);
	char *copy = malloc(sizeof(char)*(len+1));

	if (!copy)
		return copy;

	copy[len] = '\0';

	for (len--; len>=0; len--)
		copy[len] = inp[len];

	return copy;
}


/* Frees the list - note that it calls free on all data elements, but
 * doesn't know what they are (void *) and so doesn't free anything
 * inside/pointed-to-by data.
 */
struct list *free_list(struct list *list)
{
	struct list *t1, *t2;
	for (t1=list; t1; t1=t2)
	{
		t2=t1->next;
		free(t1->data);
		free(t1);
	}
	return NULL;
}

/* returns n^k, be careful with overflows 
 * courtesy of an old stackoverflow post
 */
long de_pow(int x, short p)
{
	int n;
	long tmp;

	if (p == 0)
		return 1;
        if (p == 1)
		return x;
        n = 15;
        while ((p <<= 1) >= 0) n--;

        tmp = x;
        while (--n > 0)
        	tmp = tmp * tmp * (((p <<= 1) < 0)? x : 1);
        return tmp;
}


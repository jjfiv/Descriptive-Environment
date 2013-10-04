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
/* constant.c
 * Skip Jordan
 *
 * Various functions related to constants
 * chj 11/16/06	created
 */

#include "protos.h"
#include "types.h"
#include <string.h>

struct constant *get_constant(const char *name,
    const struct structure *struc)
{
  struct constant *cons = struc->cons;

  while (cons && strcmp(cons->name, name))
    cons=cons->next;
  return cons;
}

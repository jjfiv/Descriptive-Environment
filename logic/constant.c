// ISC LICENSE
/* constant.c
 * Various functions related to constants
 */

#include "protos.h"
#include "types.h"
#include <string.h>

Constant *get_constant(const char *name, const Structure *struc) {
  Constant *cons = struc->cons;

  while (cons && strcmp(cons->name, name))
    cons=cons->next;

  return cons;
}

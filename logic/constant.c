// ISC LICENSE
/* constant.c
 * Various functions related to constants
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

// ISC LICENSE
/* constant.c
 * Various functions related to constants
 */

#include "protos.h"
#include "types.h"
#include <string>
using std::string;

Constant *get_constant(const char *name, const Structure *struc) {
  const string qname(name);

  for(Constant *cons = struc->cons; cons; cons=cons->next) {
    if(cons->name == qname)
      return cons;
  }
  return nullptr;
}

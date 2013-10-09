// ISC LICENSE
/* constant.c
 * Various functions related to constants
 */

#include "protos.h"
#include "types.h"
#include <string>
using std::string;

Constant *get_constant(const string &name, const Structure *struc) {
  for(Constant *cons = struc->cons; cons; cons=cons->next) {
    if(cons->name == name)
      return cons;
  }
  return nullptr;
}

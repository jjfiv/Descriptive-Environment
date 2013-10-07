// ISC LICENSE

#include "parse.h"
#include "protos.h"
#include "types.h"
#include <stdio.h>
#include "protos.h"
#include "types.h"


int main(int argc, char **argv)
{
  init_env();
  printf("Welcome to the DescriptiveEnvironment (DE).\n");
  command_loop();

  return 0;
}


//ISC LICENSE

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

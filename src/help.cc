//ISC LICENSE

#include <cstdlib>
#include <cstdio>

void do_help(void)
{
  printf(
      "\n"
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
      "See the README, and the paper and file \"tests\" for examples.\n\n"
      );
}

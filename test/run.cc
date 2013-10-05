#include <cstdio>
#include <iostream>
#include <string>
using std::cout;
using std::string;

extern "C" {
#include "protos.h"
}

void testCommand(const string &str) {
  cout << "testCommand\t" << str;
  do_cmd_str(str.c_str(), str.size());
}

int main(int argc, char **argv) {
  puts("Hello World!");
  init_env();

  // line test
  testCommand("line := new structure{graph, 20, E:2 := x2=x1+1, s:=0, t:=19}.\n");
  testCommand("line.t.\n");
  testCommand("line.s.\n");
  testCommand("line.E.\n");

  // primes test
  testCommand("set := new vocabulary{S:1}.\n");
  testCommand("primes := new structure{set,1000, S:1 is (\\A x, y.(x<=x1 & y<=x) : ((x*y=x1)->(x=1|y=1)))}.\n");
  testCommand("primes.S.\n");


  return 0;
}


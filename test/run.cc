#include "protos.h"
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
using std::cout;
using std::string;

int main(int argc, char **argv) {

  std::ifstream fp;
  fp.open("test/input.de");

  init_env();
  
  while(fp) {
    string line;
    getline(fp, line);

    string trimmed = simplify(line);
    if(trimmed == "" || trimmed[0] == '#') continue;
    
    cout << "> " << trimmed << "\n";
    runCommand(cur_env, trimmed);
  }


  return 0;
}


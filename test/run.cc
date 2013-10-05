#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
using std::cout;
using std::string;

extern "C" {
#include "protos.h"
}

void testCommand(const string &str) {
  string withNewline = str + '\n';
  do_cmd_str(withNewline.c_str(), withNewline.size());
}

// compact and trim spaces
string simplify(const string &str) {
  string out;
  bool lastSpace = true;
  for(char c : str) {
    if(c <= ' ' && !lastSpace) {
      out += ' ';
      lastSpace = true;
    } else if(c > ' ') {
      lastSpace = false;
      out += c;
    }
  }
  if(lastSpace && out.size() > 0) {
    out.pop_back();
  }
  return out;
}

int main(int argc, char **argv) {

  std::ifstream fp;
  fp.open("test/input.de");

  init_env();
  
  while(fp) {
    string line;
    getline(fp, line);
    
    string trimmed = simplify(line);
    if(trimmed.size() == 0) {
      continue;
    }

    cout << "> " << trimmed << "\n";
    testCommand(trimmed);
  }


  return 0;
}


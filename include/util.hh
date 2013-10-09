#ifndef _UTIL_HH
#define _UTIL_HH

#include <string>
#include <iostream>
using std::string;

#define show(x) \
  std::cout << __FILE__ ":" << __LINE__ << ": " #x << " = " << x << '\n'

typedef struct List List;

char *dupstr(const char *);
long de_pow(int x, short p);
List *free_list(List *);
string simplify(const string &);
string stringf(const char *fmt, ...);
string temporaryFileName();

#endif


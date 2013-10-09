#ifndef _UTIL_HH
#define _UTIL_HH

#include <string>
using std::string;

typedef struct List;

char *dupstr(const char *);
long de_pow(int x, short p);
List *free_list(List *);
string simplify(const string &);
string stringf(const char *fmt, ...);
string temporaryFileName();

#endif


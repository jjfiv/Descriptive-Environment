// ISC LICENSE

#include "types.h"
#include "protos.h"
#include <cstring>
#include <cstdlib>
#include <cstdarg>

/* strdup implementation.  strdup is non-ANSI and a reserved name for string.h */
char *dupstr(const char *inp)
{
  int len = strlen(inp);
  char *copy = (char*) malloc(sizeof(char)*(len+1));

  if (!copy)
    return copy;

  copy[len] = '\0';

  for (len--; len>=0; len--)
    copy[len] = inp[len];

  return copy;
}


/* Frees the list - note that it calls free on all data elements, but
 * doesn't know what they are (void *) and so doesn't free anything
 * inside/pointed-to-by data.
 */
List *free_list(List *list)
{
  List *t1, *t2;
  for (t1=list; t1; t1=t2)
  {
    t2=t1->next;
    free(t1->data);
    free(t1);
  }
  return NULL;
}

/* returns n^k, be careful with overflows 
 * courtesy of an old stackoverflow post
 */
long de_pow(int x, short p)
{
  int n;
  long tmp;

  if (p == 0)
    return 1;
  if (p == 1)
    return x;
  n = 15;
  while ((p <<= 1) >= 0) n--;

  tmp = x;
  while (--n > 0)
    tmp = tmp * tmp * (((p <<= 1) < 0)? x : 1);
  return tmp;
}

/** compact and trim spaces */
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

string stringf(const char* fmt, ...) {
  string buf;
  int len = 0;
  va_list args;

  // calculate length
  va_start(args, fmt);
  len = vsnprintf(nullptr, 0, fmt, args)+1;
  va_end(args);

  // resize buf appropriately
  buf.resize(len);

  // fill in data
  va_start(args, fmt);
  len = vsnprintf(&buf[0], buf.size(), fmt, args);
  va_end(args);

  // chop null
  buf.resize(len); 

  return buf;
}


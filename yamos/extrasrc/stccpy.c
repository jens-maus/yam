#include "extra.h"

/* Copy the q to the n chars buffer pointed by p.
   The result is null terminated.
   Returns the number of copied bytes, including '\0'. */

int stccpy(char *p, const char *q, int n)
{
   char *t = p;
   while ((*p++ = *q++) && --n > 0);
   p[-1] = '\0';
   return p - t;
}

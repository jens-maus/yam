#include <ctype.h>
#include "extra.h"

/* Skip white space. */

char *stpblk(const char *p)
{
   while (*p && isspace(*p))
      ++p;
   return (char *)p;
}

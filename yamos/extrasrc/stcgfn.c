#include <string.h>
#include "extra.h"

/* Get the base name. */

int stcgfn(char *node, const char *name)
{
   const char *p = name + strlen(name);
   const char *q = p;
   while (p > name && p[-1] != '/' && p[-1] != ':')
      --p;
   if (q - p < FNSIZE)
   {
      memcpy(node, p, q - p + 1);
      return q - p;
   }
   *node = '\0';
   return 0;
}

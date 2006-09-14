#include <string.h>

/* Get the filename extension. */

int stcgfe(char *ext, const char *name)
{
   const char *p = name + strlen(name);
   const char *q = p;
   while (p > name && *--p != '.' && *p != '/' && *p != ':');
   if (*p++ == '.' && q - p < FESIZE)
   {
      memcpy(ext, p, q - p + 1);
      return q - p;
   }
   *ext = '\0';
   return 0;
}

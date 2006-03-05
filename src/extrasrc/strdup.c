#include <stdlib.h>
#include <string.h>
#include "extra.h"

char *
strdup(const char *str)
{
  if (str) {
    char *s = malloc(1+strlen(str));
    if (s) {
      strcpy(s,str);
      return s;
    }
  }

  return NULL;
}

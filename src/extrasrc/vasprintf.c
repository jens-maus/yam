#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int vasprintf(char **ptr, const char * format, va_list ap)
{
  int ret;

  ret = vsnprintf(NULL, 0, format, ap);
  if(ret <= 0) return ret;

  (*ptr) = (char *)malloc(ret+1);
  if(!*ptr) return -1;
  ret = vsnprintf(*ptr, ret+1, format, ap);

  return ret;
}

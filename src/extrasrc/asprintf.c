#include <stdarg.h>
#include <stdio.h>

int asprintf(char **ptr, const char * format, ...)
{
  va_list ap;
  int ret;

  *ptr = NULL;
  va_start(ap, format);
  ret = vasprintf(ptr, format, ap);
  va_end(ap);

  return ret;
}

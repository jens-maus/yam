#include <stdarg.h>
#include <stdio.h>

int snprintf(char *s, size_t maxlen, const char *format, ...)
{
  int ret;
  va_list ap;

  va_start(ap, format);
  ret = (int)vsnprintf(s, maxlen, format, ap);
  va_end(ap);

  return ret;
}

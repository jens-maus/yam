#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#include <stdarg.h>
#include <stdio.h>

int VARARGS68K snprintf(char *s, size_t maxlen, const char *format, ...)
{
  VA_LIST ap;
  int ret;

  VA_START(ap, format);
  ret = (int)vsnprintf(s, maxlen, format, ap);
  VA_END(ap);

  return ret;
}

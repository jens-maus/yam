#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#include <stdarg.h>
#include <stdio.h>

int VARARGS68K asprintf(char **ptr, const char * format, ...)
{
  VA_LIST ap;
  int ret;

  *ptr = NULL;
  VA_START(ap, format);
  ret = vasprintf(ptr, format, ap);
  VA_END(ap);

  return ret;
}

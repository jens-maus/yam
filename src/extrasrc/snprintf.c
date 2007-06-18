#include <stdarg.h>

/// snprintf
//  length limited version of sprintf() with variable arguments
int snprintf(char *buffer, size_t maxlen, const char *fmt, ...)
{
  va_list args;
  int result;

  va_start(args, fmt);
  result = vsnprintf(buffer, maxlen, fmt, args);
  va_end(args);

  return result;
}
///


#include <string.h>

/* Append a file name to a path. */

void strmfp(char *name, const char *path, const char *node)
{
  size_t len = path ? strlen(path) : 0;
  if (len)
  {
    memcpy(name, path, len);
    if (name[len-1] != '/' && name[len-1] != ':')
      name[len++] = '/';
  }
  strcpy(name + len, node);
}

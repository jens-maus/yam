#include <string.h>
#include "extra.h"

/* Build a filename from its components */

void strsfn(const char *file, char *drive, char *path, char *node, char *ext)
{
  const char *end = file + strlen(file);
  const char *p = file;

  while (*p && *p != ':')
     ++p;
  if (*p++ == ':')
  {
    if (drive)
    {
      memcpy(drive, file, p - file);
      drive += p - file;
    }
    file = p;
  }
  if (drive)
    *drive = '\0';

  p = end;
  while (p > file && p[-1] != '.' && p[-1] != '/')
     --p;
  if (ext)
  {
    if (p > file && end - p < FESIZE)
      memcpy(ext, p, end - p + 1);
    else
      *ext = '\0';
  }
  if (p > file)
    end = p - 1;

  p = end;
  while (p > file && p[-1] != '/')
    --p;
  if (node)
  {
    if (end > p && end - p < FNSIZE)
    {
      memcpy(node, p, end - p);
      node += end - p;
    }
    *node = '\0';
  }
  end = p > file ? p - 1 : p;

  if (path)
  {
    if (end > p && end - p < FMSIZE)
    {
      memcpy(path, p, end - p);
      path += end - p;
    }
    *path = '\0';
  }
}

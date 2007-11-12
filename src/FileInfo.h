#ifndef FILEINFO_H
#define FILEINFO_H 1

#include <exec/types.h>

enum FType
{
  FIT_NONEXIST = 0,
  FIT_UNKNOWN,
  FIT_FILE,
  FIT_DRAWER
};

enum FileInfo
{
  FI_SIZE = 0 ,
  FI_PROTECTION,
  FI_COMMENT,
  FI_DATE,
  FI_TIME,
  FI_TYPE,
};

BOOL ObtainFileInfo(const char *name, enum FileInfo which, void *valuePtr);

#endif /* FILEINFO_H */

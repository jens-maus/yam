#include <proto/exec.h>
#include "SDI_compiler.h"

struct FormatContext
{
  char *fc_Index;
  size_t fc_Size;
};

/// StuffChar
// stuff a single character in the given string obeying the maximum length
static void ASM StuffChar( REG(a3, struct FormatContext *fc),
                           REG(d0, UBYTE c) )
{
  // is there still room?
  if(fc->fc_Size > 0)
  {
    (*fc->fc_Index) = c;
    fc->fc_Index++;
    fc->fc_Size--;

    // if there only a single character left we just add the NUL char
    // and don't accept any further chars
    if(fc->fc_Size == 1) {
      (*fc->fc_Index) = '\0';
      fc->fc_Size = 0;
    }
  }
}
///
/// vsnprintf
//  length limited version of vsprintf()
int vsnprintf(char *buffer, size_t maxlen, const char *fmt, va_list args)
{
  struct FormatContext fc;
  int result;

  fc.fc_Index = buffer;
  fc.fc_Size = maxlen;

  // put in the data
  RawDoFmt(fmt, (APTR)args, (void (*)())StuffChar, (APTR)&fc);

  result = strlen(buffer);

  return result;
}
///


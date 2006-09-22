#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

// DoSuperNew()
// Calls parent NEW method within a subclass

Object * STDARGS VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  Object *rc;
  VA_LIST args;

  VA_START(args, obj);
  rc = (Object *)DoSuperMethod(cl, obj, OM_NEW, VA_ARG(args, ULONG), NULL);
  VA_END(args);

  return rc;
}

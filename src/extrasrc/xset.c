#include <proto/intuition.h>

ULONG xset(Object *obj, ...)
{
  // Note: in contrast to the xset() macro this function does NOT
  // ensure that the supplied tag list is terminated by TAG_DONE!!
  // This function only exists to keep SAS/C compatibility!
  return SetAttrsA(obj, (struct TagItem *)(&obj + 1));
}

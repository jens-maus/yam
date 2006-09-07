#include <proto/intuition.h>

ULONG xget(Object *obj, const ULONG attr)
{
  ULONG b = 0;
  GetAttr(attr, obj, &b);
  return b;
}

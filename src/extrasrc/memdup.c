#include <stdlib.h>
#include <string.h>

/// memdup
//  Duplicates a memory block of given size
void *memdup(const void *source, const size_t size)
{
  void *dest = NULL;

  if(source != NULL && (dest = malloc(size)) != NULL)
    memcpy(dest, source, size);

  return dest;
}
///

#include <string.h>

/// memdup
//  Duplicates a memory block of given size
void *memdup(const void *source, const size_t size)
{
  void *dest;

  if((dest = malloc(size)) != NULL)
    memcpy(dest, source, size);

  return dest;
}
///

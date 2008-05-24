#include <exec/types.h>
#include <proto/exec.h>

/// AllocVecPooled
// allocate a vector of <memSize> bytes from the pool specified by <poolHeader>
APTR AllocVecPooled(APTR poolHeader, ULONG memSize)
{
  ULONG *memory;

  ENTER();

  // add the number of bytes used to store the size information
  memSize += sizeof(ULONG);

  // allocate memory from the pool
  if((memory = (ULONG *)AllocPooled(poolHeader, memSize)) != NULL)
  {
    // and finally store the size of the memory block, including the size itself
    *memory++ = memSize;
  }

  RETURN(memory);
  return memory;
}
///

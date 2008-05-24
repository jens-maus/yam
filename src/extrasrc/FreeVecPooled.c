#include <exec/types.h>
#include <proto/exec.h>

/// FreeVecPooled
// return a vector to the pool
void FreeVecPooled(APTR poolHeader, APTR memory)
{
  ULONG *mem = (ULONG *)memory;
  ULONG memSize;

  ENTER();

  // skip back over the stored size information
  memSize = *(--mem);

  // an return the memory block to the pool
  FreePooled(poolHeader, mem, memSize);

  LEAVE();
}
///

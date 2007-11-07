#include <exec/types.h>
#include <proto/exec.h>

/// SetProcWindow
// sets pr_WindowPtr of the current process to forbid (newWindowPtr==(APTR)-1) or
// permit (newWindowPtr==NULL) error requesters from DOS
APTR SetProcWindow(const void *newWindowPtr)
{
  struct Process *pr;
  APTR oldWindowPtr;

  ENTER();

  pr = (struct Process *)FindTask(NULL);
  oldWindowPtr = pr->pr_WindowPtr;
  pr->pr_WindowPtr = (APTR)newWindowPtr;

  RETURN(oldWindowPtr);
  return oldWindowPtr;
}
///

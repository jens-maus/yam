#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>

struct PathNode {
   BPTR next;
   BPTR dir;
};

BPTR cloneWorkbenchPath(struct ExecBase *SysBase,
			struct DosLibrary *DOSBase,
			struct WBStartup *wbmsg)
{
   BPTR path = 0;

   Forbid();
   if (wbmsg->sm_Message.mn_ReplyPort)
   {
      if (((LONG)wbmsg->sm_Message.mn_ReplyPort->mp_Flags & PF_ACTION) == PA_SIGNAL)
      {
	 struct Process *wbproc = wbmsg->sm_Message.mn_ReplyPort->mp_SigTask;
	 if (wbproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
	 {
	    struct CommandLineInterface *cli = BADDR(wbproc->pr_CLI);
	    if (cli)
	    {
	       BPTR *p = &path;
	       BPTR dir = cli->cli_CommandDir;
	       while (dir)
	       {
		  BPTR dir2;
		  struct FileLock *lock = BADDR(dir);
		  struct PathNode *node;
		  dir = lock->fl_Link;
		  dir2 = DupLock(lock->fl_Key);
		  if (!dir2)
		     break;
		  node = AllocVec(8, MEMF_PUBLIC);
		  if (!node)
		  {
		     UnLock(dir2);
		     break;
		  }
		  node->next = 0;
		  node->dir = dir2;
		  *p = MKBADDR(node);
		  p = &node->next; 
	       }
	    }
	 }
      }
   }
   Permit();

   return path;
}

void freeWorkbenchPath(struct ExecBase *SysBase,
		       struct DosLibrary *DOSBase,
		       BPTR path)
{
   while (path)
   {
      struct PathNode *node = BADDR(path);
      path = node->next;
      UnLock(node->dir);
      FreeVec(node);
   }
}

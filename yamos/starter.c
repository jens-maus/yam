/*************************************************************************

  starter.c

  Copyright 2000 Emmanuel Lesueur

  Automatically select between an AmigaOS 68k executable and a native
  MorphOS ppc one.

  Usage:

  To make two version of the program 'PROG', compile this program with
  a 68k gcc with:

   gcc -nostdlib -DNAME=\"PROG\" -O -fomit-frame-pointer starter.c -o PROG

  Then name the 68k version of 'PROG' 'PROG.amigaos', the MorphOS version
  'PROG.morphos'.

  This code is in the public domain. Use it as you like.

*************************************************************************/

#include <dos/dosextens.h>
#include <proto/intuition.h>
#include <proto/exec.h>
#include <proto/dos.h>

#define REG(r, x) x __asm(#r)

asm("
	.text
	jra _start

_call:	moveml	d2-d7/a2-a6,sp@-
	addal	a1,a1
	addal	a1,a1
	jsr	a1@(4)
	moveml	sp@+,d2-d7/a2-a6
	rts
");

int call(REG(a0, char *line), REG(d0, int len), REG(a1, BPTR start));

int start(REG(a0, char *line), REG(d0, int len))
{ APTR IntuitionBase, DOSBase, SysBase = *(APTR *)4;
  struct Message *wbmsg;
  struct Process *proc;
  int ret = RETURN_FAIL;
  BPTR seg;

  proc = (struct Process *)FindTask(NULL);

  if (!proc->pr_CLI) {
    WaitPort(&proc->pr_MsgPort);
    wbmsg = GetMsg(&proc->pr_MsgPort);
  }
  else {
    wbmsg = NULL;
  }

  if ((DOSBase = OpenLibrary("dos.library", 39))) {

    if (!FindResident("MorphOS") || (0 == (seg = LoadSeg("PROGDIR:" NAME ".morphos"))))
      seg = LoadSeg("PROGDIR:" NAME ".amigaos");

    if (seg) {

      if (wbmsg) {
        PutMsg(&proc->pr_MsgPort, wbmsg);
        wbmsg = NULL;
      }

      ret = call(line, len, seg);

      UnLoadSeg(seg);
    }
    else if (wbmsg) {

      if ((IntuitionBase = OpenLibrary("intuition.library", 39))) {

        static const struct EasyStruct params = {
          sizeof(struct EasyStruct),
          0,
          "Error",
          "Can't load \"PROGDIR:" NAME ".amigaos\".",
          "Ok"
        };

        EasyRequestArgs(NULL, &params, NULL, NULL);

        CloseLibrary(IntuitionBase);
      }
    }
    else {

      PutStr("Can't load \"PROGDIR:" NAME ".amigaos\".\n");

    }

    CloseLibrary(DOSBase);
  }

  if (wbmsg) {
    Forbid();
    ReplyMsg(wbmsg);
  }

  return ret;
}

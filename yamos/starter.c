/*************************************************************************

  starter.c

  Copyright 2000 Emmanuel Lesueur

  Automatically select between an AmigaOS 68k executable and a native
  MorphOS ppc one.

  Usage:

  To make two version of the program 'PROG', compile this program with
  a 68k gcc with:

     gcc -nostdlib -o PROG -O2 -noixemul -DNAME="\"PROG\"" starter.c

  Then name the 68k version of 'PROG' 'PROG.amigaos', the MorphOS version
  'PROG.morphos'.

  This code is in the public domain. Use it as you like.

*************************************************************************/

#define __NOLIBBASE
#include <exec/exec.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#define REG(r, x) x __asm(#r)

asm("
	.text
	bra _start
");

int call(REG(a0, char *line), REG(d0, int len), REG(a1, void *start));
asm("
	.text
_call:
	moveml	d2-d7/a2-a6,sp@-
	jbsr	 (a1)
	moveml	sp@+,d2-d7/a2-a6
	rts
");


int start(REG(a0, char *line), REG(d0, int len)) {
    struct Library *SysBase = *(struct Library **)4;
    struct Library *DOSBase;
    struct Library *IntuitionBase;
    struct Process *proc = (struct Process *)FindTask(NULL);
    struct Message *wbmsg;
    int ret = RETURN_FAIL;
    BPTR seg = 0;

    if (proc->pr_CLI)
	wbmsg = NULL;
    else {
	WaitPort(&proc->pr_MsgPort);
	wbmsg = GetMsg(&proc->pr_MsgPort);
    }

    if (DOSBase = OpenLibrary("dos.library", 39)) {

	if (FindResident("MorphOS")) {
	    seg = LoadSeg("PROGDIR:" NAME ".morphos");
	}

	if (!seg)
	    seg = LoadSeg("PROGDIR:" NAME ".amigaos");

	if (seg) {
	    int (*start)(REG(a0, char *line), REG(d0, int len));

	    start = (void *)((BPTR *)BADDR(seg) + 1);

	    if (wbmsg) {
		PutMsg(&proc->pr_MsgPort, wbmsg);
		wbmsg = NULL;
	    }

	    ret = call(line, len, start);

	    UnLoadSeg(seg);
	}

	if (!seg) {
	    if (wbmsg) {
		
		if (IntuitionBase = (APTR)OpenLibrary("intuition.library", 39)) {
		    static struct EasyStruct params = {
			sizeof(struct EasyStruct),
			0,
			"Error",
			"Can't load \"PROGDIR:" NAME ".amigaos\".",
			"Ok"
		    };

		    EasyRequestArgs(NULL, &params, NULL, NULL);

		    CloseLibrary(IntuitionBase);
		}

	    } else
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


#ifndef NEWREADARGS_H
#define NEWREADARGS_H

/*
**	$VER: newreadargs.h 37.1 (18.6.97)
**
**
**	NewReadArgs structure and protos
**
**	(C) Copyright 1997 Stephan Rupprecht
**	    All Rights Reserved
*/

#ifndef WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif

#ifndef WORKBENCH_STARTUP_H
#include <workbench/startup.h>
#endif

struct NewRDArgs {
	/* you MUST initialize these fields 
	   before calling NewReadArgs() !!! */
	STRPTR	Template;	/* ReadArgs template */
	STRPTR	ExtHelp;	/* ExtHelp string or NULL, shell ONLY */
	STRPTR	Window; 	/* WB window descriptor, eg. "CON:////Test" */
	LONG   *Parameters;	/* array to store parsed parameters */
	LONG	FileParameter;	/* -1 means none, 0 means all */
	LONG	PrgToolTypesOnly;
	/* private !!! */
	ULONG	reserved[6];	/* keep off */
};

void NewFreeArgs(struct NewRDArgs *);
LONG NewReadArgs(struct WBStartup *, struct NewRDArgs *);

#endif /* NEWREADARGS_H */

#ifndef NEWREADARGS_H
#define NEWREADARGS_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 NOTE:
 This implementation of NewReadArgs() is inspired by the implementation of
 Stephan Rupprecht's NewReadArgs().

 $Id$

***************************************************************************/

#ifndef WORKBENCH_WORKBENCH_H
#include <workbench/workbench.h>
#endif

#ifndef WORKBENCH_STARTUP_H
#include <workbench/startup.h>
#endif

struct NewRDArgs
{
  // you MUST initialize these fields 
  // before calling NewReadArgs() !!!
  STRPTR Template;        // ReadArgs-like template
  STRPTR ExtHelp;         // ExtHelp string or NULL, shell ONLY
  STRPTR Window;          // WB window descriptor, eg. "CON:////Test"
  IPTR   *Parameters;     // array to store parsed parameters
  LONG   FileParameter;   // -1 means none, 0 means all
  LONG   PrgToolTypesOnly;
  
  // private data section
  struct RDArgs *RDArgs;    // RDArgs we give to ReadArgs()
  struct RDArgs *FreeArgs;  // RDArgs we get from ReadArgs()
  STRPTR *Args;
  ULONG MaxArgs;
  STRPTR ToolWindow;
  
  APTR Pool;
  
  BPTR WinFH;     // i/o window stream
  BPTR OldInput;  // old i/o streams
  BPTR OldOutput;
};

#endif /* NEWREADARGS_H */

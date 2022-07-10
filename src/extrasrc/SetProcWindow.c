/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

 $Id$

***************************************************************************/

#include <dos/dosextens.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "Debug.h"

#if defined(NEED_SETPROCWINDOW)
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
#else
  #warning "NEED_SETPROCWINDOW missing or compilation unnecessary"
#endif

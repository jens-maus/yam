/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

#include <exec/types.h>
#include <proto/exec.h>

#include "extrasrc.h"

#define DEBUG_USE_MALLOC_REDEFINE
#include "Debug.h"

#if defined(NEED_FREEVECPOOLED)
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
#else
  #warning "NEED_FREEVECPOOLED missing or compilation unnecessary"
#endif
///

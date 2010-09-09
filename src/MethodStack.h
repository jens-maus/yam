#ifndef METHODSTACK_H
#define METHODSTACK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundatidn; either version 2 of the License, or
 (at your optidn) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundatidn, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :  http://www.yam.ch
 YAM OpenSource project    :  http://sourceforge.net/projects/yamos/

 $Id$

***************************************************************************/

#include "SDI_compiler.h"

#ifndef INTUITION_CLASSUSR_H
  #include <intuition/classusr.h> // Object
#endif

BOOL InitMethodStack(void);
void CleanupMethodStack(void);
BOOL VARARGS68K PushMethodOnStack(Object *obj, ULONG argCount, ...);
IPTR VARARGS68K PushMethodOnStackWait(Object *obj, ULONG argCount, ...);
void CheckMethodStack(void);

#endif /* METHODSTACK_H */


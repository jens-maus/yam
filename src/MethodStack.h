#ifndef METHODSTACK_H
#define METHODSTACK_H

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

#include "SDI_compiler.h"

#ifndef INTUITION_CLASSUSR_H
  #include <intuition/classusr.h> // Object
#endif

BOOL InitMethodStack(void);
void CleanupMethodStack(void);
BOOL PushMethodOnStackA(Object *obj, ULONG argCount, struct TagItem *tags);
#define PushMethodOnStack(obj, argCount, ...) ({ ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; PushMethodOnStackA(obj, argCount, (struct TagItem *)_tags); })
IPTR PushMethodOnStackWaitA(Object *obj, ULONG argCount, struct TagItem *tags);
#define PushMethodOnStackWait(obj, argCount, ...) ({ ULONG _tags[] = { SDI_VACAST(__VA_ARGS__) }; PushMethodOnStackWaitA(obj, argCount, (struct TagItem *)_tags); })
void CheckMethodStack(void);

#endif /* METHODSTACK_H */


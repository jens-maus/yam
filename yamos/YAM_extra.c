/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include <string.h>
#include <ctype.h>
#define DoMethod _DoMethod
#include <clib/alib_protos.h>
#undef DoMethod
#include "YAM.h"

#include "extrasrc/astcsma.c"

int max(int x, int y)
{
   return x > y ? x : y;
}

size_t StrLen(const char *str)
{
   return strlen(str);
}

ULONG DoMethod(void *obj,ULONG a1,ULONG a2,ULONG a3,ULONG a4,ULONG a5,ULONG a6,ULONG a7,ULONG a8,ULONG a9,ULONG a10,ULONG a11,ULONG a12,ULONG a13,ULONG a14,ULONG a15)
{
    ULONG t[15];
    t[0]=a1;
    t[1]=a2;
    t[2]=a3;
    t[3]=a4;
    t[4]=a5;
    t[5]=a6;
    t[6]=a7;
    t[7]=a8;
    t[8]=a9;
    t[9]=a10;
    t[10]=a11;
    t[11]=a12;
    t[12]=a13;
    t[13]=a14;
    t[14]=a15;
    return DoMethodA(obj,(Msg)t);
}

#include "extrasrc/md5.c"

#include "extrasrc/wbpath.c"

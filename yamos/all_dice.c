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

#include "extrasrc/md5.c"
#include "extrasrc/astcsma.c"
#include "extrasrc/getft.c"
#include "extrasrc/stccpy.c"
#include "extrasrc/stcgfe.c"
#include "extrasrc/stcgfn.c"
#include "extrasrc/stpblk.c"
#include "extrasrc/strmfp.c"
#include "extrasrc/strsfn.c"
#include "extrasrc/wbpath.c"
#include "extrasrc/NewReadArgs.c"
#include "extrasrc/stch_i.c"
#include "extrasrc/dice.c"

void wbmain(struct WBStartup *wbs)
{
   main(0, (char**)wbs);
}

#include <stdarg.h>
int KPrintF(const char *format)
{
   int n; va_list vl;

   va_start(vl,format);
   n=fprintf(stderr,format,vl);
   va_end(vl);
   fflush(stderr);
   return n;
}

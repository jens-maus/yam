#ifndef YAM_GLOBAL_H
#define YAM_GLOBAL_H

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

#include "YAM_stringsizes.h"

extern char *             ContType[];
extern char *             ContTypeDesc[];
extern char *             FolderNames[4];
extern char *             months[12];
extern char *             SecCodes[5];
extern char *             SigNames[3];
extern char *             Status[9];
extern struct WBStartup * WBmsg;
extern char *             wdays[7];
extern char *             yamversion;
extern char *             yamversionver;
extern char *             yamversionstring;
extern char *             yamversiondate;
extern unsigned long      yamversiondays;

enum ContTypeEntry
{
   CT_TX_PLAIN=0,
   CT_TX_HTML,
   CT_TX_GUIDE,
   CT_AP_OCTET,
   CT_AP_PS,
   CT_AP_PDF,
   CT_AP_RTF,
   CT_AP_LHA,
   CT_AP_LZX,
   CT_AP_ZIP,
   CT_AP_AEXE,
   CT_AP_SCRIPT,
   CT_AP_REXX,
   CT_IM_JPG,
   CT_IM_GIF,
   CT_IM_PNG,
   CT_IM_TIFF,
   CT_IM_ILBM,
   CT_AU_AU,
   CT_AU_8SVX,
   CT_AU_WAV,
   CT_VI_MPG,
   CT_VI_MOV,
   CT_VI_ANIM,
   CT_VI_AVI,
   CT_ME_EMAIL
};

#endif /* YAM_GLOBAL_H */

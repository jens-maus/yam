#ifndef YAM_GLOBAL_H
#define YAM_GLOBAL_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

extern const char* const ContType[];
extern const char* const ContTypeDesc[];
extern const char* const FolderNames[4];
extern const char* const months[12];
extern const char* const SecCodes[5];
extern const char* const SigNames[3];
extern const char* const wdays[7];
extern const char* const yamversion;
extern const char* const yamversionver;
extern const char* const yamversionstring;
extern const char* const yamverxmailer;
extern const char* const yamversiondate;
extern const char* const yamcopyright;
extern const char* const yamfullcopyright;
extern const char* const yamcompiler;
extern const unsigned long yamversiondays;
extern struct WBStartup * WBmsg;

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

// transforms a define into a string
#define STR(x)  STR2(x)
#define STR2(x) #x

#endif /* YAM_GLOBAL_H */

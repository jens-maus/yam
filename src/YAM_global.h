#ifndef YAM_GLOBAL_H
#define YAM_GLOBAL_H

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

extern const char* const months[12];
extern const char* const SecCodes[5];
extern const char* const SigNames[3];
extern const char* const wdays[7];
extern const char* const yamver;
extern const char* const yamversion;
extern const char* const yamversionver;
extern const char* const yamversionstring;
extern const char* const yamuseragent;
extern const char* const yambuildid;
extern const char* const yamversiondate;
extern const char* const yamcopyright;
extern const char* const yamfullcopyright;
extern const char* const yamcompiler;
extern const char* const yamurl;
extern const unsigned long yamversiondays;
extern struct WBStartup * WBmsg;
#if defined(__amigaos3__)
extern struct Library * PictureDTBase;
#endif

// transforms a define into a string
#define STR(x)  STR2(x)
#define STR2(x) #x

#endif /* YAM_GLOBAL_H */

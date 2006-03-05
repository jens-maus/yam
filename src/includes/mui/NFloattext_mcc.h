/***************************************************************************

 NFloattext.mcc - New Floattext MUI Custom Class
 Registered MUI class, Serial Number: 1d51 (0x9d5100a1 to 0x9d5100aF)

 Copyright (C) 1996-2001 by Gilles Masson
 Copyright (C) 2001-2005 by NList Open Source Team

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 NList classes Support Site:  http://www.sf.net/projects/nlist-classes

 $Id$

***************************************************************************/

#ifndef MUI_NFloattext_MCC_H
#define MUI_NFloattext_MCC_H

#ifndef LIBRARIES_MUI_H
#include <libraries/mui.h>
#endif

#ifndef MUI_NListview_MCC_H
#include <mui/NListview_mcc.h>
#endif

#ifdef __GNUC__
  #ifdef __PPC__
    #pragma pack(2)
  #endif
#elif defined(__VBCC__)
  #pragma amiga-align
#endif

#define MUIC_NFloattext "NFloattext.mcc"
#define NFloattextObject MUI_NewObject(MUIC_NFloattext


/* Attributes */

#define MUIA_NFloattext_Text                0x9d5100a1 /* GM  isg STRPTR             */
#define MUIA_NFloattext_SkipChars           0x9d5100a2 /* GM  isg char *             */
#define MUIA_NFloattext_TabSize             0x9d5100a3 /* GM  isg ULONG              */
#define MUIA_NFloattext_Justify             0x9d5100a4 /* GM  isg BOOL               */
#define MUIA_NFloattext_Align               0x9d5100a5 /* GM  isg LONG               */

#define MUIM_NFloattext_GetEntry            0x9d5100aF /* GM */
struct  MUIP_NFloattext_GetEntry            { ULONG MethodID; LONG pos; APTR *entry; };

#ifdef __GNUC__
  #ifdef __PPC__
    #pragma pack()
  #endif
#elif defined(__VBCC__)
  #pragma default-align
#endif

#endif /* MUI_NFloattext_MCC_H */

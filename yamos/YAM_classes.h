#ifndef YAM_CLASSES_H
#define YAM_CLASSES_H

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

struct PL_Data
{ 
   struct Hook DisplayHook;
   Object *    Object[MAXCPAGES];
   APTR        Image[MAXCPAGES];
};

#define MUIA_Bodychunk_File          0x80002501
#define MUIA_Bodychunk_UseOld        0x80002502
#define MUIM_MainWindow_CloseWindow  0x80002521

extern struct MUI_CustomClass *CL_AddressList;
extern struct MUI_CustomClass *CL_AttachList;
extern struct MUI_CustomClass *CL_BodyChunk;
extern struct MUI_CustomClass *CL_DDList;
extern struct MUI_CustomClass *CL_DDString;
extern struct MUI_CustomClass *CL_FolderList;
extern struct MUI_CustomClass *CL_MailList;
extern struct MUI_CustomClass *CL_MainWin;
extern struct MUI_CustomClass *CL_PageList;
extern struct MUI_CustomClass *CL_TextEditor;

void ExitClasses(void);
BOOL InitClasses(void);

#endif /* YAM_CLASSES_H */

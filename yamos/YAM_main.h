#ifndef YAM_MAIN_H
#define YAM_MAIN_H

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

#include <mui/Toolbar_mcc.h>

struct MA_GUIData
{
   APTR WI;
   APTR MN_FOLDER;
   APTR MN_REXX;
   APTR MS_MAIN;
   APTR BC_STAT[17];
   APTR ST_LAYOUT;
   APTR MI_ERRORS;
   APTR MI_CSINGLE;
   APTR MI_IMPORT;
   APTR MI_EXPORT;
   APTR MI_SENDALL;
   APTR MI_EXCHANGE;
   APTR MI_GETMAIL;
   APTR MI_READ;
   APTR MI_EDIT;
   APTR MI_MOVE;
   APTR MI_COPY;
   APTR MI_DELETE;
   APTR MI_PRINT;
   APTR MI_SAVE;
   APTR MI_ATTACH;
   APTR MI_SAVEATT;
   APTR MI_REMATT;
   APTR MI_EXPMSG;
   APTR MI_REPLY;
   APTR MI_FORWARD;
   APTR MI_BOUNCE;
   APTR MI_GETADDRESS;
   APTR MI_STATUS;
   APTR MI_TOREAD;
   APTR MI_TOUNREAD;
   APTR MI_TOHOLD;
   APTR MI_TOQUEUED;
   APTR MI_CHSUBJ;
   APTR MI_SEND;
   APTR LV_FOLDERS;
   APTR NL_FOLDERS;
   APTR LV_MAILS;
   APTR NL_MAILS;
   APTR TO_TOOLBAR;
   APTR GA_INFO;
   struct MUIP_Toolbar_Description TB_TOOLBAR[18];
};

struct MA_ClassData  /* main window */
{
   struct MA_GUIData GUI;
   char WinTitle[SIZE_DEFAULT];
};

extern struct Hook MA_ChangeSelectedHook;
extern struct Hook MA_SetFolderInfoHook;
extern struct Hook MA_SetMessageInfoHook;

#endif /* YAM_MAIN_H */

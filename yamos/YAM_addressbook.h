#ifndef YAM_ADDRESSBOOK_H
#define YAM_ADDRESSBOOK_H

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

#include "SDI_compiler.h"
#include "YAM_stringsizes.h"

#define ASM_ALIAS    0
#define ASM_REALNAME 1
#define ASM_ADDRESS  2
#define ASM_TYPEMASK 7
#define ASM_USER     8
#define ASM_LIST     16
#define ASM_GROUP    32
#define ASM_COMPLETE 64

struct AB_GUIData
{
   APTR WI;
   APTR TO_TOOLBAR;
   APTR LV_ADRESSES;
   APTR BT_TO;
   APTR BT_CC;
   APTR BT_BCC;
   struct MUIP_Toolbar_Description TB_TOOLBAR[13];
};
 
struct AB_ClassData  /* address book window */
{
   struct AB_GUIData GUI;
   int  Hits;
   int  Mode;
   int  SortBy;
   int  WrWin;
   BOOL Modified;
   char WTitle[SIZE_DEFAULT];
};

extern struct Hook AB_SaveABookHook;
extern struct Hook AB_LV_DspFuncHook;
extern struct Hook AB_DeleteHook;

int STACKEXT AB_SearchEntry(struct MUI_NListtree_TreeNode *list, char *text, int mode,
   int *hits, struct MUI_NListtree_TreeNode **lasthit);
BOOL STACKEXT AB_FindEntry(struct MUI_NListtree_TreeNode *list, char *pattern, int mode,
   char **result);

#endif /* YAM_ADDRESSBOOK_H */

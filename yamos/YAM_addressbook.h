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

#include <mui/NListtree_mcc.h>
#include <mui/Toolbar_mcc.h>

#include "SDI_compiler.h"
#include "YAM_addressbookEntry.h"
#include "YAM_stringsizes.h"

// special Searchtypes for AB_SearchEntry()
#define ASM_ALIAS    1
#define ASM_REALNAME 2
#define ASM_ADDRESS  4
#define ASM_TYPEMASK 7
#define ASM_USER     8
#define ASM_LIST     16
#define ASM_GROUP    32
#define ASM_COMPLETE 64

enum AddressbookMode { ABM_EDIT, ABM_TO, ABM_CC, ABM_BCC, ABM_REPLYTO, ABM_FROM };

enum AddressbookFind { ABF_USER, ABF_RX, ABF_RX_NAME, ABF_RX_EMAIL, ABF_RX_NAMEEMAIL };

struct AB_GUIData
{
   APTR WI;
   APTR TO_TOOLBAR;
   APTR LV_ADDRESSES;
   APTR BT_TO;
   APTR BT_CC;
   APTR BT_BCC;
   struct MUIP_Toolbar_Description TB_TOOLBAR[13];
};
 
struct AB_ClassData  /* address book window */
{
   struct AB_GUIData    GUI;
   int                  Hits;
   int                  SortBy;
   int                  WrWin;
   enum AddressbookMode Mode;
   BOOL                 Modified;
   char                 WTitle[SIZE_DEFAULT];
};

extern struct Hook AB_DeleteHook;
extern struct Hook AB_LV_DspFuncHook;
extern struct Hook AB_OpenHook;
extern struct Hook AB_SaveABookHook;

void   AB_CheckBirthdates(void);
char * AB_CompleteAlias(char *text);
long   AB_CompressBD(char *datestr);
char * AB_ExpandBD(long date);
BOOL STACKEXT AB_FindEntry(struct MUI_NListtree_TreeNode *list, char *pattern, enum AddressbookFind mode, char **result);
APTR   AB_GotoEntry(char *alias);
void   AB_InsertAddress(APTR string, char *alias, char *name, char *address);
BOOL   AB_LoadTree(char *fname, BOOL append, BOOL sorted);
void   AB_MakeABFormat(APTR lv);
struct AB_ClassData *AB_New(void);
BOOL   AB_SaveTree(char *fname);
int    AB_SearchEntry(char *text, int mode, struct ABEntry **ab);

#endif /* YAM_ADDRESSBOOK_H */

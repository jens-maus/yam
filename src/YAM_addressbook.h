#ifndef YAM_ADDRESSBOOK_H
#define YAM_ADDRESSBOOK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <stdlib.h>

#include <mui/NListtree_mcc.h>

#include "SDI_compiler.h"

#include "YAM_stringsizes.h"

// forward declarations
struct ABEntry;
struct Person;

// special Searchtypes for AB_SearchEntry()
#define ASM_ALIAS    1
#define ASM_REALNAME 2
#define ASM_ADDRESS  4
#define ASM_TYPEMASK 7
#define ASM_USER     8
#define ASM_LIST     16
#define ASM_GROUP    32
#define ASM_COMPLETE 64

#define isAliasSearch(mode)     (isFlagSet((mode), ASM_ALIAS))
#define isRealNameSearch(mode)  (isFlagSet((mode), ASM_REALNAME))
#define isAddressSearch(mode)   (isFlagSet((mode), ASM_ADDRESS))
#define isUserSearch(mode)      (isFlagSet((mode), ASM_USER))
#define isListSearch(mode)      (isFlagSet((mode), ASM_LIST))
#define isGroupSearch(mode)     (isFlagSet((mode), ASM_GROUP))
#define isCompleteSearch(mode)  (isFlagSet((mode), ASM_COMPLETE))

enum AddressbookMode { ABM_NONE=0, ABM_EDIT, ABM_TO, ABM_CC, ABM_BCC, ABM_REPLYTO, ABM_FROM, ABM_CONFIG };

enum AddressbookFind { ABF_USER=0, ABF_RX, ABF_RX_NAME, ABF_RX_EMAIL, ABF_RX_NAMEEMAIL };

struct AB_GUIData
{
  Object *WI;
  Object *LV_ADDRESSES;
  Object *BT_TO;
  Object *BT_CC;
  Object *BT_BCC;
  Object *TB_TOOLBAR;
};

struct AB_ClassData  /* address book window */
{
  struct AB_GUIData    GUI;
  int                  SortBy;
  enum AddressbookMode Mode;
  BOOL                 Modified;
  char                 WTitle[SIZE_DEFAULT];
  Object               *parentStringGadget; // in case ABM_CONFIG is used.
  int                  winNumber;           // related write window number
};

extern struct Hook AB_DeleteHook;
extern struct Hook AB_LV_DspFuncHook;
extern struct Hook AB_OpenHook;
extern struct Hook AB_SaveABookHook;
extern struct Hook AB_FindHook;
extern struct Hook AB_AddEntryHook;
extern struct Hook AB_EditHook;
extern struct Hook AB_PrintHook;
extern struct Hook AB_FoldUnfoldHook;

void   AB_CheckBirthdates(BOOL check);
char * AB_CompleteAlias(const char *text);
long   AB_CompressBD(const char *datestr);
BOOL   AB_CreateEmptyABook(const char *fname);
BOOL   AB_ExpandBD(const long date, char *dateStr, const size_t dateStrSize);
int    AB_FindEntry(const char *pattern, enum AddressbookFind mode, char **result);
APTR   AB_GotoEntry(const char *alias);
BOOL   AB_LoadTree(const char *fname, BOOL append, BOOL sorted);
void   AB_MakeABFormat(APTR lv);
struct AB_ClassData *AB_New(void);
BOOL   AB_SaveTree(const char *fname);
int    AB_SearchEntry(const char *text, int mode, struct ABEntry **ab);

#endif /* YAM_ADDRESSBOOK_H */

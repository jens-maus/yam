#ifndef YAM_ADDRESSBOOK_H
#define YAM_ADDRESSBOOK_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#define ASM_ALIAS    (1<<0)
#define ASM_REALNAME (1<<1)
#define ASM_ADDRESS  (1<<2)
#define ASM_TYPEMASK 7
#define ASM_USER     (1<<3)
#define ASM_LIST     (1<<4)
#define ASM_GROUP    (1<<5)
#define ASM_COMPLETE (1<<6)

#define isAliasSearch(mode)     (isFlagSet((mode), ASM_ALIAS))
#define isRealNameSearch(mode)  (isFlagSet((mode), ASM_REALNAME))
#define isAddressSearch(mode)   (isFlagSet((mode), ASM_ADDRESS))
#define isUserSearch(mode)      (isFlagSet((mode), ASM_USER))
#define isListSearch(mode)      (isFlagSet((mode), ASM_LIST))
#define isGroupSearch(mode)     (isFlagSet((mode), ASM_GROUP))
#define isCompleteSearch(mode)  (isFlagSet((mode), ASM_COMPLETE))

enum AddressbookMode
{
  ABM_NONE=0,
  ABM_EDIT,
  ABM_FROM,
  ABM_TO,
  ABM_CC,
  ABM_BCC,
  ABM_REPLYTO,
  ABM_CONFIG
};

enum AddressbookFind
{
 ABF_USER=0,
 ABF_RX,
 ABF_RX_NAME,
 ABF_RX_EMAIL,
 ABF_RX_NAMEEMAIL
};

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

  enum AddressbookMode Mode;
  Object               *parentStringGadget; // in case ABM_CONFIG is used.
  int                  winNumber;           // related write window number
  char                 windowTitle[SIZE_DEFAULT];
  char                 screenTitle[SIZE_DEFAULT];
};

extern struct Hook AB_OpenHook;
extern struct Hook AB_SaveABookHook;

void   AB_CheckBirthdates(BOOL check);
char * AB_CompleteAlias(const char *text);
long   AB_CompressBD(const char *datestr);
BOOL   AB_CreateEmptyABook(const char *fname);
BOOL   AB_ExpandBD(const long date, char *dateStr, const size_t dateStrSize);
int    AB_FindEntry(const char *pattern, enum AddressbookFind mode, char **result);
APTR   AB_GotoEntry(const char *alias);
BOOL   AB_LoadTree(const char *fname, BOOL append, BOOL sorted);
struct AB_ClassData *AB_New(void);
BOOL   AB_SaveTree(const char *fname);
int    AB_SearchEntry(const char *text, int mode, struct ABEntry **ab);

void AB_PrintLevel(Object *tree, struct MUI_NListtree_TreeNode *list, FILE *prt, int mode);
void AB_PrintLongEntry(FILE *prt, struct ABEntry *ab);
void AB_PrintShortEntry(FILE *prt, struct ABEntry *ab);
void AB_InsertAddressTreeNode(Object *writeWindow, ULONG type, Object *tree, struct MUI_NListtree_TreeNode *tn);

BOOL AB_ExportTreeLDIF(Object *tree, const char *fname);
BOOL AB_ImportTreeLDIF(Object *tree, const char *fname, BOOL append, BOOL sorted);
BOOL AB_ImportTreeTabCSV(Object *tree, const char *fname, BOOL append, BOOL sorted, char delim);
BOOL AB_ExportTreeTabCSV(Object *tree, const char *fname, char delim);
BOOL AB_ImportTreeXML(Object *tree, const char *fname, BOOL append, BOOL sorted);

#endif /* YAM_ADDRESSBOOK_H */

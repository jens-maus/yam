#ifndef YAM_ADDRESSBOOKENTRY_H
#define YAM_ADDRESSBOOKENTRY_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

#include "SDI_compiler.h"

#include "YAM_stringsizes.h"
#include "YAM_write.h"

// forward declarations
struct MUI_NListtree_TreeNode;

struct EA_GUIData
{
  Object *WI;
  Object *ST_ALIAS;
  Object *ST_REALNAME;
  Object *ST_ADDRESS;
  Object *ST_COMMENT;
  Object *ST_PHONE;
  Object *ST_STREET;
  Object *ST_CITY;
  Object *ST_COUNTRY;
  Object *ST_PGPKEY;
  Object *CY_DEFSECURITY;
  Object *ST_HOMEPAGE;
  Object *ST_BIRTHDAY;
  Object *GR_PHOTO;
  Object *LV_MEMBER;
  Object *ST_MEMBER;
  Object *BT_ADD;
  Object *BT_DEL;
  Object *BT_OKAY;
  Object *BT_CANCEL;
};

struct EA_ClassData  /* address book entry window */
{
  struct EA_GUIData GUI;
  struct ABEntry    *ABEntry;
  int               Type;
  int               EntryPos;
  char              PhotoName[SIZE_PATHFILE];
};

#define AddrName(abentry) ((abentry).RealName[0]?(abentry).RealName:(abentry).Address)

enum ABEntry_Type
{
  AET_USER=0,
  AET_LIST,
  AET_GROUP
};

struct ABEntry
{
  char *            Members;
  long              BirthDay;
  enum ABEntry_Type Type;
  enum Security     DefSecurity;

  char Address[SIZE_ADDRESS];
  char RealName[SIZE_REALNAME];
  char Comment[SIZE_DEFAULT];
  char Alias[SIZE_NAME];
  char Phone[SIZE_DEFAULT];
  char Street[SIZE_DEFAULT];
  char City[SIZE_DEFAULT];
  char Country[SIZE_DEFAULT];
  char Homepage[SIZE_URL];
  char PGPId[SIZE_ADDRESS];
  char Photo[SIZE_PATHFILE];
};

void   EA_AddSingleMember(Object *obj, struct MUI_NListtree_TreeNode *tn);
void STACKEXT EA_AddMembers(Object *obj, struct MUI_NListtree_TreeNode *list);
void   EA_FixAlias(struct ABEntry *ab, BOOL excludemyself);
int    EA_Init(enum ABEntry_Type type, struct ABEntry *ab);
void   EA_InsertBelowActive(struct ABEntry *addr, int flags);
void   EA_SetDefaultAlias(struct ABEntry *ab);
void   EA_Setup(int winnum, struct ABEntry *ab);

#endif /* YAM_ADDRESSBOOKENTRY_H */

#ifndef YAM_ADDRESSBOOKENTRY_H
#define YAM_ADDRESSBOOKENTRY_H

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

#include "SDI_compiler.h"

#include "YAM_stringsizes.h"

struct EA_GUIData
{
   APTR WI;
   APTR ST_ALIAS;
   APTR ST_REALNAME;
   APTR ST_ADDRESS;
   APTR ST_COMMENT;
   APTR ST_PHONE;
   APTR ST_STREET;
   APTR ST_CITY;
   APTR ST_COUNTRY;
   APTR ST_PGPKEY;
   APTR CY_DEFSECURITY;
   APTR ST_HOMEPAGE;
   APTR ST_BIRTHDAY;
   APTR GR_PHOTO;
   APTR BC_PHOTO;
   APTR BT_SELECTPHOTO;
   APTR BT_LOADPHOTO;
   APTR LV_MEMBER;
   APTR ST_MEMBER;
   APTR BT_ADD;
   APTR BT_DEL;
   APTR BT_OKAY;
   APTR BT_CANCEL;
};

struct EA_ClassData  /* address book entry window */
{
   struct EA_GUIData              GUI;
   struct MUI_NListtree_TreeNode *EditNode;
   int                            Type;
   int                            EntryPos;
   char                           PhotoName[SIZE_PATHFILE];
};

enum ABEntry_Type { AET_USER=0, AET_LIST, AET_GROUP };

struct ABEntry
{
   char *            Members;
   long              BirthDay;
   enum ABEntry_Type Type;
   int               DefSecurity;

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
int    EA_Init(enum ABEntry_Type type, struct MUI_NListtree_TreeNode *tn);
void   EA_InsertBelowActive(struct ABEntry *addr, int flags);
void   EA_SetDefaultAlias(struct ABEntry *ab);
void   EA_Setup(int winnum, struct ABEntry *ab);

#endif /* YAM_ADDRESSBOOKENTRY_H */

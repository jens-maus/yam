#ifndef YAM_USERLIST_H
#define YAM_USERLIST_H

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

#include <intuition/classusr.h> // Object

#include "YAM_stringsizes.h"

struct US_GUIData
{
   Object *WI;
   Object *LV_USERS;
   Object *BT_ADD;
   Object *BT_DEL;
   Object *PO_MAILDIR;
   Object *ST_MAILDIR;
   Object *ST_USER;
   Object *ST_PASSWD;
   Object *CH_USEDICT;
   Object *CH_USEADDR;
   Object *CH_CLONE;
   Object *CH_ROOT;
};

struct US_ClassData  /* user list window */
{
   struct US_GUIData GUI;
   BOOL              Supervisor;
};

struct User
{
   int  ID;

   BOOL Limited;
   BOOL UseAddr;
   BOOL UseDict;
   BOOL Clone;
   BOOL IsNew;

   char Name[SIZE_NAME];
   char Password[SIZE_USERID];
   char MailDir[SIZE_PATH];
};

struct Users
{
   int         Num;
   int         CurrentID;
   struct User User[MAXUSERS];
};

extern struct Hook US_OpenHook;

struct User *US_GetCurrentUser(void);
BOOL         US_Login(const char *username, const char *password,
                      const char *maildir, const char *prefsfile);

#endif /* YAM_USERLIST_H */

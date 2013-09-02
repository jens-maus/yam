#ifndef ADDRESSBOOK_H
#define ADDRESSBOOK_H 1

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
#include <stdio.h>

#include "YAM_stringsizes.h"
#include "YAM_write.h"

enum ABookNodeType
{
  AET_USER = 0,
  AET_LIST,
  AET_GROUP
};

struct ABookNode
{
  struct MinNode node;
  enum ABookNodeType type;
  char Alias[SIZE_NAME];
  char Comment[SIZE_DEFAULT];

  union
  {
    struct
    {
      ULONG Birthday;
      enum Security DefSecurity;
      char Address[SIZE_ADDRESS];
      char RealName[SIZE_REALNAME];
      char Phone[SIZE_DEFAULT];
      char Street[SIZE_DEFAULT];
      char City[SIZE_DEFAULT];
      char Country[SIZE_DEFAULT];
      char Homepage[SIZE_URL];
      char PGPId[SIZE_ADDRESS];
      char Photo[SIZE_PATHFILE];
    } user;
    struct
    {
      char *Members;
      char Address[SIZE_ADDRESS];
      char RealName[SIZE_REALNAME];
    } list;
    struct
    {
      struct MinList Members;
    } group;
  } content;
};

struct AddressBook
{
  struct MinList root;
  BOOL modified;
};

// special search types for SearchAddressBook()
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

struct ABookNode *CreateABookNode(enum ABookNodeType type);
void InitABookNode(struct ABookNode *abn, enum ABookNodeType type);
void DeleteABookNode(struct ABookNode *abn);
void InitAddressBook(struct AddressBook *abook);
void ClearAddressBook(struct AddressBook *abook);
BOOL IterateAddressBook(struct AddressBook *abook, BOOL (*nodeFunc)(const struct ABookNode *abn, BOOL first, const void *userData), const void *userData);
BOOL LoadAddressBook(const char *filename, struct AddressBook *abook, BOOL append);
BOOL SaveAddressBook(const char *filename, const struct AddressBook *abook);
BOOL ImportAddressBookLDIF(const char *filename, struct AddressBook *abook, BOOL append);
BOOL ExportAddressBookLDIF(const char *filename, const struct AddressBook *abook);
BOOL ImportAddressBookCSV(const char *filename, struct AddressBook *abook, BOOL append, char delimiter);
BOOL ExportAddressBookCSV(const char *filename, const struct AddressBook *abook, char delimiter);
int SearchAddressBook(const struct AddressBook *abook, const char *text, int mode, struct ABookNode **abn);

#endif /* ADDRESSBOOK_H */

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

// forward declarations
struct Person;

enum ABookNodeType
{
  ABNT_USER = 0,
  ABNT_LIST,
  ABNT_GROUP
};

struct ABookNode
{
  struct MinNode node;
  enum ABookNodeType type;
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
  ULONG Birthday;
  enum Security DefSecurity;
  struct MinList GroupMembers;
  char *ListMembers;
};

#define AddrName(abn) ((abn).RealName[0] != '\0' ? (abn).RealName : (abn).Address)

struct ABook
{
  struct ABookNode  rootGroup;
  struct ABookNode *arexxABN;
  BOOL modified;
};

// flags for IterateABook()
#define IABF_VISIT_GROUPS_TWICE (1<<0)
#define IABF_FIRST_GROUP_VISIT  (1<<1)
#define IABF_SECOND_GROUP_VISIT (1<<2)

// special search types for SearchABook()
#define ASM_ALIAS    (1<<0)
#define ASM_REALNAME (1<<1)
#define ASM_ADDRESS  (1<<2)
#define ASM_COMMENT  (1<<3)
#define ASM_USERINFO (1<<4)
#define ASM_USER     (1<<8)
#define ASM_LIST     (1<<9)
#define ASM_GROUP    (1<<10)
#define ASM_COMPLETE (1<<11)

#define isAliasSearch(mode)     (isFlagSet((mode), ASM_ALIAS))
#define isRealNameSearch(mode)  (isFlagSet((mode), ASM_REALNAME))
#define isAddressSearch(mode)   (isFlagSet((mode), ASM_ADDRESS))
#define isCommentSearch(mode)   (isFlagSet((mode), ASM_COMMENT))
#define isUserInfoSearch(mode)  (isFlagSet((mode), ASM_USERINFO))
#define isUserTypeSearch(mode)  (isFlagSet((mode), ASM_USER))
#define isListTypeSearch(mode)  (isFlagSet((mode), ASM_LIST))
#define isGroupTypeSearch(mode) (isFlagSet((mode), ASM_GROUP))
#define isCompleteSearch(mode)  (isFlagSet((mode), ASM_COMPLETE))

struct ABookNode *CreateABookNode(enum ABookNodeType type);
void InitABookNode(struct ABookNode *abn, enum ABookNodeType type);
void DeleteABookNode(struct ABookNode *abn);
void AddABookNode(struct ABookNode *group, struct ABookNode *member, struct ABookNode *afterThis);
void RemoveABookNode(struct ABookNode *member);
void MoveABookNode(struct ABookNode *group, struct ABookNode *member, struct ABookNode *afterThis);
BOOL CompareABookNodes(const struct ABookNode *abn1, const struct ABookNode *abn2);
void InitABook(struct ABook *abook);
void ClearABook(struct ABook *abook);
void MoveABookNodes(struct ABook *dst, struct ABook *src);
BOOL IterateABook(const struct ABook *abook, ULONG flags, BOOL (*nodeFunc)(const struct ABookNode *abn, ULONG flags, void *userData), void *userData);
BOOL IterateABookGroup(const struct ABookNode *group, ULONG flags, BOOL (*nodeFunc)(const struct ABookNode *abn, ULONG flags, void *userData), void *userData);
BOOL CreateEmptyABookFile(const char *filename);
BOOL LoadABook(const char *filename, struct ABook *abook, BOOL append);
BOOL SaveABook(const char *filename, const struct ABook *abook);
BOOL ImportLDIFABook(const char *filename, struct ABook *abook, BOOL append);
BOOL ExportLDIFABook(const char *filename, const struct ABook *abook);
BOOL ImportCSVABook(const char *filename, struct ABook *abook, BOOL append, char delimiter);
BOOL ExportCSVABook(const char *filename, const struct ABook *abook, char delimiter);
BOOL ImportXMLABook(const char *filename, struct ABook *abook, BOOL append);
ULONG SearchABook(const struct ABook *abook, const char *text, ULONG mode, struct ABookNode **abn);
ULONG PatternSearchABook(const struct ABook *abook, const char *pattern, ULONG mode, char **aliases);
struct ABookNode *CreateABookGroup(struct ABook *abook, const char *name);
struct ABookNode *FindPersonInABook(const struct ABook *abook, const struct Person *pe);
void CheckABookBirthdays(const struct ABook *abook, BOOL check);
void FixAlias(const struct ABook *abook, struct ABookNode *abn, const struct ABookNode *excludeThis);
void SetDefaultAlias(struct ABookNode *abn);
void PrintABook(const struct ABook *abook, FILE *prt, int mode);
void PrintABookGroup(const struct ABookNode *group, FILE *prt, int mode);
void PrintShortABookEntry(const struct ABookNode *abn, FILE *prt);
void PrintLongABookEntry(const struct ABookNode *abn, FILE *prt);

#endif /* ADDRESSBOOK_H */

#ifndef YAM_FOLDERCONFIG_H
#define YAM_FOLDERCONFIG_H

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

#include "YAM_stringsizes.h"

struct FO_GUIData
{
   APTR WI;
   APTR ST_FNAME;
   APTR TX_FPATH;
   APTR BT_MOVE;
   APTR NM_MAXAGE;
   APTR CY_FMODE;
   APTR CY_FTYPE;
   APTR CY_SORT[2];
   APTR CH_REVERSE[2];
   APTR ST_MLPATTERN;
   APTR ST_MLFROMADDRESS;
   APTR ST_MLREPLYTOADDRESS;
   APTR ST_MLADDRESS;
   APTR CY_MLSIGNATURE;
   APTR CH_STATS;
   APTR CH_MLSUPPORT;
   APTR BT_AUTODETECT;
   APTR BT_OKAY;
   APTR BT_CANCEL;
};

struct FO_ClassData  /* folder configuration window */
{
   struct FO_GUIData GUI;
   struct Folder *   EditFolder;
};

#define OUTGOING(type)      (type == FT_OUTGOING || type == FT_SENT || type == FT_CUSTOMSENT)
#define CUSTOMFOLDER(type)  (type == FT_CUSTOM || type == FT_CUSTOMSENT || type == FT_CUSTOMMIXED)

enum FolderType { FT_CUSTOM=0, FT_INCOMING, FT_OUTGOING, FT_SENT, FT_DELETED, FT_GROUP, FT_CUSTOMSENT, FT_CUSTOMMIXED };
enum SetOrder   { SO_SAVE=0, SO_RESET };

#define FolderName(fo) ((fo) ? (fo)->Name : "?")

#define FOFL_MODIFY  1
#define FOFL_FREEXS  2

struct Folder
{
   APTR            BC_FImage;
   struct BodyChunkData *   FImage;
   struct Mail *   Messages;
   ULONG           Flags;
   LONG            Size;
   int             MLSignature;
   int             XPKType;
   int             Total;
   int             New;
   int             Unread;
   int             Sent;
   int             Deleted;
   int             Sort[2];
   int             Stats;
   int             MaxAge;
   int             LastActive;
   int             LoadedMode;
   int             SortIndex;
   int             Open;
   int             ImageIndex;

   enum FolderType Type;

   char            Name[SIZE_NAME];
   char            Path[SIZE_PATH];
   char            Password[SIZE_USERID];   
   char            MLPattern[SIZE_PATTERN];
   char            MLAddress[SIZE_ADDRESS];
   char            MLFromAddress[SIZE_ADDRESS];
   char            MLReplyToAddress[SIZE_ADDRESS];

   BOOL            MLSupport;
};

extern struct Hook FO_DeleteFolderHook;
extern struct Hook FO_EditFolderHook;
extern struct Hook FO_NewFolderGroupHook;
extern struct Hook FO_NewFolderHook;
extern struct Hook FO_SetOrderHook;

BOOL            FO_CreateFolder(enum FolderType type, char *path, char *name);
struct Folder **FO_CreateList(void);
BOOL            FO_FreeFolder(struct Folder *folder);
struct Folder * FO_GetCurrentFolder(void);
BOOL            FO_SetCurrentFolder(struct Folder *fo);
struct Folder * FO_GetFolderByName(char *name, int *pos);
struct Folder * FO_GetFolderByType(enum FolderType type, int *pos);
struct Folder * FO_GetFolderRexx(char *arg, int *pos);
int             FO_GetFolderPosition(struct Folder *findfo);
BOOL            FO_LoadConfig(struct Folder *fo);
BOOL            FO_LoadTree(char *fname);
BOOL            FO_LoadFolderImages(struct Folder *fo);
struct Folder * FO_NewFolder(enum FolderType type, char *path, char *name);
void            FO_SaveConfig(struct Folder *fo);
BOOL            FO_SaveTree(char *fname);

#endif /* YAM_FOLDERCONFIG_H */

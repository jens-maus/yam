#ifndef YAM_FOLDERCONFIG_H
#define YAM_FOLDERCONFIG_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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
   APTR ST_FPATH;
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

// Foldertype macros
enum FolderType { FT_CUSTOM=0, FT_INCOMING, FT_OUTGOING, FT_SENT, FT_DELETED, FT_GROUP, FT_CUSTOMSENT, FT_CUSTOMMIXED };
#define isOutgoingFolder(folder)  ((folder)->Type == FT_OUTGOING || (folder)->Type == FT_SENT || (folder)->Type == FT_CUSTOMSENT)
#define isCustomFolder(folder)    ((folder)->Type == FT_CUSTOM || (folder)->Type == FT_CUSTOMSENT || (folder)->Type == FT_CUSTOMMIXED)

// SetOrder enum
enum SetOrder   { SO_SAVE=0, SO_RESET };

// LoadedMode enum (if folder index is valid/flushed or unloaded)
enum LoadedMode { LM_UNLOAD=0, LM_FLUSHED, LM_VALID };

// XPKType flags and macros
#define XPK_OFF         0
#define XPK_CRYPT       1
#define XPK_COMPRESSED  2
#define isCryptedFolder(folder)   (isFlagSet((folder)->XPKType, XPK_CRYPT))
#define isComprFolder(folder)     (isFlagSet((folder)->XPKType, XPK_COMPRESSED))

// flags and macros for the folder
#define FOFL_MODIFY  1
#define FOFL_FREEXS  2
#define isModified(folder)        (isFlagSet((folder)->Flags, FOFL_MODIFY))
#define isFreeAccess(folder)      (isFlagSet((folder)->Flags, FOFL_FREEXS))

#define FolderName(fo) ((fo) ? (fo)->Name : "?")

struct Folder
{
   APTR            BC_FImage;
   struct BodyChunkData *   FImage;
   struct Mail *   Messages;
   ULONG           Flags;
   LONG            Size;
   int             MLSignature;
   int             Total;
   int             New;
   int             Unread;
   int             Sent;
   int             Deleted;
   int             Sort[2];
   int             Stats;
   int             MaxAge;
   int             LastActive;
   int             SortIndex;
   int             Open;
   int             ImageIndex;
   int             XPKType;

   enum FolderType Type;
   enum LoadedMode LoadedMode;

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
int             FO_GetFolderPosition(struct Folder *findfo, BOOL withGroups);
BOOL            FO_LoadConfig(struct Folder *fo);
BOOL            FO_LoadTree(char *fname);
BOOL            FO_LoadFolderImages(struct Folder *fo);
struct Folder * FO_NewFolder(enum FolderType type, char *path, char *name);
BOOL            FO_SaveConfig(struct Folder *fo);
BOOL            FO_SaveTree(char *fname);

#endif /* YAM_FOLDERCONFIG_H */

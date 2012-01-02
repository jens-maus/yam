#ifndef YAM_FOLDERCONFIG_H
#define YAM_FOLDERCONFIG_H 1

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

#include <proto/intuition.h>

#include "YAM_stringsizes.h"

// forward declarations
struct MailList;

struct FO_GUIData
{
   Object *WI;
   Object *ST_FNAME;
   Object *ST_FPATH;
   Object *NM_MAXAGE;
   Object *CY_FMODE;
   Object *CY_FTYPE;
   Object *CY_SORT[2];
   Object *CH_REVERSE[2];
   Object *CH_EXPIREUNREAD;
   Object *ST_MLPATTERN;
   Object *ST_MLFROMADDRESS;
   Object *ST_MLREPLYTOADDRESS;
   Object *ST_MLADDRESS;
   Object *CY_MLSIGNATURE;
   Object *CH_STATS;
   Object *CH_MLSUPPORT;
   Object *BT_AUTODETECT;
   Object *BT_OKAY;
   Object *BT_CANCEL;
   Object *ST_HELLOTEXT;
   Object *ST_BYETEXT;
};

struct FO_ClassData  /* folder configuration window */
{
  struct FO_GUIData GUI;
  struct Folder *   EditFolder;
};

// Foldertype macros
enum FolderType { FT_CUSTOM=0,   // custom folder with received mail
                  FT_INCOMING,   // the mandatory INCOMING folder
                  FT_OUTGOING,   // the mandatory OUTGOING folder
                  FT_SENT,       // the mandatory SENT folder
                  FT_TRASH,      // the mandatory TRASH folder
                  FT_GROUP,      // folder is a group and not a real folder
                  FT_CUSTOMSENT, // custom folder with sent mail
                  FT_CUSTOMMIXED,// custom folder with sent&received mail
                  FT_SPAM,       // the mandatory SPAM folder
                  FT_NUM         // MUST be the last one in the enum!
                };

extern const char* const FolderName[FT_NUM];

#define isCustomFolder(folder)      ((folder)->Type == FT_CUSTOM)
#define isIncomingFolder(folder)    ((folder)->Type == FT_INCOMING)
#define isOutgoingFolder(folder)    ((folder)->Type == FT_OUTGOING)
#define isSentFolder(folder)        ((folder)->Type == FT_SENT)
#define isTrashFolder(folder)       ((folder)->Type == FT_TRASH)
#define isGroupFolder(folder)       ((folder)->Type == FT_GROUP)
#define isCustomSentFolder(folder)  ((folder)->Type == FT_CUSTOMSENT)
#define isCustomMixedFolder(folder) ((folder)->Type == FT_CUSTOMMIXED)
#define isSpamFolder(folder)        (C->SpamFilterEnabled == TRUE && (folder)->Type == FT_SPAM)

#define isDefaultFolder(folder)     (isIncomingFolder(folder) || \
                                     isOutgoingFolder(folder) || \
                                     isSentFolder(folder)     || \
                                     isTrashFolder(folder)    || \
                                     isSpamFolder(folder))

#define isSentMailFolder(folder) (isOutgoingFolder(folder)   || \
                                  isSentFolder(folder)       || \
                                  isCustomSentFolder(folder))

// SetOrder enum
enum SetOrder   { SO_SAVE=0, SO_RESET };

// LoadedMode enum (if folder index is valid/flushed or unloaded)
enum LoadedMode { LM_UNLOAD=0,  // invalid/unloaded
                  LM_FLUSHED,   // flushed
                  LM_VALID,     // valid index
                  LM_REBUILD,   // currently rebuilding
                };

// Folder modes
enum FolderMode { FM_NORMAL=0,  // normal folder
                  FM_SIMPLE,    // simple protected folder with PW
                  FM_XPKCOMP,   // XPK compressed folder
                  FM_XPKCRYPT   // XPK compressed+crypted folder
                };
#define isProtectedFolder(folder) (((folder)->Mode == FM_SIMPLE || (folder)->Mode == FM_XPKCRYPT))
#define isXPKFolder(folder)       (((folder)->Mode == FM_XPKCOMP || (folder)->Mode == FM_XPKCRYPT))

// flags and macros for the folder
#define FOFL_MODIFY  (1<<0)
#define FOFL_FREEXS  (1<<1)
#define isModified(folder)        (isFlagSet((folder)->Flags, FOFL_MODIFY))
#define isFreeAccess(folder)      (isFlagSet((folder)->Flags, FOFL_FREEXS))

#define FolderName(fo) ((fo) ? (fo)->Name : "?")

// for managing the folder colums via a bitmask
#define FCOL_NAME               (1<<0) // folder name
#define FCOL_TOTAL              (1<<1) // total amount of mails
#define FCOL_UNREAD             (1<<2) // number of unread mails
#define FCOL_NEW                (1<<3) // number of new mails
#define FCOL_SIZE               (1<<4) // size in bytes of folder
#define hasFColName(v)          (isFlagSet((v), FCOL_NAME))
#define hasFColTotal(v)         (isFlagSet((v), FCOL_TOTAL))
#define hasFColUnread(v)        (isFlagSet((v), FCOL_UNREAD))
#define hasFColNew(v)           (isFlagSet((v), FCOL_NEW))
#define hasFColSize(v)          (isFlagSet((v), FCOL_SIZE))

// for managing the different standard folder icons we manage our IDs and ESC sequences here
#define FICON_ID_FOLD           0  // folder_fold
#define FICON_ID_UNFOLD         1  // folder_unfold
#define FICON_ID_INCOMING       2  // folder_incoming
#define FICON_ID_INCOMING_NEW   3  // folder_incoming_new
#define FICON_ID_OUTGOING       4  // folder_outgoing
#define FICON_ID_OUTGOING_NEW   5  // folder_outgoing_new
#define FICON_ID_TRASH          6  // folder_deleted
#define FICON_ID_TRASH_NEW      7  // folder_deleted_new
#define FICON_ID_SENT           8  // folder_sent
#define FICON_ID_PROTECTED      9  // status_crypt
#define FICON_ID_SPAM           10 // folder_spam
#define FICON_ID_SPAM_NEW       11 // folder_spam_new

#define FICON_FOLD          "\033o[" STR(FICON_ID_FOLD)         "]"
#define FICON_UNFOLD        "\033o[" STR(FICON_ID_UNFOLD)       "]"
#define FICON_INCOMING      "\033o[" STR(FICON_ID_INCOMING)     "]"
#define FICON_INCOMING_NEW  "\033o[" STR(FICON_ID_INCOMING_NEW) "]"
#define FICON_OUTGOING      "\033o[" STR(FICON_ID_OUTGOING)     "]"
#define FICON_OUTGOING_NEW  "\033o[" STR(FICON_ID_OUTGOING_NEW) "]"
#define FICON_TRASH         "\033o[" STR(FICON_ID_TRASH)        "]"
#define FICON_TRASH_NEW     "\033o[" STR(FICON_ID_TRASH_NEW)    "]"
#define FICON_SENT          "\033o[" STR(FICON_ID_SENT)         "]"
#define FICON_PROTECTED     "\033o[" STR(FICON_ID_PROTECTED)    "]"
#define FICON_SPAM          "\033o[" STR(FICON_ID_SPAM)         "]"
#define FICON_SPAM_NEW      "\033o[" STR(FICON_ID_SPAM_NEW)     "]"

struct Folder
{
  Object *          imageObject;
  struct MailList * messages;
  struct MUI_NListtree_TreeNode *Treenode; // links to MainMailList
  struct FolderNode *parent; // ptr to parent folder node, NULL if parent is root
  ULONG             Flags;
  LONG              Size;
  int               MLSignature;
  int               Total;
  int               New;
  int               Unread;
  int               Sent;
  int               Deleted;
  int               Sort[2];
  int               MaxAge;
  int               LastActive;
  int               SortIndex;
  int               ImageIndex;

  enum FolderMode   Mode;
  enum FolderType   Type;
  enum LoadedMode   LoadedMode;

  time_t            lastAccessTime;        // when the folder was last accessed/loaded

  char              Name[SIZE_NAME];       // the name of the folder
  char              Path[SIZE_PATH];       // relative or absolute path of the folder's directory
  char              Fullpath[SIZE_PATH];   // absolute path of the folder's directory
  char              Password[SIZE_USERID];
  char              MLPattern[SIZE_PATTERN];
  char              MLAddress[SIZE_ADDRESS];
  char              MLFromAddress[SIZE_ADDRESS];
  char              MLReplyToAddress[SIZE_ADDRESS];
  char              WriteIntro[SIZE_INTRO];
  char              WriteGreetings[SIZE_INTRO];

  BOOL              ExpireUnread;
  BOOL              Stats;
  BOOL              MLSupport;
};

extern struct Hook FO_DeleteFolderHook;
extern struct Hook FO_EditFolderHook;
extern struct Hook FO_NewFolderGroupHook;
extern struct Hook FO_NewFolderHook;
extern struct Hook FO_SetOrderHook;

BOOL            FO_CreateFolder(enum FolderType type, const char * const path, const char *name);
BOOL            FO_FreeFolder(struct Folder *folder);
struct Folder * FO_GetFolderByName(const char *name, int *pos);
struct Folder * FO_GetFolderByPath(const char *path, int *pos);
struct Folder * FO_GetFolderByType(const enum FolderType type, int *pos);
struct Folder * FO_GetFolderRexx(const char *arg, int *pos);
int             FO_GetFolderPosition(struct Folder *findfo, BOOL withGroups);
BOOL            FO_LoadConfig(struct Folder *fo);
BOOL            FO_LoadTree(void);
struct Folder * FO_NewFolder(enum FolderType type, const char *path, const char *name);
BOOL            FO_SaveConfig(struct Folder *fo);
BOOL            FO_SaveTree(void);
void            FO_SetFolderImage(struct Folder *folder);
void            FO_UpdateStatistics(struct Folder *folder);
void            FO_UpdateTreeStatistics(const struct Folder *folder, const BOOL redraw);

struct Folder * GetCurrentFolder(void);
void            SetCurrentFolder(const struct Folder *folder);
void            ActivateFolder(const struct Folder *fo);

#endif /* YAM_FOLDERCONFIG_H */

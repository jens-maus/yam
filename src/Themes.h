#ifndef THEMES_H
#define THEMES_H

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

#include <exec/types.h>

// forward declarations
struct DiskObject;

enum ConfigImages
{
  ci_First = 0,
  ci_ABook = 0,
  ci_ABookBig,
  ci_Answer,
  ci_AnswerBig,
  ci_Filters,
  ci_FiltersBig,
  ci_FirstStep,
  ci_FirstStepBig,
  ci_Lists,
  ci_ListsBig,
  ci_LookFeel,
  ci_LookFeelBig,
  ci_MIME,
  ci_MIMEBig,
  ci_Misc,
  ci_MiscBig,
  ci_Network,
  ci_NetworkBig,
  ci_NewMail,
  ci_NewMailBig,
  ci_Read,
  ci_ReadBig,
  ci_Scripts,
  ci_ScriptsBig,
  ci_Security,
  ci_SecurityBig,
  ci_Signature,
  ci_SignatureBig,
  ci_Spam,
  ci_SpamBig,
  ci_Start,
  ci_StartBig,
  ci_Update,
  ci_UpdateBig,
  ci_Write,
  ci_WriteBig,
  ci_Max
};

enum FolderImages
{
  fi_First = 0,
  fi_Fold = 0,
  fi_Unfold,
  fi_Incoming,
  fi_IncomingNew,
  fi_Outgoing,
  fi_OutgoingNew,
  fi_Sent,
  fi_Spam,
  fi_SpamNew,
  fi_Trash,
  fi_TrashNew,
  fi_Max
};

enum IconImages
{
  ii_First = 0,
  ii_Check = 0,
  ii_Empty,
  ii_New,
  ii_Old,
  ii_Max
};

enum StatusImages
{
  si_First = 0,
  si_Attach = 0,
  si_Crypt,
  si_Delete,
  si_Download,
  si_Error,
  si_Forward,
  si_Group,
  si_Hold,
  si_Mark,
  si_New,
  si_Old,
  si_Reply,
  si_Report,
  si_Sent,
  si_Signed,
  si_Spam,
  si_Unread,
  si_Urgent,
  si_WaitSend,
  si_Max
};

enum ToolbarImageModes
{
  tbim_Normal = 0,
  tbim_Selected,
  tbim_Ghosted,
  tbim_Max
};

enum ToolbarImages
{
  tbi_First = 0,
  tbi_Read = 0,
  tbi_Edit,
  tbi_Move,
  tbi_Delete,
  tbi_GetAddr,
  tbi_NewMail,
  tbi_Reply,
  tbi_Forward,
  tbi_GetMail,
  tbi_SendAll,
  tbi_Spam,
  tbi_Ham,
  tbi_Filter,
  tbi_Find,
  tbi_AddrBook,
  tbi_Config,
  tbi_Prev,
  tbi_Next,
  tbi_PrevThread,
  tbi_NextThread,
  tbi_Display,
  tbi_Save,
  tbi_Print,
  tbi_Editor,
  tbi_Insert,
  tbi_Cut,
  tbi_Copy,
  tbi_Paste,
  tbi_Undo,
  tbi_Bold,
  tbi_Italic,
  tbi_Underline,
  tbi_Colored,
  tbi_NewUser,
  tbi_NewList,
  tbi_NewGroup,
  tbi_OpenTree,
  tbi_CloseTree,
  tbi_Max

};

enum MainWindowToolbarImages
{
  mwtbi_First = 0,
  mwtbi_Read = 0,
  mwtbi_Edit,
  mwtbi_Move,
  mwtbi_Delete,
  mwtbi_GetAddr,
  mwtbi_NewMail,
  mwtbi_Reply,
  mwtbi_Forward,
  mwtbi_GetMail,
  mwtbi_SendAll,
  mwtbi_Spam,
  mwtbi_Ham,
  mwtbi_Filter,
  mwtbi_Find,
  mwtbi_AddrBook,
  mwtbi_Config,
  mwtbi_Null,
  mwtbi_Max
};

enum ReadWindowToolbarImages
{
  rwtbi_First = 0,
  rwtbi_Prev = 0,
  rwtbi_Next,
  rwtbi_PrevThread,
  rwtbi_NextThread,
  rwtbi_Display,
  rwtbi_Save,
  rwtbi_Print,
  rwtbi_Delete,
  rwtbi_Move,
  rwtbi_Reply,
  rwtbi_Forward,
  rwtbi_Spam,
  rwtbi_Ham,
  rwtbi_Null,
  rwtbi_Max
};

enum WriteWindowToolbarImages
{
  wwtbi_First = 0,
  wwtbi_Editor= 0,
  wwtbi_Insert,
  wwtbi_Cut,
  wwtbi_Copy,
  wwtbi_Paste,
  wwtbi_Undo,
  wwtbi_Bold,
  wwtbi_Italic,
  wwtbi_Underline,
  wwtbi_Colored,
  wwtbi_Search,
  wwtbi_Null,
  wwtbi_Max
};

enum ABookWindowToolbarImages
{
  awtbi_First = 0,
  awtbi_Save = 0,
  awtbi_Find,
  awtbi_NewUser,
  awtbi_NewList,
  awtbi_NewGroup,
  awtbi_Edit,
  awtbi_Delete,
  awtbi_Print,
  awtbi_OpenTree,
  awtbi_CloseTree,
  awtbi_Null,
  awtbi_Max
};

struct Theme
{
  char *name;
  char *author;
  char *url;
  char *version;
  char *configImages[ci_Max];
  char *folderImages[fi_Max];
  char *iconImages[ii_Max];
  char *statusImages[si_Max];
  char *mainWindowToolbarImages[tbim_Max][mwtbi_Max];
  char *readWindowToolbarImages[tbim_Max][rwtbi_Max];
  char *writeWindowToolbarImages[tbim_Max][wwtbi_Max];
  char *abookWindowToolbarImages[tbim_Max][awtbi_Max];
  struct DiskObject *icons[ii_Max];

  char directory[SIZE_PATHFILE];
  BOOL loaded;
};

void AllocTheme(struct Theme *theme, const char *themeName);
void FreeTheme(struct Theme *theme);
LONG ParseThemeFile(const char *themeFile, struct Theme *theme);
void LoadTheme(struct Theme *theme, const char *themeName);
void UnloadTheme(struct Theme *theme);

char *BuildNListString(enum StatusImages id);
// a macro to build the necessary string to use an image in an NList object
#define SI_STR(id)  BuildNListString(id)

#endif /* THEMES_H */


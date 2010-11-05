/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <libraries/mui.h>
#include <mui/NListtree_mcc.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_config.h"

#include "FolderList.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
};

struct results
{
  char *username;
  char *email;
  char *realname;
  char *config;
  char *maildir;
  long *folders;
};

struct optional
{
  long folders;
};

void rx_userinfo(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
{
  struct args *args = params->args;
  struct results *results = params->results;
  struct optional *optional = params->optional;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      params->args = AllocVecPooled(G->SharedMemPool, sizeof(*args));
      params->results = AllocVecPooled(G->SharedMemPool, sizeof(*results));
      params->optional = AllocVecPooled(G->SharedMemPool, sizeof(*optional));
    }
    break;

    case RXIF_ACTION:
    {
      struct User *u = US_GetCurrentUser();
      int numfolders;
      struct FolderNode *fnode;

      results->username = u->Name;
      results->email = C->EmailAddress;
      results->realname = C->RealName;
      results->config = G->CO_PrefsFile;
      results->maildir = G->MA_MailDir;

      // count the real folders now
      numfolders = 0;
      LockFolderListShared(G->folders);
      ForEachFolderNode(G->folders, fnode)
      {
        struct Folder *folder = fnode->folder;

        if(!isGroupFolder(folder))
          numfolders++;
      }
      UnlockFolderList(G->folders);

      optional->folders = numfolders;
      results->folders = &optional->folders;
    }
    break;

    case RXIF_FREE:
    {
      if(args != NULL)
        FreeVecPooled(G->SharedMemPool, args);
      if(results != NULL)
        FreeVecPooled(G->SharedMemPool, results);
      if(optional != NULL)
        FreeVecPooled(G->SharedMemPool, optional);
    }
    break;
  }

  LEAVE();
}

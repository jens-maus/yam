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

#include <strings.h>
#include <ctype.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>

#include "extrasrc.h"

#include "YAM.h"

#include "mui/AddressBookWindow.h"

#include "Locale.h"
#include "Logfile.h"
#include "MUIObjects.h"
#include "Rexx.h"

#include "Debug.h"

struct args
{
  struct RexxResult varStem;
  char *type;
  char *alias;
  char *name;
  char *email;
};

struct results
{
  char *alias;
};

struct optional
{
  struct ABookNode abn;
};

void rx_addrnew(UNUSED struct RexxHost *host, struct RexxParams *params, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg)
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
      if(params->optional == NULL)
        params->rc = RETURN_ERROR;
    }
    break;

    case RXIF_ACTION:
    {
      enum ABookNodeType type;

      if(args->type != NULL)
      {
        if(tolower(*args->type) == 'g')
          type = ABNT_GROUP;
        else if(tolower(*args->type) == 'l')
          type = ABNT_LIST;
        else
          type = ABNT_USER;
      }
      else
        type = ABNT_USER;

      InitABookNode(&optional->abn, type);

      if(args->alias != NULL)
        strlcpy(optional->abn.Alias, args->alias, sizeof(optional->abn.Alias));
      if(args->name != NULL)
        strlcpy(optional->abn.RealName, args->name, sizeof(optional->abn.RealName));
      if(args->email != NULL)
        strlcpy(optional->abn.Address, args->email, sizeof(optional->abn.Address));

      if(IsStrEmpty(optional->abn.Alias) == TRUE)
      {
        if(optional->abn.type == ABNT_USER)
          SetDefaultAlias(&optional->abn);
        else
          params->rc = RETURN_ERROR;
      }

      if(params->rc == 0)
      {
        struct ABookNode *abn;

        FixAlias(&G->abook, &optional->abn, NULL);
        results->alias = optional->abn.Alias;

        if((abn = CreateABookNode(optional->abn.type)) != NULL)
        {
          struct ABookNode *group;
          struct ABookNode *afterThis;

          memcpy(abn, &optional->abn, sizeof(*abn));

          if(G->ABookWinObject != NULL)
          {
            group = (struct ABookNode *)xget(G->ABookWinObject, MUIA_AddressBookWindow_ActiveGroup);
            afterThis = (struct ABookNode *)xget(G->ABookWinObject, MUIA_AddressBookWindow_ActiveEntry);
          }
          else
          {
            group = &G->abook.rootGroup;
            afterThis = NULL;
          }

          AddABookNode(group, abn, afterThis);
          G->abook.arexxABN = abn;
          G->abook.modified = TRUE;

          // update an existing address book window as well
          if(G->ABookWinObject != NULL)
            DoMethod(G->ABookWinObject, MUIM_AddressBookWindow_RebuildTree);

          set(G->ABookWinObject, MUIA_AddressBookWindow_Modified, TRUE);
          AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), optional->abn.Alias);
        }
        else
          params->rc = RETURN_ERROR;
      }
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

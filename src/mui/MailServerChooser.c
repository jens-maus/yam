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

 Superclass:  MUIC_Cycle
 Description: Cycle object to choose a mail server

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "MailServerChooser_cl.h"

#include "YAM_utilities.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "MailServers.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct MinList *serverList;       // list of servers
  struct MailServerNode *curServer; // ptr to currently active mail server
  char **serverArray;               // titles for the different servers that can be selected
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_CycleChain,    TRUE,
    MUIA_Font,          MUIV_Font_Button,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // we must know the list on which we operate as there is a difference
    // between the global config and the config being edited
    data->serverList = (struct MinList *)GetTagData(ATTR(MailServerList), (IPTR)NULL, inittags(msg));
    data->curServer = (struct MailServerNode *)GetTagData(ATTR(MailServer), (IPTR)NULL, inittags(msg));

    // set up the full description of all active mail servers
    DoMethod(obj, METHOD(UpdateMailServers));

    // notify ourselves about changed active items
    DoMethod(obj, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(MailServerIndex), MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  ULONG result;

  ENTER();

  // free the string array
  FreeStrArray(data->serverArray);

  // signal the super class to dispose as well
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(MailServerList):
      {
        data->serverList = (struct MinList *)tag->ti_Data;
        DoMethod(obj, METHOD(UpdateMailServers));
	  }
	  break;

      case ATTR(MailServer):
      {
        struct MailServerNode *newServer = (struct MailServerNode *)tag->ti_Data;

        if(newServer != data->curServer && data->serverList != NULL)
        {
          int j = 0;

          // find the new server and set it as active entry
          if(newServer != NULL)
          {
            int i = 0;
            struct Node *curNode;

            IterateList(data->serverList, curNode)
            {
              struct MailServerNode *msn = (struct MailServerNode *)curNode;

              if(msn->id == newServer->id)
              {
                j = i;
                break;
              }
              else if(isServerActive(msn))
                i++;
            }
          }

          data->curServer = newServer;

          // set the new active item without triggering notifications
          nnset(obj, MUIA_Cycle_Active, j);
        }
      }
      break;

      case ATTR(MailServerIndex):
      {
        if(data->serverList != NULL)
        {
          struct MailServerNode *newServer = GetMailServer(data->serverList, tag->ti_Data);

          // set the new server and trigger possible notifications
          set(obj, ATTR(MailServer), newServer);
        }
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(MailServer): *store = (IPTR)data->curServer; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(UpdateMailServers)
// updates the str array containing all servers
DECLARE(UpdateMailServers)
{
  GETDATA;

  ENTER();

  // we have to sync the content of the server list
  // with the GUI elements of the write window
  FreeStrArray(data->serverArray);

  if(data->serverList != NULL)
  {
    struct Node *curNode;
    int numServers = 0;

    // first we find out how many entries the server list has
    IterateList(data->serverList, curNode)
    {
      struct MailServerNode *msn = (struct MailServerNode *)curNode;

      if(isServerActive(msn))
        numServers++;
    }

    // allocate enough space +1 for NUL termination
    if((data->serverArray = calloc(MAX(2, numServers+1), sizeof(char *))) != NULL)
    {
    // set a single empty entry if no active servers were found
    // this works around a bug in MUI4 of MorphOS which uses a non-static
    // replacement entry on the stack instead otherwise
      if(numServers == 0)
      {
        data->serverArray[0] = strdup("");
      }
      else
      {
        int i;

        // now we walk through the serverList again
        // and clone the description string
        i = 0;
        IterateList(data->serverList, curNode)
        {
          struct MailServerNode *msn = (struct MailServerNode *)curNode;

          if(isServerActive(msn))
          {
            // construct the new string via asprintf() so that the necessary
            // memory is automatically allocated.
            data->serverArray[i] = strdup(msn->description);

            i++;
          }
        }
      }

      // update the entry strings and set the active entry
      nnset(obj, MUIA_Cycle_Entries, data->serverArray);
    }
  }

  RETURN(0);
  return 0;
}

///

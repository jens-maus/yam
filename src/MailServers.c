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

#include <stdlib.h>
#include <string.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <proto/exec.h>

#include "YAM_config.h"
#include "YAM_utilities.h"

#include "extrasrc.h"

#include "MailServers.h"

#include "Debug.h"

/***************************************************************************
 Module: MailServer related routines
***************************************************************************/

/// CreateNewMailServer
//  Initializes a new POP3/SMTP account
struct MailServerNode *CreateNewMailServer(const enum MailServerType type, const struct Config *co, const BOOL first)
{
  struct MailServerNode *msn;

  ENTER();

  if((msn = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*msn),
                                          ASONODE_Min, TRUE,
                                          TAG_DONE)) != NULL)
  {
    // initialize all variables as AllocSysObject() does not clear the memory
    msn->type = type;
    msn->id = 0;
    msn->account[0] = '\0';
    msn->hostname[0] = '\0';
    msn->domain[0] = '\0';
    msn->port = 0;
    msn->username[0] = '\0';
    msn->password[0] = '\0';
    msn->flags = MSF_ACTIVE;
    msn->smtpFlags = 0;

    switch(msn->type)
    {
      case MST_POP3:
      {
        if(first == TRUE)
        {
          char *p = strchr(co->EmailAddress, '@');

          strlcpy(msn->username, co->EmailAddress, p ? (unsigned int)(p - co->EmailAddress + 1) : sizeof(msn->username));

          // now we get the first SMTP server in our list and reuse
          // the hostname of it for the new POP3 server
          #warning "FIXME: use first SMTP server from list"
          //strlcpy(msn->hostname, co->SMTP_Server, sizeof(msn->hostname));
        }

        msn->port = 110;
        SET_FLAG(msn->flags, MSF_PURGEMESSGAES);
      }
      break;

      case MST_SMTP:
      {
        msn->port = 25;
      }
      break;

      default:
        // nothing to do
      break;
    }
  }

  RETURN(msn);
  return msn;
}

///
/// FreeMailServerList
void FreeMailServerList(struct MinList *mailServerList)
{
  struct Node *curNode;

  ENTER();


  // we have to free the mailServerList
  while((curNode = RemHead((struct List *)mailServerList)) != NULL)
  {
    struct MailServerNode *msn = (struct MailServerNode *)curNode;

    FreeSysObject(ASOT_NODE, msn);
  }

  NewMinList(mailServerList);

  LEAVE();
}

///
/// CompareMailServerLists
// compare two MailServer lists
BOOL CompareMailServerLists(const struct MinList *msl1, const struct MinList *msl2)
{
  BOOL equal = TRUE;
  BOOL empty1;
  BOOL empty2;

  ENTER();

  empty1 = IsMinListEmpty(msl1);
  empty2 = IsMinListEmpty(msl2);
  if(empty1 == FALSE && empty2 == FALSE)
  {
    struct Node *node1 = GetHead((struct List *)msl1);
    struct Node *node2 = GetHead((struct List *)msl2);

    // walk through both lists in parallel and compare the single nodes
    while(node1 != NULL && node2 != NULL)
    {
      struct MailServerNode *msn1 = (struct MailServerNode *)node1;
      struct MailServerNode *msn2 = (struct MailServerNode *)node2;

      // compare every single member of the structure
      // "UIDLchecked" must not be checked, because that is not saved but
      // modified while YAM is looking for new mails.
      if(msn1->type != msn2->type ||
         strcmp(msn1->account,  msn2->account) != 0 ||
         strcmp(msn1->hostname, msn2->hostname) != 0 ||
         strcmp(msn1->username, msn2->username) != 0 ||
         strcmp(msn1->password, msn2->password) != 0 ||
         msn1->port           != msn2->port ||
         msn1->flags          != msn2->flags ||
         msn1->smtpFlags      != msn2->smtpFlags)
      {
        // something does not match
        equal = FALSE;
        break;
      }

      node1 = GetSucc(node1);
      node2 = GetSucc(node2);
    }

    // if there are any nodes left then the two lists cannot be equal
    if((node1 != NULL && GetSucc(node1) != NULL) || (node2 != NULL && GetSucc(node2) != NULL))
    {
      equal = FALSE;
    }
  }
  else if((empty1 == TRUE && empty2 == FALSE) || (empty1 == FALSE && empty2 == TRUE))
  {
    // if one list is empty while the other is not the two lists cannot be equal
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// GetMailServer
// function to extract the structure of a POP3 server from our mailserver list
struct MailServerNode *GetMailServer(const struct MinList *mailServerList, const enum MailServerType type, const unsigned int num)
{
  struct MailServerNode *result = NULL;

  ENTER();

  if(IsMinListEmpty(mailServerList) == FALSE)
  {
    unsigned int count = 0;
    struct Node *curNode;

    IterateList(mailServerList, curNode)
    {
      struct MailServerNode *msn = (struct MailServerNode *)curNode;

      if(msn->type == type)
      {
        if(count == num)
        {
          result = msn;
          break;
        }

        count++;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// IsUniqueMailServerID
// check if the ID is unique within the list of servers
BOOL IsUniqueMailServerID(const struct MinList *mailServerList, const int id)
{
  BOOL isUnique = TRUE;
  struct Node *curNode;

  ENTER();


  IterateList(mailServerList, curNode)
  {
    struct MailServerNode *msn = (struct MailServerNode *)curNode;

    if(msn->id == id)
    {
      // we found exactly this ID, this is bad
      isUnique = FALSE;
      break;
    }
  }

  RETURN(isUnique);
  return isUnique;
}

///

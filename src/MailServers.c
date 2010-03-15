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

#include "MailServers.h"

#include "Debug.h"

/***************************************************************************
 Module: MailServer related routines
***************************************************************************/

/// CreateNewMailServer
//  Initializes a new POP3/SMTP account
struct MailServerNode *CreateNewMailServer(enum MailServerType type, struct Config *co, BOOL first)
{
  struct MailServerNode *msn;

  ENTER();

  if((msn = (struct MailServerNode *)calloc(1, sizeof(struct MailServerNode))) != NULL)
  {
    msn->type = type;
    SET_FLAG(msn->flags, MSF_ACTIVE);

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
    }
  }

  RETURN(msn);
  return msn;
}

///
/// FreeMailServerList
void FreeMailServerList(struct MinList *mailServerList)
{
  ENTER();

  if(IsListEmpty((struct List *)mailServerList) == FALSE)
  {
    struct MinNode *curNode;

    // we have to free the mimeTypeList
    while((curNode = (struct MinNode *)RemHead((struct List *)mailServerList)) != NULL)
    {
      struct MailServerNode *msn = (struct MailServerNode *)curNode;

      free(msn);
    }

    NewList((struct List *)mailServerList);
  }

  LEAVE();
}

///
/// CompareMailServerLists
// compare two MailServer lists
BOOL CompareMailServerLists(const struct MinList *msl1, const struct MinList *msl2)
{
  BOOL equal = TRUE;
  struct MinNode *mln1 = msl1->mlh_Head;
  struct MinNode *mln2 = msl2->mlh_Head;

  ENTER();

  // walk through both lists in parallel and compare the single nodes
  while(mln1->mln_Succ != NULL && mln2->mln_Succ != NULL)
  {
    struct MailServerNode *msn1 = (struct MailServerNode *)mln1;
    struct MailServerNode *msn2 = (struct MailServerNode *)mln2;

    // compare every single member of the structure
    // "UIDLchecked" must not be checked, because that is not saved but
    // modified while YAM is looking for new mails.
    if(msn1->type != msn2->type ||
       strcmp(msn1->account,  msn2->account) != 0 ||
       strcmp(msn1->hostname, msn2->hostname) != 0 ||
       strcmp(msn1->username, msn2->username) != 0 ||
       strcmp(msn1->password, msn2->password) != 0 ||
       msn1->port           != msn2->port ||
       isServerActive(msn1) != isServerActive(msn2) ||
       hasServerAPOP(msn1)  != hasServerAPOP(msn2)  ||
       hasServerPurge(msn1) != hasServerPurge(msn2) ||
       hasServerSSL(msn1)   != hasServerSSL(msn2)   ||
       hasServerTLS(msn1)       != hasServerSSL(msn2)   ||
       hasServerAuth_AUTO(msn1)   != hasServerAuth_AUTO(msn2)   ||
       hasServerAuth_DIGEST(msn1) != hasServerAuth_DIGEST(msn2) ||
       hasServerAuth_CRAM(msn1)   != hasServerAuth_CRAM(msn2)   ||
       hasServerAuth_LOGIN(msn1)  != hasServerAuth_LOGIN(msn2)  ||
       hasServerAuth_PLAIN(msn1)  != hasServerAuth_PLAIN(msn2)  ||
       hasServer8bit(msn1)        != hasServer8bit(msn2))
    {
      // something does not match
      equal = FALSE;
      break;
    }

    mln1 = mln1->mln_Succ;
    mln2 = mln2->mln_Succ;
  }

  // if there are any nodes left then the two lists cannot be equal
  if(mln1->mln_Succ != NULL || mln2->mln_Succ != NULL)
    equal = FALSE;

  RETURN(equal);
  return equal;
}

///
/// GetPOP3Server
// function to extract the structure of a POP3 server from our mailserver list
struct MailServerNode *GetMailServer(struct MinList *mailServerList, enum MailServerType type, unsigned int num)
{
  struct MailServerNode *result = NULL;

  ENTER();

  if(IsListEmpty((struct List *)mailServerList) == FALSE)
  {
    unsigned int count = 0;
    struct MinNode *curNode;

    for(curNode = mailServerList->mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
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


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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <proto/exec.h>

#include "YAM_config.h"
#include "YAM_utilities.h"

#include "extrasrc.h"

#include "MailList.h"
#include "MailServers.h"
#include "Timer.h"
#include "UserIdentity.h"

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
    memset(msn, 0, sizeof(*msn));
    msn->flags = MSF_ACTIVE;

    switch(type)
    {
      case MST_POP3:
      {
        // POP3 servers keep a list of downloaded mails
        if((msn->downloadedMails = CreateMailList()) != NULL)
        {
          if(CreateTRequest(&msn->downloadTimer, -1, msn) == TRUE)
          {
            if(first == TRUE)
            {
              struct UserIdentityNode *uin;
              struct MailServerNode *smtpMSN;

              if((uin = GetUserIdentity(&co->userIdentityList, 0, TRUE)) != NULL)
              {
                char *p = strchr(uin->address, '@');
                strlcpy(msn->username, uin->address, p ? (unsigned int)(p - uin->address + 1) : sizeof(msn->username));
              }

              // now we get the first SMTP server in our list and reuse
              // the hostname of it for the new POP3 server
              if((smtpMSN = GetMailServer(&co->smtpServerList, 0)) != NULL)
                strlcpy(msn->hostname, smtpMSN->hostname, sizeof(msn->hostname));
            }

            msn->port = 110;
            setFlag(msn->flags, MSF_PURGEMESSGAES);
            setFlag(msn->flags, MSF_AVOID_DUPLICATES);
            setFlag(msn->flags, MSF_DOWNLOAD_LARGE_MAILS);
            // set a download interval of 10 minutes, but don't enable it
            msn->downloadInterval = 10;
            msn->largeMailSizeLimit = 1024;
          }
          else
          {
            DeleteMailList(msn->downloadedMails);
            FreeSysObject(ASOT_NODE, msn);
            msn = NULL;
		  }
        }
        else
        {
          FreeSysObject(ASOT_NODE, msn);
          msn = NULL;
        }
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
/// CloneMailServer
struct MailServerNode *CloneMailServer(const struct MailServerNode *msn)
{
  struct MailServerNode *clone;

  ENTER();

  if((clone = DuplicateNode(msn, sizeof(*msn))) != NULL)
  {
    // the clone is not in use
    clearFlag(clone->flags, MSF_IN_USE);

    // POP3 servers keep a list of downloaded mails
    if(msn->downloadedMails != NULL)
    {
      if((clone->downloadedMails = CreateMailList()) != NULL)
      {
        if(CreateTRequest(&clone->downloadTimer, -1, clone) == FALSE)
        {
          DeleteMailList(clone->downloadedMails);
          FreeSysObject(ASOT_NODE, clone);
          clone = NULL;
        }
      }
      else
      {
        FreeSysObject(ASOT_NODE, clone);
        clone = NULL;
	  }
    }
  }

  RETURN(clone);
  return clone;
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

    if(msn->downloadedMails != NULL)
      DeleteMailList(msn->downloadedMails);

    if(msn->downloadTimer.tr != NULL)
      DeleteTRequest(&msn->downloadTimer);

    FreeSysObject(ASOT_NODE, msn);
  }

  NewMinList(mailServerList);

  LEAVE();
}

///
/// CompareMailServerNodes
static BOOL CompareMailServerNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct MailServerNode *msn1 = (const struct MailServerNode *)n1;
  const struct MailServerNode *msn2 = (const struct MailServerNode *)n2;

  ENTER();

  // compare every single member of the structure, except the
  // list of downloaded mails for POP3 servers
  if(msn1->id != msn2->id ||
     strcmp(msn1->description,  msn2->description) != 0 ||
     strcmp(msn1->hostname, msn2->hostname) != 0 ||
     strcmp(msn1->username, msn2->username) != 0 ||
     strcmp(msn1->password, msn2->password) != 0 ||
     msn1->port           != msn2->port ||
     msn1->flags          != msn2->flags ||
     msn1->smtpFlags      != msn2->smtpFlags ||
     msn1->preselection   != msn2->preselection)
  {
    // something does not match
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///

/// CompareMailServerLists
// compare two MailServer lists
BOOL CompareMailServerLists(const struct MinList *msl1, const struct MinList *msl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)msl1, (const struct List *)msl2, CompareMailServerNodes);

  RETURN(equal);
  return equal;
}

///
/// GetMailServer
// return the num-th server from the list of servers
struct MailServerNode *GetMailServer(const struct MinList *mailServerList, const unsigned int num)
{
  struct MailServerNode *result = NULL;
  unsigned int count = 0;
  struct Node *curNode;

  ENTER();

  IterateList(mailServerList, curNode)
  {
    struct MailServerNode *msn = (struct MailServerNode *)curNode;

    if(count == num)
    {
      result = msn;
      break;
    }

    count++;
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
/// FindMailServer
// find a mail server based on a unique ID
struct MailServerNode *FindMailServer(const struct MinList *mailServerList, const int id)
{
  struct MailServerNode *result = NULL;
  struct Node *curNode;

  ENTER();

  IterateList(mailServerList, curNode)
  {
    struct MailServerNode *msn = (struct MailServerNode *)curNode;

    if(msn->id == id)
    {
      result = msn;
      break;
    }
  }

  RETURN(result);
  return result;
}

///

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/exec.h>

#include "YAM.h"
#include "YAM_utilities.h"

#include "extrasrc.h"

#include "Config.h"
#include "Locale.h"
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

  if((msn = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*msn),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    // initialize all variables as AllocSysObject() does not clear the memory
    memset(msn, 0, sizeof(*msn));

    InitSemaphore(&msn->lock);
    msn->type = type;
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
            struct Folder *incomingFolder;

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
            #if defined(__amigaos4__)
            if(G->applicationID != 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 2) == TRUE)
              msn->notifyByOS41System = TRUE;
            else
              msn->notifyByRequester = TRUE;
            #else
            msn->notifyByRequester = TRUE;
            #endif

            // get the name of the incoming folder so that it will be
            // the default of that mail server
            if((incomingFolder = FO_GetFolderByType(FT_INCOMING, NULL)) != NULL)
            {
              msn->mailStoreFolderID = incomingFolder->ID;
              strlcpy(msn->mailStoreFolderName, incomingFolder->Name, sizeof(msn->mailStoreFolderName));
            }
            else
            {
              // leave the folder ID unassigned, this will be resolved later
              strlcpy(msn->mailStoreFolderName, tr(MSG_MA_Incoming), sizeof(msn->mailStoreFolderName));
            }
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
        struct Folder *sentFolder;

        msn->port = 25;

        // get the name of the incoming folder so that it will be
        // the default of that mail server
        if((sentFolder = FO_GetFolderByType(FT_SENT, NULL)) != NULL)
        {
          msn->mailStoreFolderID = sentFolder->ID;
          strlcpy(msn->mailStoreFolderName, sentFolder->Name, sizeof(msn->mailStoreFolderName));
        }
        else
        {
          // leave the folder ID unassigned, this will be resolved later
          strlcpy(msn->mailStoreFolderName, tr(MSG_MA_Sent), sizeof(msn->mailStoreFolderName));
        }
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
    memset(&clone->lock, 0, sizeof(clone->lock));
    InitSemaphore(&clone->lock);

    // the clone is not in use yet
    clone->useCount = 0;

    // prepare some stuff for POP3 servers
    if(msn->type == MST_POP3)
    {
      // POP3 servers keep a list of downloaded mails and a timer
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
/// DeleteMailServer
void DeleteMailServer(struct MailServerNode *msn)
{
  ENTER();

  // free some additional stuff of POP3 servers first
  if(msn->type == MST_POP3)
  {
    DeleteMailList(msn->downloadedMails);
    DeleteTRequest(&msn->downloadTimer);
  }

  FreeSysObject(ASOT_NODE, msn);

  LEAVE();
}

///
/// FreeMailServerList
void FreeMailServerList(struct MinList *mailServerList)
{
  struct MailServerNode *msn;
  struct MailServerNode *next;

  ENTER();

  // we have to free the mailServerList
  SafeIterateList(mailServerList, struct MailServerNode *, msn, next)
  {
    DeleteMailServer(msn);
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

  // compare the common members of the structure first
  if(msn1->id             != msn2->id ||
     strcmp(msn1->description, msn2->description) != 0 ||
     strcmp(msn1->hostname,    msn2->hostname) != 0 ||
     strcmp(msn1->username,    msn2->username) != 0 ||
     strcmp(msn1->password,    msn2->password) != 0 ||
     strcmp(msn1->certFingerprint, msn2->certFingerprint) != 0 ||
     msn1->mailStoreFolderID != msn2->mailStoreFolderID ||
     strcmp(msn1->mailStoreFolderName, msn2->mailStoreFolderName) != 0 ||
     msn1->certFailures   != msn2->certFailures ||
     msn1->port           != msn2->port ||
     msn1->flags          != msn2->flags)
  {
    // something does not match
    equal = FALSE;
  }

  // if the two nodes are still considered equal then compare
  // some server type specific stuff
  if(equal == TRUE)
  {
    if(msn1->type == MST_POP3)
    {
      if(msn1->preselection       != msn2->preselection ||
         msn1->downloadInterval   != msn2->downloadInterval ||
         msn1->largeMailSizeLimit != msn2->largeMailSizeLimit ||
         msn1->notifyByRequester  != msn2->notifyByRequester ||
         msn1->notifyByOS41System != msn2->notifyByOS41System ||
         msn1->notifyBySound      != msn2->notifyBySound ||
         msn1->notifyByCommand    != msn2->notifyByCommand ||
         strcmp(msn1->notifySound,   msn2->notifySound) != 0 ||
         strcmp(msn1->notifyCommand, msn2->notifyCommand) != 0)
      {
        // something does not match
        equal = FALSE;
      }
    }
    else if(msn1->type == MST_SMTP)
    {
      // nothing to be compared here
      // the smtpFlags field will be modified during a connection but it is never
      // saved to a configuration file
    }
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
  struct MailServerNode *result;

  ENTER();

  result = (struct MailServerNode *)GetNthNode(mailServerList, num);

  RETURN(result);
  return result;
}

///
/// IsUniqueMailServerID
// check if the ID is unique within the list of servers
BOOL IsUniqueMailServerID(const struct MinList *mailServerList, const int id)
{
  BOOL isUnique = TRUE;
  struct MailServerNode *msn;

  ENTER();

  IterateList(mailServerList, struct MailServerNode *, msn)
  {
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
  struct MailServerNode *msn;

  ENTER();

  IterateList(mailServerList, struct MailServerNode *, msn)
  {
    if(msn->id == id)
    {
      result = msn;
      break;
    }
  }

  if(result == NULL)
  {
    // fall back to the first configured mail server if the requested one could not be found
    W(DBF_ALWAYS, "mail server with id %08lx not found, using first one", id);
    result = GetMailServer(mailServerList, 0);
    if(result == NULL)
      E(DBF_ALWAYS, "there is no mail server configured at all!");
  }

  RETURN(result);
  return result;
}

///

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

#include <proto/exec.h>

#include "extrasrc.h"

#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "Locale.h"
#include "MailServers.h"
#include "Signature.h"
#include "UserIdentity.h"

#include "Debug.h"

/***************************************************************************
 Module: UserIdentity related routines
***************************************************************************/

/// CreateNewUserIdentity
//  Initializes a new UserIdentityNode
struct UserIdentityNode *CreateNewUserIdentity(const struct Config *co)
{
  struct UserIdentityNode *uin;

  ENTER();

  if((uin = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*uin),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    struct MailServerNode *msn;
    struct Folder *sentFolder;

    // initialize all variables as AllocSysObject() does not clear the memory
    memset(uin, 0, sizeof(*uin));
    uin->active = TRUE;

    // now we fill the UserIdentity structure with some sensible
    // defaults
    uin->quoteMails = TRUE;
    uin->quotePosition = QPOS_BELOW;
    uin->signaturePosition = SPOS_BELOW;
    uin->pgpSelfEncrypt = TRUE;
    uin->saveSentMail = TRUE;
    uin->sigReply = TRUE;
    uin->sigForwarding = TRUE;

    // we get the first valid smtpServer from the list and put that
    // as the default one so that we don't have a NULL pointer in
    // uin->smtpServer
    IterateList(&co->smtpServerList, struct MailServerNode *, msn)
    {
      if(isServerActive(msn))
      {
        uin->smtpServer = msn;
        break;
      }
    }

    // we get the first valid signature from the list and put that
    // as the default one. However, putting NULL there would mean
    // no signature
    uin->signature = GetSignature(&co->signatureList, 0, TRUE);

    // get the name of the first sent folder so that we make
    // that one as the default
    if((sentFolder = FO_GetFolderByType(FT_SENT, NULL)) != NULL)
    {
      uin->sentFolderID = sentFolder->ID;
    }
  }

  RETURN(uin);
  return uin;
}

///
/// DeleteUserIdentity
void DeleteUserIdentity(struct UserIdentityNode *uin)
{
  ENTER();

  FreeSysObject(ASOT_NODE, uin);

  LEAVE();
}

///
/// FreeUserIdentityList
void FreeUserIdentityList(struct MinList *userIdentityList)
{
  struct UserIdentityNode *uin;
  struct UserIdentityNode *next;

  ENTER();

  // we have to free the userIdentityList
  SafeIterateList(userIdentityList, struct UserIdentityNode *, uin, next)
  {
    DeleteUserIdentity(uin);
  }
  NewMinList(userIdentityList);

  LEAVE();
}

///
/// CompareUserIdentityNodes
static BOOL CompareUserIdentityNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct UserIdentityNode *uin1 = (const struct UserIdentityNode *)n1;
  const struct UserIdentityNode *uin2 = (const struct UserIdentityNode *)n2;

  ENTER();

  // compare every single member of the structure
  if(uin1->id != uin2->id ||
     uin1->active != uin2->active ||
     strcmp(uin1->description, uin2->description) != 0 ||
     strcmp(uin1->realname, uin2->realname) != 0 ||
     strcmp(uin1->address, uin2->address) != 0 ||
     strcmp(uin1->organization, uin2->organization) != 0 ||
     (uin1->smtpServer != NULL ? uin1->smtpServer->id : -1) != (uin2->smtpServer != NULL ? uin2->smtpServer->id : -1) ||
     (uin1->signature != NULL ? uin1->signature->id : -1) != (uin2->signature != NULL ? uin2->signature->id : -1) ||
     strcmp(uin1->mailCC, uin2->mailCC) != 0 ||
     strcmp(uin1->mailBCC, uin2->mailBCC) != 0 ||
     strcmp(uin1->mailReplyTo, uin2->mailReplyTo) != 0 ||
     strcmp(uin1->extraHeaders, uin2->extraHeaders) != 0 ||
     strcmp(uin1->photoURL, uin2->photoURL) != 0 ||
     strcmp(uin1->sentFolderName, uin2->sentFolderName) != 0 ||
     uin1->sentFolderID != uin2->sentFolderID ||
     uin1->saveSentMail != uin2->saveSentMail ||
     uin1->quoteMails != uin2->quoteMails ||
     uin1->quotePosition != uin2->quotePosition ||
     uin1->signaturePosition != uin2->signaturePosition ||
     uin1->sigReply != uin2->sigReply ||
     uin1->sigForwarding != uin2->sigForwarding ||
     uin1->addPersonalInfo != uin2->addPersonalInfo ||
     uin1->requestMDN != uin2->requestMDN ||
     uin1->usePGP != uin2->usePGP ||
     strcmp(uin1->pgpKeyID, uin2->pgpKeyID) != 0 ||
     strcmp(uin1->pgpKeyURL, uin2->pgpKeyURL) != 0 ||
     uin1->pgpSignUnencrypted != uin2->pgpSignUnencrypted ||
     uin1->pgpSignEncrypted != uin2->pgpSignEncrypted ||
     uin1->pgpEncryptAll != uin2->pgpEncryptAll ||
     uin1->pgpSelfEncrypt != uin2->pgpSelfEncrypt)
  {
    // something does not match
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareUserIdentityLists
// compare two UserIdentity lists
BOOL CompareUserIdentityLists(const struct MinList *msl1, const struct MinList *msl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)msl1, (const struct List *)msl2, CompareUserIdentityNodes);

  RETURN(equal);
  return equal;
}

///
/// GetUserIdentity
// function to extract the structure of a User Identity from our user identity list
struct UserIdentityNode *GetUserIdentity(const struct MinList *userIdentityList,
                                         const unsigned int num, const BOOL activeOnly)
{
  struct UserIdentityNode *result = NULL;
  struct UserIdentityNode *uin;
  unsigned int count = 0;

  ENTER();

  IterateList(userIdentityList, struct UserIdentityNode *, uin)
  {
    if(activeOnly == FALSE || uin->active == TRUE)
    {
      if(count == num)
      {
        result = uin;
        break;
      }

      count++;
    }
  }

  RETURN(result);
  return result;
}

///
/// WhichUserIdentity
// returns the user identity which belongs to a certain email structure
struct UserIdentityNode *WhichUserIdentity(const struct MinList *userIdentityList, const struct ExtendedMail *email)
{
  struct UserIdentityNode *result = NULL;
  struct UserIdentityNode *uin;

  ENTER();

  // now we try to find out which user identity matches
  // to a certain mail (thus, which mail was received by which user identity)
  // here we simply browse through all potential recipient addresses of
  // the email and compare it to the current user identity
  IterateList(userIdentityList, struct UserIdentityNode *, uin)
  {
    int i;

    // search all To: addresses
    if(stricmp(email->Mail.To.Address, uin->address) == 0)
    {
      result = uin;
      break;
    }

    for(i=0; i < email->NumSTo; i++)
    {
      if(stricmp(email->STo[i].Address, uin->address) == 0)
      {
        result = uin;
        break;
      }
    }

    if(result != NULL)
      break;

    // search all CC addresses
    for(i=0; i < email->NumCC; i++)
    {
      if(stricmp(email->CC[i].Address, uin->address) == 0)
      {
        result = uin;
        break;
      }
    }

    if(result != NULL)
      break;
  }

  if(result == NULL)
  {
    result = GetUserIdentity(userIdentityList, 0, TRUE);
    E(DBF_MAIL, "found no matching user identity, falling back to default identity '%s'", result != NULL ? result->description : "NULL");
  }

  RETURN(result);
  return result;
}

///
/// FindUserIdentityByID
// find a user identity by a given ID
struct UserIdentityNode *FindUserIdentityByID(const struct MinList *userIdentityList, const int id)
{
  struct UserIdentityNode *result = NULL;

  ENTER();

  if(id > 0)
  {
    struct UserIdentityNode *uin;

    IterateList(userIdentityList, struct UserIdentityNode *, uin)
    {
      // check if we found exactly this ID
      if(id == uin->id)
      {
        result = uin;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// FindUserIdentityByDescription
// find an active user identity by its description
struct UserIdentityNode *FindUserIdentityByDescription(const struct MinList *userIdentityList, const char *description)
{
  struct UserIdentityNode *result = NULL;

  ENTER();

  if(description != NULL)
  {
    struct UserIdentityNode *uin;

    IterateList(userIdentityList, struct UserIdentityNode *, uin)
    {
      // check if the identity is active and if the description matches
      if(uin->active == TRUE && strcasestr(description, uin->description) != NULL)
      {
        result = uin;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// FindUserIdentityByAddress
// find an active user identity by a given address
struct UserIdentityNode *FindUserIdentityByAddress(const struct MinList *userIdentityList, const char *address)
{
  struct UserIdentityNode *result = NULL;

  ENTER();

  if(address != NULL)
  {
    struct UserIdentityNode *uin;

    IterateList(userIdentityList, struct UserIdentityNode *, uin)
    {
      // check if the identity is active and if the address matches
      if(uin->active == TRUE && strcasestr(address, uin->address) != NULL)
      {
        result = uin;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// NumberOfUserIdentities
// count the number of active user identities
ULONG NumberOfUserIdentities(const struct MinList *userIdentityList)
{
  ULONG count = 0;
  struct UserIdentityNode *uin;

  ENTER();

  IterateList(userIdentityList, struct UserIdentityNode *, uin)
  {
    // count active identites only
    if(uin->active == TRUE)
      count++;
  }

  RETURN(count);
  return count;
}

///
/// IndexOfUserIdentity
// return the index of a user identity within the list
LONG IndexOfUserIdentity(const struct MinList *userIdentityList, const struct UserIdentityNode *uin)
{
  LONG result = -1;
  LONG idx;
  struct UserIdentityNode *iter;

  ENTER();

  idx = 0;
  IterateList(userIdentityList, struct UserIdentityNode *, iter)
  {
    // count active identites only
    if(iter->active == TRUE)
    {
      if(iter == uin)
      {
        result = idx;
        break;
      }

      idx++;
    }
  }

  RETURN(result);
  return result;
}

///

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/dos.h>
#if defined(__amigaos4__)
#include <dos/obsolete.h>
#endif

#include "extrasrc.h"

#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

#include "FileInfo.h"
#include "MailServers.h"
#include "UIDL.h"

#include "Debug.h"

/// BuildUIDLFilename
// set up a name for a UIDL file to be accessed
static void BuildUIDLFilename(const struct MailServerNode *msn, char *uidlPath, const size_t uidlPathSize)
{
  ENTER();

  if(msn != NULL)
  {
    char *uidlName;

    // create a file name using the mail server's unique ID
    if(asprintf(&uidlName, ".uidl_%08lx", msn->id) != -1)
    {
      CreateFilename(uidlName, uidlPath, uidlPathSize);

      free(uidlName);
    }
  }
  else
  {
    // use the old style .uidl name
    CreateFilename(".uidl", uidlPath, uidlPathSize);
  }

  LEAVE();
}

///
/// InitUIDLhash
// Initialize the UIDL list and load it from the .uidl file
struct UIDLhash *InitUIDLhash(const struct MailServerNode *msn)
{
  struct UIDLhash *uidlHash;

  ENTER();

  if((uidlHash = malloc(sizeof(*uidlHash))) != NULL)
  {
    // allocate a new hashtable for managing the UIDL data
    if((uidlHash->hash = HashTableNew(HashTableGetDefaultStringOps(), NULL, sizeof(struct UIDLtoken), 512)) != NULL)
    {
      char uidlPath[SIZE_PATHFILE];
      LONG size;
      FILE *fh = NULL;
      BOOL oldUIDLFile = FALSE;

      // try to access the account specific .uidl file first
      BuildUIDLFilename(msn, uidlPath, sizeof(uidlPath));
      if(ObtainFileInfo(uidlPath, FI_SIZE, &size) == TRUE && size > 0)
      {
        fh = fopen(uidlPath, "r");
      }

      if(fh == NULL)
      {
        // an account specific UIDL does not seem to exist, try the old .uidl file instead
        BuildUIDLFilename(NULL, uidlPath, sizeof(uidlPath));
        if(ObtainFileInfo(uidlPath, FI_SIZE, &size) == TRUE && size > 0)
        {
          fh = fopen(uidlPath, "r");
          // this file is definitely an old style UIDL file without header
          oldUIDLFile = TRUE;
        }
      }

      if(fh != NULL)
      {
        // now read in the UIDL/MsgIDs line-by-line
        char *uidl = NULL;
        size_t uidlLen = 0;
        BOOL validFile = FALSE;

        D(DBF_UIDL, "opened UIDL database file '%s'", uidlPath);

        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

        if(oldUIDLFile == TRUE)
        {
          // old UIDL files are considered to be always valid
          validFile = TRUE;
        }
        else
        {
          // new UIDL files must contain the usual header
          if(GetLine(&uidl, &uidlLen, fh) >= 0 && strncmp(uidl, "UIDL", 4) == 0)
            validFile = TRUE;
        }

        if(validFile == TRUE)
        {
          // add all read UIDLs to the hash marking them as OLD
          while(GetLine(&uidl, &uidlLen, fh) >= 0)
            AddUIDLtoHash(uidlHash, uidl, UIDLF_OLD);
        }
        else
          W(DBF_UIDL, "file '%s' is no valid UIDL database file", uidlPath);

        fclose(fh);

        free(uidl);
      }
      else
        W(DBF_UIDL, "UIDL database file '%s' does not exist", uidlPath);

      // remember the mail server to be able to regenerate the file name upon cleanup
      uidlHash->mailServer = (struct MailServerNode *)msn;
      // we start with an unmodified hash table
      uidlHash->isDirty = FALSE;

      SHOWVALUE(DBF_UIDL, uidlHash->hash->entryCount);
    }
    else
    {
      E(DBF_UIDL, "couldn't create new Hashtable for UIDL management");
      free(uidlHash);
      uidlHash = NULL;
    }
  }
  else
    E(DBF_UIDL, "couldn't create new Hashtable for UIDL management");

  RETURN(uidlHash);
  return uidlHash;
}

///
/// SaveUIDLtoken
// HashTable callback function to save an UIDLtoken
static enum HashTableOperator SaveUIDLtoken(UNUSED struct HashTable *table,
                                            struct HashEntryHeader *entry,
                                            UNUSED ULONG number,
                                            void *arg)
{
  struct UIDLtoken *token = (struct UIDLtoken *)entry;

  ENTER();

  // Check whether the UIDL is a new one (received from the server), then we keep it.
  // Otherwise (OLD set, but not NEW) we skip it, because the mail belonging to this
  // UIDL does no longer exist on the server and we can forget about it.
  if(isFlagSet(token->flags, UIDLF_NEW))
  {
    FILE *fh = (FILE *)arg;

    fprintf(fh, "%s\n", token->uidl);
    D(DBF_UIDL, "saved UIDL '%s' to .uidl file", token->uidl);
  }
  else
    D(DBF_UIDL, "outdated UIDL '%s' found and deleted", token->uidl);

  RETURN(htoNext);
  return htoNext;
}

///
/// CleanupUIDLhash
// Cleanup the whole UIDL hash
void CleanupUIDLhash(struct UIDLhash *uidlHash)
{
  ENTER();

  if(uidlHash != NULL)
  {
    if(uidlHash->hash != NULL)
    {
      // save the UIDLs only if something has been changed
      if(uidlHash->isDirty == TRUE)
      {
        char uidlPath[SIZE_PATHFILE];
        FILE *fh;

        // we are saving account specific .uidl files only, the old one will be kept
        // in case it still contains UIDLs of multiple accounts
        BuildUIDLFilename(uidlHash->mailServer, uidlPath, sizeof(uidlPath));

        // before we go and destroy the UIDL hash we have to
        // write it to the .uidl file back again.
        if((fh = fopen(uidlPath, "w")) != NULL)
        {
          setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

          fprintf(fh, "UIDL - YAM UIDL database for %s@%s\n", uidlHash->mailServer->username, uidlHash->mailServer->hostname);

          // call HashTableEnumerate with the SaveUIDLtoken callback function
          HashTableEnumerate(uidlHash->hash, SaveUIDLtoken, fh);

          fclose(fh);
        }
        else
          E(DBF_UIDL, "couldn't open '%s' for writing", uidlPath);
      }

      // now we can destroy the uidl hash
      HashTableDestroy(uidlHash->hash);
      uidlHash->hash = NULL;
      D(DBF_UIDL, "destroyed UIDL hash table");
    }

    free(uidlHash);

    D(DBF_UIDL, "cleaned up UIDLhash");
  }

  LEAVE();
}

///
/// AddUIDLtoHash
// adds the UIDL of a mail transfer node to the hash
struct UIDLtoken *AddUIDLtoHash(struct UIDLhash *uidlHash, const char *uidl, const ULONG flags)
{
  struct UIDLtoken *token = NULL;
  struct HashEntryHeader *entry;

  ENTER();

  if((entry = HashTableOperate(uidlHash->hash, uidl, htoLookup)) != NULL && HASH_ENTRY_IS_LIVE(entry))
  {
    token = (struct UIDLtoken *)entry;

    token->flags |= flags;

    D(DBF_UIDL, "updated flags for UIDL '%s' (%08lx)", uidl, token);
    uidlHash->isDirty = TRUE;
  }
  else if((entry = HashTableOperate(uidlHash->hash, uidl, htoAdd)) != NULL)
  {
    token = (struct UIDLtoken *)entry;

    token->uidl = strdup(uidl);
    token->flags = flags;

    D(DBF_UIDL, "added UIDL '%s' (%08lx) to hash", uidl, token);
    uidlHash->isDirty = TRUE;
  }
  else
    E(DBF_UIDL, "couldn't add UIDL '%s' to hash", uidl);

  RETURN(token);
  return token;
}

///
/// FindUIDL
// try to find a UIDL in the hash
struct UIDLtoken *FindUIDL(const struct UIDLhash *uidlHash, const char *uidl)
{
  struct UIDLtoken *token = NULL;
  struct HashEntryHeader *entry;

  ENTER();

  if((entry = HashTableOperate(uidlHash->hash, uidl, htoLookup)) != NULL && HASH_ENTRY_IS_LIVE(entry))
  {
    token = (struct UIDLtoken *)entry;
  }

  RETURN(token);
  return token;
}

///
/// DeleteUIDLfile
// delete a UIDL file in case it it no longer needed, i.e. the account is deleted
void DeleteUIDLfile(const struct MailServerNode *msn)
{
  char uidlPath[SIZE_PATHFILE];

  ENTER();

  BuildUIDLFilename(msn, uidlPath, sizeof(uidlPath));

  if(DeleteFile(uidlPath) == 0)
    AddZombieFile(uidlPath);

  LEAVE();
}

///

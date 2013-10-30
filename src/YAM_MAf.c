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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include <clib/alib_protos.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"
#include "timeval.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/InfoBar.h"
#include "mui/QuickSearchBar.h"
#include "mui/ReadMailGroup.h"
#include "mime/base64.h"
#include "mime/rfc2047.h"

#include "AppIcon.h"
#include "Busy.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Rexx.h"
#include "Signature.h"
#include "UserIdentity.h"

#include "Debug.h"

/*
** The following structures are used to build the
** .index files of a folder.
**
** Alignment is very important here, because the
** structures are saved RAW to a file.
** Therefore we have to ensure that the alignment
** will be the same for 68k and PPC.
** That's why we deal with the following #pragma
** calls
*/

#include "amiga-align.h"

/*
** structure of the compressed mail
**
** DO NOT CHANGE ALIGNMENT here or the .index
** files of a folder will be corrupt !
**
*/
struct ComprMail
{
  char             mailFile[SIZE_MFILE]; // mail filename without path
  struct DateStamp date;                 // the creation date of the mail (UTC)
  struct TimeVal   transDate;            // the received/sent date with ms (UTC)
  unsigned int     sflags;               // mail status flags
  unsigned int     mflags;               // general mail flags
  unsigned long    cMsgID;               // compressed MessageID
  unsigned long    cIRTMsgID;            // compressed InReturnTo MessageID
  long             size;                 // the total size of the message
  unsigned int     moreBytes;            // more bytes to follow as the subject
  // ... more data of size 'moreBytes' follows here
};

/*
** structure of the Folder Index
**
** DO NOT CHANGE ALIGNMENT here or the .index
** files of a folder will be corrupt !
**
*/
struct FIndex
{
  ULONG ID;
  int   Total;
  int   New;
  int   Unread;
  int   Size;
  long  reserved[2];
};

// whenever you change something up there (in FIndex or ComprMail) you
// need to increase this version ID!
#define FINDEX_VER  (MAKE_ID('Y','I','N','8'))

#include "default-align.h"

/* local protos */
static BOOL MA_ScanMailBox(struct Folder *folder);

/***************************************************************************
 Module: Main - Folder handling
***************************************************************************/

/*** Index Maintenance ***/
/// MA_PromptFolderPassword
//  Asks user for folder password
BOOL MA_PromptFolderPassword(struct Folder *fo, APTR win)
{
  BOOL success = FALSE;

  ENTER();

  if(isFreeAccess(fo))
  {
    D(DBF_FOLDER, "folder '%s' has no access restrictions", fo->Name);
    success = TRUE;
  }
  else
  {
    struct User *user;

    user = US_GetCurrentUser();

    if(Stricmp(fo->Password, user->Password) == 0)
    {
      D(DBF_FOLDER, "folder '%s' and user '%s' share the same password", fo->Name, user->Name);
      success = TRUE;
    }
    else
    {
      BOOL retry = TRUE;
      char prompt[SIZE_LARGE];

      snprintf(prompt, sizeof(prompt), tr(MSG_MA_GetFolderPass), fo->Name);

      do
      {
        char passwd[SIZE_PASSWORD];

        passwd[0] = '\0';
        if(StringRequest(passwd, SIZE_PASSWORD, tr(MSG_Folder), prompt, tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, win) > 0)
          // try again if the password doesn't match
          success = (Stricmp(passwd, fo->Password) == 0);
        else
          // the user cancelled the requester, no more tries
          retry = FALSE;
      }
      while(success == FALSE && retry);
    }
  }

  RETURN(success);
  return success;
}

///
/// MA_ValidateStatus
//  Avoids invalid status values
static void MA_ValidateStatus(struct Folder *folder)
{
  ENTER();

  // the whole job only makes sense if there are new mails at all
  if(folder->New > 0)
  {
    BOOL sent = isSentFolder(folder);

    // mark new mails as unread during the startup phase only, but not upon reloading a folder index
    if(sent == TRUE || (C->UpdateNewMail == TRUE && G->InStartupPhase == TRUE))
    {
      struct MailNode *mnode;

      D(DBF_FOLDER, "validating status of new messages in folder '%s'", folder->Name);

      LockMailListShared(folder->messages);

      ForEachMailNode(folder->messages, mnode)
      {
        struct Mail *mail = mnode->mail;

        if(hasStatusNew(mail))
        {
          if(sent == TRUE)
          {
            D(DBF_FOLDER, "set status 'sent' of mail with subject '%s'", mail->Subject);
            setStatusToSent(mail);
          }
          else
          {
            D(DBF_FOLDER, "set status 'unread' of mail with subject '%s'", mail->Subject);
            setStatusToUnread(mail);
          }
        }
      }

      UnlockMailList(folder->messages);
    }
  }

  LEAVE();
}

///
/// MA_LoadIndex
//  Loads a folder index from disk
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full)
{
  char indexFileName[SIZE_PATHFILE];
  ULONG indexFileSize;
  enum LoadedMode indexloaded = LM_UNLOAD;
  BOOL corrupt = FALSE;
  BOOL error = FALSE;

  ENTER();

  D(DBF_FOLDER, "Loading index for folder '%s'", folder->Name);

  AddPath(indexFileName, folder->Fullpath, ".index", sizeof(indexFileName));

  // Check the size of the index file. Even an empty folder has an index
  // file with a size of at least sizeof(struct FIndex).
  if(ObtainFileInfo(indexFileName, FI_SIZE, &indexFileSize) == TRUE && indexFileSize >= sizeof(struct FIndex))
  {
    FILE *fh;

    if((fh = fopen(indexFileName, "r")) != NULL)
    {
      struct BusyNode *busy;
      struct FIndex fi;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      busy = BusyBegin(BUSY_TEXT);
      BusyText(busy, tr(MSG_BusyLoadingIndex), folder->Name);
      if(fread(&fi, sizeof(fi), 1, fh) != 1)
      {
        E(DBF_FOLDER, "error while loading struct FIndex from .index file");
        error = TRUE;
      }
      else if(fi.ID == FINDEX_VER)
      {
        folder->Total  = fi.Total;
        folder->New    = fi.New;
        folder->Unread = fi.Unread;
        folder->Size   = fi.Size;
        indexloaded = LM_FLUSHED;

        if(full == TRUE)
        {
          struct Folder *tempFolder;

          ClearFolderMails(folder, TRUE);

          // allocate a temporary folder structure to avoid having to lock the real folder's
          // mail list for each single mail we get from the index
          if((tempFolder = AllocFolder()) != NULL)
          {
            do
            {
              struct Mail *mail;
              struct ComprMail cmail;
              char utf8buf[SIZE_LARGE];
              char *buf;
              char *line;
              char *nextLine;
              int lineNr;

              if(fread(&cmail, sizeof(struct ComprMail), 1, fh) != 1)
              {
                // check if we are here because of an error or EOF
                if(ferror(fh) != 0 || feof(fh) == 0)
                {
                  E(DBF_FOLDER, "error while loading ComprMail struct from .index file");
                  error = TRUE;
                }

                // if we end up here it is just a EOF and no error.
                break;
              }

              if(cmail.moreBytes > sizeof(utf8buf)-1)
              {
                ER_NewError(tr(MSG_ER_INDEX_CORRUPTED), indexFileName, folder->Name, ftell(fh), cmail.mailFile, cmail.moreBytes);
                corrupt = TRUE;
                break;
              }

              // read the moreBytes data
              if(fread(utf8buf, cmail.moreBytes, 1, fh) != 1)
              {
                E(DBF_FOLDER, "fread error while reading index file");
                error = TRUE;
                break;
              }

              // make sure to NUL terminate the utf8 string
              utf8buf[cmail.moreBytes] = '\0';

              // convert the utf8 encoded buffer to the local charset
              if((buf = CodesetsUTF8ToStr(CSA_Source,          utf8buf,
                                          CSA_SourceLen,       cmail.moreBytes,
                                          CSA_DestCodeset,     G->systemCodeset,
                                          CSA_MapForeignChars, C->MapForeignChars,
                                          TAG_DONE)) == NULL)
              {
                E(DBF_FOLDER, "error while converting UTF8 data to local charset");
                error = TRUE;
                break;
              }

              // create a new mail structure
              if((mail = AllocMail()) != NULL)
              {
                line = buf;
                lineNr = 0;
                do
                {
                  if((nextLine = strchr(line, '\n')) != NULL)
                    *nextLine++ = '\0';

                  lineNr++;

                  switch(lineNr)
                  {
                    case 1:
                      strlcpy(mail->Subject, line, sizeof(mail->Subject));
                    break;

                    case 2:
                      strlcpy(mail->From.Address, line, sizeof(mail->From.Address));
                    break;

                    case 3:
                      strlcpy(mail->From.RealName, line, sizeof(mail->From.RealName));
                    break;

                    case 4:
                      strlcpy(mail->To.Address, line, sizeof(mail->To.Address));
                    break;

                    case 5:
                      strlcpy(mail->To.RealName, line, sizeof(mail->To.RealName));
                    break;

                    case 6:
                      strlcpy(mail->ReplyTo.Address, line, sizeof(mail->ReplyTo.Address));
                    break;

                    case 7:
                      strlcpy(mail->ReplyTo.RealName, line, sizeof(mail->ReplyTo.RealName));
                    break;
                  }

                  line = nextLine;
                }
                while(line != NULL && lineNr < 7);

                mail->mflags = cmail.mflags;
                mail->sflags = cmail.sflags;
                // we have to make sure that the volatile flag field isn't loaded
                setVOLValue(mail, 0);
                strlcpy(mail->MailFile, cmail.mailFile, sizeof(mail->MailFile));
                mail->Date = cmail.date;
                mail->transDate = cmail.transDate;
                mail->cMsgID = cmail.cMsgID;
                mail->cIRTMsgID = cmail.cIRTMsgID;
                mail->Size = cmail.size;

                // finally add the new mail structure to the temporary folder
                // no message list locking or index expiring is necessary here,
                // because it is a temporary folder which is not publically known
                AddMailToFolderSimple(mail, tempFolder);

                // the AddMailToFolderSimple() call set the mail's folder pointer to the
                // temporary folder. But since this is a temporary one only and will be
                // invalid after leaving this function we must set the mail's folder
                // pointer to the correct current folder.
                mail->Folder = folder;
              }
              else
                error = TRUE;

              // free the codesets buffer
              CodesetsFreeA(buf, NULL);
            }
            while(error == FALSE);
          }

          // if everything went well then move all mails from the temporary folder
          // to the real folder
          if(error == FALSE)
            MoveFolderContents(folder, tempFolder);

          // free the temporary folder in any case
          FreeFolder(tempFolder);
        }
      }

      if(ferror(fh) != 0)
      {
        E(DBF_FOLDER, "ferror() returned != 0");
        error = TRUE;
      }

      BusyEnd(busy);
      fclose(fh);
    }
    else if(errno != ENOENT)
    {
      E(DBF_FOLDER, "fopen() on '%s' failed.", indexFileName);
      error = TRUE;
    }
  }
  else
  {
    E(DBF_FOLDER, "index file '%s' of folder '%s' has invalid size %ld (minimum required %ld)", indexFileName, folder->Name, indexFileSize, sizeof(struct FIndex));
    // report this index file as corrupted if it is smaller than the required minimum size, but not empty
    if(indexFileSize > 0)
      corrupt = TRUE;
  }

  // in case an error occurred we report it
  // to the user
  if(error == TRUE)
  {
    E(DBF_FOLDER, "error %ld occurred while trying to load the index file '%s'", errno, indexFileName);
    ClearFolderMails(folder, TRUE);
    indexloaded = LM_UNLOAD;

    // report failure
    ER_NewError(tr(MSG_ER_CANNOT_READ_INDEX), indexFileName, folder->Name);
  }
  else if(corrupt == TRUE || indexloaded == LM_UNLOAD)
  {
    W(DBF_FOLDER, "%s .index file '%s' detected, %s", corrupt ? "corrupt" : "missing",
                                                      indexFileName,
                                                      full ? "rebuilding..." : "skipping...");

    // clear the mail list of the folder
    ClearFolderMails(folder, TRUE);

    // if the "full" mode was requested we make sure we
    // rescan the index accordingly
    if(full == TRUE)
    {
      // rebuild the index (rescanning the mailbox directory)
      if(MA_ScanMailBox(folder) == TRUE && MA_SaveIndex(folder) == TRUE)
        indexloaded = LM_VALID;
    }
  }
  else if(full == TRUE)
  {
    indexloaded = LM_VALID;
    clearFlag(folder->Flags, FOFL_MODIFY);
  }

  RETURN(indexloaded);
  return indexloaded;
}

///
/// MA_SaveIndex
//  Saves a folder index to disk
BOOL MA_SaveIndex(struct Folder *folder)
{
  BOOL success = FALSE;
  char indexFileName[SIZE_PATHFILE];
  FILE *fh;

  ENTER();

  D(DBF_FOLDER, "save index of folder '%s'", folder->Name);

  AddPath(indexFileName, folder->Fullpath, ".index", sizeof(indexFileName));

  if((fh = fopen(indexFileName, "w")) != NULL)
  {
    struct BusyNode *busy;
    struct FIndex fi;
    struct MailNode *mnode;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    busy = BusyBegin(BUSY_TEXT);
    BusyText(busy, tr(MSG_BusySavingIndex), folder->Name);

    // lets prepare the Folder Index struct and write it out
    // we clear it first, so that the reserved field is also 0
    memset(&fi, 0, sizeof(struct FIndex));
    fi.ID = FINDEX_VER;
    fi.Total = folder->Total;
    fi.New = folder->New;
    fi.Unread = folder->Unread;
    fi.Size = folder->Size;

    // write the index header out first
    if(fwrite(&fi, sizeof(fi), 1, fh) == 1)
    {
      // assume success at first
      success = TRUE;

      LockMailListShared(folder->messages);
      ForEachMailNode(folder->messages, mnode)
      {
        struct Mail *mail = mnode->mail;
        struct ComprMail cmail;
        char buf[SIZE_LARGE];
        UTF8 *utf8buf;

        // create the moreBytes string we append at the end
        snprintf(buf, sizeof(buf), "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
                                   mail->Subject,
                                   mail->From.Address, mail->From.RealName,
                                   mail->To.Address, mail->To.RealName,
                                   mail->ReplyTo.Address, mail->ReplyTo.RealName);

        // convert the buffer string to UTF8
        // the length of the generated string is directly put into the moreBytes variable
        if((utf8buf = CodesetsUTF8Create(CSA_Source, buf,
                                         CSA_SourceCodeset, G->systemCodeset,
                                         CSA_DestLenPtr, &cmail.moreBytes,
                                         TAG_DONE)) != NULL)
        {
          strlcpy(cmail.mailFile, mail->MailFile, sizeof(cmail.mailFile));
          cmail.date = mail->Date;
          cmail.transDate = mail->transDate;
          cmail.sflags = mail->sflags;
          cmail.mflags = mail->mflags;
          // we have to make sure that the volatile flag field isn't saved
          setVOLValue(&cmail, 0);
          cmail.cMsgID = mail->cMsgID;
          cmail.cIRTMsgID = mail->cIRTMsgID;
          cmail.size = mail->Size;

          if(fwrite(&cmail, sizeof(cmail), 1, fh) != 1 ||
             fwrite(utf8buf, cmail.moreBytes, 1, fh) != 1)
          {
            E(DBF_FOLDER, "couldn't write index data of mail '%s'", cmail.mailFile);
            success = FALSE;
          }

          // free the codesets buffer
          CodesetsFreeA(utf8buf, NULL);
        }
        else
          success = FALSE;

        // break out if something went wrong
        if(success == FALSE)
          break;
      }

      UnlockMailList(folder->messages);

      clearFlag(folder->Flags, FOFL_MODIFY);
    }

    fclose(fh);
    BusyEnd(busy);
  }
  else
  {
    W(DBF_FOLDER, "saving index file '%s' of folder '%s' failed", indexFileName, folder->Name);
    ER_NewError(tr(MSG_ER_CANNOT_WRITE_INDEX), indexFileName, folder->Name);
  }

  RETURN(success);
  return success;
}

///
/// MA_GetIndex
//  Opens/unlocks a folder
BOOL MA_GetIndex(struct Folder *folder)
{
  BOOL result = FALSE;

  ENTER();

  if(folder != NULL && isGroupFolder(folder) == FALSE)
  {
    D(DBF_FOLDER, "folder: '%s' path: '%s' type: %ld mode: %ld pw '%s'", folder->Name, folder->Fullpath, folder->Type, folder->LoadedMode, folder->Password);

    // check that the folder is in a valid state for
    // getting the index
    if(folder->LoadedMode != LM_VALID && folder->LoadedMode != LM_REBUILD)
    {
      BOOL canLoadIndex;

      // check the protected status of the folder and prompt
      // the user for the password in case it is required.
      if(isProtectedFolder(folder) == FALSE || folder->Password[0] == '\0')
        canLoadIndex = TRUE;
      else
        canLoadIndex = MA_PromptFolderPassword(folder, G->MA->GUI.WI);

      if(canLoadIndex == TRUE)
      {
        // load the index file (and eventually rebuild it)
        folder->LoadedMode = MA_LoadIndex(folder, TRUE);

        // we need to refresh the entry in the folder listtree this folder belongs to
        // but only if there is a GUI already!
        if(folder->LoadedMode == LM_VALID)
        {
          MA_ValidateStatus(folder);
        }
        else
          W(DBF_MAIL, "status of loaded folder '%s' != LM_VALID (%ld)", folder->Name, folder->LoadedMode);

        if(G->MA != NULL)
          DisplayStatistics(folder, FALSE);
      }
      else
        W(DBF_FOLDER, "password of protected folder '%s' couldn't be verified!", folder->Name);
    }
    else
      W(DBF_FOLDER, "skipping index loading due to LoadedMode %ld for folder '%s'", folder->LoadedMode, folder->Name);

    // set the lastAccessTime of the folder to the current time
    // so that the index expunge timer knows when to free the folder
    // index in case it hasn't been touched for a certain time
    folder->lastAccessTime = GetDateStamp();

    // check if the load status is valid or not
    result = (BOOL)(folder->LoadedMode == LM_VALID);
  }

  RETURN(result);
  return result;
}

///
/// MA_ExpireIndex
//  Invalidates a folder index
void MA_ExpireIndex(struct Folder *folder)
{
  ENTER();

  if(!isModified(folder))
  {
    char indexFileName[SIZE_PATHFILE];

    AddPath(indexFileName, folder->Fullpath, ".index", sizeof(indexFileName));
    DeleteFile(indexFileName);
  }

  setFlag(folder->Flags, FOFL_MODIFY);

  LEAVE();
}

///
/// MA_RebuildIndexes
//  Rebuild indices of all folders
void MA_RebuildIndexes(void)
{
  struct FolderNode *fnode;

  ENTER();

  LockFolderListShared(G->folders);

  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    if(folder != NULL && !isGroupFolder(folder))
    {
      char indexFileName[SIZE_PATHFILE];
      ULONG dirDate;
      ULONG indexDate;

      AddPath(indexFileName, folder->Fullpath, ".index", sizeof(indexFileName));

      // get date of the folder directory and the .index file
      // itself
      if(ObtainFileInfo(folder->Fullpath, FI_TIME, &dirDate) == TRUE &&
         ObtainFileInfo(indexFileName, FI_TIME, &indexDate) == TRUE)
      {
        // only consider starting to rebuilding the .index if
        // either the date of the directory is greater than the
        // date of the .index file itself, or if there is no index
        // file date at all (no file present)
        if(dirDate > indexDate + 30)
        {
          ULONG dirProtection;
          ULONG indexProtection;

          // get the protection bits of the folder index file
          // and the folder directory, and if both have the A
          // bit set we skip the index rescanning process because
          // the A bits might have been set by a backup program
          if(ObtainFileInfo(folder->Fullpath, FI_PROTECTION, &dirProtection) == TRUE &&
             ObtainFileInfo(indexFileName, FI_PROTECTION, &indexProtection) == TRUE)
          {
            if(isFlagClear(indexProtection, FIBF_ARCHIVE) ||
               isFlagClear(dirProtection, FIBF_ARCHIVE))
            {
              // lets first delete the .index file to
              // make sure MA_GetIndex() is going to
              // rebuild it.
              if(indexDate > 0)
                DeleteFile(indexFileName);

              // then lets call GetIndex() to start rebuilding
              // the .index - but only if this folder is one of the folders
              // that should update it indexes during startup
              if((isIncomingFolder(folder) || isOutgoingFolder(folder) ||
                  isTrashFolder(folder) || C->LoadAllFolders) &&
                 !isProtectedFolder(folder))
              {
                if(MA_GetIndex(folder) == TRUE)
                {
                  // if we finally rebuilt the .index we
                  // immediatly flush it here so that another
                  // following index rebuild doesn't take
                  // all remaining memory.
                  if((isSentFolder(folder) || !isDefaultFolder(folder)) &&
                      folder->LoadedMode == LM_VALID &&
                      isFreeAccess(folder))
                  {
                    if(isModified(folder))
                      MA_SaveIndex(folder);

                    ClearFolderMails(folder, FALSE);
                    folder->LoadedMode = LM_FLUSHED;
                    clearFlag(folder->Flags, FOFL_FREEXS);
                  }
                }
              }
              else
              {
                // otherwise we make sure everything is cleared
                ClearFolderMails(folder, FALSE);
                folder->LoadedMode = LM_FLUSHED;
                clearFlag(folder->Flags, FOFL_FREEXS);
              }
            }
          }
        }
      }
    }
  }

  UnlockFolderList(G->folders);

  LEAVE();
}

///

/*** Private functions ***/
/// MA_ChangeFolder
//  Changes to another folder
void MA_ChangeFolder(struct Folder *folder, BOOL set_active)
{
  struct Folder *current;

  ENTER();

  if((current = GetCurrentFolder()) != NULL)
  {
    BOOL folderChanged = TRUE;

    if(folder == NULL)
      folder = current;
    else if(folder == current)
      folderChanged = FALSE;
    else if(set_active == TRUE)
    {
      ActivateFolder(folder);
      current = folder;
    }

    if(folderChanged == TRUE)
    {
      struct MA_GUIData *gui = &G->MA->GUI;

      // in case the main window has an embedded read pane, we have to
      // clear it before changing the actual folder
      if(C->EmbeddedReadPane == TRUE)
        DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_Clear, MUIF_NONE);

      // if this folder should be disabled, lets do it now
      if(isGroupFolder(folder) || MA_GetIndex(folder) == FALSE)
      {
        xset(gui->PG_MAILLIST, MUIA_Disabled,     TRUE,
                               MUIA_NList_Active, MUIV_NList_Active_Off);

        // set the quickbar as disabled as well and abort a search still in progress
        if(C->QuickSearchBarPos != QSB_POS_OFF)
        {
          xset(gui->GR_QUICKSEARCHBAR, MUIA_Disabled, TRUE,
                                       MUIA_QuickSearchBar_AbortSearch, TRUE);
        }

        // also set an embedded read pane as disabled.
        if(C->EmbeddedReadPane == TRUE)
          set(gui->MN_EMBEDDEDREADPANE, MUIA_Disabled, TRUE);

        DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);

        // make sure the main mail list noticies that
        // the selection has changed so that if a user reactivates a valid
        // folder the main mail list will get updated accordingly.
        MA_ChangeSelected(TRUE);
      }
      else if(folder == current) // check again for the current folder
      {
        BOOL jumped;

        // set the SortFlag in the NList accordingly
        MA_SetSortFlag();

        // Now we update the InfoBar accordingly
        DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);

        // enable an embedded read pane again
        if(C->EmbeddedReadPane == TRUE)
          set(gui->MN_EMBEDDEDREADPANE, MUIA_Disabled, FALSE);

        // In case the main window has an quicksearchbar, we have to
        // clear it as well before changing the folder. We also abort
        // a search still being in progress.
        if(C->QuickSearchBarPos != QSB_POS_OFF)
        {
          set(gui->GR_QUICKSEARCHBAR, MUIA_QuickSearchBar_AbortSearch, TRUE);
          DoMethod(gui->GR_QUICKSEARCHBAR, MUIM_QuickSearchBar_Clear);
        }

        // Create the Mail List and display it
        DisplayMailList(folder, gui->PG_MAILLIST);

        // now we have to assure that the folder is enabled
        set(gui->PG_MAILLIST, MUIA_Disabled, FALSE);

        // Now we jump to messages that are NEW
        jumped = FALSE;
        if(jumped == FALSE && folder->JumpToUnread == TRUE && (folder->New != 0 || folder->Unread != 0))
          jumped = MA_JumpToNewMsg();

        if(jumped == FALSE && folder->LastActive >= 0)
        {
          DoMethod(gui->PG_MAILLIST, MUIM_NList_SetActive, folder->LastActive, MUIV_NList_SetActive_Jump_Center);
          jumped = TRUE;
        }

        if(jumped == FALSE && folder->JumpToRecent == TRUE)
          jumped = MA_JumpToRecentMsg();

        // if there is still no entry active in the NList we make the first one active
        if(jumped == FALSE)
          set(gui->PG_MAILLIST, MUIA_NList_Active, MUIV_NList_Active_Top);

        // if there are no messages in the folder the GUI needs to be updated nevertheless
        if(folder->Total == 0)
          MA_ChangeSelected(TRUE);
      }
    }
  }

  LEAVE();
}

///
/// MA_JumpToNewMsg
// Function that jumps to the first or last unread mail in a folder,
// depending on sort order of the folder
BOOL MA_JumpToNewMsg(void)
{
  Object *lv;
  struct Folder *folder;
  int i, incr, newIdx = -1;
  BOOL jumped = FALSE;

  ENTER();

  lv = G->MA->GUI.PG_MAILLIST;
  folder = GetCurrentFolder();

  if(folder->Sort[0] < 0 || folder->Sort[1] < 0)
  {
    i = xget(lv, MUIA_NList_Entries) - 1;
    incr = -1;
  }
  else
  {
    i = 0;
    incr = 1;
  }

  while(TRUE)
  {
    struct Mail *mail;

    DoMethod(lv, MUIM_NList_GetEntry, i, &mail);
    if(mail == NULL)
      break;

    if(hasStatusNew(mail) || !hasStatusRead(mail))
    {
      newIdx = i;
      break;
    }

    i += incr;
  }

  if(newIdx >= 0 && newIdx != folder->LastActive)
  {
    set(lv, MUIA_NList_Active, newIdx);
    jumped = TRUE;
  }

  RETURN(jumped);
  return jumped;
}

///
/// MA_JumpToRecentMsg
// Function that jumps to the most recent mail in a folder
BOOL MA_JumpToRecentMsg(void)
{
  struct MailNode *recent = NULL;
  struct MailNode *mnode;
  struct Folder *folder;
  BOOL jumped = FALSE;

  ENTER();

  folder = GetCurrentFolder();

  LockMailList(folder->messages);

  mnode = FirstMailNode(folder->messages);
  while(mnode != NULL)
  {
    if(recent == NULL || CompareMailsByDate(mnode, recent) > 0)
    {
      // this mail is more recent than the yet most recent known
      recent = mnode;
    }

    mnode = NextMailNode(mnode);
  }

  if(recent != NULL)
  {
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_SetActive, recent->mail, MUIV_NList_SetActive_Entry|MUIV_NList_SetActive_Jump_Center);
    jumped = TRUE;
  }

  UnlockMailList(folder->messages);

  RETURN(jumped);
  return jumped;
}

///
/// MA_ConvertOldMailFile
// This function takes a fileinfoblock and a folder and converts the
// oldstyle mailfile referred by fib to a new YAM 2.5+ conform mail file.
static char *MA_ConvertOldMailFile(char *filename, struct Folder *folder)
{
  char dateFilePart[13];  // the base64 encoded date part occupies 12+1 bytes
  char statusFilePart[4]; // we only need 3+1 bytes for the status flags here
  char oldFilePath[SIZE_PATHFILE+1];
  char *statusPartPtr = statusFilePart;
  char *comment;
  char *ptr;
  char *result = filename;

  ENTER();

  // clear
  dateFilePart[0] = '\0';
  statusFilePart[0] = '\0';

  // construct the full path of the old filename
  // and get the file comment
  AddPath(oldFilePath, folder->Fullpath, filename, sizeof(oldFilePath));
  if(ObtainFileInfo(oldFilePath, FI_COMMENT, &comment) == FALSE || comment == NULL)
  {
    RETURN(NULL);
    return NULL;
  }

  // for a proper conversion we have to take mainly the file comment into
  // account as in there all status information aswell as the transfer
  // data was stored in YAM versions prior to 2.5
  if(comment[0] != '\0')
  {
    // read out the mailstatus - which was normally the first character
    // of the comment
    switch(comment[0])
    {
      case 'U':
        // nothing
      break;

      case 'R':
        *statusPartPtr++ = SCHAR_READ;
        *statusPartPtr++ = SCHAR_REPLIED;
      break;

      case 'F':
        *statusPartPtr++ = SCHAR_READ;
        *statusPartPtr++ = SCHAR_FORWARDED;
      break;

      case 'W':
        *statusPartPtr++ = SCHAR_READ;
      break;

      case 'H':
        *statusPartPtr++ = SCHAR_READ;
      break;

      case 'O':
        *statusPartPtr++ = SCHAR_READ;
      break;

      case 'E':
        *statusPartPtr++ = SCHAR_ERROR;
      break;

      case 'S':
        *statusPartPtr++ = SCHAR_READ;
        *statusPartPtr++ = SCHAR_SENT;
      break;

      case 'N':
      default:
        *statusPartPtr++ = SCHAR_NEW;
      break;
    }

    // now we check if second char is present and if we set the mailfag
    // as "marked" and the arexx permanent flag
    if(comment[1] != '\0' && comment[1] != ' ')
    {
      int pval = 0;

      // we have to check for the permanent flag also.
      if(comment[1] >= 'M' && comment[1] <= 'M'+7)
      {
        *statusPartPtr++ = SCHAR_MARKED;

        pval = comment[1]-'M';
      }
      else if(comment[1] >= '1' && comment[1] <= '7')
      {
        pval = comment[1]-'1'+1;
      }

      if(pval > 0)
        *statusPartPtr++ = '0'+pval;
    }

    *statusPartPtr = '\0'; // NUL terminate the status part

    // we check if this comment also has the transfer Date included
    // we only take the string if it is exactly 12bytes long or
    // otherwise it could be some weird data in the Comment string
    if(comment[1] != '\0' && comment[2] != '\0' && comment[14] == '\0')
    {
      // ok we have the transfer Date, so lets put it at the beginning of
      // the new filename
      strlcpy(dateFilePart, &comment[2], sizeof(dateFilePart));
    }
  }

  // free the duplicated comment again
  free(comment);

  if(dateFilePart[0] == '\0')
  {
    struct TimeVal newDate;
    char *b64_dateFilePart = NULL;

    // so we don't seem to have a transfer Date in the file comment.
    // What we do now is that we take the date from the original file
    // (the first 5 numerical numbers are the days)
    newDate.Seconds = atol(filename) * 24 * 60 * 60;
    newDate.Microseconds = 0;

    // encode this date as a base64 encoded string
    if(base64encode(&b64_dateFilePart, (char *)&newDate, sizeof(newDate)) > 0)
    {
      if(strlcpy(dateFilePart, b64_dateFilePart, sizeof(dateFilePart)) > sizeof(dateFilePart))
        W(DBF_ALWAYS, "length of b64_dateFilePart > dateFilePart");

      free(b64_dateFilePart);
    }
    else
      E(DBF_ALWAYS, "Error when trying to base64 encode dateFilePart");
  }

  // as the dateFilePart may contain slashes "/" we have to replace them
  // with "-" chars to don't drive the filesystem crazy :)
  ptr = dateFilePart;
  while((ptr = strchr(ptr, '/')) != NULL)
    *ptr = '-';

  do
  {
    static char newFileName[SIZE_MFILE];
    char newFilePath[SIZE_PATHFILE+1];

    // ok, now we should have all main parts of the new filename, so we
    // can concatenate it to one new filename and try to rename the old
    // style mailfile to the newstyle equivalent.
    snprintf(newFileName, sizeof(newFileName), "%s.001,%s", dateFilePart, statusFilePart);

    // so, now we should be finished with finding the new filename of the mail file.
    // lets try to rename it with the dos.library's Rename() function
    if(AddPath(newFilePath, folder->Fullpath, newFileName, sizeof(newFilePath)) == NULL)
    {
      result = NULL;
      break;
    }

    // try to rename it and if it fails go and find out if we have to increase
    // the mail counter or not.
    if(Rename(oldFilePath, newFilePath) == 0)
    {
      char pattern[15+1];
      LONG mailCounter;

      // search for files matching the dateFilePart
      snprintf(pattern, sizeof(pattern), "%s.#?", dateFilePart);
      mailCounter = FileCount(folder->Fullpath, pattern);

      // if we didn't find any matching file then this signals
      // another error than an already existing file!
      if(mailCounter <= 0)
      {
        E(DBF_FOLDER, "  error on renaming '%s' to '%s'", oldFilePath, newFilePath);
        result = NULL;
        break;
      }

      // let us now try it again to rename the file
      snprintf(newFileName, sizeof(newFileName), "%s.%03d,%s", dateFilePart, (unsigned int)(++mailCounter), statusFilePart);
      if(AddPath(newFilePath, folder->Fullpath, newFileName, sizeof(newFilePath)) == NULL)
      {
        result = NULL;
        break;
      }

      // try to rename it and if it fails finally return an error
      if(Rename(oldFilePath, newFilePath) == 0)
      {
        E(DBF_FOLDER, "  error on renaming '%s' to '%s'", oldFilePath, newFilePath);
        result = NULL;
        break;
      }
    }

    result = newFileName;

    // and to make everything as clean as possible, lets erase
    // the filecomment as YAM 2.5+ doesn't require file comments anymore
    SetComment(newFilePath, "");
  }
  while(FALSE);

  // return failure if we didn't change anything
  if(result == filename)
    result = NULL;

  RETURN(result);
  return result;
}
///
/// CompressMsgID
//  Creates a crc32 checksum of the MsgID, so that it can be used later
//  for the follow-up algorithms aso.
static unsigned long CompressMsgID(const char *msgid)
{
  unsigned long id = 0;

  ENTER();

  // if the MsgID is valid we calculate the CRC32 checksum and as it
  // consists only of one cycle through the crc function we call it
  // with -1
  if(IsStrEmpty(msgid) == FALSE)
  {
    char *end;
    size_t len;

    // skip the leading angle bracket (see RFC 4130)
    if(msgid[0] == '<')
      msgid++;

    // find the trailing angle bracket and skip it (see RFC 4130)
    if((end = strchr(msgid, '>')) != NULL)
      len = end-msgid+1;
    else
    {
      W(DBF_MAIL, "can't find closing '>' in msgid '%s'", msgid);
      len = strlen(msgid);
    }

    // calculate the CRC32 checksum for the compressed
    // message ID
    id = CRC32(msgid, len, -1L);
  }
  else
    W(DBF_MAIL, "couldn't calculate compressed MsgID from empty string");

  RETURN(id);
  return id;
}

///
/// FindMailByMsgID
// find a mail by message-id in the given folder
struct Mail *FindMailByMsgID(struct Folder *folder, const char *msgid)
{
  struct Mail *result = NULL;
  struct MailNode *mnode;
  unsigned long msgidCRC;

  ENTER();

  msgidCRC = CompressMsgID(msgid);

  LockMailList(folder->messages);

  ForEachMailNode(folder->messages, mnode)
  {
    struct Mail *mail = mnode->mail;

    // compare the compressed message-ids only for speed reasons
    if(mail->cMsgID == msgidCRC)
    {
      // now go into detail and check if the full message-id matches
      struct ExtendedMail *email;

      if((email = MA_ExamineMail(folder, mail->MailFile, TRUE)) != NULL)
      {
        if(strcmp(email->messageID, msgid) == 0)
        {
          // return the mail
          result = mail;
        }

        MA_FreeEMailStruct(email);
      }

      if(result != NULL)
        break;
    }
  }

  UnlockMailList(folder->messages);

  RETURN(result);
  return result;
}

///

/*** Mail header scanning ***/
/// MA_NewMailFile
//  Function that creates a new plain mail filename or by taking provided
//  data into account. It returns the full path to the new mail file in the
//  folder and also writes it into the mailfile parameter
BOOL MA_NewMailFile(const struct Folder *folder, char *fullPath, const size_t fullPathSize)
{
  char *dateFilePart = NULL;
  char newFileName[SIZE_MFILE];
  char *ptr;
  struct TimeVal curDate;
  int mCounter;
  BOOL result;

  ENTER();

  // take the current time and use it as the datePart of the
  // new mailfile name
  GetSysTimeUTC(&curDate);

  // encode this date as a base64 encoded string
  base64encode(&dateFilePart, (char *)&curDate, sizeof(curDate));

  // as the dateFilePart may contain slashes "/" we have to replace them
  // with "-" chars to not drive the filesystem crazy :)
  ptr = dateFilePart;
  while((ptr = strchr(ptr, '/')) != NULL)
    *ptr = '-';

  mCounter = 0;
  do
  {
    snprintf(newFileName, sizeof(newFileName), "%s.%03d,N", dateFilePart, ++mCounter);

    AddPath(fullPath, folder->Fullpath, newFileName, fullPathSize);

    // don't care about duplicate files in the drafts folder
    if(isDraftsFolder(folder) == TRUE)
      break;
  }
  while(mCounter < 999 && FileExists(fullPath) == TRUE);

  result = (mCounter < 999);

  free(dateFilePart);

  RETURN(result);
  return result;
}

///
/// MA_DetectUUE
//  Checks if message contains an uuencoded file
static BOOL MA_DetectUUE(FILE *fh)
{
  char *buffer = NULL;
  size_t size = 0;
  BOOL found = FALSE;

  ENTER();

  // Now we process the whole mailfile and check if there is any line that
  // starts with "begin xxx"
  while(GetLine(&buffer, &size, fh) >= 7)
  {
    // lets check for digit first because this will throw out many others first
    if(isdigit((int)buffer[6]) && strncmp(buffer, "begin ", 6) == 0)
    {
      found = TRUE;
      break;
    }
  }

  free(buffer);

  RETURN(found);
  return found;
}

///
/// FindHeader
// search for a specific header line
static struct HeaderNode *FindHeader(struct MinList *headerList, const char *name)
{
  struct HeaderNode *result = NULL;
  struct HeaderNode *hdrNode;

  ENTER();

  IterateList(headerList, struct HeaderNode *, hdrNode)
  {
    // compare the names
    if(stricmp(hdrNode->name, name) == 0)
    {
      result = hdrNode;
      break;
    }
  }

  RETURN(result);
  return result;
}

///
/// SplitAddressLine
// split a line of addresses into its parts
static char **SplitAddressLine(const char *line, ULONG *numParts)
{
  char **parts = NULL;
  int numCommas;
  const char *cptr;
  char *lineCopy;

  ENTER();

  *numParts = 0;

  // Count the number of commas in the given line. This is a rough estimation
  // about how many recipients may exist at most.
  numCommas = 0;
  cptr = line;
  do
  {
    if((cptr = MyStrChr(cptr, ',')) != NULL)
    {
      cptr++;
      numCommas++;
    }
  }
  while(cptr != NULL);

  SHOWVALUE(DBF_MIME, numCommas);

  // we must duplicate the line as we are going to modify it
  if((lineCopy = strdup(line)) != NULL)
  {
    SHOWSTRING(DBF_MIME, lineCopy);

    // Get some memory for the part pointers. We allocate one more than we
    // counted before, because there is one more recipient than the number
    // of commas. And even one more to NUL terminate the char** array
    if((parts = calloc(numCommas+2, sizeof(char *))) != NULL)
    {
      char *ptr = lineCopy;
      ULONG cnt = 0;

      // split the line into the individual parts separated by commas
      do
      {
        char *e;
        char *p;

        // find the next comma, but respect quotes
        if((e = MyStrChr(ptr, ',')) != NULL)
        {
          // terminate the part string
          *e++ = '\0';
        }

        p = TrimStart(ptr);
        if(*p != '\0')
        {
          // remember the duplicated non-empty part
          if((parts[cnt] = strdup(p)) == NULL)
          {
            // abort in case we couldn't duplicate the string
            break;
          }
          cnt++;
        }

        ptr = e;
        if(ptr != NULL)
          ptr = TrimStart(ptr);
      }
      while(ptr != NULL);

      *numParts = cnt;
    }

    // the duplicated line can be freed again
    free(lineCopy);
  }

  D(DBF_MIME, "splitted line into %ld parts", *numParts);

  RETURN(parts);
  return parts;
}

///
/// IsValidAddressLine
// check whether the given line of addresses is valid, i.e. all name+address
// parts are correctly quoted
static char *ValidateAddressLine(const char *line)
{
  char *validLine = NULL;
  char **parts;
  ULONG numParts;

  ENTER();

  // Certain MS software creates invalid address lines if the recipients name contains
  // accented characters and has reversed first and last name separated by a comma.
  // In this case the complete name will be encoded as quoted printable *without* being
  // enclosed in quotes.
  // For example a mail contains this address line:
  // =?iso-8859-1?Q?_DoE9=2C_John= <john.doe@anonymous.com>
  // After decoding the string looks like this (note the missing quotes):
  // Doé, John <john.doe@anonymous.com>
  // If such a line is parsed as usual assuming that two full addresses are separated
  // by a comma then we will get two false recipients:
  // 1. "Doé"
  // 2. "John <john.doe@anonymous.com>"
  // none of which is really correct.
  // Therefore we split the complete string into parts separated by commas while respecting
  // correctly quoted commas. After that the parts are combined again to an address line
  // and any missing quotes will be inserted as necessary.

  SHOWSTRING(DBF_MIME, line);

  // split the line into its parts
  if((parts = SplitAddressLine(line, &numParts)) != NULL)
  {
    if(numParts == 1)
    {
      // a single part is just taken as it is
      dstrcpy(&validLine, parts[0]);
    }
    else if(numParts >= 2)
    {
      ULONG i = 0;
      char *part;
      BOOL openQuotePending = FALSE;

      // now check if each part contains at least the @ character to make
      // it a valid address
      while(i < numParts)
      {
        part = parts[i];
        i++;
        if(part != NULL)
        {
          SHOWSTRING(DBF_MIME, part);

          if(validLine != NULL)
            dstrcat(&validLine, ", ");

          if(strchr(part, '@') == NULL)
          {
            BOOL atAppended = FALSE;

            D(DBF_MIME, "line part '%s' contains no '@' character", part);

            // Now we combine a new recipient of the current and the following parts
            // until we finally find a part which contains the @ character.

            // Most probably the @-less part was created because there was an unquoted
            // comma, so we add the missing quotes. They cause no harm.
            dstrcat(&validLine, "\"");
            dstrcat(&validLine, part);
            openQuotePending = TRUE;

            do
            {
              dstrcat(&validLine, ", ");

              part = parts[i];
              i++;
              if(part != NULL)
              {
                SHOWSTRING(DBF_MIME, part);

                // check whether this is the part containing the address
                if(strchr(part, '@') != NULL)
                {
                  char *addrStart;

                  // look for the beginning of the address
                  if((addrStart = strstr(part, " <")) != NULL)
                  {
                    // temporarily terminate the part before the address and add it to the line
                    *addrStart = '\0';

                    // append the first part
                    dstrcat(&validLine, part);

                    // add the closing quote
                    dstrcat(&validLine, "\"");
                    openQuotePending = FALSE;

                    // restore the space and continue with the remaining part
                    *addrStart = ' ';
                    part = addrStart;
                  }
                  atAppended = TRUE;
                }

                dstrcat(&validLine, part);
              }
            }
            while(i < numParts && atAppended == FALSE);
          }
          else
          {
            // this is a valid part, just append it
            dstrcat(&validLine, part);
          }
        }
      }

      // close any still opened quote
      if(openQuotePending == TRUE)
        dstrcat(&validLine, "\"");
    }

    // free the array of parts
    FreeStrArray(parts);
  }

  RETURN(validLine);
  return validLine;
}

///
/// MA_ReadHeader
//  Reads header lines of a message into memory
BOOL MA_ReadHeader(const char *mailFile, FILE *fh, struct MinList *headerList, enum ReadHeaderMode mode)
{
  BOOL success = FALSE;

  ENTER();

  if(headerList != NULL)
  {
    unsigned int linesread = 0;
    char *buffer = NULL;
    size_t size = 0;
    BOOL finished = FALSE;
    struct HeaderNode *hdrNode = NULL;

    D(DBF_MIME, "reading header lines of mail file '%s'", mailFile);

    // clear the headerList first
    NewMinList(headerList);

    // we read out the whole header line by line and
    // concatenate lines that are belonging together.
    while((GetLine(&buffer, &size, fh) >= 0 && (++linesread, buffer[0] != '\0')) ||
          (finished == FALSE && (finished = TRUE)))
    {
      // if the start of this line is a space or a tabulator sign
      // this line belongs to the last header also and we have to
      // add it to the last one.
      if((buffer[0] == ' ' || buffer[0] == '\t') && finished == FALSE)
      {
        if(hdrNode != NULL)
        {
          char *ptr;

          // move to the "real" start of the string so that we can copy
          // from there to our previous header.
          for(ptr = buffer; *ptr && isspace(*ptr); ptr++);

          // insert a space in case we are extending a previously parsed content
          if(dstrlen(hdrNode->content) != 0)
            dstrcat(&hdrNode->content, " ");

          // now concatenate this new headerstring to our previous one
          dstrcat(&hdrNode->content, ptr);
        }
      }
      else
      {
        // it seems that we have found another header line because
        // it didn't start with a linear-white-space, so lets
        // first validate the previous one, if it exists.
        if(hdrNode != NULL)
        {
          char *ptr;
          char *hdrContents = hdrNode->content;
          int len;

          // we first decode the header according to RFC 2047 which
          // should give us the full charset interpretation
          if((len = rfc2047_decode(hdrContents, hdrContents, strlen(hdrContents))) == -1)
          {
            E(DBF_FOLDER, "ERROR: malloc() error during rfc2047() decoding");
            break; // break-out
          }
          else if(len == -2)
          {
            W(DBF_FOLDER, "WARNING: unknown header encoding found");

            // signal an error but continue.
            ER_NewError(tr(MSG_ER_UNKNOWN_HEADER_ENCODING), hdrContents, mailFile);
          }
          else if(len == -3)
          {
            W(DBF_FOLDER, "WARNING: rfc2047 (base64) header decoding failed");
          }

          // now that we have decoded the headerline accoring to rfc2047
          // we have to strip out eventually existing ESC sequences as
          // this can be dangerous with MUI.
          for(ptr=hdrContents; *ptr; ptr++)
          {
            // if we find an ESC sequence, strip it!
            if(*ptr == 0x1b)
              *ptr = ' ';
          }

          // the headerNode seems to be finished so we put it into our
          // headerList
          D(DBF_MIME, "add header '%s' with content '%s'", hdrNode->name, hdrNode->content);
          AddTail((struct List *)headerList, (struct Node *)hdrNode);
        }

        // if we are finished we break out here
        if(finished == TRUE)
        {
          success = TRUE;
          break;
        }

        // now that we have finished the last header line
        // we can finally start processing a new one.
        // Which means we allocate a new HeaderNode and try to get out the header
        // name
        if((hdrNode = AllocHeaderNode()) != NULL)
        {
          char *ptr;

          // now we try to find the name of the header (ends with a ':' and no white space
          // or control character in between
          for(ptr = buffer; *ptr; ptr++)
          {
            if(*ptr == ':')
              break;
            else if(*ptr < 33 || *ptr > 126)
            {
              ptr = NULL;
              break;
            }
          }

          if(IsStrEmpty(ptr) == FALSE)
          {
            *ptr++ = '\0';

            // use our dstrcpy() function to copy the name of the header
            // into our ->name element
            if(dstrcpy(&hdrNode->name, buffer) != NULL)
            {
              // now we copy also the rest of buffer into the contents
              // of the headerNode
              // NOTE: this might be an empty string in case the contents
              // start on the next line. Thus we must not check the return
              // value to indicate that something has been copied.
              dstrcpy(&hdrNode->content, Trim(ptr));

              // everything seemed to work fine, so lets continue
              continue;
            }
          }

          // if we end up here then something went wrong and we have to clear
          // the header node and stuff
          FreeHeaderNode(hdrNode);
          hdrNode = NULL;
        }
        else
          break;
      }
    }

    // if we haven't had success in reading the headers
    // we make sure we clean everything up. If we read no
    // headers at all we return a failure. But if we were able to
    // read a single empty line it is a signal that this part of
    // the mail doesn't have any header at all (which may be valid)
    if(success == FALSE)
      ClearHeaderList(headerList);
    else if(IsMinListEmpty(headerList) == TRUE &&
            (mode == RHM_MAINHEADER || IsStrEmpty(buffer) == FALSE || linesread != 1))
    {
      W(DBF_MAIL, "no required header data found while scanning '%s'", mailFile);
      success = FALSE;
    }

    free(buffer);
  }

  if(success == TRUE)
  {
    struct HeaderNode *microsuckHeader;

    // So far only Microsoft Exchange seems to generate broken address lines.
    // Time will show if this will become an longer list...
    if((microsuckHeader = FindHeader(headerList, "x-mimeole")) != NULL && strstr(microsuckHeader->content, "Microsoft Exchange") != NULL)
    {
      const char *addressLineNames[] =
      {
        "from", "to", "reply-to", "cc", "bcc"
      };
      ULONG i;

      D(DBF_MIME, "mail was created by possibly broken Microsoft software ('%s'), validating address lines", microsuckHeader->content);

      // iterate over the possibly malformed header lines
      for(i = 0; i < ARRAY_SIZE(addressLineNames); i++)
      {
        struct HeaderNode *addressHeader;

        if((addressHeader = FindHeader(headerList, addressLineNames[i])) != NULL)
        {
          // Check whether potential EMail address are valid.
          // Buggy Microsoft software very often creates invalid addresses,
          // i.e 'lastname, firstname <address>' without the necessary quotes around the name
          char *validLine;

          D(DBF_MIME, "validating '%s' header with content '%s'", addressHeader->name, addressHeader->content);

          if((validLine = ValidateAddressLine(addressHeader->content)) != NULL)
          {
            dstrfree(addressHeader->content);
            addressHeader->content = validLine;
          }
        }
      }
    }
  }

  RETURN(success);
  return success;
}

///
/// MA_FreeEMailStruct
//  Frees an extended email structure
void MA_FreeEMailStruct(struct ExtendedMail *email)
{
  ENTER();

  if(email != NULL)
  {
    dstrfree(email->SenderInfo);
    email->SenderInfo = NULL;

    dstrfree(email->extraHeaders);
    email->extraHeaders = NULL;

    dstrfree(email->messageID);
    email->messageID = NULL;

    dstrfree(email->inReplyToMsgID);
    email->inReplyToMsgID = NULL;

    dstrfree(email->references);
    email->references = NULL;

    if(email->SFrom != NULL)
    {
      ASSERT(email->NumSFrom > 0);

      free(email->SFrom);
      email->SFrom = NULL;
      email->NumSFrom = 0;
    }

    if(email->STo != NULL)
    {
      ASSERT(email->NumSTo > 0);

      free(email->STo);
      email->STo = NULL;
      email->NumSTo = 0;
    }

    if(email->SReplyTo != NULL)
    {
      ASSERT(email->NumSReplyTo > 0);

      free(email->SReplyTo);
      email->SReplyTo = NULL;
      email->NumSReplyTo = 0;
    }

    if(email->CC != NULL)
    {
      ASSERT(email->NumCC > 0);

      free(email->CC);
      email->CC = NULL;
      email->NumCC = 0;
    }

    if(email->BCC != NULL)
    {
      ASSERT(email->NumBCC > 0);

      free(email->BCC);
      email->BCC = NULL;
      email->NumBCC = 0;
    }

    if(email->ResentTo != NULL)
    {
      ASSERT(email->NumResentTo > 0);

      free(email->ResentTo);
      email->ResentTo = NULL;
      email->NumResentTo = 0;
    }

    if(email->ResentCC != NULL)
    {
      ASSERT(email->NumResentCC > 0);

      free(email->ResentCC);
      email->ResentCC = NULL;
      email->NumResentCC = 0;
    }

    if(email->ResentBCC != NULL)
    {
      ASSERT(email->NumResentBCC > 0);

      free(email->ResentBCC);
      email->ResentBCC = NULL;
      email->NumResentBCC = 0;
    }

    if(email->FollowUpTo != NULL)
    {
      ASSERT(email->NumFollowUpTo > 0);

      free(email->FollowUpTo);
      email->FollowUpTo = NULL;
      email->NumFollowUpTo = 0;
    }

    if(email->MailReplyTo != NULL)
    {
      ASSERT(email->NumMailReplyTo > 0);

      free(email->MailReplyTo);
      email->MailReplyTo = NULL;
      email->NumMailReplyTo = 0;
    }

    free(email);
  }

  LEAVE();
}

///
/// MA_GetRecipients
//  Extracts recipients from a header field
static int MA_GetRecipients(char *h, struct Person **per)
{
  int cnt = 0;
  char *p = h;

  ENTER();

  while(*p != '\0')
  {
    cnt++;
    if((p = MyStrChr(p, ',')) != NULL)
      p++;
    else
      break;
  }

  if(cnt > 0)
  {
    // allocate enough memory to carry all
    // found recipients in an array of struct Person
    // structures.
    if((*per = calloc(cnt, sizeof(struct Person))) != NULL)
    {
      struct Person *cur = *per;

      for(p=h; *p; cur++)
      {
        char *next;

        if((next = MyStrChr(p, ',')) != NULL)
          *next++ = '\0';

        ExtractAddress(p, cur);

        D(DBF_MIME, "extracted rcpt: '%s' '%s'", cur->RealName, cur->Address);

        p = next;
        if(p == NULL)
          break;
      }
    }
    else
      cnt = 0;
  }
  else
    *per = NULL;

  RETURN(cnt);
  return cnt;
}

///
/// MA_ScanDate
//  Converts textual date header into datestamp format
static BOOL MA_ScanDate(struct Mail *mail, const char *date)
{
  BOOL success = FALSE;
  int count = 0;
  int day = 0;
  int mon = 0;
  int year = 0;
  int hour = 0;
  int min = 0;
  int sec = 0;
  int gmtOffset = INT_MIN; // INT_MIN means not set
  char *s;
  char tdate[SIZE_SMALL];
  char ttime[SIZE_SMALL];
  char tzAbbr[SIZE_SMALL];
  struct DateTime dt;

  ENTER();

  // make sure to skip the weekday definition if it exists
  if((s = strpbrk(date, " |;,")) != NULL)
  {
    // check if we did reach here because the whole
    // weekday definition was missing
    if(isspace(*s) && isdigit(*date))
      s = (char *)date;
    else
      s++;
  }
  else
  {
    W(DBF_MAIL, "no starting separator found!!");

    s = (char *)date;
  }

  // ensure a valid time zone abbreviation string
  // in case the parsing process fails to find one
  tzAbbr[0] = '\0';

  // skip leading spaces
  while(*s && isspace(*s))
    s++;

  while(*s)
  {
    char *e;

    if((e = strpbrk(s, " |;,")) == NULL)
      e = s+strlen(s);

    switch(count)
    {
      // get the day
      case 0:
      {
        if(!isdigit(*s) || (day = atoi(s)) > 31)
        {
          W(DBF_MAIL, "couldn't parse day from '%s'", s);

          day = 1;
        }
      }
      break;

      // get the month
      case 1:
      {
        for(mon = 1; mon <= 12; mon++)
        {
          if(strnicmp(s, months[mon-1], 3) == 0)
            break;
        }

        if(mon > 12)
        {
          W(DBF_MAIL, "couldn't parse month from '%s'", s);

          mon = 1;
        }
      }
      break;

      // get the year
      case 2:
      {
        if(isdigit(*s))
          year = atoi(s);
        else
        {
          W(DBF_MAIL, "couldn't parse year from '%s'", s);

          year = 1978;
        }
      }
      break;

      // get the time values
      case 3:
      {
        if(sscanf(s, "%d:%d:%d", &hour, &min, &sec) != 3)
        {
          if(sscanf(s, "%d:%d", &hour, &min) == 2)
            sec = 0;
          else
          {
            W(DBF_MAIL, "couldn't parse time from '%s'", s);

            hour = 0;
            min = 0;
            sec = 0;
          }
        }
      }
      break;

      // get the gmt offset (+0000 / -0000)
      case 4:
      {
        // skip any leading parentheses
        while(*s && *s == '(')
          s++;

        // check that the gmtOffset string starts with "+" or "-" or
        // with a numeric value or otherwise the date string doesn't
        // have a GMT offset but comes with a timezone abbreviation
        if(*s == '+' || *s == '-' || isdigit(*s))
        {
          if(isdigit(*s))
            gmtOffset = atoi(s);
          else
            gmtOffset = atoi(&s[1]);

          if(gmtOffset != 0 && gmtOffset/100 == 0)
          {
            char *c;

            // multiply by 100 so that we have now a correct format
            gmtOffset *= 100;

            // then check if we have a : to seperate HH:MM and add the minutes
            // to tzcorr
            if((c = strchr(s, ':')))
              gmtOffset += atoi(c);
          }

          // now we have to distingush between + and -
          if(s[0] == '-')
            gmtOffset = -gmtOffset;

          // convert to minutes now
          gmtOffset = (gmtOffset/100)*60 + (gmtOffset%100);
        }
        else
        {
          W(DBF_MAIL, "No GMT offset found in date string: %s", date);

          // make sure the next iteration ends up at the same position
          e = s;
        }
      }
      break;

      // the textual timezone abbreviation (e.g. 'CET')
      case 5:
      {
        // skip any leading parentheses
        while(*s && *s == '(')
          s++;

        // make sure the first char is A-Za-z
        if(isalpha(*s))
        {
          // remove any ending parentheses
          while(*(e-1) && *(e-1) == ')')
            e--;

          strlcpy(tzAbbr, s, MIN(sizeof(tzAbbr), (unsigned int)(e-s+1)));
        }
        else
          W(DBF_MAIL, "No timezone abbreviation found in date string: %s", date);
      }
      break;
    }

    // if we iterated until 4 we can break out
    if(count == 5)
      break;

    count++;

    // set the next start to our last search
    if(*e)
    {
      // skip leading spaces
      while(*e && isspace(*e))
        e++;

      s = e;
    }
    else
      break;
  }

  // then format a standard DateStamp string like string
  // so that we can use StrToDate()
  snprintf(tdate, sizeof(tdate), "%02d-%02d-%02d", mon, day, year % 100);
  snprintf(ttime, sizeof(ttime), "%02d:%02d:%02d", hour, min, sec);
  dt.dat_Format  = FORMAT_USA;
  dt.dat_Flags   = 0;
  dt.dat_StrDate = tdate;
  dt.dat_StrTime = ttime;
  if(StrToDate(&dt))
  {
    struct DateStamp *ds = &dt.dat_Stamp;

    // lets see if we found a valid GMT offset
    if(gmtOffset != INT_MIN)
      mail->gmtOffset = gmtOffset;
    else if(tzAbbr[0] != '\0')
      mail->gmtOffset = TZtoMinutes(tzAbbr);

    // save the tzone abbreviation
    strlcpy(mail->tzAbbr, tzAbbr, sizeof(mail->tzAbbr));

    // bring the date in relation to UTC
    ds->ds_Minute -= mail->gmtOffset;

    // we need to check the datestamp variable that it is still in it's borders
    // after the UTC correction
    while(ds->ds_Minute < 0)     { ds->ds_Minute += 1440; ds->ds_Days--; }
    while(ds->ds_Minute >= 1440) { ds->ds_Minute -= 1440; ds->ds_Days++; }

    // now we do copy the datestamp stuff over the one from our mail
    memcpy(&mail->Date, ds, sizeof(struct DateStamp));

    success = TRUE;
  }

  RETURN(success);
  return success;
}
///
/// MA_ExamineMail
//  Parses the header lines of a message and fills email structure
struct ExtendedMail *MA_ExamineMail(const struct Folder *folder, const char *file, const BOOL deep)
{
  struct ExtendedMail *email;
  static struct Person pe;
  struct MinList headerList;
  struct Mail *mail;
  char fullfile[SIZE_PATHFILE];
  BOOL dateFound = FALSE;
  FILE *fh;

  ENTER();

  D(DBF_MAIL, "Examining mail file '%s' from folder '%s' with deep %d", file, folder != NULL ? folder->Name : "<NULL>", deep);

  // first we generate a new ExtendedMail buffer
  if((email = calloc(1, sizeof(*email))) == NULL)
  {
    RETURN(NULL);
    return NULL;
  }

  mail = &email->Mail;
  strlcpy(mail->MailFile, file, sizeof(mail->MailFile));

  GetMailFile(fullfile, sizeof(fullfile), folder, mail);
  if((fh = fopen(fullfile, "r")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // if the first three bytes are 'X' 'P' 'K', then this is an XPK packed
    // file and we have to unpack it first.
    if(fgetc(fh) == 'X' && fgetc(fh) == 'P' && fgetc(fh) == 'K')
    {
      char mailfile[SIZE_PATHFILE];

      // temporary close the file
      fclose(fh);

      GetMailFile(mailfile, sizeof(mailfile), folder, mail);
      // then unpack the file with XPK routines.
      if(StartUnpack(mailfile, fullfile, folder) == NULL)
      {
        MA_FreeEMailStruct(email);

        E(DBF_MAIL, "couldn't unpack mailfile");

        RETURN(NULL);
        return NULL;
      }

      // reopen it again.
      if((fh = fopen(fullfile, "r")) != NULL)
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);
    }
    else
      rewind(fh); // rewind the file handle to the start
  }
  else
    E(DBF_MAIL, "couldn't open mail file for reading main header");

  // check if the file handle is valid and then immediatly read in the
  // header lines
  if(fh != NULL && MA_ReadHeader(fullfile, fh, &headerList, RHM_MAINHEADER) == TRUE)
  {
    BOOL foundFrom = FALSE;
    BOOL foundTo = FALSE;
    BOOL foundReplyTo = FALSE;
    char *ptr;
    char dateFilePart[12+1];
    LONG size;
    struct HeaderNode *hdrNode;
    struct UserIdentityNode *fromUIN = NULL;
    struct UserIdentityNode *toUIN = NULL;
    struct UserIdentityNode *replyToUIN = NULL;
    struct UserIdentityNode *ccUIN = NULL;
    struct UserIdentityNode *bccUIN = NULL;

    // Now we process the read header to set all flags accordingly.
    // The identities found within this loop must not be used immediately
    // for the mail's identity pointer as the single header lines might
    // appear in arbitrary order.
    IterateList(&headerList, struct HeaderNode *, hdrNode)
    {
      char *field = hdrNode->name;
      char *value = hdrNode->content;

      if(stricmp(field, "from") == 0)
      {
        char *p;
        foundFrom = TRUE;

        // find out if there are more than one From: address
        if((p = MyStrChr(value, ',')) != NULL)
         *p++ = '\0';

        // extract the main mail address
        ExtractAddress(value, &pe);
        mail->From = pe;

        // we have to check if we can match the user identity
        // from the email address
        if(deep == TRUE)
        {
          fromUIN = FindUserIdentityByAddress(&C->userIdentityList, mail->From.Address);
          D(DBF_MAIL, "finduinByAddr (from): '%s' %08lx", mail->From.Address, fromUIN);
        }

        // if we have more addresses waiting we
        // go and process them yet
        if(p != NULL)
        {
          if(deep == TRUE)
          {
            if(email->NumSFrom == 0)
              email->NumSFrom = MA_GetRecipients(p, &(email->SFrom));

            if(email->NumSFrom > 0)
            {
              int i;

              // if we haven't found the identity yet we process the
              // other from addresses
              for(i=0; fromUIN == NULL && i < email->NumSFrom; i++)
              {
                fromUIN = FindUserIdentityByAddress(&C->userIdentityList, email->SFrom[i].Address);
                D(DBF_MAIL, "finduinByAddr (from): '%s' %08lx", email->SFrom[i].Address, fromUIN);
              }

              setFlag(mail->mflags, MFLAG_MULTISENDER);
            }
          }
          else if(strlen(p) >= 7) // minimum rcpts size "a@bc.de"
            setFlag(mail->mflags, MFLAG_MULTISENDER);
        }

        D(DBF_MIME, "'From' senders: %ld", email->NumSFrom+1);
      }
      else if(stricmp(field, "reply-to") == 0)
      {
        char *p;

        foundReplyTo = TRUE;

        // find out if there are more than one ReplyTo: address
        if((p = MyStrChr(value, ',')) != NULL)
         *p++ = '\0';

        ExtractAddress(value, &pe);
        mail->ReplyTo = pe;

        // we have to check if we can match the user identity
        // from the email address
        if(deep == TRUE)
        {
          replyToUIN = FindUserIdentityByAddress(&C->userIdentityList, mail->ReplyTo.Address);
          D(DBF_MAIL, "finduinByAddr (replyto): '%s' %08lx", mail->ReplyTo.Address, replyToUIN);
        }

        // if we have more addresses waiting we
        // go and process them yet
        if(p != NULL)
        {
          if(deep == TRUE)
          {
            if(email->NumSReplyTo == 0)
              email->NumSReplyTo = MA_GetRecipients(p, &(email->SReplyTo));

            if(email->NumSReplyTo > 0)
            {
              int i;

              // if we haven't found the identity yet we process the
              // other from addresses
              for(i=0; replyToUIN == NULL && i < email->NumSReplyTo; i++)
              {
                replyToUIN = FindUserIdentityByAddress(&C->userIdentityList, email->SReplyTo[i].Address);
                D(DBF_MAIL, "finduinByAddr (replyto): '%s' %08lx", email->SReplyTo[i].Address, replyToUIN);
              }

              setFlag(mail->mflags, MFLAG_MULTIREPLYTO);
            }
          }
          else if(strlen(p) >= 7) // minimum rcpts size "a@bc.de"
            setFlag(mail->mflags, MFLAG_MULTIREPLYTO);
        }

        D(DBF_MIME, "'ReplyTo' recipients: %ld", email->NumSReplyTo+1);
      }
      else if(stricmp(field, "original-recipient") == 0)
      {
        ExtractAddress(value, &pe);
        email->OriginalRcpt = pe;
      }
      else if(stricmp(field, "return-path") == 0)
      {
        ExtractAddress(value, &pe);
        email->ReturnPath = pe;
      }
      else if(stricmp(field, "disposition-notification-to") == 0 ||
              stricmp(field, "return-receipt-to") == 0)
      {
        ExtractAddress(value, &pe);
        email->ReceiptTo = pe;
        setFlag(mail->mflags, MFLAG_SENDMDN);
      }
      else if(stricmp(field, "to") == 0)
      {
        if(foundTo == FALSE)
        {
          char *p;

          foundTo = TRUE;

          if((p = MyStrChr(value, ',')) != NULL)
            *p++ = '\0';

          ExtractAddress(value, &pe);
          mail->To = pe;

          // we have to check if we can match the user identity
          // from the email address
          if(deep == TRUE)
          {
            toUIN = FindUserIdentityByAddress(&C->userIdentityList, mail->To.Address);
            D(DBF_MAIL, "finduinByAddr (to): '%s' %08lx", mail->To.Address, toUIN);
          }

          if(p != NULL)
          {
            if(deep == TRUE)
            {
              if(email->NumSTo == 0)
                email->NumSTo = MA_GetRecipients(p, &(email->STo));

              if(email->NumSTo > 0)
              {
                int i;

                // if we haven't found the identity yet we process the
                // other from addresses
                for(i=0; toUIN == NULL && i < email->NumSTo; i++)
                {
                  toUIN = FindUserIdentityByAddress(&C->userIdentityList, email->STo[i].Address);
                  D(DBF_MAIL, "finduinByAddr (to): '%s' %08lx", email->STo[i].Address, toUIN);
                }

                setFlag(mail->mflags, MFLAG_MULTIRCPT);
              }
            }
            else if(strlen(p) >= 7) // minimum rcpts size "a@bc.de"
              setFlag(mail->mflags, MFLAG_MULTIRCPT);
          }

          D(DBF_MIME, "'To:' recipients: %ld", email->NumSTo+1);
        }
      }
      else if(stricmp(field, "cc") == 0)
      {
        if(deep == TRUE)
        {
          if(email->NumCC == 0)
            email->NumCC = MA_GetRecipients(value, &(email->CC));

          D(DBF_MIME, "'Cc:' recipients: %ld", email->NumCC);

          if(email->NumCC > 0)
          {
            int i;

            // if we haven't found the identity yet we process the
            // other from addresses
            for(i=0; ccUIN == NULL && i < email->NumCC; i++)
            {
              ccUIN = FindUserIdentityByAddress(&C->userIdentityList, email->CC[i].Address);
              D(DBF_MAIL, "finduinByAddr (cc): '%s' %08lx", email->CC[i].Address, ccUIN);
            }

            setFlag(mail->mflags, MFLAG_MULTIRCPT);
          }
        }
        else if(strlen(value) >= 7) // minimum rcpts size "a@bc.de"
          setFlag(mail->mflags, MFLAG_MULTIRCPT);
      }
      else if(stricmp(field, "bcc") == 0)
      {
        if(deep == TRUE)
        {
          if(email->NumBCC == 0)
            email->NumBCC = MA_GetRecipients(value, &(email->BCC));

          D(DBF_MIME, "'BCC:' recipients: %ld", email->NumBCC);

          if(email->NumBCC > 0)
          {
            int i;

            // if we haven't found the identity yet we process the
            // other from addresses
            for(i=0; bccUIN == NULL && i < email->NumBCC; i++)
            {
              bccUIN = FindUserIdentityByAddress(&C->userIdentityList, email->BCC[i].Address);
              D(DBF_MAIL, "finduinByAddr (bcc): '%s' %08lx", email->BCC[i].Address, bccUIN);
            }

            setFlag(mail->mflags, MFLAG_MULTIRCPT);
          }
        }
        else if(strlen(value) >= 7) // minimum rcpts size "a@bc.de"
          setFlag(mail->mflags, MFLAG_MULTIRCPT);
      }
      else if(stricmp(field, "resent-to") == 0)
      {
        if(email->NumResentTo == 0)
          email->NumResentTo = MA_GetRecipients(value, &(email->ResentTo));

        D(DBF_MIME, "'Resent-To:' recipients: %ld", email->NumResentTo);
      }
      else if(stricmp(field, "resent-cc") == 0)
      {
        if(email->NumResentCC == 0)
          email->NumResentCC = MA_GetRecipients(value, &(email->ResentCC));

        D(DBF_MIME, "'Resent-CC:' recipients: %ld", email->NumResentCC);
      }
      else if(stricmp(field, "resent-bcc") == 0)
      {
        if(email->NumResentBCC == 0)
          email->NumResentBCC = MA_GetRecipients(value, &(email->ResentBCC));

        D(DBF_MIME, "'Resent-BCC:' recipients: %ld", email->NumResentBCC);
      }
      else if(stricmp(field, "mail-followup-to") == 0)
      {
        if(email->NumFollowUpTo == 0)
          email->NumFollowUpTo = MA_GetRecipients(value, &(email->FollowUpTo));

        D(DBF_MIME, "'Mail-Followup-To:' recipients: %ld", email->NumFollowUpTo);
      }
      else if(stricmp(field, "mail-reply-to") == 0)
      {
        if(email->NumMailReplyTo == 0)
          email->NumMailReplyTo = MA_GetRecipients(value, &(email->MailReplyTo));

        D(DBF_MIME, "'Mail-Reply-To:' recipients: %ld", email->NumMailReplyTo);
      }
      else if(stricmp(field, "subject") == 0)
      {
        strlcpy(mail->Subject, Trim(value), sizeof(mail->Subject));
      }
      else if(stricmp(field, "message-id") == 0)
      {
        dstrcat(&email->messageID, Trim(value));
        mail->cMsgID = CompressMsgID(email->messageID);
      }
      else if(stricmp(field, "in-reply-to") == 0)
      {
        dstrcat(&email->inReplyToMsgID, Trim(value));
        mail->cIRTMsgID = CompressMsgID(email->inReplyToMsgID);
      }
      else if(stricmp(field, "references") == 0)
      {
        dstrcat(&email->references, Trim(value));
        D(DBF_MAIL, "References: '%s'", email->references);
      }
      else if(stricmp(field, "date") == 0)
      {
        dateFound = MA_ScanDate(mail, value);
      }
      else if(stricmp(field, "importance") == 0)
      {
        if(getImportanceLevel(mail) == IMP_NORMAL)
        {
          char *p = Trim(value);

          if(stricmp(p, "high") == 0)
            setImportanceLevel(mail, IMP_HIGH);
          else if(stricmp(p, "low") == 0)
            setImportanceLevel(mail, IMP_LOW);
        }
      }
      else if(stricmp(field, "priority") == 0)
      {
        if(getImportanceLevel(mail) == IMP_NORMAL)
        {
          char *p = Trim(value);

          if(stricmp(p, "urgent") == 0)
            setImportanceLevel(mail, IMP_HIGH);
          else if(stricmp(p, "non-urgent") == 0)
            setImportanceLevel(mail, IMP_HIGH);
        }
      }
      else if(stricmp(field, "content-type") == 0)
      {
        char *p = Trim(value);

        if(strnicmp(p, "multipart", 9) == 0)
        {
          p += 10;

          // we do specify the multipart content-type in
          // accordance to RFC 2046/RFC2387
          if(strnicmp(p, "mixed", 5) == 0)             // RFC 2046 (5.1.3)
            setFlag(mail->mflags, MFLAG_MP_MIXED);
          else if(strnicmp(p, "alternative", 11) == 0) // RFC 2046 (5.1.4)
            setFlag(mail->mflags, MFLAG_MP_ALTERN);
          else if(strnicmp(p, "report", 6) == 0)       // RFC 3462
            setFlag(mail->mflags, MFLAG_MP_REPORT);
          else if(strnicmp(p, "encrypted", 9) == 0)    // RFC 1847 (2.2)
            setFlag(mail->mflags, MFLAG_MP_CRYPT);
          else if(strnicmp(p, "signed", 6) == 0)       // RFC 1847 (2.1)
            setFlag(mail->mflags, MFLAG_MP_SIGNED);
          else
          {
            // "mixed" is the primary subtype and in fact RFC 2046 (5.1.7)
            // suggests to fall back to mixed if a MIME subtype is unknown
            // to a MIME parser, which we do here now.
            setFlag(mail->mflags, MFLAG_MP_MIXED);
          }
        }
        else if(strnicmp(p, "message/partial", 15) == 0) // RFC 2046 (5.2.2)
        {
          setFlag(mail->mflags, MFLAG_PARTIAL);
        }
      }
      else if(stricmp(field, "x-senderinfo") == 0)
      {
        setFlag(mail->mflags, MFLAG_SENDERINFO);
        if(deep == TRUE)
          dstrcat(&email->SenderInfo, value);
      }
      else if(deep == TRUE) // and if we end up here we check if we really have to go further
      {
        if(stricmp(field, "x-yam-options") == 0)
        {
          char *p;
          int sec;

          // check for the delsent flag first
          if(strcasestr(value, "delsent") != NULL)
          {
            D(DBF_MIME, "delsent found");
            email->DelSend = TRUE;
          }

          // check for the signature flag
          if((p = strcasestr(value, "signature=")) != NULL)
          {
            char idStr[9] = ""; // the is only 8 chars long + 1 NUL

            strlcpy(idStr, &p[8], sizeof(idStr));
            email->signatureID = strtol(idStr, NULL, 16);

            // try to get the signature structure
            if(email->signatureID != 0)
            {
              email->signature = FindSignatureByID(&C->signatureList, email->signatureID);
              D(DBF_MAIL, "findSignatureById: '%08x' %08lx", email->signatureID, email->signature);
            }

            D(DBF_MAIL, "found signature: '%s' %08x %08x", idStr, email->signatureID, email->signature != NULL ? email->signature->id : 0);
          }

          // check if the identity is listed and if so this has
          // absolute priority (so we overwrite any previously
          // set email->identity ptr)
          if((p = strcasestr(value, "identity=")) != NULL)
          {
            char idStr[9] = ""; // the id is only 8 chars long + 1 NUL

            strlcpy(idStr, &p[9], sizeof(idStr));
            email->identityID = strtol(idStr, NULL, 16);

            // try to get the identity structure
            if(email->identityID != 0)
            {
              email->identity = FindUserIdentityByID(&C->userIdentityList, email->identityID);
              D(DBF_MAIL, "finduinById: '%08x' %08lx", email->identityID, email->identity);
            }

            D(DBF_MAIL, "found identity: '%s' %08x %08x", idStr, email->identityID, email->identity != NULL ? email->identity->id : 0);
          }

          // check security flags
          if((p = strcasestr(value, "security=")) != NULL)
          {
            // check for the security flags for signing/encrypting a mail
            // but walk backwards here otherwise we can't use break
            for(sec = SEC_DEFAULTS; sec >= SEC_NONE; sec--)
            {
              if(strcasestr(&p[9], SecCodes[sec]) != NULL)
              {
                email->Security = sec;
                D(DBF_MIME, "found security: %d (%s)", email->Security, SecCodes[sec]);
                break;
              }
            }
          }
        }
        else if(strnicmp(field, "x-yam-header-", 13) == 0)
        {
          dstrcat(&email->extraHeaders, &field[13]);
          dstrcat(&email->extraHeaders, ":");
          dstrcat(&email->extraHeaders, value);
          dstrcat(&email->extraHeaders, "\\n");
        }
      }
    }

    // if now the mail is still not MULTIPART we have to check for uuencoded attachments
    if(!isMP_MixedMail(mail) && MA_DetectUUE(fh) == TRUE)
      setFlag(mail->mflags, MFLAG_MP_MIXED);

    // in case we found no From: head line we try to construct a name
    // from a possible Sender: line
    if(foundFrom == FALSE)
    {
      D(DBF_MIME, "no From: header");

      if((hdrNode = FindHeader(&headerList, "sender")) != NULL)
      {
        char *value = hdrNode->content;
        char *p;

        // find out if there are more than one From: address
        if((p = MyStrChr(value, ',')) != NULL)
         *p++ = '\0';

        // extract the main mail address
        ExtractAddress(value, &pe);
        mail->From = pe;

        // we have to check if we can match the user identity
        // from the email address
        if(deep == TRUE)
        {
          fromUIN = FindUserIdentityByAddress(&C->userIdentityList, mail->From.Address);
          D(DBF_MAIL, "finduinByAddr (sender): '%s' %08lx", mail->From.Address, fromUIN);
        }

        D(DBF_MIME, "From: address obtained from Sender: header");

        // if we have more addresses waiting we
        // go and process them yet
        if(p != NULL)
        {
          if(deep == TRUE)
          {
            if(email->NumSFrom == 0)
              email->NumSFrom = MA_GetRecipients(p, &(email->SFrom));

            if(email->NumSFrom > 0)
            {
              int i;

              // if we haven't found the identity yet we process the
              // other from addresses
              for(i=0; fromUIN == NULL && i < email->NumSFrom; i++)
              {
                fromUIN = FindUserIdentityByAddress(&C->userIdentityList, email->SFrom[i].Address);
                D(DBF_MAIL, "finduinByAddr (sender): '%s' %08lx", email->SFrom[i].Address, fromUIN);
              }

              setFlag(mail->mflags, MFLAG_MULTISENDER);
            }
          }
          else if(strlen(p) >= 7) // minimum rcpts size "a@bc.de"
            setFlag(mail->mflags, MFLAG_MULTISENDER);
        }

        D(DBF_MIME, "'Sender' senders: %ld", email->NumSFrom+1);

        foundFrom = TRUE;
      }
    }

    // And now we close the Mailfile and clear the temporary headerList again
    fclose(fh);
    ClearHeaderList(&headerList);

    // Now choose the user identity from the identities found in the loop
    // above. We start with the identity found by the To: header line, as
    // this is the one which should match best. Next are the other possible
    // "receiving" addresses and finally the possible sender addresses.
    if(toUIN != NULL)
      email->identity = toUIN;
    else if(ccUIN != NULL)
      email->identity = ccUIN;
    else if(bccUIN != NULL)
      email->identity = bccUIN;
    else if(replyToUIN != NULL)
      email->identity = replyToUIN;
    else if(fromUIN != NULL)
      email->identity = fromUIN;

    // if we still don't have identified a potential user identity
    // that matches the From:/To:/Reply-To:, etc. headers contents
    // (e.g. because deep==FALSE) then we set the default identity
    if(email->identity == NULL)
      email->identity = GetUserIdentity(&C->userIdentityList, 0, TRUE);

    if(email->identity != NULL)
      D(DBF_MAIL, "final identity: %08x '%s'", email->identity->id, email->identity->description);
    else
      E(DBF_MAIL, "no identities configured");

    // in case the replyTo recipient doesn't have a realname yet and it is
    // completly the same like the from address we go and copy the realname as both
    // are the same.
    if(foundReplyTo == TRUE && mail->ReplyTo.RealName[0] != '\0' && stricmp(mail->ReplyTo.Address, mail->From.Address) == 0)
      strlcpy(mail->ReplyTo.RealName, mail->From.RealName, sizeof(mail->ReplyTo.RealName));

    // if this function call has a folder of NULL then we are examining a virtual mail
    // which means this mail doesn't have any folder and also no filename that may contain
    // any usable date or stuff
    if(folder != NULL)
    {
      char *timebuf = NULL;

      // now we take the filename of our mailfile into account to check for
      // the transfer date at the start of the name and for the set status
      // flags at the end of it.
      strlcpy(dateFilePart, mail->MailFile, sizeof(dateFilePart));

      // make sure there is no "-" in the base64 encoded part as we just mapped
      // the not allowed "/" to "-" to make it possible to use base64 for
      // the timeval encoding
      ptr = dateFilePart;
      while((ptr = strchr(ptr, '-')) != NULL)
        *ptr = '/';

      // lets decode the base64 encoded timestring in a temporary buffer
      if(base64decode(&timebuf, dateFilePart, strlen(dateFilePart)) <= 0)
      {
        W(DBF_FOLDER, "WARNING: failure in decoding the encoded date from mailfile: '%s'", mail->MailFile);

        // if we weren't able to decode the base64 encoded string
        // we have to validate the transDate so that the calling function
        // recognizes to rewrite the comment with a valid string.
        mail->transDate.Seconds      = 0;
        mail->transDate.Microseconds = 0;
      }
      else
      {
        // everything seems to have worked so lets copy the binary data in our
        // transDate structure
        memcpy(&mail->transDate, timebuf, sizeof(mail->transDate));

        free(timebuf);
      }

      // now grab the status out of the end of the mailfilename
      ptr = &mail->MailFile[17];
      while(*ptr != '\0')
      {
        if(*ptr >= '1' && *ptr <= '7')
        {
          setPERValue(mail, *ptr-'1'+1);
        }
        else
        {
          switch(*ptr)
          {
            case SCHAR_READ:
              setFlag(mail->sflags, SFLAG_READ);
            break;

            case SCHAR_REPLIED:
              setFlag(mail->sflags, SFLAG_REPLIED);
            break;

            case SCHAR_FORWARDED:
              setFlag(mail->sflags, SFLAG_FORWARDED);
            break;

            case SCHAR_NEW:
              setFlag(mail->sflags, SFLAG_NEW);
            break;

            case SCHAR_SENT:
              setFlag(mail->sflags, SFLAG_SENT);
            break;

            case SCHAR_MARKED:
              setFlag(mail->sflags, SFLAG_MARKED);
            break;

            case SCHAR_ERROR:
              setFlag(mail->sflags, SFLAG_ERROR);
            break;

            case SCHAR_USERSPAM:
              setFlag(mail->sflags, SFLAG_USERSPAM);
            break;

            case SCHAR_AUTOSPAM:
              setFlag(mail->sflags, SFLAG_AUTOSPAM);
            break;

            case SCHAR_HAM:
              setFlag(mail->sflags, SFLAG_HAM);
            break;

            default:
              W(DBF_FOLDER, "invalid mail status character '%lc' found", *ptr);
            break;
          }
        }

        ptr++;
      }
    }

    // if we didn't find a Date: header we take the transfered date (if found)
    if(dateFound == FALSE)
    {
      if(mail->transDate.Seconds > 0)
      {
        // convert the UTC transDate to a UTC mail Date
        TimeVal2DateStamp(&mail->transDate, &mail->Date, TZC_NONE);
      }
      else
      {
        // and as a fallback we take the date of the mail file
        if(ObtainFileInfo(mail->MailFile, FI_DATE, &mail->Date) == TRUE)
        {
          // we store the mail date in UTC, thus convert it to
          // a UTC relative DateStamp accordingly.
          DateStampTZConvert(&mail->Date, TZC_LOCAL2UTC);
        }
      }

      // set the timeZone to our local one
      mail->gmtOffset = G->gmtOffset;
      strlcpy(mail->tzAbbr, G->tzAbbr, sizeof(mail->tzAbbr));
    }

    // lets calculate the mailSize out of the FileSize() function
    if(ObtainFileInfo(fullfile, FI_SIZE, &size) == TRUE)
      mail->Size = size;
    else
      mail->Size = -1;

    FinishUnpack(fullfile);

    RETURN(email);
    return email;
  }
  else
    E(DBF_MAIL, "couldn't read/parse mail header of mail file '%s'", fullfile);

  FinishUnpack(fullfile);

  // finish up everything before we exit with an error
  if(fh != NULL)
    fclose(fh);

  MA_FreeEMailStruct(email);

  RETURN(NULL);
  return NULL;
}

///
/// MA_ScanMailBox
//  Scans for message files in a folder directory
static BOOL MA_ScanMailBox(struct Folder *folder)
{
  long filecount;
  BOOL result = TRUE;
  static BOOL alreadyScanning = FALSE;

  ENTER();

  filecount = FileCount(folder->Fullpath, NULL);

  // check if there are files in this mailbox or not.
  if(filecount < 0)
  {
    // an error happened in FileCount()
    result = FALSE;
  }
  else if(filecount == 0)
  {
    // an empty directory is ok
    result = TRUE;
  }
  else
  {
    // check if we are already in this function or not
    // (due to a previously started scanning
    if(alreadyScanning == TRUE)
    {
      result = FALSE;
    }
    else
    {
      struct MA_GUIData *gui = &G->MA->GUI;
      struct BusyNode *busy;
      APTR context;

      // make sure others notice that an index scanning already
      // runs
      alreadyScanning = TRUE;

      // now we make sure some GUI components will be disabled
      // or cleared if the rescanning folder is the current one
      if(GetCurrentFolder() == folder)
      {
        // before we go and rebuild the index of the folder we make
        // sure all major GUI components of it are disabled for the
        // time being...
        xset(gui->PG_MAILLIST, MUIA_Disabled, TRUE);
        DoMethod(gui->PG_MAILLIST, MUIM_NList_Clear);

        // and now we also make sure an eventually enabled quicksearch bar
        // is enabled again as well.
        if(C->QuickSearchBarPos != QSB_POS_OFF)
          set(gui->GR_QUICKSEARCHBAR, MUIA_Disabled, TRUE);

        // also set an embedded read pane as disabled.
        if(C->EmbeddedReadPane == TRUE)
        {
          DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_Clear, MUIF_NONE);
          set(gui->MN_EMBEDDEDREADPANE, MUIA_Disabled, TRUE);
        }
      }

      busy = BusyBegin(BUSY_PROGRESS_ABORT);
      BusyText(busy, tr(MSG_BusyScanning), folder->Name);
      ClearFolderMails(folder, TRUE);

      D(DBF_FOLDER, "Scanning folder: '%s' (path '%s', %ld files)...", folder->Name, folder->Fullpath, filecount);

      if((context = ObtainDirContextTags(EX_StringName, (IPTR)folder->Fullpath,
                                         EX_DataFields, EXF_TYPE|EXF_NAME|EXF_SIZE,
                                         TAG_DONE)) != NULL)
      {
        struct ExamineData *ed;
        LONG error;
        long processedFiles = 0;
        BOOL convertAllOld = FALSE;
        BOOL skipAllOld = FALSE;
        BOOL convertAllUnknown = FALSE;
        BOOL skipAllUnknown = FALSE;
        BOOL ignoreInvalids = FALSE;

        // Now that the folder is locked we go and define its loaded
        // mode to LM_REBUILD so that others don't try to access it
        // anymore.
        folder->LoadedMode = LM_REBUILD;
        // visually update this state change
        DoMethod(gui->NL_FOLDERS, MUIM_NListtree_Redraw, folder->Treenode, MUIF_NONE);

        while((ed = ExamineDir(context)) != NULL)
        {
          // set the gauge and check the stopButton status as well.
          if(BusyProgress(busy, ++processedFiles, filecount) == FALSE)
          {
            D(DBF_FOLDER, "scan process aborted by user");
            result = FALSE;
            break;
          }

          // give the GUI the chance to refresh
          DoMethod(G->App,MUIM_Application_InputBuffered);

          // then check whether this is a file as we don't care for subdirectories
          if(EXD_IS_FILE(ed))
          {
            // check whether the filename is a valid mailfilename
            char fbuf[SIZE_PATHFILE+1];
            char *fname = (char *)ed->Name;
            BOOL validMailFile = isValidMailFile(fname);

            if(validMailFile == FALSE)
            {
              // ok, the file doesn't seem to have to be a valid mail filename, so
              // let's see if it is an "old" file (<= YAM2.4) or just trash
              int i = 0;
              BOOL oldFound = TRUE;

              do
              {
                // on position 5 should be a colon
                if(i == 5)
                {
                  if(fname[i] != '.')
                  {
                    oldFound = FALSE;
                    break;
                  }
                }
                else if(!isdigit(fname[i]))
                {
                  oldFound = FALSE;
                  break;
                }
              }
              while(fname[++i] != '\0');

              // check if our test was successfully and we found and old style
              // filename
              if(oldFound == TRUE)
              {
                int res;
                BOOL convertOnce = FALSE;

                // ok we seem to have found an old-fashioned mailfile, so let's
                // convert it to the newstyle
                W(DBF_FOLDER, "found < v2.5 style mailfile '%s'", fname);

                // let's ask if the user wants to convert the file or not
                if(convertAllOld == FALSE && skipAllOld == FALSE)
                {
                  res = MUI_Request(G->App, G->MA != NULL ? G->MA->GUI.WI : NULL, MUIF_NONE,
                                    tr(MSG_MA_CREQ_OLDFILE_TITLE),
                                    tr(MSG_MA_YESNOTOALL),
                                    tr(MSG_MA_CREQUEST_OLDFILE),
                                    fname, folder->Name);

                  // if the user has clicked on Yes or YesToAll then
                  // set the flags accordingly
                  if(res == 0)
                    skipAllOld = TRUE;
                  else if(res == 1)
                    convertOnce = TRUE;
                  else if(res == 2)
                    convertAllOld = TRUE;
                }

                if(convertAllOld == TRUE || convertOnce == TRUE)
                {
                  char *newfname;

                  // now we finally convert the file to a new style mail file
                  if((newfname = MA_ConvertOldMailFile(fname, folder)) == NULL)
                  {
                    // if there occurred any error we skip to the next file.
                    ER_NewError(tr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                    continue;
                  }
                  else
                  {
                    // use the new name from now on
                    fname = newfname;
                  }
                }
                else
                  continue;
              }
              else if(fname[0] != '.')
              {
                // to make it as convienent as possible for a user we also allow
                // to copy mail files to a folder directory without having to take
                // care that they have the correct filename.
                int res;
                BOOL convertOnce = FALSE;

                W(DBF_FOLDER, "found unknown file '%s'", fname);

                // lets ask if the user wants to convert the file or not
                if(convertAllUnknown == FALSE && skipAllUnknown == FALSE)
                {
                  res = MUI_Request(G->App, G->MA != NULL ? G->MA->GUI.WI : NULL, MUIF_NONE,
                                    tr(MSG_MA_CREQ_UNKNOWN_TITLE),
                                    tr(MSG_MA_YESNOTOALL),
                                    tr(MSG_MA_CREQUEST_UNKNOWN),
                                    fname, folder->Name);

                  // if the user has clicked on Yes or YesToAll then
                  // set the flags accordingly
                  if(res == 0)
                    skipAllUnknown = TRUE;
                  else if(res == 1)
                    convertOnce = TRUE;
                  else if(res == 2)
                    convertAllUnknown = TRUE;
                }

                if(convertAllUnknown == TRUE || convertOnce == TRUE)
                {
                  // now it is our job to get a new mailfile name and replace the old one
                  char oldfilePath[SIZE_PATHFILE];
                  char newfilePath[SIZE_PATHFILE];

                  AddPath(oldfilePath, folder->Fullpath, fname, sizeof(oldfilePath));
                  if(MA_NewMailFile(folder, newfilePath, sizeof(newfilePath)) == TRUE)
                  {
                    if(Rename(oldfilePath, newfilePath))
                    {
                      strlcpy(fbuf, FilePart(newfilePath), sizeof(fbuf));
                      fname = fbuf;
                    }
                    else
                    {
                      // if there occurred any error we skip to the next file.
                      ER_NewError(tr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                      continue;
                    }
                  }
                  else
                  {
                    // if there occurred any error we skip to the next file.
                    ER_NewError(tr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                    continue;
                  }
                }
                else
                  continue;
              }
              else
              {
                D(DBF_FOLDER, "skipping file '%s'", fname);
                continue;
              }
            }

            // check the filesize of the mail file
            if(ed->FileSize > 0)
            {
              struct ExtendedMail *email;

              D(DBF_FOLDER, "examining mail file '%s'", fname);

              while((email = MA_ExamineMail(folder, fname, FALSE)) == NULL &&
                    ignoreInvalids == FALSE)
              {
                // if the MA_ExamineMail() operation failed we
                // warn the user and ask him how to proceed with
                // the file

                int res = MUI_Request(G->App, G->MA != NULL ? G->MA->GUI.WI : NULL, MUIF_NONE,
                                     tr(MSG_MA_INVALIDMFILE_TITLE),
                                     tr(MSG_MA_INVALIDMFILE_BT),
                                     tr(MSG_MA_INVALIDMFILE),
                                     fname, folder->Name);

                if(res == 0) // cancel/ESC
                {
                  result = FALSE;
                  break;
                }
                else if(res == 1) // Retry
                  continue;
                else if(res == 2) // Ignore
                  break;
                else if(res == 3) // Ignore All
                {
                  ignoreInvalids = TRUE;
                  break;
                }
                else if(res == 4) // Delete
                {
                  char path[SIZE_PATHFILE+1];

                  AddPath(path, folder->Fullpath, fname, sizeof(path));
                  DeleteFile(path);

                  break;
                }
              }

              if(email != NULL)
              {
                struct Mail *newMail;

                if((newMail = CloneMail(&email->Mail)) != NULL)
                {
                  AddMailToFolder(newMail, folder);

                  // if this new mail hasn't got a valid transDate we have to check if we
                  // have to take the fileDate as a fallback value.
                  if(newMail->transDate.Seconds == 0)
                  {
                    // only if it is _not_ a "waitforsend" and "hold" message we can take the fib_Date
                    // as the fallback
                    if(isDraftsFolder(folder) == FALSE && isOutgoingFolder(folder) == FALSE)
                    {
                      char mailfile[SIZE_PATHFILE];
                      struct DateStamp ds;

                      W(DBF_FOLDER, "no transfer date information found in mail file, using file date...");

                      GetMailFile(mailfile, sizeof(mailfile), NULL, newMail);
                      // obtain the datestamp information from  and as a fallback we take the date of the mail file
                      if(ObtainFileInfo(mailfile, FI_DATE, &ds) == TRUE)
                      {
                        // now convert the local TZ fib_Date to a UTC transDate
                        DateStamp2TimeVal(&ds, &newMail->transDate, TZC_LOCAL2UTC);
                      }

                      // then we update the mailfilename
                      MA_UpdateMailFile(newMail);
                    }
                  }
                }

                MA_FreeEMailStruct(email);
              }
            }
            else
            {
              char path[SIZE_PATHFILE+1];

              AddPath(path, folder->Fullpath, fname, sizeof(path));
              DeleteFile(path);

              W(DBF_FOLDER, "found empty file '%s' in mail folder and deleted it", path);
            }
          }
        }

        error = IoErr();
        if(error != 0 && error != ERROR_NO_MORE_ENTRIES)
          E(DBF_FOLDER, "ExamineDir() failed, error %ld", error);

        ReleaseDirContext(context);
      }
      else
      {
        W(DBF_FOLDER, "couldn't allocate DirContext structure for directory '%s', IoErr()=%ld", folder->Fullpath, IoErr());
        result = FALSE;
      }

      D(DBF_FOLDER, "scanning finished %s", result ? "successfully" : "unsuccessfully");

      BusyEnd(busy);

      // make sure others can use this function again
      alreadyScanning = FALSE;
    }
  }

  RETURN(result);
  return result;
}
///

/*** Hooks ***/

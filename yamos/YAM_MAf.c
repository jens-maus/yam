/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_rexx.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

/*
** The following structures are used to build the
** .index files of a folder.
**
** Alignment is very important here, because the
** structures are saved RAW to a file.
** Therefore we have to ensure that the alignment
** will be the same for 68k and PPC.
** That`s why we deal with the following #pragma
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
   unsigned char    mailFile[SIZE_MFILE]; // mail filename without path
   struct DateStamp date;                 // the creation date of the mail (UTC)
   struct timeval   transDate;            // the received/sent date with ms (UTC)
   unsigned int     sflags;               // mail status flags
   unsigned int     mflags;               // general mail flags
   unsigned long    cMsgID;               // compressed MessageID
   unsigned long    cIRTMsgID;            // compressed InReturnTo MessageID
   long             size;                 // the total size of the message
   unsigned int     moreBytes;            // more bytes to follow as the subject
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
#define FINDEX_VER  (MAKE_ID('Y','I','N','7'))

#include "default-align.h"

/* local protos */
static void MA_ValidateStatus(struct Folder*);
static char *MA_IndexFileName(struct Folder*);
static BOOL MA_DetectUUE(FILE*);
static void MA_GetRecipients(char*, struct Person**, int*);
static BOOL MA_ScanDate(struct Mail *mail, const char *date);
static char *MA_ConvertOldMailFile(char *filename, struct Folder *folder);

/***************************************************************************
 Module: Main - Folder handling
***************************************************************************/

/*** Index Maintenance ***/
/// MA_PromptFolderPassword
//  Asks user for folder password
BOOL MA_PromptFolderPassword(struct Folder *fo, APTR win)
{
   char passwd[SIZE_PASSWORD], prompt[SIZE_LARGE];
   struct User *user = US_GetCurrentUser();

   if (isFreeAccess(fo)) return TRUE;
   if (!Stricmp(fo->Password, user->Password)) return TRUE;
   sprintf(prompt, GetStr(MSG_MA_GetFolderPass), fo->Name);
   do {
      *passwd = 0;
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_Folder), prompt, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, win)) return FALSE;
   } while (Stricmp(passwd, fo->Password));
   return TRUE;
}

///
/// MA_ValidateStatus
//  Avoids invalid status values
static void MA_ValidateStatus(struct Folder *folder)
{
   struct Mail *mail;

   if(C->UpdateNewMail            ||
      folder->Type == FT_OUTGOING ||
      folder->Type == FT_SENT
     )
   {
      DB(kprintf("Validating status of new msgs in folder %s\n", folder->Name);)

      for(mail = folder->Messages; mail; mail = mail->Next)
      {
        if(hasStatusNew(mail))
        {
          if(folder->Type == FT_OUTGOING)
            setStatusToQueued(mail);
          else if(folder->Type == FT_SENT)
            setStatusToSent(mail);
          else
            setStatusToUnread(mail);
        }
      }
   }
}

///
/// MA_IndexFileName
//  Returns file name of folder index
static char *MA_IndexFileName(struct Folder *folder)
{
   static char buffer[SIZE_PATHFILE];
   strcpy(buffer, GetFolderDir(folder));
   AddPart(buffer, ".index", SIZE_PATHFILE);
   return buffer;
}

///
/// MA_LoadIndex
//  Loads a folder index from disk
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full)
{
   FILE *fh;
   enum LoadedMode indexloaded = LM_UNLOAD;
   char buf[SIZE_LARGE];
   BOOL corrupt = FALSE;
   BOOL error = FALSE;

   DB( kprintf("Loading index for folder %s\n", folder->Name); )

   if ((fh = fopen(MA_IndexFileName(folder), "r")))
   {
      struct FIndex fi;

      BusyText(GetStr(MSG_BusyLoadingIndex), folder->Name);
      if(fread(&fi, sizeof(struct FIndex), 1, fh) != 1)
      {
        DB(kprintf("error while loading struct FIndex from .index file\n");)
        error = TRUE;
      }
      else if(fi.ID == FINDEX_VER)
      {
         folder->Total  = fi.Total;
         folder->New    = fi.New;
         folder->Unread = fi.Unread;
         folder->Size   = fi.Size;
         indexloaded = LM_FLUSHED;

         if (full)
         {
            ClearMailList(folder, TRUE);
            for(;;)
            {
               struct Mail mail;
               struct ComprMail cmail;
               memset(&mail, 0, sizeof(struct Mail));
               if(fread(&cmail, sizeof(struct ComprMail), 1, fh) != 1)
               {
                  // check if we are here because of an error or EOF
                  if(ferror(fh) != 0 || feof(fh) == 0)
                  {
                    DB(kprintf("error while loading ComprMail struct from .index file\n");)
                    error = TRUE;
                  }

                  // if we end up here it is just a EOF and no error.
                  break;
               }

               if(cmail.moreBytes > SIZE_LARGE)
               {
                  printf("WARNING: Index of folder '%s' CORRUPTED near mailfile '%s' (MoreBytes: 0x%x) - aborting!\n", folder->Name, cmail.mailFile, cmail.moreBytes);
                  printf("File position: %ld\n", ftell(fh));

                  corrupt = TRUE;
                  break;
               }

               if(fread(buf, cmail.moreBytes, 1, fh) != 1)
               {
                 DB(kprintf("fread error while reading index file\n");)
                 error = TRUE;
                 break;
               }

               strcpy(mail.Subject, GetNextLine(buf));
               strcpy(mail.From.Address, GetNextLine(NULL));
               strcpy(mail.From.RealName, GetNextLine(NULL));
               strcpy(mail.To.Address, GetNextLine(NULL));
               strcpy(mail.To.RealName, GetNextLine(NULL));
               strcpy(mail.ReplyTo.Address, GetNextLine(NULL));
               strcpy(mail.ReplyTo.RealName, GetNextLine(NULL));
               mail.Folder = folder;
               mail.mflags = cmail.mflags;
               mail.sflags = cmail.sflags;
               setVOLValue(&mail, 0);  // we have to make sure that the volatile flag field isn`t loaded
               strcpy(mail.MailFile, cmail.mailFile);
               mail.Date = cmail.date;
               mail.transDate = cmail.transDate;
               mail.cMsgID = cmail.cMsgID;
               mail.cIRTMsgID = cmail.cIRTMsgID;
               mail.Size = cmail.size;

               // finally add the new mail structure to our mail list
               if(AddMailToList(&mail, folder) == NULL)
               {
                 DB(kprintf("AddMailToList returned NULL!\n");)
                 error = TRUE;
                 break;
               }
            }
         }
      }

      if(!error && ferror(fh) == 1)
        error = TRUE;

      BusyEnd();
      fclose(fh);
   }

   if(error)
   {
     DB(kprintf("an error occurred while trying to load the index file '%s'\n", MA_IndexFileName(folder));)
     ClearMailList(folder, TRUE);

     return LM_UNLOAD;
   }

   if(corrupt || indexloaded == LM_UNLOAD)
   {
     DB( kprintf("  corrupt or outdated .index file detected, %s\n", full ? "rebuilding..." : "skipping..."); )
     ClearMailList(folder, TRUE);
     if(full)
     {
        MA_ScanMailBox(folder);
        if(MA_SaveIndex(folder)) indexloaded = LM_VALID;
     }
   }
   else if(full)
   {
     indexloaded = LM_VALID;
     CLEAR_FLAG(folder->Flags, FOFL_MODIFY);
   }

   return indexloaded;
}

///
/// MA_SaveIndex
//  Saves a folder index to disk
BOOL MA_SaveIndex(struct Folder *folder)
{
   struct Mail *mail;
   FILE *fh;
   struct ComprMail cmail;
   char buf[SIZE_LARGE];
   struct FIndex fi;

   if (!(fh = fopen(MA_IndexFileName(folder), "w"))) return FALSE;
   BusyText(GetStr(MSG_BusySavingIndex), folder->Name);

   // lets prepare the Folder Index struct and write it out
   // we clear it first, so that the reserved field is also 0
   memset(&fi, 0, sizeof(struct FIndex));
   fi.ID = FINDEX_VER;
   fi.Total = folder->Total; fi.New = folder->New; fi.Unread = folder->Unread; fi.Size = folder->Size;
   fwrite(&fi, sizeof(struct FIndex), 1, fh);

   for (mail = folder->Messages; mail; mail = mail->Next)
   {
      memset(&cmail, 0, sizeof(struct ComprMail));
      sprintf(buf, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
         mail->Subject,
         mail->From.Address, mail->From.RealName,
         mail->To.Address, mail->To.RealName,
         mail->ReplyTo.Address, mail->ReplyTo.RealName);

      strcpy(cmail.mailFile, mail->MailFile);
      cmail.date = mail->Date;
      cmail.transDate = mail->transDate;
      cmail.sflags = mail->sflags;
      cmail.mflags = mail->mflags;
      setVOLValue(&cmail, 0);  // we have to make sure that the volatile flag field isn`t saved
      cmail.cMsgID = mail->cMsgID;
      cmail.cIRTMsgID = mail->cIRTMsgID;
      cmail.size = mail->Size;
      cmail.moreBytes = strlen(buf);
      fwrite(&cmail, sizeof(struct ComprMail), 1, fh);
      fwrite(buf, 1, cmail.moreBytes, fh);
   }

   fclose(fh);
   CLEAR_FLAG(folder->Flags, FOFL_MODIFY);
   BusyEnd();

   return TRUE;
}

///
/// MA_GetIndex
//  Opens/unlocks a folder
BOOL MA_GetIndex(struct Folder *folder)
{
   if(!folder || folder->Type == FT_GROUP)
     return FALSE;

   if(folder->LoadedMode != LM_VALID)
   {
      if(isProtectedFolder(folder) && *folder->Password &&
         !MA_PromptFolderPassword(folder, G->MA->GUI.WI))
      {
        return FALSE;
      }

      // load the index file
      folder->LoadedMode = MA_LoadIndex(folder, TRUE);

      // we need to refresh the entry in the folder listtree this folder belongs to
      // but only if there is a GUI already!
      if(folder->LoadedMode == LM_VALID)
      {
        MA_ValidateStatus(folder);

        if(G->MA)
          DisplayStatistics(folder, FALSE);
      }
   }

   return (BOOL)(folder->LoadedMode == LM_VALID);
}

///
/// MA_ExpireIndex
//  Invalidates a folder index
void MA_ExpireIndex(struct Folder *folder)
{
   if(!isModified(folder)) DeleteFile(MA_IndexFileName(folder));

   SET_FLAG(folder->Flags, FOFL_MODIFY);
}

///
/// MA_UpdateIndexes
//  Updates indices of all folders
void MA_UpdateIndexes(BOOL initial)
{
   int i;
   struct Folder **flist;

   if((flist = FO_CreateList()))
   {
      for(i = 1; i <= (int)*flist; i++)
      {
        struct Folder *fo = flist[i];

        if(fo && fo->Type != FT_GROUP)
        {
          if(initial)
          {
            char *folderDir = GetFolderDir(fo);
            char *indexFile = MA_IndexFileName(fo);

            // get date of the folder directory and the .index file
            // itself
            long dirdate = FileTime(folderDir);
            long inddate = FileTime(indexFile);

            // only consider starting to rebuilding the .index if
            // either the date of the directory is greater than the
            // date of the .index file itself, or if there is no index
            // file date at all (no file present)
            if(dirdate > inddate+30)
            {
               // get the protection bits of the folder index file
               // and the folder directory, and if both have the A
               // bit set we skip the index rescanning process because
               // the A bits might have been set by a backup program
               if(isFlagClear(FileProtection(indexFile), FIBF_ARCHIVE) ||
                  isFlagClear(FileProtection(folderDir), FIBF_ARCHIVE))
               {
                  // lets first delete the .index file to
                  // make sure MA_GetIndex() is going to
                  // rebuild it.
                  if(inddate > 0)
                     DeleteFile(indexFile);

                  // then lets call GetIndex() to star rebuilding
                  // the .index
                  if(MA_GetIndex(fo) == TRUE)
                  {
                     // if we finally rebuilt the .index we
                     // immediatly flush it here so that another
                     // following index rebuild doesn't take
                     // all remaining memory.
                     if((fo->Type == FT_SENT   ||
                         fo->Type == FT_CUSTOM ||
                         fo->Type == FT_CUSTOMSENT) &&
                         fo->LoadedMode == LM_VALID &&
                         isFreeAccess(fo))
                     {
                        if(isModified(fo))
                          MA_SaveIndex(fo);

                        ClearMailList(fo, FALSE);
                        fo->LoadedMode = LM_FLUSHED;
                        CLEAR_FLAG(fo->Flags, FOFL_FREEXS);
                     }
                  }
               }
            }
          }
          else
          {
            if(fo->LoadedMode == LM_VALID && isModified(fo))
               MA_SaveIndex(fo);
          }
        }
      }

      free(flist);
   }
}

///
/// MA_FlushIndexes
//  Removes loaded folder indices from memory and closes folders
void MA_FlushIndexes(BOOL all)
{
   int i;
   struct Folder *fo, **flist, *actfo = FO_GetCurrentFolder();

   if(!actfo) return;

   if ((flist = FO_CreateList()))
   {
      for (i = 1; i <= (int)*flist; i++)
      {
         fo = flist[i];

         if ((fo->Type == FT_SENT || fo->Type == FT_CUSTOM || fo->Type == FT_CUSTOMSENT) && fo != actfo  && fo->LoadedMode == LM_VALID && (all || isFreeAccess(fo)))
         {
            if(isModified(fo)) MA_SaveIndex(fo);
            ClearMailList(fo, FALSE);
            fo->LoadedMode = LM_FLUSHED;
            CLEAR_FLAG(fo->Flags, FOFL_FREEXS);
         }
      }
      free(flist);
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_All, MUIF_NONE);
   }
}

HOOKPROTONHNONP(MA_FlushIndexFunc, void)
{
   MA_FlushIndexes(TRUE);
}
MakeHook(MA_FlushIndexHook, MA_FlushIndexFunc);
///

/*** Private functions ***/
/// MA_ChangeFolder
//  Changes to another folder
void MA_ChangeFolder(struct Folder *folder, BOOL set_active)
{
  struct Folder *actfo = FO_GetCurrentFolder();
  struct MA_GUIData *gui = &G->MA->GUI;

  if(!actfo)
    return;

  if(!folder)
    folder = actfo;
  else if(actfo == folder)
    return;
  else if(set_active)
    FO_SetCurrentFolder(folder);

  // in case the main window has a embedded read pane, we have to
  // clear it before changing the actual folder
  if(C->EmbeddedReadPane)
    DoMethod(gui->MN_EMBEDDEDREADPANE, MUIM_ReadMailGroup_Clear);

  // if this folder should be disabled, lets do it now
  if(folder->Type == FT_GROUP || MA_GetIndex(folder) == FALSE)
  {
    SetAttrs(gui->NL_MAILS, MUIA_Disabled, TRUE,
                            MUIA_ShortHelp, NULL,
                            TAG_DONE);

    DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);
  }
  else
  {
    // set the SortFlag in the NList accordingly
    MA_SetSortFlag();

    // Now we update the InfoBar accordingly
    DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);

    // Create the Mail List and display it
    DisplayMailList(folder, gui->NL_MAILS);

    // now we have to assure that the folder is enabled
    set(gui->NL_MAILS, MUIA_Disabled, FALSE);

    // Now we jump to messages that are NEW
    if(C->JumpToNewMsg)
      MA_JumpToNewMsg();
    else if(folder->LastActive >= 0)
      set(gui->NL_MAILS, MUIA_NList_Active, folder->LastActive);

    // if there is still no entry active in the NList we make the first one active
    if(xget(gui->NL_MAILS, MUIA_NList_Active) == (ULONG)MUIV_NList_Active_Off)
      set(gui->NL_MAILS, MUIA_NList_Active, MUIV_NList_Active_Top);
  }
}

HOOKPROTONHNONP(MA_ChangeFolderFunc, void)
{
   MA_ChangeFolder(NULL, FALSE);
}
MakeHook(MA_ChangeFolderHook, MA_ChangeFolderFunc);
///
/// MA_JumpToNewMsg
// Function that jumps to the first or last unread mail in a folder,
// depending on sort order of the folder
BOOL MA_JumpToNewMsg(VOID)
{
  struct Folder *folder = FO_GetCurrentFolder();
  int i, incr, pos = -1;

  if(folder->Sort[0] < 0 || folder->Sort[1] < 0)
  {
    i = xget(G->MA->GUI.NL_MAILS, MUIA_NList_Entries);
    i--;
    incr = -1;
  }
  else
  {
    i = 0;
    incr = 1;
  }

  while(1)
  {
    struct Mail *mail;
    DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, i, &mail);
    if(!mail) break;

    if(hasStatusNew(mail) || !hasStatusRead(mail))
    {
      pos = i;
      break;
    }

    i += incr;
  }

  set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, pos >= 0 ? pos : folder->LastActive);

  return TRUE;
}
///
/// MA_ConvertOldMailFile()
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

  // clear
  dateFilePart[0] = '\0';
  statusFilePart[0] = '\0';

  // construct the full path of the old filename
  // and get the file comment
  strncpy(oldFilePath, GetFolderDir(folder), SIZE_PATHFILE);
  oldFilePath[SIZE_PATHFILE] = '\0';

  if(AddPart(oldFilePath, filename, SIZE_PATHFILE) == 0 ||
     (comment = FileComment(oldFilePath)) == NULL)
  {
    return NULL;
  }

  // for a proper conversion we have to take mainly the file comment into
  // account as in there all status information aswell as the transfer
  // data was stored in YAM versions prior to 2.5
  if(*comment)
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
        *statusPartPtr++ = SCHAR_QUEUED;
      break;

      case 'H':
        *statusPartPtr++ = SCHAR_READ;
        *statusPartPtr++ = SCHAR_HOLD;
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
    }

    // now we check if second char is present and if we set the mailfag
    // as "marked" and the arexx permanent flag
    if(comment[1] && comment[1] != ' ')
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
    if(comment[1] && comment[2] && comment[14] == '\0')
    {
      // ok we have the transfer Date, so lets put it at the beginning of
      // the new filename
      strcpy(dateFilePart, &comment[2]);
    }
  }

  if(dateFilePart[0] == '\0')
  {
    struct timeval newDate;

    // so we don't seem to have a transfer Date in the file comment.
    // What we do now is that we take the date from the original file
    // (the first 5 numerical numbers are the days)
    newDate.tv_secs  = atol(filename) * 24 * 60 * 60;
    newDate.tv_micro = 0;

    // encode this date as a base64 encoded string
    base64encode(dateFilePart, (char *)&newDate, sizeof(struct timeval));
  }

  // as the dateFilePart may contain slashes "/" we have to replace them
  // with "-" chars to don't drive the filesystem crazy :)
  ptr = dateFilePart;
  while((ptr = strchr(ptr, '/')))
    *ptr = '-';

  do
  {
    static char newFileName[SIZE_MFILE];
    char newFilePath[SIZE_PATHFILE+1];

    // ok, now we should have all main parts of the new filename, so we
    // can concatenate it to one new filename and try to rename the old
    // style mailfile to the newstyle equivalent.
    sprintf(newFileName, "%s.001,%s", dateFilePart, statusFilePart);

    // so, now we should be finished with finding the new filename of the mail file.
    // lets try to rename it with the dos.library's Rename() function
    strncpy(newFilePath, GetFolderDir(folder), SIZE_PATHFILE);
    newFilePath[SIZE_PATHFILE] = '\0';

    if(AddPart(newFilePath, newFileName, SIZE_PATHFILE) == 0)
    {
      result = NULL;
      break;
    }

    // try to rename it and if it fails go and find out if we have to increase
    // the mail counter or not.
    if(Rename(oldFilePath, newFilePath) == 0)
    {
      BPTR dirLock;
      int mailCounter = 0;

      if((dirLock = Lock(GetFolderDir(folder), ACCESS_READ)))
      {
        struct ExAllControl *eac;

        if((eac = AllocDosObject(DOS_EXALLCONTROL, NULL)))
        {
          struct ExAllData *eabuffer;
          char matchPattern[15+1];
          char pattern[16*2+2];
          LONG more;

          eac->eac_LastKey = 0;
          eac->eac_MatchFunc = NULL;

          // search for files matching the dateFilePart
          sprintf(matchPattern, "%s.#?", dateFilePart);
          ParsePatternNoCase(matchPattern, pattern, 16*2+2);
          eac->eac_MatchString = pattern;

          if((eabuffer = malloc(SIZE_EXALLBUF)))
          {
            do
            {
              more = ExAll(dirLock, eabuffer, SIZE_EXALLBUF, ED_NAME, eac);
              if(!more && IoErr() != ERROR_NO_MORE_ENTRIES)
              {
                result = NULL;
                break;
              }

              mailCounter += eac->eac_Entries;
            }
            while(more);

            free(eabuffer);
          }
          else
          {
            DB(kprintf("  error on allocating enough buffers\n");)
            result = NULL;
          }

          FreeDosObject(DOS_EXALLCONTROL, eac);
        }
        else
        {
          DB(kprintf("  error on allocating dos object\n");)
          result = NULL;
        }

        UnLock(dirLock);
      }
      else
      {
        DB(kprintf("  error on getting folderdir lock\n");)
        result = NULL;
      }

      // if we didn't find any matching file then this is signals
      // another error than an already existing file!
      if(mailCounter == 0)
      {
        DB(kprintf("  error on renaming '%s' to '%s'\n", oldFilePath, newFilePath);)
        result = NULL;
        break;
      }

      // let us now try it again to rename the file
      sprintf(newFileName, "%s.%03d,%s", dateFilePart, ++mailCounter, statusFilePart);

      strncpy(newFilePath, GetFolderDir(folder), SIZE_PATHFILE);
      newFilePath[SIZE_PATHFILE] = '\0';

      if(AddPart(newFilePath, newFileName, SIZE_PATHFILE) == 0)
      {
        result = NULL;
        break;
      }

      // try to rename it and if it fails finally return an error
      if(Rename(oldFilePath, newFilePath) == 0)
      {
        DB(kprintf("  error on renaming '%s' to '%s'\n", oldFilePath, newFilePath);)
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

  return result != filename ? result : NULL;
}
///

/*** Mail header scanning ***/
/// MA_NewMailFile
//  Function that creates a new plain mail filename or by taking provided
//  data into account. It returns the full path to the new mail file in the
//  folder and also writes it into the mailfile parameter
char *MA_NewMailFile(struct Folder *folder, char *mailfile)
{
  static char fullpath[SIZE_PATHFILE+1];
  char dateFilePart[12+1];
  char newFileName[SIZE_MFILE];
  char *folderDir = GetFolderDir(folder);
  char *ptr;
  struct timeval curDate;
  int mCounter = 0;

  // take the current time and use it as the datePart of the
  // new mailfile name
  GetSysTimeUTC(&curDate);

  // encode this date as a base64 encoded string
  base64encode(dateFilePart, (char *)&curDate, sizeof(struct timeval));

  // as the dateFilePart may contain slashes "/" we have to replace them
  // with "-" chars to don't drive the filesystem crazy :)
  ptr = dateFilePart;
  while((ptr = strchr(ptr, '/')))
    *ptr = '-';

  do
  {
    sprintf(newFileName, "%s.%03d,N", dateFilePart, ++mCounter);

    if(mCounter > 999)
      return NULL;

    strcpy(fullpath, folderDir);
    AddPart(fullpath, newFileName, SIZE_PATHFILE);
  }
  while(FileExists(fullpath));

  // copy the newFileName to our mailfile buffer
  if(mailfile)
    strcpy(mailfile, newFileName);

  return fullpath;
}

///
/// MA_DetectUUE
//  Checks if message contains an uuencoded file
static BOOL MA_DetectUUE(FILE *fh)
{
   char *buffer;
   BOOL found = FALSE;

   buffer = calloc(SIZE_LINE,1);

   // Now we process the whole mailfile and check if there is any line that
   // starts with "begin xxx"
   while(GetLine(fh, buffer, SIZE_LINE))
   {
      // lets check for digit first because this will throw out many others first
      if(isdigit((int)buffer[6]) && strncmp(buffer, "begin ", 6) == 0)
      {
         found = TRUE;
         break;
      }
   }

   free(buffer);
   return found;
}

///
/// MA_ReadHeader
//  Reads header lines of a message into memory
BOOL MA_ReadHeader(FILE *fh, struct MinList *headerList)
{
  char *buffer;
  BOOL success = FALSE;
  int linesread = 0;

  if(headerList == NULL)
    return FALSE;

  // Allocate some memory for use as a read buffer
  if((buffer = calloc(SIZE_LINE, sizeof(char))))
  {
    char *ptr;
    BOOL finished = FALSE;
    struct HeaderNode *hdrNode = NULL;

    // clear the headerList first
    NewMinList(headerList);

    // we read out the whole header line by line and
    // concatenate lines that are belonging together.
    while((GetLine(fh, buffer, SIZE_LINE) && (++linesread, buffer[0])) ||
          (finished == FALSE && (finished = TRUE)))
    {
      // if the start of this line is a space or a tabulator sign
      // this line belongs to the last header also and we have to
      // add it to the last one.
      if((buffer[0] == ' ' || buffer[0] == '\t') && finished == FALSE)
      {
        if(hdrNode)
        {
          // move to the "real" start of the string so that we can copy
          // from there to our previous header.
          for(ptr = buffer; *ptr && isspace(*ptr); ptr++);

          // we want to preserve the last space so that this headerline
          // is correctly connected
          if(ptr != buffer)
            *(--ptr) = ' ';

          // now concatenate this new headerstring to our previous one
          hdrNode->content = StrBufCat(hdrNode->content, ptr);
        }
      }
      else
      {
        // it seems that we have found another header line because
        // it didn`t start with a linear-white-space, so lets
        // first validate the previous one, if it exists.
        if(hdrNode)
        {
          char *hdrContents = hdrNode->content;
          int len;

          // we first decode the header according to RFC 2047 which
          // should give us the full charset interpretation
          if((len = rfc2047_decode(hdrContents, hdrContents, strlen(hdrContents),
             (G->TTin && G->TTin->Header) ? G->TTin : NULL)) == -1)
          {
            DB(kprintf("ERROR: malloc() error during rfc2047() decoding\n");)
            break; // break-out
          }
          else if(len == -2)
          {
            DB(kprintf("WARNING: unknown header encoding found\n");)

            // signal an error but continue.
            ER_NewError(GetStr(MSG_ER_UnknownHeaderEnc), hdrContents, NULL);
          }
          else if(len == -3)
          {
            DB(kprintf("WARNING: base64 header decoding failed\n");)
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
          AddTail((struct List *)headerList, (struct Node *)hdrNode);
        }

        // if we are finished we break out here
        if(finished)
        {
          success = TRUE;
          break;
        }

        // now that we have finished the last header line
        // we can finally start processing a new one.
        // Which means we allocate a new HeaderNode and try to get out the header
        // name
        if((hdrNode = calloc(1, sizeof(struct HeaderNode))))
        {
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

          if(ptr && *ptr)
          {
            *ptr = '\0';

            // use our StrBufCpy() function to copy the name of the header
            // into our ->name element
            if((hdrNode->name = StrBufCpy(NULL, buffer)))
            {
              // now we copy also the rest of buffer into the contents
              // of the headerNode
              if((hdrNode->content = StrBufCpy(NULL, Trim(ptr+1))))
              {
                // everything seemed to work fine, so lets continue
                continue;
              }
            }
          }

          // if we end up here then something went wrong and we have to clear
          // the header Node and stuff
          free(hdrNode);
          hdrNode = NULL;
        }
        else
          break;
      }
    }

    free(buffer);
  }

  // if we haven't had success in reading the headers
  // we make sure we clean everything up
  if(!success)
    FreeHeaderList(headerList);

  return (BOOL)((success == TRUE && IsMinListEmpty(headerList) == FALSE) || linesread == 1);
}

///
/// MA_FreeEMailStruct
//  Frees an extended email structure
void MA_FreeEMailStruct(struct ExtendedMail *email)
{
   if(email)
   {
      FreeStrBuf(email->SenderInfo);
      FreeStrBuf(email->extraHeaders);
      if (email->NoSTo) free(email->STo);
      if (email->NoCC ) free(email->CC );
      if (email->NoBCC) free(email->BCC);
      free(email);
   }
}

///
/// MA_GetRecipients
//  Extracts recipients from a header field
static void MA_GetRecipients(char *h, struct Person **per, int *percnt)
{
   int cnt;
   char *p = h, *next;
   for (cnt = 0; *p;)
   {
      cnt++;
      if ((p = MyStrChr(p, ','))) p++; else break;
   }
   *percnt = cnt;
   if (cnt)
   {
      *per = calloc(cnt, sizeof(struct Person));
      for (cnt = 0, p = h; *p; cnt++)
      {
         if ((next = MyStrChr(p, ','))) *next++ = 0;
         ExtractAddress(p, (struct Person *)((ULONG)*per+cnt*sizeof(struct Person)));
         if (!(p = next)) break;
      }
   }
}

///
/// MA_ExamineMail
//  Parses the header lines of a message and fills email structure
struct ExtendedMail *MA_ExamineMail(struct Folder *folder, char *file, BOOL deep)
{
   struct ExtendedMail *email;
   static struct Person pe;
   struct MinList headerList;
   struct Mail *mail;
   char *p;
   char fullfile[SIZE_PATHFILE];
   int ok;
   BOOL dateFound = FALSE;
   FILE *fh;

   // first we generate a new ExtendedMail buffer
   if(!(email = calloc(1, sizeof(struct ExtendedMail))))
     return NULL;

   mail = &email->Mail;
   stccpy(mail->MailFile, file, SIZE_MFILE);
   email->DelSend = !C->SaveSent;
   if((fh = fopen(GetMailFile(fullfile, folder, mail), "r")))
   {
      // if the first three bytes are 'X' 'P' 'K', then this is an XPK packed
      // file and we have to unpack it first.
      if(fgetc(fh) == 'X' && fgetc(fh) == 'P' && fgetc(fh) == 'K')
      {
         // temporary close the file
         fclose(fh);

         // then unpack the file with XPK routines.
         if(!StartUnpack(GetMailFile(NULL, folder, mail), fullfile, folder))
         {
           free(email);
           return NULL;
         }

         // reopen it again.
         fh = fopen(fullfile, "r");
      }
      else rewind(fh); // rewind the file handle to the start
   }

   // check if the file handle is valid and the immediatly read in the
   // header lines
   if(fh && MA_ReadHeader(fh, &headerList))
   {
      char *ptr;
      char dateFilePart[12+1];
      char timebuf[sizeof(struct timeval)+1]; // +1 because the b64decode does set a NUL byte
      struct MinNode *curNode = headerList.mlh_Head;

      // Now we process the read header to set all flags accordingly
      for(ok=0; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
         struct HeaderNode *hdrNode = (struct HeaderNode *)curNode;
         char *field = hdrNode->name;
         char *value = hdrNode->content;

         if(!stricmp(field, "from"))
         {
           SET_FLAG(ok, 1);
           SParse(value);
           ExtractAddress(value, &pe);
           mail->From = pe;
         }
         else if(!stricmp(field, "reply-to"))
         {
           SET_FLAG(ok, 8);
           SParse(value);
           ExtractAddress(value, &pe);
           mail->ReplyTo = pe;
         }
         else if(!stricmp(field, "original-recipient"))
         {
           ExtractAddress(value, &pe);
           email->OriginalRcpt = pe;
         }
         else if(!stricmp(field, "disposition-notification-to"))
         {
           ExtractAddress(value, &pe);
           email->ReceiptTo = pe;
           email->ReceiptType = RCPT_TYPE_ALL;
           SET_FLAG(mail->mflags, MFLAG_SENDMDN);
         }
         else if(!stricmp(field, "return-view-to"))
         {
           ExtractAddress(value, &pe);
           email->ReceiptTo = pe;
           email->ReceiptType = RCPT_TYPE_READ;
         }
         else if(!stricmp(field, "return-receipt-to"))
         {
           email->RetRcpt = TRUE;
         }
         else if(!stricmp(field, "to"))
         {
           if(!(ok & 2))
           {
             SET_FLAG(ok, 2);
             SParse(value);
             if((p = MyStrChr(value, ',')))
               *p++ = 0;

             ExtractAddress(value, &pe);
             mail->To = pe;
             if(p)
             {
               if(deep)
               {
                 if(email->NoSTo == 0)
                   MA_GetRecipients(p, &(email->STo), &(email->NoSTo));

                 if(email->NoSTo > 0)
                   SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
               }
               else if(strlen(p) >= 7) // minimum rcpts size "a@bc.de"
                 SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
             }
           }
         }
         else if(!stricmp(field, "cc"))
         {
           if(deep)
           {
             if(email->NoCC == 0)
             {
               SParse(value);
               MA_GetRecipients(value, &(email->CC), &(email->NoCC));
             }

             if(email->NoCC > 0)
               SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
           }
           else if(strlen(value) >= 7) // minimum rcpts size "a@bc.de"
             SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
         }
         else if(!stricmp(field, "bcc"))
         {
           if(deep)
           {
             if(email->NoBCC == 0)
             {
               SParse(value);
               MA_GetRecipients(value, &(email->BCC), &(email->NoBCC));
             }

             if(email->NoBCC > 0)
               SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
           }
           else if(strlen(value) >= 7) // minimum rcpts size "a@bc.de"
             SET_FLAG(mail->mflags, MFLAG_MULTIRCPT);
         }
         else if(!stricmp(field, "subject"))
         {
           SET_FLAG(ok, 4);
           SParse(value);
           stccpy(mail->Subject, Trim(value), SIZE_SUBJECT);
         }
         else if(!stricmp(field, "message-id"))
         {
           mail->cMsgID = CompressMsgID(p = Trim(value));
           stccpy(email->MsgID, p, SIZE_MSGID);
         }
         else if(!stricmp(field, "in-reply-to"))
         {
           mail->cIRTMsgID = CompressMsgID(p = Trim(value));
           stccpy(email->IRTMsgID, p, SIZE_MSGID);
         }
         else if(!stricmp(field, "date"))
         {
           dateFound = MA_ScanDate(mail, value);
         }
         else if(!stricmp(field, "importance"))
         {
           if(getImportanceLevel(mail) == IMP_NORMAL)
           {
             p = Trim(value);
             if(!stricmp(p, "high"))
               setImportanceLevel(mail, IMP_HIGH);
             else if(!stricmp(p, "low"))
               setImportanceLevel(mail, IMP_LOW);
           }
         }
         else if(!stricmp(field, "priority"))
         {
           if(getImportanceLevel(mail) == IMP_NORMAL)
           {
             p = Trim(value);
             if(!stricmp(p, "urgent"))
               setImportanceLevel(mail, IMP_HIGH);
             else if(!stricmp(p, "non-urgent"))
               setImportanceLevel(mail, IMP_HIGH);
           }
         }
         else if(!stricmp(field, "content-type"))
         {
           p = Trim(value);
           if(!strnicmp(p, "multipart", 9))
           {
             p += 10;

             if(!strnicmp(p, "mixed", 5))
               SET_FLAG(mail->mflags, MFLAG_MP_MIXED);
             else if(!strnicmp(p, "report", 6))
               SET_FLAG(mail->mflags, MFLAG_MP_REPORT);
             else if(!strnicmp(p, "encrypted", 9))
               SET_FLAG(mail->mflags, MFLAG_MP_CRYPT);
             else if(!strnicmp(p, "signed", 6))
               SET_FLAG(mail->mflags, MFLAG_MP_SIGNED);
             else if(!strnicmp(p, "alternative", 11))
               SET_FLAG(mail->mflags, MFLAG_MP_ALTERN);
           }
         }
         else if(!stricmp(field, "x-senderinfo"))
         {
            SET_FLAG(mail->mflags, MFLAG_SENDERINFO);
            SParse(value);
            if(deep)
              email->SenderInfo = StrBufCpy(email->SenderInfo, value);
         }
         else if(deep) // and if we end up here we check if we really have to go further
         {
           if(!stricmp(field, "x-yam-options"))
           {
             enum Security sec;

             if(strstr(value, "delsent"))
               email->DelSend = TRUE;

             if((p = strstr(value, "sigfile")))
               email->Signature = p[7]-'0'+1;

             for(sec = SEC_SIGN; sec <= SEC_SENDANON; sec++)
             {
               if(strstr(value, SecCodes[sec]))
                 email->Security = sec;
             }
           }
           else if(!strnicmp(field, "x-yam-header-", 13))
           {
             email->extraHeaders = StrBufCat(StrBufCat(email->extraHeaders, &field[13]), ":");
             email->extraHeaders = StrBufCat(StrBufCat(email->extraHeaders, value), "\\n");
           }
         }

      }

      // if now the mail is still not MULTIPART we have to check for uuencoded attachments
      if(!isMP_MixedMail(mail) && MA_DetectUUE(fh))
        SET_FLAG(mail->mflags, MFLAG_MP_MIXED);

      // And now we close the Mailfile and clear the temporary headerList again
      fclose(fh);
      FreeHeaderList(&headerList);

      if((ok & 8) && !mail->ReplyTo.RealName[0] && !stricmp(mail->ReplyTo.Address, mail->From.Address))
        strcpy(mail->ReplyTo.RealName, mail->From.RealName);


      // if this function call has a folder of NULL then we are examining a virtual mail
      // which means this mail doesn't have any folder and also no filename that may contain
      // any usable date or stuff
      if(folder != NULL)
      {
        // now we take the filename of our mailfile into account to check for
        // the transfer date at the start of the name and for the set status
        // flags at the end of it.
        strncpy(dateFilePart, mail->MailFile, 12);
        dateFilePart[12] = '\0';

        // make sure there is no "-" in the base64 encoded part as we just mapped
        // the not allowed "/" to "-" to make it possible to use base64 for
        // the timeval encoding
        ptr = dateFilePart;
        while((ptr = strchr(ptr, '-')))
          *ptr = '/';

        // lets decode the base64 encoded timestring in a temporary buffer
        if(base64decode(timebuf, dateFilePart, 12) <= 0)
        {
          DB(kprintf("WARNING: failure in decoding the encoded date from mailfile: '%s'\n", mail->MailFile);)

          // if we weren`t able to decode the base64 encoded string
          // we have to validate the transDate so that the calling function
          // recognizes to rewrite the comment with a valid string.
          mail->transDate.tv_micro = 0;
          mail->transDate.tv_secs  = 0;
        }
        else
        {
          // everything seems to have worked so lets copy the binary data in our
          // transDate structure
          memcpy(&mail->transDate, timebuf, sizeof(struct timeval));
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
                SET_FLAG(mail->sflags, SFLAG_READ);
              break;

              case SCHAR_REPLIED:
                SET_FLAG(mail->sflags, SFLAG_REPLIED);
              break;

              case SCHAR_FORWARDED:
                SET_FLAG(mail->sflags, SFLAG_FORWARDED);
              break;

              case SCHAR_NEW:
                SET_FLAG(mail->sflags, SFLAG_NEW);
              break;

              case SCHAR_QUEUED:
                SET_FLAG(mail->sflags, SFLAG_QUEUED);
              break;

              case SCHAR_HOLD:
                SET_FLAG(mail->sflags, SFLAG_HOLD);
              break;

              case SCHAR_SENT:
                SET_FLAG(mail->sflags, SFLAG_SENT);
              break;

              case SCHAR_DELETED:
                SET_FLAG(mail->sflags, SFLAG_DELETED);
              break;

              case SCHAR_MARKED:
                SET_FLAG(mail->sflags, SFLAG_MARKED);
              break;

              case SCHAR_ERROR:
                SET_FLAG(mail->sflags, SFLAG_ERROR);
              break;
            }
          }

          ptr++;
        }
      }

      // if we didn't find a Date: header we take the transfered date (if found)
      if(dateFound == FALSE)
      {
        if(mail->transDate.tv_secs > 0)
        {
          // convert the UTC transDate to a UTC mail Date
          TimeVal2DateStamp(&mail->transDate, &mail->Date, TZC_NONE);
        }
        else
        {
          BPTR lock;

          // and as a fallback we take the date of the mail file
          if((lock = Lock(mail->MailFile, ACCESS_READ)))
          {
            struct FileInfoBlock *fib;

            if((fib = AllocDosObject(DOS_FIB, NULL)))
            {
              if(Examine(lock, fib))
              {
                memcpy(&mail->Date, &fib->fib_Date, sizeof(struct DateStamp));
                DateStampTZConvert(&mail->Date, TZC_UTC);
              }

              FreeDosObject(DOS_FIB, fib);
            }

            UnLock(lock);
          }
        }

        // set the timeZone to our local one
        mail->tzone = C->TimeZone;
      }

      // lets calculate the mailSize out of the FileSize() function
      mail->Size = FileSize(fullfile);

      FinishUnpack(fullfile);

      return email;
   }

   FinishUnpack(fullfile);

   // finish up everything before we exit with an error
   fclose(fh);
   free(email);
   return NULL;
}

///
/// MA_ScanMailBox
//  Scans for message files in a folder directory
BOOL MA_ScanMailBox(struct Folder *folder)
{
  struct ExtendedMail *email;
  BPTR dirLock;
  long filecount = FileCount(GetFolderDir(folder));
  long processedFiles = 0;
  BOOL result = TRUE;

  if(filecount < 1)
    return FALSE;

  BusyGauge(GetStr(MSG_BusyScanning), folder->Name, filecount);
  ClearMailList(folder, TRUE);

  DB(kprintf("Recanning index for folder: %s...\n", folder->Name);)

  if((dirLock = Lock(GetFolderDir(folder), ACCESS_READ)))
  {
    struct ExAllControl *eac;

    if((eac = AllocDosObject(DOS_EXALLCONTROL, NULL)))
    {
      struct ExAllData *ead;
      struct ExAllData *eabuffer;
      LONG more;
      eac->eac_LastKey = 0;
      eac->eac_MatchString = NULL;
      eac->eac_MatchFunc = NULL;

      if((eabuffer = malloc(SIZE_EXALLBUF)))
      {
        BOOL convertAllOld = FALSE;
        BOOL skipAllOld = FALSE;
        BOOL convertAllUnknown = FALSE;
        BOOL skipAllUnknown = FALSE;

        do
        {
          more = ExAll(dirLock, eabuffer, SIZE_EXALLBUF, ED_SIZE, eac);
          if(!more && IoErr() != ERROR_NO_MORE_ENTRIES)
          {
            result = FALSE;
            break;
          }

          if(eac->eac_Entries == 0)
            continue;

          ead = (struct ExAllData *)eabuffer;

          do
          {
            // set the gauge
            BusySet(++processedFiles);

            // give the GUI the chance to refresh
            DoMethod(G->App,MUIM_Application_InputBuffered);

            // then check wheter this is a file as we don't care for subdirectories
            if(isFile(ead->ed_Type))
            {
              // check wheter the filename is a valid mailfilename
              char fbuf[SIZE_PATHFILE+1];
              char *fname = ead->ed_Name;
              BOOL validMailFile = isValidMailFile(fname);

              if(validMailFile == FALSE)
              {
                // ok, the file doesn't seem to have to be a valid mailfilename, so
                // lets see if it is an "old" file (<= YAM2.4) or just trash
                int i=0;
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
                if(oldFound)
                {
                  int res;
                  BOOL convertOnce = FALSE;

                  // ok we seem to have found an old-fashioned mailfile, so lets
                  // convert it to the newstyle
                  DB(kprintf("  found < v2.5 style mailfile: %s\n", fname);)

                  // lets ask if the user wants to convert the file or not
                  if(!convertAllOld && !skipAllOld)
                  {
                    res = MUI_Request(G->App, NULL, 0,
                                      GetStr(MSG_MA_CREQ_OLDFILE_TITLE),
                                      GetStr(MSG_MA_YESNOTOALL),
                                      GetStr(MSG_MA_CREQUEST_OLDFILE),
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

                  if(convertAllOld || convertOnce)
                  {
                    // now we finally convert the file to a new style mail file
                    if((fname = MA_ConvertOldMailFile(fname, folder)) == NULL)
                    {
                      // if there occurred any error we skip to the next file.
                      ER_NewError(GetStr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                      continue;
                    }
                  }
                  else continue;
                }
                else if(fname[0] != '.')
                {
                  // to make it as convienent as possible for a user we also allow
                  // to copy mail files to a folder directory without having to take
                  // care that they have the correct filename.
                  int res;
                  BOOL convertOnce = FALSE;

                  DB(kprintf("  found unknown file: %s\n", fname);)

                  // lets ask if the user wants to convert the file or not
                  if(!convertAllUnknown && !skipAllUnknown)
                  {
                    res = MUI_Request(G->App, NULL, 0,
                                      GetStr(MSG_MA_CREQ_UNKNOWN_TITLE),
                                      GetStr(MSG_MA_YESNOTOALL),
                                      GetStr(MSG_MA_CREQUEST_UNKNOWN),
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

                  if(convertAllUnknown || convertOnce)
                  {
                    // now it is our job to get a new mailfile name and replace the old one
                    char oldfile[SIZE_PATHFILE+1];
                    char *newfile;

                    strncpy(oldfile, GetFolderDir(folder), SIZE_PATHFILE);
                    oldfile[SIZE_PATHFILE] = '\0';
                    AddPart(oldfile, fname, SIZE_PATHFILE);

                    if((newfile = MA_NewMailFile(folder, fbuf)))
                    {
                      if(Rename(oldfile, newfile))
                      {
                        fname = fbuf;
                      }
                      else
                      {
                        // if there occurred any error we skip to the next file.
                        ER_NewError(GetStr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                        continue;
                      }
                    }
                    else
                    {
                      // if there occurred any error we skip to the next file.
                      ER_NewError(GetStr(MSG_ER_CONVERTMFILE), fname, folder->Name);
                      continue;
                    }
                  }
                  else continue;
                }
                else continue;
              }

              if(ead->ed_Size)
              {
                DB(kprintf("  examining MailFile: %s\n", fname);)

                if((email = MA_ExamineMail(folder, fname, FALSE)))
                {
                  struct Mail *newMail = AddMailToList(&email->Mail, folder);

                  // if this new mail hasn`t got a valid transDate we have to check if we
                  // have to take the fileDate as a fallback value.
                  if(newMail->transDate.tv_secs == 0)
                  {
                    // only if it is _not_ a "waitforsend" and "hold" message we can take the fib_Date
                    // as the fallback
                    if(!hasStatusQueued(newMail) && !hasStatusHold(newMail))
                    {
                       DB(kprintf("    no transfer Date information found in mail file, taking fileDate...\n");)

                       // now convert the local TZ fib_Date to a UTC transDate
                       DateStamp2TimeVal(FileDate(fname), &newMail->transDate, TZC_UTC);

                       // then we update the mailfilename
                       MA_UpdateMailFile(newMail);
                    }
                  }

                  MA_FreeEMailStruct(email);
                }
              }
              else
              {
                char path[SIZE_PATHFILE+1];
                strncpy(path, GetFolderDir(folder), SIZE_PATHFILE);
                path[SIZE_PATHFILE] = '\0';
                AddPart(path, fname, SIZE_PATHFILE);
                DeleteFile(path);
              }
            }
          }
          while((ead = ead->ed_Next));
        }
        while(more);

        free(eabuffer);
      }
      else result = FALSE;

      FreeDosObject(DOS_EXALLCONTROL, eac);
    }
    else result = FALSE;

    UnLock(dirLock);
  }
  else result = FALSE;

  BusyEnd();
  return result;
}
///
/// MA_ScanDate
//  Converts textual date header into datestamp format
static BOOL MA_ScanDate(struct Mail *mail, const char *date)
{
   int count = 0, day = 0, mon = 0, year = 0, hour = 0, min = 0, sec = 0;
   char *tzone = "", *p, tdate[SIZE_SMALL], ttime[SIZE_SMALL];
   struct DateTime dt;
   struct DateStamp *ds = &dt.dat_Stamp;

   if((p = strchr(date, ',')))
     p++;
   else
     p = (char *)date;

   while(*p && isspace(*p))
     p++;

   while((p = strtok(p, " \t")))
   {
      switch(count)
      {
         // get the day
         case 0:
         {
            if(!isdigit(*p) || (day = atoi(p)) > 31)
              return FALSE;
         }
         break;

         // get the month
         case 1:
         {
            for(mon = 1; mon <= 12; mon++)
            {
              if(strnicmp(p, months[mon-1], 3) == 0) break;
            }

            if(mon > 12)
              return FALSE;
         }
         break;

         // get the year
         case 2:
         {
            year = atoi(p);
         }
         break;

         // get the time values
         case 3:
         {
            if(sscanf(p, "%d:%d:%d", &hour, &min, &sec) != 3)
            {
              if(sscanf (p, "%d:%d", &hour, &min) == 2)
                sec = 0;
              else
                return FALSE;
            }
         }
         break;

         // get the time zone
         case 4:
         {
            while (*p && (isspace(*p) || *p == '(')) p++;
            tzone = p;
         }
         break;
      }

      // if we iterated until 4 we can break out
      if(count == 4) break;

      count++;
      p = NULL;
   }

   // then format a standard DateStamp string like string
   // so that we can use StrToDate()
   sprintf(tdate, "%02d-%02d-%02d", mon, day, year%100);
   sprintf(ttime, "%02d:%02d:%02d", hour, min, sec);
   dt.dat_Format  = FORMAT_USA;
   dt.dat_Flags   = 0;
   dt.dat_StrDate = (STRPTR)tdate;
   dt.dat_StrTime = (STRPTR)ttime;
   if(!StrToDate(&dt))
     return FALSE;

   // save the timezone
   mail->tzone = TZtoMinutes(tzone);

   // bring the date in relation to UTC
   ds->ds_Minute -= mail->tzone;

   // we need to check the datestamp variable that it is still in it`s borders
   // after the UTC correction
   while(ds->ds_Minute < 0)     { ds->ds_Minute += 1440; ds->ds_Days--; }
   while(ds->ds_Minute >= 1440) { ds->ds_Minute -= 1440; ds->ds_Days++; }

   // now we do copy the datestamp stuff over the one from our mail
   memcpy(&mail->Date, ds, sizeof(struct DateStamp));

   return TRUE;
}
///

/*** Hooks ***/
/// PO_InitFolderList
//  Creates a popup list of all folders
HOOKPROTONHNP(PO_InitFolderList, long, Object *pop)
{
   int i;
   struct Folder **flist;

   DoMethod(pop, MUIM_List_Clear);
   DoMethod(pop, MUIM_List_InsertSingle, GetStr(MSG_MA_Cancel), MUIV_List_Insert_Bottom);
   if ((flist = FO_CreateList()))
   {
      for (i = 1; i <= (int)*flist; i++) if (flist[i]->Type != FT_GROUP)
         DoMethod(pop, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
      free(flist);
   }
   return TRUE;
}
MakeHook(PO_InitFolderListHook, PO_InitFolderList);

///
/// MA_LV_FDspFunc
//  Folder listview display hook
HOOKPROTONHNO(MA_LV_FDspFunc, long, struct MUIP_NListtree_DisplayMessage *msg)
{
   if(msg == NULL)
    return 0;

   if(msg->TreeNode != NULL)
   {
      static char dispfold[SIZE_DEFAULT], disptot[SIZE_SMALL], dispunr[SIZE_SMALL], dispnew[SIZE_SMALL], dispsiz[SIZE_SMALL];

      struct Folder *entry = (struct Folder *)msg->TreeNode->tn_User;

      msg->Array[0] = msg->Array[1] = msg->Array[2] = msg->Array[3] = msg->Array[4] = "";
      *dispsiz = 0;

      switch(entry->Type)
      {
        case FT_GROUP:
        {
          sprintf(msg->Array[0] = dispfold, "\033o[%d] %s", (isFlagSet(msg->TreeNode->tn_Flags, TNF_OPEN) ? 1 : 0), entry->Name);
          msg->Preparse[0] = (entry->New+entry->Unread) ? (MUIX_B MUIX_I ) : MUIX_B MUIX_I "\0334";
        }
        break;

        default:
        {
          if(entry->ImageIndex >= 0)
            sprintf(msg->Array[0] = dispfold, "\033o[%d] ", entry->ImageIndex);
          else
            strcpy(msg->Array[0] = dispfold, " ");

          if(entry->Name[0])
            strcat(dispfold, entry->Name);
          else
            sprintf(dispfold, "[%s]", FilePart(entry->Path));

          if(entry->LoadedMode != LM_UNLOAD)
          {
            if(entry->New)
            {
              msg->Preparse[0] = MUIX_B;
              if((C->FolderCols & (1<<3)) == 0)
                sprintf(dispfold, "%s (%d)", dispfold, entry->Unread);
            }
            else if(entry->Unread)
            {
              msg->Preparse[0] = MUIX_B "\0334";
              if((C->FolderCols & (1<<2)) == 0)
                sprintf(dispfold, "%s (%d)", dispfold, entry->Unread);
            }

            // if other folder columns are enabled lets fill the values
            // in
            if(C->FolderCols & (1<<1))
              sprintf(msg->Array[1] = disptot, "%d", entry->Total);

            if(C->FolderCols & (1<<2) && entry->Unread-entry->New > 0)
              sprintf(msg->Array[2] = dispunr, "%d", entry->Unread-entry->New);

            if(C->FolderCols & (1<<3) && entry->New > 0)
              sprintf(msg->Array[3] = dispnew, "%d", entry->New);

            if(C->FolderCols & (1<<4) && entry->Size > 0)
              FormatSize(entry->Size, msg->Array[4] = dispsiz);
          }
          else
            msg->Preparse[0] = MUIX_I;

          if(isProtectedFolder(entry))
            sprintf(dispfold, "%s \033o[%d]", dispfold, MAXBCFOLDERIMG);
        }
      }
   }
   else
   {
      msg->Array[0] = GetStr(MSG_Folder);
      msg->Array[1] = GetStr(MSG_Total);
      msg->Array[2] = GetStr(MSG_Unread);
      msg->Array[3] = GetStr(MSG_New);
      msg->Array[4] = GetStr(MSG_Size);
   }

   return 0;
}
MakeHook(MA_LV_FDspFuncHook,MA_LV_FDspFunc);

///
/// MA_MakeFOFormat
//  Creates format definition for folder listview
void MA_MakeFOFormat(Object *lv)
{
   static const int defwidth[FOCOLNUM] = { -1,-1,-1,-1,-1 };
   char format[SIZE_LARGE];
   BOOL first = TRUE;
   int i;

   *format = 0;
   for(i = 0; i < FOCOLNUM; i++)
   {
      if(C->FolderCols & (1<<i))
      {
          if(first)
            first = FALSE;
          else
            strcat(format, " BAR,");

          sprintf(&format[strlen(format)], "COL=%d W=%d", i, defwidth[i]);

          if(i > 0)
            strcat(format, " P=\033r");
      }
   }
   strcat(format, " BAR");

   set(lv, MUIA_NListtree_Format, format);
}
///

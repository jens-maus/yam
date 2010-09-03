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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__AROS__)
#include <sys/types.h>
#else
#include <sys/filio.h>
#endif

#include <netinet/ip.h>
#include <netinet/tcp.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <libraries/iffparse.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/amissl.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#if !defined(__amigaos4__) && !defined(__AROS__)
#include <proto/miami.h>
#include <proto/genesis.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/Classes.h"
#include "mime/base64.h"
#include "mime/md5.h"

#include "AppIcon.h"
#include "HashTable.h"
#include "FileInfo.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"
#include "UIDL.h"

#include "tcp/Connection.h"
#include "tcp/pop3.h"

#include "Debug.h"

/**************************************************************************/
// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o]) | (((unsigned char*)(p))[o+1]<<8) | (((unsigned char*)(p))[o+2]<<16) | (((unsigned char*)(p))[o+3]<<24))

/***************************************************************************
 Module: Transfer
***************************************************************************/

/*** General connecting/disconnecting & transfer ***/
/// TR_Disconnect
//  Terminates a connection
void TR_Disconnect(void)
{
  ENTER();

  D(DBF_NET, "disconnecting TCP/IP session...");

  DisconnectFromHost(G->TR->connection);

  LEAVE();
}
///
/// TR_SetWinTitle
//  Sets the title of the transfer window
void TR_SetWinTitle(BOOL from, const char *text)
{
  // compose the window title
  snprintf(G->TR->WTitle, sizeof(G->TR->WTitle), tr(from ? MSG_TR_MailTransferFrom : MSG_TR_MailTransferTo), text);

  // set the window title
  set(G->TR->GUI.WI, MUIA_Window_Title, G->TR->WTitle);
}
///
/// TR_DisplayMailList
//  Displays a list of messages ready for download
static void TR_DisplayMailList(BOOL largeonly)
{
  Object *lv = G->TR->GUI.LV_MAILS;
  struct Node *curNode;
  int pos=0;

  ENTER();

  set(lv, MUIA_NList_Quiet, TRUE);

  // search through our transferList
  IterateList(&G->TR->transferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
    #if defined(DEBUG)
    struct Mail *mail = mtn->mail;
    #endif

    D(DBF_GUI, "checking mail with flags %08lx and subject '%s'", mtn->tflags, mail->Subject);
    // only display mails to be downloaded
    if(hasTR_LOAD(mtn) || hasTR_PRESELECT(mtn))
    {
      // add this mail to the transfer list in case we either
      // should show ALL mails or the mail size is >= the warning size
      if(largeonly == FALSE || hasTR_PRESELECT(mtn))
      {
        mtn->position = pos++;

        DoMethod(lv, MUIM_NList_InsertSingle, mtn, MUIV_NList_Insert_Bottom);
        D(DBF_GUI, "added mail with subject '%s' and size %ld to preselection list", mail->Subject, mail->Size);
      }
      else
        D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
    }
    else
      D(DBF_GUI, "skipped mail with subject '%s' and size %ld", mail->Subject, mail->Size);
  }

  xset(lv, MUIA_NList_Active, MUIV_NList_Active_Top,
           MUIA_NList_Quiet, FALSE);

  LEAVE();
}
///
/// TR_ApplyRemoteFilters
//  Applies remote filters to a message
void TR_ApplyRemoteFilters(struct MailTransferNode *mtn)
{
  ENTER();

  // if there is no search count we can break out immediatly
  if(G->TR->SearchCount > 0)
  {
    struct Node *curNode;

    // Now we process the read header to set all flags accordingly
    IterateList(&C->filterList, curNode)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mtn->mail) == TRUE)
      {
        if(hasExecuteAction(filter) && *filter->executeCmd)
           LaunchCommand(filter->executeCmd, FALSE, OUT_STDOUT);

        if(hasPlaySoundAction(filter) && *filter->playSound)
           DoAction(TA_PlaySound, TT_PlaySound_Filename, filter->playSound, TAG_DONE);

        if(hasDeleteAction(filter))
           SET_FLAG(mtn->tflags, TRF_DELETE);
        else
           CLEAR_FLAG(mtn->tflags, TRF_DELETE);

        if(hasSkipMsgAction(filter))
           CLEAR_FLAG(mtn->tflags, TRF_LOAD);
        else
           SET_FLAG(mtn->tflags, TRF_LOAD);

        // get out of this loop after a successful search
        break;
      }
    }
  }

  LEAVE();
}
///
/// TR_ChangeTransFlagsFunc
//  Changes transfer flags of all selected messages
HOOKPROTONHNO(TR_ChangeTransFlagsFunc, void, int *arg)
{
  int id = MUIV_NList_NextSelected_Start;

  do
  {
    struct MailTransferNode *mtn;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_NextSelected, &id);
    if(id == MUIV_NList_NextSelected_End)
      break;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, id, &mtn);
    mtn->tflags = *arg;

    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Redraw, id);
  }
  while(TRUE);
}
MakeStaticHook(TR_ChangeTransFlagsHook, TR_ChangeTransFlagsFunc);
///
/// TR_TransStat_Init
//  Initializes transfer statistics
void TR_TransStat_Init(struct TransStat *ts)
{
  struct Node *curNode;

  ENTER();

  ts->Msgs_Tot = 0;
  ts->Size_Tot = 0;

  if(G->TR->GUI.GR_LIST != NULL)
  {
    set(G->TR->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
  }

  // search through our transferList
  IterateList(&G->TR->transferList, curNode)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    ts->Msgs_Tot++;

    if(hasTR_LOAD(mtn))
      ts->Size_Tot += mtn->mail->Size;
  }

  LEAVE();
}
///
/// TR_TransStat_Start
//  Resets statistics display
void TR_TransStat_Start(struct TransStat *ts)
{
  ENTER();

  ts->Msgs_Done = 0;
  ts->Size_Done = 0;

  // get the actual time we started the TransferStatus
  GetSysTime(TIMEVAL(&ts->Clock_Last));
  ts->Clock_Start = ts->Clock_Last.Seconds;

  memset(&ts->Clock_Last, 0, sizeof(ts->Clock_Last));

  snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), ts->Msgs_Tot);
  xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                            MUIA_Gauge_Max,      ts->Msgs_Tot,
                            MUIA_Gauge_Current,  0);

  LEAVE();
}
///
/// TR_TransStat_Finish
//  updates statistics display to represent the final state
void TR_TransStat_Finish(struct TransStat *ts)
{
  ENTER();

  // make sure we have valid strings to display
  FormatSize(ts->Size_Curr_Max, ts->str_size_curr_max, sizeof(ts->str_size_curr_max), SF_AUTO);

  // show the final statistics
  snprintf(G->TR->CountLabel, sizeof(G->TR->CountLabel), tr(MSG_TR_MESSAGEGAUGE), ts->Msgs_Tot);
  xset(G->TR->GUI.GA_COUNT, MUIA_Gauge_InfoText, G->TR->CountLabel,
                            MUIA_Gauge_Max,      ts->Msgs_Tot,
                            MUIA_Gauge_Current,  ts->Msgs_Tot);

  snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                         ts->str_size_curr_max, ts->str_size_curr_max);
  xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel,
                            MUIA_Gauge_Max,      100,
                            MUIA_Gauge_Current,  100);
  LEAVE();
}
///
/// TR_TransStat_NextMsg
//  Updates statistics display for next message
void TR_TransStat_NextMsg(struct TransStat *ts, int index, int listpos, LONG size, const char *status)
{
  ENTER();

  ts->Msgs_Curr = index;
  ts->Msgs_ListPos = listpos;
  ts->Msgs_Done++;
  ts->Size_Curr = 0;
  ts->Size_Curr_Max = size;

  // format the current mail's size ahead of any refresh
  FormatSize(size, ts->str_size_curr_max, sizeof(ts->str_size_curr_max), SF_AUTO);

  TR_TransStat_Update(ts, 0, status);

  LEAVE();
}
///
/// TR_TransStat_Update
//  Updates statistics display for next block of data
void TR_TransStat_Update(struct TransStat *ts, int size_incr, const char *status)
{
  ENTER();

  if(size_incr > 0)
  {
    ts->Size_Done += size_incr;
    ts->Size_Curr += size_incr;
  }
  else if(size_incr == TS_SETMAX)
  {
    // first update the total transferred size
    ts->Size_Done += ts->Size_Curr_Max - ts->Size_Curr;
    // we are done with this mail, so make sure the current size equals the final size
    ts->Size_Curr = ts->Size_Curr_Max;
  }

  // if the window isn't open we don't need to update it, do we?
  if(xget(G->TR->GUI.WI, MUIA_Window_Open) == TRUE)
  {
    // update the stats at most 4 times per second
    if(TimeHasElapsed(&ts->Clock_Last, 250000) == TRUE)
    {
      ULONG deltatime = ts->Clock_Last.Seconds - ts->Clock_Start;
      ULONG speed = 0;
      LONG remclock = 0;
      ULONG max;
      ULONG current;

      // if we have a preselection window, update it.
      if(G->TR->GUI.GR_LIST != NULL && ts->Msgs_ListPos >= 0)
        set(G->TR->GUI.LV_MAILS, MUIA_NList_Active, ts->Msgs_ListPos);

      // first we calculate the speed in bytes/sec
      // to display to the user
      if(deltatime != 0)
        speed = ts->Size_Done / deltatime;

      // calculate the estimated remaining time
      if(speed != 0 && ((remclock = (ts->Size_Tot / speed) - deltatime) < 0))
        remclock = 0;

      // show the current status
      set(G->TR->GUI.TX_STATUS, MUIA_Text_Contents, status);

      // show the current message index
      set(G->TR->GUI.GA_COUNT, MUIA_Gauge_Current, ts->Msgs_Curr);

      // format the size done and size total strings
      FormatSize(ts->Size_Done, ts->str_size_done, sizeof(ts->str_size_done), SF_MIXED);
      FormatSize(ts->Size_Tot, ts->str_size_tot, sizeof(ts->str_size_tot), SF_MIXED);
      FormatSize(ts->Size_Curr, ts->str_size_curr, sizeof(ts->str_size_curr), SF_AUTO);

      FormatSize(speed, ts->str_speed, sizeof(ts->str_speed), SF_MIXED);

      // now format the StatsLabel and update it
      snprintf(G->TR->StatsLabel, sizeof(G->TR->StatsLabel), tr(MSG_TR_TRANSFERSTATUS),
                                  ts->str_size_done, ts->str_size_tot, ts->str_speed,
                                  deltatime / 60, deltatime % 60,
                                  remclock / 60, remclock % 60);

      set(G->TR->GUI.TX_STATS, MUIA_Text_Contents, G->TR->StatsLabel);

      // update the gauge
      snprintf(G->TR->BytesLabel, sizeof(G->TR->BytesLabel), tr(MSG_TR_TRANSFERSIZE),
                                                             ts->str_size_curr, ts->str_size_curr_max);
      if(size_incr == TS_SETMAX)
      {
        max = 100;
        current = 100;
      }
      else if(ts->Size_Curr_Max <= 65536)
      {
        max = ts->Size_Curr_Max;
        current = ts->Size_Curr;
      }
      else
      {
        max = ts->Size_Curr_Max / 1024;
        current = ts->Size_Curr / 1024;
      }
      xset(G->TR->GUI.GA_BYTES, MUIA_Gauge_InfoText, G->TR->BytesLabel,
                                MUIA_Gauge_Max,      max,
                                MUIA_Gauge_Current,  current);

      // signal the application to update now
      DoMethod(G->App, MUIM_Application_InputBuffered);
    }
  }

  LEAVE();
}
///
/// TR_Cleanup
//  Free temporary message and UIDL lists
void TR_Cleanup(void)
{
  struct Node *curNode;

  ENTER();

  if(G->TR->GUI.LV_MAILS != NULL)
    DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_Clear);

  while((curNode = RemHead((struct List *)&G->TR->transferList)) != NULL)
  {
    struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

    // free the mail pointer
    free(mtn->mail);

    // free the UIDL
    free(mtn->UIDL);

    // free the node itself
    free(mtn);
  }

  NewList((struct List *)&G->TR->transferList);

  LEAVE();
}
///
/// TR_AbortnClose
//  Aborts a transfer
void TR_AbortnClose(void)
{
  ENTER();

  set(G->TR->GUI.WI, MUIA_Window_Open, FALSE);
  TR_Cleanup();
  MA_ChangeTransfer(TRUE);

  DeleteMailList(G->TR->downloadedMails);
  G->TR->downloadedMails = NULL;

  DisposeModulePush(&G->TR);

  LEAVE();
}
///
/// TR_ApplySentFilters
//  Applies filters to a sent message
BOOL TR_ApplySentFilters(struct Mail *mail)
{
  BOOL result = TRUE;

  ENTER();

  // only if we have a positiv search count we start
  // our filtering at all, otherwise we return immediatly
  if(G->TR->SearchCount > 0)
  {
    struct Node *curNode;

    // Now we process the read header to set all flags accordingly
    IterateList(&C->filterList, curNode)
    {
      struct FilterNode *filter = (struct FilterNode *)curNode;

      if(DoFilterSearch(filter, mail) == TRUE)
      {
        if(ExecuteFilterAction(filter, mail) == FALSE)
        {
          result = FALSE;
          break;
        }
      }
    }
  }

  RETURN(result);
  return result;
}
///

/*** EXPORT ***/
/// TR_ProcessEXPORT
//  Saves a list of messages to a MBOX mailbox file
BOOL TR_ProcessEXPORT(char *fname, struct MailList *mlist, BOOL append)
{
  BOOL success = FALSE;
  BOOL abort = FALSE;
  struct MailNode *mnode;
  int i;

  ENTER();

  // reset our processing list
  NewList((struct List *)&G->TR->transferList);

  // temporarly copy all data out of our mlist to the
  // processing list and mark all mails to get "loaded"
  i = 0;

  LockMailListShared(mlist);

  ForEachMailNode(mlist, mnode)
  {
    struct Mail *mail = mnode->mail;

    if(mail != NULL)
    {
      struct MailTransferNode *mtn;

      if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
      {
        if((mtn->mail = memdup(mail, sizeof(struct Mail))) != NULL)
        {
          mtn->index = i + 1;

          // set to LOAD
          mtn->tflags = TRF_LOAD;

          AddTail((struct List *)&(G->TR->transferList), (struct Node *)mtn);
        }
        else
        {
          // we end up in a low memory condition, lets exit
          // after having freed everything
          free(mtn);
          abort = TRUE;
          break;
        }
      }
      else
      {
        // we end up in a low memory condition, lets exit
        abort = TRUE;
        break;
      }
    }

    i++;
  }

  UnlockMailList(mlist);

  // if we have now something in our processing list,
  // lets go on
  if(abort == FALSE &&
     IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    FILE *fh;
    struct TransStat ts;

    TR_SetWinTitle(FALSE, (char *)FilePart(fname));
    TR_TransStat_Init(&ts);
    TR_TransStat_Start(&ts);

    // open our final destination file either in append or in a fresh
    // write mode.
    if((fh = fopen(fname, append ? "a" : "w")) != NULL)
    {
      struct Node *curNode;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      success = TRUE;

      IterateList(&G->TR->transferList, curNode)
      {
        struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
        struct Mail *mail = mtn->mail;
        char fullfile[SIZE_PATHFILE];

        // update the transfer status
        TR_TransStat_NextMsg(&ts, mtn->index, -1, mail->Size, tr(MSG_TR_Exporting));

        if(StartUnpack(GetMailFile(NULL, NULL, mail), fullfile, mail->Folder) != NULL)
        {
          FILE *mfh;

          // open the message file to start exporting it
          if((mfh = fopen(fullfile, "r")) != NULL)
          {
            char datstr[64];
            char *buf = NULL;
            size_t buflen = 0;
            ssize_t curlen;
            BOOL inHeader = TRUE;

            // printf out our leading "From " MBOX format line first
            DateStamp2String(datstr, sizeof(datstr), &mail->Date, DSS_UNIXDATE, TZC_NONE);
            fprintf(fh, "From %s %s", mail->From.Address, datstr);

            // let us put out the Status: header field
            fprintf(fh, "Status: %s\n", MA_ToStatusHeader(mail));

            // let us put out the X-Status: header field
            fprintf(fh, "X-Status: %s\n", MA_ToXStatusHeader(mail));

            // now we iterate through every line of our mail and try to substitute
            // found "From " line with quoted ones
            while(G->TR->Abort == FALSE &&
                  (curlen = getline(&buf, &buflen, mfh)) > 0)
            {
              char *tmp = buf;

              // check if this is a single \n so that it
              // signals the end if a line
              if(buf[0] == '\n' || (buf[0] == '\r' && buf[1] == '\n'))
              {
                inHeader = FALSE;

                if(fwrite(buf, curlen, 1, fh) != 1)
                {
                  // write error, bail out
                  break;
                }

                continue;
              }

              // the mboxrd format specifies that we need to quote any
              // From, >From, >>From etc. occurance.
              // http://www.qmail.org/man/man5/mbox.html
              while(*tmp == '>')
                tmp++;

              if(strncmp(tmp, "From ", 5) == 0)
              {
                if(fputc('>', fh) == EOF)
                {
                  // write error, bail out
                  break;
                }
              }
              else if(inHeader == TRUE)
              {
                // let us skip some specific headerlines
                // because we placed our own here
                if(strncmp(buf, "Status: ", 8) == 0 ||
                   strncmp(buf, "X-Status: ", 10) == 0)
                {
                  // skip line
                  continue;
                }
              }

              // write the line to our destination file
              if(fwrite(buf, curlen, 1, fh) != 1)
              {
                // write error, bail out
                break;
              }

              // make sure we have a newline at the end of the line
              if(buf[curlen-1] != '\n')
              {
                if(fputc('\n', fh) == EOF)
                {
                  // write error, bail out
                  break;
                }
              }

              // update the transfer status
              TR_TransStat_Update(&ts, curlen, tr(MSG_TR_Exporting));
            }

            // check why we exited the while() loop and if everything is fine
            if(ferror(fh) != 0)
            {
              E(DBF_NET, "error on writing data! ferror(fh)=%ld", ferror(fh));

              // an error occurred, lets return failure
              success = FALSE;
            }
            else if(ferror(mfh) != 0 || feof(mfh) == 0)
            {
              E(DBF_NET, "error on reading data! ferror(mfh)=%ld feof(mfh)=%ld", ferror(mfh), feof(mfh));

              // an error occurred, lets return failure
              success = FALSE;
            }

            // close file pointer
            fclose(mfh);

            free(buf);

            // put the transferStat to 100%
            TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Exporting));
          }
          else
           success = FALSE;

          FinishUnpack(fullfile);
        }
        else
          success = FALSE;

        if(G->TR->Abort == TRUE || success == FALSE)
          break;
      }

      // close file pointer
      fclose(fh);

      // write the status to our logfile
      mnode = FirstMailNode(mlist);
      AppendToLogfile(LF_ALL, 51, tr(MSG_LOG_Exporting), ts.Msgs_Done, mnode->mail->Folder->Name, fname);
    }

    TR_TransStat_Finish(&ts);
  }

  TR_AbortnClose();

  RETURN(success);
  return success;
}
///

/*** IMPORT ***/
/// TR_AddMessageHeader
//  Parses downloaded message header
static struct MailTransferNode *TR_AddMessageHeader(int *count, int size, long addr, char *tfname)
{
  struct MailTransferNode *ret = NULL;
  struct ExtendedMail *email;

  ENTER();

  if((email = MA_ExamineMail(NULL, tfname, FALSE)) != NULL)
  {
    struct MailTransferNode *mtn;

    if((mtn = calloc(1, sizeof(struct MailTransferNode))) != NULL)
    {
      struct Mail *mail;

      if((mail = memdup(&email->Mail, sizeof(struct Mail))) != NULL)
      {
        mail->Folder  = NULL;
        mail->Size    = size;

        // flag the mail as being transfered
        mtn->index      = ++(*count);
        mtn->tflags     = TRF_LOAD;
        mtn->mail       = mail;
        mtn->importAddr = addr;

        AddTail((struct List *)&G->TR->transferList, (struct Node *)mtn);

        D(DBF_IMPORT, "added mail '%s' (%ld bytes) to import list.", mail->Subject, size);

        ret = mtn;
      }
      else
      {
        free(mtn);
        E(DBF_IMPORT, "Couldn't allocate enough memory for struct Mail");
      }
    }
    else
      E(DBF_IMPORT, "Couldn't allocate enough memory for struct MailTransferNode");

    MA_FreeEMailStruct(email);
  }
  else
    E(DBF_IMPORT, "MA_ExamineMail() returned an error!");

  RETURN(ret);
  return ret;
}

///
/// ReadDBXMessage()
// Extract a certain message from a dbx (Outlook Express) file into
// a separate output file.
static BOOL ReadDBXMessage(FILE *fh, FILE *out, unsigned int addr)
{
  // This can be static as this function is the leave of the recursion
  static unsigned char buf[0x210];
  unsigned int size;
  BOOL result = TRUE;

  ENTER();

  D(DBF_IMPORT, "reading message from addr 0x%08lx", addr);

  while(addr)
  {
    unsigned char *pFirst;
    unsigned char *pLast;
    unsigned char *pCur;
    unsigned char *pPrev;
    unsigned int writeSize;

    // seek to the actual message position
    if(fseek(fh, addr, SEEK_SET))
    {
      result = FALSE;
      break;
    }

    // read out the whole message at once
    if(fread(buf, 1, sizeof(buf), fh) != sizeof(buf))
    {
      result = FALSE;
      break;
    }

    // get the size of the read part and the addr
    // of the next part of the message
    size = writeSize = GetLong(buf, 8);
    addr = GetLong(buf, 12);

    // as *.dbx files are created under Windows they
    // may carry "\r\n" return sequences. But as we are
    // on amiga we do not need and want them, that's why
    // we strip them off
    pFirst = pPrev = &buf[16]; // start of the message part
    pLast = pFirst+size;       // end of the message part

    // we search for '\r' chars and strip them
    for(pCur = pPrev; pCur < pLast; pCur++)
    {
      if(*pCur != '\r')
      {
        if(pCur != pPrev)
          *pPrev = *pCur;

        pPrev++;
      }
      else
        writeSize--;
    }

    // write out the message part at one
    if(writeSize > 0)
    {
      if(fwrite(pFirst, 1, writeSize, out) != writeSize)
      {
        result = FALSE;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// ReadDBXMessageInfo()
// reads out the message info of a dbx (Outlook Express) Mail Archive file
static BOOL ReadDBXMessageInfo(FILE *fh, char *outFileName, unsigned int addr, unsigned int size, int *mail_accu, BOOL preview)
{
  BOOL rc = FALSE;
  unsigned char *buf;
  FILE *mailout = NULL;

  unsigned char *data;
  unsigned char *body;
  unsigned int i;
  unsigned int length_of_idxs;
  unsigned int num_of_idxs;
  unsigned int object_marker;
  unsigned char *entries[32];
  unsigned char *msg_entry;
  unsigned int msg_addr = 0;
  unsigned int mailStatusFlags = SFLAG_NONE;

  ENTER();

  if(size < 12)
    size = 12;

  if(!(buf = malloc(size)))
  {
    E(DBF_IMPORT, "Couldn't allocate %ld bytes", size);
    RETURN(FALSE);
    return FALSE;
  }

  // seek to the position where to find the message info object
  if(fseek(fh, addr, SEEK_SET))
  {
    E(DBF_IMPORT, "Unable to seek at %08lx", addr);
    goto out;
  }

  // read in the whole message info object
  if(fread(buf, 1, size, fh) != size)
  {
    E(DBF_IMPORT, "Unable to read %ld bytes", size);
    goto out;
  }

  // check if the object marker matches
  object_marker = GetLong(buf, 0);
  if(object_marker != addr)
  {
    E(DBF_IMPORT, "Object marker didn't match");
    goto out;
  }

  // check the number of indexes
  length_of_idxs = GetLong(buf, 4);
  num_of_idxs = buf[10];
  if(num_of_idxs > sizeof(entries)/sizeof(entries[0]))
  {
    E(DBF_IMPORT, "Too many indexes");
    goto out;
  }

  // check if we have read enough data, if not we must read more
  if(size-12 < length_of_idxs)
  {
    unsigned char *newbuf;

    D(DBF_IMPORT, "read in %ld bytes of object at 0x%08lx, but index length is %ld", size, addr, length_of_idxs);

    if(!(newbuf = malloc(length_of_idxs + 12)))
    {
      E(DBF_IMPORT, "Couldn't allocate %ld bytes", length_of_idxs+12);
      goto out;
    }

    memcpy(newbuf, buf, size);
    if(fread(&newbuf[size], 1, length_of_idxs - size, fh) != length_of_idxs - size)
    {
      E(DBF_IMPORT, "Couldn't load more bytes");

      free(newbuf);
      goto out;
    }

    free(buf);
    buf = newbuf;
  }

  body = buf + 12;
  data = body + num_of_idxs * 4;

  memset(entries, 0, sizeof(entries));

  for(i=0; i<num_of_idxs; i++)
  {
    unsigned int idx = body[0];
    unsigned int addr = body[1] | (body[2] << 8) | (body[3] << 16);

    // check the index value
    if((idx & 0x7f) > sizeof(entries)/sizeof(entries[0]))
    {
      E(DBF_IMPORT, "Wrong index");
      goto out;
    }

    if(idx & 0x80)
    {
      // overwrite the body (and enforce little endian)
      body[0] = addr & 0xff;
      body[1] = (addr & 0xff00) >> 8;
      body[2] = (addr & 0xff0000) >> 16;
      body[3] = 0;

      // is direct value
      entries[idx & 0x7f] = body;
    }
    else
    {
      entries[idx] = data + addr;
    }

    body += 4;
  }

  // Index number 1 points to flags
  if(entries[1])
  {
    unsigned int flags = GetLong(entries[1], 0);

    // check all flags and set the new mail status
    if(flags & (1UL << 5)) // mail has been marked
      SET_FLAG(mailStatusFlags, SFLAG_MARKED);

    if(flags & (1UL << 7)) // mail has been read
      SET_FLAG(mailStatusFlags, SFLAG_READ);

    if(flags & (1UL << 19)) // mail has replied status
      SET_FLAG(mailStatusFlags, SFLAG_REPLIED);
  }

  // Index number 4 points to the whole message
  if(!(msg_entry = entries[4]))
  {
    E(DBF_IMPORT, "Did not find a message");
    goto out;
  }

  // extract the message address
  msg_addr = GetLong(msg_entry, 0);

  // Open the output file
  if((mailout = fopen(outFileName, "wb")) == NULL)
  {
    E(DBF_IMPORT, "Couldn't open %s for output", outFileName);
    goto out;
  }

  // Write the message into the out file */
  if(ReadDBXMessage(fh, mailout, msg_addr) == FALSE)
  {
    E(DBF_IMPORT, "Couldn't read dbx message @ addr %08lx", msg_addr);

    fclose(mailout);
    mailout = NULL;
    DeleteFile(outFileName);
    goto out;
  }

  rc = TRUE;

out:
  if(mailout != NULL)
    fclose(mailout);

  free(buf);

  if(rc)
  {
    if(preview == TRUE)
    {
      LONG size;
      struct MailTransferNode *mtn;

      ObtainFileInfo(outFileName, FI_SIZE, &size);

      // if this is the preview run we go and
      // use the TR_AddMessageHeader method to
      // add the found mail to our mail list
      if((mtn = TR_AddMessageHeader(mail_accu, size, msg_addr, FilePart(outFileName))))
      {
        SET_FLAG(mtn->mail->sflags, mailStatusFlags);
      }

      DeleteFile(outFileName);
    }
    else
      ++(*mail_accu);
  }

  RETURN(rc);
  return rc;
}

///
/// ReadDBXNode()
// Function that reads in a node within the tree of a DBX Mail archive
// file from Outlook Express.
static BOOL ReadDBXNode(FILE *fh, char *outFileName, unsigned int addr, int *mail_accu, BOOL preview)
{
  unsigned char *buf;
  unsigned char *body;
  unsigned int child;
  int object_marker;
  int entries;

  ENTER();

  // alloc enough memory to facilitate the the whole tree
  if(!(buf = malloc((0x18+0x264))))
  {
    E(DBF_IMPORT, "Couldn't allocate enough memory for node");
    RETURN(FALSE);
    return FALSE;
  }

  // seek to the start of the tree node
  if(fseek(fh, addr, SEEK_SET))
  {
    free(buf);

    E(DBF_IMPORT, "Unable to seek at %08lx", addr);
    RETURN(FALSE);
    return FALSE;
  }

  // read in the whole tree with one read operation
  if(fread(buf, 1, (0x18+0x264), fh) != (0x18+0x264))
  {
    free(buf);

    E(DBF_IMPORT, "Unable to read %ld bytes", 0x18+0x264);
    RETURN(FALSE);
    return FALSE;
  }

  object_marker = GetLong(buf, 0);
  child = GetLong(buf, 8);
  entries = buf[17];
  body = &buf[0x18];

  while(entries--)
  {
    unsigned int value = GetLong(body, 0);
    unsigned int chld = GetLong(body, 4);
    unsigned int novals = GetLong(body, 8);

    // value points to a pointer to a message
    if(value)
    {
      if(ReadDBXMessageInfo(fh, outFileName, value, novals, mail_accu, preview) == FALSE)
      {
        free(buf);

        E(DBF_IMPORT, "Failed to read the indexed info");
        RETURN(FALSE);
        return FALSE;
      }
    }

    if(chld)
    {
      if(ReadDBXNode(fh, outFileName, chld, mail_accu, preview) == FALSE)
      {
        free(buf);

        E(DBF_IMPORT, "Failed to read node at %08lx", chld);
        RETURN(FALSE);
        return FALSE;
      }
    }

    body += 12;
  }

  free(buf);

  if(child)
    return ReadDBXNode(fh, outFileName, child, mail_accu, preview);

  RETURN(TRUE);
  return TRUE;
}

///
/// TR_GetMessageList_IMPORT
//  Collects messages from our different supported import formats
BOOL TR_GetMessageList_IMPORT(void)
{
  BOOL result = FALSE;

  ENTER();

  // check if MA_ImportMessages() did correctly set all required
  // data
  if(G->TR->ImportFile[0] != '\0' && G->TR->ImportFolder != NULL)
  {
    char tfname[SIZE_MFILE];
    char fname[SIZE_PATHFILE];
    int c = 0;

    // let the application sleep while we parse the file
    set(G->App, MUIA_Application_Sleep, TRUE);

    // clear the found mail list per default
    NewList((struct List *)&G->TR->transferList);

    // prepare the temporary filename buffers
    snprintf(tfname, sizeof(tfname), "YAMi%08x.tmp", (unsigned int)GetUniqueID());
    AddPath(fname, C->TempDir, tfname, sizeof(fname));

    // before this function is called the MA_ImportMessages() function
    // already found out which import format we can expect. So we
    // distinguish between the different known formats here
    switch(G->TR->ImportFormat)
    {
      // treat the file as a MBOX compliant file
      case IMF_MBOX:
      {
        FILE *ifh;

        D(DBF_IMPORT, "trying to retrieve mail list from MBOX compliant file");

        if((ifh = fopen(G->TR->ImportFile, "r")) != NULL)
        {
          FILE *ofh = NULL;
          char *buffer = NULL;
          size_t bufsize = 0;
          BOOL foundBody = FALSE;
          int size = 0;
          long addr = 0;

          setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

          while(GetLine(&buffer, &bufsize, ifh) >= 0)
          {
            // now we parse through the input file until we
            // find the "From " separator
            if(strncmp(buffer, "From ", 5) == 0)
            {
              // now we know that a new mail has started so if
              // we already found a previous mail we can add it
              // to our list
              if(foundBody == TRUE)
              {
                D(DBF_IMPORT, "found subsequent 'From ' separator: '%s'", buffer);

                result = (TR_AddMessageHeader(&c, size, addr, tfname) != NULL);
                DeleteFile(fname);

                if(result == FALSE)
                  break;
              }
              else
                D(DBF_IMPORT, "found first 'From ' separator: '%s'", buffer);

              // as a new mail is starting we have to
              // open a new file handler
              if((ofh = fopen(fname, "w")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              size = 0;
              foundBody = FALSE;
              addr = ftell(ifh);

              // continue with the next iteration
              continue;
            }

            // if we already have an opened tempfile
            // and we didn't found the separating mail body
            // yet we go and write out the buffer content
            if(ofh != NULL && foundBody == FALSE)
            {
              fprintf(ofh, "%s\n", buffer);

              // if the buffer is empty we found the corresponding body
              // of the mail and can close the ofh pointer
              if(buffer[0] == '\0')
              {
                fclose(ofh);
                ofh = NULL;
                foundBody = TRUE;

                D(DBF_IMPORT, "found body part of import mail");
              }
            }

            // to sum the size we count the length of our read buffer
            if(ofh != NULL || foundBody == TRUE)
              size += strlen(buffer)+1;
          }

          // check the reason why we exited the while loop
          if(feof(ifh) == 0)
          {
            E(DBF_IMPORT, "while loop seems to have exited without having scanned until EOF!");
            result = foundBody = FALSE;
          }

          // after quiting the while() loop, we have to check
          // if there is still some data to process
          if(foundBody == TRUE)
          {
            result = (TR_AddMessageHeader(&c, size, addr, tfname) != NULL);
            DeleteFile(fname);
          }
          else if(ofh != NULL)
          {
            fclose(ofh);
            ofh = NULL;
            DeleteFile(fname);
          }

          fclose(ifh);

          free(buffer);
        }
        else
          E(DBF_IMPORT, "Error on trying to open file '%s'", G->TR->ImportFile);
      }
      break;

      // treat the file as a file that contains a single
      // unencoded mail (*.eml alike file)
      case IMF_PLAIN:
      {
        if(CopyFile(fname, NULL, G->TR->ImportFile, NULL) == TRUE)
        {
          LONG size;

          ObtainFileInfo(fname, FI_SIZE, &size);
          // if the file was identified as a plain .eml file we
          // just have to go and call TR_AddMessageHeader to let
          // YAM analyze the file
          result = (TR_AddMessageHeader(&c, size, 0, tfname) != NULL);

          DeleteFile(fname);
        }
      }
      break;

      // treat the file as a DBX (Outlook Express) compliant mail archive
      case IMF_DBX:
      {
        FILE *ifh;

        // lets open the file and read out the root node of the dbx mail file
        if((ifh = fopen(G->TR->ImportFile, "rb")) != NULL)
        {
          unsigned char *file_header;

          setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

          // read the 9404 bytes long file header for properly identifying
          // an Outlook Express database file.
          if((file_header = (unsigned char *)malloc(0x24bc)) != NULL)
          {
            if(fread(file_header, 1, 0x24bc, ifh) == 0x24bc)
            {
              // try to identify the file as a CLSID_MessageDatabase file
              if((file_header[0] == 0xcf && file_header[1] == 0xad &&
                  file_header[2] == 0x12 && file_header[3] == 0xfe) &&
                 (file_header[4] == 0xc5 && file_header[5] == 0xfd &&
                  file_header[6] == 0x74 && file_header[7] == 0x6f))
              {
                int number_of_mails = GetLong(file_header, 0xc4);
                unsigned int root_node = GetLong(file_header, 0xe4);

                D(DBF_IMPORT, "number of mails in dbx file: %ld", number_of_mails);

                // now we actually start at the root node and read in all messages
                // accordingly
                if(ReadDBXNode(ifh, fname, root_node, &c, TRUE) == TRUE && c == number_of_mails)
                  result = TRUE;
                else
                  E(DBF_IMPORT, "Failed to read from root_node; c=%ld", c);
              }
            }

            free(file_header);
          }

          fclose(ifh);
        }
      }
      break;

      case IMF_UNKNOWN:
        // nothing
      break;
    };

    TR_DisplayMailList(FALSE);

    // if everything went fine but we didn't find any mail to import we signal failure
    if(c == 0)
      result = FALSE;

    // wake up the application again
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(result);
  return result;
}
///
/// TR_AbortIMPORTFunc
//  Aborts import process
HOOKPROTONHNONP(TR_AbortIMPORTFunc, void)
{
   TR_AbortnClose();
}
MakeStaticHook(TR_AbortIMPORTHook, TR_AbortIMPORTFunc);
///
/// TR_ProcessIMPORTFunc
//  Imports messages from a MBOX mailbox file
HOOKPROTONHNONP(TR_ProcessIMPORTFunc, void)
{
  ENTER();

  // if there is nothing to import we can skip
  // immediately.
  if(IsMinListEmpty(&G->TR->transferList) == FALSE)
  {
    struct TransStat ts;

    TR_TransStat_Init(&ts);
    if(ts.Msgs_Tot > 0)
    {
      struct Folder *folder = G->TR->ImportFolder;
      enum FolderType ftype = folder->Type;

      TR_TransStat_Start(&ts);

      // now we distinguish between the different import format
      // and import the mails out of it
      switch(G->TR->ImportFormat)
      {
        // treat the file as a MBOX compliant file but also
        // in case of plain (*.eml) file we can that the very same
        // routines.
        case IMF_MBOX:
        case IMF_PLAIN:
        {
          FILE *ifh;

          if((ifh = fopen(G->TR->ImportFile, "r")) != NULL)
          {
            struct Node *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our transferList and seek to
            // each position/address of a mail
            IterateList(&G->TR->transferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;
              FILE *ofh = NULL;
              char mfile[SIZE_MFILE];
              char *buffer = NULL;
              size_t size = 0;
              BOOL foundBody = FALSE;
              unsigned int status = SFLAG_NONE;
              unsigned int xstatus = SFLAG_NONE;
              BOOL ownStatusFound = FALSE;

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(hasTR_LOAD(mtn) == FALSE)
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, mtn->importAddr, SEEK_SET) != 0)
                break;

              TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Importing));

              if((ofh = fopen(MA_NewMailFile(folder, mfile), "w")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              // now that we seeked to the mail address we go
              // and read in line by line
              while(GetLine(&buffer, &size, ifh) >= 0 && G->TR->Abort == FALSE)
              {
                // if we did not find the message body yet
                if(foundBody == FALSE)
                {
                  if(buffer[0] == '\0')
                    foundBody = TRUE; // we found the body part
                  else
                  {
                    // we search for some interesting header lines (i.e. X-Status: etc.)
                    if(strnicmp(buffer, "X-Status: ", 10) == 0)
                    {
                      xstatus = MA_FromXStatusHeader(&buffer[10]);
                      ownStatusFound = TRUE;
                    }
                    else if(strnicmp(buffer, "Status: ", 8) == 0)
                    {
                      status = MA_FromStatusHeader(&buffer[8]);
                      ownStatusFound = TRUE;
                    }
                  }

                  fprintf(ofh, "%s\n", buffer);
                }
                else
                {
                  char *p;

                  // now that we are parsing within the message body we have to
                  // search for new "From " lines as well.
                  if(strncmp(buffer, "From ", 5) == 0)
                    break;

                  // the mboxrd format specifies that we need to unquote any >From, >>From etc. occurance.
                  // http://www.qmail.org/man/man5/mbox.html
                  p = buffer;
                  while(*p == '>')
                    p++;

                  // if we found a quoted line we need to check if there is a following "From " and if so
                  // we have to skip ONE quote.
                  if(p != buffer && strncmp(p, "From ", 5) == 0)
                    fprintf(ofh, "%s\n", &buffer[1]);
                  else
                    fprintf(ofh, "%s\n", buffer);
                }

                // update the transfer statistics
                TR_TransStat_Update(&ts, strlen(buffer)+1, tr(MSG_TR_Importing));
              }

              fclose(ofh);
              ofh = NULL;

              free(buffer);

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(ownStatusFound == FALSE)
              {
                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  status = SFLAG_QUEUED | SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  status = SFLAG_SENT | SFLAG_READ;
                else
                  status = SFLAG_NEW;
              }
              else
              {
                // Check whether Status and X-Status contained some contradicting flags.
                // The X-Status header line contains no explicit information about the "new"
                // state of a mail, but the Status header line does. Hence we derive this
                // flag from the Status header line only.
                if(isFlagClear(status, SFLAG_NEW) && isFlagSet(xstatus, SFLAG_NEW))
                  CLEAR_FLAG(xstatus, SFLAG_NEW);
              }

              // set the status flags now
              SET_FLAG(mail->sflags, status | xstatus);

              // depending on the Status we have to set the transDate or not
              if(!hasStatusQueued(mail) && !hasStatusHold(mail))
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              if((mail = AddMailToList(mail, folder)) != NULL)
              {
                // update the mailFile Path
                strlcpy(mail->MailFile, mfile, sizeof(mail->MailFile));

                // if this was a compressed/encrypted folder we need to pack the mail now
                if(folder->Mode > FM_SIMPLE)
                  RepackMailFile(mail, -1, NULL);

                // update the mailfile accordingly.
                MA_UpdateMailFile(mail);

                // put the transferStat to 100%
                TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Importing));
              }

              if(G->TR->Abort == TRUE)
                break;
            }

            fclose(ifh);
          }
        }
        break;

        // the file was previously identified as a *.dbx file which
        // was created by Outlook Express.
        case IMF_DBX:
        {
          FILE *ifh;

          if((ifh = fopen(G->TR->ImportFile, "rb")) != NULL)
          {
            struct Node *curNode;

            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            // iterate through our transferList and seek to
            // each position/address of a mail
            IterateList(&G->TR->transferList, curNode)
            {
              struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
              struct Mail *mail = mtn->mail;
              FILE *ofh = NULL;
              char mfile[SIZE_MFILE];

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(hasTR_LOAD(mtn) == FALSE)
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, mtn->importAddr, SEEK_SET) != 0)
                break;

              TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Importing));

              if((ofh = fopen(MA_NewMailFile(folder, mfile), "wb")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              if(ReadDBXMessage(ifh, ofh, mtn->importAddr) == FALSE)
                E(DBF_IMPORT, "Couldn't import dbx message from addr %08lx", mtn->importAddr);

              fclose(ofh);

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(mail->sflags != SFLAG_NONE)
              {
                unsigned int stat = SFLAG_NONE;

                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  stat = SFLAG_QUEUED | SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  stat = SFLAG_SENT | SFLAG_READ;
                else
                  stat = SFLAG_NEW;

                SET_FLAG(mail->sflags, stat);
              }

              // depending on the Status we have to set the transDate or not
              if(!hasStatusQueued(mail) && !hasStatusHold(mail))
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              if((mail = AddMailToList(mail, folder)) != NULL)
              {
                // update the mailFile Path
                strlcpy(mail->MailFile, mfile, sizeof(mail->MailFile));

                // if this was a compressed/encrypted folder we need to pack the mail now
                if(folder->Mode > FM_SIMPLE)
                  RepackMailFile(mail, -1, NULL);

                // update the mailfile accordingly.
                MA_UpdateMailFile(mail);

                // put the transferStat to 100%
                TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Importing));
              }

              if(G->TR->Abort == TRUE)
                break;
            }

            fclose(ifh);
          }

          case IMF_UNKNOWN:
            // nothing
          break;
        }
        break;
      }

      TR_TransStat_Finish(&ts);

      AppendToLogfile(LF_ALL, 50, tr(MSG_LOG_Importing), ts.Msgs_Done, G->TR->ImportFile, folder->Name);
      DisplayStatistics(folder, TRUE);
      MA_ChangeFolder(NULL, FALSE);
    }

    TR_AbortnClose();
  }

  LEAVE();
}
MakeHook(TR_ProcessIMPORTHook, TR_ProcessIMPORTFunc);
///

/*** GET ***/
/// TR_AbortGETFunc
//  Aborts a POP3 download
HOOKPROTONHNONP(TR_AbortGETFunc, void)
{
  ENTER();

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  // first set the Abort variable so that other can benefit from it
  G->TR->Abort = TRUE;

  // we can easily abort the transfer by setting the POP_Nr to -1
  // and issue a GetMailFromNextPOP command. With this solution YAM will
  // also process the filters for mails that were already downloaded
  // even if the user aborted the transfer somehow
  G->TR->POP_Nr = -1;
  TR_GetMailFromNextPOP(FALSE, -1, 0);

  LEAVE();
}
MakeStaticHook(TR_AbortGETHook, TR_AbortGETFunc);
///
/// TR_NewMailAlert
//  Notifies user when new mail is available
void TR_NewMailAlert(void)
{
  struct DownloadResult *stats = &G->TR->Stats;
  struct RuleResult *rr = &G->RuleResults;

  ENTER();

  SHOWVALUE(DBF_NET, stats->Downloaded);
  SHOWVALUE(DBF_NET, rr->Spam);

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(stats->Downloaded > 0 && stats->Downloaded > rr->Spam)
  {
    if(hasRequesterNotify(C->NotifyType) && G->TR->GUIlevel != POP_REXX)
    {
      static char buffer[SIZE_LARGE];

      // make sure the application isn't iconified
      if(xget(G->App, MUIA_Application_Iconified) == TRUE)
        PopUp();

      snprintf(buffer, sizeof(buffer), tr(MSG_TR_NewMailReq), stats->Downloaded, stats->OnServer-stats->Deleted, stats->DupSkipped);
      if(C->SpamFilterEnabled == TRUE)
      {
        // include the number of spam classified mails
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FILTER_STATS_SPAM),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted,
                                                                         rr->Spam);
      }
      else
      {
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FilterStats),
                                                                         rr->Checked,
                                                                         rr->Bounced,
                                                                         rr->Forwarded,
                                                                         rr->Replied,
                                                                         rr->Executed,
                                                                         rr->Moved,
                                                                         rr->Deleted);
      }

      // show the info window.
      InfoWindowObject,
        MUIA_Window_Title, tr(MSG_TR_NewMail),
        MUIA_Window_RefWindow, G->MA->GUI.WI,
        MUIA_Window_Activate, G->TR->GUIlevel == POP_USER,
        MUIA_InfoWindow_Body, buffer,
      End;
    }

    #if defined(__amigaos4__)
    if(hasOS41SystemNotify(C->NotifyType))
    {
      D(DBF_GUI, "appID is %ld, application.lib is V%ld.%ld (needed V%ld.%ld)", G->applicationID, ApplicationBase->lib_Version, ApplicationBase->lib_Revision, 53, 7);
      // Notify() is V53.2+, but 53.7 fixes some serious issues
      if(G->applicationID > 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 7) == TRUE)
      {
        // 128 chars is the current maximum :(
        char message[128];
        int count = stats->Downloaded - rr->Spam;

        // distinguish between single and multiple mails
        if(count >= 2)
          snprintf(message, sizeof(message), tr(MSG_TR_NEW_MAIL_NOTIFY_MANY), count);
        else
          strlcpy(message, tr(MSG_TR_NEW_MAIL_NOTIFY_ONE), sizeof(message));

        // We require 53.7+. From this version on proper tag values are used, hence there
        // is no need to distinguish between v1 and v2 interfaces here as we have to do for
        // other application.lib functions.
        Notify(G->applicationID, APPNOTIFY_Title, (uint32)"YAM",
                                 APPNOTIFY_PubScreenName, (uint32)"FRONT",
                                 APPNOTIFY_Text, (uint32)message,
                                 APPNOTIFY_CloseOnDC, TRUE,
                                 APPNOTIFY_BackMsg, (uint32)"POPUP",
                                 TAG_DONE);
      }
    }
    #endif // __amigaos4__

    if(hasCommandNotify(C->NotifyType))
      LaunchCommand(C->NotifyCommand, FALSE, OUT_STDOUT);

    if(hasSoundNotify(C->NotifyType))
      DoAction(TA_PlaySound, TT_PlaySound_Filename, C->NotifySound, TAG_DONE);
  }

  LEAVE();
}

///
/// TR_ProcessGETFunc
/*** TR_ProcessGETFunc - Downloads messages from a POP3 server ***/
HOOKPROTONHNONP(TR_ProcessGETFunc, void)
{
  struct TransStat ts;

  ENTER();

  // initialize the transfer statistics
  TR_TransStat_Init(&ts);

  // make sure the NOOP timer is definitly stopped
  StopTimer(TIMER_POP3_KEEPALIVE);

  if(ts.Msgs_Tot > 0)
  {
    struct Folder *infolder = FO_GetFolderByType(FT_INCOMING, NULL);
    struct Node *curNode;
    struct MUI_NListtree_TreeNode *incomingTreeNode = FO_GetFolderTreeNode(infolder);

    if(C->TransferWindow == TWM_SHOW && xget(G->TR->GUI.WI, MUIA_Window_Open) == FALSE)
      set(G->TR->GUI.WI, MUIA_Window_Open, TRUE);

    TR_TransStat_Start(&ts);

    IterateList(&G->TR->transferList, curNode)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;
      struct Mail *mail = mtn->mail;

      D(DBF_NET, "download flags %08lx=%s%s%s for mail with subject '%s' and size %ld",mtn->tflags, hasTR_LOAD(mtn) ? "TR_LOAD " : "" , hasTR_DELETE(mtn) ? "TR_DELETE " : "", hasTR_PRESELECT(mtn) ? "TR_PRESELECT " : "", mail->Subject, mail->Size);
      if(hasTR_LOAD(mtn))
      {
        D(DBF_NET, "downloading mail with subject '%s' and size %ld", mail->Subject, mail->Size);

        // update the transfer status
        TR_TransStat_NextMsg(&ts, mtn->index, mtn->position, mail->Size, tr(MSG_TR_Downloading));

        if(TR_LoadMessage(infolder, &ts, mtn->index) == TRUE)
        {
          // redraw the folderentry in the listtree
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, incomingTreeNode, MUIF_NONE);

          // put the transferStat for this mail to 100%
          TR_TransStat_Update(&ts, TS_SETMAX, tr(MSG_TR_Downloading));

          G->TR->Stats.Downloaded++;

          // Remember the UIDL of this mail, no matter if it is going
          // to be deleted or not. Some servers don't delete a mail
          // right after the DELETE command, but only after a successful
          // QUIT command. Personal experience shows that pop.gmx.de is
          // one of these servers.
          if(C->AvoidDuplicates == TRUE)
          {
            D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
            // add the UIDL to the hash table or update an existing entry
            AddUIDLtoHash(G->TR->UIDLhashTable, mtn->UIDL, UIDLF_NEW);
          }

          if(hasTR_DELETE(mtn))
          {
            D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

            TR_DeleteMessage(&ts, mtn->index);
          }
          else
            D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
        }
      }
      else if(hasTR_DELETE(mtn))
      {
        D(DBF_NET, "deleting mail with subject '%s' on server", mail->Subject);

        // now we "know" that this mail had existed, don't forget this in case
        // the delete operation fails
        if(C->AvoidDuplicates == TRUE)
        {
          D(DBF_NET, "adding mail with subject '%s' to UIDL hash", mail->Subject);
          // add the UIDL to the hash table or update an existing entry
          AddUIDLtoHash(G->TR->UIDLhashTable, mtn->UIDL, UIDLF_NEW);
        }

        TR_DeleteMessage(&ts, mtn->index);
      }
      else
      {
        D(DBF_NET, "leaving mail with subject '%s' and size %ld on server to be downloaded again", mail->Subject, mail->Size);
        // Do not modify the UIDL hash here!
        // The mail was marked as "don't download", but here we don't know if that
        // is due to the duplicates checking or if the user did that himself.
      }

      if(G->TR->Abort == TRUE || G->TR->connection->error != CONNECTERR_NO_ERROR)
        break;
    }

    TR_TransStat_Finish(&ts);

    DisplayStatistics(infolder, TRUE);

    // in case the current folder after the download
    // is the incoming folder we have to update
    // the main toolbar and menu items
    if(FO_GetCurrentFolder() == infolder)
      MA_ChangeSelected(TRUE);
  }

  TR_GetMailFromNextPOP(FALSE, 0, 0);

  LEAVE();
}
MakeHook(TR_ProcessGETHook, TR_ProcessGETFunc);

///
/// TR_GetMessageInfoFunc
//  Requests message header of a message selected by the user
HOOKPROTONHNONP(TR_GetMessageInfoFunc, void)
{
  int line;
  struct MailTransferNode *mtn;

  ENTER();

  line = xget(G->TR->GUI.LV_MAILS, MUIA_NList_Active);
  DoMethod(G->TR->GUI.LV_MAILS, MUIM_NList_GetEntry, line, &mtn);
  TR_GetMessageDetails(mtn, line);

  LEAVE();
}
MakeStaticHook(TR_GetMessageInfoHook, TR_GetMessageInfoFunc);
///
/// TR_CompleteMsgList
//  Gets details for messages on server
void TR_CompleteMsgList(void)
{
  struct TR_ClassData *tr = G->TR;

  ENTER();

  // first we have to set the notifies to the default values.
  // this is needed so that if we get mail from more than one POP3 at a line this
  // abort stuff works out
  set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, FALSE);
  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Start));
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(tr->Abort));

  if(C->PreSelection < PSM_ALWAYSLARGE)
  {
    struct MinNode *curNode = tr->GMD_Mail;

    for(; curNode->mln_Succ && tr->Abort == FALSE && tr->connection->error == CONNECTERR_NO_ERROR; curNode = curNode->mln_Succ)
    {
      struct MailTransferNode *mtn = (struct MailTransferNode *)curNode;

      if(tr->Pause == TRUE)
        break;

      if(tr->Start == TRUE)
      {
        TR_ProcessGETFunc();
        break;
      }

      if(C->PreSelection != PSM_LARGE || mtn->mail->Size >= C->WarnSize*1024)
      {
        TR_GetMessageDetails(mtn, tr->GMD_Line++);

        // set the next mail as the active one for the display,
        // so that if the user pauses we can go on here
        tr->GMD_Mail = curNode->mln_Succ;
      }
    }
  }

  set(G->TR->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
  DoMethod(tr->GUI.BT_START, MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessGETHook);
  DoMethod(tr->GUI.BT_QUIT , MUIM_KillNotify, MUIA_Pressed);
  DoMethod(tr->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortGETHook);

  if(tr->Abort == TRUE)
    TR_AbortGETFunc();
  else
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }

  LEAVE();
}
///
/// TR_PauseFunc
//  Pauses or resumes message download
HOOKPROTONHNO(TR_PauseFunc, void, int *arg)
{
  BOOL pause = *arg;

  ENTER();

  set(G->TR->GUI.BT_RESUME, MUIA_Disabled, pause == FALSE);
  set(G->TR->GUI.BT_PAUSE,  MUIA_Disabled, pause == TRUE);

  if(pause == TRUE)
  {
    // start the timer which makes sure we send
    // a regular NOOP command to the POP3 server
    // so that it doesn't drop the connection
    RestartTimer(TIMER_POP3_KEEPALIVE, C->KeepAliveInterval, 0);
  }
  else
  {
    G->TR->Pause = FALSE;
    TR_CompleteMsgList();
  }

  LEAVE();
}
MakeStaticHook(TR_PauseHook, TR_PauseFunc);
///

/*** GUI ***/
/// TR_New
//  Creates transfer window
struct TR_ClassData *TR_New(enum TransferType TRmode)
{
  struct TR_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct TR_ClassData))) != NULL)
  {
    if((data->downloadedMails = CreateMailList()) != NULL)
    {
      Object *bt_all = NULL, *bt_none = NULL, *bt_loadonly = NULL, *bt_loaddel = NULL, *bt_delonly = NULL, *bt_leave = NULL;
      Object *gr_sel, *gr_proc, *gr_win;
      BOOL fullwin = (TRmode == TR_GET_USER || TRmode == TR_GET_AUTO || TRmode == TR_IMPORT);
      static char status_label[SIZE_DEFAULT];
      static char size_gauge_label[SIZE_DEFAULT];
      static char msg_gauge_label[SIZE_DEFAULT];
      static char str_size_done[SIZE_SMALL];
      static char str_size_tot[SIZE_SMALL];
      static char str_speed[SIZE_SMALL];
      static char str_size_curr[SIZE_SMALL];
      static char str_size_curr_max[SIZE_SMALL];

      NewList((struct List *)&data->transferList);

      // prepare the initial text object content
      FormatSize(0, str_size_done, sizeof(str_size_done), SF_MIXED);
      FormatSize(0, str_size_tot, sizeof(str_size_tot), SF_MIXED);
      FormatSize(0, str_speed, sizeof(str_speed), SF_MIXED);
      snprintf(status_label, sizeof(status_label), tr(MSG_TR_TRANSFERSTATUS),
                                      str_size_done, str_size_tot, str_speed, 0, 0, 0, 0);

      snprintf(msg_gauge_label, sizeof(msg_gauge_label), tr(MSG_TR_MESSAGEGAUGE), 0);

      FormatSize(0, str_size_curr, sizeof(str_size_curr), SF_AUTO);
      FormatSize(0, str_size_curr_max, sizeof(str_size_curr_max), SF_AUTO);
      snprintf(size_gauge_label, sizeof(size_gauge_label), tr(MSG_TR_TRANSFERSIZE),
                                                           str_size_curr, str_size_curr_max);

      gr_proc = ColGroup(2), GroupFrameT(tr(MSG_TR_Status)),
         Child, data->GUI.TX_STATS = TextObject,
            MUIA_Text_Contents, status_label,
            MUIA_Background,    MUII_TextBack,
            MUIA_Frame,         MUIV_Frame_Text,
            MUIA_Text_PreParse, MUIX_C,
         End,
         Child, VGroup,
            Child, data->GUI.GA_COUNT = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz,    TRUE,
               MUIA_Gauge_InfoText, msg_gauge_label,
            End,
            Child, data->GUI.GA_BYTES = GaugeObject,
               GaugeFrame,
               MUIA_Gauge_Horiz,    TRUE,
               MUIA_Gauge_InfoText, size_gauge_label,
            End,
         End,
         Child, data->GUI.TX_STATUS = TextObject,
            MUIA_Background,MUII_TextBack,
            MUIA_Frame     ,MUIV_Frame_Text,
         End,
         Child, data->GUI.BT_ABORT = MakeButton(tr(MSG_TR_Abort)),
      End;

      if(fullwin == TRUE)
      {
        data->GUI.GR_LIST = VGroup, GroupFrameT(TRmode==TR_IMPORT ? tr(MSG_TR_MsgInFile) : tr(MSG_TR_MsgOnServer)),
           MUIA_ShowMe, TRmode==TR_IMPORT || C->PreSelection >= PSM_ALWAYS,
           Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, data->GUI.LV_MAILS = TransferMailListObject,
              End,
           End,
        End;
        gr_sel = VGroup, GroupFrameT(tr(MSG_TR_Control)),
           Child, ColGroup(5),
              Child, bt_all = MakeButton(tr(MSG_TR_All)),
              Child, bt_loaddel = MakeButton(tr(MSG_TR_DownloadDelete)),
              Child, bt_leave = MakeButton(tr(MSG_TR_Leave)),
              Child, HSpace(0),
              Child, data->GUI.BT_PAUSE = MakeButton(tr(MSG_TR_Pause)),
              Child, bt_none = MakeButton(tr(MSG_TR_Clear)),
              Child, bt_loadonly = MakeButton(tr(MSG_TR_DownloadOnly)),
              Child, bt_delonly = MakeButton(tr(MSG_TR_DeleteOnly)),
              Child, HSpace(0),
              Child, data->GUI.BT_RESUME = MakeButton(tr(MSG_TR_Resume)),
           End,
           Child, ColGroup(2),
              Child, data->GUI.BT_START = MakeButton(tr(MSG_TR_Start)),
              Child, data->GUI.BT_QUIT = MakeButton(tr(MSG_TR_Abort)),
           End,
        End;
        gr_win = VGroup,
           Child, data->GUI.GR_LIST,
           Child, data->GUI.GR_PAGE = PageGroup,
              Child, gr_sel,
              Child, gr_proc,
           End,
        End;
      }
      else
      {
        gr_win = VGroup, MUIA_Frame, MUIV_Frame_None,
           Child, gr_proc,
        End;
      }

      data->GUI.WI = WindowObject,
         MUIA_Window_ID, MAKE_ID('T','R','A','0'+TRmode),
         MUIA_Window_CloseGadget, FALSE,
         MUIA_Window_Activate, (TRmode == TR_IMPORT || TRmode == TR_EXPORT || TRmode == TR_GET_USER || TRmode == TR_SEND_USER),
         MUIA_HelpNode, "TR_W",
         WindowContents, gr_win,
      End;

      if(data->GUI.WI != NULL)
      {
        DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
        SetHelp(data->GUI.TX_STATUS,MSG_HELP_TR_TX_STATUS);
        SetHelp(data->GUI.BT_ABORT ,MSG_HELP_TR_BT_ABORT);

        if(fullwin == TRUE)
        {
          set(data->GUI.WI, MUIA_Window_DefaultObject, data->GUI.LV_MAILS);
          set(data->GUI.BT_RESUME, MUIA_Disabled, TRUE);

          if(TRmode == TR_IMPORT)
          {
            set(data->GUI.BT_PAUSE, MUIA_Disabled, TRUE);
            set(bt_delonly        , MUIA_Disabled, TRUE);
            set(bt_loaddel        , MUIA_Disabled, TRUE);
            DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_ProcessIMPORTHook);
            DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_AbortIMPORTHook);
          }
          else
          {
            set(data->GUI.GR_PAGE, MUIA_Group_ActivePage, 1);
            DoMethod(data->GUI.BT_RESUME,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, FALSE);
            DoMethod(data->GUI.BT_PAUSE ,MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_PauseHook, TRUE);
            DoMethod(data->GUI.BT_PAUSE, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Pause));
            DoMethod(data->GUI.LV_MAILS ,MUIM_Notify, MUIA_NList_DoubleClick,TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &TR_GetMessageInfoHook);
            DoMethod(bt_delonly,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_DELETE);
            DoMethod(bt_loaddel,         MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, (TRF_LOAD|TRF_DELETE));
            DoMethod(data->GUI.BT_START, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Start));
            DoMethod(data->GUI.BT_QUIT , MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
          }
          DoMethod(bt_loadonly,        MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_LOAD);
          DoMethod(bt_leave,           MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_CallHook, &TR_ChangeTransFlagsHook, TRF_NONE);
          DoMethod(bt_all,             MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL);
          DoMethod(bt_none,            MUIM_Notify, MUIA_Pressed, FALSE, data->GUI.LV_MAILS, 4, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL);
        }
        DoMethod(data->GUI.BT_ABORT, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 3, MUIM_WriteLong, TRUE, &(data->Abort));
        MA_ChangeTransfer(FALSE);
      }
      else
      {
        DeleteMailList(data->downloadedMails);
        free(data);
        data = NULL;
      }
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
///

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

#include "FileInfo.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailExport.h"
#include "MailList.h"
#include "MailTransferList.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "mui/ClassesExtra.h"
#include "mui/TransferControlGroup.h"
#include "mui/YAMApplication.h"
#include "tcp/Connection.h"

#include "Debug.h"

struct TransferContext
{
  struct Connection *connection;
  Object *transferGroup;
  char transferGroupTitle[SIZE_DEFAULT]; // the TransferControlGroup's title
  struct MailTransferList transferList;
};

/// ExportMails
//  Saves a list of messages to a MBOX mailbox file
BOOL ExportMails(const char *fname, struct MailList *mlist, const ULONG flags)
{
  BOOL success = FALSE;
  struct TransferContext *tc;

  ENTER();

  if((tc = calloc(1, sizeof(*tc))) != NULL)
  {
    // no socket required
    if((tc->connection = CreateConnection(FALSE)) != NULL)
    {
      ULONG twFlags;

      snprintf(tc->transferGroupTitle, sizeof(tc->transferGroupTitle), tr(MSG_TR_MailTransferTo), fname);

      twFlags = TWF_ACTIVATE;
      if(isFlagClear(flags, EXPORTF_QUIET))
        setFlag(twFlags, TWF_FORCE_OPEN);

      if((tc->transferGroup = (Object *)PushMethodOnStackWait(G->App, 5, MUIM_YAMApplication_CreateTransferGroup, CurrentThread(), tc->transferGroupTitle, tc->connection, twFlags)) != NULL)
      {
        BOOL abortExport = FALSE;
        struct MailNode *mnode;
        ULONG totalSize = 0;
        int i;

        // reset our processing list
        InitMailTransferList(&tc->transferList);

        // temporarly copy all data out of our mlist to the
        // processing list and mark all mails as "to be transferred"
        LockMailListShared(mlist);

        i = 0;
        ForEachMailNode(mlist, mnode)
        {
          struct Mail *mail = mnode->mail;

          if(mail != NULL)
          {
            struct MailTransferNode *tnode;

            if((tnode = CreateMailTransferNode(mail, TRF_TRANSFER)) != NULL)
            {
              tnode->index = i + 1;

              totalSize += mail->Size;

              AddMailTransferNode(&tc->transferList, tnode);
            }
            else
            {
              // we end up in a low memory condition, let's exit
              abortExport = TRUE;
              break;
            }
          }

          i++;
        }

        UnlockMailList(mlist);

        // if we have now something in our processing list,
        // lets go on
        if(abortExport == FALSE && IsMailTransferListEmpty(&tc->transferList) == FALSE)
        {
          FILE *fh;

          PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Start, tc->transferList.count, totalSize);

          // open our final destination file either in append or in a fresh
          // write mode.
          if((fh = fopen(fname, isFlagSet(flags, EXPORTF_APPEND) ? "a" : "w")) != NULL)
          {
            struct MailTransferNode *tnode;

            setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

            // assume success for the beginning
            success = TRUE;

            ForEachMailTransferNode(&tc->transferList, tnode)
            {
              struct Mail *mail = tnode->mail;
              char mailfile[SIZE_PATHFILE];
              char fullfile[SIZE_PATHFILE];

              // update the transfer status
              PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, tnode->index, -1, mail->Size, tr(MSG_TR_Exporting));

              GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
              if(StartUnpack(mailfile, fullfile, mail->Folder) != NULL)
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
                  while(tc->connection->abort == FALSE &&
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
                    // From, >From, >>From etc-> occurance.
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
                    PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, curlen, tr(MSG_TR_Exporting));
                  }

                  // check why we exited the while() loop and if everything is fine
                  if(tc->connection->abort == TRUE)
                  {
                    D(DBF_NET, "export was aborted by the user");
                    success = FALSE;
                  }
                  else if(ferror(fh) != 0)
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
                  PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Exporting));
                }
                else
                  success = FALSE;

                FinishUnpack(fullfile);
              }
              else
                success = FALSE;

              if(tc->connection->abort == TRUE || success == FALSE)
                break;
            }

            // close file pointer
            fclose(fh);

            // write the status to our logfile
            LockMailListShared(mlist);
            mnode = FirstMailNode(mlist);
            AppendToLogfile(LF_ALL, 51, tr(MSG_LOG_Exporting), tc->transferList.count, mnode->mail->Folder->Name, fname);
            UnlockMailList(mlist);
          }

          PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);
        }

        // delete all nodes in our temporary list
        ClearMailTransferList(&tc->transferList);

        PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DeleteTransferGroup, tc->transferGroup);
      }
    }

    DeleteConnection(tc->connection);

    free(tc);
  }

  // delete the list of mails no matter if the export succeeded or not
  DeleteMailList(mlist);

  // wake up the calling thread if this is requested
  if(isFlagSet(flags, EXPORTF_SIGNAL))
    WakeupThread(NULL);

  RETURN(success);
  return success;
}

///

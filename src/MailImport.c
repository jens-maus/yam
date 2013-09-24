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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/utility.h>
#include <libraries/mui.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"
#include "YAM_utilities.h"

#include "Config.h"
#include "FileInfo.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailImport.h"
#include "MailList.h"
#include "MailTransferList.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Threads.h"

#include "mui/ClassesExtra.h"
#include "mui/PreselectionWindow.h"
#include "mui/TransferControlGroup.h"
#include "mui/YAMApplication.h"
#include "tcp/Connection.h"

#include "Debug.h"

enum ImportFormat
{
  IMF_UNKNOWN = 0,
  IMF_MBOX,
  IMF_DBX,
  IMF_PLAIN
};

struct TransferContext
{
  struct Connection *conn;
  char windowTitle[SIZE_DEFAULT];        // the preselection window's title
  char transferGroupTitle[SIZE_DEFAULT]; // the TransferControlGroup's title
  Object *transferGroup;
  struct MailTransferList *importList;
  enum ImportFormat format;
};

/**************************************************************************/
// local macros & defines
#define GetLong(p,o)  ((((unsigned char*)(p))[o]) | (((unsigned char*)(p))[o+1]<<8) | (((unsigned char*)(p))[o+2]<<16) | (((unsigned char*)(p))[o+3]<<24))

/// AddMessageHeader
//  Parses downloaded message header
static struct MailTransferNode *AddMessageHeader(struct TransferContext *tc, int *count, const int size, const long addr, const char *tfname)
{
  struct MailTransferNode *ret = NULL;
  struct ExtendedMail *email;

  ENTER();

  if((email = MA_ExamineMail(NULL, tfname, FALSE)) != NULL)
  {
    if((mail = CloneMail(&email->Mail)) != NULL)
    {
      struct MailTransferNode *tnode;

      if((tnode = CreateMailTransferNode(mail, TRF_TRANSFER)) != NULL)
      {
        struct Mail *mail = tnode->mail;

        mail->Folder  = NULL;
        mail->Size    = size;

        tnode->index      = ++(*count);
        tnode->importAddr = addr;

        AddMailTransferNode(tc->importList, tnode);
        D(DBF_IMPORT, "added mail '%s' (%ld bytes) to import list.", mail->Subject, size);

        ret = tnode;
      }
      else
      {
        E(DBF_IMPORT, "couldn't allocate enough memory for struct MailTransferNode");
        FreeMail(mail);
      }
    }
    else
      E(DBF_IMPORT, "couldn't allocate enough memory for struct Mail");

    MA_FreeEMailStruct(email);
  }
  else
    E(DBF_IMPORT, "MA_ExamineMail() returned an error!");

  RETURN(ret);
  return ret;
}

///
/// ReadDBXMessage
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
/// ReadDBXMessageInfo
// reads out the message info of a dbx (Outlook Express) Mail Archive file
static BOOL ReadDBXMessageInfo(struct TransferContext *tc, FILE *fh, char *outFileName, unsigned int addr, unsigned int size, int *mail_accu, BOOL preview)
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
    unsigned int offset = body[1] | (body[2] << 8) | (body[3] << 16);

    // check the index value
    if((idx & 0x7f) > sizeof(entries)/sizeof(entries[0]))
    {
      E(DBF_IMPORT, "Wrong index");
      goto out;
    }

    if(idx & 0x80)
    {
      // overwrite the body (and enforce little endian)
      body[0] = offset & 0xff;
      body[1] = (offset & 0xff00) >> 8;
      body[2] = (offset & 0xff0000) >> 16;
      body[3] = 0;

      // is direct value
      entries[idx & 0x7f] = body;
    }
    else
    {
      entries[idx] = data + offset;
    }

    body += 4;
  }

  // Index number 1 points to flags
  if(entries[1])
  {
    unsigned int flags = GetLong(entries[1], 0);

    // check all flags and set the new mail status
    if(flags & (1UL << 5)) // mail has been marked
      setFlag(mailStatusFlags, SFLAG_MARKED);

    if(flags & (1UL << 7)) // mail has been read
      setFlag(mailStatusFlags, SFLAG_READ);

    if(flags & (1UL << 19)) // mail has replied status
      setFlag(mailStatusFlags, SFLAG_REPLIED);
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

  if(rc == TRUE)
  {
    if(preview == TRUE)
    {
      LONG fsize;
      struct MailTransferNode *tnode;

      ObtainFileInfo(outFileName, FI_SIZE, &fsize);

      // if this is the preview run we go and
      // use the TR_AddMessageHeader method to
      // add the found mail to our mail list
      if((tnode = AddMessageHeader(tc, mail_accu, fsize, msg_addr, FilePart(outFileName))) != NULL)
      {
        setFlag(tnode->mail->sflags, mailStatusFlags);
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
/// ReadDBXNode
// Function that reads in a node within the tree of a DBX Mail archive
// file from Outlook Express.
static BOOL ReadDBXNode(struct TransferContext *tc, FILE *fh, char *outFileName, unsigned int addr, int *mail_accu, BOOL preview)
{
  unsigned char *buf;
  unsigned char *body;
  unsigned int child;
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
      if(ReadDBXMessageInfo(tc, fh, outFileName, value, novals, mail_accu, preview) == FALSE)
      {
        free(buf);

        E(DBF_IMPORT, "Failed to read the indexed info");
        RETURN(FALSE);
        return FALSE;
      }
    }

    if(chld)
    {
      if(ReadDBXNode(tc, fh, outFileName, chld, mail_accu, preview) == FALSE)
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
    return ReadDBXNode(tc, fh, outFileName, child, mail_accu, preview);

  RETURN(TRUE);
  return TRUE;
}

///
/// BuildImportList
// build a list of mails in a file to be imported
static void BuildImportList(struct TransferContext *tc, const char *importFile)
{
  BOOL result = FALSE;
  char tfname[SIZE_MFILE];
  char fname[SIZE_PATHFILE];
  int c = 0;

  ENTER();

  // prepare the temporary filename buffers
  snprintf(tfname, sizeof(tfname), "YAMi%08x.tmp", (unsigned int)GetUniqueID());
  AddPath(fname, C->TempDir, tfname, sizeof(fname));

  // before this function is called the MA_ImportMessages() function
  // already found out which import format we can expect. So we
  // distinguish between the different known formats here
  switch(tc->format)
  {
    // treat the file as a MBOX compliant file
    case IMF_MBOX:
    {
      FILE *ifh;

      D(DBF_IMPORT, "trying to retrieve mail list from MBOX compliant file");

      if((ifh = fopen(importFile, "r")) != NULL)
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

              result = (AddMessageHeader(tc, &c, size, addr, tfname) != NULL);
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
          result = FALSE;
          foundBody = FALSE;
        }

        // after quiting the while() loop, we have to check
        // if there is still some data to process
        if(foundBody == TRUE)
        {
          result = (AddMessageHeader(tc, &c, size, addr, tfname) != NULL);
        }
        else if(ofh != NULL)
        {
          fclose(ofh);
          ofh = NULL;
        }

        // delete the temporary file in any case
        DeleteFile(fname);

        fclose(ifh);

        free(buffer);
      }
      else
        E(DBF_IMPORT, "Error on trying to open file '%s'", importFile);
    }
    break;

    // treat the file as a file that contains a single
    // unencoded mail (*.eml alike file)
    case IMF_PLAIN:
    {
      if(CopyFile(fname, NULL, importFile, NULL) == TRUE)
      {
        LONG size;

        ObtainFileInfo(fname, FI_SIZE, &size);
        // if the file was identified as a plain .eml file we
        // just have to go and call TR_AddMessageHeader to let
        // YAM analyze the file
        result = (AddMessageHeader(tc, &c, size, 0, tfname) != NULL);
      }

      // delete the temporary file in any case
      DeleteFile(fname);
    }
    break;

    // treat the file as a DBX (Outlook Express) compliant mail archive
    case IMF_DBX:
    {
      FILE *ifh;

      // lets open the file and read out the root node of the dbx mail file
      if((ifh = fopen(importFile, "rb")) != NULL)
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
              if(ReadDBXNode(tc, ifh, fname, root_node, &c, TRUE) == TRUE && c == number_of_mails)
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
  }
}

///
/// DetectMBoxFormat
// detect the format of a selected file to be imported
static enum ImportFormat DetectMBoxFormat(const char *importFile)
{
  enum ImportFormat format = IMF_UNKNOWN;
  FILE *fh;

  ENTER();

  // check if the file exists or not and if so, open
  // it immediately.
  if((fh = fopen(importFile, "r")) != NULL)
  {
    int i=0;
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // what we do first is to try to find out which
    // file the user tries to import and if it is a valid
    // and supported one.

    // try to identify the file as an MBOX file by trying
    // to find a line starting with "From " in the first 10
    // successive lines.
    D(DBF_IMPORT, "processing MBOX file identification");
    while(i < 10 && getline(&buf, &buflen, fh) > 0)
    {
      if(strncmp(buf, "From ", 5) == 0)
      {
        format = IMF_MBOX;
        break;
      }

      i++;
    }

    // if we still couldn't identify the file
    // we go and try to identify it as a dbx (Outlook Express)
    // message file
    // Please check http://oedbx.aroh.de/ for a recent description
    // of the format!
    if(format == IMF_UNKNOWN)
    {
      unsigned char *file_header;

      D(DBF_IMPORT, "processing DBX file identification");

      // seek the file pointer back
      fseek(fh, 0, SEEK_SET);

      // read the 9404 bytes long file header for properly identifying
      // an Outlook Express database file.
      if((file_header = (unsigned char *)malloc(0x24bc)) !=  NULL)
      {
        if(fread(file_header, 1, 0x24bc, fh) == 0x24bc)
        {
          // try to identify the file as a CLSID_MessageDatabase file
          if((file_header[0] == 0xcf && file_header[1] == 0xad &&
              file_header[2] == 0x12 && file_header[3] == 0xfe) &&
             (file_header[4] == 0xc5 && file_header[5] == 0xfd &&
              file_header[6] == 0x74 && file_header[7] == 0x6f))
          {
            // the file seems to be indeed an Outlook Express
            // message database file (.dbx)
            format = IMF_DBX;
          }
        }

        free(file_header);
      }
    }

    // if we still haven't identified the file we try to find out
    // if it might be just a RAW mail file without a common "From "
    // phrase a MBOX compliant mail file normally contains.
    if(format == IMF_UNKNOWN || format == IMF_MBOX)
    {
      int foundTokens = 0;

      D(DBF_IMPORT, "processing PLAIN mail file identification");

      // seek the file pointer back
      fseek(fh, 0, SEEK_SET);

      // Let's try to find up to 4 known header lines within the first
      // 100 lines which might indicate a valid .mbox file. If we find at
      // least 2 of these this will satisfy us.
      i = 0;
      while(i < 100 && foundTokens < 2 && getline(&buf, &buflen, fh) > 0)
      {
        if(strnicmp(buf, "From:", 5) == 0)
          foundTokens++;
        else if(strnicmp(buf, "To:", 3) == 0)
          foundTokens++;
        else if(strnicmp(buf, "Date:", 5) == 0)
          foundTokens++;
        else if(strnicmp(buf, "Subject:", 8) == 0)
          foundTokens++;

        i++;
      }

      // if we found enough tokens we can set the ImportFormat accordingly.
      if(foundTokens >= 2)
        format = (format == IMF_UNKNOWN ? IMF_PLAIN : IMF_MBOX);
      else
        format = IMF_UNKNOWN;
    }

    fclose(fh);

    free(buf);
  }

  RETURN(format);
  return format;
}

///
/// ProcessImport
static void ProcessImport(struct TransferContext *tc, const char *importFile, struct Folder *folder, const ULONG flags)
{
  struct MailTransferNode *tnode;
  int numberOfMails = 0;
  ULONG totalSize = 0;
  struct Connection *conn;

  ENTER();

  // sum up the mails to be imported and their sizes
  ForEachMailTransferNode(tc->importList, tnode)
  {
    if(isFlagSet(tnode->tflags, TRF_TRANSFER))
    {
      numberOfMails++;
      totalSize += tnode->mail->Size;
    }
  }

  // no socket required
  if((conn = CreateConnection(FALSE)) != NULL)
  {
    snprintf(tc->transferGroupTitle, sizeof(tc->transferGroupTitle), tr(MSG_TR_MsgInFile), importFile);

    if((tc->transferGroup = (Object *)PushMethodOnStackWait(G->App, 6, MUIM_YAMApplication_CreateTransferGroup, CurrentThread(), tc->transferGroupTitle, conn, TRUE, isFlagClear(flags, IMPORTF_QUIET))) != NULL)
    {
      enum FolderType ftype = folder->Type;

      PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Start, numberOfMails, totalSize);

      // now we distinguish between the different import format
      // and import the mails out of it
      switch(tc->format)
      {
        // treat the file as a MBOX compliant file but also
        // in case of plain (*.eml) file we can that the very same
        // routines.
        case IMF_MBOX:
        case IMF_PLAIN:
        {
          FILE *ifh;

          if((ifh = fopen(importFile, "r")) != NULL)
          {
            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            D(DBF_IMPORT, "import mails from MBOX or plain file '%s'", importFile);

            // iterate through our importList and seek to
            // each position/address of a mail
            ForEachMailTransferNode(tc->importList, tnode)
            {
              struct Mail *mail = tnode->mail;
              FILE *ofh = NULL;
              char mfilePath[SIZE_PATHFILE];
              char *buffer = NULL;
              size_t size = 0;
              BOOL foundBody = FALSE;
              unsigned int status = SFLAG_NONE;
              unsigned int xstatus = SFLAG_NONE;
              BOOL ownStatusFound = FALSE;
              ssize_t lineLength;

              if(conn->abort == TRUE)
                break;

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(isFlagClear(tnode->tflags, TRF_TRANSFER))
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, tnode->importAddr, SEEK_SET) != 0)
                break;

              PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, tnode->index, tnode->position, mail->Size, tr(MSG_TR_Importing));

              if(MA_NewMailFile(folder, mfilePath, sizeof(mfilePath)) == FALSE)
                break;
              if((ofh = fopen(mfilePath, "w")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              // now that we seeked to the mail address we go
              // and read in line by line
              while((lineLength = GetLine(&buffer, &size, ifh)) >= 0 && conn->abort == FALSE)
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
                PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, lineLength+1, tr(MSG_TR_Importing));
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
                  status = SFLAG_READ;
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
                  clearFlag(xstatus, SFLAG_NEW);
              }

              // set the status flags now
              setFlag(mail->sflags, status | xstatus);

              // depending on the folder type we have to set the transDate or not
              if(ftype != FT_DRAFTS && ftype != FT_OUTGOING)
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              AddMailToFolder(mail, folder);

              // update the mailFile Path
              strlcpy(mail->MailFile, FilePart(mfilePath), sizeof(mail->MailFile));

              // if this was a compressed/encrypted folder we need to pack the mail now
              if(folder->Mode > FM_SIMPLE)
                RepackMailFile(mail, -1, NULL);

              // update the mailfile accordingly.
              MA_UpdateMailFile(mail);

              // put the transferStat to 100%
              PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Importing));
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

          if((ifh = fopen(importFile, "rb")) != NULL)
          {
            setvbuf(ifh, NULL, _IOFBF, SIZE_FILEBUF);

            D(DBF_IMPORT, "import mails from DBX file '%s'", importFile);

            // iterate through our importList and seek to
            // each position/address of a mail
            ForEachMailTransferNode(tc->importList, tnode)
            {
              struct Mail *mail = tnode->mail;
              FILE *ofh = NULL;
              char mfilePath[SIZE_PATHFILE];

              if(conn->abort == TRUE)
                break;

              // if the mail is not flagged as 'loading' we can continue with the next
              // node
              if(isFlagClear(tnode->tflags, TRF_TRANSFER))
                continue;

              // seek to the file position where the mail resist
              if(fseek(ifh, tnode->importAddr, SEEK_SET) != 0)
                break;

              PushMethodOnStack(tc->transferGroup, 5, MUIM_TransferControlGroup_Next, tnode->index, tnode->position, mail->Size, tr(MSG_TR_Importing));


              if(MA_NewMailFile(folder, mfilePath, sizeof(mfilePath)) == FALSE)
                break;
              if((ofh = fopen(mfilePath, "wb")) == NULL)
                break;

              setvbuf(ofh, NULL, _IOFBF, SIZE_FILEBUF);

              if(ReadDBXMessage(ifh, ofh, tnode->importAddr) == FALSE)
                E(DBF_IMPORT, "Couldn't import dbx message from addr %08lx", tnode->importAddr);

              fclose(ofh);

              // after writing out the mail to a
              // new mail file we go and add it to the folder
              if(mail->sflags != SFLAG_NONE)
              {
                unsigned int stat = SFLAG_NONE;

                // define the default status flags depending on the
                // folder
                if(ftype == FT_OUTGOING)
                  stat = SFLAG_READ;
                else if(ftype == FT_SENT || ftype == FT_CUSTOMSENT)
                  stat = SFLAG_SENT | SFLAG_READ;
                else
                  stat = SFLAG_NEW;

                setFlag(mail->sflags, stat);
              }

              // depending on the folder type we have to set the transDate or not
              if(ftype != FT_DRAFTS && ftype != FT_OUTGOING)
                GetSysTimeUTC(&mail->transDate);

              // add the mail to the folderlist now
              AddMailToFolder(mail, folder);

              // update the mailFile Path
              strlcpy(mail->MailFile, FilePart(mfilePath), sizeof(mail->MailFile));

              // if this was a compressed/encrypted folder we need to pack the mail now
              if(folder->Mode > FM_SIMPLE)
                RepackMailFile(mail, -1, NULL);

              // update the mailfile accordingly.
              MA_UpdateMailFile(mail);

              // put the transferStat to 100%
              PushMethodOnStack(tc->transferGroup, 3, MUIM_TransferControlGroup_Update, TCG_SETMAX, tr(MSG_TR_Importing));
            }

            fclose(ifh);
          }

          case IMF_UNKNOWN:
            // nothing
          break;
        }
        break;
      }

      PushMethodOnStack(tc->transferGroup, 1, MUIM_TransferControlGroup_Finish);
    }

    PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DeleteTransferGroup, tc->transferGroup);

    AppendToLogfile(LF_ALL, 50, tr(MSG_LOG_Importing), numberOfMails, importFile, folder->Name);
    PushMethodOnStack(G->App, 3, MUIM_YAMApplication_DisplayStatistics, folder, TRUE);
    PushMethodOnStack(G->App, 3, MUIM_YAMApplication_ChangeFolder, NULL, FALSE);
  }

  DeleteConnection(conn);

  LEAVE();
}

///
/// ImportMails
//  Import mails from a file
BOOL ImportMails(const char *importFile, struct Folder *folder, const ULONG flags)
{
  BOOL success = FALSE;
  struct TransferContext *tc;

  ENTER();

  D(DBF_IMPORT, "import mails from file '%s' to folder '%s', flags %08lx", importFile, folder->Name, flags);

  if((tc = calloc(1, sizeof(*tc))) != NULL)
  {
    if((tc->format = DetectMBoxFormat(importFile)) != IMF_UNKNOWN)
    {
      if((tc->importList = CreateMailTransferList()) != NULL)
      {
        // being able to open the file is enough to signal success
        success = TRUE;

        BuildImportList(tc, importFile);

        if(IsMailTransferListEmpty(tc->importList) == FALSE)
        {
          BOOL doImport = FALSE;

          if(isFlagClear(flags, IMPORTF_QUIET) || isFlagSet(flags, IMPORTF_WAIT))
          {
            // show the preselection window in case user interaction is requested
            Object *preselectWin;

            snprintf(tc->windowTitle, sizeof(tc->windowTitle), tr(MSG_TR_MsgInFile), importFile);

            if((preselectWin = (Object *)PushMethodOnStackWait(G->App, 6, MUIM_YAMApplication_CreatePreselectionWindow, CurrentThread(), tc->windowTitle, 0, PRESELWINMODE_IMPORT, tc->importList)) != NULL)
            {
              if(SleepThread() == TRUE)
              {
                ULONG result = FALSE;

                PushMethodOnStackWait(preselectWin, 3, OM_GET, MUIA_PreselectionWindow_Result, &result);
                if(result == TRUE)
                {
                  doImport = TRUE;
                }
              }

              PushMethodOnStack(G->App, 2, MUIM_YAMApplication_DisposeWindow, preselectWin);
            }
          }
          else
          {
            // otherwise we perform the import no matter what
            doImport = TRUE;
          }

          SHOWVALUE(DBF_IMPORT, doImport);

          if(doImport == TRUE)
            ProcessImport(tc, importFile, folder, flags);
        }
        else
        {
          // the file did not contain any mails to import
          ER_NewError(tr(MSG_IMPORT_NO_MAILS), importFile);
        }

        DeleteMailTransferList(tc->importList);
      }
    }
    else
    {
      // known file format
      ER_NewError(tr(MSG_IMPORT_UNKNOWN_FILE_FORMAT), importFile);
    }

    free(tc);
  }

  // wake up the calling thread if this is requested
  if(isFlagSet(flags, IMPORTF_SIGNAL))
    WakeupThread(NULL);

  RETURN(success);
  return success;
}

///

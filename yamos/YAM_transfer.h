#ifndef YAM_TRANSFER_H
#define YAM_TRANSFER_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

enum TransferType {TR_IMPORT,TR_EXPORT,TR_GET,TR_SEND};

struct DownloadResult
{
   long Downloaded;
   long OnServer;
   long DupSkipped;
   long Deleted;
   BOOL Error;
};

struct TR_GUIData
{
   APTR WI;
   APTR GR_LIST;
   APTR GR_PAGE;
   APTR LV_MAILS;
   APTR BT_PAUSE;
   APTR BT_RESUME;
   APTR BT_QUIT;
   APTR BT_START;
   APTR TX_STATS;
   APTR TX_STATUS;
   APTR GA_COUNT;
   APTR GA_BYTES;
   APTR BT_ABORT;
   char *ST_STATUS;
};

struct TR_ClassData  /* transfer window */
{
   struct TR_GUIData     GUI;
   struct Mail *         List;
   struct Mail *         GMD_Mail;
   struct Search *       Search[2*MAXRU];
   struct Folder *       ImportBox;
   char *                UIDLloc;

   long                  Abort;
   long                  Pause;
   long                  Start;
   int                   Scnt;
   int                   GMD_Line;
   int                   GUIlevel;
   int                   POP_Nr;
   BOOL                  SinglePOP;
   BOOL                  Checking;
   BOOL                  supportUIDL;
   struct DownloadResult Stats;

   char                  WTitle[SIZE_DEFAULT];
   char                  ImportFile[SIZE_PATHFILE];
   char                  CountLabel[SIZE_DEFAULT];
   char                  BytesLabel[SIZE_DEFAULT];
   char                  StatsLabel[SIZE_DEFAULT];
};

extern struct Hook TR_ProcessGETHook;
extern struct Hook TR_ProcessIMPORTHook;

void  TR_Cleanup(void);
void  TR_CloseTCPIP(void);
BOOL  TR_DownloadURL(char *url0, char *url1, char *url2, char *filename);
void  TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, int guilevel);
void  TR_GetMessageList_IMPORT(FILE *fh);
BOOL  TR_IsOnline(void);
struct TR_ClassData *TR_New(enum TransferType TRmode);
BOOL  TR_OpenTCPIP(void);
BOOL  TR_ProcessEXPORT(char *fname, struct Mail **mlist, BOOL append);
BOOL  TR_ProcessSEND(struct Mail **mlist);
void  TR_SetWinTitle(BOOL from, char *host);

#endif /* YAM_TRANSFER_H */

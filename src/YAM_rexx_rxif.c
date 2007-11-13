/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <exec/memory.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <rexx/rxslib.h>
#include <rexx/storage.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mime.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_rexx_rxcl.h"
#include "YAM_write.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "FileInfo.h"
#include "extrasrc.h"

#include "Debug.h"

extern struct Library *SocketBase;

/** REXX Commands **/

/// rx_show
void rx_show( UNUSED struct RexxHost *host, struct rxd_show **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_show *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      // lets signal the application to uniconify and open the windows
      PopUp();
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_hide
void rx_hide( UNUSED struct RexxHost *host, struct rxd_hide **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_hide *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      set(G->App, MUIA_Application_Iconified, TRUE);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_quit
void rx_quit( UNUSED struct RexxHost *host, struct rxd_quit **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_quit *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(rd->arg.force)
        set(G->App, MUIA_Application_ForceQuit, TRUE);

      DoMethod(G->App, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_help
void rx_help( UNUSED struct RexxHost *host, struct rxd_help **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_help *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct rxs_command *rxc;
      FILE *fp = NULL;
      FILE *out = stdout;

      if(rd->arg.file && (fp = fopen(rd->arg.file, "w")))
        out = fp;

      fprintf(out, "Commands for application \"YAM\"\n\nCommand          Template\n-------          --------\n");

      for(rxc = rxs_commandlist; rxc->command; rxc++)
      {
        fprintf(out, "%-16s%c%s%s%s%s%s\n", rxc->command,
                                            (rxc->results || rxc->args) ? ' ' : '\0', rxc->results ? "VAR/K,STEM/K" : "",
                                            (rxc->results && rxc->args) ? "," : "", rxc->args ? rxc->args : "",
                                            rxc->results ? " => " : "", rxc->results ? rxc->results : "");
      }

      if(fp)
        fclose(fp);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_info
void rx_info( UNUSED struct RexxHost *host, struct rxd_info **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_info *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      char *key = rd->arg.item;

      if(!key)  rd->rc = RETURN_ERROR;
      else if(!stricmp(key, "title"))       rd->res.value = (char *)xget(G->App, MUIA_Application_Title);
      else if(!stricmp(key, "author"))      rd->res.value = (char *)xget(G->App, MUIA_Application_Author);
      else if(!stricmp(key, "copyright"))   rd->res.value = (char *)xget(G->App, MUIA_Application_Copyright);
      else if(!stricmp(key, "description")) rd->res.value = (char *)xget(G->App, MUIA_Application_Description);
      else if(!stricmp(key, "version"))     rd->res.value = (char *)xget(G->App, MUIA_Application_Version);
      else if(!stricmp(key, "base"))        rd->res.value = (char *)xget(G->App, MUIA_Application_Base);
      else if(!stricmp(key, "screen"))
      {
        struct Screen *screen;
        rd->res.value = "Workbench";
        screen = (struct Screen *)xget(G->MA->GUI.WI, MUIA_Window_Screen);

        if(screen)
        {
          struct Node *pubs;
          struct List *pubscreens = LockPubScreenList();

          for(pubs = pubscreens->lh_Head; pubs->ln_Succ; pubs = pubs->ln_Succ)
          {
            if(((struct PubScreenNode *)pubs)->psn_Screen == screen)
            {
              rd->res.value = pubs->ln_Name;
              break;
            }
          }

          UnlockPubScreenList();
        }
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeto
void rx_writeto( UNUSED struct RexxHost *host, struct rxd_writeto **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writeto *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_TO, rd->arg.address, (BOOL)rd->arg.add);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writecc
void rx_writecc( UNUSED struct RexxHost *host, struct rxd_writecc **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writecc *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_CC, rd->arg.address, (BOOL)rd->arg.add);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writebcc
void rx_writebcc( UNUSED struct RexxHost *host, struct rxd_writebcc **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writebcc *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_BCC, rd->arg.address, (BOOL)rd->arg.add);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeattch
void rx_writeattach( UNUSED struct RexxHost *host, struct rxd_writeattach **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writeattach *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(FileExists(rd->arg.file) == TRUE && G->WR[G->ActiveWriteWin])
      {
        WR_AddFileToList((int)G->ActiveWriteWin, rd->arg.file, NULL, FALSE);

        if(rd->arg.desc)    setstring(G->WR[G->ActiveWriteWin]->GUI.ST_DESC, rd->arg.desc);
        if(rd->arg.encmode) setmutex (G->WR[G->ActiveWriteWin]->GUI.RA_ENCODING, !strnicmp(rd->arg.encmode, "uu", 2) ? 1 : 0);
        if(rd->arg.ctype)   setstring(G->WR[G->ActiveWriteWin]->GUI.ST_CTYPE, rd->arg.ctype);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeletter
void rx_writeletter( UNUSED struct RexxHost *host, struct rxd_writeletter **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writeletter *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin] && CopyFile(G->WR_Filename[G->ActiveWriteWin], 0, rd->arg.file, 0))
      {
        if(C->UseSignature && !rd->arg.nosig)
          WR_AddSignature(G->ActiveWriteWin, -1);

        FileToEditor(G->WR_Filename[G->ActiveWriteWin], G->WR[G->ActiveWriteWin]->GUI.TE_EDIT);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeoptions
void rx_writeoptions( UNUSED struct RexxHost *host, struct rxd_writeoptions **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writeoptions *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
      {
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_DELSEND, rd->arg.delete);
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_MDN, rd->arg.receipt);
        setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_ADDINFO, rd->arg.addinfo);
        if(rd->arg.importance) setcycle(G->WR[G->ActiveWriteWin]->GUI.CY_IMPORTANCE, *rd->arg.importance);
        if(rd->arg.sig)        setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SIGNATURE, *rd->arg.sig);
        if(rd->arg.security)   setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SECURITY, *rd->arg.security);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writequeue
void rx_writequeue( UNUSED struct RexxHost *host, struct rxd_writequeue **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writequeue *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        WR_NewMail(rd->arg.hold ? WRITE_HOLD : WRITE_QUEUE, (int)G->ActiveWriteWin);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writesend
void rx_writesend( UNUSED struct RexxHost *host, struct rxd_writesend **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writesend *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        WR_NewMail(WRITE_SEND, (int)G->ActiveWriteWin);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailwrite
void rx_mailwrite( UNUSED struct RexxHost *host, struct rxd_mailwrite **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailwrite *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int winnr = rd->arg.window ? *rd->arg.window : -1;
      rd->res.window = &G->ActiveWriteWin;

      if(winnr < 0)
      {
        if((winnr = MA_NewMessage(NEW_NEW, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
        {
          G->ActiveWriteWin = winnr;

          if(rd->arg.quiet == FALSE && G->WR[winnr])
            set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
        }
        else
          rd->rc = RETURN_ERROR;
      }
      else
      {
        if(winnr >= 0 && winnr <= 1)
        {
          if(G->WR[winnr])
          {
            G->ActiveWriteWin = winnr;

            if(rd->arg.quiet == FALSE && G->WR[winnr])
              set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
          }
          else
            rd->rc = RETURN_WARN;
        }
        else
          rd->rc = RETURN_ERROR;
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailreply
void rx_mailreply( UNUSED struct RexxHost *host, struct rxd_mailreply **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailreply *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int winnr;

      rd->res.window = &G->ActiveWriteWin;

      if((winnr = MA_NewMessage(NEW_REPLY, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
      {
        G->ActiveWriteWin = winnr;

        if(rd->arg.quiet == FALSE && G->WR[winnr])
          set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailforward
void rx_mailforward( UNUSED struct RexxHost *host, struct rxd_mailforward **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailforward *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int winnr;

      rd->res.window = &G->ActiveWriteWin;

      if((winnr = MA_NewMessage(NEW_FORWARD, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
      {
        G->ActiveWriteWin = winnr;

        if(rd->arg.quiet == FALSE && G->WR[winnr])
          set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailmove
void rx_mailmove( UNUSED struct RexxHost *host, struct rxd_mailmove **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailmove *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *folder;

      if((folder = FO_GetFolderRexx(rd->arg.folder, NULL)))
        MA_MoveCopy(NULL, FO_GetCurrentFolder(), folder, FALSE, TRUE);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailread
void rx_mailread( UNUSED struct RexxHost *host, struct rxd_mailread **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailread *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      static int winNumber = -1;

      rd->res.window = &winNumber;

      if(rd->arg.window == NULL)
      {
        struct Mail *mail;

        if((mail = MA_GetActiveMail(NULL, NULL, NULL)) != NULL)
        {
          struct ReadMailData *rmData;

          if((rmData = CreateReadWindow(TRUE)) != NULL)
          {
            G->ActiveRexxRMData = rmData;

            if(rd->arg.quiet == FALSE)
              SafeOpenWindow(rmData->readWindow);

            if(DoMethod(rmData->readWindow, MUIM_ReadWindow_ReadMail, mail) == FALSE)
            {
              // on any error we make sure to delete the read window
              // immediatly again.
              CleanupReadMailData(rmData, TRUE);

              rd->rc = RETURN_ERROR;
            }
            else
            {
              // Set the active Rexx RMData again, as this might have been overwritten
              // in the meantime by the read window itself. In this case the read window
              // will try to cleanup all the stuff allocated for a previous mail.
              G->ActiveRexxRMData = rmData;
              winNumber = xget(rmData->readWindow, MUIA_ReadWindow_Num);
            }
          }
          else
            rd->rc = RETURN_ERROR;
        }
        else
          rd->rc = RETURN_WARN;
      }
      else
      {
        // if a window number was specified with the command we have to search
        // through our ReadDataList and find the window with this particular
        // number
        int winnr = *rd->arg.window;
        struct MinNode *curNode = G->readMailDataList.mlh_Head;

        for(; curNode->mln_Succ; curNode = curNode->mln_Succ)
        {
          struct ReadMailData *rmData = (struct ReadMailData *)curNode;

          if(rmData->readWindow != NULL &&
             (int)xget(rmData->readWindow, MUIA_ReadWindow_Num) == winnr)
          {
            G->ActiveRexxRMData = rmData;
            winNumber = winnr;

            // bring the window to the user's attention
            if(rd->arg.quiet == FALSE)
              set(rmData->readWindow, MUIA_Window_Activate, TRUE);

            break;
          }
        }

        // check if we successfully found the window with that
        // number or if we have to return an error message
        if(curNode->mln_Succ == NULL)
          rd->rc = RETURN_ERROR;
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailsend
void rx_mailsend( UNUSED struct RexxHost *host, struct rxd_mailsend **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailsend *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!MA_Send(rd->arg.all ? SEND_ALL_AUTO : SEND_ACTIVE_AUTO))
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_maildelete
void rx_maildelete( UNUSED struct RexxHost *host, struct rxd_maildelete **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_maildelete *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      MA_DeleteMessage((BOOL)rd->arg.atonce, (BOOL)rd->arg.force);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailcheck
void rx_mailcheck( UNUSED struct RexxHost *host, struct rxd_mailcheck **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailcheck *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int pop;
      int popnr = -2;

      if(rd->arg.pop)
      {
        if((pop = *rd->arg.pop) >= 0 && pop < MAXP3 && C->P3[pop])
          popnr = pop;
      }
      else
        popnr = -1;

      if(popnr > -2)
      {
        static long remaining;

        MA_PopNow(rd->arg.manual ? POP_USER : POP_REXX, popnr);

        remaining = G->LastDL.OnServer - G->LastDL.Deleted;

        rd->res.downloaded = &G->LastDL.Downloaded;
        rd->res.onserver = &remaining;
        rd->res.dupskipped = &G->LastDL.DupSkipped;
        rd->res.deleted = &G->LastDL.Deleted;

        if(G->LastDL.Error)
          rd->rc = RETURN_WARN;
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailimport
void rx_mailimport( UNUSED struct RexxHost *host, struct rxd_mailimport **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailimport *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!MA_ImportMessages(rd->arg.filename))
        rd->rc = RETURN_ERROR;
      else if(!rd->arg.wait)
        CallHookPkt(&TR_ProcessIMPORTHook, 0, 0);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailexport
void rx_mailexport( UNUSED struct RexxHost *host, struct rxd_mailexport **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailexport *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!MA_ExportMessages((BOOL)rd->arg.all, rd->arg.filename, (BOOL)rd->arg.append))
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailupdate
void rx_mailupdate( UNUSED struct RexxHost *host, struct rxd_mailupdate **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailupdate *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      DoMethod(G->App, MUIM_CallHook, &MA_RescanIndexHook);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailfilter
void rx_mailfilter( UNUSED struct RexxHost *host, struct rxd_mailfilter **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailfilter *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct RuleResult *rr = &G->RRs;

      DoMethod(G->App, MUIM_CallHook, &ApplyFiltersHook, rd->arg.all ? APPLY_RX_ALL : APPLY_RX, 0);

      rd->res.checked = &rr->Checked;
      rd->res.bounced = &rr->Bounced;
      rd->res.forwarded = &rr->Forwarded;
      rd->res.replied = &rr->Replied;
      rd->res.executed = &rr->Executed;
      rd->res.moved = &rr->Moved;
      rd->res.deleted = &rr->Deleted;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailinfo
void rx_mailinfo( UNUSED struct RexxHost *host, struct rxd_mailinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
     struct rxd_mailinfo rd;
     long active;
     char from[SIZE_ADDRESS], to[SIZE_ADDRESS], replyto[SIZE_ADDRESS], flags[SIZE_SMALL];
     char filename[SIZE_PATHFILE], date[64], msgid[9];
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail = NULL;
      struct Folder *folder = NULL;

      if(rd->rd.arg.index)
      {
        Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
        rd->active = *rd->rd.arg.index;
        DoMethod(lv, MUIM_NList_GetEntry, rd->active, &mail);
      }
      else
        mail = MA_GetActiveMail(NULL, &folder, (LONG *)&rd->active);

      if(mail)
      {
        int pf = getPERValue(mail);
        int vf = getVOLValue(mail);

        GetMailFile(rd->rd.res.filename = rd->filename, folder, mail);
        rd->rd.res.index = &rd->active;

        if(hasStatusError(mail))
          rd->rd.res.status = "E"; // Error status
        else if(hasStatusQueued(mail))
          rd->rd.res.status = "W"; // Queued (WaitForSend) status
        else if(hasStatusHold(mail))
          rd->rd.res.status = "H"; // Hold status
        else if(hasStatusSent(mail))
          rd->rd.res.status = "S"; // Sent status
        else if(hasStatusReplied(mail))
          rd->rd.res.status = "R"; // Replied status
        else if(hasStatusForwarded(mail))
          rd->rd.res.status = "F"; // Forwarded status
        else if(!hasStatusRead(mail))
        {
          if(hasStatusNew(mail))
            rd->rd.res.status = "N"; // New status
          else
            rd->rd.res.status = "U"; // Unread status
        }
        else if(!hasStatusNew(mail))
          rd->rd.res.status = "O"; // Old status

        strlcpy(rd->rd.res.from = rd->from, AB_BuildAddressStringPerson(&mail->From), sizeof(rd->from));
        strlcpy(rd->rd.res.to = rd->to, AB_BuildAddressStringPerson(&mail->To), sizeof(rd->to));
        strlcpy(rd->rd.res.replyto = rd->replyto, AB_BuildAddressStringPerson(mail->ReplyTo.Address[0] != '\0' ? &mail->ReplyTo : &mail->From), sizeof(rd->replyto));
        DateStamp2String(rd->rd.res.date = rd->date, sizeof(rd->date), &mail->Date, DSS_USDATETIME, TZC_LOCAL);
        rd->rd.res.subject = mail->Subject;
        rd->rd.res.size = &mail->Size;
        snprintf(rd->rd.res.msgid = rd->msgid, sizeof(rd->msgid), "%lX", mail->cMsgID);
        snprintf(rd->rd.res.flags = rd->flags, sizeof(rd->flags), "%c%c%c%c%c-%c%c%c",
                  isMultiRCPTMail(mail) ? 'M' : '-',
                  isMP_MixedMail(mail)  ? 'A' : '-',
                  isMP_ReportMail(mail) ? 'R' : '-',
                  isMP_CryptedMail(mail)? 'C' : '-',
                  isMP_SignedMail(mail) ? 'S' : '-',
                  pf ? pf+'0' : '-',
                  vf ? vf+'0' : '-',
                  hasStatusMarked(mail) ? 'M' : '-'
               );
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_setfolder
void rx_setfolder( UNUSED struct RexxHost *host, struct rxd_setfolder **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_setfolder *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *folder;

      if((folder = FO_GetFolderRexx(rd->arg.folder, NULL)))
        MA_ChangeFolder(folder, TRUE);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_setmail
void rx_setmail( UNUSED struct RexxHost *host, struct rxd_setmail **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_setmail *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int mail, max;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

      mail = *rd->arg.num;
      max = xget(lv, MUIA_NList_Entries);

      if(mail < 0 || mail >= max)
        rd->rc = RETURN_ERROR;
      else
        set(lv, MUIA_NList_Active, mail);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeeditor
void rx_writeeditor( UNUSED struct RexxHost *host, struct rxd_writeeditor **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writeeditor *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
      {
        ULONG p = DoMethod(G->WR[G->ActiveWriteWin]->GUI.TE_EDIT, MUIM_TextEditor_ARexxCmd, rd->arg.command);

        switch(p)
        {
          case FALSE: rd->rc = RETURN_WARN; break;
          case TRUE:  break;
          default:    rd->res.result = (char *)p; break;
        }
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.result)
        FreeVec(rd->res.result);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_request
void rx_request( UNUSED struct RexxHost *host, struct rxd_request **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
     struct rxd_request rd;
     long result;
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      char *reqtext = AllocReqText(rd->rd.arg.body);

      rd->result = MUI_Request(G->App, NULL, 0, NULL, rd->rd.arg.gadgets, reqtext);
      rd->rd.res.result = &rd->result;

      free(reqtext);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailarchive
void rx_mailarchive( UNUSED struct RexxHost *host, struct rxd_mailarchive **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  rx_mailmove(host, (struct rxd_mailmove **)rxd, action, rexxmsg);
}

///
/// rx_mailsendall
void rx_mailsendall( UNUSED struct RexxHost *host, struct rxd_mailsendall **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailsendall *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!MA_Send(SEND_ALL_AUTO))
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_getfolderinfo
void rx_getfolderinfo( UNUSED struct RexxHost *host, struct rxd_getfolderinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_getfolderinfo rd;
    char result[SIZE_SMALL];
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *fo = FO_GetCurrentFolder();
      char *key = rd->rd.arg.item;

      // this command should only act on a folder folder and
      // also only on a non-group
      if(fo && !isGroupFolder(fo))
      {
        int num = FO_GetFolderPosition(fo, FALSE);

        if(!strnicmp(key, "NUM", 3))      snprintf(rd->rd.res.value = rd->result, sizeof(rd->result), "%d", num);
        else if(!strnicmp(key, "NAM", 3)) rd->rd.res.value = fo->Name;
        else if(!strnicmp(key, "PAT", 3)) rd->rd.res.value = fo->Path;
        else if(!strnicmp(key, "MAX", 3)) snprintf(rd->rd.res.value = rd->result, sizeof(rd->result), "%d", fo->Total);
        else rd->rd.rc = RETURN_ERROR;
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_getmailinfo
void rx_getmailinfo( UNUSED struct RexxHost *host, struct rxd_getmailinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_getmailinfo rd;
    char result[SIZE_LARGE];
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail;
      LONG active = 0;

      if((mail = MA_GetActiveMail(NULL, NULL, &active)))
      {
        char *key;

        rd->rd.res.value = rd->result;
        key = rd->rd.arg.item;

        if(!strnicmp(key, "ACT", 3))
          snprintf(rd->result, sizeof(rd->result), "%ld", active);
        else if(!strnicmp(key, "STA", 3))
        {
          if(hasStatusError(mail))
            rd->rd.res.value = "E"; // Error status
          else if(hasStatusQueued(mail))
            rd->rd.res.value = "W"; // Queued (WaitForSend) status
          else if(hasStatusHold(mail))
            rd->rd.res.value = "H"; // Hold status
          else if(hasStatusSent(mail))
            rd->rd.res.value = "S"; // Sent status
          else if(hasStatusReplied(mail))
            rd->rd.res.value = "R"; // Replied status
          else if(hasStatusForwarded(mail))
            rd->rd.res.value = "F"; // Forwarded status
          else if(!hasStatusRead(mail))
          {
            if(hasStatusNew(mail))
              rd->rd.res.value = "N"; // New status
            else
              rd->rd.res.value = "U"; // Unread status
          }
          else if(!hasStatusNew(mail))
            rd->rd.res.value = "O"; // Old status
        }
        else if(!strnicmp(key, "FRO", 3)) strlcpy(rd->result, AB_BuildAddressStringPerson(&mail->From), sizeof(rd->result));
        else if(!strnicmp(key, "TO" , 2)) strlcpy(rd->result, AB_BuildAddressStringPerson(&mail->To), sizeof(rd->result));
        else if(!strnicmp(key, "REP", 3)) strlcpy(rd->result, AB_BuildAddressStringPerson(mail->ReplyTo.Address[0] != '\0' ? &mail->ReplyTo : &mail->From), sizeof(rd->result));
        else if(!strnicmp(key, "SUB", 3)) rd->rd.res.value = mail->Subject;
        else if(!strnicmp(key, "FIL", 3))
        {
          GetMailFile(rd->result, mail->Folder, mail);
          rd->rd.res.value = rd->result;
        }
        else
          rd->rd.rc = RETURN_ERROR;
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_getconfiginfo
void rx_getconfiginfo( UNUSED struct RexxHost *host, struct rxd_getconfiginfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_getconfiginfo *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      char *key = rd->arg.item;

      if(!strnicmp(key, "NAM", 3))
        rd->res.value = C->RealName;
      else if(!strnicmp(key, "EMA", 3))
        rd->res.value = C->EmailAddress;
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_folderinfo
void rx_folderinfo( UNUSED struct RexxHost *host, struct rxd_folderinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_folderinfo *rd = *rxd;

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *fo = rd->arg.folder ? FO_GetFolderRexx(rd->arg.folder, NULL) : FO_GetCurrentFolder();

      // this command should only act on a folder and
      // only on a non-group
      if(fo && !isGroupFolder(fo))
      {
        static int num;
        static int total;
        static int new;
        static int unread;
        static LONG size;
        static int type;

        num = FO_GetFolderPosition(fo, FALSE);
        total = fo->Total;
        new = fo->New;
        unread = fo->Unread;
        size = fo->Size;
        type = fo->Type;

        rd->res.number = &num;
        rd->res.name = fo->Name;
        rd->res.path = fo->Path;
        rd->res.total = &total;
        rd->res.new = &new;
        rd->res.unread = &unread;
        rd->res.size = &size;
        rd->res.type = &type;
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writesubject
void rx_writesubject( UNUSED struct RexxHost *host, struct rxd_writesubject **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writesubject *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        setstring(G->WR[G->ActiveWriteWin]->GUI.ST_SUBJECT, rd->arg.subject);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_screentoback
void rx_screentoback( UNUSED struct RexxHost *host, struct rxd_screentoback **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_screentoback *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      rd->rc = 0;

      if(G->MA)
        DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToBack);
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_screentofront
void rx_screentofront( UNUSED struct RexxHost *host, struct rxd_screentofront **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_screentofront *rd = *rxd;

  ENTER();

  switch( action )
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      rd->rc = 0;

      if(G->MA)
        DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToFront);
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_setflag
void rx_setflag( UNUSED struct RexxHost *host, struct rxd_setflag **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_setflag *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int value;
      struct Mail *mail;

      if((mail = MA_GetActiveMail(NULL, NULL, NULL)))
      {
        if(rd->arg.vol)
        {
          if((value = *rd->arg.vol) >= 0 && value < 8)
            setVOLValue(mail, value);
          else
            rd->rc = RETURN_ERROR;
        }

        if(rd->arg.per)
        {
          if((value = *rd->arg.per) >= 0 && value < 8)
          {
            if((unsigned int)value != getPERValue(mail))
            {
              setPERValue(mail, value);

              MA_UpdateMailFile(mail);
            }
          }
          else
            rd->rc = RETURN_ERROR;
        }
      }
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailedit
void rx_mailedit( UNUSED struct RexxHost *host, struct rxd_mailedit **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailedit *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int winnr;

      rd->res.window = &G->ActiveWriteWin;

      if((winnr = MA_NewMessage(NEW_EDIT, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
      {
         G->ActiveWriteWin = winnr;
         if(rd->arg.quiet == FALSE && G->WR[winnr])
           set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_readinfo
void rx_readinfo( UNUSED struct RexxHost *host, struct rxd_readinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_readinfo *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
      {
        struct Part *part;
        int i, parts;

        for(parts = 0, part = rmData->firstPart->Next; part; parts++, part = part->Next);

        rd->res.filename = calloc(parts+1, sizeof(char *));
        rd->res.filetype = calloc(parts+1, sizeof(char *));
        rd->res.filesize = calloc(parts+1, sizeof(long));
        rd->res.tempfile = calloc(parts+1, sizeof(char *));

        for(i = 0, part = rmData->firstPart->Next; part; i++, part = part->Next)
        {
          rd->res.filename[i] = part->Name;
          rd->res.filetype[i] = part->ContentType;
          rd->res.filesize[i] = (long *)&part->Size;
          rd->res.tempfile[i] = part->Filename;
        }
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.filename) free(rd->res.filename);
      if(rd->res.filetype) free(rd->res.filetype);
      if(rd->res.filesize) free(rd->res.filesize);
      if(rd->res.tempfile) free(rd->res.tempfile);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_readsave
void rx_readsave( UNUSED struct RexxHost *host, struct rxd_readsave **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_readsave *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      BOOL success = FALSE;
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
      {
        struct TempFile *tf;

        if(rd->arg.part)
        {
          struct Part *part;

          for(part = rmData->firstPart->Next; part; part = part->Next)
          {
            if(part->Nr == *(rd->arg.part) &&
               RE_DecodePart(part))
            {
              char file[SIZE_PATHFILE];

              strmfp(file, C->DetachDir, part->Name);

              success = RE_Export(rmData,
                                  part->Filename,
                                  rd->arg.filename ? rd->arg.filename : "",
                                  part->Name,
                                  part->Nr,
                                  TRUE,
                                  (BOOL)rd->arg.overwrite,
                                  part->ContentType);
            }
          }
        }
        else if((tf = OpenTempFile("w")))
        {
          DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, tf->FP);

          fclose(tf->FP);
          tf->FP = NULL;

          success = RE_Export(rmData,
                              tf->Filename,
                              rd->arg.filename ? rd->arg.filename : "",
                              "",
                              0,
                              TRUE,
                              (BOOL)rd->arg.overwrite,
                              IntMimeTypeArray[MT_TX_PLAIN].ContentType);

          CloseTempFile(tf);
        }
      }

      if(!success)
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_readprint
void rx_readprint( UNUSED struct RexxHost *host, struct rxd_readprint **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_readprint *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct ReadMailData *rmData = G->ActiveRexxRMData;
      BOOL success = FALSE;

      if(rmData)
      {
        FILE *prt;

        if(rd->arg.part)
        {
          struct Part *part;

          for(part = rmData->firstPart->Next; part; part = part->Next)
          {
            if(part->Nr == *(rd->arg.part) &&
               RE_DecodePart(part))
            {
              success = CopyFile("PRT:", 0, part->Filename, 0);
            }
          }
        }
        else if((prt = fopen("PRT:", "w")))
        {
          setvbuf(prt, NULL, _IOFBF, SIZE_FILEBUF);

          DoMethod(rmData->readMailGroup, MUIM_ReadMailGroup_SaveDisplay, prt);

          if(ferror(prt) == 0)
            success = TRUE;

          fclose(prt);
        }
      }

      if(success == FALSE)
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailbounce
void rx_mailbounce( UNUSED struct RexxHost *host, struct rxd_mailbounce **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailbounce *rd = *rxd;

  ENTER();

  switch( action )
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int winnr;

      rd->res.window = &G->ActiveWriteWin;

      if((winnr = MA_NewMessage(NEW_BOUNCE, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
      {
        G->ActiveWriteWin = winnr;
        if(rd->arg.quiet == FALSE && G->WR[winnr])
          set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrfind
void rx_addrfind( UNUSED struct RexxHost *host, struct rxd_addrfind **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrfind *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      int hits;
      int mode;

      if(rd->arg.nameonly)
        mode = rd->arg.emailonly ? ABF_RX_NAMEEMAIL : ABF_RX_NAME;
      else
        mode = rd->arg.emailonly ? ABF_RX_EMAIL     : ABF_RX;

      if((hits = AB_FindEntry(rd->arg.pattern, mode, NULL)) > 0)
      {
        rd->res.alias = calloc(hits+1, sizeof(char *));
        if(AB_FindEntry(rd->arg.pattern, mode, rd->res.alias) == 0)
          rd->rc = RETURN_WARN;
      }
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.alias)
        free(rd->res.alias);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrinfo
void rx_addrinfo( UNUSED struct RexxHost *host, struct rxd_addrinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_addrinfo rd;
    char *members, **memberptr;
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct ABEntry *ab = NULL;

      if(AB_SearchEntry(rd->rd.arg.alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &ab) && (ab != NULL))
      {
        switch(ab->Type)
        {
          case AET_USER:
            rd->rd.res.type = "P";
          break;

          case AET_LIST:
            rd->rd.res.type = "L";
          break;

          case AET_GROUP:
            rd->rd.res.type = "G";
          break;
        }

        rd->rd.res.name = ab->RealName;
        rd->rd.res.email = ab->Address;
        rd->rd.res.pgp = ab->PGPId;
        rd->rd.res.homepage = ab->Homepage;
        rd->rd.res.street = ab->Street;
        rd->rd.res.city = ab->City;
        rd->rd.res.country = ab->Country;
        rd->rd.res.phone = ab->Phone;
        rd->rd.res.comment = ab->Comment;
        rd->rd.res.birthdate = &ab->BirthDay;
        rd->rd.res.image = ab->Photo;

        if(ab->Members && (rd->members = strdup(ab->Members)))
        {
          char *ptr;
          int i;
          int j;

          for(j = 0, ptr = rd->members; *ptr; j++, ptr++)
          {
            if((ptr = strchr(ptr, '\n')))
              *ptr = '\0';
            else
              break;
          }

          rd->rd.res.members = rd->memberptr = calloc(j+1, sizeof(char *));
          for(i = 0, ptr = rd->members; i < j; ptr += strlen(ptr)+1)
            rd->memberptr[i++] = ptr;
        }
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      if (rd->members) free(rd->members);
      if (rd->memberptr) free(rd->memberptr);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrresolve
void rx_addrresolve( UNUSED struct RexxHost *host, struct rxd_addrresolve **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_addrresolve rd;
    char *string;
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      // generate a "fake" RecipientstringObject and use it on the resolve task
      Object *str = RecipientstringObject,
                      MUIA_Recipientstring_MultipleRecipients, TRUE,
                      MUIA_String_Contents,                    rd->rd.arg.alias,
                    End;

      STRPTR res = (STRPTR)DoMethod(str, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoCache);
      if(res && strcmp(rd->rd.arg.alias, res)) /* did the string change ? */
      {
        if((rd->rd.res.recpt = rd->string = AllocStrBuf(strlen(res)+1)))
          strlcpy(rd->string, res, strlen(res)+1);
      }
      else
        rd->rd.rc = RETURN_WARN;

      MUI_DisposeObject(str);
    }
    break;

    case RXIF_FREE:
    {
      FreeStrBuf(rd->string);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_newmailfile
void rx_newmailfile( UNUSED struct RexxHost *host, struct rxd_newmailfile **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_newmailfile rd;
    char result[SIZE_PATHFILE];
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *folder;

      if(rd->rd.arg.folder)
        folder = FO_GetFolderRexx(rd->rd.arg.folder, NULL);
      else
        folder = FO_GetCurrentFolder();

      if(folder && !isGroupFolder(folder))
      {
        char mfile[SIZE_MFILE];
        strlcpy(rd->rd.res.filename = rd->result, MA_NewMailFile(folder, mfile), sizeof(rd->result));
      }
      else
        rd->rd.rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writefrom
void rx_writefrom( UNUSED struct RexxHost *host, struct rxd_writefrom **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writefrom *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        setstring(G->WR[G->ActiveWriteWin]->GUI.ST_FROM, rd->arg.address);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writereplyto
void rx_writereplyto( UNUSED struct RexxHost *host, struct rxd_writereplyto **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writereplyto *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        setstring(G->WR[G->ActiveWriteWin]->GUI.ST_REPLYTO, rd->arg.address);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_listselect
void rx_listselect( UNUSED struct RexxHost *host, struct rxd_listselect **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_listselect *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      Object *nl = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

      switch(rd->arg.mode[0])
      {
        case 'a': case 'A': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL); break;
        case 'n': case 'N': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL); break;
        case 't': case 'T': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Toggle, NULL); break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
        case '8': case '9': DoMethod(nl, MUIM_NList_Select, atol(rd->arg.mode), MUIV_NList_Select_On, NULL); break;
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_readclose
void rx_readclose( UNUSED struct RexxHost *host, struct rxd_readclose **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_readclose *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct ReadMailData *rmData = G->ActiveRexxRMData;

      if(rmData)
        CleanupReadMailData(rmData, TRUE);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_setmailfile
void rx_setmailfile( UNUSED struct RexxHost *host, struct rxd_setmailfile **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_setmailfile *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail *mail = NULL;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);
      char *mfile;
      int i;

      mfile = (char *)FilePart(rd->arg.mailfile);

      for(i=0;;i++)
      {
        DoMethod(lv, MUIM_NList_GetEntry, i, &mail);
        if(!mail)
          break;

        if(!stricmp(mail->MailFile, mfile))
        {
          set(lv, MUIA_NList_Active, i);
          break;
        }
      }

      if(!mail)
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailcopy
void rx_mailcopy( UNUSED struct RexxHost *host, struct rxd_mailcopy **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailcopy *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *folder;

      if((folder = FO_GetFolderRexx(rd->arg.folder, NULL)))
        MA_MoveCopy(NULL, FO_GetCurrentFolder(), folder, TRUE, FALSE);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_appbusy
void rx_appbusy( UNUSED struct RexxHost *host, struct rxd_appbusy **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_appbusy *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      char *s = rd->arg.text;

      // we don`t make a text a requirement. If the user hasn`t supplied
      // a text for the busytext we simply use a empty string.
      if(s)
        BusyText(s, "");
      else
        BusyText(" ", "");

      if(BusyLevel == 1)
        nnset(G->App, MUIA_Application_Sleep, TRUE);

      rd->rc = BusyLevel ? 1 : 0;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_appnobusy
void rx_appnobusy( UNUSED struct RexxHost *host, struct rxd_appnobusy **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_appnobusy *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      BusyEnd();

      if(BusyLevel <= 0)
        nnset(G->App, MUIA_Application_Sleep, FALSE);

      rd->rc = BusyLevel ? 1 : 0;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_writeemailto
void rx_writemailto( UNUSED struct RexxHost *host, struct rxd_writemailto **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_writemailto *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(G->WR[G->ActiveWriteWin])
        InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_TO, rd->arg.address, FALSE);
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_userinfo
void rx_userinfo( UNUSED struct RexxHost *host, struct rxd_userinfo **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_userinfo rd;
    long folders;
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct User *u = US_GetCurrentUser();
      int i = 0, numfolders = 0;
      struct MUI_NListtree_TreeNode *tn;

      rd->rd.res.username = u->Name;
      rd->rd.res.email = C->EmailAddress;
      rd->rd.res.realname = C->RealName;
      rd->rd.res.config = G->CO_PrefsFile;
      rd->rd.res.maildir = G->MA_MailDir;

      // count the real folders now
      while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)))
      {
        if(!isFlagSet(tn->tn_Flags, TNF_LIST))
          numfolders++;

        i++;
      }

      rd->folders = numfolders;
      rd->rd.res.folders = &rd->folders;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailstatus
void rx_mailstatus( UNUSED struct RexxHost *host, struct rxd_mailstatus **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailstatus *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      switch (tolower(rd->arg.status[0]))
      {
        case 'o': MA_SetStatusTo(SFLAG_READ,              SFLAG_NEW, FALSE);                         break;
        case 'u': MA_SetStatusTo(SFLAG_NONE,              SFLAG_NEW|SFLAG_READ, FALSE);              break;
        case 'h': MA_SetStatusTo(SFLAG_HOLD|SFLAG_READ,   SFLAG_QUEUED|SFLAG_ERROR, FALSE);          break;
        case 'w': MA_SetStatusTo(SFLAG_QUEUED|SFLAG_READ, SFLAG_SENT|SFLAG_HOLD|SFLAG_ERROR, FALSE); break;

        default:
          rd->rc = RETURN_WARN;
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_isonline
void rx_isonline( UNUSED struct RexxHost *host, struct rxd_isonline **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_isonline *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      rd->rc = SocketBase ? 1 : 0;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_requeststring
void rx_requeststring( UNUSED struct RexxHost *host, struct rxd_requeststring **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct
  {
    struct rxd_requeststring rd;
    char string[SIZE_DEFAULT];
  } *rd = (void *)*rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      char *reqtext = AllocReqText(rd->rd.arg.body);

      if(rd->rd.arg.string)
        strlcpy(rd->string, rd->rd.arg.string, sizeof(rd->string));

      rd->rd.rc = !StringRequest(rd->string, SIZE_DEFAULT, NULL, reqtext, tr(MSG_Okay), NULL, tr(MSG_Cancel), (BOOL)rd->rd.arg.secret, G->MA->GUI.WI);
      rd->rd.res.string = rd->string;

      free(reqtext);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_requestfolder
void rx_requestfolder( UNUSED struct RexxHost *host, struct rxd_requestfolder **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_requestfolder *rd = *rxd;

  ENTER();

  switch( action )
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Folder *exfolder, *folder;
      char *reqtext = AllocReqText(rd->arg.body);

      exfolder = rd->arg.excludeactive ? FO_GetCurrentFolder() : NULL;

      if((folder = FolderRequest(NULL, reqtext, tr(MSG_Okay), tr(MSG_Cancel), exfolder, G->MA->GUI.WI)))
        rd->res.folder = folder->Name;
      else
        rd->rc = 1;

      free(reqtext);
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_getselected
void rx_getselected( UNUSED struct RexxHost *host, struct rxd_getselected **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_getselected *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail **mlist;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

      if((mlist = MA_CreateMarkedList(lv, FALSE)) != NULL)
      {
        if((rd->res.num = calloc(1 + (int)mlist[0], sizeof(long))))
        {
          int i;

          for(i = 0; i < (int)mlist[0]; i++)
          {
            struct Mail *mail = mlist[i + 2];

            if(mail != NULL)
              rd->res.num[i] = &mail->position;
            else
              rd->res.num[i] = 0;
          }
        }

        free(mlist);
      }
      else
      {
        rd->res.num    = calloc(1, sizeof(long));
        rd->res.num[0] = 0;
      }
    }
    break;

    case RXIF_FREE:
    {
      if(rd->res.num)
        free(rd->res.num);

      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addredit
void rx_addredit( UNUSED struct RexxHost *host, struct rxd_addredit **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addredit *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct MUI_NListtree_TreeNode *tn;

      if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
      {
        struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);

        if(rd->arg.alias)    strlcpy(ab->Alias, rd->arg.alias, sizeof(ab->Alias));
        if(rd->arg.name)     strlcpy(ab->RealName, rd->arg.name, sizeof(ab->RealName));
        if(rd->arg.email)    strlcpy(ab->Address, rd->arg.email, sizeof(ab->Address));
        if(rd->arg.pgp)      strlcpy(ab->PGPId, rd->arg.pgp, sizeof(ab->PGPId));
        if(rd->arg.homepage) strlcpy(ab->Homepage, rd->arg.homepage, sizeof(ab->Homepage));
        if(rd->arg.street)   strlcpy(ab->Street, rd->arg.street, sizeof(ab->Street));
        if(rd->arg.city)     strlcpy(ab->City, rd->arg.city, sizeof(ab->City));
        if(rd->arg.country)  strlcpy(ab->Country, rd->arg.country, sizeof(ab->Country));
        if(rd->arg.phone)    strlcpy(ab->Phone, rd->arg.phone, sizeof(ab->Phone));
        if(rd->arg.comment)  strlcpy(ab->Comment, rd->arg.comment, sizeof(ab->Comment));

        if(rd->arg.birthdate)
        {
          // if the user supplied 0 as the birthdate, he wants to "delete" the current
          // birthdate
          if(*rd->arg.birthdate == 0) ab->BirthDay = 0;
          else if(AB_ExpandBD(*rd->arg.birthdate)[0]) ab->BirthDay = *rd->arg.birthdate;
          else
          {
            rd->rc = RETURN_ERROR;
            break;
          }
        }

        if(rd->arg.image)
          strlcpy(ab->Photo, rd->arg.image, sizeof(ab->Photo));

        if(rd->arg.member && ab->Type == AET_LIST)
        {
           char **p, *memb = AllocStrBuf(SIZE_DEFAULT);
           if (rd->arg.add && ab->Members) memb = StrBufCpy(memb, ab->Members);
           for (p = rd->arg.member; *p; p++) { memb = StrBufCat(memb, *p); memb = StrBufCat(memb, "\n"); }
           if (ab->Members) free(ab->Members);
           ab->Members = strdup(memb);
           FreeStrBuf(memb);
        }

        DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
        G->AB->Modified = TRUE;
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrdelete
void rx_addrdelete( UNUSED struct RexxHost *host, struct rxd_addrdelete **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrdelete *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      // if the command was called without any parameter it will delete the active entry
      // if not we search for the one in question and if found delete it.
      if(!rd->arg.alias)
      {
        if(xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active) != MUIV_NListtree_Active_Off)
          CallHookPkt(&AB_DeleteHook, 0, 0);
        else
          rd->rc = RETURN_WARN;
      }
      else if(AB_GotoEntry(rd->arg.alias))
        CallHookPkt(&AB_DeleteHook, 0, 0);
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrsave
void rx_addrsave( UNUSED struct RexxHost *host, struct rxd_addrsave **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrsave *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(rd->arg.filename)
      {
        if(!AB_SaveTree(rd->arg.filename))
          rd->rc = RETURN_ERROR;
      }
      else
      {
        if(AB_SaveTree(G->AB_Filename))
          G->AB->Modified = FALSE;
        else
          rd->rc = RETURN_ERROR;
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrload
void rx_addrload( UNUSED struct RexxHost *host, struct rxd_addrload **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrload *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!AB_LoadTree(rd->arg.filename, FALSE, FALSE))
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrgoto
void rx_addrgoto( UNUSED struct RexxHost *host, struct rxd_addrgoto **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrgoto *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(!AB_GotoEntry(rd->arg.alias))
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_addrnew
void rx_addrnew( UNUSED struct RexxHost *host, struct rxd_addrnew **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_addrnew *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      static struct ABEntry addr;

      memset(&addr, 0, sizeof(struct ABEntry));
      addr.Type = AET_USER;
      addr.Members = (char *)"";

      if(rd->arg.type)
      {
        if(tolower(*rd->arg.type) == 'g')
          addr.Type = AET_GROUP;
        else if(tolower(*rd->arg.type) == 'l')
          addr.Type = AET_LIST;
      }

      if(rd->arg.alias)    strlcpy(addr.Alias, rd->arg.alias, sizeof(addr.Alias));
      if(rd->arg.name)     strlcpy(addr.RealName, rd->arg.name, sizeof(addr.RealName));
      if(rd->arg.email)    strlcpy(addr.Address, rd->arg.email, sizeof(addr.Address));

      if(!*addr.Alias)
      {
        if(addr.Type == AET_USER)
          EA_SetDefaultAlias(&addr);
        else
          rd->rc = RETURN_ERROR;
      }

      if(!rd->rc)
      {
        EA_FixAlias(&addr, FALSE);
        rd->res.alias = addr.Alias;
        EA_InsertBelowActive(&addr, addr.Type == AET_GROUP ? TNF_LIST : 0);
        G->AB->Modified = TRUE;
        AppendToLogfile(LF_VERBOSE, 71, tr(MSG_LOG_NewAddress), addr.Alias);
      }
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_mailchangesubjcet
void rx_mailchangesubject( UNUSED struct RexxHost *host, struct rxd_mailchangesubject **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_mailchangesubject *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      struct Mail **mlist;
      Object *lv = (Object *)xget(G->MA->GUI.PG_MAILLIST, MUIA_MainMailListGroup_MainList);

      if((mlist = MA_CreateMarkedList(lv, FALSE)) != NULL)
      {
        int i, selected = (int)*mlist;

        for(i = 0; i < selected; i++)
        {
          struct Mail *mail = mlist[i + 2];

          if(mail != NULL)
            MA_ChangeSubject(mail, rd->arg.subject);
        }

        free(mlist);

        DoMethod(lv, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
      }
      else
        rd->rc = RETURN_ERROR;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///
/// rx_geturl
void rx_geturl( UNUSED struct RexxHost *host, struct rxd_geturl **rxd, enum RexxAction action, UNUSED struct RexxMsg *rexxmsg )
{
  struct rxd_geturl *rd = *rxd;

  ENTER();

  switch(action)
  {
    case RXIF_INIT:
    {
      *rxd = AllocVecPooled(G->SharedMemPool, sizeof(*rd));
    }
    break;

    case RXIF_ACTION:
    {
      if(TR_OpenTCPIP())
      {
        BusyText(tr(MSG_TR_Downloading), "");

        if(!TR_DownloadURL(rd->arg.url, NULL, rd->arg.filename))
          rd->rc = RETURN_ERROR;

        TR_CloseTCPIP();
        BusyEnd();
      }
      else
        rd->rc = RETURN_WARN;
    }
    break;

    case RXIF_FREE:
    {
      FreeVecPooled(G->SharedMemPool, rd);
    }
    break;
  }

  LEAVE();
}

///

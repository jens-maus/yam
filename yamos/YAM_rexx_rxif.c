/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2003 by YAM Open Source Team

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <clib/alib_protos.h>
#include <exec/memory.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/rexxsyslib.h>
#include <proto/utility.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_rexx_rxcl.h"
#include "YAM_write.h"
#include "classes/Classes.h"

/** REXX Commands **/

/// rx_show
void rx_show( struct RexxHost *host, struct rxd_show **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_show *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
         // lets signal the application to uniconify and open the windows
         PopUp();
      }
      break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_hide
void rx_hide( struct RexxHost *host, struct rxd_hide **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_hide *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         set(G->App, MUIA_Application_Iconified, TRUE);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_quit
void rx_quit( struct RexxHost *host, struct rxd_quit **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_quit *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (rd->arg.force) set(G->App, MUIA_Application_ForceQuit, TRUE);
         DoMethod(G->App, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_help
void rx_help( struct RexxHost *host, struct rxd_help **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_help *rd = *rxd;
   struct rxs_command *rxc;
   FILE *fp = NULL, *out = stdout;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (rd->arg.file) if ((fp = fopen(rd->arg.file, "w"))) out = fp;
         fprintf(out, "Commands for application \"YAM\"\n\nCommand          Template\n-------          --------\n");
         for (rxc = rxs_commandlist; rxc->command; rxc++)
            fprintf(out, "%-16s%c%s%s%s%s%s\n", rxc->command,
               (rxc->results || rxc->args) ? ' ' : '\0', rxc->results ? "VAR/K,STEM/K" : "",
               (rxc->results && rxc->args) ? "," : "", rxc->args ? rxc->args : "",
               rxc->results ? " => " : "", rxc->results ? rxc->results : "");
         if (fp) fclose(fp);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_info
void rx_info( struct RexxHost *host, struct rxd_info **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_info *rd = *rxd;
   char *key;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         key = rd->arg.item;
         if (!key) rd->rc = RETURN_ERROR;
         else if (!stricmp(key, "title"))       get(G->App, MUIA_Application_Title, &rd->res.value);
         else if (!stricmp(key, "author"))      get(G->App, MUIA_Application_Author, &rd->res.value);
         else if (!stricmp(key, "copyright"))   get(G->App, MUIA_Application_Copyright, &rd->res.value);
         else if (!stricmp(key, "description")) get(G->App, MUIA_Application_Description, &rd->res.value);
         else if (!stricmp(key, "version"))     get(G->App, MUIA_Application_Version, &rd->res.value);
         else if (!stricmp(key, "base"))        get(G->App, MUIA_Application_Base, &rd->res.value);
         else if (!stricmp(key, "screen"))
         {
            struct Screen *screen;
            rd->res.value = "Workbench";
            get(G->MA->GUI.WI, MUIA_Window_Screen, &screen);
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
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeto
void rx_writeto( struct RexxHost *host, struct rxd_writeto **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writeto *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_TO, rd->arg.address, (BOOL)rd->arg.add);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writecc
void rx_writecc( struct RexxHost *host, struct rxd_writecc **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writecc *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_CC, rd->arg.address, (BOOL)rd->arg.add);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writebcc
void rx_writebcc( struct RexxHost *host, struct rxd_writebcc **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writebcc *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_BCC, rd->arg.address, (BOOL)rd->arg.add);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeattch
void rx_writeattach( struct RexxHost *host, struct rxd_writeattach **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writeattach *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (access(rd->arg.file,F_OK) == 0 && G->WR[G->ActiveWriteWin])
         {
            WR_AddFileToList((int)G->ActiveWriteWin, rd->arg.file, NULL, FALSE);
            if (rd->arg.desc)    setstring(G->WR[G->ActiveWriteWin]->GUI.ST_DESC, rd->arg.desc);
            if (rd->arg.encmode) setmutex (G->WR[G->ActiveWriteWin]->GUI.RA_ENCODING, !strnicmp(rd->arg.encmode, "uu", 2) ? 1 : 0);
            if (rd->arg.ctype)   setstring(G->WR[G->ActiveWriteWin]->GUI.ST_CTYPE, rd->arg.ctype);
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeletter
void rx_writeletter( struct RexxHost *host, struct rxd_writeletter **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writeletter *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin] && CopyFile(G->WR_Filename[G->ActiveWriteWin], 0, rd->arg.file, 0))
         {
            if (C->UseSignature && !rd->arg.nosig) WR_AddSignature(G->ActiveWriteWin, -1);
            FileToEditor(G->WR_Filename[G->ActiveWriteWin], G->WR[G->ActiveWriteWin]->GUI.TE_EDIT);
         }
         else rd->rc = RETURN_ERROR;
         break;

      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeoptions
void rx_writeoptions( struct RexxHost *host, struct rxd_writeoptions **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writeoptions *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;

      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin])
         {
            setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_DELSEND, rd->arg.delete);
            setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_RECEIPT, rd->arg.receipt);
            setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_DISPNOTI, rd->arg.notif);
            setcheckmark(G->WR[G->ActiveWriteWin]->GUI.CH_ADDINFO, rd->arg.addinfo);
            if (rd->arg.importance) setcycle(G->WR[G->ActiveWriteWin]->GUI.CY_IMPORTANCE, *rd->arg.importance);
            if (rd->arg.sig)        setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SIGNATURE, *rd->arg.sig);
            if (rd->arg.security)   setmutex(G->WR[G->ActiveWriteWin]->GUI.RA_SECURITY, *rd->arg.security);
         }
         else rd->rc = RETURN_ERROR;
         break;

      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writequeue
void rx_writequeue( struct RexxHost *host, struct rxd_writequeue **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writequeue *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;

      case RXIF_ACTION:
         if(G->WR[G->ActiveWriteWin]) WR_NewMail(rd->arg.hold ? WRITE_HOLD : WRITE_QUEUE, (int)G->ActiveWriteWin);
         else rd->rc = RETURN_ERROR;
         break;

      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writesend
void rx_writesend( struct RexxHost *host, struct rxd_writesend **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writesend *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;

      case RXIF_ACTION:
         if(G->WR[G->ActiveWriteWin]) WR_NewMail(WRITE_SEND, (int)G->ActiveWriteWin);
         else rd->rc = RETURN_ERROR;
         break;

      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailwrite
void rx_mailwrite( struct RexxHost *host, struct rxd_mailwrite **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailwrite *rd = *rxd;
   int winnr;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;

      case RXIF_ACTION:
         winnr = rd->arg.window ? *rd->arg.window : -1;
         rd->res.window = &G->ActiveWriteWin;
         if (winnr < 0)
         {
            if ((winnr = MA_NewMessage(NEW_NEW, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
            {
              G->ActiveWriteWin = winnr;
              if(rd->arg.quiet == FALSE && G->WR[winnr])
              {
                set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
              }
            }
            else rd->rc = RETURN_ERROR;
         }
         else
         {
            if (winnr >= 0 && winnr <= 1)
            {
              if(G->WR[winnr])
              {
                G->ActiveWriteWin = winnr;
                if(rd->arg.quiet == FALSE && G->WR[winnr])
                {
                  set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
                }
              }else rd->rc = RETURN_WARN;
            }
            else rd->rc = RETURN_ERROR;
         }
         break;

      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailreply
void rx_mailreply( struct RexxHost *host, struct rxd_mailreply **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailreply *rd = *rxd;
   int winnr;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->res.window = &G->ActiveWriteWin;
         if ((winnr = MA_NewMessage(NEW_REPLY, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
         {
            G->ActiveWriteWin = winnr;
            if(rd->arg.quiet == FALSE && G->WR[winnr])
            {
              set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailforward
void rx_mailforward( struct RexxHost *host, struct rxd_mailforward **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailforward *rd = *rxd;
   int winnr;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->res.window = &G->ActiveWriteWin;
         if ((winnr = MA_NewMessage(NEW_FORWARD, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
         {
            G->ActiveWriteWin = winnr;
            if(rd->arg.quiet == FALSE && G->WR[winnr])
            {
              set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailmove
void rx_mailmove( struct RexxHost *host, struct rxd_mailmove **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailmove *rd = *rxd;
   struct Folder *folder;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((folder = FO_GetFolderRexx(rd->arg.folder, NULL)))
            MA_MoveCopy(NULL, FO_GetCurrentFolder(), folder, FALSE);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailread
void rx_mailread( struct RexxHost *host, struct rxd_mailread **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailread *rd = *rxd;
   struct Mail *mail;
   int winnr;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         winnr = rd->arg.window ? *rd->arg.window : -1;
         rd->res.window = &G->ActiveReadWin;
         if (winnr < 0)
         {
            if ((mail = MA_GetActiveMail(ANYBOX, NULL, NULL)))
               if ((winnr = RE_Open(-1, TRUE)) >= 0)
               {
                  G->ActiveReadWin = winnr;
                  if (!rd->arg.quiet) set(G->RE[winnr]->GUI.WI, MUIA_Window_Open, TRUE);
                  RE_ReadMessage(winnr, mail);
               }
               else rd->rc = RETURN_ERROR;
            else rd->rc = RETURN_WARN;
         }
         else
         {
            if (winnr >= 0 && winnr <= 3 && G->RE[winnr])
            {
              G->ActiveReadWin = winnr;
              if(!rd->arg.quiet) set(G->RE[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
            }
            else rd->rc = RETURN_ERROR;
         }
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailsend
void rx_mailsend( struct RexxHost *host, struct rxd_mailsend **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailsend *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!MA_Send(rd->arg.all ? SEND_ALL : SEND_ACTIVE)) rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_maildelete
void rx_maildelete( struct RexxHost *host, struct rxd_maildelete **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_maildelete *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         MA_DeleteMessage((BOOL)rd->arg.atonce, (BOOL)rd->arg.force);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailcheck
void rx_mailcheck( struct RexxHost *host, struct rxd_mailcheck **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailcheck *rd = *rxd;
   int pop, popnr = -2;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (rd->arg.pop) { if ((pop = *rd->arg.pop) >= 0 && pop < MAXP3) if (C->P3[pop]) popnr = pop; }
         else popnr = -1;

         if (popnr > -2)
         {
           static long remaining;

           MA_PopNow(rd->arg.manual ? POP_USER : POP_REXX, popnr);

           remaining = G->LastDL.OnServer - G->LastDL.Deleted;

           rd->res.downloaded = &G->LastDL.Downloaded;
           rd->res.onserver = &remaining;
           rd->res.dupskipped = &G->LastDL.DupSkipped;
           rd->res.deleted = &G->LastDL.Deleted;
           if (G->LastDL.Error) rd->rc = RETURN_WARN;
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailimport
void rx_mailimport( struct RexxHost *host, struct rxd_mailimport **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailimport *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!MA_ImportMessages(rd->arg.filename)) rd->rc = RETURN_ERROR;
         else if(!rd->arg.wait)
           CallHookPkt(&TR_ProcessIMPORTHook, 0, 0);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailexport
void rx_mailexport( struct RexxHost *host, struct rxd_mailexport **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailexport *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!MA_ExportMessages((BOOL)rd->arg.all, rd->arg.filename, (BOOL)rd->arg.append)) rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailupdate
void rx_mailupdate( struct RexxHost *host, struct rxd_mailupdate **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailupdate *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         DoMethod(G->App, MUIM_CallHook, &MA_RescanIndexHook);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailfilter
void rx_mailfilter( struct RexxHost *host, struct rxd_mailfilter **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailfilter *rd = *rxd;
   struct RuleResult *rr = &G->RRs;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         DoMethod(G->App, MUIM_CallHook, &MA_ApplyRulesHook, rd->arg.all ? APPLY_RX_ALL : APPLY_RX, 0, FALSE);
         rd->res.checked = &rr->Checked;
         rd->res.bounced = &rr->Bounced;
         rd->res.forwarded = &rr->Forwarded;
         rd->res.replied = &rr->Replied;
         rd->res.executed = &rr->Executed;
         rd->res.moved = &rr->Moved;
         rd->res.deleted = &rr->Deleted;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailinfo
void rx_mailinfo( struct RexxHost *host, struct rxd_mailinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_mailinfo rd;
      long active;
      char from[SIZE_ADDRESS], to[SIZE_ADDRESS], replyto[SIZE_ADDRESS], flags[SIZE_SMALL];
      char filename[SIZE_PATHFILE], date[32], msgid[9];
   } *rd = (void *)*rxd;
   struct Mail *mail;
   struct Folder *folder = 0;
   int pf, vf;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
         if (rd->rd.arg.index)
         {
            rd->active = *rd->rd.arg.index;
            DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, rd->active, &mail);
         }
         else mail = MA_GetActiveMail(ANYBOX, &folder, (int *)&rd->active);
         if (mail)
         {
            struct Person *pe = GetReturnAddress(mail);

            pf = getPERValue(mail);
            vf = getVOLValue(mail);
            GetMailFile(rd->rd.res.filename = rd->filename, folder, mail);
            rd->rd.res.index = &rd->active;
            rd->rd.res.status = Status[mail->Status];
            stccpy(rd->rd.res.from    = rd->from   , BuildAddrName2(&mail->From), SIZE_ADDRESS);
            stccpy(rd->rd.res.to      = rd->to     , BuildAddrName2(&mail->To), SIZE_ADDRESS);
            stccpy(rd->rd.res.replyto = rd->replyto, BuildAddrName2(pe), SIZE_ADDRESS);
            strcpy(rd->rd.res.date    = rd->date   , DateStamp2String(&mail->Date, DSS_USDATETIME));
            rd->rd.res.subject = mail->Subject;
            rd->rd.res.size = &mail->Size;
            sprintf(rd->rd.res.msgid = rd->msgid, "%lx", mail->cMsgID);
            kprintf("[%s]\n", rd->rd.res.msgid);
            sprintf(rd->rd.res.flags = rd->flags, "%c%c%c%c%c-%c%c%c",
                      isMultiRCPTMail(mail) ? 'M' : '-',
                      isMultiPartMail(mail) ? 'A' : '-',
                      isReportMail(mail)    ? 'R' : '-',
                      isCryptedMail(mail)   ? 'C' : '-',
                      isSignedMail(mail)    ? 'S' : '-',
                      pf ? pf+'0' : '-',
                      vf ? vf+'0' : '-',
                      isMarkedMail(mail)    ? 'M' : '-'
                   );
         }
         else rd->rd.rc = RETURN_ERROR;
      }
      break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_setfolder
void rx_setfolder( struct RexxHost *host, struct rxd_setfolder **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_setfolder *rd = *rxd;
   struct Folder *folder;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((folder = FO_GetFolderRexx(rd->arg.folder, NULL))) MA_ChangeFolder(folder, TRUE);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_setmail
void rx_setmail( struct RexxHost *host, struct rxd_setmail **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_setmail *rd = *rxd;
   int mail, max;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         mail = *rd->arg.num;
         get(G->MA->GUI.NL_MAILS, MUIA_NList_Entries, &max);
         if (mail < 0 || mail >= max) rd->rc = RETURN_ERROR;
         else set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, mail);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeeditor
void rx_writeeditor( struct RexxHost *host, struct rxd_writeeditor **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writeeditor *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin])
         {
            ULONG p = DoMethod(G->WR[G->ActiveWriteWin]->GUI.TE_EDIT, MUIM_TextEditor_ARexxCmd, rd->arg.command);
            switch (p)
            {
               case FALSE: rd->rc = RETURN_WARN; break;
               case TRUE:  break;
               default:    rd->res.result = (char *)p; break;
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         if (rd->res.result) FreeVec(rd->res.result);
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_request
void rx_request( struct RexxHost *host, struct rxd_request **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_request rd;
      long result;
   } *rd = (void *)*rxd;
   char *reqtext;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         reqtext = AllocReqText(rd->rd.arg.body);
         rd->result = MUI_Request(G->App, NULL, 0, NULL, rd->rd.arg.gadgets, reqtext);
         rd->rd.res.result = &rd->result;
         free(reqtext);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailarchive
void rx_mailarchive( struct RexxHost *host, struct rxd_mailarchive **rxd, long action, struct RexxMsg *rexxmsg )
{
   rx_mailmove(host, (struct rxd_mailmove **)rxd, action, rexxmsg);
}

///
/// rx_mailsendall
void rx_mailsendall( struct RexxHost *host, struct rxd_mailsendall **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailsendall *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!MA_Send(SEND_ALL)) rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_getfolderinfo
void rx_getfolderinfo( struct RexxHost *host, struct rxd_getfolderinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_getfolderinfo rd;
      char result[SIZE_SMALL];
   } *rd = (void *)*rxd;
   struct Folder *fo;
   int num;
   char *key;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         key = rd->rd.arg.item;
         fo = FO_GetCurrentFolder();

         // this command should only act on a folder folder and
         // also only on a non-group
         if(fo && fo->Type != FT_GROUP)
         {
            num = FO_GetFolderPosition(fo, FALSE);
            if (!strnicmp(key, "NUM", 3))      sprintf(rd->rd.res.value = rd->result, "%d", num);
            else if (!strnicmp(key, "NAM", 3)) rd->rd.res.value = fo->Name;
            else if (!strnicmp(key, "PAT", 3)) rd->rd.res.value = fo->Path;
            else if (!strnicmp(key, "MAX", 3)) sprintf(rd->rd.res.value = rd->result, "%d", fo->Total);
            else rd->rd.rc = RETURN_ERROR;
         }
         else rd->rd.rc = RETURN_ERROR;

         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_getmailinfo
void rx_getmailinfo( struct RexxHost *host, struct rxd_getmailinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_getmailinfo rd;
      char result[SIZE_LARGE];
   } *rd = (void *)*rxd;
   struct Mail *mail;
   int active;
   char *key;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((mail = MA_GetActiveMail(ANYBOX, NULL, &active)))
         {
            struct Person *pe = GetReturnAddress(mail);

            rd->rd.res.value = rd->result;
            key = rd->rd.arg.item;
            if (!strnicmp(key, "ACT", 3)) sprintf(rd->result, "%d", active);
            else if (!strnicmp(key, "STA", 3)) rd->rd.res.value = Status[mail->Status];
            else if (!strnicmp(key, "FRO", 3)) strcpy(rd->result, BuildAddrName2(&mail->From));
            else if (!strnicmp(key, "TO" , 2)) strcpy(rd->result, BuildAddrName2(&mail->To));
            else if (!strnicmp(key, "REP", 3)) strcpy(rd->result, BuildAddrName2(pe));
            else if (!strnicmp(key, "SUB", 3)) rd->rd.res.value = mail->Subject;
            else if (!strnicmp(key, "FIL", 3)) GetMailFile(rd->rd.res.value = rd->result, mail->Folder, mail);
            else rd->rd.rc = RETURN_ERROR;
         }
         else rd->rd.rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_getconfiginfo
void rx_getconfiginfo( struct RexxHost *host, struct rxd_getconfiginfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_getconfiginfo *rd = *rxd;
   char *key;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         key = rd->arg.item;
         if (!strnicmp(key, "NAM", 3)) rd->res.value = C->RealName;
         else if (!strnicmp(key, "EMA", 3)) rd->res.value = C->EmailAddress;
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_folderinfo
void rx_folderinfo( struct RexxHost *host, struct rxd_folderinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_folderinfo *rd = *rxd;
   struct Folder *fo;
   static long num;
   static long type;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         fo = rd->arg.folder ? FO_GetFolderRexx(rd->arg.folder, NULL) : FO_GetCurrentFolder();

         // this command should only act on a folder folder and
         // also only on a non-group
         if (fo && fo->Type != FT_GROUP)
         {
            num = FO_GetFolderPosition(fo, FALSE);
            rd->res.number = &num;
            rd->res.name = fo->Name;
            rd->res.path = fo->Path;
            rd->res.total = (long *)&fo->Total;
            rd->res.new = (long *)&fo->New;
            rd->res.unread = (long *)&fo->Unread;
            rd->res.size = (long *)&fo->Size;

            // per definition of the .guide arexx docu.
            if(fo->Type == FT_CUSTOM)           type = 0;
            else if(fo->Type == FT_INCOMING)    type = 1;
            else if(fo->Type == FT_OUTGOING)    type = 2;
            else if(fo->Type == FT_SENT)        type = 3;
            else if(fo->Type == FT_DELETED)     type = 4;
            else if(fo->Type == FT_CUSTOMMIXED) type = 5;
            else if(fo->Type == FT_CUSTOMSENT)  type = 6;

            rd->res.type = &type;
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writesubject
void rx_writesubject( struct RexxHost *host, struct rxd_writesubject **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writesubject *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) setstring(G->WR[G->ActiveWriteWin]->GUI.ST_SUBJECT, rd->arg.subject);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_screentoback
void rx_screentoback( struct RexxHost *host, struct rxd_screentoback **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_screentoback *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->rc = 0;
         if (G->MA) DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToBack);
         else rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_screentofront
void rx_screentofront( struct RexxHost *host, struct rxd_screentofront **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_screentofront *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->rc = 0;
         if (G->MA) DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToFront);
         else rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_setflag
void rx_setflag( struct RexxHost *host, struct rxd_setflag **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_setflag *rd = *rxd;
   int value;
   struct Mail *mail;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
         if ((mail = MA_GetActiveMail(ANYBOX, NULL, NULL)))
         {
            if(rd->arg.vol)
            {
              if((value = *rd->arg.vol) >= 0 && value < 8)
              {
                setVOLValue(mail, value);
              }
              else rd->rc = RETURN_ERROR;
            }

            if(rd->arg.per)
            {
              if((value = *rd->arg.per) >= 0 && value < 8)
              {
                if(value != getPERValue(mail))
                {
                  setPERValue(mail, value);

                  MA_SetMailComment(mail);
                }
              }
              else rd->rc = RETURN_ERROR;
            }
         }
         else rd->rc = RETURN_WARN;
      }
      break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailedit
void rx_mailedit( struct RexxHost *host, struct rxd_mailedit **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailedit *rd = *rxd;
   int winnr;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->res.window = &G->ActiveWriteWin;
         if ((winnr = MA_NewMessage(NEW_EDIT, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
         {
            G->ActiveWriteWin = winnr;
            if(rd->arg.quiet == FALSE && G->WR[winnr])
            {
              set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_readinfo
void rx_readinfo( struct RexxHost *host, struct rxd_readinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_readinfo *rd = *rxd;
   struct Part *part;
   int i, parts;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->RE[G->ActiveReadWin])
         {
            for (parts = 0, part = G->RE[G->ActiveReadWin]->FirstPart->Next; part; parts++, part = part->Next);
            rd->res.filename = calloc(parts+1, sizeof(char *));
            rd->res.filetype = calloc(parts+1, sizeof(char *));
            rd->res.filesize = calloc(parts+1, sizeof(long));
            rd->res.tempfile = calloc(parts+1, sizeof(char *));
            for (i = 0, part = G->RE[G->ActiveReadWin]->FirstPart->Next; part; i++, part = part->Next)
            {
               rd->res.filename[i] = part->Name;
               rd->res.filetype[i] = part->ContentType;
               rd->res.filesize[i] = (long *)&part->Size;
               rd->res.tempfile[i] = part->Filename;
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         if (rd->res.filename) free(rd->res.filename);
         if (rd->res.filetype) free(rd->res.filetype);
         if (rd->res.filesize) free(rd->res.filesize);
         if (rd->res.tempfile) free(rd->res.tempfile);
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_readsave
void rx_readsave( struct RexxHost *host, struct rxd_readsave **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_readsave *rd = *rxd;
   struct Part *part;
   struct TempFile *tf;
   BOOL success = FALSE;
   char file[SIZE_PATHFILE];
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->RE[G->ActiveReadWin])
         {
            if (rd->arg.part)
            {
               for (part = G->RE[G->ActiveReadWin]->FirstPart->Next; part; part = part->Next)
                  if (part->Nr == *(rd->arg.part))
                     if (RE_DecodePart(part))
                     {
                        strmfp(file, C->DetachDir, part->Name);
                        success = RE_Export((int)G->ActiveReadWin, part->Filename, rd->arg.filename ? rd->arg.filename : "", part->Name, part->Nr, TRUE, (BOOL)rd->arg.overwrite, part->ContentType);
                     }
            }
            else if ((tf = OpenTempFile("w")))
            {
               RE_SaveDisplay((int)G->ActiveReadWin, tf->FP);
               fclose(tf->FP); tf->FP = NULL;
               success = RE_Export((int)G->ActiveReadWin, tf->Filename, rd->arg.filename ? rd->arg.filename : "", "", 0, TRUE, (BOOL)rd->arg.overwrite, ContType[CT_TX_PLAIN]);
               CloseTempFile(tf);
            }
         }
         if (!success) rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_readprint
void rx_readprint( struct RexxHost *host, struct rxd_readprint **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_readprint *rd = *rxd;
   struct Part *part;
   FILE *prt;
   BOOL success = FALSE;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (C->PrinterCheck) if (!CheckPrinter()) { rd->rc = RETURN_ERROR; break; }
         if (G->RE[G->ActiveReadWin])
         {
            if (rd->arg.part)
            {
               for (part = G->RE[G->ActiveReadWin]->FirstPart->Next; part; part = part->Next)
                  if (part->Nr == *(rd->arg.part))
                     if (RE_DecodePart(part)) success = CopyFile("PRT:", 0, part->Filename, 0);
            }
            else if ((prt = fopen("PRT:","w")))
            {
               RE_SaveDisplay((int)G->ActiveReadWin, prt);
               fclose(prt);
               success = TRUE;
            }
         }
         if (!success) rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailbounce
void rx_mailbounce( struct RexxHost *host, struct rxd_mailbounce **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailbounce *rd = *rxd;
   int winnr;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->res.window = &G->ActiveWriteWin;
         if ((winnr = MA_NewMessage(NEW_BOUNCE, rd->arg.quiet?NEWF_QUIET:0)) >= 0)
         {
            G->ActiveWriteWin = winnr;
            if(rd->arg.quiet == FALSE && G->WR[winnr])
            {
              set(G->WR[winnr]->GUI.WI, MUIA_Window_Activate, TRUE);
            }
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrfind
void rx_addrfind( struct RexxHost *host, struct rxd_addrfind **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_addrfind *rd = *rxd;
   int mode;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         G->AB->Hits = 0;
         if (rd->arg.nameonly) mode = rd->arg.emailonly ? ABF_RX_NAMEEMAIL : ABF_RX_NAME;
                          else mode = rd->arg.emailonly ? ABF_RX_EMAIL     : ABF_RX;
         AB_FindEntry(MUIV_NListtree_GetEntry_ListNode_Root, rd->arg.pattern, mode, NULL);
         if (G->AB->Hits)
         {
            rd->res.alias = calloc(G->AB->Hits+1, sizeof(char *));
            AB_FindEntry(MUIV_NListtree_GetEntry_ListNode_Root, rd->arg.pattern, mode, rd->res.alias);
         }
         else rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         if (rd->res.alias) free(rd->res.alias);
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrinfo
void rx_addrinfo( struct RexxHost *host, struct rxd_addrinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_addrinfo rd;
      char *members, **memberptr;
   } *rd = (void *)*rxd;
   static char *types[3] = { "P","L","G"};
   char *ptr;
   struct ABEntry *ab = NULL;
   int i, j;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
        if (AB_SearchEntry(rd->rd.arg.alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &ab) && (ab != NULL))
        {
          rd->rd.res.type = types[ab->Type];
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

          if (ab->Members)
          {
            rd->members = calloc(strlen(ab->Members)+1,1);
            strcpy(rd->members, ab->Members);
            for (j = 0, ptr = rd->members; *ptr; j++, ptr++)
            {
              if ((ptr = strchr(ptr, '\n'))) *ptr = 0; else break;
            }
            rd->rd.res.members = rd->memberptr = calloc(j+1, sizeof(char *));
            for (i = 0, ptr = rd->members; i < j; ptr += strlen(ptr)+1) rd->memberptr[i++] = ptr;
          }
        }
        else rd->rd.rc = RETURN_ERROR;
      }
      break;
      
      case RXIF_FREE:
      {
        if (rd->members) free(rd->members);
        if (rd->memberptr) free(rd->memberptr);
        FreeVec( rd );
      }
      break;
   }
   return;
}

///
/// rx_addrresolve
void rx_addrresolve( struct RexxHost *host, struct rxd_addrresolve **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_addrresolve rd;
      char *string;
   } *rd = (void *)*rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
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
          strcpy(rd->string, res);
        }
        else
        {
          rd->rd.rc = RETURN_WARN;
        }
        MUI_DisposeObject(str);
      }
      break;

      case RXIF_FREE:
         FreeStrBuf(rd->string);
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_newmailfile
void rx_newmailfile( struct RexxHost *host, struct rxd_newmailfile **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_newmailfile rd;
      char result[SIZE_PATHFILE];
   } *rd = (void *)*rxd;
   struct Folder *folder;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (rd->rd.arg.folder) folder = FO_GetFolderRexx(rd->rd.arg.folder, NULL);
         else folder = FO_GetCurrentFolder();
         if (folder && folder->Type != FT_GROUP)
         {
           strcpy(rd->rd.res.filename = rd->result, MA_NewMailFile(folder, NULL, 0));
         }
         else rd->rd.rc = RETURN_ERROR;

         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writefrom
void rx_writefrom( struct RexxHost *host, struct rxd_writefrom **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writefrom *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) setstring(G->WR[G->ActiveWriteWin]->GUI.ST_FROM, rd->arg.address);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writereplyto
void rx_writereplyto( struct RexxHost *host, struct rxd_writereplyto **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writereplyto *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) setstring(G->WR[G->ActiveWriteWin]->GUI.ST_REPLYTO, rd->arg.address);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_listselect
void rx_listselect( struct RexxHost *host, struct rxd_listselect **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_listselect *rd = *rxd;
   APTR nl;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         nl = G->MA->GUI.NL_MAILS;
         switch (rd->arg.mode[0])
         {
            case 'a': case 'A': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_On, NULL); break;
            case 'n': case 'N': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Off, NULL); break;
            case 't': case 'T': DoMethod(nl, MUIM_NList_Select, MUIV_NList_Select_All, MUIV_NList_Select_Toggle, NULL); break;
            case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
            case '8': case '9': DoMethod(nl, MUIM_NList_Select, atol(rd->arg.mode), MUIV_NList_Select_On, NULL); break;
         }
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_readclose
void rx_readclose( struct RexxHost *host, struct rxd_readclose **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_readclose *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->RE[G->ActiveReadWin]) DoMethod(G->App, MUIM_CallHook, &RE_CloseHook, G->ActiveReadWin);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_setmailfile
void rx_setmailfile( struct RexxHost *host, struct rxd_setmailfile **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_setmailfile *rd = *rxd;
   struct Mail *mail = NULL;
   int i;
   char *mfile;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         mfile = FilePart(rd->arg.mailfile);
         for (i = 0;; i++)
         {
            DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, i, &mail);
            if (!mail) break;
            if (!stricmp(mail->MailFile, mfile)) { set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, i); break; }
         }
         if (!mail) rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailcopy
void rx_mailcopy( struct RexxHost *host, struct rxd_mailcopy **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailcopy *rd = *rxd;
   struct Folder *folder;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((folder = FO_GetFolderRexx(rd->arg.folder, NULL)))
            MA_MoveCopy(NULL, FO_GetCurrentFolder(), folder, TRUE);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_appbusy
void rx_appbusy( struct RexxHost *host, struct rxd_appbusy **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_appbusy *rd = *rxd;
   char *s;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         s = rd->arg.text;

         // we don`t make a text a requirement. If the user hasn`t supplied
         // a text for the busytext we simply use a empty string.
         if (s) BusyText(s, "");
         else   BusyText(" ", "");

         if(BusyLevel == 1) nnset(G->App, MUIA_Application_Sleep, TRUE);
         rd->rc = BusyLevel ? 1 : 0;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_appnobusy
void rx_appnobusy( struct RexxHost *host, struct rxd_appnobusy **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_appnobusy *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         BusyEnd;
         if(BusyLevel <= 0) nnset(G->App, MUIA_Application_Sleep, FALSE);
         rd->rc = BusyLevel ? 1 : 0;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_writeemailto
void rx_writemailto( struct RexxHost *host, struct rxd_writemailto **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_writemailto *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (G->WR[G->ActiveWriteWin]) InsertAddresses(G->WR[G->ActiveWriteWin]->GUI.ST_TO, rd->arg.address, FALSE);
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_userinfo
void rx_userinfo( struct RexxHost *host, struct rxd_userinfo **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_userinfo rd;
      int folders;
   } *rd = (void *)*rxd;
   struct User *u;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         u = US_GetCurrentUser();
         rd->rd.res.username = u->Name;
         rd->rd.res.email = C->EmailAddress;
         rd->rd.res.realname = C->RealName;
         rd->rd.res.config = G->CO_PrefsFile;
         rd->rd.res.maildir = G->MA_MailDir;

         // count the real folders now
         {
            int i = 0, numfolders = 0;
            struct MUI_NListtree_TreeNode *tn;

            while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)))
            {
              if(!isFlagSet(tn->tn_Flags, TNF_LIST)) numfolders++;
              i++;
            }

            rd->folders = numfolders;
         }
         rd->rd.res.folders = (long *)&rd->folders;

         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailstatus
void rx_mailstatus( struct RexxHost *host, struct rxd_mailstatus **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailstatus *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         switch (tolower(rd->arg.status[0]))
         {
            case 'o': MA_SetStatusTo(STATUS_OLD); break;
            case 'u': MA_SetStatusTo(STATUS_UNR); break;
            case 'h': MA_SetStatusTo(STATUS_HLD); break;
            case 'w': MA_SetStatusTo(STATUS_WFS); break;
            default: rd->rc = RETURN_WARN;
         }
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_isonline
void rx_isonline( struct RexxHost *host, struct rxd_isonline **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_isonline *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         rd->rc = SocketBase ? 1 : 0;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_requeststring
void rx_requeststring( struct RexxHost *host, struct rxd_requeststring **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct {
      struct rxd_requeststring rd;
      char string[SIZE_DEFAULT];
   } *rd = (void *)*rxd;
   char *reqtext;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         reqtext = AllocReqText(rd->rd.arg.body);
         if (rd->rd.arg.string) stccpy(rd->string, rd->rd.arg.string, SIZE_DEFAULT);
         rd->rd.rc = !StringRequest(rd->string, SIZE_DEFAULT, NULL, reqtext, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), (BOOL)rd->rd.arg.secret, G->MA->GUI.WI);
         rd->rd.res.string = rd->string;
         free(reqtext);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_requestfolder
void rx_requestfolder( struct RexxHost *host, struct rxd_requestfolder **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_requestfolder *rd = *rxd;
   char *reqtext;
   struct Folder *exfolder, *folder;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         reqtext = AllocReqText(rd->arg.body);
         exfolder = rd->arg.excludeactive ? FO_GetCurrentFolder() : NULL;
         if ((folder = FolderRequest(NULL, reqtext, GetStr(MSG_Okay), GetStr(MSG_Cancel), exfolder, G->MA->GUI.WI))) rd->res.folder = folder->Name;
         else rd->rc = 1;
         free(reqtext);
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_getselected
void rx_getselected( struct RexxHost *host, struct rxd_getselected **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_getselected *rd = *rxd;
   struct Mail **mlist;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS)))
         {
            int i;
            rd->res.num = calloc(1+(int)mlist[0], sizeof(long));
            for (i = 0; i < (int)mlist[0]; i++) rd->res.num[i] = (long *)&(mlist[i+2]->Position);
            free(mlist);
         }
         else
         {
            rd->res.num    = calloc(1, sizeof(long));
            rd->res.num[0] = 0;
         }

         break;
      
      case RXIF_FREE:
         if (rd->res.num) free(rd->res.num);
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addredit
void rx_addredit( struct RexxHost *host, struct rxd_addredit **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct MUI_NListtree_TreeNode *tn;
   struct rxd_addredit *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
         if((tn = (struct MUI_NListtree_TreeNode *)xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active)))
         {
            struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
            if (rd->arg.alias)    stccpy(ab->Alias, rd->arg.alias, SIZE_NAME);
            if (rd->arg.name)     stccpy(ab->RealName, rd->arg.name, SIZE_REALNAME);
            if (rd->arg.email)    stccpy(ab->Address, rd->arg.email, SIZE_ADDRESS);
            if (rd->arg.pgp)      stccpy(ab->PGPId, rd->arg.pgp, SIZE_ADDRESS);
            if (rd->arg.homepage) stccpy(ab->Homepage, rd->arg.homepage, SIZE_URL);
            if (rd->arg.street)   stccpy(ab->Street, rd->arg.street, SIZE_DEFAULT);
            if (rd->arg.city)     stccpy(ab->City, rd->arg.city, SIZE_DEFAULT);
            if (rd->arg.country)  stccpy(ab->Country, rd->arg.country, SIZE_DEFAULT);
            if (rd->arg.phone)    stccpy(ab->Phone, rd->arg.phone, SIZE_DEFAULT);
            if (rd->arg.comment)  stccpy(ab->Comment, rd->arg.comment, SIZE_DEFAULT);
            if (rd->arg.birthdate)
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
            if (rd->arg.image)    stccpy(ab->Photo, rd->arg.image, SIZE_PATHFILE);
            if (rd->arg.member && ab->Type == AET_LIST)
            {
               char **p, *memb = AllocStrBuf(SIZE_DEFAULT);
               if (rd->arg.add && ab->Members) memb = StrBufCpy(memb, ab->Members);
               for (p = rd->arg.member; *p; p++) { memb = StrBufCat(memb, *p); memb = StrBufCat(memb, "\n"); }
               if (ab->Members) free(ab->Members);
               ab->Members = malloc(strlen(memb)+1);
               strcpy(ab->Members, memb);
               FreeStrBuf(memb);
            }
            DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
            G->AB->Modified = TRUE;
         }
         else rd->rc = RETURN_ERROR;
      }
      break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrdelete
void rx_addrdelete( struct RexxHost *host, struct rxd_addrdelete **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_addrdelete *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
      {
         // if the command was called without any parameter it will delete the active entry
         // if not we search for the one in question and if found delete it.
         if(!rd->arg.alias)
         {
            if(xget(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_Active) != MUIV_NListtree_Active_Off)
            {
              CallHookPkt(&AB_DeleteHook, 0, 0);
            }
            else rd->rc = RETURN_WARN;
         }
         else if(AB_GotoEntry(rd->arg.alias)) CallHookPkt(&AB_DeleteHook, 0, 0);
         else rd->rc = RETURN_WARN;
      }
      break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrsave
void rx_addrsave( struct RexxHost *host, struct rxd_addrsave **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_addrsave *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (rd->arg.filename)
         {
            if (!AB_SaveTree(rd->arg.filename)) rd->rc = RETURN_ERROR;
         }
         else
         {
            if (AB_SaveTree(G->AB_Filename)) G->AB->Modified = FALSE; else rd->rc = RETURN_ERROR;
         }
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrload
void rx_addrload( struct RexxHost *host, struct rxd_addrload **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_addrload *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!AB_LoadTree(rd->arg.filename, FALSE, FALSE)) rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrgoto
void rx_addrgoto( struct RexxHost *host, struct rxd_addrgoto **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_addrgoto *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (!AB_GotoEntry(rd->arg.alias)) rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_addrnew
void rx_addrnew( struct RexxHost *host, struct rxd_addrnew **rxd, long action, struct RexxMsg *rexxmsg )
{
   static struct ABEntry addr;
   struct rxd_addrnew *rd = *rxd;

   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         memset(&addr, 0, sizeof(struct ABEntry));
         addr.Type = AET_USER;
         addr.Members = "";
         if (rd->arg.type) if (tolower(*rd->arg.type) == 'g') addr.Type = AET_GROUP;
                      else if (tolower(*rd->arg.type) == 'l') addr.Type = AET_LIST;
         if (rd->arg.alias)    stccpy(addr.Alias, rd->arg.alias, SIZE_NAME);
         if (rd->arg.name)     stccpy(addr.RealName, rd->arg.name, SIZE_REALNAME);
         if (rd->arg.email)    stccpy(addr.Address, rd->arg.email, SIZE_ADDRESS);
         if (!*addr.Alias)
         {
            if (addr.Type == AET_USER) EA_SetDefaultAlias(&addr);
            else rd->rc = RETURN_ERROR;
         }

         if (!rd->rc)
         {
            EA_FixAlias(&addr, FALSE);
            rd->res.alias = addr.Alias;
            EA_InsertBelowActive(&addr, addr.Type == AET_GROUP ? TNF_LIST : 0);
            G->AB->Modified = TRUE;
            AppendLogVerbose(71, GetStr(MSG_LOG_NewAddress), addr.Alias, "", "", "");
         }
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_mailchangesubjcet
void rx_mailchangesubject( struct RexxHost *host, struct rxd_mailchangesubject **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_mailchangesubject *rd = *rxd;
   struct Mail **mlist;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if ((mlist = MA_CreateMarkedList(G->MA->GUI.NL_MAILS)))
         {
            int i, selected = (int)*mlist;
            for (i = 0; i < selected; i++) MA_ChangeSubject(mlist[i+2], rd->arg.subject);
            free(mlist);
            DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Redraw, MUIV_NList_Redraw_All);
         }
         else rd->rc = RETURN_ERROR;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///
/// rx_geturl
void rx_geturl( struct RexxHost *host, struct rxd_geturl **rxd, long action, struct RexxMsg *rexxmsg )
{
   struct rxd_geturl *rd = *rxd;
   switch( action )
   {
      case RXIF_INIT:
         *rxd = AllocVec( sizeof *rd, MEMF_CLEAR );
         break;
         
      case RXIF_ACTION:
         if (TR_OpenTCPIP())
         {
            BusyText(GetStr(MSG_TR_Downloading), "");
            if (!TR_DownloadURL(rd->arg.url, NULL, NULL, rd->arg.filename)) rd->rc = RETURN_ERROR;
            TR_CloseTCPIP();
            BusyEnd;
         }
         else rd->rc = RETURN_WARN;
         break;
      
      case RXIF_FREE:
         FreeVec( rd );
         break;
   }
   return;
}

///


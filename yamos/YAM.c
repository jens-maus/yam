/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <clib/alib_protos.h>
#include <exec/execbase.h>
#include <libraries/asl.h>
#include <libraries/genesis.h>
#include <mui/NList_mcc.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/genesis.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/rexxsyslib.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>
#include <proto/amissl.h>

#include "extra.h"
#include "NewReadArgs.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_debug.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_rexx.h"
#include "YAM_write.h"
#include "classes/Classes.h"

/***************************************************************************
 Module: Root
***************************************************************************/

struct Global *G;

static struct NewRDArgs nrda;
static BPTR olddirlock = -1; /* -1 is an unset indicator */

struct Args {
   char  *user;
   char  *password;
   char  *maildir;
   char  *prefsfile;
   LONG   nocheck;
   LONG   hide;
   LONG   debug;
   char  *mailto;
   char  *subject;
   char  *letter;
   char **attach;
};

/**************************************************************************/

// the used number of timerIO requests (refer to YAM.h)
#define TIO_NUM (4)

// Timer Class
static struct TC_Data
{
   struct MsgPort     *port;
   struct timerequest *timerIO[TIO_NUM]; // lets generate timerIO requests

} TCData;

// AutoDST related variables
enum ADSTmethod { ADST_NONE=0, ADST_SETDST, ADST_FACTS, ADST_SGUARD, ADST_IXGMT };
static const char *ADSTfile[] = { "", "ENV:TZONE", "ENV:FACTS/DST", "ENV:SUMMERTIME", "ENV:IXGMTOFFSET" };
static struct ADST_Data
{
  struct NotifyRequest nRequest;
  enum ADSTmethod method;

} ADSTdata;

/// TC_Start
//  Start a delay depending on the time specified
void TC_Start(enum TimerIO tio, ULONG seconds, ULONG micros)
{
  if(micros > 0 || seconds > 0)
  {
    struct IORequest *ioreq = &TCData.timerIO[tio]->tr_node;
    struct timeval *timeval = &TCData.timerIO[tio]->tr_time;

    // before we set a new timer request we make sure only one
    // is active at a time
    if(ioreq->io_Command != 0)
    {
      if(CheckIO(ioreq) == NULL)
        AbortIO(ioreq);

      WaitIO(ioreq);
    }

    // issue a new timerequest
    ioreq->io_Command = TR_ADDREQUEST;
    timeval->tv_secs  = seconds;
    timeval->tv_micro = micros;

    DB(kprintf("Queueing timerIO[%ld] in %ld seconds %ld micros\n", tio, seconds, micros);)
    SendIO(ioreq);
  }
}

///
/// TC_Stop
//  Stop a currently running TimerIO request
void TC_Stop(enum TimerIO tio)
{
  struct IORequest *ioreq = &TCData.timerIO[tio]->tr_node;

  // check if we have a already issued ioreq running
  if(ioreq && ioreq->io_Command != 0)
  {
    if(CheckIO(ioreq) == NULL)
      AbortIO(ioreq);

    WaitIO(ioreq);

    DB(kprintf("Stopped timerIO[%ld]\n", tio);)
  }
}

///
/// TC_Exit
//  Frees timer resources
static void TC_Exit(void)
{
  // first we abort & delete the IORequests
  if(TCData.timerIO[0] != NULL)
  {
    int i;
    struct IORequest *ioreq;

    // first make sure every TimerIO is stoppped
    for(i=0; i < TIO_NUM; i++)
    {
      ioreq  = &TCData.timerIO[i]->tr_node;

      // check if we have a already issued ioreq running
      if(ioreq && ioreq->io_Command != 0)
      {
        if(CheckIO(ioreq) == NULL) AbortIO(ioreq);

        WaitIO(ioreq);
      }
    }

    // then close the device
    if(TCData.timerIO[0]->tr_node.io_Device != NULL)
    {
      // drop the OS4 Interface of the TimerBase
      DROPINTERFACE(ITimer);

      CloseDevice(&TCData.timerIO[0]->tr_node);
    }

    // and then we delete the IO requests
    for(i=0; i < TIO_NUM; i++)
    {
      if((ioreq = &TCData.timerIO[i]->tr_node))
      {
        if(i==0) DeleteIORequest(ioreq);
        else     FreeMem(ioreq, sizeof(struct timerequest));

        TCData.timerIO[i] = NULL;
      }
    }
  }

  // remove the MsgPort now.
  if(TCData.port != NULL)
  {
    DeleteMsgPort(TCData.port);
    TCData.port = NULL;
  }
}

///
/// TC_Init
//  Initializes timer resources
static BOOL TC_Init(void)
{
  // clear our static structure first
  memset(&TCData, 0, sizeof(struct TC_Data));

  // create message port
  if((TCData.port = CreateMsgPort()))
  {
    // create the TimerIOs now
    if((TCData.timerIO[0] = (struct timerequest *)CreateIORequest(TCData.port, sizeof(struct timerequest))))
    {
      // then open the device
      if(!OpenDevice(TIMERNAME, UNIT_VBLANK, &TCData.timerIO[0]->tr_node, 0L))
      {
        int i;

        // needed to get GetSysTime() working
        if((TimerBase = (APTR)TCData.timerIO[0]->tr_node.io_Device) &&
           GETINTERFACE(ITimer, TimerBase))
        {
          // create our other TimerIOs now
          for(i=1; i < TIO_NUM; i++)
          {
            if(!(TCData.timerIO[i] = AllocMem(sizeof(struct timerequest), MEMF_PUBLIC|MEMF_CLEAR))) return FALSE;

            // then copy the data of our timerIO[0] to the other ones
            memcpy(TCData.timerIO[i], TCData.timerIO[0], sizeof(struct timerequest));
          }

          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

///
/// TC_ActiveEditor
//  Returns TRUE if the internal editor is currently being used
static BOOL TC_ActiveEditor(int wrwin)
{
   if (G->WR[wrwin])
   {
      APTR ao = (APTR)xget(G->WR[wrwin]->GUI.WI, MUIA_Window_ActiveObject);

      return (BOOL)(ao==G->WR[wrwin]->GUI.TE_EDIT);
   }
   return FALSE;
}

///
/// TC_Dispatcher
//  Dispatcher for timer class
static void TC_Dispatcher(enum TimerIO tio)
{
   // if the IORequest isn`t ready yet we don`t wait
   // or else we get perhaps into a deadlock
   if(CheckIO(&TCData.timerIO[tio]->tr_node))
   {
      int i;

      // then wait&remove the IORequest
      WaitIO(&TCData.timerIO[tio]->tr_node);

      // now dispatch between the differnent timerIOs
      switch(tio)
      {
        // in case the WriteIndexes TimerIO request was triggered
        // we first check if no Editor is currently active and
        // if so we write the indexes.
        case TIO_WRINDEX:
        {
          DB(kprintf("TIO_WRINDEX triggered at: %ld\n", time(NULL));)

          // only write the indexes if no Editor is actually in use
          if(!TC_ActiveEditor(0) && !TC_ActiveEditor(1))
          {
            MA_UpdateIndexes(FALSE);
          }

          // restart timer now.
          TC_Start(tio, C->WriteIndexes, 0);
        }
        break;

        // in case the checkMail timerIO request was triggered we
        // need to check if no writewindow is currently in use and
        // then check for new mail.
        case TIO_CHECKMAIL:
        {
          DB(kprintf("TIO_CHECKMAIL triggered at: %ld\n", time(NULL));)

          // only if there is currently no write window open we
          // check for new mail.
          for(i=0; i < MAXWR && !G->WR[i]; i++) ;

          // also the configuration window needs to be closed
          // or we skip the pop operation
          if(i == MAXWR && !G->CO)
          {
            MA_PopNow(POP_TIMED,-1);
          }

          // restart timer now.
          TC_Start(tio, C->CheckMailDelay*60, 0);
        }
        break;

        // and in case the AutoSave timerIO was triggered we check
        // wheter there is really need to autosave the content of
        // the currently used editors.
        case TIO_AUTOSAVE:
        {
          DB(kprintf("TIO_AUTOSAVE triggered at: %ld\n", time(NULL));)

          for(i=0; i < MAXWR; i++)
          {
            if(G->WR[i] && G->WR[i]->Mode != NEW_BOUNCE)
            {
              EditorToFile(G->WR[i]->GUI.TE_EDIT, WR_AutoSaveFile(i), NULL);
              G->WR[i]->AS_Done = TRUE;
            }
          }

          // restart timer now
          TC_Start(tio, C->AutoSave, 0);
        }
        break;

        case TIO_PREVIEWUPDATE:
        {
          DB(kprintf("TIO_PREVIEWUPDATE triggered at: %ld\n", time(NULL));)

          if(C->MailPreview)
          {
            struct MA_GUIData *gui = &G->MA->GUI;
            struct Mail *mail;

            // get the actually active mail
            DoMethod(gui->NL_MAILS, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);

            // update the readMailGroup of the main window.
            if(mail)
              DoMethod(gui->MN_MAILPREVIEW, MUIM_ReadMailGroup_ReadMail, mail);
          }
        }
        break;
      }
   }
}

///
/// ADSTnotify_start
//  AutoDST Notify start function
static BOOL ADSTnotify_start(void)
{
  if(ADSTdata.method != ADST_NONE)
  {
    // prepare the NotifyRequest structure
    BYTE signalAlloc;

    if((signalAlloc = AllocSignal(-1)) >= 0)
    {
      struct NotifyRequest *nr = &ADSTdata.nRequest;

      nr->nr_Name  = (UBYTE *)ADSTfile[ADSTdata.method];
      nr->nr_Flags = NRF_SEND_SIGNAL;

      // prepare the nr_Signal now
      nr->nr_stuff.nr_Signal.nr_Task      = FindTask(NULL);
      nr->nr_stuff.nr_Signal.nr_SignalNum = signalAlloc;

      return StartNotify(nr);
    }
    else
    {
      memset(&ADSTdata, 0, sizeof(struct ADST_Data));
    }
  }

  return FALSE;
}

///
/// ADSTnotify_stop
//  AutoDST Notify stop function
static void ADSTnotify_stop(void)
{
  if(ADSTdata.method != ADST_NONE)
  {
    // stop the NotifyRequest
    struct NotifyRequest *nr = &ADSTdata.nRequest;

    if(nr->nr_Name)
    {
      EndNotify(nr);
      FreeSignal((LONG)nr->nr_stuff.nr_Signal.nr_SignalNum);
    }
  }
}

///
/// SplashProgress
//  Shows progress of program initialization in the splash window
static void SplashProgress(char *txt, int percent)
{
  DoMethod(G->SplashWinObject, MUIM_Splashwindow_StatusChange, txt, percent);
}
///
/// PopUp
//  Un-iconify YAM
void PopUp(void)
{
   int i;
   Object *window = G->MA->GUI.WI;

   nnset(G->App, MUIA_Application_Iconified, FALSE);

   // avoid MUIA_Window_Open's side effect of activating the window if it was already open
   if(!xget(G->MA->GUI.WI, MUIA_Window_Open))
     set(G->MA->GUI.WI, MUIA_Window_Open, TRUE);

   DoMethod(G->MA->GUI.WI, MUIM_Window_ScreenToFront);
   DoMethod(G->MA->GUI.WI, MUIM_Window_ToFront);

   // Now we check if there is any read and write window open and bring it also
   // to the front
   if(IsMinListEmpty(&G->ReadMailDataList) == FALSE)
   {
      // search through our ReadDataList
      struct MinNode *curNode = G->ReadMailDataList.mlh_Head;
      for(curNode = G->ReadMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        struct ReadMailData *rmData = (struct ReadMailData *)curNode;
        if(rmData->readWindow)
        {
          DoMethod(rmData->readWindow, MUIM_Window_ToFront);
          window = rmData->readWindow;
        }
      }
   }

   // Bring the write window to the front
   for(i = 0; i < MAXWR; i++)
   {
     if(G->WR[i])
     {
       DoMethod(G->WR[i]->GUI.WI, MUIM_Window_ToFront);
       window = G->WR[i]->GUI.WI;
     }
   }

   // now we activate the window that is on the top
   set(window, MUIA_Window_Activate, TRUE);
}
///
/// DoublestartHook
//  A second copy of YAM was started
HOOKPROTONHNONP(DoublestartFunc, void)
{
   if(G->App && G->MA && G->MA->GUI.WI) PopUp();
}
MakeStaticHook(DoublestartHook, DoublestartFunc);
///
/// StayInProg
//  Makes sure that the user really wants to quit the program
static BOOL StayInProg(void)
{
   int i;
   BOOL req = FALSE;

   if (G->AB->Modified)
   {
     // make sure the application is uniconified before we popup the requester
     nnset(G->App, MUIA_Application_Iconified, FALSE);

     if (MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_MA_ABookModifiedGad), GetStr(MSG_AB_Modified)))
       CallHookPkt(&AB_SaveABookHook, 0, 0);
   }

   for(i=0; i < MAXEA && !req; i++) if(G->EA[i]) req = TRUE;
   for(i=0; i < MAXWR && !req; i++) if(G->WR[i]) req = TRUE;

   if(req || G->CO || C->ConfirmOnQuit)
   {
     // make sure the application is uniconified before we popup the requester
     nnset(G->App, MUIA_Application_Iconified, FALSE);

     if (!MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_ConfirmReq), GetStr(MSG_YesNoReq), GetStr(MSG_QuitYAMReq)))
       return TRUE;
   }

   return FALSE;
}
///
/// Root_GlobalDispatcher
//  Processes return value of MUI_Application_NewInput
static int Root_GlobalDispatcher(ULONG app_input)
{
   int ret = 0;

   switch (app_input)
   {
      case MUIV_Application_ReturnID_Quit:
      {
        if(!xget(G->App, MUIA_Application_ForceQuit))
        {
          ret = (int)!StayInProg();
        }
        else ret = 1;
      }
      break;

      case ID_CLOSEALL:
      {
        if(!C->IconifyOnQuit) ret = (int)!StayInProg();
        set(G->App, MUIA_Application_Iconified, TRUE);
      }
      break;

      case ID_RESTART:
      {
        ret = 2;
      }
      break;

      case ID_ICONIFY:
      {
        MA_UpdateIndexes(FALSE);
      }
      break;
   }

   return ret;
}
///
/// Root_New
//  Creates MUI application
static BOOL Root_New(BOOL hidden)
{
   if((G->App = YAMObject, End))
   {
      set(G->App, MUIA_Application_HelpFile, "YAM.guide");
      if(hidden)
        set(G->App, MUIA_Application_Iconified, TRUE);

      DoMethod(G->App, MUIM_Notify, MUIA_Application_DoubleStart, TRUE, MUIV_Notify_Application, 2, MUIM_CallHook, &DoublestartHook);
      DoMethod(G->App, MUIM_Notify, MUIA_Application_Iconified, TRUE, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, ID_ICONIFY);

      // create the splash window object and return true if
      // everything worked out fine.
      if((G->SplashWinObject = SplashwindowObject, End))
      {
        G->InStartupPhase = TRUE;

        set(G->SplashWinObject, MUIA_Window_Open, !hidden);

        return TRUE;
      }
   }

   return FALSE;
}
///

/// Terminate
//  Deallocates used memory and MUI modules and terminates
static void Terminate(void)
{
   int i;

   DB(kprintf("Terminate()\n");)

   while (PNum > 0)
      if (PPtr[PNum-1]) free(PPtr[--PNum]);

   if (G->CO)
   {
      CO_FreeConfig(CE);
      free(CE);
      DisposeModule(&G->CO);
   }

   for (i = 0; i < MAXEA; i++)
      DisposeModule(&G->EA[i]);

   // cleanup the still existing readmailData objects
   if(IsMinListEmpty(&G->ReadMailDataList) == FALSE)
   {
      // search through our ReadDataList
      struct MinNode *curNode = G->ReadMailDataList.mlh_Head;
      for(curNode = G->ReadMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        struct ReadMailData *rmData = (struct ReadMailData *)curNode;

        CleanupReadMailData(rmData);
      }
   }

   for (i = 0; i <= MAXWR; i++)
   {
      if (G->WR[i])
      {
         WR_Cleanup(i);
         DisposeModule(&G->WR[i]);
      }
   }

   if (G->TR)
   {
      TR_Cleanup();
      TR_CloseTCPIP();
      DisposeModule(&G->TR);
   }

   if (G->FO) DisposeModule(&G->FO);
   if (G->FI) DisposeModule(&G->FI);
   if (G->ER) DisposeModule(&G->ER);
   if (G->US) DisposeModule(&G->US);

   if (G->MA)
   {
      MA_UpdateIndexes(FALSE);
      SaveLayout(TRUE);
      set(G->MA->GUI.WI, MUIA_Window_Open, FALSE);
   }

   if (G->AB) DisposeModule(&G->AB);

   if(G->MA)
   {
     // make sure the mail preview objects are also properly disposed
     MA_CleanupMailPreview();
     DisposeModule(&G->MA);
   }

   if (G->TTin) free(G->TTin);
   if (G->TTout) free(G->TTout);

   for (i = 0; i < MAXASL; i++)
      if (G->ASLReq[i])
         MUI_FreeAslRequest(G->ASLReq[i]);

   for (i = 0; i <= MAXWR; i++)
      if (G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port)
         DeleteMsgPort(G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port);

   if (G->AppIcon) RemoveAppIcon(G->AppIcon);
   if (G->AppPort) DeleteMsgPort(G->AppPort);
   if (G->RexxHost) CloseDownARexxHost(G->RexxHost);

   TC_Exit();

   // stop the AutoDST notify
   ADSTnotify_stop();

   // check if we have an allocated NewMailSound_Obj and dispose it.
   if(G->NewMailSound_Obj)
     DisposeDTObject(G->NewMailSound_Obj);

   if(G->HideIcon)
     FreeDiskObject(G->HideIcon);

   if(G->App)
     MUI_DisposeObject(G->App);

   for(i = 0; i < MAXICONS; i++)
   {
      if(G->DiskObj[i])
        FreeDiskObject(G->DiskObj[i]);
   }

   for(i = 0; i < MAXIMAGES; i++)
   {
      if(G->BImage[i])
        FreeBCImage(G->BImage[i]);
   }

   CO_FreeConfig(C);
   YAM_CleanupClasses();

   // on OS4 we close the Interfaces now
   CLOSELIB(DataTypesBase, IDataTypes);
   CLOSELIB(XpkBase,       IXpk);
   CLOSELIB(AmiSSLBase,    IAmiSSL);
   CLOSELIB(MUIMasterBase, IMUIMaster);
   CLOSELIB(RexxSysBase,   IRexxSys);
   CLOSELIB(IFFParseBase,  IIFFParse);
   CLOSELIB(KeymapBase,    IKeymap);
   CLOSELIB(WorkbenchBase, IWorkbench);

   // close the catalog and locale now
   CloseYAMCatalog();
   if(G->Locale)
     CloseLocale(G->Locale);

   CLOSELIB(LocaleBase, ILocale);

   free(C);
   free(G);

   DB(kprintf("End of Terminate()\n");)
}
///
/// Abort
//  Shows error requester, then terminates the program
static void Abort(APTR formatnum, ...)
{
   static char error[SIZE_LINE];
   va_list a;

   va_start(a, formatnum);

   if(formatnum)
   {
      vsprintf(error, GetStr(formatnum), a);

      if(MUIMasterBase && G && G->App)
      {
         MUI_Request(G->App, NULL, MUIF_NONE, GetStr(MSG_ErrorStartup), GetStr(MSG_Quit), error);
      }
      else if(IntuitionBase)
      {
         struct EasyStruct ErrReq;

         ErrReq.es_StructSize   = sizeof(struct EasyStruct);
         ErrReq.es_Flags        = 0;
         ErrReq.es_Title        = (char *)GetStr(MSG_ErrorStartup);
         ErrReq.es_TextFormat   = error;
         ErrReq.es_GadgetFormat = (char *)GetStr(MSG_Quit);

         EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
      }
      else
        puts(error);
   }
   va_end(a);

   // do a hard exit.
   exit(5);
}
///
/// CheckMCC
//  Checks if a certain version of a MCC is available
static BOOL CheckMCC(char *name, ULONG minver, ULONG minrev, BOOL req)
{
   BOOL flush = TRUE;

   DB(kprintf("CheckMCC: %s ", name);)

   for(;;)
   {
     // First we attempt to acquire the version and revision through MUI
     Object *obj = MUI_NewObject(name, TAG_DONE);
     if (obj)
     {
       ULONG ver = xget(obj, MUIA_Version);
       ULONG rev = xget(obj, MUIA_Revision);

       struct Library *base;
       char libname[256];

       MUI_DisposeObject(obj);

       if(ver > minver || (ver == minver && rev >= minrev))
       {
         DB(kprintf("v%ld.%ld found through MUIA_Version/Revision\n", ver, rev);)
         return TRUE;
       }

       // If we did't get the version we wanted, let's try to open the
       // libraries ourselves and see what happens...
       strcpy(libname, "PROGDIR:mui/");
       strcat(libname, name);

       if ((base = OpenLibrary(&libname[8], 0)) || (base = OpenLibrary(&libname[0], 0)))
       {
         UWORD OpenCnt = base->lib_OpenCnt;

         ver = base->lib_Version;
         rev = base->lib_Revision;

         CloseLibrary(base);

         // we add some additional check here so that eventual broken .mcc also have
         // a chance to pass this test (i.e. Toolbar.mcc is broken)
         if (ver > minver || (ver == minver && rev >= minrev))
         {
           DB(kprintf("v%ld.%ld found through OpenLibrary()\n", ver, rev);)
           return TRUE;
         }

         if (OpenCnt > 1)
         {
           if (req && MUI_Request(NULL, NULL, 0L, GetStr(MSG_ErrorStartup), GetStr(MSG_RETRY_QUIT_GAD), GetStr(MSG_MCC_IN_USE), name, minver, minrev, ver, rev))
             flush = TRUE;
           else
             break;
         }

         // Attempt to flush the library if open count is 0 or because the
         // user wants to retry (meaning there's a chance that it's 0 now)

         if (flush)
         {
           struct Library *result;
           Forbid();
           if ((result = (struct Library *)FindName(&((struct ExecBase *)SysBase)->LibList, name)))
             RemLibrary(result);
           Permit();
           flush = FALSE;
         }
         else
         {
           DB(kprintf("couldn`t find minimum required version.\n");)

           // We're out of luck - open count is 0, we've tried to flush
           // and still haven't got the version we want
           if (req && MUI_Request(NULL, NULL, 0L, GetStr(MSG_ErrorStartup), GetStr(MSG_RETRY_QUIT_GAD), GetStr(MSG_MCC_OLD), name, minver, minrev, ver, rev))
             flush = TRUE;
           else
             break;
         }
       }
     }
     else
     {
       // No MCC at all - no need to attempt flush
       flush = FALSE;
       if (!MUI_Request(NULL, NULL, 0L, GetStr(MSG_ErrorStartup), GetStr(MSG_RETRY_QUIT_GAD), GetStr(MSG_NO_MCC), name, minver, minrev))
         break;
     }

   }

   if (req)
     exit(5); // Ugly

   return FALSE;
}
///
/// InitLib
//  Opens a library & on OS4 also the main interface
static APTR InitLib(STRPTR libname, struct Library **libbase, ULONG version, int revision, BOOL required, BOOL close)
{
   struct Library *base;

   if(!libbase)
     return NULL;

   // open the library
   base = OpenLibrary(libname, version);

   if(base && revision)
   {
     if(base->lib_Version == version && base->lib_Revision < revision)
     {
       DB(kprintf("InitLib: can`t open library %s with minimum version v%ld.%d\n", libname, version, revision);)

       CloseLibrary(base);
       base = NULL;
     }
   }

   if(base && close)
   {
     DB(kprintf("InitLib: library %s v%ld.%ld successfully opened - autoclosed.\n", libname, base->lib_Version, base->lib_Revision);)

     CloseLibrary(base);
     base = NULL;
   }

   // store base
   *libbase = base;

   // if we end up here, we can open the OS4 base library interface
   #if defined(__amigaos4__)
   if(base)
   {
     // get the "main" interface
     APTR iFace;

     // if we weren`t able to obtain the main interface, lets close the library also
     if(GETINTERFACE(iFace, base) == NULL)
     {
       DB(kprintf("InitLib: can`t get main interface of library %s\n", libname);)

       CloseLibrary(base);
       *libbase = NULL;
     }
     else DB(kprintf("InitLib: library %s v%ld.%ld successfully opened.\n", libname, base->lib_Version, base->lib_Revision);)

     base = iFace;
   }
   else if(base)
   {
     DB(kprintf("InitLib: library %s v%ld.%ld successfully opened.\n", libname, base->lib_Version, base->lib_Revision);)
   }
   #endif

   if(!base && required)
     Abort(MSG_ERR_OPENLIB, libname, version, revision);

   return base;
}
///
/// SetupAppIcons
//  Sets location of mailbox status icon on workbench screen
void SetupAppIcons(void)
{
   int i;
   for (i = 0; i < MAXICONS; i++)
   {
      if (G->DiskObj[i])
      {
         G->DiskObj[i]->do_CurrentX = C->IconPositionX;
         G->DiskObj[i]->do_CurrentY = C->IconPositionY;
      }
   }
}
///
/// Initialise2
//  Phase 2 of program initialization (after user logs in)
static void Initialise2(void)
{
   BOOL newfolders = FALSE;
   int i;
   struct Folder *folder, **oldfolders = NULL;

   SplashProgress(GetStr(MSG_LoadingConfig), 30);
   CO_SetDefaults(C, -1);
   CO_LoadConfig(C, G->CO_PrefsFile, &oldfolders);
   CO_Validate(C, FALSE);
   SplashProgress(GetStr(MSG_CreatingGUI), 40);

   // Create a new Main & Addressbook Window
   if(!(G->MA = MA_New()) || !(G->AB = AB_New()))
   {
      Abort(MSG_ErrorMuiApp);
   }

   MA_SetupMailPreview();

   // Now we have to check on which position we should display the InfoBar and if it`s not
   // center or off we have to resort the main group
   if(C->InfoBar != IB_POS_CENTER && C->InfoBar != IB_POS_OFF)
   {
      MA_SortWindow();
   }

   MA_SetupDynamicMenus();
   CallHookPkt(&MA_ChangeSelectedHook, 0, 0);
   SetupAppIcons();
   LoadLayout();

   if(G->CO_AutoTranslateIn)
     LoadParsers();

   SplashProgress(GetStr(MSG_LoadingFolders), 50);
   if(!FO_LoadTree(CreateFilename(".folders")) && oldfolders)
   {
      for(i = 0; i < 100; i++)
      {
        if (oldfolders[i])
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, oldfolders[i]->Name, oldfolders[i], MUIV_NListtree_Insert_ListNode_Root);
      }

      newfolders = TRUE;
   }

   if (oldfolders) { for (i = 0; oldfolders[i]; i++) free(oldfolders[i]); free(oldfolders); }

   if(!FO_GetFolderByType(FT_INCOMING,NULL))
     newfolders |= FO_CreateFolder(FT_INCOMING, FolderNames[0], GetStr(MSG_MA_Incoming));

   if(!FO_GetFolderByType(FT_OUTGOING,NULL))
     newfolders |= FO_CreateFolder(FT_OUTGOING, FolderNames[1], GetStr(MSG_MA_Outgoing));

   if(!FO_GetFolderByType(FT_SENT    ,NULL))
     newfolders |= FO_CreateFolder(FT_SENT    , FolderNames[2], GetStr(MSG_MA_Sent));

   if(!FO_GetFolderByType(FT_DELETED ,NULL))
     newfolders |= FO_CreateFolder(FT_DELETED , FolderNames[3], GetStr(MSG_MA_Deleted));

   if(newfolders)
   {
      set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible);
      FO_SaveTree(CreateFilename(".folders"));
   }

   SplashProgress(GetStr(MSG_RebuildIndices), 60);
   MA_UpdateIndexes(TRUE);

   SplashProgress(GetStr(MSG_LoadingFolders), 75);
   for(i = 0; ;i++)
   {
      struct MUI_NListtree_TreeNode *tn;
      struct MUI_NListtree_TreeNode *tn_parent;

      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
      if(!tn || !tn->tn_User)
        break;

      folder = tn->tn_User;

      // if this entry is a group lets skip here immediatly
      if(folder->Type == FT_GROUP)
        continue;

      if((folder->Type == FT_INCOMING || folder->Type == FT_OUTGOING ||
          folder->Type == FT_DELETED || C->LoadAllFolders) &&
          !isProtectedFolder(folder))
      {
        MA_GetIndex(folder);
      }
      else if(folder->LoadedMode != LM_VALID)
      {
        folder->LoadedMode = MA_LoadIndex(folder, FALSE);
      }

      // if this folder hasn`t got any own folder image in the folder
      // directory and it is one of our standard folders we have to check which image we put in front of it
      if(!folder->BC_FImage)
      {
        if(folder->Type == FT_INCOMING)      folder->ImageIndex = (folder->New+folder->Unread) ? 3 : 2;
        else if(folder->Type == FT_OUTGOING) folder->ImageIndex = (folder->Total > 0) ? 5 : 4;
        else if(folder->Type == FT_DELETED)  folder->ImageIndex = (folder->Total > 0) ? 7 : 6;
        else if(folder->Type == FT_SENT)     folder->ImageIndex = 8;
        else folder->ImageIndex = -1;
      }

      // now we have to add the amount of mails of this folder to the foldergroup
      // aswell and also the grandparents.
      while((tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE)))
      {
         // fo_parent is NULL then it`s ROOT and we have to skip here
         // because we cannot have a status of the ROOT tree.
         struct Folder *fo_parent = (struct Folder *)tn_parent->tn_User;
         if(fo_parent)
         {
            fo_parent->Unread    += folder->Unread;
            fo_parent->New       += folder->New;
            fo_parent->Total     += folder->Total;
            fo_parent->Sent      += folder->Sent;
            fo_parent->Deleted   += folder->Deleted;

            // for the next step we set tn to the current parent so that we get the
            // grandparents ;)
            tn = tn_parent;
         }
         else break;
      }

      DoMethod(G->App, MUIM_Application_InputBuffered);
   }

   // Now we have to make sure that the current folder is really in "active" state
   // or we risk to get a unsynced message listview.
   MA_ChangeFolder(NULL, TRUE);

   SplashProgress(GetStr(MSG_LoadingABook), 90);
   AB_LoadTree(G->AB_Filename, FALSE, FALSE);
   if(!(G->RexxHost = SetupARexxHost("YAM", NULL)))
      Abort(MSG_ErrorARexx);

   SplashProgress(GetStr(MSG_OPENGUI), 100);
   G->InStartupPhase = FALSE;

   // only activate the main window if the about window is activ
   // and open it immediatly
   // we always start YAM with Window_Open TRUE or else YAM the hide
   // functionality doesn`t work as expected.
   SetAttrs(G->MA->GUI.WI,
            MUIA_Window_Activate, xget(G->SplashWinObject, MUIA_Window_Activate),
            MUIA_Window_Open,     TRUE,
            TAG_DONE);

   set(G->SplashWinObject, MUIA_Window_Open, FALSE);
}
///
/// Initialise
//  Phase 1 of program initialization (before user logs in)
static void Initialise(BOOL hidden)
{
   static const char *imnames[MAXIMAGES] = {
     "status_unread",   "status_old",    "status_forward",  "status_reply",
     "status_waitsend", "status_error",  "status_hold",     "status_sent",
     "status_new",      "status_delete", "status_download", "status_group",
     "status_urgent",   "status_attach", "status_report",   "status_crypt",
     "status_signed",   "status_mark",
     "folder_fold",     "folder_unfold",       "folder_incoming", "folder_incoming_new",
     "folder_outgoing", "folder_outgoing_new", "folder_deleted",  "folder_deleted_new",
     "folder_sent"
   };
   static const char *icnames[MAXICONS] = {
     "empty", "old", "new", "check"
   };
   char iconpath[SIZE_PATH], iconfile[SIZE_PATHFILE];
   int i;

   DateStamp(&G->StartDate);

   // First open locale.library, so we can display a translated error requester
   // in case some of the other libraries can't be opened.
   if(INITLIB(ILocale, InitLib("locale.library", (APTR)&LocaleBase, 38, 0, TRUE, FALSE)))
   {
     G->Locale = OpenLocale(NULL);
   }

   // Now load the catalog of YAM
   OpenYAMCatalog();

   INITLIB(IWorkbench, InitLib("workbench.library", (APTR)&WorkbenchBase, 36, 0, TRUE, FALSE));
   INITLIB(IKeymap,    InitLib("keymap.library",    (APTR)&KeymapBase,    36, 0, TRUE, FALSE));
   INITLIB(IIFFParse,  InitLib("iffparse.library",  (APTR)&IFFParseBase,  36, 0, TRUE, FALSE));
   INITLIB(IRexxSys,   InitLib(RXSNAME,             (APTR)&RexxSysBase,   36, 0, TRUE, FALSE));
   INITLIB(IMUIMaster, InitLib("muimaster.library", (APTR)&MUIMasterBase, 19, 0, TRUE, FALSE));

   // Check if the amissl.library is installed with the correct version
   // so that we can use it later
   if(INITLIB(IAmiSSL, InitLib("amissl.library", (APTR)&AmiSSLBase, AmiSSL_CurrentVersion, AmiSSL_CurrentRevision, FALSE, FALSE)))
   {
      DB(kprintf("AmiSSL library found and enabled!\n");)
      G->TR_UseableTLS = TRUE;
   }

   DB( SetupDebug(); )

   // Lets check for the correct Toolbar.mcc version (minimum 15.8 because earlier versions are too buggy)
   CheckMCC(MUIC_Toolbar, 15, 8, TRUE);

   // Lets check for the correct TextEditor.mcc version
   CheckMCC(MUIC_TextEditor, 15, 9, TRUE);

   // Lets check for the correct BetterString.mcc version
   CheckMCC(MUIC_BetterString, 11, 6, TRUE);

   // we have to have at least v20.111 of NList.mcc to get YAM working without risking
   // to have it buggy - so we make it a requirement. And also 111 is the fastest one ATM.
   CheckMCC(MUIC_NList, 20, 111, TRUE);

   // we make v18.12 the minimum requirement for YAM because earlier versions are
   // buggy
   CheckMCC(MUIC_NListtree, 18, 12, TRUE);

   // Initialise and Setup our own MUI custom classes before we go on
   if(!YAM_SetupClasses())
      Abort(MSG_ErrorClasses);

   // allocate the MUI root object and popup the progress/about window
   if(!Root_New(hidden))
      Abort(FindPort("YAM") ? NULL : MSG_ErrorMuiApp);

   // lets advance the progress bar to 10%
   SplashProgress(GetStr(MSG_InitLibs), 10);

   INITLIB(IXpk, InitLib(XPKNAME, (APTR)&XpkBase, 0, 0, FALSE, FALSE));

   // open the datatypes.library v39
   if(INITLIB(IDataTypes, InitLib("datatypes.library", (APTR)&DataTypesBase, 39, 0, FALSE, FALSE)))
   {
      if(CheckMCC("Dtpic.mui", 19, 0, FALSE))
        G->DtpicSupported = TRUE;
   }

   if (!TC_Init()) Abort(MSG_ErrorTimer);
   for (i = 0; i < MAXASL; i++)
      if (!(G->ASLReq[i] = MUI_AllocAslRequestTags(ASL_FileRequest, ASLFR_RejectIcons, TRUE,
         TAG_END))) Abort(MSG_ErrorAslStruct);
   G->AppPort = CreateMsgPort();
   for (i = 0; i <= MAXWR; i++)
   {
      G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port = CreateMsgPort();
      G->WR_NRequest[i].nr_Name = (UBYTE *)G->WR_Filename[i];
      G->WR_NRequest[i].nr_Flags = NRF_SEND_MESSAGE;
   }
   srand((unsigned int)GetDateStamp());
   SplashProgress(GetStr(MSG_LoadingGFX), 20);
   strmfp(iconfile, G->ProgDir, "YAM");
   if ((G->HideIcon=GetDiskObject(iconfile)))
      set(G->App, MUIA_Application_DiskObject, G->HideIcon);
   strmfp(iconpath, G->ProgDir, "Icons");
   for (i = 0; i < MAXICONS; i++)
   {
      strmfp(iconfile, iconpath, icnames[i]);
      G->DiskObj[i] = GetDiskObject(iconfile);
   }

   // load the standard images now
   for (i = 0; i < MAXIMAGES; i++)
   {
      strmfp(iconfile, iconpath, imnames[i]);
      G->BImage[i] = LoadBCImage(iconfile);
      DoMethod(G->App, MUIM_Application_InputBuffered);
   }
}
///
/// SendWaitingMail
//  Sends pending mail on startup
static void SendWaitingMail(void)
{
   struct Folder *fo = FO_GetFolderByType(FT_OUTGOING, NULL);
   struct Mail *mail;
   int tots = 0;
   int hidden;

   if(!fo)
    return;

   for(mail = fo->Messages; mail; mail = mail->Next)
   {
      if(!hasStatusHold(mail) && !hasStatusError(mail))
        tots++;
   }

   if(!tots)
    return;

   MA_ChangeFolder(fo, TRUE);

   hidden = xget(G->App, MUIA_Application_Iconified);

   if(hidden || MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_YesNoReq), GetStr(MSG_SendStartReq)))
      MA_Send(SEND_ALL);
}
///
/// DoStartup
//  Performs different checks/cleanup operations on startup
static void DoStartup(BOOL nocheck, BOOL hide)
{
   // Display the AppIcon now because if non of the below
   // do it it could happen that no AppIcon will be displayed at all.
   DisplayAppIconStatistics();

   if (C->CleanupOnStartup) DoMethod(G->App, MUIM_CallHook, &MA_DeleteOldHook);
   if (C->RemoveOnStartup) DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook);
   if (C->CheckBirthdates && !nocheck && !hide) AB_CheckBirthdates();
   if (TR_IsOnline())
   {
      if (C->GetOnStartup && !nocheck)
      {
         if(hide || C->PreSelection == 0)
         {
            MA_PopNow(POP_START, -1);
            if (G->TR) DisposeModule(&G->TR);
         }
         else
         {
            MA_PopNow(POP_USER, -1);

            // If the user has some preselection transfer
            // selected we have to get into a local event loop
            if(G->TR && xget(G->TR->GUI.WI, MUIA_Window_Open))
            {
              LONG result;

              set(G->MA->GUI.WI, MUIA_Window_Sleep, TRUE);

              DoMethod(G->TR->GUI.WI, MUIM_Notify, MUIA_Window_Open, FALSE, G->App, 2, MUIM_Application_ReturnID, 100);

              // lets collect the waiting returnIDs now
              COLLECT_RETURNIDS;

              do {
                ULONG signals;
                result = DoMethod(G->App, MUIM_Application_NewInput, &signals);
                if (result > 0) break;
                if (signals) Wait(signals);
              } while (result == 0);

              // now lets reissue the collected returnIDs again
              REISSUE_RETURNIDS;

              set(G->MA->GUI.WI, MUIA_Window_Sleep, FALSE);
            }

            if(G->TR) DisposeModule(&G->TR);
         }

         DoMethod(G->App, MUIM_Application_InputBuffered);
      }

      if (C->SendOnStartup && !nocheck)
      {
         SendWaitingMail();
         if (G->TR) DisposeModule(&G->TR);
         DoMethod(G->App, MUIM_Application_InputBuffered);
      }
   }
}
///
/// Login
//  Allows automatic login for AmiTCP-Genesis users
static void Login(char *user, char *password, char *maildir, char *prefsfile)
{
   struct genUser *guser;
   BOOL terminate = FALSE;
   BOOL loggedin = FALSE;

   if(INITLIB(IGenesis, InitLib("genesis.library", (APTR)&GenesisBase, 1, 0, FALSE, FALSE)))
   {
      if((guser = GetGlobalUser()))
      {
         terminate = !(loggedin = US_Login(guser->us_name, "\01", maildir, prefsfile));
         FreeUser(guser);
      }

      CLOSELIB(GenesisBase, IGenesis);
   }

   if(!loggedin && !terminate)
      terminate = !US_Login(user, password, maildir, prefsfile);

   if(terminate)
     exit(5);
}
///
/// GetDST
//  Checks if daylight saving time is active
//  return 0 if no DST system was found - 1 if no DST is set and 3 if set
static int GetDST(BOOL update)
{
   char buffer[50];
   char *tmp;
   int result = 0;

   // prepare the NotifyRequest structure
   if(!update) memset(&ADSTdata, 0, sizeof(struct ADST_Data));

   /* lets check the DaylightSaving stuff now
    * we check in the following order:
    * 1. SetDST (ENV:TZONE)
    * 2. FACTS (ENV:FACTS/DST)
    * 3. SummertimeGuard (ENV:SUMMERTIME)
    * 4. ixemul (ENV:IXGMTOFFSET)
    */

   // SetDST saves the DST settings in the TZONE env-variable which
   // is a bit more complex than the others, so we need to do some advance parsing
   if((!update || ADSTdata.method == ADST_SETDST)
      && GetVar((STRPTR)&ADSTfile[ADST_SETDST][4], buffer, 50, 0) >= 3)
   {
      int i;

      for(i=0; buffer[i]; i++)
      {
        if(result == 0)
        {
          // if we found the time difference in the TZONE variable we at least found a correct TZONE file
          if(buffer[i] >= '0' && buffer[i] <= '9') result = 1;
        }
        else if(isalpha(buffer[i])) result = 2; // if it is followed by a alphabetic sign we are in DST mode
      }

      ADSTdata.method = ADST_SETDST;
   }

   // FACTS saves the DST information in a ENV:FACTS/DST env variable which will be
   // Hex 00 or 01 to indicate the DST value.
   if((!update || ADSTdata.method == ADST_FACTS) && result == 0
      && GetVar((STRPTR)&ADSTfile[ADST_FACTS][4], buffer, 50, GVF_BINARY_VAR) > 0)
   {
      ADSTdata.method = ADST_FACTS;

      if(buffer[0] == 0x01)       return 2;
      else if(buffer[0] == 0x00)  return 1;
   }

   // SummerTimeGuard sets the last string to "YES" if DST is actually active
   if((!update || ADSTdata.method == ADST_SGUARD) && result == 0
      && GetVar((STRPTR)&ADSTfile[ADST_SGUARD][4], buffer, 50, 0) > 3 && (tmp = strrchr(buffer, ':')))
   {
      ADSTdata.method = ADST_SGUARD;

      if(tmp[1] == 'Y')       return 2;
      else if(tmp[1] == 'N')  return 1;
   }

   // ixtimezone sets the fifth byte in the IXGMTOFFSET variable to 01 if
   // DST is actually active.
   if((!update || ADSTdata.method == ADST_IXGMT) && result == 0
      && GetVar((STRPTR)&ADSTfile[ADST_IXGMT][4], buffer, 50, GVF_BINARY_VAR) >= 4)
   {
      ADSTdata.method = ADST_IXGMT;

      if(buffer[4] == 0x01)       return 2;
      else if(buffer[4] == 0x00)  return 1;
   }

   if(!update && result == 0) ADSTdata.method = ADST_NONE;

   // No correctly installed AutoDST tool was found
   // so lets return zero.
   return result;
}
///

/// yam_exitfunc()
/* This makes it possible to leave YAM without explicitely calling cleanup procedure */
static void yam_exitfunc(void)
{
   DB(kprintf("yam_exitfunc()\n");)

   if(olddirlock != -1)
   {  Terminate();
      CurrentDir(olddirlock);
   }
   if(nrda.Template)
      NewFreeArgs(&nrda);

   // close some libraries now
   CLOSELIB(UtilityBase,    IUtility);
   CLOSELIB(IconBase,       IIcon);
   CLOSELIB(IntuitionBase,  IIntuition);

   DB(kprintf("end of yam_exitfunc()\n");)
}

///
/// Main
//  Program entry point, main loop
int main(int argc, char **argv)
{
   struct Args args;
   BOOL yamFirst;
   BPTR progdir;
   LONG err;

   // obtain the MainInterface of Exec before anything else.
   #ifdef __amigaos4__
   IExec = (struct ExecIFace *)((*(struct ExecBase **)4L)->MainInterface);

   // check the exec version first and force be at least an 51.4 version because
	 // this is the version with the fixed crosscall hooks.
	 if(SysBase->lib_Version < 51 ||
		  (SysBase->lib_Version == 51 && SysBase->lib_Revision < 4))
	 {
      if((IntuitionBase = (APTR)OpenLibrary("intuition.library", 36)) &&
         GETINTERFACE(IIntuition, IntuitionBase))
      {
        struct EasyStruct ErrReq;

        ErrReq.es_StructSize = sizeof(struct EasyStruct);
        ErrReq.es_Flags      = 0;

        ErrReq.es_Title        = "YAM Startup Error";
        ErrReq.es_TextFormat   = "This version of YAM requires at least\n"
                                 "an AmigaOS4 kernel version 51.4";
        ErrReq.es_GadgetFormat = "Exit";

        EasyRequestArgs(NULL, &ErrReq, NULL, NULL);

        CLOSELIB(IntuitionBase, IIntuition);
        exit(0);
      }
	 }
   #endif

#if defined(DEVWARNING)
   {
     BOOL goon = TRUE;

     if((IntuitionBase = (APTR)OpenLibrary("intuition.library", 36)) &&
        GETINTERFACE(IIntuition, IntuitionBase))
     {
       if((UtilityBase = (APTR)OpenLibrary("utility.library", 36)) &&
          GETINTERFACE(IUtility, UtilityBase))
       {
         struct EasyStruct ErrReq;
         struct DateStamp ds;
         DateStamp(&ds); // get actual time/date

         ErrReq.es_StructSize = sizeof(struct EasyStruct);
         ErrReq.es_Flags      = 0;

         if(EXPDATE <= ds.ds_Days)
         {
           ErrReq.es_Title        = "YAM Developer Version Expired!";
           ErrReq.es_TextFormat   = "This developer version of YAM has expired!\n\n"
                                    "Please note that you may download a new, updated\n"
                                    "version from the YAM nightly build page at:\n\n"
                                    "http://nightly.yam.ch/\n\n"
                                    "All developer versions will automatically expire\n"
                                    "after a certian time interval. This is to insure\n"
                                    "that no old versions are floating around causing\n"
                                    "users to report bugs on old versions.\n\n"
                                    "Thanks for your help in improving YAM!";
           ErrReq.es_GadgetFormat = "Exit";

           DisplayBeep(NULL);
           EasyRequestArgs(NULL, &ErrReq, NULL, NULL);

           goon = FALSE;
         }

         if(goon && !getenv("I_KNOW_YAM_IS_UNDER_DEVELOPMENT"))
         {
           ErrReq.es_Title        = "YAM Developer Version Warning!";
           ErrReq.es_TextFormat   = "This is an *internal* developer version and\n"
                                    "not recommended or intended for public use.\n"
                                    "It may contain bugs that can lead to any loss\n"
                                    "of data and no regular support for this version\n"
                                    "will be provided in any form.\n\n"
                                    "In addition, this version will automatically\n"
                                    "expire 30 days after its compilation date.\n\n"
                                    "So if you're unsure and prefer to have a stable\n"
                                    "installation instead of a possible dangerous\n"
                                    "version, please consider to use the current\n"
                                    "stable release version available from:\n\n"
                                    "http://www.yam.ch/\n\n"
                                    "Thanks for your help in improving YAM!";
           ErrReq.es_GadgetFormat = "Go on|Exit";

           DisplayBeep(NULL);
           if(!EasyRequestArgs(NULL, &ErrReq, NULL, NULL))
             goon = FALSE;
         }

       }

       CLOSELIB(UtilityBase, IUtility);
     }

     CLOSELIB(IntuitionBase, IIntuition);
     if(!goon)
       exit(0);
   }
#endif

   atexit(yam_exitfunc); /* we need to free the stuff on exit()! */

   memset(&args, 0, sizeof(struct Args));

   WBmsg = (struct WBStartup *)(0 == argc ? argv : NULL);

   INITLIB(IIntuition, InitLib("intuition.library", (APTR)&IntuitionBase, 36, 0, TRUE, FALSE));
   INITLIB(IIcon,      InitLib("icon.library",      (APTR)&IconBase,      36, 0, TRUE, FALSE));
   INITLIB(IUtility,   InitLib("utility.library",   (APTR)&UtilityBase,   36, 0, TRUE, FALSE));

   nrda.Template = "USER/K,PASSWORD/K,MAILDIR/K,PREFSFILE/K,NOCHECK/S,HIDE/S,DEBUG/S,MAILTO/K,SUBJECT/K,LETTER/K,ATTACH/M";
   nrda.ExtHelp = NULL;
   nrda.Window = NULL;
   nrda.Parameters = (LONG *)&args;
   nrda.FileParameter = -1;
   nrda.PrgToolTypesOnly = FALSE;
   if ((err = NewReadArgs(WBmsg, &nrda)))
   {
      PrintFault(err, "YAM");
      exit(5);
   }

   if(!(progdir = GetProgramDir())) /* security only, can happen for residents only */
      exit(5);
   olddirlock = CurrentDir(progdir);

   for(yamFirst=TRUE;;)
   {
      ULONG signals, timsig, adstsig, rexsig, appsig, notsig[MAXWR+1], i;
      struct Message *msg;
      struct User *user;
      int wrwin, ret;

      G = calloc(1, sizeof(struct Global));
      C = calloc(1, sizeof(struct Config));
      NameFromLock(progdir, G->ProgDir, sizeof(G->ProgDir));

      if(!args.maildir) strcpy(G->MA_MailDir, G->ProgDir);
      args.hide = -args.hide;
      args.nocheck = -args.nocheck;
      G->TR_Debug = -args.debug;
      G->TR_Allow = TRUE;
      G->CO_DST = GetDST(FALSE);

      // prepare the List of ReadMailData structures
      NewList((struct List *)&(G->ReadMailDataList));

      // We have to initialize the ActiveWin flags to -1, so than the
      // the arexx commands for the windows are reporting an error if
      // some window wasn`t set active manually by an own rexx command.
      G->ActiveWriteWin = -1;

      if(yamFirst)
      {
         Initialise((BOOL)args.hide);
         Login(args.user, args.password, args.maildir, args.prefsfile);
         Initialise2();
      }
      else
      {
         Initialise(FALSE);
         Login(NULL, NULL, NULL, NULL);
         Initialise2();
      }

      DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENVARC);
      AppendLog(0, GetStr(MSG_LOG_Started), "", "", "", "");
      MA_StartMacro(MACRO_STARTUP, NULL);

      if(yamFirst)
      {
        DoStartup((BOOL)args.nocheck, (BOOL)args.hide);
        if (args.mailto || args.letter || args.subject || args.attach) if ((wrwin = MA_NewNew(NULL, 0)) >= 0)
        {
          if (args.mailto) setstring(G->WR[wrwin]->GUI.ST_TO, args.mailto);
          if (args.subject) setstring(G->WR[wrwin]->GUI.ST_SUBJECT, args.subject);
          if (args.letter) FileToEditor(args.letter, G->WR[wrwin]->GUI.TE_EDIT);
          if (args.attach)
          {
            char **sptr;
            for (sptr = args.attach; *sptr; sptr++)
            {
              if (FileSize(*sptr) >= 0) WR_AddFileToList(wrwin, *sptr, NULL, FALSE);
            }
          }
        }
        yamFirst = FALSE;
      }
      else DisplayAppIconStatistics();

      user = US_GetCurrentUser();
      AppendLogNormal(1, GetStr(MSG_LOG_LoggedIn), user->Name, "", "", "");
      AppendLogVerbose(2, GetStr(MSG_LOG_LoggedInVerbose), user->Name, G->CO_PrefsFile, G->MA_MailDir, "");

      // start our TimerIO requests for different purposes (writeindexes/mailcheck/autosave)
      TC_Start(TIO_WRINDEX,   C->WriteIndexes, 0);
      TC_Start(TIO_CHECKMAIL, C->CheckMailDelay*60, 0);
      TC_Start(TIO_AUTOSAVE,  C->AutoSave, 0);

      // Now start the NotifyRequest for the AutoDST file
      if(ADSTnotify_start()) adstsig = 1UL << ADSTdata.nRequest.nr_stuff.nr_Signal.nr_SignalNum;
      else                   adstsig = 0;

      // prepare the other signal bits
      timsig    = 1UL << TCData.port->mp_SigBit;
      rexsig    = 1UL << G->RexxHost->port->mp_SigBit;
      appsig    = 1UL << G->AppPort->mp_SigBit;
      notsig[0] = 1UL << G->WR_NRequest[0].nr_stuff.nr_Msg.nr_Port->mp_SigBit;
      notsig[1] = 1UL << G->WR_NRequest[1].nr_stuff.nr_Msg.nr_Port->mp_SigBit;
      notsig[2] = 1UL << G->WR_NRequest[2].nr_stuff.nr_Msg.nr_Port->mp_SigBit;

      #ifdef DEBUG
      kprintf("YAM allocated signals:\n");
      kprintf(" adstsig  = %08lx\n", adstsig);
      kprintf(" timsig   = %08lx\n", timsig);
      kprintf(" rexsig   = %08lx\n", rexsig);
      kprintf(" appsig   = %08lx\n", appsig);
      kprintf(" notsig[0]= %08lx\n", notsig[0]);
      kprintf(" notsig[1]= %08lx\n", notsig[1]);
      kprintf(" notsig[2]= %08lx\n", notsig[2]);
      #endif

      // start the event loop
      while (!(ret = Root_GlobalDispatcher(DoMethod(G->App, MUIM_Application_NewInput, &signals))))
      {
         if (signals)
         {
            signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_F | timsig | rexsig | appsig | notsig[0] | notsig[1] | notsig[2] | adstsig);
            if (signals & SIGBREAKF_CTRL_C) { ret = 1; break; }
            if (signals & SIGBREAKF_CTRL_D) { ret = 0; break; }
            if (signals & SIGBREAKF_CTRL_F) PopUp();

            // check for a TimerIO event
            if(signals & timsig)
            {
              int i;
              struct timerequest *timeReq;

              // check if we have a waiting message
              while((timeReq = (struct timerequest *)GetMsg(TCData.port)))
              {
                for(i=0; i < TIO_NUM; i++)
                {
                  if(timeReq == TCData.timerIO[i])
                  {
                    // call the dispatcher with signalling which timerIO
                    // this request caused
                    TC_Dispatcher(i);
                    break;
                  }
                }

                // no ReplyMsg() needed
              }
            }

            // check for a Arexx signal
            if (signals & rexsig) ARexxDispatch(G->RexxHost);

            // check for a AppMessage signal
            if (signals & appsig)
            {
               struct AppMessage *apmsg;
               while ((apmsg = (struct AppMessage *)GetMsg(G->AppPort)))
               {
                  if (apmsg->am_Type == AMTYPE_APPICON)
                  {
                     PopUp();
                     if (apmsg->am_NumArgs)
                     {
                        int wrwin;
                        if      (G->WR[0]) wrwin = 0;
                        else if (G->WR[1]) wrwin = 1;
                        else wrwin = MA_NewNew(NULL, 0);
                        if (wrwin >= 0) WR_App(wrwin, apmsg);
                     }
                  }
                  ReplyMsg(&apmsg->am_Message);
               }
            }

            // check for the write window signals
            for(i = 0; i <= MAXWR; i++)
            {
               if (signals & notsig[i])
               {
                  while ((msg = GetMsg(G->WR_NRequest[i].nr_stuff.nr_Msg.nr_Port))) ReplyMsg(msg);
                  if (G->WR[i]) FileToEditor(G->WR_Filename[i], G->WR[i]->GUI.TE_EDIT);
               }
            }

            // check for the AutoDST signal
            if(signals & adstsig)
            {
              // check the DST file and validate the configuration once more.
              G->CO_DST = GetDST(TRUE);
              CO_Validate(C, FALSE);
            }
         }
      }
      if (C->SendOnQuit && !args.nocheck) if (TR_IsOnline()) SendWaitingMail();
      if (C->CleanupOnQuit) DoMethod(G->App, MUIM_CallHook, &MA_DeleteOldHook);
      if (C->RemoveOnQuit) DoMethod(G->App, MUIM_CallHook, &MA_DeleteDeletedHook, TRUE);

      AppendLog(99, GetStr(MSG_LOG_Terminated), "", "", "", "");
      MA_StartMacro(MACRO_QUIT, NULL);

      FreeData2D(&Header);

      // if the user really wants to exit, do it now as Terminate() is broken !
      if(ret == 1) exit(0);

      DB(kprintf("Restart issued\n");)

      // prepare for restart
      Terminate();
   }
   /* not reached */
   return 0;
}
///

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

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include <exec/execbase.h>
#include <libraries/amisslmaster.h>
#include <libraries/asl.h>
#include <mui/BetterString_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NFloattext_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <mui/TheBar_mcc.h>
#include <mui/NBalance_mcc.h>
#include <proto/amissl.h>
#include <proto/amisslmaster.h>
#include <proto/codesets.h>
#include <proto/datatypes.h>
#include <proto/diskfont.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/layers.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <proto/rexxsyslib.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xadmaster.h>
#include <proto/xpkmaster.h>
#include <proto/expat.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#if !defined(__amigaos4__)
#include <proto/cybergraphics.h>
#endif

#include "extrasrc.h"
#include "extrasrc/NewReadArgs.h"

#include "YAM.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_write.h"
#include "YAM_utilities.h"

#include "tcp/Connection.h"
#include "mui/ClassesExtra.h"
#include "mui/ClassesSetup.h"
#include "mui/AddressBookWindow.h"
#include "mui/MainFolderListtree.h"
#include "mui/MainWindow.h"
#include "mui/SplashWindow.h"
#include "mui/ShutdownWindow.h"
#include "mui/WriteWindow.h"
#include "mui/YAMApplication.h"

#include "AppIcon.h"
#include "BayesFilter.h"
#include "Busy.h"
#include "Config.h"
#include "DockyIcon.h"
#include "FileInfo.h"
#include "MUIObjects.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "Logfile.h"
#include "MailList.h"
#include "MailServers.h"
#include "MethodStack.h"
#include "Requesters.h"
#include "Rexx.h"
#include "Threads.h"
#include "Timer.h"
#include "TZone.h"
#include "UpdateCheck.h"
#include "UserIdentity.h"

#include "Debug.h"

/***************************************************************************
 Module: Root
***************************************************************************/

struct Global *G = NULL;

struct Args
{
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
  LONG   noImgWarning;
  LONG   noCatalog;
  LONG   noSplashWindow;
};

static struct NewRDArgs nrda;
static struct Args args;
static BPTR olddirlock = (BPTR)-1; /* -1 is an unset indicator */

/**************************************************************************/

static void Abort(const char *message, ...);

/**************************************************************************/

// Semaphore related suff
static struct StartupSemaphore
{
  struct SignalSemaphore semaphore; // a standard semaphore structure
  ULONG UseCount;                   // how many other participants know this semaphore
} *startupSemaphore = NULL;

#define STARTUP_SEMAPHORE_NAME      "YAM_Startup"

/*** Library/MCC check routines ***/
/// InitLib
//  Opens a library & on OS4 also the interface
#if defined(__amigaos4__)
static BOOL InitLib(const char *libname,
                    ULONG version,
                    ULONG revision,
                    struct Library **libbase,
                    const char *iname,
                    ULONG iversion,
                    struct Interface **iface,
                    BOOL required,
                    const char *homepage)
#else
static BOOL InitLib(const char *libname,
                    ULONG version,
                    ULONG revision,
                    struct Library **libbase,
                    BOOL required,
                    const char *homepage)
#endif
{
  struct Library *base = NULL;

  ENTER();

  #if defined(__amigaos4__)
  if(libbase != NULL && iface != NULL)
  #else
  if(libbase != NULL)
  #endif
  {
    // open the library base
    base = OpenLibrary(libname, version);

    if(base != NULL && revision != 0)
    {
      if(LIB_VERSION_IS_AT_LEAST(base, version, revision) == FALSE)
      {
        CloseLibrary(base);
        base = NULL;
      }
    }

    // if we end up here, we can open the OS4 base library interface
    if(base != NULL)
    {
      #if defined(__amigaos4__)
      struct Interface *i;

      // if we weren't able to obtain the interface, lets close the library also
      if(GETINTERFACE(iname, iversion, i, base) == NULL)
      {
        D(DBF_STARTUP, "InitLib: can't get v%ld '%s' interface of library %s", iversion, iname, libname);

        CloseLibrary(base);
        *libbase = NULL;
        base = NULL;
      }
      else
        D(DBF_STARTUP, "InitLib: library %s v%ld.%ld with iface '%s' v%ld successfully opened.", libname, base->lib_Version, base->lib_Revision, iname, iversion);

      // store interface pointer
      *iface = i;
      #else
      D(DBF_STARTUP, "InitLib: library %s v%ld.%ld successfully opened.", libname, base->lib_Version, base->lib_Revision);
      #endif
    }
    else
      D(DBF_STARTUP, "InitLib: can't open library %s with minimum version v%ld.%ld", libname, version, revision);

    if(base == NULL && required == TRUE)
    {
      if(homepage != NULL)
      {
        char error[SIZE_LINE];
        BOOL gotoURLPossible = GotoURLPossible();
        LONG answer;

        snprintf(error, sizeof(error), tr(MSG_ER_LIB_URL), libname, version, revision, homepage);

        if(MUIMasterBase != NULL && G != NULL && G->App != NULL)
        {
          answer = MUI_Request(G->App, NULL, MUIF_NONE, tr(MSG_ErrorStartup), (gotoURLPossible == TRUE) ? tr(MSG_HOMEPAGE_QUIT_GAD) : tr(MSG_Quit), error);
        }
        else if(IntuitionBase != NULL)
        {
          struct EasyStruct ErrReq;

          ErrReq.es_StructSize   = sizeof(struct EasyStruct);
          ErrReq.es_Flags        = 0;
          ErrReq.es_Title        = (STRPTR)tr(MSG_ErrorStartup);
          ErrReq.es_TextFormat   = error;
          ErrReq.es_GadgetFormat = (gotoURLPossible == TRUE) ? (STRPTR)tr(MSG_HOMEPAGE_QUIT_GAD) : (STRPTR)tr(MSG_Quit);

          answer = EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
        }
        else
        {
          puts(error);
          answer = 0;
        }

        // visit the home page if the user requested that
        if(answer == 1)
          GotoURL(homepage, FALSE);

        Abort(NULL);
      }
      else
        Abort(tr(MSG_ER_LIB), libname, version, revision);
    }

    // store base
    *libbase = base;
  }

  RETURN((BOOL)(base != NULL));
  return (BOOL)(base != NULL);
}

///
/// CheckMCC
//  Checks if a certain version of a MCC is available
static BOOL CheckMCC(const char *name,
                     ULONG minver, ULONG minrev,
                     ULONG maxver, ULONG maxrev,
                     BOOL req, const char *url)
{
  BOOL success = FALSE;
  BOOL flush = TRUE;
  BOOL gotoURLPossible;

  ENTER();

  D(DBF_STARTUP, "checking for v%ld.%ld+ of '%s'", minver, minrev, name);

  gotoURLPossible = GotoURLPossible();

  // lets go any try to find out if the
  // mcc is there and in the right condition to be used.
  do
  {
    Object *obj;
    struct MUI_CustomClass *subclass;

    // First we attempt to acquire the version and revision through MUI via
    // creating an object of the mcc
    if((obj = MUI_NewObject(name, TAG_DONE)) != NULL)
    {
      ULONG ver = xget(obj, MUIA_Version);
      ULONG rev = xget(obj, MUIA_Revision);

      MUI_DisposeObject(obj);
      obj = NULL;

      D(DBF_STARTUP, "found %s v%ld.%ld through MUIA_Version/Revision", name, ver, rev);

      // check that the minimum required version/revision is matched
      if(VERSION_IS_AT_LEAST(ver, rev, minver, minrev) == TRUE)
      {
        D(DBF_STARTUP, "found %s to be >= v%ld.%ld", name, minver, minrev);

        // check that the maximum version/revision is not exceeded
        if(maxver > 0 &&
           VERSION_IS_LOWER(ver, rev, maxver, maxrev) == FALSE)
        {
          W(DBF_STARTUP, "found %s to be >= v%ld.%ld (max)", name, maxver, maxrev);
        }
        else
          success = TRUE; // success, the mcc meets our requirements
      }
    }

    // now we try out if we can subclass the mcc as otherwise this might be a fake
    // or broken mcc which needs to be reinstalled by the user.
    if((subclass = MUI_CreateCustomClass(NULL, name, NULL, 0, NULL)) != NULL)
    {
      D(DBF_STARTUP, "successfully created test custom class of '%s'", name);
      MUI_DeleteCustomClass(subclass);
      subclass = NULL;
    }
    else
    {
      W(DBF_STARTUP, "couldn't create test custom class of '%s'", name);
      success = FALSE;
    }

    // if we end up here the version of the .mcc couldn't either be retrieved
    // by creating a MUI object of it (e.g. because it requires more mandatory
    // attribute to create the object) or because the version didn't meet
    // our requirements.
    //
    // So what we do now is to try to open the .mcc as a normal library and
    // see if the version of the library base of it matches.
    if(success == FALSE)
    {
      struct Library *base;
      char libname[SIZE_DEFAULT];

      // If we did't get the version we wanted, let's try to open the
      // MCC via OpenLibrary() ourselves and see what happens
      snprintf(libname, sizeof(libname), "mui/%s", name);

      // we check to find the mcc via querying it by "mui/XXXXX.mcc". As ramlib
      // also automatically checks in PROGDIR: this should catch the case where
      // the user has falsly installed the .mcc under YAM:mui
      if((base = OpenLibrary(libname, 0)) != NULL)
      {
        ULONG ver = base->lib_Version;
        ULONG rev = base->lib_Revision;
        UWORD openCnt = base->lib_OpenCnt;

        D(DBF_STARTUP, "successfully opened '%s' as library, found v%ld.%ld, open count %ld", name, ver, rev, openCnt);

        // close the library immediately as we don't require any more
        // information from it
        CloseLibrary(base);
        base = NULL;

        // we add some additional check here so that eventual broken .mcc also have
        // a chance to pass this test (e.g. _very_ old versions of Toolbar.mcc are broken
        // and don't have any MUIA_Version/Revision attributes.
        if(VERSION_IS_AT_LEAST(ver, rev, minver, minrev) == TRUE)
        {
          success = TRUE;
          break;
        }

        // the version still doesn't match our expectations, so let's see
        // if the library is already opened by another application and if so
        // we give the user the chance to select what he wants to do to solve
        // the problem (as he might still have an old version in memory)
        if(openCnt > 1)
        {
          if(req == TRUE)
          {
            LONG answer;

            answer = MUI_Request(G != NULL ? G->App : NULL, NULL, MUIF_NONE, tr(MSG_ErrorStartup), (gotoURLPossible == TRUE) ? tr(MSG_RETRY_HOMEPAGE_QUIT_GAD) : tr(MSG_RETRY_QUIT_GAD), tr(MSG_ER_MCC_IN_USE), name, minver, minrev, ver, rev, url);
            if(answer == 0)
            {
              // cancel
              break;
            }
            else if(answer == 1)
            {
              // flush and retry
              flush = TRUE;
            }
            else
            {
              // visit the home page if it is known but bail out nevertheless
              GotoURL(url, FALSE);
              break;
            }
          }
          else
            break;
        }

        // Attempt to flush the library if open count is 0 or because the
        // user wants to retry (meaning there's a chance that it's 0 now)
        if(flush == TRUE)
        {
          struct Library *result;

          D(DBF_STARTUP, "trying to flush '%s'", name);
          Forbid();
          if((result = (struct Library *)FindName(&((struct ExecBase *)SysBase)->LibList, name)) != NULL)
            RemLibrary(result);
          Permit();
          flush = FALSE;
        }
        else
        {
          E(DBF_STARTUP, "couldn't find minimum required v%ld.%ld of '%s'", minver, minrev, name);

          // We're out of luck - open count is 0, we've tried to flush
          // and still haven't got the version we want
          if(req == TRUE)
          {
            LONG answer;

            answer = MUI_Request(G != NULL ? G->App : NULL, NULL, MUIF_NONE, tr(MSG_ErrorStartup), (gotoURLPossible == TRUE) ? tr(MSG_RETRY_HOMEPAGE_QUIT_GAD) : tr(MSG_RETRY_QUIT_GAD), tr(MSG_ER_MCC_OLD), name, minver, minrev, ver, rev, url);
            if(answer == 0)
            {
              // cancel
              break;
            }
            else if(answer == 1)
            {
              // flush and retry
              flush = TRUE;
            }
            else
            {
              // visit the home page if it is known but bail out nevertheless
              GotoURL(url, FALSE);
              break;
            }
          }
          else
            break;
        }
      }
      else
      {
        LONG answer;

        // No MCC at all - no need to attempt flush
        flush = FALSE;
        answer = MUI_Request(G != NULL ? G->App : NULL, NULL, MUIF_NONE, tr(MSG_ErrorStartup), (gotoURLPossible == TRUE) ? tr(MSG_RETRY_HOMEPAGE_QUIT_GAD) : tr(MSG_RETRY_QUIT_GAD), tr(MSG_ER_NO_MCC), name, minver, minrev, url);

        if(answer == 0)
        {
          // cancel
          break;
        }
        else if(answer == 2)
        {
          // visit the home page if it is known but bail out nevertheless
          GotoURL(url, FALSE);
          break;
        }
      }
    }
  }
  while(success == FALSE);

  if(success == FALSE && req == TRUE)
    exit(RETURN_ERROR); // Ugly

  RETURN(success);
  return success;
}

///

/*** XPK Packer initialization routines ***/
/// InitXPKPackerList
// initializes the internal XPK PackerList
static BOOL InitXPKPackerList(void)
{
  BOOL result = FALSE;
  LONG error = 0;

  ENTER();

  // create the list first
  if((G->xpkPackerList = AllocSysObjectTags(ASOT_LIST,
    ASOLIST_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    if(XpkBase != NULL)
    {
      struct XpkPackerList *xpl;

      if((xpl = malloc(sizeof(*xpl))) != NULL)
      {
        // obtain the list of all available packers
        if((error = XpkQueryTags(XPK_PackersQuery, xpl, TAG_DONE)) == 0)
        {
          unsigned int i;

          D(DBF_XPK, "loaded XPK packer list, %ld packers found", xpl->xpl_NumPackers);

          // assume success for now
          result = TRUE;

          for(i=0; i < xpl->xpl_NumPackers; i++)
          {
            struct XpkPackerInfo xpi;

            // obtain the basic information about the individual packers
            if((error = XpkQueryTags(XPK_PackMethod, xpl->xpl_Packer[i], XPK_PackerQuery, &xpi, TAG_DONE)) == 0)
            {
              struct xpkPackerNode *newPacker;

              if((newPacker = AllocSysObjectTags(ASOT_NODE,
                ASONODE_Size, sizeof(*newPacker),
                ASONODE_Min, TRUE,
                TAG_DONE)) != NULL)
              {
                memcpy(&newPacker->info, &xpi, sizeof(newPacker->info));

                // because the short name isn't always equal to the packer short name
                // we work around that problem and make sure they are equal.
                strlcpy((char *)newPacker->info.xpi_Name, (char *)xpl->xpl_Packer[i], sizeof(newPacker->info.xpi_Name));

                D(DBF_XPK, "found XPK packer #%ld: '%s' flags = %08lx", i, newPacker->info.xpi_Name, newPacker->info.xpi_Flags);

                // add the new packer to our internal list.
                AddTail((struct List *)G->xpkPackerList, (struct Node *)newPacker);
              }
            }
            else
            {
              // something failed, so lets query the error!
              #if defined(DEBUG)
              char buf[1024];

              XpkFault(error, NULL, buf, sizeof(buf));

              E(DBF_XPK, "error on XpkQuery() of packer '%s': '%s'", xpl->xpl_Packer[i], buf);
              #endif

              result = FALSE;
            }
          }
        }
        else
        {
          // something failed, so lets query the error!
          #if defined(DEBUG)
          char buf[1024];

          XpkFault(error, NULL, buf, sizeof(buf));

          E(DBF_XPK, "error on general XpkQuery(): '%s'", buf);
          #endif
        }

        free(xpl);
      }
    }
  }

  RETURN((BOOL)(result == TRUE && error == 0));
  return (BOOL)(result == TRUE && error == 0);
}

///
/// FreeXPKPackerList
// free all content of our previously loaded XPK packer list
static void FreeXPKPackerList(void)
{
  ENTER();

  if(G->xpkPackerList != NULL)
  {
    struct Node *curNode;

    // subsequently remove all nodes from the list and free them
    while((curNode = RemHead((struct List *)G->xpkPackerList)) != NULL)
    {
      // free everything of the node
      FreeSysObject(ASOT_NODE, curNode);
    }

    FreeSysObject(ASOT_LIST, G->xpkPackerList);
    G->xpkPackerList = NULL;
  }

  LEAVE();
}

///

/*** Synchronization routines ***/
/// CreateStartupSemaphore
//  create a new startup semaphore or find an old instance
static struct StartupSemaphore *CreateStartupSemaphore(void)
{
  struct StartupSemaphore *semaphore;

  ENTER();

  D(DBF_STARTUP, "creating startup semaphore...");

  // we have to disable multitasking before looking for an old instance with the same name
  Forbid();
  if((semaphore = (struct StartupSemaphore *)FindSemaphore((STRPTR)STARTUP_SEMAPHORE_NAME)) != NULL)
  {
    // the semaphore already exists, so just bump the counter
    semaphore->UseCount++;
  }
  Permit();

  // if we didn't find any semaphore with that name we generate a new one
  if(semaphore == NULL)
  {
    // allocate the memory for the semaphore system structure itself
    if((semaphore = AllocSysObjectTags(ASOT_SEMAPHORE,
      ASOSEM_Size, sizeof(*semaphore),
      ASOSEM_Name, (ULONG)STARTUP_SEMAPHORE_NAME,
      ASOSEM_CopyName, TRUE,
      ASOSEM_Public, TRUE,
      TAG_DONE)) != NULL)
    {
      // initialize the semaphore structure and start with a use counter of 1
      semaphore->UseCount = 1;
    }
  }

  RETURN(semaphore);
  return semaphore;
}

///
/// DeleteStartupSemaphore
//  delete a public semaphore, removing it from the system if it is no longer in use
static void DeleteStartupSemaphore(void)
{
  ENTER();

  if(startupSemaphore != NULL)
  {
    // first obtain the semaphore so that nobody else can interfere
    ObtainSemaphore(&startupSemaphore->semaphore);

    // protect access to the semaphore
    Forbid();

    // now we can release the semaphore again, because nobody else can steal it
    ReleaseSemaphore(&startupSemaphore->semaphore);

    // one user less for this semaphore
    startupSemaphore->UseCount--;

    // if nobody else uses this semaphore it can be removed complete
    if(startupSemaphore->UseCount == 0)
    {
      // free the semaphore structure
      // this will also remove our public semaphore from the list
      FreeSysObject(ASOT_SEMAPHORE, startupSemaphore);
      startupSemaphore = NULL;
    }

    // free access to the semaphore
    Permit();
  }

  LEAVE();
}

///

/*** Application Abort/Termination routines ***/
/// Terminate
//  Deallocates used memory and MUI modules and terminates
static void Terminate(void)
{
  int i;
  struct Node *curNode;

  ENTER();

  // we are going down from now on
  G->Terminating = TRUE;

  D(DBF_STARTUP, "aborting all working threads...");
  AbortWorkingThreads();

  D(DBF_STARTUP, "cleaning up thread system...");
  CleanupThreads();

  D(DBF_STARTUP, "freeing spam filter module...");
  BayesFilterCleanup();

  D(DBF_STARTUP, "freeing config module...");
  if(G->ConfigWinObject != NULL)
    DoMethod(G->App, MUIM_YAMApplication_CloseConfigWindow);
  FreeConfig(CE);
  CE = NULL;

  D(DBF_STARTUP, "freeing readMailData...");
  // cleanup the still existing readmailData objects
  while((curNode = RemHead((struct List *)&G->readMailDataList)) != NULL)
  {
    struct ReadMailData *rmData = (struct ReadMailData *)curNode;

    CleanupReadMailData(rmData, TRUE);
  }

  D(DBF_STARTUP, "freeing writeMailData...");
  // cleanup the still existing writemailData objects
  while((curNode = RemHead((struct List *)&G->writeMailDataList)) != NULL)
  {
    struct WriteMailData *wmData = (struct WriteMailData *)curNode;

    CleanupWriteMailData(wmData);
  }

  D(DBF_STARTUP, "freeing user login window...");
  if(G->US != NULL)
    DisposeModule(&G->US);

  D(DBF_STARTUP, "finalizing indexes and closing main window...");
  if(G->MA != NULL)
  {
    DoMethod(G->App, MUIM_YAMApplication_FlushFolderIndexes, TRUE);
    // remember the current layout, but don't make that permanent yet
    SaveLayout(FALSE);
    set(G->MA->GUI.WI, MUIA_Window_Open, FALSE);
    // close any open folder edit window immediately
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_MainFolderListtree_CloseFolderEditWindow, TRUE);
  }

  D(DBF_STARTUP, "freeing folders...");
  if(G->folders != NULL)
  {
    struct FolderNode *fnode;

    LockFolderList(G->folders);

    ForEachFolderNode(G->folders, fnode)
    {
      FO_FreeFolder(fnode->folder);
      fnode->folder = NULL;
    }

    UnlockFolderList(G->folders);
    DeleteFolderList(G->folders);
  }

  D(DBF_STARTUP, "freeing addressbook module...");
  DoMethod(G->App, MUIM_YAMApplication_DisposeWindow, G->ABookWinObject);
  G->ABookWinObject = NULL;
  ClearABook(&G->abook);

  D(DBF_STARTUP, "freeing main window module...");
  if(G->MA != NULL)
    DisposeModule(&G->MA);

  D(DBF_STARTUP, "freeing FileReqCache structures...");
  for(i = 0; i < ASL_MAX; i++)
  {
    FreeFileReqCache(G->FileReqCache[i]);
    free(G->FileReqCache[i]);
    G->FileReqCache[i] = NULL;
  }

  FreeAppIcon();

  D(DBF_STARTUP, "freeing write window file notify port...");
  if(G->writeWinNotifyPort != NULL)
    FreeSysObject(ASOT_PORT, G->writeWinNotifyPort);

  D(DBF_STARTUP, "freeing Arexx port...");
  if(G->RexxHost != NULL)
    CloseDownARexxHost(G->RexxHost);

  D(DBF_STARTUP, "freeing timer resources...");
  CleanupTimers();

  // check if we have an allocated NewMailSound_Obj and dispose it.
  D(DBF_STARTUP, "freeing sound object...");
  if(G->SoundDTObj != NULL)
  {
    DoMethod(G->SoundDTObj, DTM_TRIGGER, NULL, STM_STOP, NULL);
    DisposeDTObject(G->SoundDTObj);
  }

  D(DBF_STARTUP, "freeing hideIcon...");
  if(G->HideIcon != NULL)
    FreeDiskObject(G->HideIcon);

  D(DBF_STARTUP, "deleting zombie files...");
  if(DeleteZombieFiles(FALSE) == FALSE)
  {
    BOOL ignore = FALSE;

    do
    {
      if(MUI_Request(G->App, NULL, MUIF_NONE, tr(MSG_ER_ZOMBIE_FILES_EXIST_TITLE),
                                              tr(MSG_ER_ZOMBIE_FILES_EXIST_BT),
                                              tr(MSG_ER_ZOMBIE_FILES_EXIST)) == 0)
      {
        D(DBF_STARTUP, "ignoring further zombie files");
        ignore = TRUE;
      }
    }
    while(DeleteZombieFiles(ignore) == FALSE);
  }

  // we unregister the application from application.library
  FreeDockyIcon();

  D(DBF_STARTUP, "freeing toolbar cache...");
  ToolbarCacheCleanup();

  if(G->ER != NULL)
    DisposeModule(&G->ER);

  D(DBF_STARTUP, "freeing %ld error messages...", G->ER_NumErr);
  for(i = 0; i < G->ER_NumErr; i++)
  {
    free(G->ER_Message[i]);
    G->ER_Message[i] = NULL;
  }
  G->ER_NumErr = 0;

  // free our private codesets list
  D(DBF_STARTUP, "freeing private codesets list...");
  if(G->codesetsList != NULL)
  {
    CodesetsListDelete(CSA_CodesetList, G->codesetsList,
                       TAG_DONE);

    G->codesetsList = NULL;
  }

  // free our private internal XPK PackerList
  D(DBF_STARTUP, "cleaning up XPK stuff...");
  FreeXPKPackerList();
  CLOSELIB(XpkBase, IXpk);

  // free our xad stuff
  D(DBF_STARTUP, "cleaning up XAD stuff...");
  CLOSELIB(xadMasterBase, IxadMaster);

  D(DBF_STARTUP, "freeing main application object...");
  if(G->App != NULL)
  {
    MUI_DisposeObject(G->App);
    G->App = NULL;
  }

  D(DBF_STARTUP, "freeing config...");
  FreeConfig(C);
  C = NULL;

  D(DBF_STARTUP, "unloading/freeing theme images...");
  FreeTheme(&G->theme);

  D(DBF_STARTUP, "freeing image cache...");
  ImageCacheCleanup();

  D(DBF_STARTUP, "freeing internal MUI classes...");
  YAM_CleanupClasses();

  D(DBF_STARTUP, "deleting semaphore...");
  DeleteStartupSemaphore();

  D(DBF_STARTUP, "cleaning up method stack...");
  CleanupMethodStack();

  D(DBF_STARTUP, "cleaning up connection stuff...");
  CleanupConnections();

  // cleaning up all AmiSSL stuff
  D(DBF_STARTUP, "cleaning up AmiSSL stuff...");
  if(AmiSSLBase != NULL)
  {
    CleanupAmiSSLA(NULL);

    DROPINTERFACE(IAmiSSL);
    CloseAmiSSL();
    AmiSSLBase = NULL;
  }
  CLOSELIB(AmiSSLMasterBase, IAmiSSLMaster);

  D(DBF_STARTUP, "cleaning up busy actions...");
  BusyCleanup();

  D(DBF_STARTUP, "cleaning up tzone stuff...");
  TZoneCleanup();

  // close all libraries now.
  D(DBF_STARTUP, "closing all opened libraries...");
  #if defined(__amigaos4__)
  CLOSELIB(ApplicationBase, IApplication);
  #else
  CLOSELIB(CyberGfxBase,    ICyberGfx);
  #endif
  CLOSELIB(ExpatBase,       IExpat);
  CLOSELIB(OpenURLBase,     IOpenURL);
  CLOSELIB(CodesetsBase,    ICodesets);
  CLOSELIB(DataTypesBase,   IDataTypes);
  CLOSELIB(MUIMasterBase,   IMUIMaster);
  CLOSELIB(RexxSysBase,     IRexxSys);
  CLOSELIB(IFFParseBase,    IIFFParse);
  CLOSELIB(KeymapBase,      IKeymap);
  CLOSELIB(LayersBase,      ILayers);
  CLOSELIB(WorkbenchBase,   IWorkbench);
  CLOSELIB(GfxBase,         IGraphics);

  // close the catalog and locale now
  D(DBF_STARTUP, "closing catalog...");
  CloseYAMCatalog();
  if(G->Locale != NULL)
  {
    CloseLocale(G->Locale);
    G->Locale = NULL;
  }

  CLOSELIB(LocaleBase, ILocale);

  // free the configuration semaphore
  if(G->configSemaphore != NULL)
  {
    FreeSysObject(ASOT_SEMAPHORE, G->configSemaphore);
    G->configSemaphore = NULL;
  }

  // free the global semaphore
  if(G->globalSemaphore != NULL)
  {
    FreeSysObject(ASOT_SEMAPHORE, G->globalSemaphore);
    G->globalSemaphore = NULL;
  }

  // free the two virtual mail parts
  free(G->virtualMailpart[0]);
  free(G->virtualMailpart[1]);

  // free the item pools
  if(G->mailItemPool != NULL)
  {
    FreeSysObject(ASOT_ITEMPOOL, G->mailItemPool);
    G->mailItemPool = NULL;
  }
  if(G->mailNodeItemPool != NULL)
  {
    FreeSysObject(ASOT_ITEMPOOL, G->mailNodeItemPool);
    G->mailNodeItemPool = NULL;
  }

  if(G->mailsInTransfer != NULL)
  {
    if(IsMailListEmpty(G->mailsInTransfer) == FALSE)
      E(DBF_NET, "there are still %ld mails being sent", G->mailsInTransfer->count);

    DeleteMailList(G->mailsInTransfer);
    G->mailsInTransfer = NULL;
  }

  if(G->lowMemHandler != NULL)
  {
    RemMemHandler(G->lowMemHandler);
    FreeSysObject(ASOT_INTERRUPT, G->lowMemHandler);
    G->lowMemHandler = NULL;
  }

  // make sure to free the shared memory pool before
  // freeing the rest
  if(G->SharedMemPool != NULL)
  {
    FreeSysObject(ASOT_MEMPOOL, G->SharedMemPool);
    G->SharedMemPool = NULL;
  }

  // last, but not least, free the global structure
  free(G);
  G = NULL;

  LEAVE();
}

///
/// Abort
//  Shows error requester, then terminates the program
static void Abort(const char *message, ...)
{
  ENTER();

  if(message != NULL)
  {
    va_list a;
    char error[SIZE_LINE];

    va_start(a, message);
    vsnprintf(error, sizeof(error), message, a);
    va_end(a);

    W(DBF_STARTUP, "aborting application due to reason '%s'", error);

    if(MUIMasterBase != NULL && G != NULL && G->App != NULL)
    {
      MUI_Request(G->App, NULL, MUIF_NONE, tr(MSG_ErrorStartup), tr(MSG_Quit), error);
    }
    else if(IntuitionBase != NULL)
    {
      struct EasyStruct ErrReq;

      ErrReq.es_StructSize   = sizeof(struct EasyStruct);
      ErrReq.es_Flags        = 0;
      ErrReq.es_Title        = (STRPTR)tr(MSG_ErrorStartup);
      ErrReq.es_TextFormat   = error;
      ErrReq.es_GadgetFormat = (STRPTR)tr(MSG_Quit);

      EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
    }
    else
      puts(error);
  }
  else
    W(DBF_STARTUP, "aborting application");

  // do a hard exit.
  exit(RETURN_ERROR);

  LEAVE();
}

///
/// yam_exitfunc
/* This makes it possible to leave YAM without explicitely calling cleanup procedure */
static void yam_exitfunc(void)
{
  ENTER();

  D(DBF_STARTUP, "cleaning up in 'yam_exitfunc'...");

  if(olddirlock != (BPTR)-1)
  {
    Terminate();
    CurrentDir(olddirlock);
  }

  // Free the NewReadArgs structure/memory
  if(nrda.Template != NULL)
    NewFreeArgs(&nrda);

  // close some libraries now
  CLOSELIB(DiskfontBase,   IDiskfont);
  CLOSELIB(UtilityBase,    IUtility);
  CLOSELIB(IconBase,       IIcon);
  CLOSELIB(IntuitionBase,  IIntuition);

  LEAVE();

  // cleanup our debugging system.
  #if defined(DEBUG)
  CleanupDebug();
  #endif
}

///

/// SplashProgress
//  Shows progress of program initialization in the splash window
static void SplashProgress(const char *txt, int percent)
{
  ENTER();

  DoMethod(G->SplashWinObject, MUIM_SplashWindow_StatusChange, txt, percent);

  LEAVE();
}

///
/// PopUp
//  Un-iconify YAM
void PopUp(void)
{
  ENTER();

  set(G->App, MUIA_Application_Iconified, FALSE);

  // let the main window appear only if we are fully started yet
  if(G->InStartupPhase == FALSE)
  {
    Object *window = G->MA->GUI.WI;
    struct ReadMailData *rmData;
    struct WriteMailData *wmData;

    // avoid MUIA_Window_Open's side effect of activating the window if it was already open
    if(xget(window, MUIA_Window_Open) == FALSE)
      set(window, MUIA_Window_Open, TRUE);

    DoMethod(window, MUIM_Window_ScreenToFront);
    DoMethod(window, MUIM_Window_ToFront);

    // Now we check if there is any read window open and bring it also
    // to the front
    IterateList(&G->readMailDataList, struct ReadMailData *, rmData)
    {
      if(rmData->readWindow != NULL)
      {
        DoMethod(rmData->readWindow, MUIM_Window_ToFront);
        window = rmData->readWindow;
      }
    }

    // Now we check if there is any write window open and bring it also
    // to the front
    IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
    {
      if(wmData->window != NULL)
      {
        DoMethod(wmData->window, MUIM_Window_ToFront);
        window = wmData->window;
      }
    }

    // now we activate the window that is on the top
    set(window, MUIA_Window_Activate, TRUE);
  }

  LEAVE();
}

///
/// StayInProg
//  Makes sure that the user really wants to quit the program
BOOL StayInProg(void)
{
  BOOL stayIn = FALSE;

  ENTER();

  if(stayIn == FALSE && G->abook.modified == TRUE)
  {
    int result;

    result = MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_ABOOK_MODIFIED_GAD), tr(MSG_AB_Modified));
    switch(result)
    {
      default:
      case 0:
      {
        // dont' quit
        stayIn = TRUE;
      }
      break;

      case 1:
      {
        // save and quit
        SaveABook(G->abookFilename, &G->abook);
      }
      break;

      case 2:
      {
        // quit without save
      }
      break;
    }
  }

  if(stayIn == FALSE && C->ConfigIsSaved == FALSE)
  {
    int result;

    result = MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_CONFIG_MODIFIED_GAD), tr(MSG_CONFIG_MODIFIED));
    switch(result)
    {
      default:
      case 0:
      {
        // dont' quit
        stayIn = TRUE;
      }
      break;

      case 1:
      {
        // save and quit
        SaveConfig(C, G->CO_PrefsFile);
      }
      break;

      case 2:
      {
        // quit without save
      }
      break;
    }
  }

  if(stayIn == FALSE)
  {
    // search through our WriteDataList
    struct WriteMailData *wmData;
    BOOL saveMails = FALSE;

    IterateList(&G->writeMailDataList, struct WriteMailData *, wmData)
    {
      if(wmData->window != NULL)
      {
        int result;

        result = MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_OPEN_WRITEWINDOWS_GAD), tr(MSG_OPEN_WRITEWINDOWS));
        switch(result)
        {
          default:
          case 0:
          {
            // dont' quit
            stayIn = TRUE;
          }
          break;

          case 1:
          {
            // save and quit
            saveMails = TRUE;
          }
          break;

          case 2:
          {
            // quit without save
          }
          break;
        }
      }
    }

    if(saveMails == TRUE)
    {
      struct WriteMailData *succ;

      // put the mails of all still opened write windows on hold
      // we must use SafeIterateList() here, because composing the final mail
      // will close the window and hence modify the list we are iterating over
      SafeIterateList(&G->writeMailDataList, struct WriteMailData *, wmData, succ)
      {
        if(wmData->window != NULL)
          DoMethod(wmData->window, MUIM_WriteWindow_ComposeMail, WRITE_QUEUE);
      }
    }
  }

  if(stayIn == FALSE)
  {
    if(G->ConfigWinObject != NULL || C->ConfirmOnQuit == TRUE)
    {
      if(MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, tr(MSG_MA_ConfirmReq), tr(MSG_YesNoReq), tr(MSG_QuitYAMReq)) == 0)
        stayIn = TRUE;
    }
  }

  RETURN(stayIn);
  return stayIn;
}

///
/// Root_GlobalDispatcher
//  Processes return value of MUI_Application_NewInput
static int Root_GlobalDispatcher(ULONG app_input)
{
  int ret = 0;

  // no ENTER() call here, because this function will be called *very*
  // often and a @all or @ctrace log will be swamped with too much
  // useless trace informations.

  switch(app_input)
  {
    // user initiated a normal QUIT command
    case MUIV_Application_ReturnID_Quit:
    {
      if(xget(G->App, MUIA_Application_ForceQuit) == FALSE)
        ret = StayInProg() ? 0 : 1;
      else
        ret = 1;
    }
    break;

    // user closed the main window
    case ID_CLOSEALL:
    {
      if(C->IconifyOnQuit == FALSE)
        ret = StayInProg() ? 0 : 1;
      else
        set(G->App, MUIA_Application_Iconified, TRUE);
    }
    break;

    // user initiated a 'restart' action
    case ID_RESTART:
    {
      ret = StayInProg() ? 0 : 2;
    }
    break;

    // user initiated a 'forced restart' action
    case ID_RESTART_FORCE:
    {
      ret = 2;
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
  BOOL result = FALSE;

  ENTER();

  // make the following operations single threaded
  // MUI chokes if a single task application is created a second time while the first instance is not yet fully created
  ObtainSemaphore(&startupSemaphore->semaphore);

  if((G->App = YAMApplicationObject,
    MUIA_Application_SingleTask, G->SingleTask,
    MUIA_YAMApplication_Hidden, hidden,
  End) != NULL)
  {
    // create the splash window object and return true if
    // everything worked out fine.
    if((G->SplashWinObject = SplashWindowObject, End) != NULL)
    {
      G->InStartupPhase = TRUE;

      set(G->SplashWinObject, MUIA_Window_Open, !(hidden || args.noSplashWindow));

      result = TRUE;
    }
    else
      E(DBF_STARTUP, "couldn't create splash window object!");
  }
  else
  {
    E(DBF_STARTUP, "couldn't create root object!");

    // in case only one instance of YAM is allowed we try to send possible startup
    // actions (i.e. write a new mail) to the first instance
    if(G->SingleTask == TRUE)
    {
      if(args.mailto != NULL || args.letter != NULL || args.subject != NULL || args.attach != NULL)
      {
        char command[SIZE_LINE];

        // start composing a new mail
        SendToYAMInstance((char *)"MAILWRITE");

        // pass all given parameters to the first instance
        if(args.mailto != NULL)
        {
          snprintf(command, sizeof(command), "WRITETO \"%s\"", args.mailto);
          SendToYAMInstance(command);
        }

        if(args.subject != NULL)
        {
          snprintf(command, sizeof(command), "WRITESUBJECT \"%s\"", args.subject);
          SendToYAMInstance(command);
        }

        if(args.letter != NULL)
        {
          snprintf(command, sizeof(command), "WRITELETTER \"%s\"", args.letter);
          SendToYAMInstance(command);
        }

        if(args.attach != NULL)
        {
          char **sptr;

          for(sptr = args.attach; *sptr; sptr++)
          {
            LONG size;

            if(ObtainFileInfo(*sptr, FI_SIZE, &size) == TRUE && size > 0)
            {
              snprintf(command, sizeof(command), "WRITEATTACH \"%s\"", *sptr);
              SendToYAMInstance(command);
            }
          }
        }
      }
    }
  }

  // now a second instance may continue
  ReleaseSemaphore(&startupSemaphore->semaphore);

  RETURN(result);
  return result;
}

///

/// InitAfterLogin
//  Phase 2 of program initialization (after user logs in)
static void InitAfterLogin(void)
{
  struct FolderList *oldfolders = NULL;
  struct FolderNode *fnode;
  enum LoadTreeResult ltr;
  BOOL newfolders;
  BOOL splashWasActive;
  char pubScreenName[MAXPUBSCREENNAME + 1];
  struct Screen *pubScreen;
  int res;

  ENTER();

  // clear the configuration (set defaults) and load it
  // from the user defined .config file
  D(DBF_STARTUP, "loading configuration...");
  SplashProgress(tr(MSG_LoadingConfig), 20);

  res = LoadConfig(C, G->CO_PrefsFile, &oldfolders);
  if(res == 0)
    Abort(NULL); // user requested to abort because newer config file found
  else if(res == -1)
    SetDefaultConfig(C, cp_AllPages); // reset things to defaults if config file missing/invalid

  // load all necessary graphics/themes
  SplashProgress(tr(MSG_LoadingGFX), 30);

  // load the choosen theme of the user
  LoadTheme(&G->theme, C->ThemeName);

  // make sure we initialize the toolbar Cache which in turn will
  // cause YAM to cache all often used toolbars and their images
  if(ToolbarCacheInit() == FALSE)
    Abort(NULL); // exit the application

  // make sure the config has valid values and save any changes automatically
  ValidateConfig(C, FALSE, TRUE);

  // create all necessary GUI elements
  D(DBF_STARTUP, "creating GUI...");
  SplashProgress(tr(MSG_CreatingGUI), 40);

  // before we go and create the first MUI windows
  // we register the application to application.library
  InitDockyIcon();

  // Create a new Main Window
  if((G->MA = MA_New()) == NULL)
    Abort(tr(MSG_ErrorMuiApp));

  // make sure the GUI objects for the embedded read pane are created
  MA_SetupEmbeddedReadPane();

  // place the info and quicksearch bars at the configured positions
  DoMethod(G->MA->GUI.WI, MUIM_MainWindow_Relayout);

  // load the main window GUI layout from the ENV: variable
  LoadLayout();

  SplashProgress(tr(MSG_LoadingFolders), 50);

  newfolders = FALSE;
  ltr = FO_LoadTree();
  if(ltr == LTR_QuitYAM)
  {
    // do a hard termination
	Abort(NULL);
  }
  else if(ltr == LTR_Failure && oldfolders != NULL)
  {
    // add all YAM 1.x style folders
    ForEachFolderNode(oldfolders, fnode)
    {
      struct Folder *folder = fnode->folder;

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder->Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE);
    }

    newfolders = TRUE;
  }

  // free any YAM 1.x style folder
  if(oldfolders != NULL)
  {
    ForEachFolderNode(oldfolders, fnode)
    {
      free(fnode->folder);
    }

    DeleteFolderList(oldfolders);
  }

  if(FO_GetFolderByType(FT_INCOMING, NULL) == NULL)
  {
    if(FO_CreateFolder(FT_INCOMING, FolderName[FT_INCOMING], tr(MSG_MA_Incoming)) == TRUE)
    {
      // move the new incoming folder to the top
      struct Folder *this = FO_GetFolderByType(FT_INCOMING, NULL);

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, MUIV_NListtree_Move_NewTreeNode_Head, MUIF_NONE);
      newfolders = TRUE;
    }
  }

  if(FO_GetFolderByType(FT_DRAFTS, NULL) == NULL)
  {
    if(FO_CreateFolder(FT_DRAFTS, FolderName[FT_DRAFTS], tr(MSG_MA_DRAFTS)) == TRUE)
    {
      // move the new drafts folder after the incoming folder
      struct Folder *this = FO_GetFolderByType(FT_DRAFTS, NULL);
      struct Folder *prev = FO_GetFolderByType(FT_INCOMING, NULL);

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);
      newfolders = TRUE;
    }
  }

  if(FO_GetFolderByType(FT_OUTGOING, NULL) == NULL)
  {
    if(FO_CreateFolder(FT_OUTGOING, FolderName[FT_OUTGOING], tr(MSG_MA_Outgoing)) == TRUE)
    {
      // move the new outgoing folder after the drafts folder
      struct Folder *this = FO_GetFolderByType(FT_OUTGOING, NULL);
      struct Folder *prev = FO_GetFolderByType(FT_DRAFTS, NULL);

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);
      newfolders = TRUE;
    }
  }

  if(FO_GetFolderByType(FT_SENT, NULL) == NULL)
  {
    if(FO_CreateFolder(FT_SENT, FolderName[FT_SENT], tr(MSG_MA_Sent)) == TRUE)
    {
      // move the new sent folder after the outgoing folder
      struct Folder *this = FO_GetFolderByType(FT_SENT, NULL);
      struct Folder *prev = FO_GetFolderByType(FT_OUTGOING, NULL);

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);
      newfolders = TRUE;
    }
  }

  if(FO_GetFolderByType(FT_TRASH, NULL) == NULL)
  {
    if(FO_CreateFolder(FT_TRASH, FolderName[FT_TRASH], tr(MSG_MA_TRASH)) == TRUE)
    {
      // move the new trash folder after the sent folder
      struct Folder *this = FO_GetFolderByType(FT_TRASH, NULL);
      struct Folder *prev = FO_GetFolderByType(FT_SENT, NULL);

      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);
      newfolders = TRUE;
    }
  }

  if(C->SpamFilterEnabled == TRUE)
  {
    // check if the spam folder has to be created
    if(FO_GetFolderByType(FT_SPAM, NULL) == NULL)
    {
      char spamPath[SIZE_PATHFILE];
      BOOL createSpamFolder;
      enum FType type;

      if(ObtainFileInfo(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FI_TYPE, &type) == TRUE && type == FIT_NONEXIST)
      {
        // no directory named "spam" exists, so let's create it
        createSpamFolder = TRUE;
      }
      else
      {
        // the directory "spam" already exists, but it is not the standard spam folder
        // let the user decide what to do
        ULONG result;

        result = MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL,
                                                               tr(MSG_ER_SPAMDIR_EXISTS_ANSWERS),
                                                               tr(MSG_ER_SPAMDIR_EXISTS));
        switch(result)
        {
          default:
          case 0:
          {
            // the user has chosen to disable the spam filter, so we do it
            // or the requester was cancelled
            C->SpamFilterEnabled = FALSE;
            createSpamFolder = FALSE;
          }
          break;

          case 1:
          {
            // delete everything in the folder, the directory itself can be kept
            DeleteMailDir(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FALSE);
            createSpamFolder = TRUE;
          }
          break;

          case 2:
          {
            // keep the folder contents
            createSpamFolder = TRUE;
          }
          break;
        }
      }

      if(createSpamFolder == TRUE)
      {
        struct Folder *spamFolder;

        // try to remove the existing folder named "spam"
        if((spamFolder = FO_GetFolderByPath(FolderName[FT_SPAM], NULL)) != NULL)
        {
          if(spamFolder->Treenode != NULL)
          {
            // remove the folder from the folder list
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, spamFolder->Treenode, MUIF_NONE);
          }
          if(spamFolder->imageObject != NULL)
          {
            // we make sure that the NList also doesn't use the image in future anymore
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, spamFolder->ImageIndex, MUIF_NONE);
            spamFolder->imageObject = NULL;
          }
        }
        // finally, create the spam folder
        if(FO_CreateFolder(FT_SPAM, FolderName[FT_SPAM], tr(MSG_MA_SPAM)) == TRUE)
        {
          // move the new spam folder after the trash folder
          struct Folder *this = FO_GetFolderByType(FT_SPAM, NULL);
          struct Folder *prev = FO_GetFolderByType(FT_TRASH, NULL);

          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);
          newfolders = TRUE;
        }
      }
    }

    // now try to import external spam filters
    // this happened while loading the config already, but has failed
    // most probably because the folder tree was not loaded at that time
    ImportExternalSpamFilters(C);
  }

  if(newfolders == TRUE)
  {
    set(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible);
    FO_SaveTree();
  }

  // setup some dynamic (changing) menus
  MA_SetupDynamicMenus();

  // do some initial call to ChangeSelected() for correctly setting up
  // some mail information
  MA_ChangeSelected(TRUE);

  SplashProgress(tr(MSG_VALIDATING_FOLDERS), 55);
  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    // if this entry is a group lets skip here immediately
    if(isGroupFolder(folder))
      continue;

    if((isIncomingFolder(folder) || isOutgoingFolder(folder) || isTrashFolder(folder) ||
        C->LoadAllFolders == TRUE) && !isProtectedFolder(folder))
    {
      // call the getIndex function which on one hand loads the full .index file
      // and makes sure that all "new" mail is marked to unread if the user
      // enabled the C->UpdateNewMail option in the configuration.
      MA_GetIndex(folder);
    }
    else if(folder->LoadedMode != LM_VALID)
    {
      // do not load the full index, do load only the header of the .index
      // which summarizes everything
      folder->LoadedMode = MA_LoadIndex(folder, FALSE);

      // if the user wishs to make sure all "new" mail is flagged as
      // read upon start we go through our folders and make sure they show
      // no "new" mail, even if their .index file is not fully loaded
      if(C->UpdateNewMail == TRUE && folder->LoadedMode == LM_FLUSHED &&
         folder->New > 0)
      {
        // to perform this operation we simply call MA_GetIndex() as that
        // takes care via MA_ValidateStatus() that the status of new mail
        // is cleared upon starting YAM
        MA_GetIndex(folder);
      }
    }

    // update the folder's image
    FO_SetFolderImage(folder);

    // now we have to add the amount of mails of this folder to the foldergroup
    // aswell and also the grandparents.
    FO_UpdateTreeStatistics(folder, FALSE);

    DoMethod(G->App, MUIM_Application_InputBuffered);
  }

  SplashProgress(tr(MSG_RebuildIndices), 60);
  MA_RebuildIndexes();

  SplashProgress(tr(MSG_LOADINGUPDATESTATE), 70);
  LoadUpdateState();

  SplashProgress(tr(MSG_LOADINGSPAMTRAININGDATA), 80);
  BayesFilterInit();

  SplashProgress(tr(MSG_LoadingABook), 90);
  LoadABook(G->abookFilename, &G->abook, FALSE);

  if((G->RexxHost = SetupARexxHost("YAM", NULL)) == NULL)
    Abort(tr(MSG_ErrorARexx));

  SplashProgress(tr(MSG_OPENGUI), 100);
  G->InStartupPhase = FALSE;

  // Lock the screen that YAM has opened its splash window on to prevent this screen
  // from closing after the splash window is closed and before the main window is opened.
  // This is necessary if YAM is running on its own screen instead of the Workbench.
  GetPubScreenName((struct Screen *)xget(G->SplashWinObject, MUIA_Window_Screen), pubScreenName, sizeof(pubScreenName));
  pubScreen = LockPubScreen(pubScreenName);

  // close the splash window right before we open our main YAM window
  // but ask it before closing if it was activated or not.
  splashWasActive = (args.noSplashWindow) ? TRUE : xget(G->SplashWinObject, MUIA_Window_Activate);
  set(G->SplashWinObject, MUIA_Window_Open, FALSE);

  // cleanup the splash window object immediately
  DoMethod(G->App, OM_REMMEMBER, G->SplashWinObject);
  MUI_DisposeObject(G->SplashWinObject);
  G->SplashWinObject = NULL;

  // Only activate the main window if the about window is active and open it immediatly.
  // We always start YAM with Window_Open=TRUE or else the hide functionality does not work as expected.
  xset(G->MA->GUI.WI, MUIA_Window_Activate, splashWasActive,
                      MUIA_Window_Open,     TRUE);

  // Now we have to make sure that the current folder is really in "active" state
  // or we risk to get a unsynced message listview. We also have to do that AFTER
  // the final MUIA_Window_Open call or e.g. a MUIM_NList_Jump call will not work
  // correctly as NList doesn't know anything about its visible height yet.
  MA_ChangeFolder(NULL, TRUE);

  // unlock the public screen again now that the main window is open
  UnlockPubScreen(pubScreenName, pubScreen);

  LEAVE();
}

///
/// InitBeforeLogin
//  Phase 1 of program initialization (before user logs in)
static void InitBeforeLogin(BOOL hidden)
{
  char var;
  int i;
  const char *failSuperClass;
  const char *failClass;

  ENTER();

  // lets save the current date/time in our startDate value
  DateStamp(&G->StartDate);

  // initialize the random number seed.
  srand((unsigned int)GetDateStamp());

  // First open locale.library, so we can display a translated error requester
  // in case some of the other libraries can't be opened.
  if(INITLIB("locale.library", 38, 0, &LocaleBase, "main", 1, &ILocale, TRUE, NULL))
    G->Locale = OpenLocale(NULL);

  // Now load the catalog of YAM
  if(G->NoCatalogTranslation == FALSE && OpenYAMCatalog() == FALSE)
    Abort(NULL);

  // load&initialize all required libraries
  // first all system relevant libraries
  INITLIB("graphics.library",      36, 0, &GfxBase,       "main", 1, &IGraphics,  TRUE,  NULL);
  INITLIB("layers.library",        39, 0, &LayersBase,    "main", 1, &ILayers,    TRUE,  NULL);
  INITLIB("workbench.library",     36, 0, &WorkbenchBase, "main", 1, &IWorkbench, TRUE,  NULL);
  INITLIB("keymap.library",        36, 0, &KeymapBase,    "main", 1, &IKeymap,    TRUE,  NULL);
  INITLIB("iffparse.library",      36, 0, &IFFParseBase,  "main", 1, &IIFFParse,  TRUE,  NULL);
  INITLIB("rexxsyslib.library",    36, 0, &RexxSysBase,   "main", 1, &IRexxSys,   TRUE,  NULL);
  INITLIB("datatypes.library",     39, 0, &DataTypesBase, "main", 1, &IDataTypes, TRUE,  NULL);

  // try to open the cybergraphics.library on non-OS4 systems as on OS4 we use the
  // new graphics.library functions instead.
  #if !defined(__amigaos4__)
  INITLIB("cybergraphics.library", 40, 0, &CyberGfxBase,  "main", 1, &ICyberGfx,  FALSE, NULL);
  #endif

  // try to open MUI 3.8+
  INITLIB("muimaster.library",     19, 0, &MUIMasterBase, "main", 1, &IMUIMaster, TRUE, "http://www.sasg.com/");

  // openurl.library has a homepage, but providing that homepage without having OpenURL
  // installed would result in a paradoxon, because InitLib() would provide a button
  // to visit the URL which in turn requires OpenURL to be installed...
  // Hence we try to open openurl.library without
  INITLIB("openurl.library",        1, 0, &OpenURLBase,   "main", 1, &IOpenURL,   FALSE, NULL);

  // try to open the mandatory codesets.library
  INITLIB("codesets.library",       6, 15, &CodesetsBase,  "main", 1, &ICodesets,  TRUE, "http://sf.net/p/codesetslib/");

  // try to open expat.library for our XML import stuff
  INITLIB("expat.library", XML_MAJOR_VERSION, 0, &ExpatBase, "main", 1, &IExpat, FALSE, NULL);

  // we check for the amisslmaster.library v3 accordingly
  if(INITLIB("amisslmaster.library", AMISSLMASTER_MIN_VERSION, 5, &AmiSSLMasterBase, "main", 1, &IAmiSSLMaster, FALSE, NULL))
  {
    if(InitAmiSSLMaster(AMISSL_CURRENT_VERSION, TRUE))
    {
      if((AmiSSLBase = OpenAmiSSL()) != NULL &&
         GETINTERFACE("main", 1, IAmiSSL, AmiSSLBase))
      {
        G->TR_UseableTLS = TRUE;

        D(DBF_STARTUP, "successfully opened AmiSSL library.");
      }
    }
  }

  #if defined(__amigaos4__)
  // now we try to open the application.library which is part of OS4
  // and will be used to notify YAM of certain events and also manage
  // the docky icon accordingly.
  // try version 2 first, if that is not available try version 1
  if(INITLIB("application.library", 50, 0, &ApplicationBase, "application", 2, &IApplication, FALSE, NULL) == FALSE)
    INITLIB("application.library", 50, 0, &ApplicationBase, "application", 1, &IApplication, FALSE, NULL);
  #endif

  // initialize the shared connection semaphore
  if(InitConnections() == FALSE)
    Abort(tr(MSG_ERROR_CONNECTIONS));

  // initialize the method stack
  if(InitMethodStack() == FALSE)
    Abort(tr(MSG_ERROR_METHODSTACK));

  // initialize the thread system of YAM
  if(InitThreads() == FALSE)
    Abort(tr(MSG_ERROR_THREADS));

  // Check that the user has all the required third party mcc classes installed or
  // abort otherwise.
  //
  // Note: In addition to checking for a minimum required version we also check for
  //       certain classes (e.g. betterstring.mcc) that the version is within a certain
  //       range as on MorphOS there unfortunately exist fake classes which cause severe
  //       problems when used with YAM. It's unclear why the MOS developers do that instead
  //       of actively contributing to the classes they tend to call broken. In the end
  //       this causes the MUI version of MorphOS to be borked, thus this workaround!
  //
  //       customclass      minv minr maxv maxr  mand  url
  CheckMCC(MUIC_TheBar,       26,  12,   0,   0, TRUE, "http://sf.net/p/thebar/");
  CheckMCC(MUIC_TheBarVirt,   26,  12,   0,   0, TRUE, "http://sf.net/p/thebar/");
  CheckMCC(MUIC_TheButton,    26,  12,   0,   0, TRUE, "http://sf.net/p/thebar/");
  CheckMCC(MUIC_BetterString, 11,  25,  30,   0, TRUE, "http://sf.net/p/bstring-mcc/");
  CheckMCC(MUIC_NList,        20, 135,   0,   0, TRUE, "http://sf.net/p/nlist-classes/");
  CheckMCC(MUIC_NListview,    19,  90,   0,   0, TRUE, "http://sf.net/p/nlist-classes/");
  CheckMCC(MUIC_NFloattext,   19,  71,   0,   0, TRUE, "http://sf.net/p/nlist-classes/");
  CheckMCC(MUIC_NListtree,    18,  42,   0,   0, TRUE, "http://sf.net/p/nlist-classes/");
  CheckMCC(MUIC_NBalance,     15,  16,   0,   0, TRUE, "http://sf.net/p/nlist-classes/");
  CheckMCC(MUIC_TextEditor,   15,  41,   0,   0, TRUE, "http://sf.net/p/texteditor-mcc/");

  // now we search through PROGDIR:Charsets and load all user defined
  // codesets via codesets.library
  G->codesetsList = CodesetsListCreateA(NULL);

  // create a public semaphore which can be used to single thread certain actions
  if((startupSemaphore = CreateStartupSemaphore()) == NULL)
    Abort(tr(MSG_ER_CANNOT_CREATE_SEMAPHORE));

  // try to find out if DefIcons is running or not by querying
  // the Port of DefIcons. Alternatively the Ambient desktop
  // should provide the same functionallity.
  Forbid();
  G->DefIconsAvailable = (FindPort((APTR)"DEFICONS") != NULL || FindPort((APTR)"AMBIENT") != NULL);
  Permit();

  // initialize our own MUI custom classes before we go on
  D(DBF_STARTUP, "setup internal MUI classes...");
  if(YAM_SetupClasses(&failClass, &failSuperClass) == FALSE)
    Abort(tr(MSG_CANNOT_CREATE_CUSTOM_CLASS), failClass, failSuperClass);

  G->SingleTask = TRUE;
  // let us check if there is a "MultipleYAM" env variable and if
  // so we set SingleTask to true
  if(GetVar("MultipleYAM", &var, sizeof(var), 0) > -1)
    G->SingleTask = FALSE;

  // allocate the MUI root object and popup the progress/about window
  D(DBF_STARTUP, "creating root object...");
  if(Root_New(hidden) == FALSE)
  {
    BOOL activeYAM;

    Forbid();
    activeYAM = (FindPort((APTR)"YAM") != NULL);
    Permit();

    Abort(activeYAM ? NULL : tr(MSG_ErrorMuiApp));
  }

  // signal that we are loading our libraries
  D(DBF_STARTUP, "init libraries...");
  SplashProgress(tr(MSG_InitLibs), 10);

  // try to open the xadmaster.library v12.1+ as this is the somewhat
  // most recent version publically available
  INITLIB(XADNAME, 12, 1, &xadMasterBase, "main", 1, &IxadMaster, FALSE, NULL);

  // try to open xpkmaster.library v5.0+ as this is somewhat the most
  // stable version available. Previous version might have some issues
  // as documented in our FAQ.
  INITLIB(XPKNAME, 5, 0, &XpkBase, "main", 1, &IXpk, FALSE, NULL);
  InitXPKPackerList();

  // initialize our timers
  if(InitTimers() == FALSE)
    Abort(tr(MSG_ErrorTimer));

  // initialize our ASL FileRequester cache stuff
  for(i = 0; i < ASL_MAX; i++)
  {
    if((G->FileReqCache[i] = calloc(sizeof(struct FileReqCache), 1)) == NULL)
      Abort(NULL);
  }

  // initialize the AppIcon related stuff
  if(InitAppIcon() == FALSE)
    Abort(NULL);

  // initialize the write window file nofifications
  if((G->writeWinNotifyPort = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) == NULL)
    Abort(NULL);

  // initialize our tzone handling
  if(ParseZoneTabFile() == FALSE)
    Abort(tr(MSG_TZONE_ZONETAB_FILE_MISSING), G->ProgDir, G->ProgDir);

  LEAVE();
}

///
/// SendWaitingMail
//  Sends pending mail on startup
static BOOL SendWaitingMail(const BOOL hideDisplay)
{
  struct Folder *fo;
  BOOL sendableMail = FALSE;

  ENTER();

  if((fo = FO_GetFolderByType(FT_OUTGOING, NULL)) != NULL)
  {
    struct MailNode *mnode;

    LockMailListShared(fo->messages);

    ForEachMailNode(fo->messages, mnode)
    {
      if(!hasStatusError(mnode->mail))
      {
        sendableMail = TRUE;
        break;
      }
    }

    UnlockMailList(fo->messages);

    // in case the folder contains
    // mail which could be sent, we ask the
    // user what to do with it
    if(sendableMail == TRUE && hideDisplay == FALSE && xget(G->App, MUIA_Application_Iconified) == FALSE)
    {
      // change the folder first so that the user
      // might have a look at the mails
      MA_ChangeFolder(fo, TRUE);

      // now ask the user for permission to send the mail.
      sendableMail = MUI_Request(G->App, G->MA->GUI.WI, MUIF_NONE, NULL, tr(MSG_YesNoReq), tr(MSG_SendStartReq));
    }
  }

  if(sendableMail == TRUE)
    sendableMail = MA_Send(SENDMAIL_ALL_USER, SENDF_SIGNAL);

  RETURN(sendableMail);
  return(sendableMail);
}

///
/// DoStartup
//  Performs different checks/cleanup operations on startup
static void DoStartup(BOOL nocheck, BOOL hide)
{
  static char lastUserName[SIZE_NAME] = "";
  char *currentUserName = NULL;
  struct User *currentUser;

  ENTER();

  // display the AppIcon now because if non of the functions below
  // does it it could happen that no AppIcon will be displayed at all.
  UpdateAppIcon();

  // execute the startup stuff only if the user changed upon a restart or if
  // we start for the first time
  if((currentUser = US_GetCurrentUser()) != NULL)
    currentUserName = currentUser->Name;

  // we must compare the names here instead of the IDs, because the IDs will
  // change upon every restart
  if(currentUserName != NULL)
  {
    if(strcmp(lastUserName, currentUserName) != 0)
    {
      D(DBF_STARTUP, "user '%s' differs from '%s' of the last start, performing startup actions", currentUserName, lastUserName);

      // if the user wishs to delete all old mail during startup of YAM,
      // we do it now
      if(C->CleanupOnStartup == TRUE)
        DoMethod(G->App, MUIM_YAMApplication_DeleteOldMails);

      // if the user wants to clean the trash upon starting YAM, do it
      if(C->RemoveOnStartup == TRUE)
        DoMethod(G->App, MUIM_YAMApplication_EmptyTrashFolder, FALSE);

      // Check for current birth days in our addressbook if the user
      // configured it. This will also setup the timer for the repeated
      // birthday check on the next day.
      CheckABookBirthdays(&G->abook, C->CheckBirthdates == TRUE && nocheck == FALSE && hide == FALSE);

      // the rest of the startup jobs require a running TCP/IP stack,
      // so check if it is properly running.
      if(nocheck == FALSE && ConnectionIsOnline(NULL) == TRUE)
      {
        // perform the configured startup actions for receiving and sending mails
        MA_PopNow(NULL, RECEIVEF_STARTUP|RECEIVEF_USER, NULL);

        if(C->SendOnStartup == TRUE)
          SendWaitingMail(hide);
      }
    }
    else
      D(DBF_STARTUP, "user '%s' is the same as upon last start, no startup actions done", currentUserName);

    // remember the current user name for a possible restart
    strlcpy(lastUserName, currentUserName, sizeof(lastUserName));
  }
  else
  {
    E(DBF_STARTUP, "current user has no name?!?");
    // erase the last user name to be on the save side
    lastUserName[0] = '\0';
  }

  LEAVE();
}

///
/// Login
//  Log in a given user or prompt for user and password
static void Login(const char *user, const char *password,
                  const char *maildir, const char *prefsfile)
{
  ENTER();

  if(US_Login(user, password, maildir, prefsfile) == FALSE)
  {
    E(DBF_STARTUP, "terminating due to incorrect login information");
    exit(RETURN_WARN);
  }

  LEAVE();
}

///

/*** Command-Line Argument parsing routines ***/
/// ParseCommandArgs
//
static LONG ParseCommandArgs(void)
{
  LONG result = 0;
  char *extHelp;

  ENTER();

  // clear the args structure
  memset(&args, 0, sizeof(args));
  memset(&nrda, 0, sizeof(nrda));

  // set argument template
  nrda.Template = (STRPTR)"USER/K,"
                          "PASSWORD/K,"
                          "MAILDIR/K,"
                          "PREFSFILE/K,"
                          "NOCHECK/S,"
                          "HIDE/S,"
                          "DEBUG/S,"
                          "MAILTO/K,"
                          "SUBJECT/K,"
                          "LETTER/K,"
                          "ATTACH/M,"
                          "NOIMGWARNING/S,"
                          "NOCATALOG/S,"
                          "NOSPLASHWINDOW/S";

  // now we build an extended help page text
  if((asprintf(&extHelp, "%s (%s)\n%s\n\nUsage: YAM <options>\nOptions/Tooltypes:\n"
                         "  USER=<username>     : Selects the active YAM user and skips\n"
                         "                        the login process.\n"
                         "  PASSWORD=<password> : Password of selected user (if required).\n"
                         "  MAILDIR=<path>      : Sets the home directory for the folders\n"
                         "                        and configuration.\n"
                         "  PREFSFILE=<filename>: Configuration file that should be used\n"
                         "                        instead of the default.\n"
                         "  NOCHECK             : Starts YAM without trying to receive/send\n"
                         "                        any mail.\n"
                         "  HIDE                : Starts YAM in iconify mode.\n"
                         "  DEBUG               : Sends all conversations between YAM and a\n"
                         "                        mail server to the console window.\n"
                         "  MAILTO=<recipient>  : Creates a new mail for the specified\n"
                         "                        recipients when YAM started.\n"
                         "  SUBJECT=<subject>   : Sets the subject text for a new mail.\n"
                         "  LETTER=<file>       : The text file containing the actual mail\n"
                         "                        text of a new message.\n"
                         "  ATTACH=<file>       : Attaches the specified file to the new\n"
                         "                        mail created.\n"
                         "  NOIMGWARNING        : Supresses all warnings regarding missing\n"
                         "                        image files.\n"
                         "  NOCATALOG           : Starts YAM without loading any catalog\n"
                         "                        translation (english).\n"
                         "  NOSPLASHWINDOW      : Starts YAM without opening the splash\n"
                         "                        and shutdown windows.\n"
                         "%s", yamversion,
                               yamversiondate,
                               yamcopyright,
                               nrda.Template)) != -1)
  {
    // set the extHelp pointer
    nrda.ExtHelp = (STRPTR)extHelp;

    // set rest of new read args structure elements
    nrda.Window = NULL;
    nrda.Parameters = (APTR)&args;
    nrda.FileParameter = -1;
    nrda.PrgToolTypesOnly = FALSE;

    // now call NewReadArgs to parse all our commandline/tooltype arguments in accordance
    // to the above template
    result = NewReadArgs(WBmsg, &nrda);

    free(extHelp);
    nrda.ExtHelp = NULL;
  }
  else
    E(DBF_STARTUP, "asprintf() returned -1");

  RETURN(result);
  return result;
}

///
/// LowMemHandler
// low memory handler function to flush all folder indexes
#if defined(__amigaos4__)
static LONG LowMemHandler(struct ExecBase *ExecBase, UNUSED struct MemHandlerData *memHandlerData, UNUSED APTR data)
#else
static LONG ASM LowMemHandler(REG(a6, UNUSED struct ExecBase *ExecBase), REG(a0, UNUSED struct MemHandlerData *memHandlerData), REG(a1, UNUSED APTR data))
#endif
{
  #if defined(__amigaos4__)
  struct ExecIFace *IExec = (struct ExecIFace *)ExecBase->MainInterface;
  #endif

  // restart the index flush timer
  RestartTimer(TIMER_WRINDEX, 0, 1, FALSE);

  // that was all we could do
  return MEM_ALL_DONE;
}

///

/*** main entry function ***/
/// main
//  Program entry point, main loop
int main(int argc, char **argv)
{
  BOOL yamFirst;
  BPTR progdir;
  LONG err;

  // obtain the MainInterface of Exec before anything else.
  #if defined(__amigaos4__)
  IExec = (struct ExecIFace *)((struct ExecBase *)SysBase)->MainInterface;

  // check the exec version first and force be at least an 52.2 version
  // from AmigaOS4 final. This should assure we are are using the very
  // latest stable version.
  if(LIB_VERSION_IS_AT_LEAST(SysBase, 52, 2) == FALSE)
  {
    if((IntuitionBase = (APTR)OpenLibrary("intuition.library", 36)) != NULL &&
       GETINTERFACE("main", 1, IIntuition, IntuitionBase))
    {
      struct EasyStruct ErrReq;

      ErrReq.es_StructSize = sizeof(struct EasyStruct);
      ErrReq.es_Flags      = 0;
      ErrReq.es_Title        = (STRPTR)"YAM Startup Error";
      ErrReq.es_TextFormat   = (STRPTR)"This version of YAM requires at least\n"
                                       "an AmigaOS4 kernel version 52.2";
      ErrReq.es_GadgetFormat = (STRPTR)"Exit";

      EasyRequestArgs(NULL, &ErrReq, NULL, NULL);

      CLOSELIB(IntuitionBase, IIntuition);
    }

    exit(RETURN_WARN);
  }
  #endif // __amigaos4__

  // we make sure that if this is a build for 68k processors and for 68020+
  // that this is really a 68020+ machine
  #if _M68060 || _M68040 || _M68030 || _M68020 || __mc68020 || __mc68030 || __mc68040 || __mc68060
  if(isFlagClear(SysBase->AttnFlags, AFF_68020))
  {
    if((IntuitionBase = (APTR)OpenLibrary("intuition.library", 36)) != NULL)
    {
      struct EasyStruct ErrReq;

      ErrReq.es_StructSize = sizeof(struct EasyStruct);
      ErrReq.es_Flags      = 0;
      ErrReq.es_Title        = (STRPTR)"YAM Startup Error";
      ErrReq.es_TextFormat   = (STRPTR)"This version of YAM requires at\n"
                                       "least an 68020 processor or higher.";
      ErrReq.es_GadgetFormat = (STRPTR)"Exit";

      EasyRequestArgs(NULL, &ErrReq, NULL, NULL);

      CloseLibrary((struct Library *)IntuitionBase);
   }

   exit(RETURN_WARN);
  }
  #endif // _M680x0

  // initialize our debugging system.
  #if defined(DEBUG)
  SetupDebug();
  #endif // DEBUG

  #if defined(DEVWARNING)
  {
    BOOL goon = TRUE;

    if((IntuitionBase = (APTR)OpenLibrary("intuition.library", 36)) != NULL &&
       GETINTERFACE("main", 1, IIntuition, IntuitionBase))
    {
      if((UtilityBase = (APTR)OpenLibrary("utility.library", 36)) != NULL &&
         GETINTERFACE("main", 1, IUtility, UtilityBase))
      {
        char var;
        struct EasyStruct ErrReq;
        struct DateStamp ds;

        // try to open openurl.library to make GotoURL() work at this early stage
        if((OpenURLBase = (APTR)OpenLibrary("openurl.library", 1)) != NULL)
        {
          #if defined(__amigaos4__)
          GETINTERFACE("main", 1, IOpenURL, OpenURLBase);
          #endif
        }

        DateStamp(&ds); // get actual time/date

        ErrReq.es_StructSize = sizeof(struct EasyStruct);
        ErrReq.es_Flags      = 0;

        #if defined(EXPDATE)
        if(EXPDATE <= ds.ds_Days)
        {
          ErrReq.es_Title        = (STRPTR)"YAM Developer Version Expired!";
          ErrReq.es_TextFormat   = (STRPTR)"This developer version of YAM has expired!\n\n"
                                   "Please note that you may download a new, updated\n"
                                   "version from the YAM nightly build page at:\n\n"
                                   "http://nightly.yam.ch/\n\n"
                                   "All developer versions will automatically expire\n"
                                   "after a certian time interval. This is to insure\n"
                                   "that no old versions are floating around causing\n"
                                   "users to report bugs on old versions.\n\n"
                                   "Thanks for your help in improving YAM!";

          if(GotoURLPossible() == TRUE)
            ErrReq.es_GadgetFormat = (STRPTR)"Visit homepage|Exit";
          else
            ErrReq.es_GadgetFormat = (STRPTR)"Exit";

          DisplayBeep(NULL);
          if(EasyRequestArgs(NULL, &ErrReq, NULL, NULL) == 1)
          {
            // visit YAM's nightly build page and exit
            GotoURL("http://nightly.yam.ch/", FALSE);
          }

          goon = FALSE;
        }
        #endif // EXPDATE

        if(goon == TRUE && GetVar("I_KNOW_YAM_IS_UNDER_DEVELOPMENT", &var, sizeof(var), 0) == -1)
        {
          LONG answer;

          ErrReq.es_Title        = (STRPTR)"YAM Developer Snapshot Warning!";
          ErrReq.es_TextFormat   = (STRPTR)"This is just an *internal* developer snapshot\n"
                                           "version of YAM. It is not recommended or intended\n"
                                           "for general use as it may contain bugs that can\n"
                                           "lead to any loss of data. No regular support\n"
                                           "for this version is provided.\n\n"
                                           #if defined(EXPDATE)
                                           "In addition, this version will automatically\n"
                                           "expire after a certain time interval.\n\n"
                                           #endif // EXPDATE
                                           "So, if you're unsure and prefer to have a stable\n"
                                           "installation instead of a potentially dangerous\n"
                                           "version, please consider to use the current\n"
                                           "stable release version available from:\n\n"
                                           "http://yam.ch/\n\n"
                                           "Thanks for your help in improving YAM!";

          if(GotoURLPossible() == TRUE)
            ErrReq.es_GadgetFormat = (STRPTR)"Go on|Visit homepage|Exit";
          else
            ErrReq.es_GadgetFormat = (STRPTR)"Go on|Exit";

          DisplayBeep(NULL);
          answer = EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
          if(answer == 0)
          {
            // exit YAM
            goon = FALSE;
          }
          else if(answer == 2)
          {
            // visit YAM's home page and continue normally
            GotoURL(yamurl, FALSE);
          }
        }

        CLOSELIB(OpenURLBase, IOpenURL);
      }

      CLOSELIB(UtilityBase, IUtility);
    }

    CLOSELIB(IntuitionBase, IIntuition);
    if(goon == FALSE)
      exit(RETURN_WARN);
  }
  #endif // DEVWARNING

  // signal that on a exit() the 'yam_exitfunc' function
  // should be called.
  atexit(yam_exitfunc);

  WBmsg = (struct WBStartup *)(0 == argc ? argv : NULL);

  INITLIB("intuition.library", 36, 0, &IntuitionBase, "main", 1, &IIntuition, TRUE, NULL);
  INITLIB("icon.library",      36, 0, &IconBase,      "main", 1, &IIcon,      TRUE, NULL);
  INITLIB("utility.library",   36, 0, &UtilityBase,   "main", 1, &IUtility,   TRUE, NULL);
  INITLIB("diskfont.library",  37, 0, &DiskfontBase,  "main", 1, &IDiskfont,  TRUE, NULL);

  // now we parse the command-line arguments
  if((err = ParseCommandArgs()) != 0)
  {
    PrintFault(err, "YAM");

    SetIoErr(err);
    exit(RETURN_ERROR);
  }

  // security only, can happen for residents only
  if((progdir = GetProgramDir()) == ZERO)
    exit(RETURN_ERROR);

  olddirlock = CurrentDir(progdir);

  for(yamFirst=TRUE;;)
  {
    ULONG signals;
    ULONG timerSig;
    ULONG rexxSig;
    ULONG appSig;
    ULONG applibSig;
    ULONG threadSig;
    ULONG wakeupSig;
    ULONG writeWinNotifySig;
    ULONG methodStackSig;
    struct User *user;
    int ret;

    // allocate our global G and C structures
    if((G = calloc(1, sizeof(*G))) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    // prepare the exec lists in G
    NewMinList(&G->readMailDataList);
    NewMinList(&G->writeMailDataList);
    NewMinList(&G->zombieFileList);
    NewMinList(&G->normalBusyList);
    NewMinList(&G->arexxBusyList);
    NewMinList(&G->tzoneContinentList);
    InitABook(&G->abook);

    if((C = AllocConfig()) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    // create the MEMF_SHARED memory pool we use for our
    // own AllocVecPooled() allocations later on
    if((G->SharedMemPool = AllocSysObjectTags(ASOT_MEMPOOL,
      ASOPOOL_MFlags,    MEMF_SHARED|MEMF_CLEAR,
      ASOPOOL_Puddle,    2048,
      ASOPOOL_Threshold, 1024,
      ASOPOOL_Name,      (ULONG)"YAM shared pool",
      ASOPOOL_LockMem,   FALSE,
      TAG_DONE)) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    // create a list for all the folders
    if((G->folders = CreateFolderList()) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    if((G->globalSemaphore = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    if((G->configSemaphore = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    // allocate two virtual mail parts for the attachment requester
    // these two must be accessible all the time
    if((G->virtualMailpart[0] = calloc(1, sizeof(*G->virtualMailpart[0]))) == NULL)
      break;
    if((G->virtualMailpart[1] = calloc(1, sizeof(*G->virtualMailpart[1]))) == NULL)
      break;

    // setup the item pools for mails and mail nodes
    if((G->mailItemPool = AllocSysObjectTags(ASOT_ITEMPOOL,
      ASOITEM_MFlags, MEMF_SHARED|MEMF_CLEAR,
      ASOITEM_ItemSize, sizeof(struct Mail),
      ASOITEM_BatchSize, 1000,
      ASOITEM_GCPolicy, ITEMGC_AFTERCOUNT,
      TAG_DONE)) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }
    if((G->mailNodeItemPool = AllocSysObjectTags(ASOT_ITEMPOOL,
      ASOITEM_MFlags, MEMF_SHARED|MEMF_CLEAR,
      ASOITEM_ItemSize, sizeof(struct MailNode),
      ASOITEM_BatchSize, 1000,
      ASOITEM_GCPolicy, ITEMGC_AFTERCOUNT,
      TAG_DONE)) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    if((G->mailsInTransfer = CreateMailList()) == NULL)
    {
      // break out immediately to signal an error!
      break;
    }

    // install the low memory handler
    if((G->lowMemHandler = AllocSysObjectTags(ASOT_INTERRUPT,
      ASOINTR_Code, LowMemHandler,
      TAG_DONE)) != NULL)
    {
      G->lowMemHandler->is_Node.ln_Pri = 50;
      G->lowMemHandler->is_Node.ln_Name = (STRPTR)"YAM index lowmem";
      AddMemHandler(G->lowMemHandler);
    }
    else
    {
      // break out immediately to signal an error!
      break;
    }

    // get the PROGDIR: and program name and put it into own variables
    NameFromLock(progdir, G->ProgDir, sizeof(G->ProgDir));
    if(WBmsg != NULL && WBmsg->sm_NumArgs > 0)
    {
      strlcpy(G->ProgName, (char *)WBmsg->sm_ArgList[0].wa_Name, sizeof(G->ProgName));
    }
    else
    {
      char buf[SIZE_PATHFILE];

      GetProgramName((STRPTR)&buf[0], sizeof(buf));
      strlcpy(G->ProgName, (char *)FilePart(buf), sizeof(G->ProgName));
    }

    D(DBF_STARTUP, "ProgDir.: '%s'", G->ProgDir);
    D(DBF_STARTUP, "ProgName: '%s'", G->ProgName);

    // set up a path to the themes directory
    strlcpy(G->ThemesDir, "PROGDIR:Resources/Themes", sizeof(G->ThemesDir));
    if(FileExists(G->ThemesDir) == FALSE)
      strlcpy(G->ThemesDir, "PROGDIR:Themes", sizeof(G->ThemesDir));

    if(args.maildir == NULL)
      strlcpy(G->MA_MailDir, G->ProgDir, sizeof(G->MA_MailDir));

    G->TR_Debug = args.debug ? TRUE : FALSE;
    G->NoImageWarning = args.noImgWarning ? TRUE : FALSE;
    G->NoCatalogTranslation = args.noCatalog ? TRUE : FALSE;

    // setup our ImageCache
    ImageCacheSetup();

    if(yamFirst == TRUE)
    {
      InitBeforeLogin(args.hide ? TRUE : FALSE);
      Login(args.user, args.password, args.maildir, args.prefsfile);
      InitAfterLogin();
    }
    else
    {
      InitBeforeLogin(FALSE);
      Login(NULL, NULL, args.maildir, NULL);
      InitAfterLogin();
    }

    DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENVARC);
    AppendToLogfile(LF_ALL, 0, tr(MSG_LOG_Started));
    DoMethod(G->App, MUIM_YAMApplication_StartMacro, MACRO_STARTUP, NULL);

    if(yamFirst == TRUE)
    {
      struct WriteMailData *wmData;

      DoStartup(args.nocheck ? TRUE : FALSE, args.hide ? TRUE : FALSE);

      if((args.mailto != NULL || args.letter != NULL || args.subject != NULL || args.attach != NULL) &&
         (wmData = NewWriteMailWindow(NULL, 0)) != NULL)
      {
        if(args.mailto != NULL)
          set(wmData->window, MUIA_WriteWindow_To, args.mailto);

        if(args.subject != NULL)
          set(wmData->window, MUIA_WriteWindow_Subject, args.subject);

        if(args.letter != NULL)
          DoMethod(wmData->window, MUIM_WriteWindow_LoadText, args.letter, FALSE);

        if(args.attach != NULL)
        {
          char **sptr;

          for(sptr = args.attach; *sptr; sptr++)
          {
            LONG size;

            if(ObtainFileInfo(*sptr, FI_SIZE, &size) == TRUE && size > 0)
              DoMethod(wmData->window, MUIM_WriteWindow_AddAttachment, *sptr, NULL, FALSE);
          }
        }
      }

      yamFirst = FALSE;
    }
    else
    {
      DoStartup(args.nocheck ? TRUE : FALSE, FALSE);
    }

    user = US_GetCurrentUser();
    AppendToLogfile(LF_NORMAL, 1, tr(MSG_LOG_LoggedIn), user->Name);
    AppendToLogfile(LF_VERBOSE, 2, tr(MSG_LOG_LoggedInVerbose), user->Name, G->CO_PrefsFile, G->MA_MailDir);

    // prepare all signal bits
    timerSig          = (1UL << G->timerData.port->mp_SigBit);
    rexxSig           = (1UL << G->RexxHost->port->mp_SigBit);
    appSig            = (1UL << G->AppPort->mp_SigBit);
    applibSig         = DockyIconSignal();
    writeWinNotifySig = (1UL << G->writeWinNotifyPort->mp_SigBit);
    threadSig         = (1UL << G->threadPort->mp_SigBit);
    wakeupSig         = (1UL << ThreadWakeupSignal());
    methodStackSig    = (1UL << G->methodStack->mp_SigBit);

    D(DBF_STARTUP, "YAM allocated signals:");
    D(DBF_STARTUP, " timerSig          = %08lx", timerSig);
    D(DBF_STARTUP, " rexxSig           = %08lx", rexxSig);
    D(DBF_STARTUP, " appSig            = %08lx", appSig);
    D(DBF_STARTUP, " applibSig         = %08lx", applibSig);
    D(DBF_STARTUP, " writeWinNotifySig = %08lx", writeWinNotifySig);
    D(DBF_STARTUP, " threadSig         = %08lx", threadSig);
    D(DBF_STARTUP, " wakeupSig         = %08lx", wakeupSig);
    D(DBF_STARTUP, " methodStackSig    = %08lx", methodStackSig);

    // start our maintanance Timer requests for
    // different purposes (writeindexes/autosave)
    PrepareTimer(TIMER_WRINDEX, C->WriteIndexes, 0, FALSE);
    PrepareTimer(TIMER_AUTOSAVE, C->AutoSave, 0, FALSE);
    PrepareTimer(TIMER_SPAMFLUSHTRAININGDATA, C->SpamFlushTrainingDataInterval, 0, FALSE);
    PrepareTimer(TIMER_PURGEIDLETHREADS, 60, 0, FALSE);
    StartTimer(TIMER_WRINDEX);
    StartTimer(TIMER_AUTOSAVE);
    StartTimer(TIMER_SPAMFLUSHTRAININGDATA);
    StartTimer(TIMER_PURGEIDLETHREADS);

    PreparePOP3Timers();
    StartPOP3Timers();

    // initialize the automatic UpdateCheck facility and schedule an
    // automatic update check during startup if necessary
    InitUpdateCheck(TRUE);

    // start the event loop
    signals = 0;
    while((ret = Root_GlobalDispatcher(DoMethod(G->App, MUIM_Application_NewInput, &signals))) == 0)
    {
      if(signals != 0)
      {
        signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_F | timerSig | rexxSig | appSig | applibSig | writeWinNotifySig | threadSig | wakeupSig | methodStackSig);

        if(isFlagSet(signals, SIGBREAKF_CTRL_C))
        {
          ret = 1;
          break;
        }

        if(isFlagSet(signals, SIGBREAKF_CTRL_D))
        {
          ret = 0;
          break;
        }

        if(isFlagSet(signals, SIGBREAKF_CTRL_F))
          PopUp();

        // handle pushed methods
        if(isFlagSet(signals, methodStackSig))
          CheckMethodStack();

        // check for a Timer event
        if(isFlagSet(signals, timerSig))
        {
          #if defined(DEBUG)
          char dateString[64];

          DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATETIME, TZC_NONE);
          D(DBF_TIMER, "timer signal received @ %s", dateString);
          #endif // DEBUG

          // call ProcessTimerEvent() to check all our
          // timers are process accordingly.
          ProcessTimerEvent();
        }

        // check for an Arexx signal
        if(isFlagSet(signals, rexxSig))
          ARexxDispatch(G->RexxHost);

        // check for a AppMessage signal
        if(isFlagSet(signals, appSig))
          HandleAppIcon();

        #if defined(__amigaos4__)
        if(isFlagSet(signals, applibSig))
        {
          // make sure to break out here in case
          // the Quit or ForceQuit succeeded.
          if(HandleDockyIcon() == TRUE)
          {
            ret = 1;
            break;
          }
        }
        #endif // __amigaos4__

        // handle thread messages
        if(isFlagSet(signals, threadSig))
          HandleThreads(TRUE);

        // there is nothing to be done for the wakeup signal
        // but it must be included in the Wait() statement above to be cleared
        // automatically as soon as it was triggered by a thread

        // check for a write window file notification signal
        if(isFlagSet(signals, writeWinNotifySig))
        {
          struct NotifyMessage *msg;

          while((msg = (struct NotifyMessage *)GetMsg(G->writeWinNotifyPort)) != NULL)
          {
            // the messages UserData field contains the WriteWindow object
            // which triggered the notification
            Object *writeWin;

            if((writeWin = (Object *)msg->nm_NReq->nr_UserData) != NULL)
            {
              DoMethod(writeWin, MUIM_WriteWindow_MailFileModified);
            }

            ReplyMsg((struct Message *)msg);
          }
        }
      }
    }

    if(C->SendOnQuit == TRUE && args.nocheck == FALSE && ConnectionIsOnline(NULL) == TRUE)
    {
      if(SendWaitingMail(FALSE) == TRUE)
      {
        MiniMainLoop();
      }
    }

    if(C->CleanupOnQuit == TRUE)
      DoMethod(G->App, MUIM_YAMApplication_DeleteOldMails);

    if(C->RemoveOnQuit == TRUE)
      DoMethod(G->App, MUIM_YAMApplication_EmptyTrashFolder, TRUE);

    AppendToLogfile(LF_ALL, 99, tr(MSG_LOG_Terminated));
    DoMethod(G->App, MUIM_YAMApplication_StartMacro, MACRO_QUIT, NULL);

    // if the user really wants to exit, do it now as Terminate() is broken !
    if(ret == 1)
    {
      // Create the shutdown window object, but only show it if the application is visible, too.
      // This window will be closed and disposed automatically as soon as the application itself
      // is disposed.
      if(xget(G->App, MUIA_Application_Iconified) == FALSE && args.noSplashWindow == FALSE)
        ShutdownWindowObject, End;

      SetIoErr(RETURN_OK);
      exit(RETURN_OK);
    }

    D(DBF_STARTUP, "Restart issued");

    // prepare for restart
    Terminate();
  }

  /* not reached */
  SetIoErr(RETURN_OK);
  return RETURN_OK;
}

///
/// MiniMainLoop
// a "stripped down to the bare minimum" version of the mainloop
// to be used in situations where we have to wait for specific events
void MiniMainLoop(void)
{
  ULONG signals;
  ULONG threadSig;
  ULONG wakeupSig;
  ULONG methodStackSig;

  ENTER();

  // prepare all signal bits
  threadSig      = (1UL << G->threadPort->mp_SigBit);
  wakeupSig      = (1UL << ThreadWakeupSignal());
  methodStackSig = (1UL << G->methodStack->mp_SigBit);

  D(DBF_STARTUP, "YAM allocated signals:");
  D(DBF_STARTUP, " threadSig         = %08lx", threadSig);
  D(DBF_STARTUP, " wakeupSig         = %08lx", wakeupSig);
  D(DBF_STARTUP, " methodStackSig    = %08lx", methodStackSig);

  // start the event loop
  signals = 0;
  while(DoMethod(G->App, MUIM_Application_NewInput, &signals) == 0)
  {
    if(signals != 0)
    {
      signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F | threadSig | wakeupSig | methodStackSig);

      if(isFlagSet(signals, SIGBREAKF_CTRL_C))
        break;

      // show ourselves if we receive a CTRL-F
      if(isFlagSet(signals, SIGBREAKF_CTRL_F))
        PopUp();

      // handle pushed methods
      if(isFlagSet(signals, methodStackSig))
        CheckMethodStack();

      // handle thread messages
      if(isFlagSet(signals, threadSig))
        HandleThreads(TRUE);

      if(isFlagSet(signals, wakeupSig))
      {
        // assume all SMTP threads to be finished first
        BOOL allFinished = TRUE;
        struct UserIdentityNode *uin;

        D(DBF_STARTUP, "got wakeup signal");

        IterateList(&C->userIdentityList, struct UserIdentityNode *, uin)
        {
          // check if the SMTP server is still in use because it is still sending mails
          if(uin->smtpServer != NULL)
          {
            int useCount;

            LockMailServer(uin->smtpServer);
            useCount = uin->smtpServer->useCount;
            UnlockMailServer(uin->smtpServer);

            if(useCount != 0)
            {
              // at least one server is still in use
              allFinished = FALSE;
              break;
            }
          }
        }

        // bail out if all SMTP threads have finished
        if(allFinished == TRUE)
        {
          break;
        }
      }
    }
  }

  LEAVE();
}

///
/// MicroMainLoop
// an even more "stripped down to the bare minimum" version of the mainloop
// to be used in situations where we have to poll for specific basic events
void MicroMainLoop(void)
{
  ULONG signals;
  ULONG threadSig;
  ULONG methodStackSig;

  // !! CAUTION !!
  // This function might be called upon early termination (i.e. due to missing
  // stuff) and hence certain structures might not be initialized yet, like the
  // method stack or the application object. Take care of this!

  ENTER();

  // prepare all signal bits
  threadSig = (1UL << G->threadPort->mp_SigBit);
  if(G->methodStack != NULL)
    methodStackSig = (1UL << G->methodStack->mp_SigBit);
  else
    methodStackSig = 0L;

  D(DBF_STARTUP, "YAM allocated signals:");
  D(DBF_STARTUP, " threadSig         = %08lx", threadSig);
  D(DBF_STARTUP, " methodStackSig    = %08lx", methodStackSig);

  // instead of Wait()ing for some signals we just poll for them
  SetSignal(0UL, threadSig|methodStackSig);

  // handle the possibly received signals
  if(G->methodStack != NULL)
    CheckMethodStack();
  HandleThreads(TRUE);

  // let the application handle some stuff
  if(G->App != NULL)
    DoMethod(G->App, MUIM_Application_NewInput, &signals);

  LEAVE();
}

///

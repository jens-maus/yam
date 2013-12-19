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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Application
 Description: Application subclass handles all "global" stuff.

***************************************************************************/

#include "YAMApplication_cl.h"

#include <string.h>

#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/muimaster.h>
#include <proto/rexxsyslib.h>
#if defined(__amigaos4__)
#include <proto/application.h>
#endif
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <workbench/icon.h>

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_global.h"
#include "YAM_mainFolder.h"

#include "mui/AboutWindow.h"
#include "mui/AddressBookListtree.h"
#include "mui/AddressBookWindow.h"
#include "mui/AddressMatchPopupWindow.h"
#include "mui/ConfigWindow.h"
#include "mui/InfoWindow.h"
#include "mui/SearchMailWindow.h"
#include "mui/StringRequestWindow.h"
#include "mui/TransferControlGroup.h"
#include "mui/TransferWindow.h"

#include "AddressBook.h"
#include "AppIcon.h"
#include "Busy.h"
#include "Config.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MUIObjects.h"
#include "UpdateCheck.h"
#include "Requesters.h"
#include "Rexx.h"
#include "Threads.h"

#include "Debug.h"

#define EMAILCACHENAME "PROGDIR:.emailcache"

/* CLASSDATA
struct Data
{
  Object *aboutWindow;
  Object *transferWindow;
  struct ABook emailCache;
  char *emailCacheName;
  char compileInfo[SIZE_DEFAULT];
  BOOL iconified;
};
*/

/* INCLUDE
#include "YAM_main.h"
#include "AddressBook.h"
#include "mui/AddressBookWindow.h"
#include "mui/PreselectionWindow.h"
*/

/* Private functions */
/// LoadEMailCache
static void LoadEMailCache(const char *name, struct ABook *cache)
{
  FILE *fh;

  ENTER();

  InitABook(cache, "cache");

  if((fh = fopen(name, "r")) != NULL)
  {
    int i = 0;
    char *line = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    // we limit the reading to a maximum of 100 so that this code can't read endlessly
    while(GetLine(&line, &size, fh) >= 0 && i++ < 100)
    {
      char *addr;
      char *end;

      if((addr = strchr(line, '<')) != NULL && (end = strchr(addr, '>')) != NULL)
      {
        struct ABookNode *abn;

        if((abn = CreateABookNode(ABNT_USER)) != NULL)
        {
          if(addr != line)
          {
            addr[-1] = '\0';
            // copy the real name
            strlcpy(abn->RealName, line, sizeof(abn->RealName));
          }
          end[0] = '\0';
          strlcpy(abn->Address, addr+1, sizeof(abn->Address));

          AddABookNode(&cache->rootGroup, abn, NULL);
        }
      }
      else
      {
        D(DBF_STARTUP, "Error with '%s', parsing line: '%s'", name, line);
      }
    }

    fclose(fh);
    free(line);
  }
  else
  {
    E(DBF_STARTUP, "Error opening file '%s' for reading", name);
  }

  LEAVE();
}

///

struct SaveEMailCacheStuff
{
  FILE *fh;
  LONG savedEntries;
};

/// SaveEMailCacheEntry
static BOOL SaveEMailCacheEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  BOOL result;
  struct SaveEMailCacheStuff *stuff = (struct SaveEMailCacheStuff *)userData;

  ENTER();

  if(stuff->savedEntries < C->EmailCache)
  {
    // the cache contains user entries only, thus we don't have to care about the type here
    if(abn->RealName[0] != '\0')
      fprintf(stuff->fh, "%s <%s>\n", abn->RealName, abn->Address);
    else
      fprintf(stuff->fh, "<%s>\n", abn->Address);

    // count the number of saved entries
    stuff->savedEntries++;

    // continue to save entries
    result = TRUE;
  }
  else
  {
    // too many entries, abort
    result = FALSE;
  }

  RETURN(result);
  return result;
}

///
/// SaveEMailCache
static void SaveEMailCache(const char *name, struct ABook *cache)
{
  FILE *fh;

  ENTER();

  if((fh = fopen(name, "w")) != NULL)
  {
    struct SaveEMailCacheStuff stuff;

    stuff.fh = fh;
    stuff.savedEntries = 0;
    IterateABook(cache, 0, SaveEMailCacheEntry, &stuff);

    fclose(fh);
  }
  else
  {
    E(DBF_STARTUP, "Error opening file '%s' for writing", name);
  }

  LEAVE();
}

///
/// MatchRealName
// check whether a given string matches any part of a real name
static BOOL MatchRealName(const char *realName, const char *text, LONG textLen, LONG *matchPart)
{
  BOOL match = FALSE;
  char *name;

  ENTER();

  // check if we have a realname at all
  if(realName[0] != '\0')
  {
    LONG part = 0;

    // first try to match the whole realname string
    // completly
    if(Strnicmp(realName, text, textLen) == 0)
      match = TRUE;
    else if((name = strdup(realName)) != NULL)
    {
      char *n = name;
      char *p;

      // if this didn't work out we see if we can seperate the realname
      // into a first/lastname and see if only parts of it matches
      do
      {
        // break up the name in single parts delimited by spaces, quotes and commas
        if((p = strpbrk(n, " \",'")) != NULL)
          *p++ = '\0';

        if(n[0] != '\0')
        {
          // now check if this part of the name matches
          if(Strnicmp(n, text, textLen) == 0)
          {
            // yes!!
            match = TRUE;
            break;
          }

          part++;
        }

        // advance to the next name part
        n = p;
      }
      while(p != NULL);

      free(name);
    }

    // remember which part of the name this is if there is any interest in it
    if(matchPart != NULL)
      *matchPart = part;
  }

  RETURN(match);
  return match;
}

///

struct FindAllABMatchesStuff
{
  const char *text;
  size_t textlen;
  Object *list;
};

/// FindAllABMatchesEntry
static BOOL FindAllABMatchesEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct FindAllABMatchesStuff *stuff = (struct FindAllABMatchesStuff *)userData;

  ENTER();

  if(abn->type == ABNT_USER || abn->type == ABNT_LIST)
  {
    struct MatchedABookEntry e = { -1, -1, NULL, NULL };

    if(Strnicmp(abn->Alias, stuff->text, stuff->textlen) == 0)
    {
      e.MatchField = 0;
      e.MatchString = (char *)abn->Alias;
    }
    else if(MatchRealName(abn->RealName, stuff->text, stuff->textlen, &e.RealNameMatchPart) == TRUE)
    {
      e.MatchField = 1;
      e.MatchString = (char *)abn->RealName;
    }
    // don't match addresses in recipient lists
    // for lists the address field represents the reply address and this should never match
    else if(abn->type == ABNT_USER && Strnicmp(abn->Address, stuff->text, stuff->textlen) == 0)
    {
      e.MatchField = 2;
      e.MatchString = (char *)abn->Address;
    }

    if(e.MatchField != -1) /* one of the fields matches, so let's insert it in the MUI list */
    {
      e.MatchEntry = (struct ABookNode *)abn;
      DoMethod(stuff->list, MUIM_NList_InsertSingle, &e, MUIV_NList_Insert_Sorted);
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// FindAllABMatches
// tries to find all matching addressbook entries and add them to the list
static void FindAllABMatches(const struct ABook *abook, const char *text, Object *list)
{
  struct FindAllABMatchesStuff stuff;

  ENTER();

  stuff.text = text;
  stuff.textlen = strlen(text);
  stuff.list = list;
  IterateABook(abook, 0, FindAllABMatchesEntry, &stuff);

  LEAVE();
}

///
/// FlushIndex
// flushes (saves/expungs) the loaded index of a folder. You have to
// make sure yourself to lock the folder list accordingly when you
// use this function. The 'minAccessTime' variable can be used to
// prevent this function from expunging the index in case the
// folder was last accessed >= minAccessTime. Use 0 to expunge it
// at all times.
static void FlushIndex(struct Folder *folder, time_t minAccessTime)
{
  ENTER();

  // make sure the folder index is saved
  if(isModified(folder))
    MA_SaveIndex(folder);

  // flush the index if
  // - the index is loaded at all, and
  // - the minimum access time has been exceeded
  // - the folder is not the currently active one
  if(folder->LoadedMode == LM_VALID &&
     (minAccessTime == 0 || minAccessTime >= folder->lastAccessTime) &&
     folder != GetCurrentFolder())
  {
    if(minAccessTime != 0)
      D(DBF_FOLDER, "flush index of folder '%s' due to lastAccessTime (%d) < minAccessTime (%d)", folder->Name, folder->lastAccessTime, minAccessTime);
    else
      D(DBF_FOLDER, "flush index of folder '%s'", folder->Name);

    ClearFolderMails(folder, FALSE);
    folder->LoadedMode = LM_FLUSHED;
    clearFlag(folder->Flags, FOFL_FREEXS);
  }

  LEAVE();
}

///

/* Public Methods */
/// DECLARE(FindEmailMatches)
DECLARE(FindEmailMatches) // STRPTR matchText, Object *list
{
  GETDATA;

  ENTER();

  if(IsStrEmpty(msg->matchText) == FALSE)
  {
    // We first try to find matches in the Addressbook
    // and add them to the MUI list
    FindAllABMatches(&G->abook, msg->matchText, msg->list);

    // If the user has selected the EmailCache feature we also have to check this
    // list and add matches to the MUI List too
    if(C->EmailCache > 0)
      FindAllABMatches(&data->emailCache, msg->matchText, msg->list);
  }

  RETURN(0);
  return 0;
}

///
/// FindEMailCacheMatchEntry
/// DECLARE(FindEmailCacheMatch)
// Method that search in the email cache and return the found entry if not more than one
DECLARE(FindEmailCacheMatch) // STRPTR matchText
{
  GETDATA;
  struct ABookNode *foundentry = NULL;

  ENTER();

  if(C->EmailCache > 0 && IsStrEmpty(msg->matchText) == FALSE)
  {
    if(SearchABook(&data->emailCache, msg->matchText, ASM_REALNAME|ASM_ADDRESS|ASM_USER, &foundentry) > 1)
      foundentry = NULL;
  }

  RETURN(foundentry);
  return (IPTR)foundentry;
}

///
/// DECLARE(AddToEmailCache)
// method that parses a string for addresses and add them to the emailcache if enabled
DECLARE(AddToEmailCache) // struct Person *person
{
  GETDATA;

  ENTER();

  // if the emailcache feature is turned off or
  // the supplied person doesn't have a address, lets exit immediatly
  if(C->EmailCache == 0 || !msg->person->Address[0])
  {
    RETURN(-1);
    return -1;
  }

  // We first check the Addressbook if this Person already exists in the AB and if
  // so we cancel this whole operation.
  if(FindPersonInABook(&G->abook, msg->person) == NULL)
  {
    struct ABookNode *abn;

    // Ok, it doesn't exists in the AB, now lets check the cache list
    // itself
    if((abn = FindPersonInABook(&data->emailCache, msg->person)) != NULL)
    {
      // if we find the same entry already in the list we just move it
      // up to the top
      MoveABookNode(&data->emailCache.rootGroup, abn, NULL);
      // the cache was modified
      data->emailCache.modified = TRUE;
    }

    // if we didn't find the person already in the list
    // we have to add it after the last node
    if(abn == NULL)
    {
      // create a new entry
      if((abn = CreateABookNode(ABNT_USER)) != NULL)
      {
        char *p;

        // Lets copy the data in the new Person struct
        // for the real name we have to check for possible commas without quotes yet
        if(strchr(msg->person->RealName, ',') != NULL && msg->person->RealName[0] != '"')
        {
          // add the necessary double quotes around the real name
          snprintf(abn->RealName, sizeof(abn->RealName), "\"%s\"", msg->person->RealName);
        }
        else
        {
          // simply copy the real name
          strlcpy(abn->RealName, msg->person->RealName, sizeof(abn->RealName));
        }
        strlcpy(abn->Address, msg->person->Address, sizeof(abn->Address));

        // strip any single quotes from the real name
        p = abn->RealName;
        while((p = strchr(p, '\'')) != NULL)
        {
          // move all characters one backward including the trailing NUL byte
          memmove(p, p+1, strlen(p)+1);
        }

        // we always add new items to the top because this is a FILO
        AddABookNode(&data->emailCache.rootGroup, abn, NULL);
        // the cache was modified
        data->emailCache.modified = TRUE;
      }
    }
  }

  RETURN(0);
  return 0;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  char filebuf[SIZE_PATHFILE];
  char verbuf[CBD_TITLELEN];

  // prepare a string pointer array with all the
  // names of the used classes within. This array is only usefull if MUI v20
  // is used and the user wants to alter the MUI settings of the application
  static const char *const Classes[] = { "BetterString.mcc",
                                         "NBalance.mcc",
                                         "NList.mcc",
                                         "NListtree.mcc",
                                         "NListviews.mcc",
                                         "TextEditor.mcc",
                                         "TheBar.mcc",
                                         "Urltext.mcc",
                                         NULL
                                       };

  // now we load the standard icons like (check.info, new.info etc)
  // but we also try to take care of different icon.library versions.
  AddPath(filebuf, G->ProgDir, G->ProgName, sizeof(filebuf));

  if(LIB_VERSION_IS_AT_LEAST(IconBase, 44, 0) == TRUE)
  {
    G->HideIcon = GetIconTags(filebuf,
      ICONGETA_FailIfUnavailable, FALSE,
      TAG_DONE);
  }
  else
  {
    G->HideIcon = GetDiskObjectNew(filebuf);
  }

  // set up the version string for the Commodity title
  // the string MUST include the "$VER:" cookie, because this one will be stripped
  // by MUI. However, to avoid any problems with two version cookies in the final
  // executable we set up this one here in a bit more obfuscated fashion.
  snprintf(verbuf, sizeof(verbuf), "%sVER: YAM %s (%s)", "$", yamver, yamversiondate);

  if((obj = (Object *)DoSuperNew(cl, obj,

    MUIA_Application_Author,         "YAM Open Source Team",
    MUIA_Application_Base,           "YAM",
    MUIA_Application_Title,          "YAM",
    MUIA_Application_Version,        verbuf,
    MUIA_Application_Copyright,      yamcopyright,
    MUIA_Application_Description,    tr(MSG_APP_DESCRIPTION),
    MUIA_Application_UseRexx,        FALSE,
    MUIA_Application_UsedClasses,    Classes,
    MUIA_Application_HelpFile,       "http://yam.ch/wiki",
    MUIA_Application_DiskObject,     G->HideIcon,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;
    struct DateTime dt;
    struct TagItem *tags = inittags(msg), *tag;

    // now we generate some static default for our whole application
    dt.dat_Stamp.ds_Days   = yamversiondays;
    dt.dat_Stamp.ds_Minute = 0;
    dt.dat_Stamp.ds_Tick   = 0;
    dt.dat_Format          = FORMAT_DEF;
    dt.dat_Flags           = 0L;
    dt.dat_StrDay          = NULL;
    dt.dat_StrDate         = data->compileInfo;
    dt.dat_StrTime         = NULL;
    DateToStr(&dt);
    data->compileInfo[31] = '\0';  // make sure that the string is really terminated at LEN_DATSTRING.

    // now we add the compiler information as YAM can be
    // compiled with different versions and types of compilers
    snprintf(data->compileInfo, sizeof(data->compileInfo), "%s (%s, r%ld)", data->compileInfo,
                                                                            yamcompiler,
                                                                            yamsvnrev);

    data->emailCacheName = (STRPTR)EMAILCACHENAME;
    while((tag = NextTagItem((APTR)&tags)) != NULL)
    {
      switch(tag->ti_Tag)
      {
        case ATTR(EMailCacheName) : data->emailCacheName = (char *)tag->ti_Data; break;
      }
    }

    LoadEMailCache(data->emailCacheName, &data->emailCache);

    if(GetTagData(ATTR(Hidden), FALSE, inittags(msg)) != FALSE)
      set(obj, MUIA_Application_Iconified, TRUE);

    DoMethod(obj, MUIM_Notify, MUIA_Application_DoubleStart, TRUE, MUIV_Notify_Self, 1, METHOD(PopUp));
  }

  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // save the email cache if it was modified
  if(data->emailCache.modified == TRUE)
    SaveEMailCache(data->emailCacheName, &data->emailCache);

  // lets free the EMailCache List ourself in here, to make it a bit cleaner.
  ClearABook(&data->emailCache);

  // then we call the supermethod to let
  // MUI free the rest for us.
  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(CompileInfo) : *store = (IPTR)data->compileInfo; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      // if the Application is going to be (un)iconified we
      // have to take care of certain tasks.
      case MUIA_Application_Iconified:
      {
        // in case we have an applicationID we make sure
        // we notify application.library that YAM was uniconified
        #if defined(__amigaos4__)
        if(G->applicationID != 0)
        {
          struct TagItem hiddenTags[] =
          {
            { APPATTR_Hidden, tag->ti_Data },
            { TAG_DONE, 0 }
          };

          // adapt the tag values for the different interface versions
          if(IApplication->Data.Version >= 2)
          {
            if(APPATTR_Hidden < TAG_USER)
              hiddenTags[0].ti_Tag += TAG_USER;
          }
          else
          {
            if(APPATTR_Hidden >= TAG_USER)
              hiddenTags[0].ti_Tag -= TAG_USER;
          }

          SetApplicationAttrsA(G->applicationID, hiddenTags);
        }
        #endif

        D(DBF_STARTUP, "application was %s", tag->ti_Data ? "iconified" : "uniconified");

        data->iconified = tag->ti_Data;
        if(data->iconified == TRUE)
          DoMethod(obj, METHOD(FlushFolderIndexes), TRUE);
      }
      break;

      case MUIA_Application_Active:
      {
        D(DBF_STARTUP, "application is %s", tag->ti_Data ? "active" : "inactive");

        set(obj, MUIA_Application_Sleep, !tag->ti_Data);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_Application_ShowHelp)
OVERLOAD(MUIM_Application_ShowHelp)
{
  char *helpFile = ((struct MUIP_Application_ShowHelp *)msg)->name;
  ULONG result = 0;

  ENTER();

  // as we use the ShowHelp method to construct the correct URL to
  // our online documentation we don't call DoSuperMethod() here but
  // do all on ourselve.

  // if helpFile is NULL we use the global one
  if(helpFile == NULL)
    helpFile = (char *)xget(obj, MUIA_Application_HelpFile);

  // check that helpFile is definitly not null
  if(helpFile != NULL)
  {
    // construct the URL from the HelpFile and the HelpNode
    char *url = NULL;
    char *helpNode = ((struct MUIP_Application_ShowHelp *)msg)->node;
    const char *langCode = tr(MSG_DOC_LANGUAGE_CODE);

    if(helpNode != NULL)
    {
      if(IsStrEmpty(langCode))
        asprintf(&url, "%s/Documentation/%s", helpFile, helpNode);
      else
        asprintf(&url, "%s/%s:Documentation/%s", helpFile, langCode, helpNode);
    }
    else
    {
      if(IsStrEmpty(langCode))
        asprintf(&url, "%s/Documentation", helpFile);
      else
        asprintf(&url, "%s/%s:Documentation", helpFile, langCode);
    }

    D(DBF_GUI, "opening help url: '%s'", url);
    GotoURL(url, FALSE);

    free(url);
  }
  else
    W(DBF_GUI, "HelpFile is NULL");

  RETURN(result);
  return result;
}

///
/// DECLARE(UpdateCheck)
DECLARE(UpdateCheck) // ULONG quiet
{
  ENTER();

  // stop a possibly running timer
  // the update check will reinitiate it if necessary
  StopTimer(TIMER_UPDATECHECK);

  // perform the update check and update our open GUI
  // elements accordingly.
  CheckForUpdates(msg->quiet);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ShowError)
// show an error message and free() the pointer
// NOTE: the message must have been allocated by malloc() or similar!
DECLARE(ShowError) // char *message
{
  ENTER();

  ER_NewError(msg->message);
  free(msg->message);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ShowWarning)
// show a warning message and free() the pointer
// NOTE: the message must have been allocated by malloc() or similar!
DECLARE(ShowWarning) // char *message
{
  ENTER();

  ER_NewWarning(msg->message);
  free(msg->message);

  RETURN(0);
  return 0;
}

///
/// DECLARE(BusyBegin)
DECLARE(BusyBegin) // ULONG type
{
  return (IPTR)BusyBegin(msg->type);
}

///
/// DECLARE(BusyText)
DECLARE(BusyText) // APTR handle, const char *text, const char *param
{
  ENTER();

  BusyText(msg->handle, msg->text, msg->param);

  RETURN(0);
  return 0;
}

///
/// DECLARE(BusyProgress)
DECLARE(BusyProgress) // APTR handle, int progress, int max
{
  return BusyProgress(msg->handle, msg->progress, msg->max);
}

///
/// DECLARE(BusyEnd)
DECLARE(BusyEnd) // APTR handle
{
  ENTER();

  BusyEnd(msg->handle);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AppendToLogfile)
// NOTE: the log message must have been allocated by malloc() or similar!
DECLARE(AppendToLogfile) // const int mode, const int id, char *logMessage
{
  ENTER();

  AppendToLogfile((enum LFMode)msg->mode, msg->id, msg->logMessage);
  free(msg->logMessage);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeFolder)
DECLARE(ChangeFolder) // struct Folder *folder, ULONG setActive
{
  ENTER();

  MA_ChangeFolder(msg->folder, msg->setActive);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DisplayStatistics)
DECLARE(DisplayStatistics) // struct Folder *folder, ULONG updateAppIcon
{
  ENTER();

  DisplayStatistics(msg->folder, msg->updateAppIcon);

  RETURN(0);
  return 0;
}

///
/// DECLARE(CreateTransferGroup)
DECLARE(CreateTransferGroup) // APTR thread, const char *title, struct Connection *connection, ULONG activate, ULONG openWindow
{
  GETDATA;
  Object *group = NULL;

  ENTER();

  // create a new transfer window if we don't have one yet
  if(data->transferWindow == NULL)
  {
    D(DBF_GUI, "creating new transfer window");

    if((data->transferWindow = TransferWindowObject,
      MUIA_Window_Activate, C->TransferWindow != TWM_HIDE && msg->openWindow == TRUE && msg->activate == TRUE,
    End) != NULL)
    {
      // enable the menu item
      set(G->MA->GUI.MI_TRANSFERS, MUIA_Menuitem_Enabled, TRUE);
    }
  }

  if(data->transferWindow != NULL)
  {
    D(DBF_GUI, "creating new transfer control group, title '%s'", msg->title);

    if((group = (Object *)DoMethod(data->transferWindow, MUIM_TransferWindow_CreateTransferControlGroup, msg->title)) != NULL)
    {
      // tell the control group about the thread and the connection being used
      xset(group, MUIA_TransferControlGroup_Thread, msg->thread,
                  MUIA_TransferControlGroup_Connection, msg->connection);

      // respect the user's settings for the transfer window
      if(C->TransferWindow != TWM_HIDE)
      {
        if(msg->openWindow == TRUE || C->TransferWindow == TWM_SHOW)
        {
          D(DBF_GUI, "visible transfer window is requested");

          // open the window only once
          if(xget(data->transferWindow, MUIA_Window_Open) == FALSE)
            SafeOpenWindow(data->transferWindow);
        }
      }
    }
  }

  RETURN((IPTR)group);
  return (IPTR)group;
}

///
/// DECLARE(DeleteTransferGroup)
DECLARE(DeleteTransferGroup) // Object *transferGroup
{
  GETDATA;

  ENTER();

  if(msg->transferGroup != NULL)
  {
    D(DBF_GUI, "removing transfer control group %08lx", msg->transferGroup);
    if(xget(data->transferWindow, MUIA_TransferWindow_NumberOfControlGroups) == 1)
    {
      // we are about to remove the last group, just dispose the window instead
      D(DBF_GUI, "closing transfer window");
      set(data->transferWindow, MUIA_Window_Open, FALSE);
      DoMethod(obj, OM_REMMEMBER, data->transferWindow);
      MUI_DisposeObject(data->transferWindow);
      data->transferWindow = NULL;

      // disable the menu item
      set(G->MA->GUI.MI_TRANSFERS, MUIA_Menuitem_Enabled, FALSE);
    }
    else
    {
      // remove the group and keep the others
      DoMethod(data->transferWindow, MUIM_TransferWindow_DeleteTransferControlGroup, msg->transferGroup);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ShowTransferWindow)
DECLARE(ShowTransferWindow)
{
  GETDATA;

  ENTER();

  if(data->transferWindow != NULL && xget(data->transferWindow, MUIA_Window_Open) == FALSE)
    SafeOpenWindow(data->transferWindow);

  RETURN(0);
  return 0;
}

///
/// DECLARE(SetStatusTo)
DECLARE(SetStatusTo) // struct Mail *mail, int addflags, int clearflags
{
  ENTER();

  MA_ChangeMailStatus(msg->mail, msg->addflags, msg->clearflags);

  RETURN(0);
  return 0;
}

///
/// DECLARE(StartMacro)
DECLARE(StartMacro) // enum Macro num, const char *param
{
  ULONG rc = FALSE;

  ENTER();

  if(msg->num == MACRO_PROMPT_USER)
  {
    if(G->RexxHost != NULL)
    {
      struct FileReqCache *frc;
      char scname[SIZE_COMMAND];

      AddPath(scname, G->ProgDir, "rexx", sizeof(scname));
      if((frc = ReqFile(ASL_REXX, G->MA->GUI.WI, tr(MSG_MA_EXECUTESCRIPT_TITLE), REQF_NONE, scname, "")))
      {
        AddPath(scname, frc->drawer, frc->file, sizeof(scname));

        // only RexxSysBase v45+ seems to support properly quoted
        // strings via the new RXFF_SCRIPT flag
        if(LIB_VERSION_IS_AT_LEAST(RexxSysBase, 45, 0) == TRUE && MyStrChr(scname, ' ') != NULL)
        {
          char command[SIZE_COMMAND];

          snprintf(command, sizeof(command), "\"%s\"", scname);
          if(SendRexxCommand(G->RexxHost, command, 0) != NULL)
            rc = TRUE;
        }
        else
        {
          if(SendRexxCommand(G->RexxHost, scname, 0) != NULL)
            rc = TRUE;
        }
      }
    }
    else
      E(DBF_REXX, "no RexxHost, cannot execute Arexx scripts");
  }
  else
  {
    struct RxHook *macro = &C->RX[msg->num];

    if(IsStrEmpty(macro->Script) == FALSE)
    {
      char command[SIZE_LARGE];
      char *s = macro->Script;
      char *p;

      command[0] = '\0';

      // now we check if the script command contains
      // the '%p' placeholder and if so we go and replace
      // it with our parameter
      while((p = strstr(s, "%p")) != NULL)
      {
        strlcat(command, s, MIN(p-s+1, (LONG)sizeof(command)));

        if(msg->param != NULL)
          strlcat(command, msg->param, sizeof(command));

        s = p+2;
      }

      // add the rest
      strlcat(command, s, sizeof(command));

      // check if the script in question is an amigados
      // or arexx script
      if(macro->IsAmigaDOS == TRUE)
      {
        struct BusyNode *busy;

        // now execute the command
        busy = BusyBegin(BUSY_TEXT);
        BusyText(busy, tr(MSG_MA_EXECUTINGCMD), "");
        LaunchCommand(command, macro->WaitTerm ? 0 : LAUNCHF_ASYNC, macro->UseConsole ? OUT_STDOUT : OUT_NIL);
        BusyEnd(busy);

        rc = TRUE;
      }
      else if(G->RexxHost != NULL) // make sure that rexx it available
      {
        BPTR fh;

        // prepare the command string
        // only RexxSysBase v45+ seems to support properly quoted
        // strings via the new RXFF_SCRIPT flag
        if(LIB_VERSION_IS_AT_LEAST(RexxSysBase, 45, 0) == FALSE)
          UnquoteString(command, FALSE);

        // make sure to open the output console handler
        if((fh = Open(macro->UseConsole ? "CON:////YAM ARexx Window/AUTO" : "NIL:", MODE_NEWFILE)) != ZERO)
        {
          struct RexxMsg *sentrm;

          // execute the Arexx command
          if((sentrm = SendRexxCommand(G->RexxHost, command, fh)) != NULL)
          {
            // if the user wants to wait for the termination
            // of the script, we do so...
            SHOWVALUE(DBF_REXX, macro->WaitTerm);
            if(macro->WaitTerm == TRUE)
            {
              struct BusyNode *busy;
              struct RexxMsg *rm;
              BOOL waiting = TRUE;

              busy = BusyBegin(BUSY_TEXT);
              BusyText(busy, tr(MSG_MA_EXECUTINGCMD), "");
              do
              {
                WaitPort(G->RexxHost->port);

                while((rm = (struct RexxMsg *)GetMsg(G->RexxHost->port)) != NULL)
                {
                  if((rm->rm_Action & RXCODEMASK) != RXCOMM)
                    ReplyMsg((struct Message *)rm);
                  else if(rm->rm_Node.mn_Node.ln_Type == NT_REPLYMSG)
                  {
                    struct RexxMsg *org = (struct RexxMsg *)rm->rm_Args[15];

                    if(org != NULL)
                    {
                      if(rm->rm_Result1 != 0)
                        ReplyRexxCommand(org, 20, ERROR_NOT_IMPLEMENTED, NULL);
                      else
                        ReplyRexxCommand(org, 0, 0, (char *)rm->rm_Result2);
                    }

                    if(rm == sentrm)
                    {
                      if(rm->rm_Result1 == 0)
                        rc = TRUE;
                      else
                        ER_NewError(tr(MSG_ER_AREXX_EXECUTION_ERROR), rm->rm_Args[0], rm->rm_Result1);

                      waiting = FALSE;
                    }

                    FreeRexxCommand(rm);
                    --G->RexxHost->replies;
                  }
                  else if(rm->rm_Args[0] != 0)
                    DoRXCommand(G->RexxHost, rm);
                  else
                    ReplyMsg((struct Message *)rm);
                }
              }
              while(waiting);
              BusyEnd(busy);
            }

            rc = TRUE;
            D(DBF_REXX, "finished");
          }
          else
          {
            Close(fh);
            ER_NewError(tr(MSG_ER_ErrorARexxScript), command);
          }
        }
        else
          ER_NewError(tr(MSG_ER_ErrorConsole));
      }
      else
        E(DBF_REXX, "no RexxHost, cannot execute Arexx script '%ld'", macro->Script);
    }
  }

  RETURN(rc);
  return rc;
}

///
/// DECLARE(MoveCopyMail)
DECLARE(MoveCopyMail) // struct Mail *mail, struct Folder *tobox, ULONG flags
{
  ENTER();

  MA_MoveCopy(msg->mail, msg->tobox, msg->flags);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteMail)
DECLARE(DeleteMail) // struct Mail *mail, ULONG flags
{
  ENTER();

  MA_DeleteSingle(msg->mail, msg->flags);

  RETURN(0);
  return 0;
}

///
/// DECLARE(FilterMail)
DECLARE(FilterMail) // const struct MinList *filterList, struct Mail *mail
{
  return FI_FilterSingleMail(msg->filterList, msg->mail, NULL, NULL);
}

///
/// DECLARE(FilterNewMails)
DECLARE(FilterNewMails) // const struct MailList *mailList, struct FilterResult *filterResult
{
  ENTER();

  FilterMails(msg->mailList, APPLY_AUTO, msg->filterResult);

  // Now we jump to the first new mail we received if the number of messages has changed
  // after the mail transfer
  if(C->JumpToIncoming == TRUE)
    MA_JumpToNewMsg();

  // only call the DisplayStatistics() function if the actual folder wasn't already the INCOMING
  // one or we would have refreshed it twice
  if(GetCurrentFolder() != NULL && !isIncomingFolder(GetCurrentFolder()))
    DisplayStatistics((struct Folder *)-1, TRUE);
  else
    UpdateAppIcon();

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateAppIcon)
DECLARE(UpdateAppIcon)
{
  ENTER();

  UpdateAppIcon();

  RETURN(0);
  return 0;
}

///
/// DECLARE(NewMailAlert)
//  Notifies user when new mail is available
DECLARE(NewMailAlert) // struct MailServerNode *msn, struct DownloadResult *downloadResult, struct FilterResult *filterResult, const ULONG flags
{
  GETDATA;

  ENTER();

  SHOWSTRING(DBF_NET, msg->msn->description);
  SHOWVALUE(DBF_NET, msg->downloadResult->downloaded);
  SHOWVALUE(DBF_NET, msg->filterResult->Spam);
  SHOWVALUE(DBF_GUI, msg->flags);

  // show the statistics only if we downloaded some mails at all,
  // and not all of them were spam mails
  if(msg->downloadResult->downloaded > 0 && msg->downloadResult->downloaded > msg->filterResult->Spam)
  {
    if(msg->msn->notifyByRequester == TRUE && isFlagClear(msg->flags, RECEIVEF_AREXX))
    {
      char buffer[SIZE_LARGE];

      // make sure the application isn't iconified
      if(data->iconified == TRUE)
        PopUp();

      snprintf(buffer, sizeof(buffer), tr(MSG_POP3_NEW_MAIL_NOTIFY_REQ), msg->msn->description, msg->downloadResult->downloaded, msg->downloadResult->onServer-msg->downloadResult->deleted, msg->downloadResult->dupeSkipped);
      if(C->SpamFilterEnabled == TRUE)
      {
        // include the number of spam classified mails
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FILTER_STATS_SPAM),
                                                                         msg->filterResult->Checked,
                                                                         msg->filterResult->Redirected,
                                                                         msg->filterResult->Forwarded,
                                                                         msg->filterResult->Replied,
                                                                         msg->filterResult->Executed,
                                                                         msg->filterResult->Moved,
                                                                         msg->filterResult->Deleted,
                                                                         msg->filterResult->Spam);
      }
      else
      {
        snprintf(&buffer[strlen(buffer)], sizeof(buffer)-strlen(buffer), tr(MSG_TR_FilterStats),
                                                                         msg->filterResult->Checked,
                                                                         msg->filterResult->Redirected,
                                                                         msg->filterResult->Forwarded,
                                                                         msg->filterResult->Replied,
                                                                         msg->filterResult->Executed,
                                                                         msg->filterResult->Moved,
                                                                         msg->filterResult->Deleted);
      }

      // show the info window.
      InfoWindowObject,
        MUIA_Window_Title, tr(MSG_TR_NewMail),
        MUIA_Window_RefWindow, G->MA->GUI.WI,
        MUIA_Window_Activate, isFlagSet(msg->flags, RECEIVEF_USER),
        MUIA_InfoWindow_Body, buffer,
      End;
    }

    #if defined(__amigaos4__)
    if(msg->msn->notifyByOS41System == TRUE)
    {
      D(DBF_GUI, "appID is %ld, application.lib is V%ld.%ld (needed V%ld.%ld)", G->applicationID, ApplicationBase->lib_Version, ApplicationBase->lib_Revision, 53, 7);
      // Notify() is V53.2+, but 53.7 fixes some serious issues
      if(G->applicationID > 0 && LIB_VERSION_IS_AT_LEAST(ApplicationBase, 53, 7) == TRUE)
      {
        // 128 chars is the current maximum :(
        char message[128];
        char imagePath[SIZE_PATHFILE];
        int count = msg->downloadResult->downloaded - msg->filterResult->Spam;

        // distinguish between single and multiple mails
        if(count == 1)
          snprintf(message, sizeof(message), tr(MSG_POP3_NEW_MAIL_NOTIFY_OS4_ONE), msg->msn->description);
        else
          snprintf(message, sizeof(message), tr(MSG_POP3_NEW_MAIL_NOTIFY_OS4_MANY), msg->msn->description, count);

        AddPath(imagePath, G->ProgDir, "Themes/default/notify", sizeof(imagePath));

        // We require 53.7+. From this version on proper tag values are used, hence there
        // is no need to distinguish between v1 and v2 interfaces here as we have to do for
        // other application.lib functions.
        Notify(G->applicationID,
          APPNOTIFY_Title, (uint32)"YAM",
          APPNOTIFY_PubScreenName, (uint32)"FRONT",
          APPNOTIFY_Text, (uint32)message,
          APPNOTIFY_CloseOnDC, TRUE,
          APPNOTIFY_BackMsg, (uint32)"POPUP",
          APPNOTIFY_ImageFile, (uint32)imagePath,
          TAG_DONE);
      }
    }
    #endif // __amigaos4__

    if(msg->msn->notifyByCommand == TRUE)
      LaunchCommand(msg->msn->notifyCommand, 0, OUT_STDOUT);

    if(msg->msn->notifyBySound == TRUE)
      PlaySound(msg->msn->notifySound);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ChangeSelected)
DECLARE(ChangeSelected) // const struct Folder *folder, const ULONG forceUpdate
{
  ENTER();

  if(msg->folder == GetCurrentFolder())
    MA_ChangeSelected(msg->forceUpdate);

  RETURN(0);
  return 0;
}

///
/// DECLARE(CreatePasswordWindow)
DECLARE(CreatePasswordWindow) // APTR thread, const char *title, const char *body, ULONG maxLength
{
  GETDATA;
  Object *window;

  ENTER();

  if((window = StringRequestWindowObject,
    MUIA_Window_Title, msg->title,
    MUIA_Window_RefWindow, G->MA->GUI.WI,
    MUIA_StringRequestWindow_Body, msg->body,
    MUIA_StringRequestWindow_YesText, tr(MSG_Okay),
    MUIA_StringRequestWindow_NoText, tr(MSG_Cancel),
    MUIA_StringRequestWindow_MaxLength, msg->maxLength,
    MUIA_StringRequestWindow_Secret, TRUE,
    MUIA_StringRequestWindow_Thread, msg->thread,
  End) != NULL)
  {
    // make sure the application isn't iconified
    if(data->iconified == TRUE)
      PopUp();

    set(window, MUIA_Window_Open, TRUE);
  }

  RETURN((IPTR)window);
  return (IPTR)window;
}

///
/// DECLARE(CreatePreselectionWindow)
DECLARE(CreatePreselectionWindow) // APTR thread, const char *title, LONG sizeLimit, enum PreselectionWindowMode mode, struct MinList *mailList
{
  GETDATA;
  Object *window;

  ENTER();

  if((window = PreselectionWindowObject,
    MUIA_Window_Title, msg->title,
    MUIA_PreselectionWindow_Thread, msg->thread,
    MUIA_PreselectionWindow_Mode, msg->mode,
    MUIA_PreselectionWindow_Mails, msg->mailList,
    MUIA_PreselectionWindow_SizeLimit, msg->sizeLimit,
  End) != NULL)
  {
    // make sure the application isn't iconified
    if(data->iconified == TRUE)
      PopUp();

    set(window, MUIA_Window_Open, TRUE);
  }

  RETURN((IPTR)window);
  return (IPTR)window;
}

///
/// DECLARE(DisposeWindow)
DECLARE(DisposeWindow) // Object *window
{
  ENTER();

  if(msg->window != NULL)
  {
    set(msg->window, MUIA_Window_Open, FALSE);
    DoMethod(obj, OM_REMMEMBER, msg->window);
    MUI_DisposeObject(msg->window);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(GotoURL)
// invoke the GotoURL() function with the given parameters
// NOTE: the error message must have been allocated by malloc() or similar!
DECLARE(GotoURL) // char *url, ULONG newWindow
{
  ENTER();

  GotoURL(msg->url, (BOOL)msg->newWindow);
  free(msg->url);

  RETURN(0);
  return 0;
}

///
/// DECLARE(PopUp)
DECLARE(PopUp)
{
  ENTER();

  if(G->MA != NULL && G->MA->GUI.WI != NULL)
    PopUp();

  RETURN(0);
  return 0;
}

///
/// DECLARE(MUIRequestA)
// call MUI requester
DECLARE(MUIRequestA) // Object *app, Object *win, LONG flags, const char *title, const char *gadgets, const char *reqtxt
{
  LONG result;
  ENTER();

  result = YAMMUIRequestA(msg->app,
                          msg->win,
                          msg->flags,
                          msg->title,
                          msg->gadgets,
                          msg->reqtxt);

  RETURN(result);
  return result;
}

///
/// DECLARE(CertWarningRequest)
//
DECLARE(CertWarningRequest) // struct Connection *conn, struct Certificate *cert
{
  BOOL result;
  ENTER();

  result = CertWarningRequest(msg->conn, msg->cert);

  RETURN(result);
  return result;
}

///
/// DECLARE(CleanupReadMailData)
// free the ReadMailData structure of a recently closed read window
DECLARE(CleanupReadMailData) // struct ReadMailData *rmData
{
  GETDATA;

  ENTER();

  // only if this is not a close operation because the application
  // is getting iconified we really cleanup our readmail data
  if(msg->rmData == G->ActiveRexxRMData || data->iconified == FALSE)
  {
    // calls the CleanupReadMailData to clean everything else up
    CleanupReadMailData(msg->rmData, TRUE);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(CleanupWriteMailData)
// free the WriteMailData structure of a recently closed write window
DECLARE(CleanupWriteMailData) // struct WriteMailData *wmData
{
  GETDATA;

  ENTER();

  // only if this is not a close operation because the application
  // is getting iconified we really cleanup our writemail data
  if(msg->wmData == G->ActiveRexxWMData || data->iconified == FALSE)
  {
    // calls the CleanupWriteMailData to clean everything else up
    CleanupWriteMailData(msg->wmData);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenAboutWindow)
// show the about window
DECLARE(OpenAboutWindow)
{
  GETDATA;

  ENTER();

  // create the about window object and open it
  if(data->aboutWindow == NULL)
  {
    data->aboutWindow = AboutWindowObject, End;

    if(data->aboutWindow != NULL)
      DoMethod(data->aboutWindow, MUIM_Notify, MUIA_Window_Open, FALSE, obj, 4, MUIM_Application_PushMethod,obj, 1, MUIM_YAMApplication_CloseAboutWindow);
  }

  if(data->aboutWindow != NULL)
    SafeOpenWindow(data->aboutWindow);

  RETURN(0);
  return 0;
}

///
/// DECLARE(CloseAboutWindow)
// close the about window
DECLARE(CloseAboutWindow)
{
  GETDATA;

  ENTER();

  DoMethod(_app(obj), MUIM_YAMApplication_DisposeWindow, data->aboutWindow);
  data->aboutWindow = NULL;

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenConfigWindow)
DECLARE(OpenConfigWindow)
{
  struct BusyNode *busy;

  ENTER();

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BUSY_OPENINGCONFIG), "");

  if(G->ConfigWinObject == NULL)
  {
    if((CE = AllocConfig()) != NULL)
    {
      if(CopyConfig(CE, C) == TRUE)
      {
        G->ConfigWinObject = ConfigWindowObject, End;
      }
    }
  }

  if(G->ConfigWinObject != NULL)
  {
    SafeOpenWindow(G->ConfigWinObject);
  }
  else
  {
    // inform the user by chiming the bells about the failure
    DisplayBeep(_screen(obj));

    FreeConfig(CE);
    CE = NULL;
  }

  BusyEnd(busy);

  RETURN(0);
  return 0;
}

///
/// DECLARE(CloseConfigWindow)
DECLARE(CloseConfigWindow)
{
  ENTER();

  DoMethod(_app(obj), MUIM_YAMApplication_DisposeWindow, G->ConfigWinObject);
  G->ConfigWinObject = NULL;

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenSearchMailWindow)
DECLARE(OpenSearchMailWindow) // struct Folder *folder
{
  ENTER();

  if(G->SearchMailWinObject == NULL)
    G->SearchMailWinObject = SearchMailWindowObject, End;

  if(G->SearchMailWinObject != NULL)
    DoMethod(G->SearchMailWinObject, MUIM_SearchMailWindow_Open, msg->folder);

  RETURN(0);
  return 0;
}

///
/// DECLARE(OpenAddressBookWindow)
DECLARE(OpenAddressBookWindow) // enum AddressbookMode mode, LONG windowNumber, Object *recipientObj
{
  BOOL result = FALSE;

  ENTER();

  if(G->ABookWinObject == NULL)
  {
    G->ABookWinObject = AddressBookWindowObject, End;
  }

  if(G->ABookWinObject != NULL)
  {
    DoMethod(G->ABookWinObject, MUIM_AddressBookWindow_Open, msg->mode, msg->windowNumber, msg->recipientObj);
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// DECLARE(EmptyTrashFolder)
DECLARE(EmptyTrashFolder) // ULONG quiet
{
  struct Folder *folder;

  ENTER();

  if((folder = FO_GetFolderByType(FT_TRASH, NULL)) != NULL)
  {
    struct BusyNode *busy;
    struct MailNode *mnode;
    int i;

    busy = BusyBegin(BUSY_PROGRESS);
    BusyText(busy, tr(MSG_BusyEmptyingTrash), "");

    LockMailList(folder->messages);

    i = 0;
    ForEachMailNode(folder->messages, mnode)
    {
      struct Mail *mail = mnode->mail;
      char mailfile[SIZE_PATHFILE];

      BusyProgress(busy, ++i, folder->Total);
      AppendToLogfile(LF_VERBOSE, 21, tr(MSG_LOG_DeletingVerbose), AddrName(mail->From), mail->Subject, folder->Name);
      GetMailFile(mailfile, sizeof(mailfile), NULL, mail);
      DeleteFile(mailfile);
    }

    // We only clear the folder if it wasn't empty anyway..
    if(i > 0)
    {
      ClearFolderMails(folder, TRUE);

      MA_ExpireIndex(folder);

      if(GetCurrentFolder() == folder)
        DisplayMailList(folder, G->MA->GUI.PG_MAILLIST);

      AppendToLogfile(LF_NORMAL, 20, tr(MSG_LOG_Deleting), i, folder->Name);

      if(msg->quiet == FALSE)
        DisplayStatistics(folder, TRUE);
    }

    UnlockMailList(folder->messages);

    BusyEnd(busy);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteOldMails)
DECLARE(DeleteOldMails)
{
  struct DateStamp today;
  ULONG today_days;
  BOOL mailsDeleted = FALSE;
  struct MailList *toBeDeletedList;

  ENTER();

  DateStampUTC(&today);
  today.ds_Minute = 0;
  today.ds_Tick = 0;
  today_days = today.ds_Days;

  // we need a temporary "to be deleted" list of mails to avoid doubly locking a folder's mail list
  if((toBeDeletedList = CreateMailList()) != NULL)
  {
    struct BusyNode *busy;
    ULONG f;
    struct FolderNode *fnode;
    ULONG delFlags = (C->RemoveOnQuit == TRUE) ? DELF_AT_ONCE : 0;

    busy = BusyBegin(BUSY_PROGRESS_ABORT);
    BusyText(busy, tr(MSG_BusyDeletingOld), "");

    LockFolderListShared(G->folders);

    f = 0;
    ForEachFolderNode(G->folders, fnode)
    {
      struct Folder *folder = fnode->folder;

      if(isGroupFolder(folder) == FALSE && folder->MaxAge > 0 && !isArchiveFolder(folder) && MA_GetIndex(folder) == TRUE)
      {
        struct MailNode *mnode;

        // calculate the maximum age for this folder
        today.ds_Days = today_days - folder->MaxAge;

        LockMailList(folder->messages);

        // initialize the list of mails to be deleted
        InitMailList(toBeDeletedList);

        ForEachMailNode(folder->messages, mnode)
        {
          struct Mail *mail = mnode->mail;

          if(CompareDates(&today, &mail->Date) < 0)
          {
            BOOL deleteMail;

            // Delete any message from trash and spam folder automatically
            // or if the message is read already (keep unread messages).
            // "Marked" messages will never be deleted automatically.
            if(isTrashFolder(folder) || isSpamFolder(folder))
            {
              // old mails in the trash and spam folders are deleted in any case
              deleteMail = TRUE;
            }
            else if(!hasStatusNew(mail) && !hasStatusMarked(mail) && hasStatusRead(mail))
            {
              // delete old mails if they are read already, but respect marked mails
              deleteMail = TRUE;
            }
            else if(folder->ExpireUnread == TRUE && !hasStatusMarked(mail))
            {
              // delete old mails if the folder's configuration allows us to do that, but
              // respect marked mails
              deleteMail = TRUE;
            }
            else
            {
              // keep the mail if it is either unread, marked or not yet old enough
              deleteMail = FALSE;
            }

            // put the mail in the "to be deleted" list if it may be deleted
            if(deleteMail == TRUE)
              AddNewMailNode(toBeDeletedList, mail);
          }
        }

        UnlockMailList(folder->messages);

        if(IsMailListEmpty(toBeDeletedList) == FALSE)
        {
          // no need to lock the "to be deleted" list as this is known in this function only.
          // Iterate through the list "by foot" as we remove the nodes, ForEachMailNode() is
          // not safe to call here!
          while((mnode = TakeMailNode(toBeDeletedList)) != NULL)
          {
            // Finally delete the mail. Removing/freeing the mail from the folder's list of mails
            // is in fact done by the MA_DeleteSingle() function itself.
            MA_DeleteSingle(mnode->mail, delFlags|DELF_QUIET);

            // remember that we deleted at least one mail
            mailsDeleted = TRUE;

            // delete the mail node itself
            DeleteMailNode(mnode);
          }

          DisplayStatistics(folder, FALSE);
        }
      }

      // if BusyProgress() returns FALSE, then the user aborted
      if(BusyProgress(busy, ++f, G->folders->count) == FALSE)
      {
        // abort the loop
        break;
      }
    }

    UnlockFolderList(G->folders);

    // delete the "to be deleted" list
    DeleteMailList(toBeDeletedList);

    BusyEnd(busy);
  }

  // MA_DeleteSingle() does not update the trash folder treeitem if something was deleted from
  // another folder, because it was advised to be quiet. So we must refresh the trash folder
  // tree item manually here to get an up-to-date folder treeview.
  if(mailsDeleted == TRUE)
  {
    struct Folder *trashFolder;

    trashFolder = FO_GetFolderByType(FT_TRASH, NULL);
    // only update the trash folder item if it is not the active one, as the active one
    // will be updated below
    if(GetCurrentFolder() != trashFolder)
      DisplayStatistics(trashFolder, FALSE);
  }

  // and last but not least we update the appIcon also
  DisplayStatistics(NULL, TRUE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteSpamMails)
DECLARE(DeleteSpamMails) // ULONG quiet
{
  ENTER();

  if(GetCurrentFolder() != NULL && isGroupFolder(GetCurrentFolder()) == FALSE)
  {
    ULONG delFlags;
    struct BusyNode *busy;
    struct MailList *mlist;

    delFlags = (msg->quiet == FALSE) ? DELF_CLOSE_WINDOWS : DELF_QUIET|DELF_CLOSE_WINDOWS;

    // show an interruptable Busy gauge
    busy = BusyBegin(BUSY_PROGRESS_ABORT);
    BusyText(busy, tr(MSG_MA_BUSYEMPTYINGSPAM), "");

    // get the complete mail list of the spam folder
    if((mlist = MA_CreateFullList(GetCurrentFolder(), FALSE)) != NULL)
    {
      struct MailNode *mnode;
      ULONG i;

      i = 0;
      ForEachMailNode(mlist, mnode)
      {
        struct Mail *mail = mnode->mail;

        // if BusyProgress() returns FALSE, then the user aborted
        if(BusyProgress(busy, ++i, mlist->count) == FALSE)
          break;

        // not every mail in the a folder *must* be spam
        // so better check this
        if(hasStatusSpam(mail))
        {
          // remove the spam mail from the folder and take care to
          // remove it immediately in case this is the SPAM folder, otherwise
          // the mail will be moved to the trash first. In fact, DeleteSingle()
          // takes care of that itself.
          MA_DeleteSingle(mail, delFlags);
        }
      }

      if(msg->quiet == FALSE)
        DisplayStatistics(GetCurrentFolder(), TRUE);

      // finally free the mail list
      DeleteMailList(mlist);
    }

    BusyEnd(busy);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(RebuildFolderIndex)
DECLARE(RebuildFolderIndex)
{
  struct Folder *folder;

  ENTER();

  folder = GetCurrentFolder();
  // on groups we don't allow any index rescanning operation
  if(folder != NULL && isGroupFolder(folder) == FALSE)
  {
    // we start a rescan by expiring the current index and issueing
    // a new MA_GetIndex(). That will also cause the GUI to refresh!
    folder->LoadedMode = LM_UNLOAD;

    MA_ExpireIndex(folder);
    if(MA_GetIndex(folder) == TRUE)
    {
      // if we are still in the folder we wanted to rescan,
      // we can refresh the list.
      if(folder == GetCurrentFolder())
        MA_ChangeFolder(NULL, FALSE);
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(FlushFolderIndexes)
DECLARE(FlushFolderIndexes) // ULONG force
{
  struct FolderNode *fnode;

  ENTER();

  D(DBF_FOLDER, "flush indexes %ld", msg->force);

  LockFolderListShared(G->folders);

  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    if(isGroupFolder(folder) == FALSE)
    {
      if(msg->force == TRUE)
      {
        FlushIndex(folder, 0);
      }
      else
      {
        if(C->ExpungeIndexes == 0)
          FlushIndex(folder, 1);
        else
          FlushIndex(folder, GetDateStamp() - C->ExpungeIndexes);
      }
    }
  }

  UnlockFolderList(G->folders);

  RETURN(0);
  return 0;
}

///
/// DECLARE(SaveLayout)
DECLARE(SaveLayout) // ULONG permanent
{
  struct List *windowList;

  ENTER();

  if((windowList = (struct List *)xget(obj, MUIA_Application_WindowList)) != NULL)
  {
    Object *window;
    Object *cstate = (Object *)GetHead(windowList);

    // trigger a snapshot action on all currently alive windows
    while((window = NextObject(&cstate)) != NULL)
      DoMethod(window, MUIM_Window_Snapshot, TRUE);
  }

  // finally save the layout of certain groups
  SaveLayout(msg->permanent);

  RETURN(0);
  return 0;
}

///

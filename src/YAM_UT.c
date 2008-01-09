/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <devices/printer.h>
#include <dos/doshunks.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/openurl.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <workbench/startup.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/keymap.h>
#include <proto/layers.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "SDI_hook.h"
#include "SDI_stdarg.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_mail_lex.h"
#include "YAM_main.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "FileInfo.h"
#include "extrasrc.h"

#include "Debug.h"

#define CRYPTBYTE 164

struct UniversalClassData
{
   struct UniversalGUIData { APTR WI; } GUI;
};

int BusyLevel = 0;


/***************************************************************************
 Utilities
***************************************************************************/

#if !defined(__amigaos4__) || (INCLUDE_VERSION < 50)
struct PathNode
{
  BPTR pn_Next;
  BPTR pn_Lock;
};
#endif

struct ZombieFile
{
  struct MinNode node;
  char *fileName;
};

/// CloneSearchPath
// This returns a duplicated search path (preferable the workbench
// searchpath) usable for NP_Path of SystemTagList().
static BPTR CloneSearchPath(void)
{
  BPTR path = 0;

  ENTER();

  if(WorkbenchBase && WorkbenchBase->lib_Version >= 44)
    WorkbenchControl(NULL, WBCTRLA_DuplicateSearchPath, &path, TAG_DONE);

  #if !defined(__amigaos4__)
  // if we couldn't obtain a duplicate copy of the workbench search
  // path here it is very likely that we are running on a system with
  // workbench.library < 44 or on MorphOS with an old workbench.lib.
  if(path == 0)
  {
    struct Process *pr = (struct Process*)FindTask(NULL);

    if(pr->pr_Task.tc_Node.ln_Type == NT_PROCESS)
    {
      struct CommandLineInterface *cli = BADDR(pr->pr_CLI);

      if(cli)
      {
        BPTR *p = &path;
        BPTR dir = cli->cli_CommandDir;

        while (dir)
        {
          BPTR dir2;
          struct FileLock *lock = BADDR(dir);
          struct PathNode *node;

          dir = lock->fl_Link;
          dir2 = DupLock(lock->fl_Key);
          if(!dir2)
            break;

          // Use AllocVec(), because this memory is freed by FreeVec()
          // by the system later
          if(!(node = AllocVec(sizeof(struct PathNode), MEMF_PUBLIC)))
          {
            UnLock(dir2);
            break;
          }

          node->pn_Next = 0;
          node->pn_Lock = dir2;
          *p = MKBADDR(node);
          p = &node->pn_Next;
        }
      }
    }
  }
  #endif

  RETURN(path);
  return path;
}

///
/// FreeSearchPath
// Free the memory returned by CloneSearchPath
static void FreeSearchPath(BPTR path)
{
  ENTER();

  if(path != 0)
  {
    #ifndef __MORPHOS__
    if(WorkbenchBase && WorkbenchBase->lib_Version >= 44)
      WorkbenchControl(NULL, WBCTRLA_FreeSearchPath, path, TAG_DONE);
    else
    #endif
    {
      #ifndef __amigaos4__
      // This is also compatible with WorkenchControl(NULL, WBCTRLA_FreeSearchPath, ...)
      // in MorphOS/Ambient environments.
      while(path)
      {
        struct PathNode *node = BADDR(path);
        path = node->pn_Next;
        UnLock(node->pn_Lock);
        FreeVec(node);
      }
      #endif
    }
  }

  LEAVE();
}
///

/*** Hooks ***/
/// AttachDspFunc
//  Attachment listview display hook
HOOKPROTONHNO(AttachDspFunc, LONG, struct NList_DisplayMessage *msg)
{
  struct Part *entry;
  char **array;

  ENTER();

  if(!msg)
  {
    RETURN(0);
    return 0;
  }

  // now we set our local variables to the DisplayMessage structure ones
  entry = (struct Part *)msg->entry;
  array = msg->strings;

  if(entry)
  {
    static char dispnu[SIZE_SMALL];
    static char dispsz[SIZE_SMALL];

    if(entry->Nr > PART_RAW)
      snprintf(array[0] = dispnu, sizeof(dispnu), "%d%s", entry->Nr, (entry->rmData && entry->Nr == entry->rmData->letterPartNum) ? "*" : "");
    else
      array[0] = (char *)"";

    if(*entry->Name)
      array[1] = entry->Name;
    else
      array[1] = (STRPTR)DescribeCT(entry->ContentType);

    // check the alternative status
    if(entry->isAltPart == TRUE && entry->Parent != NULL && entry->Parent->MainAltPart != entry)
      msg->preparses[1] = (char *)MUIX_I;

    if(entry->Size > 0)
    {
      array[2] = dispsz;

      if(entry->Decoded)
        FormatSize(entry->Size, dispsz, sizeof(dispsz), SF_AUTO);
      else
      {
        dispsz[0] = '~';
        FormatSize(entry->Size, &dispsz[1], sizeof(dispsz)-1, SF_AUTO);
      }
    }
    else
      array[2] = (char *)"";
  }
  else
  {
    array[0] = (STRPTR)tr(MSG_ATTACH_NO);
    array[1] = (STRPTR)tr(MSG_ATTACH_PART);
    array[2] = (STRPTR)tr(MSG_Size);
  }

  RETURN(0);
  return 0;
}
MakeStaticHook(AttachDspHook, AttachDspFunc);
///

/*** Requesters ***/
/// YAMMUIRequest
// Own -secure- implementation of MUI_Request with collecting and reissueing ReturnIDs
// We also have a wrapper #define MUI_Request for calling that function instead.
LONG YAMMUIRequest(Object *app, Object *win, UNUSED LONG flags, const char *tit, const char *gad, const char *format, ...)
{
  LONG result = -1;
  char reqtxt[SIZE_LINE];
  Object *WI_YAMREQ;
  Object *BT_GROUP;
  va_list args;
  char *title = NULL;
  char *gadgets = NULL;

  ENTER();

  // as the title and gadgets are const, we provide
  // local copies of those string to not risk and .rodata
  // access.
  if(tit)
    title = strdup(tit);

  if(gad)
    gadgets = strdup(gad);

  // lets create the requester text
  va_start(args, format);
  vsnprintf(reqtxt, sizeof(reqtxt), format, args);
  va_end(args);

  // if the applicationpointer is NULL we fall back to a standard requester
  if(app == NULL)
  {
    if(IntuitionBase)
    {
      struct EasyStruct ErrReq;

      ErrReq.es_StructSize   = sizeof(struct EasyStruct);
      ErrReq.es_Flags        = 0;
      ErrReq.es_Title        = title;
      ErrReq.es_TextFormat   = reqtxt;
      ErrReq.es_GadgetFormat = gadgets;

      result = EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
    }

    if(title)
      free(title);

    if(gadgets)
      free(gadgets);

    RETURN(result);
    return result;
  }

  WI_YAMREQ = WindowObject,
    MUIA_Window_Title,        title ? title : "YAM",
    MUIA_Window_RefWindow,    win,
    MUIA_Window_LeftEdge,     MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,      MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,        MUIV_Window_Width_MinMax(0),
    MUIA_Window_Height,       MUIV_Window_Height_MinMax(0),
    MUIA_Window_CloseGadget,  FALSE,
    MUIA_Window_SizeGadget,   FALSE,
    MUIA_Window_Activate,     TRUE,
    MUIA_Window_NoMenus,      TRUE,
    WindowContents, VGroup,
       MUIA_Background, MUII_RequesterBack,
       InnerSpacing(4,4),
       Child, HGroup,
          Child, TextObject,
            GroupFrame,
            InnerSpacing(8,8),
            MUIA_Background, MUII_GroupBack,
            MUIA_Text_Contents, reqtxt,
            MUIA_Text_SetMax,   TRUE,
          End,
       End,
       Child, BT_GROUP = HGroup,
          GroupSpacing(0),
       End,
    End,
  End;

  // lets see if the WindowObject could be created perfectly
  if(WI_YAMREQ)
  {
    char *next, *token;
    int num_gads, i;
    char *ul;
    BOOL active = FALSE, ie = TRUE;
    Object *BT_TEMP;

    set(app, MUIA_Application_Sleep, TRUE);
    DoMethod(app, OM_ADDMEMBER, WI_YAMREQ);

    // first we count how many gadget we have to create
    for(num_gads=1, token=gadgets; *token; token++)
    {
      if(*token == '|')
        num_gads++;
    }

    // prepare the BT_Group for the change.
    if(DoMethod(BT_GROUP, MUIM_Group_InitChange))
    {
      // now we create the buttons for the requester
      for(token=gadgets, i=0; i < num_gads; i++, token=next)
      {
        if((next = strchr(token, '|')))
          *next++ = '\0';

        if(*token == '*')
        {
          active=TRUE;
          token++;
        }

        if((ul = strchr(token, '_')))
          ie = FALSE;

        // create the button object now.
        BT_TEMP = TextObject,
                    ButtonFrame,
                    MUIA_CycleChain,    1,
                    MUIA_Text_Contents, token,
                    MUIA_Text_PreParse, "\33c",
                    MUIA_InputMode,     MUIV_InputMode_RelVerify,
                    MUIA_Background,    MUII_ButtonBack,
                    ul ? MUIA_Text_HiIndex : TAG_IGNORE, '_',
                    ul ? MUIA_ControlChar  : TAG_IGNORE, ul ? tolower(*(ul+1)) : 0,
                  End;

        if(BT_TEMP)
        {
          if(num_gads == 1)
          {
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(0));
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(0));
            DoMethod(BT_GROUP, OM_ADDMEMBER, BT_TEMP);
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(0));
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(0));
            set(WI_YAMREQ, MUIA_Window_DefaultObject, BT_TEMP);
          }
          else if(i < num_gads-1)
          {
            DoMethod(BT_GROUP, OM_ADDMEMBER, BT_TEMP);
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(4));
            DoMethod(BT_GROUP, OM_ADDMEMBER, HSpace(0));
          }
          else
          {
            DoMethod(BT_GROUP, OM_ADDMEMBER, BT_TEMP);
          }

          if(ie && num_gads == 2)
          {
            if(i==0)
            {
              DoMethod(WI_YAMREQ, MUIM_Notify, MUIA_Window_InputEvent, "y", app, 2, MUIM_Application_ReturnID, i+1);
            }
            else if(i == num_gads-1)
            {
              DoMethod(WI_YAMREQ, MUIM_Notify, MUIA_Window_InputEvent, "n", app, 2, MUIM_Application_ReturnID, i+1);
            }
          }

          if(i<=8)
          {
            // by default we set it to "-capslock f1" so that we can press f1
            // even if the capslock is on.
            char fstring[13];

            snprintf(fstring, sizeof(fstring), "-capslock f%d", i+1);
            DoMethod(WI_YAMREQ, MUIM_Notify, MUIA_Window_InputEvent, fstring, app, 2, MUIM_Application_ReturnID, i+1);
          }

          DoMethod(BT_TEMP, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, i+1);

          if(active)
          {
            set(WI_YAMREQ, MUIA_Window_ActiveObject, BT_TEMP);
            active = FALSE;
          }
        }

        // write back what we took.
        if(next)
          *(next-1) = '|';
      }

      // signal a ExitChange now
      DoMethod(BT_GROUP, MUIM_Group_ExitChange);
    }

    // we add the esc key to the input event of the requester and if we receive it we close the requester by safely
    // exiting with the last button
    DoMethod(WI_YAMREQ ,MUIM_Notify, MUIA_Window_CloseRequest, TRUE, app, 2, MUIM_Application_ReturnID, num_gads);

    // before we popup the requester we make sure
    // the application is being uniconified as popping put
    // a requester shouldn't be prevented at all.
    if(xget(G->App, MUIA_Application_Iconified) == TRUE)
      set(G->App, MUIA_Application_Iconified, FALSE);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(WI_YAMREQ))
      result = 0;
    else do
    {
      static ULONG signals=0;
      LONG ret = DoMethod(app, MUIM_Application_NewInput, &signals);

      // bail out if a button was hit
      if(ret > 0 && ret < num_gads) { result = ret; break; }
      if(ret == num_gads)           { result = 0;   break; }

      if(signals)
        signals = Wait(signals);
    }
    while(1);

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // remove & dispose the requester object
    DoMethod(app, OM_REMMEMBER, WI_YAMREQ);
    MUI_DisposeObject(WI_YAMREQ);

    // wake up the application
    set(app, MUIA_Application_Sleep, FALSE);
  }

  if(title)
    free(title);

  if(gadgets)
    free(gadgets);

  RETURN(result);
  return result;
}
///
/// StringRequest
//  Puts up a string requester
int StringRequest(char *string, int size, const char *title, const char *body,
                  const char *yestext, const char *alttext, const char *notext,
                  BOOL secret, Object *parent)
{
  Object *bt_okay;
  Object *bt_middle;
  Object *bt_cancel;
  Object *wi_sr;
  Object *st_in;
  int ret_code = -1;

  wi_sr = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
    MUIA_Window_ID,         MAKE_ID('S','R','E','Q'),
    MUIA_Window_RefWindow,  parent,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,      MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,     MUIV_Window_Height_MinMax(20),
    WindowContents, VGroup,
       MUIA_Background, MUII_RequesterBack,
       Child, VGroup,
          GroupFrame,
          MUIA_Background, MUII_GroupBack,
          Child, LLabel(body),
          Child, st_in = secret ? MakePassString("") : MakeString(size, ""),
       End,
       Child, ColGroup(3),
          Child, bt_okay = MakeButton(yestext),
          Child, bt_middle = alttext ? MakeButton(alttext) : HSpace(0),
          Child, bt_cancel = MakeButton(notext),
       End,
    End,
  End;

  if(wi_sr)
  {
    setstring(st_in, string);
    set(wi_sr, MUIA_Window_ActiveObject, st_in);
    set(G->App, MUIA_Application_Sleep, TRUE);

    DoMethod(G->App, OM_ADDMEMBER, wi_sr);

    DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(st_in,     MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_sr,     MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    if(alttext)
      DoMethod(bt_middle, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 2);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(wi_sr))
      ret_code = 0;
    else while(ret_code == -1)
    {
      static ULONG signals=0;

      switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
      {
        case 1: ret_code = 1; break;
        case 2: ret_code = 2; break;
        case 3: ret_code = 0; break;
      }

      if(ret_code == -1 && signals)
        signals = Wait(signals);
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    if(ret_code > 0)
      GetMUIString(string, st_in, size);

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, wi_sr);
    MUI_DisposeObject(wi_sr);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  return ret_code;
}
///
/// PassphraseRequest
//  Puts up a string requester for entering a PGP passphrase
static int PassphraseRequest(char *string, int size, Object *parent)
{
  char pgprem[SIZE_DEFAULT];
  Object *bt_okay;
  Object *bt_cancel;
  Object *wi_sr;
  Object *st_in;
  Object *ch_rem;
  int ret_code = -1;

  ENTER();

  snprintf(pgprem, sizeof(pgprem), "%s %d %s", tr(MSG_CO_PGPPASSINTERVAL1),
                                               abs(C->PGPPassInterval),
                                               tr(MSG_CO_PGPPASSINTERVAL2));

  wi_sr = WindowObject,
    MUIA_Window_Title,      tr(MSG_UT_PGPPASSREQ_TITLE),
    MUIA_Window_ID,         MAKE_ID('P','R','E','Q'),
    MUIA_Window_RefWindow,  parent,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,      MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,     MUIV_Window_Height_MinMax(20),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(tr(MSG_UT_PGPPassReq)),
        Child, st_in = MakePassString(""),
        Child, HGroup,
          Child, ch_rem = MakeCheck(tr(MSG_CO_PGPPASSINTERVAL1)),
          Child, Label2(pgprem),
          Child, HSpace(0),
        End,
      End,
      Child, ColGroup(3),
        Child, bt_okay = MakeButton(tr(MSG_Okay)),
        Child, HSpace(0),
        Child, bt_cancel = MakeButton(tr(MSG_Cancel)),
      End,
    End,
  End;

  if(wi_sr)
  {
    setstring(st_in, string);
    set(wi_sr, MUIA_Window_ActiveObject, st_in);
    set(ch_rem, MUIA_Selected, C->PGPPassInterval > 0);

    set(G->App, MUIA_Application_Sleep, TRUE);

    DoMethod(G->App, OM_ADDMEMBER, wi_sr);

    DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(st_in,     MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_sr,     MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(wi_sr))
      ret_code = 0;
    else while(ret_code == -1)
    {
      static ULONG signals=0;

      switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
      {
        case 1: ret_code = 1; break;
        case 3: ret_code = 0; break;
      }

      if(ret_code == -1 && signals)
        signals = Wait(signals);
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // if the user entered something reasonable
    // we get it.
    if(ret_code > 0)
    {
      GetMUIString(string, st_in, size);

      // in case the checkmark of the pgppassinterval
      // was enabled we have to enable the passinterval as
      // well
      if(GetMUICheck(ch_rem))
        C->PGPPassInterval = abs(C->PGPPassInterval);
      else if(C->PGPPassInterval > 0)
        C->PGPPassInterval = -C->PGPPassInterval;
    }

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, wi_sr);
    MUI_DisposeObject(wi_sr);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(ret_code);
  return ret_code;
}
///
/// FolderRequest
//  Allows user to choose a folder from a list
struct Folder *FolderRequest(const char *title, const char *body, const char *yestext, const char *notext,
                             struct Folder *exclude, Object *parent)
{
  struct Folder *folder = (struct Folder *)-1;
  Object *bt_okay;
  Object *bt_cancel;
  Object *wi_fr;
  Object *lv_folder;

  ENTER();

  wi_fr = WindowObject,
    MUIA_Window_Title,     title ? title : "YAM",
    MUIA_Window_ID,        MAKE_ID('F','R','E','Q'),
    MUIA_Window_RefWindow, parent,
    MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
    MUIA_Window_Height,    MUIV_Window_Height_MinMax(30),
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(body),
        Child, lv_folder = ListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_Listview_DoubleClick, TRUE,
          MUIA_Listview_List, ListObject,
            InputListFrame,
            MUIA_List_AutoVisible, TRUE,
          End,
        End,
      End,
      Child, ColGroup(3),
        Child, bt_okay = MakeButton(yestext),
        Child, HSpace(0),
        Child, bt_cancel = MakeButton(notext),
      End,
    End,
  End;

  if(wi_fr)
  {
    char *fname;
    static int lastactive;
    struct Folder **flist;

    if((flist = FO_CreateList()) != NULL)
    {
      int i;

      for(i = 1; i <= (int)*flist; i++)
      {
        if(flist[i] != exclude && !isGroupFolder(flist[i]))
          DoMethod(lv_folder, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
      }

      free(flist);
    }

    set(lv_folder, MUIA_List_Active, lastactive);
    set(wi_fr, MUIA_Window_ActiveObject, lv_folder);
    set(G->App, MUIA_Application_Sleep, TRUE);
    DoMethod(G->App, OM_ADDMEMBER, wi_fr);
    DoMethod(bt_okay  , MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(lv_folder, MUIM_Notify, MUIA_Listview_DoubleClick, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_fr, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(wi_fr))
      folder = NULL;
    else while(folder == (struct Folder *)-1)
    {
      static ULONG signals=0;

      switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
      {
        case 1:
        {
          int act = xget(lv_folder, MUIA_List_Active);

          DoMethod(lv_folder, MUIM_List_GetEntry, act, &fname);

          if((folder = FO_GetFolderByName(fname, NULL)))
            lastactive = act;
        }
        break;

        case 3:
          folder = NULL;
        break;
      }

      if(folder == (struct Folder *)-1 && signals)
        signals = Wait(signals);
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // remove & dipose the requester object
    DoMethod(G->App, OM_REMMEMBER, wi_fr);
    MUI_DisposeObject(wi_fr);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(folder);
  return folder;
}
///
/// AttachRequest
//  Allows user to select a message part (attachment) from a list
struct Part *AttachRequest(const char *title, const char *body, const char *yestext,
                           const char *notext, int mode, struct ReadMailData *rmData)
{
  struct Part *retpart = (struct Part *)-1;
  struct Part *part;
  Object *bt_okay;
  Object *bt_cancel;
  Object *wi_ar;
  Object *lv_attach;

  ENTER();

  // lets create the AttachSelection window now
  wi_ar = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
    MUIA_Window_ID,         MAKE_ID('A','R','E','Q'),
    MUIA_Window_RefWindow,  rmData->readWindow,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, LLabel(body),
        Child, NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, lv_attach = NListObject,
            InputListFrame,
            MUIA_NList_Format,               "BAR,BAR,",
            MUIA_NList_Title,                TRUE,
            MUIA_NList_DoubleClick,          TRUE,
            MUIA_NList_MultiSelect,          isMultiReq(mode) ? MUIV_NList_MultiSelect_Default : MUIV_NList_MultiSelect_None,
            MUIA_NList_DisplayHook2,         &AttachDspHook,
            MUIA_NList_DefaultObjectOnClick, FALSE,
          End,
        End,
      End,
      Child, ColGroup(3),
        Child, bt_okay = MakeButton(yestext),
        Child, HSpace(0),
        Child, bt_cancel = MakeButton(notext),
      End,
    End,
  End;

  // if creation of window was successfull
  if(wi_ar)
  {
    static struct Part spart[2];

    // add the window to our application object
    DoMethod(G->App, OM_ADDMEMBER, wi_ar);

    // lets create the static parts of the Attachrequest entries in the NList
    spart[0].Nr = PART_ORIGINAL;
    strlcpy(spart[0].Name, tr(MSG_RE_Original), sizeof(spart[0].Name));
    spart[0].Size = rmData->mail->Size;
    spart[0].Decoded = TRUE;
    DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[0], MUIV_NList_Insert_Top);
    set(lv_attach, MUIA_NList_Active, MUIV_NList_Active_Top);

    // if this AttachRequest isn`t a DISPLAY request we show all the option to select the text we actually see
    if(!isDisplayReq(mode))
    {
      spart[1].Nr = PART_ALLTEXT;
      strlcpy(spart[1].Name, tr(MSG_RE_AllTexts), sizeof(spart[1].Name));
      spart[1].Size = 0;

      DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[1], MUIV_NList_Insert_Bottom);
    }

    // now we process the mail and pick every part out to the NListview
    for(part = rmData->firstPart->Next; part; part = part->Next)
    {
      if(!isPrintReq(mode) || part->Printable)
      {
        DoMethod(lv_attach, MUIM_NList_InsertSingle, part, MUIV_NList_Insert_Bottom);
      }
    }

    // now lets create all other window dependencies (this have to be multithreaded later)
    set(wi_ar, MUIA_Window_DefaultObject, lv_attach);
    set(G->App, MUIA_Application_Sleep, TRUE);
    DoMethod(bt_okay  , MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(lv_attach, MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_ar, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    // we open the window now and listen for some events.
    if(!SafeOpenWindow(wi_ar))
      retpart = NULL;
    else while(retpart == (struct Part *)-1)
    {
      static ULONG signals=0;

      switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
      {
        case 1:
        {
          struct Part *prevpart = part;
          LONG id;

          // now we pass through every selected entry and add it to the next part.
          for(id = MUIV_NList_NextSelected_Start;; prevpart = part)
          {
            DoMethod(lv_attach, MUIM_NList_NextSelected, &id);
            if(id == MUIV_NList_NextSelected_End) break;

            DoMethod(lv_attach, MUIM_NList_GetEntry, id, &part);

            // we have to set NextSelected to NULL first
            part->NextSelected = NULL;

            if(retpart == (struct Part *)-1)
              retpart = part;
            else
              prevpart->NextSelected = part;
          }
        }
        break;

        case 3:
        {
          retpart = NULL;
        }
        break;
      }

      if(retpart == (struct Part *)-1 && signals)
        signals = Wait(signals);
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, wi_ar);
    MUI_DisposeObject(wi_ar);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(retpart);
  return retpart;
}
///
/// InfoWindow
//  Displays a text in an own modeless window
void InfoWindow(const char *title, const char *body, const char *oktext, Object *parent, BOOL active)
{
  Object *bt_okay;
  Object *wi_iw;

  ENTER();

  if((wi_iw = WindowObject,
                MUIA_Window_Title,     title,
                MUIA_Window_RefWindow, parent,
                MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
                MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
                MUIA_Window_Activate,  parent != NULL ? (active && xget(parent, MUIA_Window_Activate)) : active,
                WindowContents, VGroup,
                  MUIA_Background, MUII_RequesterBack,
                  Child, VGroup,
                    GroupFrame,
                    MUIA_Background, MUII_GroupBack,
                    Child, LLabel(body),
                  End,
                  Child, HCenter(bt_okay = MakeButton(oktext)),
                End,
              End))
  {
    DoMethod(G->App, OM_ADDMEMBER, wi_iw);
    DoMethod(bt_okay, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_DisposeSubWindow, wi_iw);
    DoMethod(wi_iw  , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_DisposeSubWindow, wi_iw);
    set(wi_iw, MUIA_Window_DefaultObject, bt_okay);
    set(wi_iw, MUIA_Window_Open, TRUE);
  }

  LEAVE();
}
///
/// CheckboxRequestFunc
// Displays a requester with a list of checkboxes
struct MUIP_CheckboxRequesterMsg
{
  ULONG active;
  ULONG *valuePtr;
  ULONG bitMask;
};

HOOKPROTONHNO(CheckboxRequesterFunc, void, struct MUIP_CheckboxRequesterMsg *msg)
{
  ENTER();

  if(msg->active)
    *msg->valuePtr |= msg->bitMask;
  else
    *msg->valuePtr &= ~msg->bitMask;

  LEAVE();
}
MakeStaticHook(CheckboxRequesterHook, CheckboxRequesterFunc);
///
/// CheckboxRequest
// Displays a requester with a list of checkboxes
LONG CheckboxRequest(Object *win, UNUSED LONG flags, const char *tit, ULONG numBoxes, const char *text, ...)
{
  char *title = NULL;
  Object *cb_group;
  Object *wi_cb;
  Object *bt_use;
  Object *bt_cancel;
  LONG result = -1;

  ENTER();

  // as the title and gadgets are const, we provide
  // local copies of those string to not risk and .rodata
  // access.
  if(tit)
    title = strdup(tit);

  wi_cb = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
//    MUIA_Window_ID,         MAKE_ID('C','R','E','Q'),
    MUIA_Window_RefWindow,  win,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    MUIA_Window_Width,      MUIV_Window_Width_MinMax(20),
    MUIA_Window_Height,     MUIV_Window_Height_MinMax(20),

    WindowContents, VGroup,
      MUIA_Background, MUII_RequesterBack,
      Child, VGroup,
        GroupFrame,
        MUIA_Background, MUII_GroupBack,
        Child, TextObject,
          MUIA_Text_Contents, text,
          MUIA_Text_SetMax,   TRUE,
        End,
        Child, VSpace(4),
        Child, cb_group = ColGroup(3),
          MUIA_Background, MUII_GroupBack,
        End,
      End,
      Child, ColGroup(3),
        Child, bt_use = MakeButton(tr(MSG_Use)),
        Child, HSpace(0),
        Child, bt_cancel = MakeButton(tr(MSG_Cancel)),
      End,
    End,
  End;

  if(wi_cb)
  {

    set(G->App, MUIA_Application_Sleep, TRUE);
    DoMethod(G->App, OM_ADDMEMBER, wi_cb);

    // prepare the group for the change.
    if(DoMethod(cb_group, MUIM_Group_InitChange))
    {
      va_list args;
      ULONG i;

      va_start(args, text);

      // start with a zero value, because we add certain bits during the creation of the boxes
      result = 0;

      // now we create the checkboxes for the requester
      for(i=0; i < numBoxes; i++)
      {
        char *label;

        Object *cb_temp;
        Object *lb_temp;
        Object *space;

        label = va_arg(args, char *);

        // create the checkbox object now.
        cb_temp = MakeCheck(label);
        lb_temp = LLabel(label);
        space   = HSpace(0);

        if(cb_temp != NULL && lb_temp != NULL)
        {
          set(cb_temp, MUIA_Selected, TRUE);
          DoMethod(cb_temp, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, G->App, 5, MUIM_CallHook, &CheckboxRequesterHook, MUIV_TriggerValue, &result, (1 << i));
          DoMethod(cb_group, OM_ADDMEMBER, cb_temp);
          DoMethod(cb_group, OM_ADDMEMBER, lb_temp);
          DoMethod(cb_group, OM_ADDMEMBER, space);

          // the checkbox is active per default, so we set the corresponding bit in the result value
          result |= (1 << i);
        }
      }

      va_end(args);

      DoMethod(cb_group, MUIM_Group_ExitChange);
    }

    DoMethod(bt_use,    MUIM_Notify, MUIA_Pressed,             FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_cb,     MUIM_Notify, MUIA_Window_CloseRequest, TRUE,  G->App, 2, MUIM_Application_ReturnID, 2);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed,             FALSE, G->App, 2, MUIM_Application_ReturnID, 3);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(wi_cb))
      result = -1;
    else
    {
      BOOL done = FALSE;

      while(done == FALSE)
      {
        static ULONG signals=0;

        switch(DoMethod(G->App, MUIM_Application_NewInput, &signals))
        {
          // user accepted the window
          // lets exit straight away
          case 1:
            done = TRUE;
          break;

          // user canceled the window signal it to the
          // caller
          case 2:
          case 3:
          {
             result = -1;
             done = TRUE;
          }
          break;
        }

        if(done == FALSE && signals)
          signals = Wait(signals);
      }
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, wi_cb);
    MUI_DisposeObject(wi_cb);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  if(title)
    free(title);

  RETURN(result);
  return result;
}
///

/*** String related ***/
/// itoa
//  Converts an integer into a string
char *itoa(int val)
{
  static char str[SIZE_SMALL];

  ENTER();

  snprintf(str, sizeof(str), "%d", val);

  RETURN(str);
  return str;
}
///
/// MatchNoCase
//  Case insensitive pattern matching
BOOL MatchNoCase(const char *string, const char *match)
{
  BOOL result = FALSE;
  LONG patternlen = strlen(match)*2+2; // ParsePattern() needs at least 2*source+2 bytes buffer
  char *pattern;

  ENTER();

  if((pattern = malloc((size_t)patternlen)) != NULL)
  {
    if(ParsePatternNoCase((STRPTR)match, pattern, patternlen) != -1)
      result = MatchPatternNoCase((STRPTR)pattern, (STRPTR)string);

    free(pattern);
  }

  RETURN(result);
  return result;
}
///
/// StripUnderscore
//  Removes underscore from button labels
char *StripUnderscore(const char *label)
{
  static char newlabel[SIZE_DEFAULT];
  char *p;

  ENTER();

  for(p=newlabel; *label; label++)
  {
    if(*label != '_')
      *p++ = *label;
  }
  *p = '\0';

  RETURN(newlabel);
  return newlabel;
}
///
/// GetNextLine
//  Reads next line from a multi-line string
char *GetNextLine(char *p1)
{
  static char *begin;
  char *p2;

  ENTER();

  if(p1 != NULL)
    begin = p1;

  p2 = begin;
  if((p1 = strchr(p2, '\n')) != NULL)
  {
    *p1 = '\0';
    begin = ++p1;
  }

  RETURN(p2);
  return p2;
}
///
/// TrimStart
//  Strips leading spaces
char *TrimStart(char *s)
{
  ENTER();

  while(*s && isspace(*s))
    ++s;

  RETURN(s);
  return s;
}
///
/// TrimEnd
//  Removes trailing spaces
char *TrimEnd(char *s)
{
  char *e = s+strlen(s)-1;

  ENTER();

  while(e >= s && isspace(*e))
    *e-- = '\0';

  RETURN(s);
  return s;
}
///
/// Trim
// Removes leading and trailing spaces
char *Trim(char *s)
{
  ENTER();

  if(s != NULL)
  {
    s = TrimStart(s);
    s = TrimEnd(s);
  }

  RETURN(s);
  return s;
}
///
/// stristr
//  Case insensitive version of strstr()
char *stristr(const char *a, const char *b)
{
  char *s = NULL;
  int l = strlen(b);

  ENTER();

  for (; *a; a++)
  {
    if(strnicmp(a, b, l) == 0)
    {
      s = (char *)a;
      break;
    }
  }

  RETURN(s);
  return s;
}
///
/// MyStrChr
//  Searches for a character in string, ignoring text in quotes
char *MyStrChr(const char *s, const char c)
{
  char *result = NULL;
  BOOL nested = FALSE;

  ENTER();

  while(*s != '\0')
  {
    if(*s == '"')
      nested = !nested;
    else if(*s == c && !nested)
    {
      result = (char *)s;
      break;
    }

    s++;
  }

  RETURN(result);
  return result;
}
///
/// AllocStrBuf
//  Allocates a dynamic buffer
char *AllocStrBuf(size_t initlen)
{
  size_t *strbuf;

  if((strbuf = calloc(initlen+sizeof(size_t), sizeof(char))) != NULL)
    *strbuf++ = initlen;

  RETURN(strbuf);
  return (char *)strbuf;
}
///
/// StrBufCpy
//  Fills a dynamic buffer
char *StrBufCpy(char *strbuf, const char *source)
{
  size_t oldlen;
  size_t newlen;
  size_t reqlen=strlen(source);
  char *newstrbuf;

  // if our strbuf is NULL we have to allocate a new buffer
  if(!strbuf && (strbuf = AllocStrBuf(reqlen+1)) == NULL)
    return NULL;

  oldlen = ((size_t *)strbuf)[-1];

  // make sure we allocate in SIZE_DEFAULT chunks
  for(newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT);

  // if we have to change the size do it now
  if(newlen != oldlen)
  {
    FreeStrBuf(strbuf);                // free previous buffer
    newstrbuf = AllocStrBuf(newlen+1); // allocate a new one
  }
  else
    newstrbuf = strbuf;

  // do a string copy into the new buffer
  if(newstrbuf)
    strlcpy(newstrbuf, source, ((size_t *)newstrbuf)[-1]);

  return newstrbuf;
}
///
/// StrBufCat
//  String concatenation using a dynamic buffer
char *StrBufCat(char *strbuf, const char *source)
{
  size_t oldlen;
  size_t newlen;
  size_t reqlen=strlen(source);
  char *newstrbuf;

  // if our strbuf is NULL we have to allocate a new buffer
  if(!strbuf)
  {
    if((strbuf = AllocStrBuf(reqlen+1)) == NULL)
      return NULL;
  }
  else
    reqlen += strlen(strbuf);

  oldlen = ((size_t *)strbuf)[-1];

  // make sure we allocate in SIZE_DEFAULT chunks
  for(newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT);

  // if we have to change the size do it now
  if(newlen != oldlen)
  {
    if((newstrbuf = AllocStrBuf(newlen+1)))
      strlcpy(newstrbuf, strbuf, newlen+1);

    FreeStrBuf(strbuf);
  }
  else
    newstrbuf = strbuf;

  // do a string copy into the new buffer
  if(newstrbuf)
    strlcat(newstrbuf, source, newlen+1);

  return newstrbuf;
}
///
/// AppendToBuffer
//  Appends a string to a dynamic-length buffer
char *AppendToBuffer(char *buf, int *wptr, int *len, const char *add)
{
  int nlen;
  int npos;

  ENTER();

  nlen = *len;
  npos = (*wptr)+strlen(add);

  while(npos >= nlen-1)
    nlen = (nlen*3)/2;

  if(nlen != *len)
    buf = realloc(buf, *len = nlen);

  if(buf != NULL)
  {
    while (*add) buf[(*wptr)++] = *add++;
    buf[*wptr] = '\0'; // we have to make sure that the string is null terminated
  }

  RETURN(buf);
  return buf;
}
///
/// Decrypt
//  Decrypts passwords
char *Decrypt(char *source)
{
  static char buffer[SIZE_PASSWORD+2];
  char *write = &buffer[SIZE_PASSWORD];

  ENTER();

  *write-- = '\0';
  while(*source != '\0')
  {
    *write-- = ((char)atoi(source)) ^ CRYPTBYTE;
    source += 4;
  }
  write++;

  RETURN(write);
  return write;
}
///
/// Encrypt
//  Encrypts passwords
char *Encrypt(const char *source)
{
  static char buffer[4*SIZE_PASSWORD+2];
  char *read = (char *)(source+strlen(source)-1);

  ENTER();

  *buffer = '\0';
  while(read >= source)
  {
    unsigned char c = (*read--) ^ CRYPTBYTE;
    int p = strlen(buffer);

    snprintf(&buffer[p], sizeof(buffer)-p, "%03d ", c);
  }

  RETURN(buffer);
  return buffer;
}
///
/// UnquoteString
//  Removes quotes from a string, skipping "escaped" quotes
char *UnquoteString(const char *s, BOOL new)
{
  char *ans;
  char *o = (char *)s;

  ENTER();

  // check if the string conatins any quotes
  if(strchr(s, '"') == NULL)
  {
    if(new)
      o = strdup(s);

    RETURN(o);
    return(o);
  }

  // now start unquoting the string
  if((ans = malloc(strlen(s)+1)))
  {
    char *t = ans;

    while(*s)
    {
      if(*s == '\\')
        *t++ = *++s;
      else if(*s == '"')
        ; // nothing
      else
        *t++ = *s;

      ++s;
    }

    *t = '\0';

    // in case the user wants to have the copy lets do it
    if(new)
    {
      RETURN(ans);
      return ans;
    }

    // otherwise overwrite the original string array
    strcpy(o, ans);

    free(ans);
  }

  RETURN(o);
  return o;
}
///

/*** File related ***/
/// GetLine
//  Gets Null terminated line of a text file
char *GetLine(FILE *fh, char *buffer, int bufsize)
{
  char *line;

  ENTER();

  // lets NUL-terminate the string at least.
  buffer[0] = '\0';

  // read in the next line or return NULL if
  // a problem occurrs. The caller then should
  // query ferror() to determine why exactly it
  // failed.
  if(fgets(buffer, bufsize, fh) != NULL)
  {
    char *ptr;

    // search for either a \r or \n and terminate there
    // if found.
    if((ptr = strpbrk(buffer, "\r\n")) != NULL)
      *ptr = '\0';

    line = buffer;
  }
  else
  {
    // something bad happened, so we return NULL to signal abortion
    line = NULL;
  }

  // now return the line
  RETURN(line);
  return line;
}

///
/// RenameFile
//  Renames a file and restores the protection bits
BOOL RenameFile(const char *oldname, const char *newname)
{
  BOOL result = FALSE;

  ENTER();

  if(Rename(oldname, newname))
  {
    // the rename succeeded, now change the file permissions

    #if defined(__amigaos4__)
    struct ExamineData *ed;

    if((ed = ExamineObjectTags(EX_StringName, newname, TAG_DONE)) != NULL)
    {
      ULONG prots = ed->Protection;

      FreeDosObject(DOS_EXAMINEDATA, ed);
      if(SetProtection(newname, prots & ~EXDF_ARCHIVE))
        result = TRUE;
    }
    #else
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB,NULL)) != NULL)
    {
      BPTR lock;

      if((lock = Lock(newname, ACCESS_READ)))
      {
        if(Examine(lock, fib))
        {
          UnLock(lock);
          if(SetProtection(newname, fib->fib_Protection & ~FIBF_ARCHIVE))
            result = TRUE;
        }
        else
          UnLock(lock);
      }
      FreeDosObject(DOS_FIB, fib);
    }
    #endif
  }

  RETURN(result);
  return result;
}
///
/// CopyFile
//  Copies a file
BOOL CopyFile(const char *dest, FILE *destfh, const char *sour, FILE *sourfh)
{
  BOOL success = FALSE;
  char *buf;

  ENTER();

  // allocate a dynamic buffer instead of placing it on the stack
  if((buf = malloc(SIZE_FILEBUF)) != NULL)
  {
    if(sour != NULL && (sourfh = fopen(sour, "r")) != NULL)
      setvbuf(sourfh, NULL, _IOFBF, SIZE_FILEBUF);

    if(sourfh != NULL && dest != NULL && (destfh = fopen(dest, "w")) != NULL)
      setvbuf(destfh, NULL, _IOFBF, SIZE_FILEBUF);

    if(sourfh != NULL && destfh != NULL)
    {
      int len;

      while((len = fread(buf, 1, SIZE_FILEBUF, sourfh)) > 0)
      {
        if(fwrite(buf, 1, len, destfh) != (size_t)len)
          break;
      }

      // if we arrived here because this was the eof of the sourcefile
      // and non of the two filehandles are in error state we can set
      // success to TRUE.
      if(feof(sourfh) && !ferror(sourfh) && !ferror(destfh))
        success = TRUE;
    }

    if(dest != NULL && destfh != NULL)
      fclose(destfh);

    if(sour != NULL && sourfh != NULL)
      fclose(sourfh);

    free(buf);
  }

  RETURN(success);
  return success;
}
///
/// MoveFile
//  Moves a file (also from one partition to another)
BOOL MoveFile(const char *oldfile, const char *newfile)
{
  BOOL success = TRUE;

  ENTER();

  // we first try to rename the file with a standard Rename()
  // and if it doesn't work we do a raw copy
  if(!RenameFile(oldfile, newfile))
  {
    // a normal rename didn't work, so lets copy the file
    if(!CopyFile(newfile, 0, oldfile, 0) ||
       DeleteFile(oldfile) == 0)
    {
      // also a copy didn't work, so lets return an error
      success = FALSE;
    }
  }

  RETURN(success);
  return success;
}
///
/// ConvertCRLF
//  Converts line breaks from LF to CRLF or vice versa
BOOL ConvertCRLF(char *in, char *out, BOOL to)
{
  BOOL success = FALSE;
  FILE *infh;

  ENTER();

  if((infh = fopen(in, "r")))
  {
    FILE *outfh;

    setvbuf(infh, NULL, _IOFBF, SIZE_FILEBUF);

    if((outfh = fopen(out, "w")))
    {
      char buf[SIZE_LINE];

      setvbuf(outfh, NULL, _IOFBF, SIZE_FILEBUF);

      while(GetLine(infh, buf, SIZE_LINE))
        fprintf(outfh, "%s%s\n", buf, to?"\r":"");

      success = TRUE;
      fclose(outfh);
    }

    fclose(infh);
  }

  RETURN(success);
  return success;
}
///
/// Word_Length
//  returns the string length of the next word
static int Word_Length(const char *buf)
{
  unsigned char c;
  int len = 0;

  while((c = *buf))
  {
    if(isspace(c))
    {
      if(c == '\n' || c == '\r')
        return 0;

      len++;
    }
    else break;

    buf++;
  }


  while((c = *buf))
  {
    if(isspace(c) || c == '\0')
      break;

    len++;
    buf++;
  }

  return len;
}
///
/// Quoting_Chars
//  Determines and copies all quoting characters ">" to the buffer "buf"
//  out of the supplied text. It also returns the number of skipable
//  characters since the start of line like "JL>"
static int Quoting_Chars(char *buf, const int len, const char *text, int *post_spaces)
{
  unsigned char c;
  BOOL quote_found = FALSE;
  int i=0;
  int last_bracket = 0;
  int skip_chars = 0;
  int pre_spaces = 0;

  ENTER();

  (*post_spaces) = 0;

  while((c = *text++) && i < len-1)
  {
    if(c == '>')
    {
      last_bracket = i+1;

      quote_found = TRUE;
    }
    else
    {
      // if the current char is a newline or not between A-Z or a-z then we
      // can break out immediately as these chars are not allowed
      if(c == '\n' || (c != ' ' && (c < 'A' || c > 'z' || (c > 'Z' && c < 'a'))))
        break;

      if(c == ' ')
      {
        if(quote_found == TRUE)
        {
          // if we end up here we can count the number of spaces
          // after the quoting characters
          (*post_spaces)++;
        }
        else if(skip_chars == 0)
        {
          pre_spaces++;
        }
        else
          break;
      }
      else if(quote_found == TRUE || skip_chars > 2)
      {
        break;
      }
      else
        skip_chars++;
    }

    buf[i++] = c;
  }

  buf[last_bracket] = '\0';

  // if we got some spaces before anything else,
  // we put the amount of found pre_spaces in the post_spaces variable
  // instead
  if(pre_spaces > 0)
    (*post_spaces) = pre_spaces;

  // return the number of skipped chars before
  // any quote char was found.
  RETURN(last_bracket ? skip_chars+pre_spaces : 0);
  return last_bracket ? skip_chars+pre_spaces : 0;
}

///
/// QuoteText
//  Main mail text quotation function. It takes the source string "src" and
//  analyzes it concerning existing quoting characters. Depending on this
//  information it adds new quoting marks "prefix" to the start of each line
//  taking care of a correct word wrap if the line gets longs than "line_max".
//  All output is directly written to the already opened filehandle "out".
void QuoteText(FILE *out, const char *src, const int len, const int line_max)
{
  ENTER();

  // make sure the output file handle is valid
  if(out)
  {
    char temp_buf[128];
    int temp_len;
    BOOL newline = TRUE;
    BOOL wrapped = FALSE; // needed to implement automatic wordwrap while quoting
    BOOL lastwasspace = FALSE;
    int skip_on_next_newline = 0;
    int line_len = 0;
    int skip_chars;
    int post_spaces = 0;
    int srclen = len;

    // find out how many quoting chars the next line has
    skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
    temp_len = strlen(temp_buf) - skip_chars;
    src += skip_chars;
    srclen -= skip_chars;

    while(srclen > 0)
    {
      char c = *src;

      // break out if we received a NUL byte, because this
      // should really never happen
      if(c == '\0')
        break;

      // skip any LF
      if(c == '\r')
      {
        src++;
        srclen--;
        continue;
      }

      // on a CR (newline)
      if(c == '\n')
      {
        src++;
        srclen--;

        // find out how many quoting chars the next line has
        skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
        src += (skip_chars + skip_on_next_newline);
        srclen -= (skip_chars + skip_on_next_newline);
        skip_on_next_newline = 0;

        if(temp_len == ((int)strlen(temp_buf)-skip_chars) && wrapped)
        {
          // the text has been wrapped previously and the quoting chars
          // are the same like the previous line, so the following text
          // probably belongs to the same paragraph

          srclen -= temp_len; // skip the quoting chars
          src += temp_len;
          wrapped = FALSE;

          // check whether the next char will be a newline or not, because
          // a newline indicates a new empty line, so there is no need to
          // cat something together at all
          if(*src != '\n')
          {
            // add a space to if this was the first quoting
            if(lastwasspace == FALSE && (temp_len == 0 || *src != ' '))
            {
              fputc(' ', out);
              line_len++;
              lastwasspace = TRUE;
            }

            continue;
          }
        }

        temp_len = strlen(temp_buf)-skip_chars;
        wrapped = FALSE;

        // check whether this line would be zero or not and if so we
        // have to care about if the user wants to also quote empty lines
        if(line_len == 0 && C->QuoteEmptyLines)
          fputs(C->QuoteChar, out);

        // then put a newline in our file
        fputc('\n', out);
        newline = TRUE;
        lastwasspace = FALSE;

        line_len = 0;

        continue;
      }

      if(newline)
      {
        if(c == '>')
        {
          fputs(C->QuoteChar, out);
          line_len += strlen(C->QuoteChar);
        }
        else
        {
          fputs(C->QuoteChar, out);
          fputc(' ', out);
          line_len += strlen(C->QuoteChar)+1;
        }

        newline = FALSE;
      }

      // we check whether this char was a whitespace
      // or not and if so we set the lastwasspace flag and we also check if
      // we are near the end of the line so that we have to initiate a word wrap
      if((lastwasspace = isspace(c)) && line_len + Word_Length(src) >= line_max)
      {
        char *indent;

        src++;
        srclen--;

        // output a newline to start a new line
        fputc('\n', out);

        // reset line_len
        line_len = 0;

        fputs(C->QuoteChar, out);
        line_len += strlen(C->QuoteChar);

        if(strlen(temp_buf))
        {
          fputs(temp_buf+skip_chars, out);
          line_len += strlen(temp_buf)-skip_chars;
          lastwasspace = FALSE;
        }
        else
        {
          fputc(' ', out);
          line_len++;
          lastwasspace = TRUE;
        }

        // lets check the indention of the next line
        if((indent = strchr(src, '\n')) && ++indent != '\0')
        {
          int pre_spaces;

          Quoting_Chars(temp_buf, sizeof(temp_buf), indent, &pre_spaces);

          skip_on_next_newline = pre_spaces;

          if(pre_spaces == 0)
            pre_spaces += post_spaces;

          while(pre_spaces--)
          {
            fputc(' ', out);
            line_len++;
            lastwasspace = TRUE;
          }
        }

        wrapped = TRUE; // indicates that a word has been wrapped manually
        continue;
      }

      fputc(c, out);
      line_len++;

      src++;
      srclen--;
    }

    // check whether we finished the quoting with
    // a newline or otherwise the followed signature won`t fit correctly
    if(newline == FALSE)
      fputc('\n', out);
  }

  LEAVE();
}
///
/// SimpleWordWrap
//  Reformats a file to a new line length
void SimpleWordWrap(char *filename, int wrapsize)
{
  BPTR fh;

  ENTER();

  if((fh = Open((STRPTR)filename, MODE_OLDFILE)))
  {
    char ch;
    int p = 0;
    int lsp = -1;
    int sol = 0;

    while(Read(fh, &ch, 1) == 1)
    {
      if(p - sol > wrapsize && lsp >= 0)
      {
        ch = '\n';
        Seek(fh, (LONG)lsp - p - 1, OFFSET_CURRENT);
        p = lsp;
        Write(fh, &ch, 1);
      }

      if(isspace(ch))
        lsp = p;
      if(ch == '\n')
      {
        sol = p + 1;
        lsp = -1;
      }
      p++;
    }
    Close(fh);
  }

  LEAVE();
}
///
/// ReqFile
//  Puts up a file requester
struct FileReqCache *ReqFile(enum ReqFileType num, Object *win,
                             const char *title, int mode,
                             const char *drawer, const char *file)
{
  // the following arrays depend on the ReqFileType enumeration
  static const char *const acceptPattern[ASL_MAX] =
  {
    "#?.addressbook#?",                    // ASL_ABOOK
    "#?.config#?",                         // ASL_CONFIG
    NULL,                                  // ASL_DETACH
    "~(#?.info)",                          // ASL_ATTACH
    "#?.(yam|rexx|rx)",                    // ASL_REXX
    "#?.(gif|jpg|jpeg|png|iff|ilbm)",      // ASL_PHOTO
    "#?.((mbx|mbox|eml|dbx|msg)|#?,#?)",   // ASL_IMPORT
    "#?.(mbx|mbox)",                       // ASL_EXPORT
    NULL,                                  // ASL_FOLDER
    "#?.(ldif|ldi)",                       // ASL_ABOOK_LIF
    "#?.csv",                              // ASL_ABOOK_CSV
    "#?.(tab|txt)",                        // ASL_ABOOK_TAB
    "#?.xml",                              // ASL_ABOOK_XML
  };

  struct FileRequester *fileReq;
  struct FileReqCache *result = NULL;

  ENTER();

  // allocate the required data for our file requester
  if((fileReq = MUI_AllocAslRequest(ASL_FileRequest, NULL)) != NULL)
  {
    const char *pattern = acceptPattern[num];
    struct FileReqCache *frc = G->FileReqCache[num];
    BOOL reqResult;
    BOOL usefrc = frc->used;

    // do the actual file request now
    reqResult = MUI_AslRequestTags(fileReq,
                                   ASLFR_Window,         xget(win, MUIA_Window_Window),
                                   ASLFR_TitleText,      title,
                                   ASLFR_PositiveText,   hasSaveModeFlag(mode) ? tr(MSG_UT_Save) : tr(MSG_UT_Load),
                                   ASLFR_DoSaveMode,     hasSaveModeFlag(mode),
                                   ASLFR_DoMultiSelect,  hasMultiSelectFlag(mode),
                                   ASLFR_DrawersOnly,    hasDrawersOnlyFlag(mode),
                                   ASLFR_RejectIcons,    FALSE,
                                   ASLFR_DoPatterns,     pattern != NULL,
                                   ASLFR_InitialFile,    file,
                                   ASLFR_InitialDrawer,  usefrc ? frc->drawer : drawer,
                                   ASLFR_InitialPattern, pattern ? pattern : "#?",
                                   usefrc ? ASLFR_InitialLeftEdge : TAG_IGNORE, frc->left_edge,
                                   usefrc ? ASLFR_InitialTopEdge  : TAG_IGNORE, frc->top_edge,
                                   usefrc ? ASLFR_InitialWidth    : TAG_IGNORE, frc->width,
                                   usefrc ? ASLFR_InitialHeight   : TAG_IGNORE, frc->height,
                                   TAG_DONE);

    // copy the data out of our fileRequester into our
    // own cached structure we return to the user
    if(reqResult)
    {
      // free previous resources
      FreeFileReqCache(frc);

      // copy all necessary data from the ASL filerequester structure
      // to our cache
      frc->file     = strdup(fileReq->fr_File);
      frc->drawer   = strdup(fileReq->fr_Drawer);
      frc->pattern  = strdup(fileReq->fr_Pattern);
      frc->numArgs  = fileReq->fr_NumArgs;
      frc->left_edge= fileReq->fr_LeftEdge;
      frc->top_edge = fileReq->fr_TopEdge;
      frc->width    = fileReq->fr_Width;
      frc->height   = fileReq->fr_Height;
      frc->used     = TRUE;

      // now we copy the optional arglist
      if(fileReq->fr_NumArgs > 0)
      {
        if((frc->argList = calloc(sizeof(char*), fileReq->fr_NumArgs)) != NULL)
        {
          int i;

          for(i=0; i < fileReq->fr_NumArgs; i++)
            frc->argList[i] = strdup(fileReq->fr_ArgList[i].wa_Name);
        }
      }
      else
        frc->argList = NULL;

      // everything worked out fine, so lets return
      // our globally cached filereq structure.
      result = frc;
    }
    else if(IoErr() != 0)
    {
      // and IoErr() != 0 signals that something
      // serious happend and that we have to inform the
      // user
      ER_NewError(tr(MSG_ER_CANTOPENASL));

      // beep the display as well
      DisplayBeep(NULL);
    }


    // free the ASL request structure again.
    MUI_FreeAslRequest(fileReq);
  }
  else
    ER_NewError(tr(MSG_ErrorAslStruct));

  RETURN(result);
  return result;
}
///
/// FreeFileReqCache
// free all structures inside a filerequest cache structure
void FreeFileReqCache(struct FileReqCache *frc)
{
  ENTER();

  if(frc->file != NULL)
    free(frc->file);

  if(frc->drawer != NULL)
    free(frc->drawer);

  if(frc->pattern != NULL)
    free(frc->pattern);

  if(frc->numArgs > 0)
  {
    int j;

    for(j=0; j < frc->numArgs; j++)
      free(frc->argList[j]);

    free(frc->argList);
  }

  LEAVE();
}
///
/// AddZombieFile
//  add an orphaned file to the zombie file list
void AddZombieFile(const char *fileName)
{
  struct ZombieFile *zombie;

  ENTER();

  if((zombie = malloc(sizeof(*zombie))) != NULL)
  {
    if((zombie->fileName = strdup(fileName)) != NULL)
    {
      AddTail((struct List *)&G->zombieFileList, (struct Node *)&zombie->node);

      D(DBF_UTIL, "added file '%s' to the zombie list", fileName);

      // trigger the retry mechanism in 5 minutes
      TC_Restart(TIO_DELETEZOMBIEFILES, 5 * 60, 0);
    }
    else
      free(zombie);
  }

  LEAVE();
}
///
/// DeleteZombieFiles
//  try to delete all files in the list of zombie files
BOOL DeleteZombieFiles(BOOL force)
{
  BOOL listCleared = TRUE;

  ENTER();

  if(IsListEmpty((struct List *)&G->zombieFileList) == FALSE)
  {
    struct MinNode *curNode;

    for(curNode = G->zombieFileList.mlh_Head; curNode->mln_Succ; )
    {
      struct ZombieFile *zombie = (struct ZombieFile *)curNode;

      // save the pointer to the next zombie first, as we probably are going to Remove() this node later
      curNode = curNode->mln_Succ;

      D(DBF_UTIL, "trying to delete zombie file '%s'", zombie->fileName);

      // try again to delete the file, if it still exists
      if(force == FALSE && FileExists(zombie->fileName) && DeleteFile(zombie->fileName) == 0)
      {
        // deleting failed again, but we are allowed to retry
        listCleared = FALSE;

        W(DBF_UTIL, "zombie file '%s' cannot be deleted, leaving in list", zombie->fileName);
      }
      else
      {
        // remove and free this node
        Remove((struct Node *)zombie);
        free(zombie->fileName);
        free(zombie);
      }
    }
  }

  RETURN(listCleared);
  return listCleared;
}
///
/// OpenTempFile
//  Creates or opens a temporary file
struct TempFile *OpenTempFile(const char *mode)
{
  struct TempFile *tf;

  ENTER();

  if((tf = calloc(1, sizeof(struct TempFile))))
  {
    // the tempfile MUST be SIZE_MFILE long because we
    // also use this tempfile routine for showing temporary mails which
    // conform to SIZE_MFILE
    char buf[SIZE_MFILE];

    // now format our temporary filename according to our Application data
    // this format tries to make the temporary filename kinda unique.
    snprintf(buf, sizeof(buf), "YAMt%08lx.tmp", GetUniqueID());

    // now add the temporary path to the filename
    AddPath(tf->Filename, C->TempDir, buf, sizeof(tf->Filename));

    if(mode != NULL)
    {
      if((tf->FP = fopen(tf->Filename, mode)) == NULL)
      {
        E(DBF_UTIL, "couldn't create temporary file: '%s'", tf->Filename);

        // on error we free everything
        free(tf);
        tf = NULL;
      }
      else
        setvbuf(tf->FP, NULL, _IOFBF, SIZE_FILEBUF);
    }
  }

  RETURN(tf);
  return tf;
}
///
/// CloseTempFile
//  Closes a temporary file
void CloseTempFile(struct TempFile *tf)
{
  ENTER();

  if(tf)
  {
    if(tf->FP)
      fclose(tf->FP);

    D(DBF_UTIL, "DeleteTempFile: %s\n", tf->Filename);
    if(DeleteFile(tf->Filename) == 0)
      AddZombieFile(tf->Filename);

    free(tf);
  }

  LEAVE();
}
///
/// DumpClipboard
//  Exports contents of clipboard unit 0 to a file
#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')
BOOL DumpClipboard(FILE *out)
{
  BOOL success = FALSE;
  struct IFFHandle *iff;

  ENTER();

  if((iff = AllocIFF()) != NULL)
  {
    if((iff->iff_Stream = (ULONG)OpenClipboard(PRIMARY_CLIP)) != 0)
    {
      InitIFFasClip(iff);
      if(OpenIFF(iff, IFFF_READ) == 0)
      {
        if(StopChunk(iff, ID_FTXT, ID_CHRS) == 0)
        {
          while(TRUE)
          {
            struct ContextNode *cn;
            long error;
            long rlen;
            UBYTE readbuf[SIZE_DEFAULT];

            error = ParseIFF(iff, IFFPARSE_SCAN);
            if(error == IFFERR_EOC)
              continue;
            else if(error != 0)
              break;

            if((cn = CurrentChunk(iff)) != NULL)
            {
              if(cn->cn_Type == ID_FTXT && cn->cn_ID == ID_CHRS)
              {
                success = TRUE;
                while((rlen = ReadChunkBytes(iff, readbuf, SIZE_DEFAULT)) > 0)
                  fwrite(readbuf, 1, (size_t)rlen, out);
              }
            }
          }
        }
        CloseIFF(iff);
      }
      CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
    }
    FreeIFF(iff);
  }

  RETURN(success);
  return success;
}
///
/// IsFolderDir
//  Checks if a directory is used as a mail folder
static BOOL IsFolderDir(const char *dir)
{
  BOOL result = FALSE;
  char *filename;
  int i;

  ENTER();

  filename = (char *)FilePart(dir);

  for(i = 0; i < FT_NUM; i++)
  {
    if(FolderName[i] != NULL && stricmp(filename, FolderName[i]) == 0)
    {
      result = TRUE;
      break;
    }
  }

  if(result == FALSE)
  {
    char fname[SIZE_PATHFILE];

    result = (FileExists(AddPath(fname, dir, ".fconfig", sizeof(fname))) ||
              FileExists(AddPath(fname, dir, ".index", sizeof(fname))));
  }

  RETURN(result);
  return result;
}
///
/// AllFolderLoaded
//  Checks if all folder index are correctly loaded
BOOL AllFolderLoaded(void)
{
  BOOL allLoaded = TRUE;
  struct Folder **flist;

  ENTER();

  if((flist = FO_CreateList()) != NULL)
  {
    int i;

    for (i = 1; i <= (int)*flist; i++)
    {
      if(flist[i]->LoadedMode != LM_VALID && !isGroupFolder(flist[i]))
      {
        allLoaded = FALSE;
        break;
      }
    }
    free(flist);
  }
  else
    allLoaded = FALSE;

  RETURN(allLoaded);
  return allLoaded;
}
///
/// DeleteMailDir (rec)
//  Recursively deletes a mail directory
BOOL DeleteMailDir(const char *dir, BOOL isroot)
{
  BOOL result = TRUE;
  APTR context;

  ENTER();

  if((context = ObtainDirContextTags(EX_StringName,   (ULONG)dir,
                                     EX_DoCurrentDir, TRUE,
                                     TAG_DONE)) != NULL)
  {
    struct ExamineData *ed;

    while((ed = ExamineDir(context)) != NULL && result == TRUE)
    {
      BOOL isdir = EXD_IS_DIRECTORY(ed);
      char *filename = (char *)ed->Name;
      char fname[SIZE_PATHFILE];

      AddPath(fname, dir, filename, sizeof(fname));

      if(isroot == TRUE)
      {
        if(isdir == TRUE)
        {
          if(IsFolderDir(fname))
            result = DeleteMailDir(fname, FALSE);
        }
        else
        {
          if(stricmp(filename, ".config")      == 0 ||
             stricmp(filename, ".glossary")    == 0 ||
             stricmp(filename, ".addressbook") == 0 ||
             stricmp(filename, ".emailcache")  == 0 ||
             stricmp(filename, ".folders")     == 0 ||
             stricmp(filename, ".spamdata")    == 0 ||
             stricmp(filename, ".uidl")        == 0)
          {
            if(DeleteFile(fname) == 0)
              result = FALSE;
          }
        }
      }
      else if(isdir == FALSE)
      {
        if(isValidMailFile(filename) == TRUE  ||
           stricmp(filename, ".fconfig") == 0 ||
           stricmp(filename, ".fimage") == 0  ||
           stricmp(filename, ".index") == 0)
        {
          if(DeleteFile(fname) == 0)
            result = FALSE;
        }
      }
    }

    ReleaseDirContext(context);

    if(result == TRUE && DeleteFile(dir) == 0)
      result = FALSE;
  }
  else
    result = FALSE;

  RETURN(result);
  return result;
}
///
/// FileToBuffer
//  Reads a complete file into memory
char *FileToBuffer(const char *file)
{
  char *text = NULL;
  LONG size;

  ENTER();

  if(ObtainFileInfo(file, FI_SIZE, &size) == TRUE &&
     size > 0 && (text = malloc((size+1)*sizeof(char))) != NULL)
  {
    FILE *fh;

    text[size] = '\0'; // NUL-terminate the string

    if((fh = fopen(file, "r")) != NULL)
    {
      if(fread(text, sizeof(char), size, fh) != (size_t)size)
      {
        free(text);
        text = NULL;
      }

      fclose(fh);
    }
    else
    {
      free(text);
      text = NULL;
    }
  }

  RETURN(text);
  return text;
}
///
/// FileCount
// returns the total number of files matching a pattern that are in a directory
// or -1 if an error occurred.
LONG FileCount(const char *directory, const char *pattern)
{
  APTR context;
  char *parsedPattern;
  LONG parsedPatternSize;
  LONG result = 0;

  ENTER();

  if(pattern == NULL)
    pattern = "#?";

  parsedPatternSize = strlen(pattern) * 2 + 2;
  if((parsedPattern = malloc(parsedPatternSize)) != NULL)
  {
    ParsePatternNoCase(pattern, parsedPattern, parsedPatternSize);

    if((context = ObtainDirContextTags(EX_StringName,  (ULONG)directory,
                                       EX_MatchString, (ULONG)parsedPattern,
                                       TAG_DONE)) != NULL)
    {
      struct ExamineData *ed;

      while((ed = ExamineDir(context)) != NULL)
      {
        // count the entries
        result++;
      }
      if(IoErr() != ERROR_NO_MORE_ENTRIES)
      {
        D(DBF_ALWAYS, "FileCount failed");
        result = -1;
      }

      ReleaseDirContext(context);
    }

    free(parsedPattern);
  }
  else
    result = -1;

  RETURN(result);
  return result;
}
///
/// AddPath
// Function that is a wrapper to AddPart so that we can add the
// specified path 'add' to an existing/non-existant 'src' which
// is then stored in dst of max size 'size'.
char *AddPath(char *dst, const char *src, const char *add, size_t size)
{
  ENTER();

  strlcpy(dst, src, size);
  if(AddPart(dst, add, size) == FALSE)
  {
    E(DBF_ALWAYS, "AddPath()/AddPart() buffer overflow detected!");
    dst = NULL;
  }

  RETURN(dst);
  return dst;
}
///

/*** Mail related ***/
/// MyRemove
//  Removes a message from a message list
static void MyRemove(struct Mail **list, struct Mail *rem)
{
  struct Mail *mail;

  ENTER();

  if(*list == rem)
    *list = rem->Next;
  else
  {
    for(mail = *list; mail->Next; mail = mail->Next)
    {
      if(mail->Next == rem)
      {
        mail->Next = rem->Next;
        break;
      }
    }
  }

  LEAVE();
}
///
/// CreateFilename
//  Prepends mail directory to a file name
char *CreateFilename(const char * const file)
{
  static char buffer[SIZE_PATHFILE];

  ENTER();

  AddPath(buffer, G->MA_MailDir, file, sizeof(buffer));

  RETURN(buffer);
  return buffer;
}
///
/// CreateDirectory
//  Makes a directory
BOOL CreateDirectory(const char *dir)
{
  BOOL success = FALSE;

  ENTER();

  // check if dir isn't empty
  if(dir[0] != '\0')
  {
    enum FType ft;

    if(ObtainFileInfo(dir, FI_TYPE, &ft) == TRUE)
    {
      if(ft == FIT_DRAWER)
        success = TRUE;
      else if(ft == FIT_NONEXIST)
      {
        char buf[SIZE_PATHFILE];
        BPTR fl;
        size_t len = strlen(dir)-1;

        // check for trailing slashes
        if(dir[len] == '/')
        {
          // we make a copy of dir first because
          // we are not allowed to modify it
          strlcpy(buf, dir, sizeof(buf));

          // remove all trailing slashes
          while(len > 0 && buf[len] == '/')
            buf[len--] = '\0';

          // set dir to our buffer
          dir = buf;
        }

        // use utility/CreateDir() to create the
        // directory
        if((fl = CreateDir(dir)))
        {
          UnLock(fl);
          success = TRUE;
        }
      }
    }

    if(G->MA != NULL && success == FALSE)
      ER_NewError(tr(MSG_ER_CantCreateDir), dir);
  }

  RETURN(success);
  return success;
}
///
/// GetFolderDir
//  Returns path of a folder directory
const char *GetFolderDir(const struct Folder *fo)
{
  static char buffer[SIZE_PATH];
  const char *dir;

  ENTER();

  if(strchr(fo->Path, ':') != NULL)
    dir = fo->Path;
  else
  {
    AddPath(buffer, G->MA_MailDir, fo->Path, sizeof(buffer));
    dir = buffer;
  }

  RETURN(dir);
  return dir;
}
///
/// GetMailFile
//  Returns path of a message file
char *GetMailFile(char *string, const struct Folder *folder, const struct Mail *mail)
{
  static char buffer[SIZE_PATHFILE];
  char *result;

  ENTER();

  if(folder == NULL && mail != NULL)
    folder = mail->Folder;

  if(string == NULL)
    string = buffer;

  AddPath(string, (folder == NULL || folder == (struct Folder *)-1) ? C->TempDir : GetFolderDir(folder), mail->MailFile, SIZE_PATHFILE);

  result = GetRealPath(string);

  RETURN(result);
  return result;
}
///
/// ExtractAddress
//  Extracts e-mail address and real name according to RFC2822 (section 3.4)
void ExtractAddress(const char *line, struct Person *pe)
{
  char *save;

  ENTER();

  pe->Address[0] = '\0';
  pe->RealName[0] = '\0';

  // create a temp copy of our source
  // string so that we don't have to alter it.
  if((save = strdup(line)) != NULL)
  {
    char *p = save;
    char *start;
    char *end;
    char *address = NULL;
    char *realname = NULL;

    // skip leading whitespaces
    while(isspace(*p))
      p++;

    // we first try to extract the email address part of the line in case
    // the email is in < > brackets (see RFC2822)
    //
    // something like: "Realname <mail@address.net>"
    if((start = MyStrChr(p, '<')) && (end = MyStrChr(start, '>')))
    {
      *start = '\0';
      *end = '\0';

      // now we have successfully extract the
      // email address between start and end
      address = ++start;

      // per definition of RFC 2822, the realname (display name)
      // should be in front of the email, but we will extract it later on
      realname = p;
    }

    // if we haven't found the email yet
    // we might have search for something like "mail@address.net (Realname)"
    if(address == NULL)
    {
      // extract the mail address first
      for(start=end=p; *end && !isspace(*end) && *end != ',' && *end != '('; end++);

      // now we should have the email address
      if(end > start)
      {
        char *s = NULL;

        if(*end != '\0')
        {
          *end = '\0';
          s = end+1;
        }

        // we have the mail address
        address = start;

        // we should have the email address now so we go and extract
        // the realname encapsulated in ( )
        if(s && (s = strchr(s, '(')))
        {
          start = ++s;

          // now we search for the last closing )
          end = strrchr(start, ')');
          if(end)
            *end = '\0';
          else
            end = start+strlen(start);

          realname = start;
        }
      }
    }

    // we successfully found an email adress, so we go
    // and copy it into our person's structure.
    if(address)
      strlcpy(pe->Address,  Trim(address), sizeof(pe->Address));

    // in case we found a descriptive realname we go and
    // parse it for quoted and escaped passages.
    if(realname)
    {
      unsigned int i;
      BOOL quoted = FALSE;

      // as a realname may be quoted '"' and also may contain escaped sequences
      // like '\"', we extract the realname more carefully here.
      p = realname;

      // make sure we strip all leading spaces
      while(isspace(*p))
        p++;

      // check if the realname is quoted or not
      if(*p == '"')
      {
        quoted = TRUE;
        p++;
      }

      for(i=0; *p && i < sizeof(pe->RealName); i++, p++)
      {
        if(quoted && (*p == '\\' || *p == '"'))
          p++;

        if(*p)
          pe->RealName[i] = *p;
        else
          break;
      }

      // make sure we properly NUL-terminate
      // the string
      if(i < sizeof(pe->RealName))
        pe->RealName[i] = '\0';
      else
        pe->RealName[sizeof(pe->RealName)-1] = '\0';

      // make sure we strip all trailing spaces
      TrimEnd(pe->RealName);
    }

    D(DBF_MIME, "addr: '%s'", pe->Address);
    D(DBF_MIME, "real: '%s'", pe->RealName);

    free(save);
  }

  LEAVE();
}
///
/// CompressMsgID
//  Creates a crc32 checksum of the MsgID, so that it can be used later
//  for the follow-up algorithms aso.
ULONG CompressMsgID(char *msgid)
{
  ULONG id = 0;

  ENTER();

  // if the MsgID is valid we calculate the CRC32 checksum and as it
  // consists only of one cycle through the crc function we call it
  // with -1
  if(msgid != NULL && msgid[0] != '\0')
    id = CRC32(msgid, strlen(msgid), -1L);

  RETURN(id);
  return id;
}
///
/// DescribeCT
//  Returns description of a content type
const char *DescribeCT(const char *ct)
{
  const char *ret = ct;

  ENTER();

  if(ct == NULL)
    ret = tr(MSG_CTunknown);
  else
  {
    struct MinNode *curNode;

    // first we search through the users' own MIME type list
    for(curNode = C->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;
      char *type;

      // find the type right after the '/' delimiter
      if((type = strchr(mt->ContentType, '/')) != NULL)
        type++;
      else
        type = (char *)"";

      // don't allow the catch-all and empty types
      if(type[0] != '*' && type[0] != '?' && type[0] != '#' && type[0] != '\0')
      {
        if(stricmp(ct, mt->ContentType) == 0 && mt->Description[0] != '\0')
        {
          ret = mt->Description;
          break;
        }
      }

    }

    // if we still haven't identified the description
    // we go and search through the internal list
    if(ret == ct)
    {
      unsigned int i;

      for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
      {
        if(stricmp(ct, IntMimeTypeArray[i].ContentType) == 0)
        {
          ret = tr(IntMimeTypeArray[i].Description);
          break;
        }
      }
    }
  }

  RETURN(ret);
  return ret;
}
///
/// GetDateStamp
//  Get number of seconds since 1/1-1978
time_t GetDateStamp(void)
{
  struct DateStamp ds;
  time_t seconds;

  ENTER();

  // get the actual time
  DateStamp(&ds);
  seconds = ds.ds_Days * 24 * 60 * 60 +
            ds.ds_Minute * 60 +
            ds.ds_Tick / TICKS_PER_SECOND;

  RETURN(seconds);
  return seconds;
}
///
/// DateStampUTC
//  gets the current system time in UTC
void DateStampUTC(struct DateStamp *ds)
{
  ENTER();

  DateStamp(ds);
  DateStampTZConvert(ds, TZC_UTC);

  LEAVE();
}
///
/// GetSysTimeUTC
//  gets the actual system time in UTC
void GetSysTimeUTC(struct TimeVal *tv)
{
  ENTER();

  GetSysTime(TIMEVAL(tv));
  TimeValTZConvert(tv, TZC_UTC);

  LEAVE();
}
///
/// TimeValTZConvert
//  converts a supplied timeval depending on the TZConvert flag to be converted
//  to/from UTC
void TimeValTZConvert(struct TimeVal *tv, enum TZConvert tzc)
{
  ENTER();

  if(tzc == TZC_LOCAL)
    tv->Seconds += (C->TimeZone + C->DaylightSaving * 60) * 60;
  else if(tzc == TZC_UTC)
    tv->Seconds -= (C->TimeZone + C->DaylightSaving * 60) * 60;

  LEAVE();
}
///
/// DateStampTZConvert
//  converts a supplied DateStamp depending on the TZConvert flag to be converted
//  to/from UTC
void DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc)
{
  ENTER();

  // convert the DateStamp from local -> UTC or visa-versa
  if(tzc == TZC_LOCAL)
    ds->ds_Minute += (C->TimeZone + C->DaylightSaving * 60);
  else if(tzc == TZC_UTC)
    ds->ds_Minute -= (C->TimeZone + C->DaylightSaving * 60);

  // we need to check the datestamp variable that it is still in it`s borders
  // after the UTC correction
  while(ds->ds_Minute < 0)
  {
    ds->ds_Minute += 1440;
    ds->ds_Days--;
  }
  while(ds->ds_Minute >= 1440)
  {
    ds->ds_Minute -= 1440;
    ds->ds_Days++;
  }

  LEAVE();
}
///
/// TimeVal2DateStamp
//  converts a struct TimeVal to a struct DateStamp
void TimeVal2DateStamp(const struct TimeVal *tv, struct DateStamp *ds, enum TZConvert tzc)
{
  LONG seconds;

  ENTER();

  seconds = tv->Seconds + (tv->Microseconds / 1000000);

  ds->ds_Days   = seconds / 86400;       // calculate the days since 1.1.1978
  ds->ds_Minute = (seconds % 86400) / 60;
  ds->ds_Tick   = (tv->Seconds % 60) * TICKS_PER_SECOND + (tv->Microseconds / 20000);

  // if we want to convert from/to UTC we need to do this now
  if(tzc != TZC_NONE)
    DateStampTZConvert(ds, tzc);

  LEAVE();
}
///
/// DateStamp2TimeVal
//  converts a struct DateStamp to a struct TimeVal
void DateStamp2TimeVal(const struct DateStamp *ds, struct TimeVal *tv, enum TZConvert tzc)
{
  ENTER();
  // check if the ptrs are set or not.
  if(ds != NULL && tv != NULL)
  {
    // creates wrong timevals from DateStamps with year >= 2114 ...
    tv->Seconds = (ds->ds_Days * 24 * 60 + ds->ds_Minute) * 60 + ds->ds_Tick / TICKS_PER_SECOND;
    tv->Microseconds = (ds->ds_Tick % TICKS_PER_SECOND) * 1000000 / TICKS_PER_SECOND;

    // if we want to convert from/to UTC we need to do this now
    if(tzc != TZC_NONE)
      TimeValTZConvert(tv, tzc);
  }
}
///
/// TimeVal2String
//  Converts a timeval structure to a string with using DateStamp2String after a convert
BOOL TimeVal2String(char *dst, int dstlen, const struct TimeVal *tv, enum DateStampType mode, enum TZConvert tzc)
{
  BOOL result;
  struct DateStamp ds;

  // convert the timeval into a datestamp
  ENTER();

  TimeVal2DateStamp(tv, &ds, TZC_NONE);

  // then call the DateStamp2String() function to get the real string
  result = DateStamp2String(dst, dstlen, &ds, mode, tzc);

  RETURN(result);
  return result;
}
///
/// DateStamp2String
//  Converts a datestamp to a string. The caller have to make sure that the destination has
//  at least 64 characters space.
BOOL DateStamp2String(char *dst, int dstlen, struct DateStamp *date, enum DateStampType mode, enum TZConvert tzc)
{
  char datestr[64], timestr[64], daystr[64]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
  struct DateTime dt;
  struct DateStamp dsnow;

  ENTER();

  // if this argument is not set we get the actual time
  if(!date)
    date = DateStamp(&dsnow);

  // now we fill the DateTime structure with the data for our request.
  dt.dat_Stamp   = *date;
  dt.dat_Format  = (mode == DSS_USDATETIME || mode == DSS_UNIXDATE) ? FORMAT_USA : FORMAT_DEF;
  dt.dat_Flags   = (mode == DSS_RELDATETIME || mode == DSS_RELDATEBEAT) ? DTF_SUBST : 0;
  dt.dat_StrDate = datestr;
  dt.dat_StrTime = timestr;
  dt.dat_StrDay  = daystr;

  // now we check whether we have to convert the datestamp to a specific TZ or not
  if(tzc != TZC_NONE)
    DateStampTZConvert(&dt.dat_Stamp, tzc);

  // lets terminate the strings as OS 3.1 is strange
  datestr[31] = '\0';
  timestr[31] = '\0';
  daystr[31]  = '\0';

  // lets convert the DateStamp now to a string
  if(DateToStr(&dt) == FALSE)
  {
    // clear the dststring as well
    dst[0] = '\0';

    RETURN(FALSE);
    return FALSE;
  }

  switch(mode)
  {
    case DSS_UNIXDATE:
    {
      int y = atoi(&datestr[6]);

      // this is a Y2K patch
      // if less then 8035 days has passed since 1.1.1978 then we are in the 20th century
      if (date->ds_Days < 8035) y += 1900;
      else y += 2000;

      snprintf(dst, dstlen, "%s %s %02d %s %d\n", wdays[dt.dat_Stamp.ds_Days%7], months[atoi(datestr)-1], atoi(&datestr[3]), timestr, y);
    }
    break;

    case DSS_DATETIME:
    case DSS_USDATETIME:
    case DSS_RELDATETIME:
    {
      snprintf(dst, dstlen, "%s %s", datestr, timestr);
    }
    break;

    case DSS_WEEKDAY:
    {
      strlcpy(dst, daystr, dstlen);
    }
    break;

    case DSS_DATE:
    {
      strlcpy(dst, datestr, dstlen);
    }
    break;

    case DSS_TIME:
    {
      strlcpy(dst, timestr, dstlen);
    }
    break;

    case DSS_BEAT:
    case DSS_DATEBEAT:
    case DSS_RELDATEBEAT:
    {
      // calculate the beat time
      LONG beat = (((date->ds_Minute-C->TimeZone+(C->DaylightSaving?0:60)+1440)%1440)*1000)/1440;

      if(mode == DSS_DATEBEAT || mode == DSS_RELDATEBEAT)
        snprintf(dst, dstlen, "%s @%03ld", datestr, beat);
      else
        snprintf(dst, dstlen, "@%03ld", beat);
    }
    break;
  }

  RETURN(TRUE);
  return TRUE;
}
///
/// DateStamp2RFCString
BOOL DateStamp2RFCString(char *dst, const int dstlen, const struct DateStamp *date, const int timeZone, const BOOL convert)
{
  struct DateStamp datestamp;
  struct ClockData cd;
  time_t seconds;
  int convertedTimeZone = (timeZone/60)*100 + (timeZone%60);

  ENTER();

  // if date == NULL we get the current time/date
  if(date == NULL)
    DateStamp(&datestamp);
  else
    memcpy(&datestamp, date, sizeof(struct DateStamp));

  // if the user wants to convert the datestamp we have to make sure we
  // substract/add the timeZone
  if(convert && timeZone != 0)
  {
    datestamp.ds_Minute += timeZone;

    // we need to check the datestamp variable that it is still in it`s borders
    // after adjustment
    while(datestamp.ds_Minute < 0)     { datestamp.ds_Minute += 1440; datestamp.ds_Days--; }
    while(datestamp.ds_Minute >= 1440) { datestamp.ds_Minute -= 1440; datestamp.ds_Days++; }
  }

  // lets form the seconds now for the Amiga2Date function
  seconds = (datestamp.ds_Days*24*60*60 + datestamp.ds_Minute*60 + datestamp.ds_Tick/TICKS_PER_SECOND);

  // use utility's Amiga2Date for calculating the correct date/time
  Amiga2Date(seconds, &cd);

  // use snprintf to format the RFC2822 conforming datetime string.
  snprintf(dst, dstlen, "%s, %02d %s %d %02d:%02d:%02d %+05d", wdays[cd.wday],
                                                               cd.mday,
                                                               months[cd.month-1],
                                                               cd.year,
                                                               cd.hour,
                                                               cd.min,
                                                               cd.sec,
                                                               convertedTimeZone);

  RETURN(TRUE);
  return TRUE;
}
///
/// DateStamp2Long
// Converts a datestamp to a pseudo numeric value
long DateStamp2Long(struct DateStamp *date)
{
  char *s;
  char datestr[64]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
  struct DateStamp dsnow;
  struct DateTime dt;
  int y;
  long res = 0;

  ENTER();

  if(!date)
    date = DateStamp(&dsnow);

  memset(&dt, 0, sizeof(struct DateTime));
  dt.dat_Stamp   = *date;
  dt.dat_Format  = FORMAT_USA;
  dt.dat_StrDate = datestr;

  if(DateToStr(&dt))
  {
    s = Trim(datestr);

    // get the year
    y = atoi(&s[6]);

    // this is a Y2K patch
    // if less then 8035 days has passed since 1.1.1978 then we are in the 20th century
    if(date->ds_Days < 8035) y += 1900;
    else y += 2000;

    res = (100*atoi(&s[3])+atoi(s))*10000+y;
  }

  RETURN(res);
  return res;
}
///
/// String2DateStamp
//  Tries to converts a string into a datestamp via StrToDate()
BOOL String2DateStamp(struct DateStamp *dst, char *string, enum DateStampType mode, enum TZConvert tzc)
{
  char datestr[64], timestr[64]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
  BOOL result = FALSE;

  ENTER();

  // depending on the DateStampType we have to try to split the string differently
  // into the separate datestr/timestr combo
  switch(mode)
  {
    case DSS_UNIXDATE:
    {
      char *p;

      // we walk from the front to the back and skip the week
      // day name
      if((p = strchr(string, ' ')))
      {
        int month;

        // extract the month
        for(month=0; month < 12; month++)
        {
          if(strnicmp(string, months[month], 3) == 0)
            break;
        }

        if(month >= 12)
          break;

        // extract the day
        if((p = strchr(p, ' ')))
        {
          int day = atoi(p+1);

          if(day < 1 || day > 31)
            break;

          // extract the timestring
          if((p = strchr(p, ' ')))
          {
            strlcpy(timestr, p+1, MIN((ULONG)8, sizeof(timestr)));

            // extract the year
            if((p = strchr(p, ' ')))
            {
              int year = atoi(p+1);

              if(year < 1970 || year > 2070)
                break;

              // now we can compose our datestr
              snprintf(datestr, sizeof(datestr), "%02d-%02d-%02d", month+1, day, year%100);

              result = TRUE;
            }
          }
        }
      }
    }
    break;

    case DSS_DATETIME:
    case DSS_USDATETIME:
    case DSS_RELDATETIME:
    {
      char *p;

      // copy the datestring
      if((p = strchr(string, ' ')))
      {
        strlcpy(datestr, string, MIN(sizeof(datestr), (unsigned int)(p - string + 1)));
        strlcpy(timestr, p + 1, sizeof(timestr));

        result = TRUE;
      }
    }
    break;

    case DSS_WEEKDAY:
    case DSS_DATE:
    {
      strlcpy(datestr, string, sizeof(datestr));
      timestr[0] = '\0';
      result = TRUE;
    }
    break;

    case DSS_TIME:
    {
      strlcpy(timestr, string, sizeof(timestr));
      datestr[0] = '\0';
      result = TRUE;
    }
    break;

    case DSS_BEAT:
    case DSS_DATEBEAT:
    case DSS_RELDATEBEAT:
      // not supported yet.
    break;
  }

  // we continue only if everything until now is fine.
  if(result == TRUE)
  {
    struct DateTime dt;

    // now we fill the DateTime structure with the data for our request.
    dt.dat_Format  = (mode == DSS_USDATETIME || mode == DSS_UNIXDATE) ? FORMAT_USA : FORMAT_DEF;
    dt.dat_Flags   = 0; // perhaps later we can add Weekday substitution
    dt.dat_StrDate = datestr;
    dt.dat_StrTime = timestr;
    dt.dat_StrDay  = NULL;

    // convert the string to a dateStamp
    if(StrToDate(&dt))
    {
      // now we check whether we have to convert the datestamp to a specific TZ or not
      if(tzc != TZC_NONE)
        DateStampTZConvert(&dt.dat_Stamp, tzc);

      // now we do copy the datestamp stuff over the one from our mail
      memcpy(dst, &dt.dat_Stamp, sizeof(struct DateStamp));
    }
    else
      result = FALSE;
  }

  if(result == FALSE)
    W(DBF_UTIL, "couldn't convert string '%s' to struct DateStamp", string);

  RETURN(result);
  return result;
}

///
/// String2TimeVal
// converts a string to a struct TimeVal, if possible.
BOOL String2TimeVal(struct TimeVal *dst, char *string, enum DateStampType mode, enum TZConvert tzc)
{
  struct DateStamp ds;
  BOOL result;

  ENTER();

  // we use the String2DateStamp function for conversion
  if((result = String2DateStamp(&ds, string, mode, tzc)))
  {
    // now we just have to convert the DateStamp to a struct TimeVal
    DateStamp2TimeVal(&ds, dst, TZC_NONE);
  }

  RETURN(result);
  return result;
}

///
/// TZtoMinutes
//  Converts time zone into a numeric offset also using timezone abbreviations
//  Refer to http://www.cise.ufl.edu/~sbeck/DateManip.html#TIMEZONES
int TZtoMinutes(char *tzone)
{
  /*
    The following timezone names are currently understood (and can be used in parsing dates).
    These are zones defined in RFC 822.
      Universal:  GMT, UT
      US zones :  EST, EDT, CST, CDT, MST, MDT, PST, PDT
      Military :  A to Z (except J)
      Other    :  +HHMM or -HHMM
      ISO 8601 :  +HH:MM, +HH, -HH:MM, -HH

      In addition, the following timezone abbreviations are also accepted. In a few
      cases, the same abbreviation is used for two different timezones (for example,
      NST stands for Newfoundland Standard -0330 and North Sumatra +0630). In these
      cases, only 1 of the two is available. The one preceded by a ``#'' sign is NOT
      available but is documented here for completeness.
   */

   static const struct
   {
     const char *TZname;
     int   TZcorr;
   } time_zone_table[] =
   {
    { "IDLW",   -1200 }, // International Date Line West
    { "NT",     -1100 }, // Nome
    { "HST",    -1000 }, // Hawaii Standard
    { "CAT",    -1000 }, // Central Alaska
    { "AHST",   -1000 }, // Alaska-Hawaii Standard
    { "AKST",    -900 }, // Alaska Standard
    { "YST",     -900 }, // Yukon Standard
    { "HDT",     -900 }, // Hawaii Daylight
    { "AKDT",    -800 }, // Alaska Daylight
    { "YDT",     -800 }, // Yukon Daylight
    { "PST",     -800 }, // Pacific Standard
    { "PDT",     -700 }, // Pacific Daylight
    { "MST",     -700 }, // Mountain Standard
    { "MDT",     -600 }, // Mountain Daylight
    { "CST",     -600 }, // Central Standard
    { "CDT",     -500 }, // Central Daylight
    { "EST",     -500 }, // Eastern Standard
    { "ACT",     -500 }, // Brazil, Acre
    { "SAT",     -400 }, // Chile
    { "BOT",     -400 }, // Bolivia
    { "EDT",     -400 }, // Eastern Daylight
    { "AST",     -400 }, // Atlantic Standard
    { "AMT",     -400 }, // Brazil, Amazon
    { "ACST",    -400 }, // Brazil, Acre Daylight
//# { "NST",     -330 }, // Newfoundland Standard       nst=North Sumatra    +0630
    { "NFT",     -330 }, // Newfoundland
//# { "GST",     -300 }, // Greenland Standard          gst=Guam Standard    +1000
//# { "BST",     -300 }, // Brazil Standard             bst=British Summer   +0100
    { "BRST",    -300 }, // Brazil Standard
    { "BRT",     -300 }, // Brazil Standard
    { "AMST",    -300 }, // Brazil, Amazon Daylight
    { "ADT",     -300 }, // Atlantic Daylight
    { "ART",     -300 }, // Argentina
    { "NDT",     -230 }, // Newfoundland Daylight
    { "AT",      -200 }, // Azores
    { "BRST",    -200 }, // Brazil Daylight (official time)
    { "FNT",     -200 }, // Brazil, Fernando de Noronha
    { "WAT",     -100 }, // West Africa
    { "FNST",    -100 }, // Brazil, Fernando de Noronha Daylight
    { "GMT",     +000 }, // Greenwich Mean
    { "UT",      +000 }, // Universal (Coordinated)
    { "UTC",     +000 }, // Universal (Coordinated)
    { "WET",     +000 }, // Western European
    { "WEST",    +000 }, // Western European Daylight
    { "CET",     +100 }, // Central European
    { "FWT",     +100 }, // French Winter
    { "MET",     +100 }, // Middle European
    { "MEZ",     +100 }, // Middle European
    { "MEWT",    +100 }, // Middle European Winter
    { "SWT",     +100 }, // Swedish Winter
    { "BST",     +100 }, // British Summer              bst=Brazil standard  -0300
    { "GB",      +100 }, // GMT with daylight savings
    { "CEST",    +200 }, // Central European Summer
    { "EET",     +200 }, // Eastern Europe, USSR Zone 1
    { "FST",     +200 }, // French Summer
    { "MEST",    +200 }, // Middle European Summer
    { "MESZ",    +200 }, // Middle European Summer
    { "METDST",  +200 }, // An alias for MEST used by HP-UX
    { "SAST",    +200 }, // South African Standard
    { "SST",     +200 }, // Swedish Summer              sst=South Sumatra    +0700
    { "EEST",    +300 }, // Eastern Europe Summer
    { "BT",      +300 }, // Baghdad, USSR Zone 2
    { "MSK",     +300 }, // Moscow
    { "EAT",     +300 }, // East Africa
    { "IT",      +330 }, // Iran
    { "ZP4",     +400 }, // USSR Zone 3
    { "MSD",     +300 }, // Moscow Daylight
    { "ZP5",     +500 }, // USSR Zone 4
    { "IST",     +530 }, // Indian Standard
    { "ZP6",     +600 }, // USSR Zone 5
    { "NOVST",   +600 }, // Novosibirsk time zone, Russia
    { "NST",     +630 }, // North Sumatra               nst=Newfoundland Std -0330
//# { "SST",     +700 }, // South Sumatra, USSR Zone 6  sst=Swedish Summer   +0200
    { "JAVT",    +700 }, // Java
    { "CCT",     +800 }, // China Coast, USSR Zone 7
    { "AWST",    +800 }, // Australian Western Standard
    { "WST",     +800 }, // West Australian Standard
    { "PHT",     +800 }, // Asia Manila
    { "JST",     +900 }, // Japan Standard, USSR Zone 8
    { "ROK",     +900 }, // Republic of Korea
    { "ACST",    +930 }, // Australian Central Standard
    { "CAST",    +930 }, // Central Australian Standard
    { "AEST",   +1000 }, // Australian Eastern Standard
    { "EAST",   +1000 }, // Eastern Australian Standard
    { "GST",    +1000 }, // Guam Standard, USSR Zone 9  gst=Greenland Std    -0300
    { "ACDT",   +1030 }, // Australian Central Daylight
    { "CADT",   +1030 }, // Central Australian Daylight
    { "AEDT",   +1100 }, // Australian Eastern Daylight
    { "EADT",   +1100 }, // Eastern Australian Daylight
    { "IDLE",   +1200 }, // International Date Line East
    { "NZST",   +1200 }, // New Zealand Standard
    { "NZT",    +1200 }, // New Zealand
    { "NZDT",   +1300 }, // New Zealand Daylight
    { NULL,         0 }  // Others can be added in the future upon request.
   };

   // Military time zone table
   static const struct
   {
      char tzcode;
      int  tzoffset;
   } military_table[] =
   {
    { 'A',  -100 },
    { 'B',  -200 },
    { 'C',  -300 },
    { 'D',  -400 },
    { 'E',  -500 },
    { 'F',  -600 },
    { 'G',  -700 },
    { 'H',  -800 },
    { 'I',  -900 },
    { 'K', -1000 },
    { 'L', -1100 },
    { 'M', -1200 },
    { 'N',  +100 },
    { 'O',  +200 },
    { 'P',  +300 },
    { 'Q',  +400 },
    { 'R',  +500 },
    { 'S',  +600 },
    { 'T',  +700 },
    { 'U',  +800 },
    { 'V',  +900 },
    { 'W', +1000 },
    { 'X', +1100 },
    { 'Y', +1200 },
    { 'Z', +0000 },
    { 0,       0 }
   };

   int tzcorr = -1;

   /*
    * first we check if the timezone string conforms to one of the
    * following standards (RFC 822)
    *
    * 1.Other    :  +HHMM or -HHMM
    * 2.ISO 8601 :  +HH:MM, +HH, -HH:MM, -HH
    * 3.Military :  A to Z (except J)
    *
    * only if none of the 3 above formats match, we take our hughe TZtable
    * and search for the timezone abbreviation
    */

   // check if the timezone definition starts with a + or -
   if(tzone[0] == '+' || tzone[0] == '-')
   {
      tzcorr = atoi(&tzone[1]);

      // check if tzcorr is correct of if it is perhaps a ISO 8601 format
      if(tzcorr != 0 && tzcorr/100 == 0)
      {
        char *c;

        // multiply it by 100 so that we have now a correct format
        tzcorr *= 100;

        // then check if we have a : to seperate HH:MM and add the minutes
        // to tzcorr
        if((c = strchr(tzone, ':'))) tzcorr += atoi(c);
      }

      // now we have to distingush between + and -
      if(tzone[0] == '-') tzcorr = -tzcorr;
   }
   else if(isalpha(tzone[0]))
   {
      int i;

      // if we end up here then the timezone string is
      // probably a abbreviation and we first check if it is a military abbr
      if(isalpha(tzone[1]) == 0) // military need to be 1 char long
      {
        for(i=0; military_table[i].tzcode; i++)
        {
          if(toupper(tzone[0]) == military_table[i].tzcode)
          {
            tzcorr = military_table[i].tzoffset;
            break;
          }
        }
      }
      else
      {
        for(i=0; time_zone_table[i].TZname; i++) // and as a last chance we scan our abbrev table
        {
          if(strnicmp(time_zone_table[i].TZname, tzone, strlen(time_zone_table[i].TZname)) == 0)
          {
            tzcorr = time_zone_table[i].TZcorr;
            D(DBF_UTIL, "TZtoMinutes: found abbreviation '%s' (%ld)", time_zone_table[i].TZname, tzcorr);
            break;
          }
        }

        if(tzcorr == -1)
          W(DBF_UTIL, "TZtoMinutes: abbreviation '%s' NOT found!", tzone);
      }
   }

   if(tzcorr == -1)
     W(DBF_UTIL, "couldn't parse timezone from '%s'", tzone);

   return tzcorr == -1 ? 0 : (tzcorr/100)*60 + (tzcorr%100);
}
///
/// FormatSize
//  Displays large numbers using group separators
void FormatSize(LONG size, char *buf, int buflen, enum SizeFormat forcedPrecision)
{
  const char *dp;
  double dsize;

  ENTER();

  dp = G->Locale ? (const char *)G->Locale->loc_DecimalPoint : ".";
  dsize = (double)size;

  // see if the user wants to force a precision output or if he simply
  // wants to output based on C->SizeFormat (forcedPrecision = SF_AUTO)
  if(forcedPrecision == SF_AUTO)
    forcedPrecision = C->SizeFormat;

  // we check what SizeFormat the user has choosen
  switch(forcedPrecision)
  {
    // the precision modes use sizes as base of 2
    enum { KB = 1024, MB = 1024 * 1024, GB = 1024 * 1024 * 1024 };

    /*
    ** ONE Precision mode
    ** This will result in the following output:
    ** 1.2 GB - 12.3 MB - 123.4 KB - 1234 B
    */
    case SF_1PREC:
    {
      if(size < KB)       snprintf(buf, buflen, "%ld B", size);
      else if(size < MB)  snprintf(buf, buflen, "%.1f KB", dsize/KB);
      else if(size < GB)  snprintf(buf, buflen, "%.1f MB", dsize/MB);
      else                snprintf(buf, buflen, "%.1f GB", dsize/GB);

      if((buf = strchr(buf, '.'))) *buf = *dp;
    }
    break;

    /*
    ** TWO Precision mode
    ** This will result in the following output:
    ** 1.23 GB - 12.34 MB - 123.45 KB - 1234 B
    */
    case SF_2PREC:
    {
      if(size < KB)       snprintf(buf, buflen, "%ld B", size);
      else if(size < MB)  snprintf(buf, buflen, "%.2f KB", dsize/KB);
      else if(size < GB)  snprintf(buf, buflen, "%.2f MB", dsize/MB);
      else                snprintf(buf, buflen, "%.2f GB", dsize/GB);

      if((buf = strchr(buf, '.'))) *buf = *dp;
    }
    break;

    /*
    ** THREE Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.345 MB - 123.456 KB - 1234 B
    */
    case SF_3PREC:
    {
      if(size < KB)       snprintf(buf, buflen, "%ld B", size);
      else if(size < MB)  snprintf(buf, buflen, "%.3f KB", dsize/KB);
      else if(size < GB)  snprintf(buf, buflen, "%.3f MB", dsize/MB);
      else                snprintf(buf, buflen, "%.3f GB", dsize/GB);

      if((buf = strchr(buf, '.'))) *buf = *dp;
    }
    break;

    /*
    ** MIXED Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.34 MB - 123.4 KB - 1234 B
    */
    case SF_MIXED:
    {
      if(size < KB)       snprintf(buf, buflen, "%ld B", size);
      else if(size < MB)  snprintf(buf, buflen, "%.1f KB", dsize/KB);
      else if(size < GB)  snprintf(buf, buflen, "%.2f MB", dsize/MB);
      else                snprintf(buf, buflen, "%.3f GB", dsize/GB);

      if((buf = strchr(buf, '.'))) *buf = *dp;
    }
    break;

    /*
    ** STANDARD mode
    ** This will result in the following output:
    ** 1,234,567 (bytes)
    */
    case SF_AUTO:
    default:
    {
      const char *gs = G->Locale ? (const char *)G->Locale->loc_GroupSeparator : ",";

      // as we just split the size to another value, we redefine the KB/MB/GB values to base 10 variables
      enum { KB = 1000, MB = 1000 * 1000, GB = 1000 * 1000 * 1000 };

      if(size < KB)      snprintf(buf, buflen, "%ld", size);
      else if(size < MB) snprintf(buf, buflen, "%ld%s%03ld", size/KB, gs, size%KB);
      else if(size < GB) snprintf(buf, buflen, "%ld%s%03ld%s%03ld", size/MB, gs, (size%MB)/KB, gs, size%KB);
      else               snprintf(buf, buflen, "%ld%s%03ld%s%03ld%s%03ld", size/GB, gs, (size%GB)/MB, gs, (size%MB)/KB, gs, size%KB);
    }
    break;
  }

  LEAVE();
}
///
/// MailExists
//  Checks if a message still exists
BOOL MailExists(struct Mail *mailptr, struct Folder *folder)
{
  BOOL exists = FALSE;

  ENTER();

  if(isVirtualMail(mailptr))
  {
    exists = TRUE;
  }
  else
  {
    struct Mail *work;

    if(folder == NULL)
      folder = mailptr->Folder;

    for(work = folder->Messages; work; work = work->Next)
    {
      if(work == mailptr)
      {
        exists = TRUE;
        break;
      }
    }
  }

  RETURN(exists);
  return exists;
}
///
/// DisplayMailList
//  Lists folder contents in the message listview
void DisplayMailList(struct Folder *fo, Object *lv)
{
  struct Mail **array;
  int lastActive;

  ENTER();

  lastActive = fo->LastActive;

  if((array = (struct Mail **)calloc(fo->Total + 1, sizeof(struct Mail *))) != NULL)
  {
    struct Mail *work;
    struct Mail **arrPtr = array;

    BusyText(tr(MSG_BusyDisplayingList), "");
    for(work = fo->Messages; work; work = work->Next)
    {
      *arrPtr++ = work;
    }

    // We do not encapsulate this Clear&Insert with a NList_Quiet because
    // this will speed up the Insert with about 3-4 seconds for ~6000 items
    DoMethod(lv, MUIM_NList_Clear);
    DoMethod(lv, MUIM_NList_Insert, array, fo->Total, MUIV_NList_Insert_Sorted,
                 C->AutoColumnResize ? MUIF_NONE : MUIV_NList_Insert_Flag_Raw);

    free(array);
    BusyEnd();
  }

  // Now we have to recover the LastActive or otherwise it will be -1 later
  fo->LastActive = lastActive;

  LEAVE();
}
///
/// AddMailToList
//  Adds a message to a folder
struct Mail *AddMailToList(struct Mail *mail, struct Folder *folder)
{
  struct Mail *new;

  ENTER();

  if((new = memdup(mail, sizeof(struct Mail))) != NULL)
  {
    new->Folder = folder;

    // lets add the new Message to our message list
    new->Next = folder->Messages;
    folder->Messages = new;

    // lets summarize the stats
    folder->Total++;
    folder->Size += mail->Size;

    if(hasStatusNew(mail))
      folder->New++;

    if(!hasStatusRead(mail))
      folder->Unread++;

    MA_ExpireIndex(folder);
  }

  RETURN(new);
  return new;
}
///
/// RemoveMailFromList
//  Removes a message from a folder
void RemoveMailFromList(struct Mail *mail, BOOL closeWindows)
{
  struct Folder *folder = mail->Folder;

  ENTER();

  // now we remove the mail from main mail
  // listviews in case the folder of it is the
  // currently active one.
  if(folder == FO_GetCurrentFolder())
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RemoveMail, mail);

  // remove the mail from the search window's mail list as well, if the
  // search window exists at all
  if(G->FI != NULL)
    DoMethod(G->FI->GUI.LV_MAILS, MUIM_MainMailList_RemoveMail, mail);

  // lets decrease the folder statistics first
  folder->Total--;
  folder->Size -= mail->Size;

  if(hasStatusNew(mail))
    folder->New--;

  if(!hasStatusRead(mail))
    folder->Unread--;

  // remove the mail from the folderlist now
  MyRemove(&(folder->Messages), mail);

  // then we have to mark the folder index as expired so
  // that it will be saved next time.
  MA_ExpireIndex(folder);

  // Now we check if there is any read window with that very same
  // mail currently open and if so we have to close it.
  if(IsListEmpty((struct List *)&G->readMailDataList) == FALSE)
  {
    // search through our ReadDataList
    struct MinNode *curNode;

    for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct ReadMailData *rmData = (struct ReadMailData *)curNode;

      if(rmData->mail == mail)
      {
        if(closeWindows && rmData->readWindow != NULL)
        {
          // Just ask the window to close itself, this will effectively clear the pointer.
          // We cannot set the attribute directly, because a DoMethod() call is synchronous
          // and then the read window would modify the list we are currently walking through
          // by calling CleanupReadMailData(). Hence we just let the application do the dirty
          // work as soon as it has the possibility to do that, but not before this loop is
          // finished. This works, because the ReadWindow class catches any modification to
          // MUIA_Window_CloseRequest itself. A simple set(win, MUIA_Window_Open, FALSE) would
          // visibly close the window, but it would not invoke the associated hook which gets
          // invoked when you close the window by clicking on the close gadget.
          DoMethod(G->App, MUIM_Application_PushMethod, rmData->readWindow, 3, MUIM_Set, MUIA_Window_CloseRequest, TRUE);
        }
        else
        {
          // Just clear pointer to this mail if we don't want to close the window or if
          // there is no window to close at all.
          rmData->mail = NULL;
        }
      }
    }
  }

  // and last, but not least we have to free the mail
  free(mail);

  LEAVE();
}
///
/// ClearMailList
//  Removes all messages from a folder
void ClearMailList(struct Folder *folder, BOOL resetstats)
{
  struct Mail *mail;
  struct Mail *next;

  ENTER();

  for(mail = folder->Messages; mail != NULL; mail = next)
  {
    next = mail->Next;

    // Now we check if there is any read window with that very same
    // mail currently open and if so we have to clean it.
    if(IsListEmpty((struct List *)&G->readMailDataList) == FALSE)
    {
      // search through our ReadDataList
      struct MinNode *curNode;

      for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        struct ReadMailData *rmData = (struct ReadMailData *)curNode;

        if(rmData->mail == mail)
          CleanupReadMailData(rmData, TRUE);
      }
    }

    // free the mail pointer
    free(mail);
  }

  if(resetstats)
  {
    folder->Total = 0;
    folder->New = 0;
    folder->Unread = 0;
    folder->Size = 0;
  }

  folder->Messages = NULL;

  LEAVE();
}
///
/// GetPackMethod
//  Returns packer type and efficiency
static BOOL GetPackMethod(enum FolderMode fMode, char **method, int *eff)
{
  BOOL result = TRUE;

  ENTER();

  switch(fMode)
  {
    case FM_XPKCOMP:
    {
      *method = C->XPKPack;
      *eff = C->XPKPackEff;
    }
    break;

    case FM_XPKCRYPT:
    {
      *method = C->XPKPackEncrypt;
      *eff = C->XPKPackEncryptEff;
    }
    break;

    default:
    {
      *method = NULL;
      *eff = 0;
      result = FALSE;
    }
    break;
  }

  RETURN(result);
  return result;
}
///
/// CompressMailFile
//  Shrinks a message file
static BOOL CompressMailFile(char *src, char *dst, char *passwd, char *method, int eff)
{
  long error = -1;

  ENTER();

  D(DBF_XPK, "CompressMailFile: %08lx - [%s] -> [%s] - [%s] - [%s] - %ld", XpkBase, src, dst, passwd, method, eff);

  if(XpkBase != NULL)
  {
    error = XpkPackTags(XPK_InName,      src,
                        XPK_OutName,     dst,
                        XPK_Password,    passwd,
                        XPK_PackMethod,  method,
                        XPK_PackMode,    eff,
                        TAG_DONE);

    #if defined(DEBUG)
    if(error != XPKERR_OK)
    {
      char buf[1024];

      XpkFault(error, NULL, buf, sizeof(buf));

      E(DBF_XPK, "XpkPackTags() returned an error %ld: '%s'", error, buf);
    }
    #endif
  }

  RETURN((BOOL)(error == XPKERR_OK));
  return (BOOL)(error == XPKERR_OK);
}
///
/// UncompressMailFile
//  Expands a compressed message file
static BOOL UncompressMailFile(const char *src, const char *dst, const char *passwd)
{
  long error = -1;

  ENTER();

  D(DBF_XPK, "UncompressMailFile: %08lx - [%s] -> [%s] - [%s]", XpkBase, src, dst, passwd);

  if(XpkBase != NULL)
  {
    error = XpkUnpackTags(XPK_InName,    src,
                          XPK_OutName,   dst,
                          XPK_Password,  passwd,
                          TAG_DONE);

    #if defined(DEBUG)
    if(error != XPKERR_OK)
    {
      char buf[1024];

      XpkFault(error, NULL, buf, sizeof(buf));

      E(DBF_XPK, "XpkUnPackTags() returned an error %ld: '%s'", error, buf);
    }
    #endif
  }

  RETURN((BOOL)(error == XPKERR_OK));
  return (BOOL)(error == XPKERR_OK);
}
///
/// TransferMailFile
//  Copies or moves a message file, handles compression
int TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder)
{
  char *pmeth;
  char srcbuf[SIZE_PATHFILE];
  char dstbuf[SIZE_PATHFILE];
  char dstFileName[SIZE_MFILE];
  struct Folder *srcfolder = mail->Folder;
  int peff = 0;
  enum FolderMode srcMode = srcfolder->Mode;
  enum FolderMode dstMode = dstfolder->Mode;
  char *srcpw = srcfolder->Password;
  char *dstpw = dstfolder->Password;
  int success = -1;

  ENTER();

  D(DBF_UTIL, "TransferMailFile: %ld->%ld [%s]->[%s]", srcMode, dstMode, mail->MailFile, GetFolderDir(dstfolder));

  if(MA_GetIndex(srcfolder) && MA_GetIndex(dstfolder))
  {
    BOOL counterExceeded = FALSE;

    // get some information we require
    GetPackMethod(dstMode, &pmeth, &peff);
    GetMailFile(srcbuf, srcfolder, mail);

    // check if we can just take the exactly same filename in the destination
    // folder or if we require to increase the mailfile counter to make it
    // unique
    strlcpy(dstFileName, mail->MailFile, sizeof(dstFileName));

    AddPath(dstbuf, GetFolderDir(dstfolder), dstFileName, sizeof(dstbuf));
    if(FileExists(dstbuf))
    {
      int mCounter = atoi(&dstFileName[13]);

      do
      {
        if(mCounter < 1 || mCounter >= 999)
          // no more numbers left
          // now we have to leave this function
          counterExceeded = TRUE;
        else
        {
          mCounter++;

          snprintf(&dstFileName[13], sizeof(dstFileName)-13, "%03d", mCounter);
          dstFileName[16] = ','; // restore it

          AddPath(dstbuf, GetFolderDir(dstfolder), dstFileName, sizeof(dstbuf));
        }
      }
      while(counterExceeded == FALSE && FileExists(dstbuf));

      if(counterExceeded == FALSE)
      {
        // if we end up here we finally found a new mailfilename which we can use, so
        // lets copy it to our MailFile variable
        strlcpy(mail->MailFile, dstFileName, sizeof(mail->MailFile));
      }
    }

    if(counterExceeded == FALSE)
    {
      // now that we have the source and destination filename
      // we can go and do the file operation depending on some data we
      // acquired earlier
      if((srcMode == dstMode && srcMode <= FM_SIMPLE) ||
         (srcMode <= FM_SIMPLE && dstMode <= FM_SIMPLE))
      {
        if(copyit)
          success = CopyFile(dstbuf, 0, srcbuf, 0) ? 1 : -1;
        else
          success = MoveFile(srcbuf, dstbuf) ? 1 : -1;
      }
      else if(isXPKFolder(srcfolder))
      {
        if(isXPKFolder(dstfolder) == FALSE)
        {
          // if we end up here the source folder is a compressed folder but the
          // destination one not. so lets uncompress it
          success = UncompressMailFile(srcbuf, dstbuf, srcpw) ? 1 : -2;
          if(success > 0 && !copyit)
            success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
        }
        else
        {
          // here the source folder is a compressed+crypted folder and the
          // destination one also, so we have to uncompress the file to a
          // temporarly file and compress it immediatly with the destination
          // password again.
          struct TempFile *tf;

          if((tf = OpenTempFile(NULL)) != NULL)
          {
            success = UncompressMailFile(srcbuf, tf->Filename, srcpw) ? 1 : -2;
            if(success > 0)
            {
              // compress it immediatly again
              success = CompressMailFile(tf->Filename, dstbuf, dstpw, pmeth, peff) ? 1 : -2;
              if(success > 0 && !copyit)
                success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
            }

            CloseTempFile(tf);
          }
        }
      }
      else
      {
        if(isXPKFolder(dstfolder))
        {
          // here the source folder is not compressed, but the destination one
          // so we compress the file in the destionation folder now
          success = CompressMailFile(srcbuf, dstbuf, dstpw, pmeth, peff) ? 1 : -2;
          if(success > 0 && !copyit)
            success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
        }
        else
          // if we end up here then there is something seriously wrong
          success = -3;
      }
    }
  }

  RETURN(success);
  return success;
}
///
/// RepackMailFile
//  (Re/Un)Compresses a message file
//  Note: If dstMode is -1 and passwd is NULL, then this function packs
//        the current mail. It will assume it is plaintext and needs to be packed now
BOOL RepackMailFile(struct Mail *mail, enum FolderMode dstMode, char *passwd)
{
  char *pmeth = NULL;
  char srcbuf[SIZE_PATHFILE];
  char dstbuf[SIZE_PATHFILE];
  struct Folder *folder;
  int peff = 0;
  enum FolderMode srcMode;
  BOOL success = FALSE;

  ENTER();

  folder = mail->Folder;
  srcMode = folder->Mode;

  // if this function was called with dstxpk=-1 and passwd=NULL then
  // we assume we need to pack the file from plain text to the currently
  // selected pack method of the folder
  if((LONG)dstMode == -1 && passwd == NULL)
  {
    srcMode = FM_NORMAL;
    dstMode = folder->Mode;
    passwd  = folder->Password;
  }

  MA_GetIndex(folder);
  GetMailFile(srcbuf, folder, mail);
  GetPackMethod(dstMode, &pmeth, &peff);
  snprintf(dstbuf, sizeof(dstbuf), "%s.tmp", srcbuf);

  SHOWSTRING(DBF_UTIL, srcbuf);

  if((srcMode == dstMode && srcMode <= FM_SIMPLE) ||
     (srcMode <= FM_SIMPLE && dstMode <= FM_SIMPLE))
  {
    // the FolderModes are the same so lets do nothing
    success = TRUE;

    D(DBF_UTIL, "repack not required.");
  }
  else if(srcMode > FM_SIMPLE)
  {
    if(dstMode <= FM_SIMPLE)
    {
      D(DBF_UTIL, "uncompressing");

      // if we end up here the source folder is a compressed folder so we
      // have to just uncompress the file
      if(UncompressMailFile(srcbuf, dstbuf, folder->Password) &&
         DeleteFile(srcbuf) != 0)
      {
        if(RenameFile(dstbuf, srcbuf) != 0)
          success = TRUE;
      }
    }
    else
    {
      // if we end up here, the source folder is a compressed+crypted one and
      // the destination mode also
      D(DBF_UTIL, "uncompressing/recompress");

      if(UncompressMailFile(srcbuf, dstbuf, folder->Password) &&
         CompressMailFile(dstbuf, srcbuf, passwd, pmeth, peff))
      {
        if(DeleteFile(dstbuf) != 0)
          success = TRUE;
      }
    }
  }
  else
  {
    if(dstMode > FM_SIMPLE)
    {
      D(DBF_UTIL, "compressing");

      // here the source folder is not compressed, but the destination mode
      // signals to compress it
      if(CompressMailFile(srcbuf, dstbuf, passwd, pmeth, peff) &&
         DeleteFile(srcbuf) != 0)
      {
        success = RenameFile(dstbuf, srcbuf);
      }
    }
  }

  MA_UpdateMailFile(mail);

  RETURN(success);
  return success;
}
///
/// DoPack
//  Compresses a file
BOOL DoPack(char *file, char *newfile, struct Folder *folder)
{
  char *pmeth = NULL;
  int peff = 0;
  BOOL result = FALSE;

  ENTER();

  if(GetPackMethod(folder->Mode, &pmeth, &peff))
  {
    if(CompressMailFile(file, newfile, folder->Password, pmeth, peff))
    {
      if(DeleteFile(file) != 0)
      {
        result = TRUE;
      }
    }
  }

  RETURN(result);
  return result;
}
///
/// StartUnpack
//  Unpacks a file to a temporary file
char *StartUnpack(const char *file, char *newfile, const struct Folder *folder)
{
  FILE *fh;
  char *result = NULL;

  ENTER();

  if((fh = fopen(file, "r")))
  {
    BOOL xpk = FALSE;

    // check if the source file is really XPK compressed or not.
    if(fgetc(fh) == 'X' && fgetc(fh) == 'P' && fgetc(fh) == 'K')
      xpk = TRUE;

    fclose(fh);
    fh = NULL;

    // now we compose a temporary filename and start
    // uncompressing the source file into it.
    if(xpk)
    {
      char nfile[SIZE_FILE];

      snprintf(nfile, sizeof(nfile), "YAMu%08lx.unp", GetUniqueID());
      AddPath(newfile, C->TempDir, nfile, SIZE_PATHFILE);

      // check that the destination filename
      // doesn't already exist
      if(FileExists(newfile) == FALSE && UncompressMailFile(file, newfile, folder ? folder->Password : ""))
        result = newfile;
    }
    else
    {
      strcpy(newfile, file);
      result = newfile;
    }
  }

  RETURN(result);
  return result;
}
///
/// FinishUnpack
//  Deletes temporary unpacked file
void FinishUnpack(char *file)
{
  char ext[SIZE_FILE];

  ENTER();

  // we just delete if this is really related to a unpack file
  stcgfe(ext, file);
  if(strcmp(ext, "unp") == 0)
  {
    if(IsListEmpty((struct List *)&G->readMailDataList) == FALSE)
    {
      // search through our ReadDataList
      struct MinNode *curNode;
      for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        struct ReadMailData *rmData = (struct ReadMailData *)curNode;

        // check if the file is still in use and if so we quit immediately
        // leaving the file untouched.
        if(stricmp(file, rmData->readFile) == 0)
        {
          LEAVE();
          return;
        }
      }
    }

    if(DeleteFile(file) == 0)
      AddZombieFile(file);
  }

  LEAVE();
}
///

/*** Editor related ***/
/// EditorToFile
//  Saves contents of a texteditor object to a file
BOOL EditorToFile(Object *editor, char *file)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(file, "w")) != NULL)
  {
    char *text = (char *)DoMethod((Object *)editor, MUIM_TextEditor_ExportText);

    // write out the whole text to the file
    if(fwrite(text, strlen(text), 1, fh) == 1)
      result = TRUE;

    FreeVec(text); // use FreeVec() because TextEditor.mcc uses AllocVec()
    fclose(fh);
  }

  RETURN(result);
  return result;
}
///
/// FileToEditor
//  Loads a file into a texteditor object
BOOL FileToEditor(char *file, Object *editor, BOOL changed)
{
  char *text;
  BOOL res = FALSE;

  ENTER();

  if((text = FileToBuffer(file)) != NULL)
  {
    char *parsedText;

    // parse the text and do some highlighting and stuff
    if((parsedText = ParseEmailText(text, FALSE)) != NULL)
    {
      // set the new text and tell the editor that its content has changed
      xset(editor, MUIA_TextEditor_Contents, parsedText,
                   MUIA_TextEditor_HasChanged, changed);
      free(parsedText);

      res = TRUE;
    }

    free(text);
  }

  RETURN(res);
  return res;
}
///

/*** Hooks ***/
/// GeneralDesFunc
//  General purpose destruction hook
HOOKPROTONHNO(GeneralDesFunc, long, void *entry)
{
  free(entry);

  return 0;
}
MakeHook(GeneralDesHook, GeneralDesFunc);
///
/// PO_SetPublicKey
//  Copies public PGP key from list to string gadget
HOOKPROTONH(PO_SetPublicKey, void, Object *pop, Object *string)
{
  char *var = NULL;

  ENTER();

  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
  if(var != NULL)
  {
    char buf[8 + 2 + 1]; // 8 chars + 2 extra chars required.

    strlcpy(buf, "0x", sizeof(buf));
    strlcat(buf, var, sizeof(buf));

    setstring(string, buf);
  }

  LEAVE();
}
MakeHook(PO_SetPublicKeyHook, PO_SetPublicKey);
///
/// PO_ListPublicKeys
//  Lists keys of public PGP keyring in a popup window
HOOKPROTONH(PO_ListPublicKeys, long, APTR pop, APTR string)
{
  APTR secret;
  char buf[SIZE_LARGE], *str, p;
  int retc, keys = 0;
  FILE *fp;

  ENTER();

  secret = str = (char *)xget(pop, MUIA_UserData);
  if(G->PGPVersion == 5)
  {
    retc = PGPCommand("pgpk", "-l +language=us", KEEPLOG);
  }
  else
  {
    strlcpy(buf, "-kv  ", sizeof(buf));
    if(secret != NULL)
    {
      GetVar("PGPPATH", &buf[4], sizeof(buf) - 4, 0);
      if((p = buf[strlen(buf) - 1]) != ':' && p != '/')
        strlcat(buf, "/", sizeof(buf));

      strlcat(buf, "secring.pgp", sizeof(buf));
    }
    retc = PGPCommand("pgp", buf, KEEPLOG);
  }

  if(retc == 0 && (fp = fopen(PGPLOGFILE, "r")) != NULL)
  {
    str = (char *)xget(string, MUIA_String_Contents);
    DoMethod(pop, MUIM_List_Clear);

    setvbuf(fp, NULL, _IOFBF, SIZE_FILEBUF);

    while(GetLine(fp, buf, sizeof(buf)) != NULL)
    {
      char entry[SIZE_DEFAULT];

      memset(entry, 0, SIZE_DEFAULT);
      if(G->PGPVersion == 5)
      {
        if(!strncmp(buf, "sec", 3) || (!strncmp(&buf[1], "ub", 2) && secret == NULL))
        {
          memcpy(entry, &buf[12], 8);

          while(GetLine(fp, buf, sizeof(buf)) != NULL)
          {
            if(!strncmp(buf, "uid", 3))
            {
              strlcat(entry, &buf[4], sizeof(entry) - 9);
              break;
            }
          }
        }
      }
      else
      {
        if(buf[9] == '/' && buf[23] == '/')
        {
          memcpy(entry, &buf[10], 8);
          strlcat(entry, &buf[29], sizeof(entry) - 8);
        }
      }
      if(entry[0] != '\0')
      {
        DoMethod(pop, MUIM_List_InsertSingle, entry, MUIV_List_Insert_Bottom);
        if(!strncmp(entry, str, 8))
          set(pop, MUIA_List_Active, keys);
        keys++;
      }
    }
    fclose(fp);

    if(DeleteFile(PGPLOGFILE) == 0)
      AddZombieFile(PGPLOGFILE);
  }
  if(keys == 0)
    ER_NewError(tr(MSG_ER_NoPublicKeys), "", "");

  RETURN(keys > 0);
  return keys > 0;
}
MakeHook(PO_ListPublicKeysHook, PO_ListPublicKeys);
///

/*** MUI related ***/
/// ShortCut
//  Finds keyboard shortcut in text label
char ShortCut(const char *label)
{
  char scut = '\0';
  char *ptr;

  ENTER();

  if((ptr = strchr(label, '_')) != NULL)
    scut = (char)ToLower((ULONG)(*++ptr));

  RETURN(scut);
  return scut;
}
///
/// MakeCycle
//  Creates a MUI cycle object
Object *MakeCycle(const char *const *labels, const char *label)
{
  return CycleObject,
           MUIA_CycleChain,    TRUE,
           MUIA_Font,          MUIV_Font_Button,
           MUIA_Cycle_Entries, labels,
           MUIA_ControlChar,   ShortCut(label),
         End;
}
///
/// MakeButton
//  Creates a MUI button
Object *MakeButton(const char *txt)
{
   Object *obj;

   if((obj = MUI_MakeObject(MUIO_Button,txt)) != NULL)
     set(obj, MUIA_CycleChain, TRUE);

   return obj;
}
///
/// MakeCheck
//  Creates a MUI checkmark object
Object *MakeCheck(const char *label)
{
  return ImageObject,
           ImageButtonFrame,
           MUIA_InputMode   , MUIV_InputMode_Toggle,
           MUIA_Image_Spec  , MUII_CheckMark,
           MUIA_Background  , MUII_ButtonBack,
           MUIA_ShowSelState, FALSE,
           MUIA_ControlChar , ShortCut(label),
           MUIA_CycleChain  , TRUE,
         End;
}
///
/// MakeCheckGroup
//  Creates a labelled MUI checkmark object
Object *MakeCheckGroup(Object **check, const char *label)
{
   return HGroup,
            Child, *check = MakeCheck(label),
            Child, Label1(label),
            Child, HSpace(0),
          End;
}
///
/// MakeString
//  Creates a MUI string object
Object *MakeString(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,      maxlen,
           MUIA_String_AdvanceOnCR, TRUE,
           MUIA_ControlChar,        ShortCut(label),
           MUIA_CycleChain,         TRUE,
         End;
}
///
/// MakePassString
//  Creates a MUI string object with hidden text
Object *MakePassString(const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       SIZE_PASSWORD,
           MUIA_String_Secret,       TRUE,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
         End;
}
///
/// MakeInteger
//  Creates a MUI string object for numeric input
Object *MakeInteger(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       maxlen+1,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
           MUIA_String_Integer,      0,
           MUIA_String_Accept,       "0123456789",
         End;
}
///
/// MakePGPKeyList
//  Creates a PGP id popup list
Object *MakePGPKeyList(Object **st, BOOL secret, const char *label)
{
  Object *po, *lv;

  if ((po = PopobjectObject,
        MUIA_Popstring_String, *st = MakeString(SIZE_DEFAULT, label),
        MUIA_Popstring_Button, PopButton(MUII_PopUp),
        MUIA_Popobject_StrObjHook, &PO_ListPublicKeysHook,
        MUIA_Popobject_ObjStrHook, &PO_SetPublicKeyHook,
        MUIA_Popobject_WindowHook, &PO_WindowHook,
        MUIA_Popobject_Object, lv = ListviewObject,
           MUIA_UserData, secret,
           MUIA_Listview_List, ListObject,
              InputListFrame,
              MUIA_List_AdjustWidth, TRUE,
              MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
              MUIA_List_DestructHook, MUIV_List_DestructHook_String,
           End,
        End,
     End))
  {
    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
  }

  return po;
}
///
/// MakeAddressField
//  Creates a recipient field
Object *MakeAddressField(Object **string, const char *label, const Object *help, int abmode, int winnum, ULONG flags)
{
  Object *obj;
  Object *bt_adr;

  ENTER();

  if((obj = HGroup,

    GroupSpacing(1),
    Child, *string = RecipientstringObject,
      MUIA_CycleChain,                          TRUE,
      MUIA_String_AdvanceOnCR,                  TRUE,
      MUIA_Recipientstring_ResolveOnCR,         TRUE,
      MUIA_Recipientstring_MultipleRecipients,  isFlagSet(flags, AFF_ALLOW_MULTI),
      MUIA_BetterString_NoShortcuts,            isFlagSet(flags, AFF_EXTERNAL_SHORTCUTS),
      MUIA_ControlChar,                         ShortCut(label),
    End,
    Child, bt_adr = PopButton(MUII_PopUp),

  End))
  {
    SetHelp(*string,help);
    SetHelp(bt_adr, MSG_HELP_WR_BT_ADR);

    if(abmode == ABM_CONFIG)
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
      DoMethod(*string, MUIM_Notify, MUIA_Recipientstring_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
    }
    else
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnum);
      DoMethod(*string, MUIM_Notify, MUIA_Recipientstring_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnum);
    }

    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime,  bt_adr, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN(obj);
  return obj;
}
///
/// MakeNumeric
//  Creates a MUI numeric slider
Object *MakeNumeric(int min, int max, BOOL percent)
{
  return NumericbuttonObject,
           MUIA_Numeric_Min, min,
           MUIA_Numeric_Max, max,
           MUIA_Numeric_Format, percent ? "%ld%%" : "%ld",
           MUIA_CycleChain, TRUE,
         End;
}
///
/// GetMUIInteger
//  Returns the numeric value of a MUI string object
int GetMUIInteger(Object *obj)
{
   return (int)xget(obj,MUIA_String_Integer);
}
///
/// GetMUICheck
//  Returns the value of a MUI checkmark object
BOOL GetMUICheck(Object *obj)
{
   return (BOOL)xget(obj, MUIA_Selected);
}
///
/// GetMUICycle
//  Returns the value of a MUI cycle object
int GetMUICycle(Object *obj)
{
   return (int)xget(obj, MUIA_Cycle_Active);
}
///
/// GetMUIRadio
//  Returns the value of a MUI radio object
int GetMUIRadio(Object *obj)
{
   return (int)xget(obj, MUIA_Radio_Active);
}
///
/// GetMUINumer
//  Returns the value of a MUI numeric slider
int GetMUINumer(Object *obj)
{
   return (int)xget(obj, MUIA_Numeric_Value);
}
///
/// SafeOpenWindow
//  Tries to open a window
BOOL SafeOpenWindow(Object *obj)
{
  BOOL success = FALSE;

  ENTER();

  if(obj != NULL)
  {
    // make sure we open the window object
    set(obj, MUIA_Window_Open, TRUE);

    // now we check whether the window was successfully
    // open or the application has been in iconify state
    if(xget(obj, MUIA_Window_Open) == TRUE ||
       xget(_app(obj), MUIA_Application_Iconified) == TRUE)
    {
      success = TRUE;
    }
  }

  if(success == FALSE)
  {
    // otherwise we perform a DisplayBeep()
    DisplayBeep(NULL);
  }

  RETURN(success);
  return success;
}
///
/// DisposeModule
// Free resources of a MUI window
void DisposeModule(void *modptr)
{
  struct UniversalClassData **module = (struct UniversalClassData **)modptr;

  ENTER();

  if(*module != NULL)
  {
    Object *window = (*module)->GUI.WI;

    D(DBF_GUI, "removing window from app: %08lx", window);

    // close the window
    set(window, MUIA_Window_Open, FALSE);

    // remove the window from our app
    DoMethod(G->App, OM_REMMEMBER, window);

    // dispose the window object
    MUI_DisposeObject(window);

    free(*module);
    *module = NULL;
  }

  LEAVE();
}
HOOKPROTONHNO(DisposeModuleFunc, void, void **arg)
{
  DisposeModule(arg[0]);
}
MakeHook(DisposeModuleHook,DisposeModuleFunc);
///
/// LoadLayout
//  Loads column widths from ENV:MUI/YAM.cfg
void LoadLayout(void)
{
  const char *ls;
  char *endptr;

  ENTER();

  // Load the application configuration from the ENV: directory.
  DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENV);

  // we encode the different weight factors which are embeeded in a dummy string
  // gadgets:
  //
  // 0:  Horizontal weight of left foldertree in main window.
  // 1:  Horizontal weight of right maillistview in main window.
  // 2:  Vertical weight of top headerlistview in read window
  // 3:  Vertical weight of bottom texteditor field in read window
  // 4:  Horizontal weight of listview group in the glossary window
  // 5:  Horizontal weight of text group in the glossary window
  // 6:  Vertical weight of top right maillistview group in main window.
  // 7:  Vertical weight of bottom right embedded read pane object in the main window.
  // 8:  Vertical weight of top object (headerlist) of the embedded read pane
  // 9:  Vertical weight of bottom object (texteditor) of the embedded read pane
  // 10: Vertical weight of top object (headerlist) in a read window
  // 11: Vertical weight of bottom object (texteditor) in a read window

  if(!*(ls = (STRPTR)xget(G->MA->GUI.ST_LAYOUT, MUIA_String_Contents)))
    ls = "30 100 25 100 30 100 25 100 5 100 5 100";

  // lets get the numbers for each weight factor out of the contents
  // of the fake string gadget
  G->Weights[0] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[0] = 30;

  ls = endptr;
  G->Weights[1] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[1] = 100;

  ls = endptr;
  G->Weights[2] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[2] = 25;

  ls = endptr;
  G->Weights[3] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[3] = 100;

  ls = endptr;
  G->Weights[4] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[4] = 30;

  ls = endptr;
  G->Weights[5] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[5] = 100;

  ls = endptr;
  G->Weights[6] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[6] = 25;

  ls = endptr;
  G->Weights[7] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[7] = 100;

  ls = endptr;
  G->Weights[8] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[8] = 5;

  ls = endptr;
  G->Weights[9] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[9] = 100;

  ls = endptr;
  G->Weights[10] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[10] = 5;

  ls = endptr;
  G->Weights[11] = strtol(ls, &endptr, 10);
  if(endptr == NULL || endptr == ls)
    G->Weights[11] = 100;

  // lets set the weight factors to the corresponding GUI elements now
  // if they exist
  set(G->MA->GUI.LV_FOLDERS,  MUIA_HorizWeight, G->Weights[0]);
  set(G->MA->GUI.GR_MAILVIEW, MUIA_HorizWeight, G->Weights[1]);
  set(G->MA->GUI.PG_MAILLIST, MUIA_VertWeight,  G->Weights[6]);

  // if the embedded read pane is active we set its weight values
  if(C->EmbeddedReadPane)
  {
    xset(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_VertWeight,                 G->Weights[7],
                                         MUIA_ReadMailGroup_HGVertWeight, G->Weights[8],
                                         MUIA_ReadMailGroup_TGVertWeight, G->Weights[9]);
  }

  LEAVE();
}
///
/// SaveLayout
//  Saves column widths to ENV(ARC):MUI/YAM.cfg
void SaveLayout(BOOL permanent)
{
  char buf[SIZE_DEFAULT+1];

  ENTER();
  SHOWVALUE(DBF_UTIL, permanent);

  // we encode the different weight factors which are embeeded in a dummy string
  // gadgets:
  //
  // 0:  Horizontal weight of left foldertree in main window.
  // 1:  Horizontal weight of right maillistview in main window.
  // 2:  Vertical weight of top headerlistview in read window
  // 3:  Vertical weight of bottom texteditor field in read window
  // 4:  Horizontal weight of listview group in the glossary window
  // 5:  Horizontal weight of text group in the glossary window
  // 6:  Vertical weight of top right maillistview group in main window.
  // 7:  Vertical weight of bottom right embedded read pane object in the main window.
  // 8:  Vertical weight of top object (headerlist) of the embedded read pane
  // 9:  Vertical weight of bottom object (texteditor) of the embedded read pane
  // 10: Vertical weight of top object (headerlist) in a read window
  // 11: Vertical weight of bottom object (texteditor) in a read window
  snprintf(buf, sizeof(buf), "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", G->Weights[0],
                                                                                G->Weights[1],
                                                                                G->Weights[2],
                                                                                G->Weights[3],
                                                                                G->Weights[4],
                                                                                G->Weights[5],
                                                                                G->Weights[6],
                                                                                G->Weights[7],
                                                                                G->Weights[8],
                                                                                G->Weights[9],
                                                                                G->Weights[10],
                                                                                G->Weights[11]);

  setstring(G->MA->GUI.ST_LAYOUT, buf);
  DoMethod(G->App, MUIM_Application_Save, MUIV_Application_Save_ENV);

  // if we want to save to ENVARC:
  if(permanent == TRUE)
  {
    APTR oldWindowPtr;

    // this is for the people out there having their SYS: partition locked and whining about
    // YAM popping up a error requester upon the exit - so it`s their fault now if
    // the MUI objects aren`t saved correctly.
    oldWindowPtr = SetProcWindow((APTR)-1);

    DoMethod(G->App, MUIM_Application_Save, MUIV_Application_Save_ENVARC);

    // restore the old windowPtr
    SetProcWindow(oldWindowPtr);
  }

  LEAVE();
}
///
/// ConvertKey
//  Converts input event to key code
unsigned char ConvertKey(const struct IntuiMessage *imsg)
{
  struct InputEvent ie;
  unsigned char code = 0;

  ENTER();

  ie.ie_NextEvent    = NULL;
  ie.ie_Class        = IECLASS_RAWKEY;
  ie.ie_SubClass     = 0;
  ie.ie_Code         = imsg->Code;
  ie.ie_Qualifier    = imsg->Qualifier;
  ie.ie_EventAddress = (APTR *) *((ULONG *)imsg->IAddress);

  if(MapRawKey(&ie, (STRPTR)&code, 1, NULL) != 1)
    E(DBF_GUI, "MapRawKey retuned != 1");

  RETURN(code);
  return code;
}
///
/// isChildOfGroup
// return TRUE if the supplied child object is part of the supplied group
BOOL isChildOfGroup(Object *group, Object *child)
{
  BOOL isChild = FALSE;
  struct List *child_list;

  ENTER();

  // get the child list of the group object
  child_list = (struct List *)xget(group, MUIA_Group_ChildList);
  if(child_list != NULL)
  {
    Object *curchild;
    Object *cstate;

    // here we check whether the child is part of the supplied group
    cstate = (Object *)child_list->lh_Head;
    while((curchild = NextObject(&cstate)) != NULL)
    {
      if(curchild == child)
      {
        isChild = TRUE;
        break;
      }
    }
  }

  RETURN(isChild);
  return isChild;
}
///
/// isChildOfFamily
// return TRUE if the supplied child object is part of the supplied family/menu object
BOOL isChildOfFamily(Object *family, Object *child)
{
  BOOL isChild = FALSE;
  struct MinList *child_list;

  ENTER();

  // get the child list of the group object
  child_list = (struct MinList *)xget(family, MUIA_Family_List);
  if(child_list != NULL)
  {
    Object *curchild;
    Object *cstate;

    // here we check whether the child is part of the supplied group
    cstate = (Object *)child_list->mlh_Head;
    while((curchild = NextObject(&cstate)) != NULL)
    {
      if(curchild == child)
      {
        isChild = TRUE;
        break;
      }
    }
  }

  RETURN(isChild);
  return isChild;
}
///

/*** GFX related ***/
#if !defined(__amigaos4__)
/// struct LayerHookMsg
struct LayerHookMsg
{
  struct Layer *layer;
  struct Rectangle bounds;
  LONG offsetx;
  LONG offsety;
};

///
/// struct BltHook
struct BltHook
{
  struct Hook hook;
  struct BitMap maskBitMap;
  struct BitMap *srcBitMap;
  LONG srcx,srcy;
  LONG destx,desty;
};

///
/// MyBltMaskBitMap
static void MyBltMaskBitMap(const struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct BitMap *destBitMap, LONG xDest, LONG yDest, LONG xSize, LONG ySize, struct BitMap *maskBitMap)
{
  ENTER();

  BltBitMap(srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);
  BltBitMap(maskBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0xe2,~0,NULL);
  BltBitMap(srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);

  LEAVE();
}

///
/// BltMaskHook
HOOKPROTO(BltMaskFunc, void, struct RastPort *rp, struct LayerHookMsg *msg)
{
  struct BltHook *h = (struct BltHook*)hook;

  LONG width = msg->bounds.MaxX - msg->bounds.MinX+1;
  LONG height = msg->bounds.MaxY - msg->bounds.MinY+1;
  LONG offsetx = h->srcx + msg->offsetx - h->destx;
  LONG offsety = h->srcy + msg->offsety - h->desty;

  MyBltMaskBitMap(h->srcBitMap, offsetx, offsety, rp->BitMap, msg->bounds.MinX, msg->bounds.MinY, width, height, &h->maskBitMap);
}
MakeStaticHook(BltMaskHook, BltMaskFunc);

///
/// MyBltMaskBitMapRastPort
void MyBltMaskBitMapRastPort(struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct RastPort *destRP, LONG xDest, LONG yDest, LONG xSize, LONG ySize, ULONG minterm, APTR bltMask)
{
  ENTER();

  if(GetBitMapAttr(srcBitMap, BMA_FLAGS) & BMF_INTERLEAVED)
  {
    LONG src_depth = GetBitMapAttr(srcBitMap, BMA_DEPTH);
    struct Rectangle rect;
    struct BltHook hook;

    // Define the destination rectangle in the rastport
    rect.MinX = xDest;
    rect.MinY = yDest;
    rect.MaxX = xDest + xSize - 1;
    rect.MaxY = yDest + ySize - 1;

    // Initialize the hook
    InitHook(&hook.hook, BltMaskHook, NULL);
    hook.srcBitMap = srcBitMap;
    hook.srcx = xSrc;
    hook.srcy = ySrc;
    hook.destx = xDest;
    hook.desty = yDest;

    // Initialize a bitmap where all plane pointers points to the mask
    InitBitMap(&hook.maskBitMap, src_depth, GetBitMapAttr(srcBitMap, BMA_WIDTH), GetBitMapAttr(srcBitMap, BMA_HEIGHT));
    while(src_depth)
    {
      hook.maskBitMap.Planes[--src_depth] = bltMask;
    }

    // Blit onto the Rastport */
    DoHookClipRects(&hook.hook, destRP, &rect);
  }
  else
    BltMaskBitMapRastPort(srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm, bltMask);

  LEAVE();
}

///
#endif

/*** Miscellaneous stuff ***/
/// PGPGetPassPhrase
//  Asks user for the PGP passphrase
void PGPGetPassPhrase(void)
{
  char pgppass[SIZE_DEFAULT];

  ENTER();

  // check if a PGPPASS variable exists already
  if(GetVar("PGPPASS", pgppass, sizeof(pgppass), GVF_GLOBAL_ONLY) < 0)
  {
    // check if we really require to request a passphrase from
    // the user
    if(G->PGPPassPhrase[0] != '\0' &&
       C->PGPPassInterval > 0 && G->LastPGPUsage > 0 &&
       time(NULL)-G->LastPGPUsage <= (time_t)(C->PGPPassInterval*60))
    {
      // nothing
    }
    else
    {
      pgppass[0] = '\0';

      if(PassphraseRequest(pgppass, SIZE_DEFAULT, G->MA->GUI.WI) > 0)
        G->LastPGPUsage = time(NULL);
      else
        G->LastPGPUsage = 0;

      strlcpy(G->PGPPassPhrase, pgppass, sizeof(G->PGPPassPhrase));
    }

    // make sure we delete the passphrase variable immediately after
    // having processed the PGP command
    G->PGPPassVolatile = TRUE;

    // set a global PGPPASS variable, but do not write it
    // to ENVARC:
    SetVar("PGPPASS", G->PGPPassPhrase, -1, GVF_GLOBAL_ONLY);
  }
  else
  {
    W(DBF_MAIL, "ENV:PGPPASS already exists!");

    // don't delete env-variable on PGPClearPassPhrase()
    G->PGPPassVolatile = FALSE;

    // copy the content of the env variable to our
    // global passphrase variable
    strlcpy(G->PGPPassPhrase, pgppass, sizeof(G->PGPPassPhrase));
    G->LastPGPUsage = 0;
  }

  LEAVE();
}
///
/// PGPClearPassPhrase
//  Clears the ENV variable containing the PGP passphrase
void PGPClearPassPhrase(BOOL force)
{
  ENTER();

  if(G->PGPPassVolatile)
    DeleteVar("PGPPASS", GVF_GLOBAL_ONLY);

  if(force)
    G->PGPPassPhrase[0] = '\0';

  LEAVE();
}
///
/// PGPCommand
//  Launches a PGP command
int PGPCommand(const char *progname, const char *options, int flags)
{
  BPTR fhi;
  char command[SIZE_LARGE];
  int error = -1;

  ENTER();

  D(DBF_UTIL, "[%s] [%s] - flags: %ld", progname, options, flags);

  if((fhi = Open("NIL:", MODE_OLDFILE)))
  {
    BPTR fho;

    if((fho = Open("NIL:", MODE_NEWFILE)))
    {
      BusyText(tr(MSG_BusyPGPrunning), "");

      AddPath(command, C->PGPCmdPath, progname, sizeof(command));
      strlcat(command, " >" PGPLOGFILE " ", sizeof(command));
      strlcat(command, options, sizeof(command));

      // use SystemTags() for executing PGP
      error = SystemTags(command, SYS_Input,    fhi,
                                  SYS_Output,   fho,
                                  SYS_Asynch,   FALSE,
                                  #if defined(__amigaos4__)
                                  SYS_Error,    NULL,
                                  NP_Child,     TRUE,
                                  #endif
                                  NP_Name,      "YAM PGP process",
                                  NP_StackSize, C->StackSize,
                                  NP_WindowPtr, -1, // no requester at all
                                  TAG_DONE);

      BusyEnd();

      Close(fho);
    }

    Close(fhi);
  }

  if(error > 0 && !hasNoErrorsFlag(flags))
    ER_NewError(tr(MSG_ER_PGPreturnsError), command, PGPLOGFILE);

  if(error < 0)
    ER_NewError(tr(MSG_ER_PGPnotfound), C->PGPCmdPath);

  if(!error && !hasKeepLogFlag(flags))
  {
    if(DeleteFile(PGPLOGFILE) == 0)
      AddZombieFile(PGPLOGFILE);
  }

  RETURN(error);
  return error;
}
///
/// AppendToLogfile
//  Appends a line to the logfile
void AppendToLogfile(enum LFMode mode, int id, const char *text, ...)
{
  ENTER();

  // check the Logfile mode
  if(C->LogfileMode != LF_NONE &&
     (mode == LF_ALL || C->LogfileMode == mode))
  {
    // check if the event in question should really be logged or
    // not.
    if(C->LogAllEvents == TRUE || (id >= 30 && id <= 49))
    {
      FILE *fh;
      char logfile[SIZE_PATHFILE];
      char filename[SIZE_FILE];

      // if the user wants to split the logfile by date
      // we go and generate the filename now.
      if(C->SplitLogfile == TRUE)
      {
        struct ClockData cd;

        Amiga2Date(GetDateStamp(), &cd);
        snprintf(filename, sizeof(filename), "YAM-%s%d.log", months[cd.month-1], cd.year);
      }
      else
        strlcpy(filename, "YAM.log", sizeof(filename));

      // add the logfile path to the filename.
      AddPath(logfile, C->LogfilePath[0] != '\0' ? C->LogfilePath : G->ProgDir, filename, sizeof(logfile));

      // open the file handle in 'append' mode and output the
      // text accordingly.
      if((fh = fopen(logfile, "a")) != NULL)
      {
        char datstr[64];
        va_list args;

        DateStamp2String(datstr, sizeof(datstr), NULL, DSS_DATETIME, TZC_NONE);

        // output the header
        fprintf(fh, "%s [%02d] ", datstr, id);

        // compose the varags values
        va_start(args, text);
        vfprintf(fh, text, args);
        va_end(args);

        fprintf(fh, "\n");
        fclose(fh);
      }
    }
  }

  LEAVE();
}
///
/// Busy
//  Displays busy message
//  returns FALSE if the user pressed the stop button on an eventually active
//  BusyGauge. The calling method is therefore suggested to take actions to
//  stop its processing.
BOOL Busy(const char *text, const char *parameter, int cur, int max)
{
  // we can have different busy levels (defined BUSYLEVEL)
  static char infotext[BUSYLEVEL][SIZE_DEFAULT];
  BOOL result = TRUE;

  ENTER();

  if(text)
  {
    if(*text)
    {
      snprintf(infotext[BusyLevel], SIZE_DEFAULT, text, parameter);

      if(max > 0)
      {
        // initialize the InfoBar gauge and also make sure it
        // shows a stop gadget in case cur < 0
        if(G->MA)
          DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowGauge, infotext[BusyLevel], cur, max);

        // check if we are in startup phase so that we also
        // update the gauge elements of the About window
        if(G->InStartupPhase)
        {
          static char progressText[SIZE_DEFAULT];

          snprintf(progressText, sizeof(progressText), "%%ld/%d", max);

          DoMethod(G->SplashWinObject, MUIM_Splashwindow_StatusChange, infotext[BusyLevel], -1);
          DoMethod(G->SplashWinObject, MUIM_Splashwindow_ProgressChange, progressText, cur, max);
        }
      }
      else
      {
        // initialize the InfoBar infotext
        if(G->MA)
          DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowInfoText, infotext[BusyLevel]);
      }

      if(BusyLevel < BUSYLEVEL-1)
        BusyLevel++;
      else
        E(DBF_UTIL, "Error: reached highest BusyLevel!!!");
    }
    else
    {
      if(BusyLevel)
        BusyLevel--;

      if(G->MA)
      {
        if(BusyLevel <= 0)
          DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_HideBars);
        else
          DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowInfoText, infotext[BusyLevel-1]);
      }
    }
  }
  else
  {
    // If the text is NULL we just have to set the Gauge of the infoBar to the current
    // level
    if(BusyLevel > 0)
    {
      if(G->MA)
        result = DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowGauge, NULL, cur, max);

      if(G->InStartupPhase)
        DoMethod(G->SplashWinObject, MUIM_Splashwindow_ProgressChange, NULL, cur, -1);
    }
  }

  RETURN(result);
  return result;
}

///
/// DisplayAppIconStatistics
//  Calculates AppIconStatistic and update the AppIcon
void DisplayAppIconStatistics(void)
{
  static char apptit[SIZE_DEFAULT/2];
  struct Folder **flist;
  enum IconImages mode;
  int new_msg = 0;
  int unr_msg = 0;
  int tot_msg = 0;
  int snt_msg = 0;
  int del_msg = 0;

  ENTER();

  // if the user wants to show an AppIcon on the workbench,
  // we go and calculate the mail stats for all folders out there.
  if((flist = FO_CreateList()) != NULL)
  {
    int i;

    for(i = 1; i <= (int)*flist; i++)
    {
      struct Folder *fo = flist[i];

      if(fo == NULL)
        break;

      if(fo->Stats == TRUE)
      {
        new_msg += fo->New;
        unr_msg += fo->Unread;
        tot_msg += fo->Total;
        snt_msg += fo->Sent;
        del_msg += fo->Deleted;
      }
    }

    free(flist);
  }

  // clear AppIcon Label first before we create it new
  apptit[0] = '\0';

  if(C->WBAppIcon == TRUE)
  {
    char *src;

    // Lets create the label of the AppIcon now
    for(src = C->AppIconText; *src; src++)
    {
      char dst[10];

      if(*src == '%')
      {
        switch (*++src)
        {
          case '%': strlcpy(dst, "%", sizeof(dst));            break;
          case 'n': snprintf(dst, sizeof(dst), "%d", new_msg); break;
          case 'u': snprintf(dst, sizeof(dst), "%d", unr_msg); break;
          case 't': snprintf(dst, sizeof(dst), "%d", tot_msg); break;
          case 's': snprintf(dst, sizeof(dst), "%d", snt_msg); break;
          case 'd': snprintf(dst, sizeof(dst), "%d", del_msg); break;
        }
      }
      else
        snprintf(dst, sizeof(dst), "%c", *src);

      strlcat(apptit, dst, sizeof(apptit));
    }
  }

  // we set the mode accordingly to the status of the folder (new/check/old)
  if(G->TR != NULL && G->TR->Checking == TRUE)
    mode = ii_Check;
  else
    mode = tot_msg ? (new_msg != 0 ? ii_New : ii_Old) : ii_Empty;


  // We first have to remove the appicon before we can change it
  if(G->AppIcon != NULL)
  {
    RemoveAppIcon(G->AppIcon);
    G->AppIcon = NULL;
  }

  // Now we create the new AppIcon and display it
  if(G->theme.icons[mode] != NULL)
  {
    struct DiskObject *dobj = G->theme.icons[mode];

    // NOTE:
    // 1.) Using the VARARGS version is better for GCC/68k and it doesn't
    //     hurt other compilers
    // 2.) Using "zero" as lock parameter avoids a header compatibility
    //     issue (old: "struct FileLock *"; new: "BPTR")
    if(C->WBAppIcon == TRUE)
    {
      // set the icon position
      dobj->do_CurrentX = C->IconPositionX < 0 ? (LONG)NO_ICON_POSITION : C->IconPositionX;
      dobj->do_CurrentY = C->IconPositionY < 0 ? (LONG)NO_ICON_POSITION : C->IconPositionY;

      // add the AppIcon accordingly. Here we use v44+ tags, however older
      // workbench versions should perfectly ignore them.
      G->AppIcon = AddAppIcon(0, 0, apptit, G->AppPort, 0, dobj, WBAPPICONA_SupportsOpen,       TRUE,
                                                                 WBAPPICONA_SupportsSnapshot,   TRUE,
                                                                 WBAPPICONA_SupportsUnSnapshot, TRUE,
                                                                 WBAPPICONA_SupportsEmptyTrash, TRUE,
                                                                 WBAPPICONA_PropagatePosition,  TRUE,
                                                                 TAG_DONE);
      SHOWVALUE(DBF_GUI, G->AppIcon);
    }

    #if defined(__amigaos4__)
    // check if application.library is used and then
    // we also notify it about the AppIcon change
    if(G->applicationID > 0 && C->DockyIcon == TRUE)
    {
      if(G->LastIconID != mode)
      {
        struct ApplicationIconInfo aii;

        aii.iconType = APPICONT_CustomIcon;
        aii.info.customIcon = dobj;

        if(SetApplicationAttrs(G->applicationID, APPATTR_IconType, (uint32)&aii, TAG_DONE))
        {
          // remember the new docky icon state
          G->LastIconID = mode;
        }
      }
    }
    #endif

    // remember this icon pointer for later use
    G->currentAppIcon = mode;
  }

  LEAVE();
}

///
/// DisplayStatistics
//  Calculates folder statistics and update mailbox status icon
void DisplayStatistics(struct Folder *fo, BOOL updateAppIcon)
{
  int pos;
  struct MUI_NListtree_TreeNode *tn;
  struct Folder *actfo = FO_GetCurrentFolder();

  ENTER();

  D(DBF_GUI, "updating statistics for folder: %08lx", fo);

  // If the parsed argument is NULL we want to show the statistics from the actual folder
  if(!fo)
    fo = actfo;
  else if(fo == (struct Folder *)-1)
    fo = FO_GetFolderByType(FT_INCOMING, NULL);

  // Get Position of Folder
  pos = FO_GetFolderPosition(fo, TRUE);
  if(pos < 0)
  {
    LEAVE();
    return;
  }

  // update the stats for this folder
  FO_UpdateStatistics(fo);

  // if this folder hasn`t got any own folder image in the folder
  // directory and it is one of our standard folders we have to check which image we put in front of it
  if(fo->imageObject == NULL)
  {
    if(isIncomingFolder(fo))      fo->ImageIndex = (fo->Unread != 0) ? FICON_ID_INCOMING_NEW : FICON_ID_INCOMING;
    else if(isOutgoingFolder(fo)) fo->ImageIndex = (fo->Total != 0) ? FICON_ID_OUTGOING_NEW : FICON_ID_OUTGOING;
    else if(isTrashFolder(fo))    fo->ImageIndex = (fo->Total != 0) ? FICON_ID_TRASH_NEW : FICON_ID_TRASH;
    else if(isSentFolder(fo))     fo->ImageIndex = FICON_ID_SENT;
    else if(isSpamFolder(fo))     fo->ImageIndex = (fo->Total != 0) ? FICON_ID_SPAM_NEW : FICON_ID_SPAM;
    else fo->ImageIndex = -1;
  }

  if(fo == actfo)
  {
    CallHookPkt(&MA_SetMessageInfoHook, 0, 0);
    CallHookPkt(&MA_SetFolderInfoHook, 0, 0);
    DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, fo);
  }

  // Recalc the number of messages of the folder group
  if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, pos, MUIF_NONE)))
  {
    struct MUI_NListtree_TreeNode *tn_parent;

    // Now lets redraw the folderentry in the listtree
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, tn, MUIF_NONE);

    // Now we have to recalculate all parent and grandparents treenodes to
    // set their status accordingly.
    while((tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE)))
    {
      // fo_parent is NULL then it`s ROOT and we have to skip here
      struct Folder *fo_parent;

      if((fo_parent = (struct Folder *)tn_parent->tn_User) != NULL)
      {
        int i;

        // clear the parent mailvariables first
        fo_parent->Unread = 0;
        fo_parent->New = 0;
        fo_parent->Total = 0;
        fo_parent->Sent = 0;
        fo_parent->Deleted = 0;

        // Now we scan every child of the parent and count the mails
        for(i=0;;i++)
        {
          struct MUI_NListtree_TreeNode *tn_child;
          struct Folder *fo_child;

          tn_child = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn_parent, i, MUIV_NListtree_GetEntry_Flag_SameLevel);
          if(!tn_child)
            break;

          fo_child = (struct Folder *)tn_child->tn_User;

          fo_parent->Unread    += fo_child->Unread;
          fo_parent->New       += fo_child->New;
          fo_parent->Total     += fo_child->Total;
          fo_parent->Sent      += fo_child->Sent;
          fo_parent->Deleted   += fo_child->Deleted;
        }

        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, tn_parent, MUIF_NONE);

        // for the next step we set tn to the current parent so that we get the
        // grandparents ;)
        tn = tn_parent;
      }
      else
        break;
    }
  }

  if(!G->AppIconQuiet && updateAppIcon)
    DisplayAppIconStatistics();

  LEAVE();
}

///
/// CheckPrinter
//  Checks if printer is ready to print something
BOOL CheckPrinter(void)
{
  BOOL result = FALSE;

  ENTER();

  // check if the user wants us to check the printer state
  // at all.
  if(C->PrinterCheck == TRUE)
  {
    struct MsgPort *mp;

    // create the message port
    if((mp = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
    {
      struct IOStdReq *pio;

      // create the IO request for checking the printer status
      if((pio = AllocSysObjectTags(ASOT_IOREQUEST,
                                   ASOIOR_Size,      sizeof(struct IOStdReq),
                                   ASOIOR_ReplyPort, (ULONG)mp,
                                   TAG_DONE)) != NULL)
      {
        // from here on we assume the printer is online
        // but we do deeper checks.
        result = TRUE;

        // open printer.device unit 0
        if(OpenDevice("printer.device", 0, (struct IORequest *)pio, 0) == 0)
        {
          // we allow to retry the checking so
          // we iterate into a do/while loop
          do
          {
            UWORD ioResult = 0;

            // fill the IO request for querying the
            // device/line status of printer.device
            pio->io_Message.mn_ReplyPort = mp;
            pio->io_Command = PRD_QUERY;
            pio->io_Data = &ioResult;
            pio->io_Actual = 0;

            // initiate the IO request
            if(DoIO((struct IORequest *)pio) == 0)
            {
              // printer seems to be a parallel printer
              if(pio->io_Actual == 1)
              {
                D(DBF_PRINT, "received io request status: %08lx", ioResult);

                // check for any possible error state
                if(isFlagSet(ioResult>>8, (1<<0))) // printer busy (offline)
                {
                  ULONG res;

                  W(DBF_PRINT, "printer found to be in 'busy or offline' status");

                  // issue a requester telling the user about the faulty
                  // printer state
                  res = MUI_Request(G->App, NULL, 0, tr(MSG_ErrorReq),
                                                     tr(MSG_ER_PRINTER_OFFLINE_GADS),
                                                     tr(MSG_ER_PRINTER_OFFLINE));

                  if(res == 0) // Cancel/ESC
                  {
                    result = FALSE;
                    break;
                  }
                  else if(res == 1) // Retry
                    continue;
                  else // Ignore
                    break;
                }
                else if(isFlagSet(ioResult>>8, (1<<1))) // paper out
                {
                  ULONG res;

                  W(DBF_PRINT, "printer found to be in 'paper out' status");

                  // issue a requester telling the user about the faulty
                  // printer state
                  res = MUI_Request(G->App, NULL, 0, tr(MSG_ErrorReq),
                                                     tr(MSG_ER_PRINTER_NOPAPER_GADS),
                                                     tr(MSG_ER_PRINTER_NOPAPER));

                  if(res == 0) // Cancel/ESC
                  {
                    result = FALSE;
                    break;
                  }
                  else if(res == 1) // Retry
                    continue;
                  else // Ignore
                    break;
                }
                else
                {
                  D(DBF_PRINT, "printer was found to be ready");
                  break;
                }
              }
              else
              {
                // the rest signals an unsupported printer device
                // for status checking, so we assume the printer to
                // be online
                W(DBF_PRINT, "unsupported printer device ID '%ld'. Assuming online.", pio->io_Actual);
                break;
              }
            }
            else
            {
              W(DBF_PRINT, "DoIO() on printer status request failed!");
              break;
            }
          }
          while(TRUE);

          CloseDevice((struct IORequest *)pio);
        }
        else
          W(DBF_PRINT, "couldn't open printer.device unit 0");

        FreeSysObject(ASOT_IOREQUEST, pio);
      }
      else
        W(DBF_PRINT, "wasn't able to create io request for printer state checking");

      FreeSysObject(ASOT_PORT, mp);
    }
    else
      W(DBF_PRINT, "wasn't able to create msg port for printer state checking");
  }
  else
  {
    W(DBF_PRINT, "PrinterCheck disabled, assuming printer online");
    result = TRUE;
  }

  RETURN(result);
  return result;
}
///
/// PlaySound
//  Plays a sound file using datatypes
void PlaySound(char *filename)
{
  ENTER();

  if(DataTypesBase != NULL)
  {
    // if we previously created a sound object
    // lets dispose it first.
    if(G->NewMailSound_Obj != NULL)
    {
      // create a datatype trigger
      struct dtTrigger dtt;

      // Fill the trigger
      dtt.MethodID     = DTM_TRIGGER;
      dtt.dtt_GInfo    = NULL;
      dtt.dtt_Function = STM_STOP;
      dtt.dtt_Data     = NULL;

      // stop the sound by calling DoDTMethodA()
      DoDTMethodA(G->NewMailSound_Obj, NULL, NULL, (APTR)&dtt);

      // finally dispose the old object
      DisposeDTObject(G->NewMailSound_Obj);
    }

    // create the new datatype object
    if((G->NewMailSound_Obj = NewDTObject(filename, DTA_GroupID,    GID_SOUND,
                                                    DTA_SourceType, DTST_FILE,
                                                    TAG_DONE)) != NULL)
    {
      // create a datatype trigger
      struct dtTrigger dtt;

      // Fill the trigger
      dtt.MethodID     = DTM_TRIGGER;
      dtt.dtt_GInfo    = NULL;
      dtt.dtt_Function = STM_PLAY;
      dtt.dtt_Data     = NULL;

      // Play the sound by calling DoDTMethodA()
      DoDTMethodA(G->NewMailSound_Obj, NULL, NULL, (APTR)&dtt);
    }
  }

  LEAVE();
}
///
/// MatchExtension
//  Matches a file extension against a list of extension
static BOOL MatchExtension(const char *fileext, const char *extlist)
{
  BOOL result = FALSE;

  ENTER();

  if(extlist)
  {
    const char *s = extlist;
    size_t extlen = strlen(fileext);

    // now we search for our delimiters step by step
    while(*s)
    {
      const char *e;

      if((e = strpbrk(s, " |;,")) == NULL)
        e = s+strlen(s);

      D(DBF_MIME, "try matching file extension '%s' with '%s' %ld", fileext, s, e-s);

      // now check if the extension matches
      if((size_t)(e-s) == extlen &&
         strnicmp(s, fileext, extlen) == 0)
      {
        D(DBF_MIME, "matched file extension '%s' with type '%s'", fileext, s);

        result = TRUE;
        break;
      }

      // set the next start to our last search
      if(*e)
        s = ++e;
      else
        break;
    }
  }

  RETURN(result);
  return result;
}

///
/// IdentifyFile
// Tries to identify a file and returns its content-type if applicable
// otherwise NULL
const char *IdentifyFile(const char *fname)
{
  char ext[SIZE_FILE];
  const char *ctype = NULL;

  ENTER();

  // Here we try to identify the file content-type in multiple steps:
  //
  // 1: try to walk through the users' mime type list and check if
  //    a specified extension in the list matches the one of our file.
  //
  // 2: check against our hardcoded internal list of known extensions
  //    and try to do some semi-detailed analysis of the file header
  //
  // 3: use datatypes.library to find out the file class and construct
  //    an artifical content-type partly matching the file.

  // extract the extension of the file name first
  stcgfe(ext, fname);
  SHOWSTRING(DBF_MIME, ext);

  // now we try to identify the file by the extension first
  if(ext[0] != '\0')
  {
    struct MinNode *curNode;

    D(DBF_MIME, "identifying file by extension (mimeTypeList)");
    // identify by the user specified mime types
    for(curNode = C->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MimeTypeNode *curType = (struct MimeTypeNode *)curNode;

      if(curType->Extension[0] != '\0' &&
         MatchExtension(ext, curType->Extension))
      {
        ctype = curType->ContentType;
        break;
      }
    }

    if(ctype == NULL)
    {
      unsigned int i;

      D(DBF_MIME, "identifying file by extension (hardcoded list)");

      // before we are going to try to identify the file by reading some bytes out of
      // it, we try to identify it only by the extension.
      for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
      {
        if(IntMimeTypeArray[i].Extension != NULL &&
           MatchExtension(ext, IntMimeTypeArray[i].Extension))
        {
          ctype = IntMimeTypeArray[i].ContentType;
          break;
        }
      }
    }
  }

  // go on if we haven't got a content-type yet and try to identify
  // it with our own, hardcoded means.
  if(ctype == NULL)
  {
    FILE *fh;

    D(DBF_MIME, "identifying file by binary comparing the first bytes of '%s'", fname);

    // now that we still haven't been able to identify the file, we go
    // and read in some bytes from the file and try to identify it by analyzing
    // the binary data.
    if((fh = fopen(fname, "r")))
    {
      char buffer[SIZE_LARGE];
      int rlen;

      // we read in SIZE_LARGE into our temporary buffer without
      // checking if it worked out.
      rlen = fread(buffer, 1, SIZE_LARGE-1, fh);
      buffer[rlen] = '\0'; // NUL terminate the buffer.

      // close the file immediately.
      fclose(fh);
      fh = NULL;

      if(!strnicmp(buffer, "@database", 9))                                      ctype = IntMimeTypeArray[MT_TX_GUIDE].ContentType;
      else if(!strncmp(buffer, "%PDF-", 5))                                      ctype = IntMimeTypeArray[MT_AP_PDF].ContentType;
      else if(!strncmp(&buffer[2], "-lh5-", 5))                                  ctype = IntMimeTypeArray[MT_AP_LHA].ContentType;
      else if(!strncmp(buffer, "LZX", 3))                                        ctype = IntMimeTypeArray[MT_AP_LZX].ContentType;
      else if(*((long *)buffer) >= HUNK_UNIT && *((long *)buffer) <= HUNK_INDEX) ctype = IntMimeTypeArray[MT_AP_AEXE].ContentType;
      else if(!strncmp(&buffer[6], "JFIF", 4))                                   ctype = IntMimeTypeArray[MT_IM_JPG].ContentType;
      else if(!strncmp(buffer, "GIF8", 4))                                       ctype = IntMimeTypeArray[MT_IM_GIF].ContentType;
      else if(!strncmp(&buffer[1], "PNG", 3))                                    ctype = IntMimeTypeArray[MT_IM_PNG].ContentType;
      else if(!strncmp(&buffer[8], "ILBM", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_IM_ILBM].ContentType;
      else if(!strncmp(&buffer[8], "8SVX", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_AU_8SVX].ContentType;
      else if(!strncmp(&buffer[8], "ANIM", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_VI_ANIM].ContentType;
      else if(stristr(buffer, "\nFrom:"))                                        ctype = IntMimeTypeArray[MT_ME_EMAIL].ContentType;
      else
      {
        // now we do a statistical analysis to see if the file
        // is a binary file or not. Because then we use datatypes.library
        // for generating an artificial MIME type.
        int notascii = 0;
        int i;

        for(i=0; i < rlen; i++)
        {
          unsigned char c = buffer[i];

          // see if the current buffer position is
          // considered an ASCII/SPACE char.
          if((c < 32 || c > 127) && !isspace(c))
            notascii++;
        }

        // if the amount of not ASCII chars is lower than rlen/10 we
        // consider it a text file and don't do a deeper analysis.
        if(notascii < rlen/10)
        {
          ULONG prot;

          ObtainFileInfo(fname, FI_PROTECTION, &prot);
          ctype = IntMimeTypeArray[(prot & FIBF_SCRIPT) ? MT_AP_SCRIPT : MT_TX_PLAIN].ContentType;
        }
        else
        {
          D(DBF_MIME, "identifying file through datatypes.library");

          // per default we end up with an "application/octet-stream" content-type
          ctype = IntMimeTypeArray[MT_AP_OCTET].ContentType;

          if(DataTypesBase != NULL)
          {
            BPTR lock;

            if((lock = Lock(fname, ACCESS_READ)))
            {
              struct DataType *dtn;

              if((dtn = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL)) != NULL)
              {
                const char *type = NULL;
                struct DataTypeHeader *dth = dtn->dtn_Header;

                switch(dth->dth_GroupID)
                {
                  case GID_SYSTEM:     break;
                  case GID_DOCUMENT:   type = "application"; break;
                  case GID_TEXT:       type = "text"; break;
                  case GID_MUSIC:
                  case GID_SOUND:
                  case GID_INSTRUMENT: type = "audio"; break;
                  case GID_PICTURE:    type = "image"; break;
                  case GID_MOVIE:
                  case GID_ANIMATION:  type = "video"; break;
                }

                if(type)
                {
                  static char contentType[SIZE_CTYPE];

                  snprintf(contentType, sizeof(contentType), "%s/x-%s", type, dth->dth_BaseName);
                  ctype = contentType;
                }

                ReleaseDataType(dtn);
              }

              UnLock (lock);
            }
          }
        }
      }
    }
  }

  RETURN(ctype);
  return ctype;
}
///
/// GetRealPath
//  Function that gets the real path out of a supplied path. It will correctly resolve pathes like PROGDIR: aso.
char *GetRealPath(char *path)
{
  char *realPath;
  BPTR lock;
  BOOL success = FALSE;
  static char buf[SIZE_PATHFILE];

  ENTER();

  // lets try to get a Lock on the supplied path
  if((lock = Lock(path, SHARED_LOCK)))
  {
    // so, if it seems to exists, we get the "real" name out of
    // the lock again.
    if(NameFromLock(lock, buf, sizeof(buf)) != DOSFALSE)
      success = TRUE;

    // And then we unlock the file/dir immediatly again.
    UnLock(lock);
  }

  // only on success we return the realpath.
  realPath = success ? buf : path;

  RETURN(realPath);
  return realPath;
}

///
/// ExecuteCommand
//  Executes a DOS command
BOOL ExecuteCommand(char *cmd, BOOL asynch, enum OutputDefType outdef)
{
  BOOL result = TRUE;
  BPTR path;
  BPTR in = 0;
  BPTR out = 0;
  #if defined(__amigaos4__)
  BPTR err = 0;
  #endif

  ENTER();
  SHOWSTRING(DBF_UTIL, cmd);

  switch(outdef)
  {
    case OUT_DOS:
    {
      in = Input();
      out = Output();
      #if defined(__amigaos4__)
      err = ErrorOutput();
      #endif

      asynch = FALSE;
    }
    break;

    case OUT_NIL:
    {
      in = Open("NIL:", MODE_OLDFILE);
      out = Open("NIL:", MODE_NEWFILE);
    }
    break;
  }

  // path may return 0, but that's fine.
  // and we also don't free it manually as this
  // is done by SystemTags/CreateNewProc itself.
  path = CloneSearchPath();

  if(SystemTags(cmd,
                SYS_Input,    in,
                SYS_Output,   out,
                #if defined(__amigaos4__)
                SYS_Error,    err,
                NP_Child,     TRUE,
                #endif
                NP_Name,      "YAM command process",
                NP_Path,      path,
                NP_StackSize, C->StackSize,
                NP_WindowPtr, -1,           // show no requesters at all
                SYS_Asynch,   asynch,
                TAG_DONE) != 0)
  {
    // an error occurred as SystemTags should always
    // return zero on success, no matter what.
    E(DBF_UTIL, "execution of command '%s' failed, IoErr()=%ld", cmd, IoErr());

    // manually free our search path
    // as SystemTags() shouldn't have freed
    // it itself.
    if(path != 0)
      FreeSearchPath(path);

    result = FALSE;
  }

  if(asynch == FALSE && outdef != OUT_DOS)
  {
    Close(out);
    Close(in);
  }

  RETURN(result);
  return result;
}
///
/// GetSimpleID
//  Returns a unique number
int GetSimpleID(void)
{
  static int num = 0;

  return ++num;
}
///
/// GotoURL
//  Loads an URL using an ARexx script or openurl.library
void GotoURL(const char *url, BOOL newWindow)
{
  ENTER();

  // The ARexx macro to open a URL is only possible after the startup phase
  // and if a script has been configured for this purpose.
  if(G != NULL && G->InStartupPhase == FALSE && C->RX[MACRO_URL].Script[0] != '\0')
  {
    char newurl[SIZE_LARGE];

    snprintf(newurl, sizeof(newurl), "\"%s\"", url);
    MA_StartMacro(MACRO_URL, newurl);
  }
  else if(OpenURLBase != NULL)
  {
    struct TagItem tags[] = { { URL_NewWindow, newWindow },
                              { TAG_DONE,      TAG_END   } };

    // open the URL in a defined web browser and
    // let the user decide himself if he wants to see
    // it popping up in a new window or not (via OpenURL
    // prefs)
    URL_OpenA((STRPTR)url, tags);
  }
  else
    W(DBF_HTML, "No openurl.library v1+ found");

  LEAVE();
}
///
/// SWSSearch
// Smith&Waterman 1981 extended string similarity search algorithm
// X, Y are the two strings that will be compared for similarity
// It will return a pattern which will reflect the similarity of str1 and str2
// in a Amiga suitable format. This is case-insensitive !
char *SWSSearch(char *str1, char *str2)
{
  char *similar;
  static char *Z = NULL;    // the destination string (result)
  int **L        = NULL;    // L matrix
  int **Ind      = NULL;    // Indicator matrix
  char *X;                  // 1.string X
  char *Y        = NULL;    // 2.string Y
  int lx;                   // length of X
  int ly;                   // length of Y
  int lz;                   // length of Z (maximum)
  int i, j;
  BOOL gap = FALSE;
  BOOL success = FALSE;

  // special enum for the Indicator
  enum  IndType { DELX=1, DELY, DONE, TAKEBOTH };

  ENTER();

  // by calling this function with (NULL, NULL) someone wants
  // to signal us to free the destination string
  if(str1 == NULL || str2 == NULL)
  {
    if(Z != NULL)
      free(Z);
    Z = NULL;

    RETURN(NULL);
    return NULL;
  }

  // calculate the length of our buffers we need
  lx = strlen(str1)+1;
  ly = strlen(str2)+1;
  lz = MAX(lx, ly)*3+3;

  // first allocate all resources
  if(!(X   = calloc(lx+1, sizeof(char)))) goto abort;
  if(!(Y   = calloc(ly+1, sizeof(char)))) goto abort;

  // now we have to alloc our help matrixes
  if(!(L   = calloc(lx,   sizeof(int))))  goto abort;
  if(!(Ind = calloc(lx,   sizeof(int))))  goto abort;
  for(i = 0; i < lx; i++)
  {
    if(!(L[i]   = calloc(ly, sizeof(int)))) goto abort;
    if(!(Ind[i] = calloc(ly, sizeof(int)))) goto abort;
  }

  // and allocate the result string separately
  if(Z != NULL)
    free(Z);
  if(!(Z = calloc(lz, sizeof(char)))) goto abort;

  // we copy str1&str2 into X and Y but have to copy a placeholder in front of them
  memcpy(&X[1], str1, lx);
  memcpy(&Y[1], str2, ly);

  for(i = 1; i < lx; i++)
    Ind[i][0] = DELX;

  for(j = 1; j < ly; j++)
    Ind[0][j] = DELY;

  Ind[0][0] = DONE;

  // Now we calculate the L matrix
  // this is the first step of the SW algorithm
  for(i = 1; i < lx; i++)
  {
    for(j = 1; j < ly; j++)
    {
      if(toupper(X[i]) == toupper(Y[j]))  // case insensitive version
      {
        L[i][j] = L[i-1][j-1] + 1;
        Ind[i][j] = TAKEBOTH;
      }
      else
      {
        if(L[i-1][j] > L[i][j-1])
        {
          L[i][j] = L[i-1][j];
          Ind[i][j] = DELX;
        }
        else
        {
          L[i][j] = L[i][j-1];
          Ind[i][j] = DELY;
        }
      }
    }
  }

#ifdef DEBUG
  // for debugging only
  // This will print out the L & Ind matrix to identify problems
/*
  printf(" ");
  for(j=0; j < ly; j++)
  {
    printf(" %c", Y[j]);
  }
  printf("\n");

  for(i=0; i < lx; i++)
  {
    printf("%c ", X[i]);

    for(j=0; j < ly; j++)
    {
      printf("%d", L[i][j]);
      if(Ind[i][j] == TAKEBOTH)  printf("`");
      else if(Ind[i][j] == DELX) printf("^");
      else if(Ind[i][j] == DELY) printf("<");
      else printf("*");
    }
    printf("\n");
  }
*/
#endif

  // the second step of the SW algorithm where we
  // process the Ind matrix which represents which
  // char we take and which we delete

  Z[--lz] = '\0';
  i = lx-1;
  j = ly-1;

  while(i >= 0 && j >= 0 && Ind[i][j] != DONE)
  {
    if(Ind[i][j] == TAKEBOTH)
    {
      Z[--lz] = X[i];

      i--;
      j--;
      gap = FALSE;
    }
    else if(Ind[i][j] == DELX)
    {
      if(!gap)
      {
        if(j > 0)
        {
          Z[--lz] = '?';
          Z[--lz] = '#';
        }
        gap = TRUE;
      }
      i--;
    }
    else if(Ind[i][j] == DELY)
    {
      if(!gap)
      {
        if(i > 0)
        {
          Z[--lz] = '?';
          Z[--lz] = '#';
        }
        gap = TRUE;
      }
      j--;
    }
  }

  success = TRUE;

abort:

  // now we free our temporary buffers now
  if(X != NULL)
    free(X);
  if(Y != NULL)
    free(Y);

  // lets free our help matrixes
  if(L != NULL)
  {
    for(i = 0; i < lx; i++)
    {
      if(L[i] != NULL)
        free(L[i]);
    }
    free(L);
  }
  if(Ind != NULL)
  {
    for(i = 0; i < lx; i++)
    {
      if(Ind[i] != NULL)
        free(Ind[i]);
    }
    free(Ind);
  }

  similar = success ? &(Z[lz]) : NULL;

  RETURN(similar);
  return similar;
}
///
/// CRC32
//  Function that calculates a 32bit crc checksum for a provided buffer.
//  See http://www.4d.com/ACIDOC/CMU/CMU79909.HTM for more information about
//  the CRC32 algorithm.
//  This implementation allows the usage of more than one persistant calls of
//  the crc32 function. This allows to calculate a valid crc32 checksum over
//  an unlimited amount of buffers.
ULONG CRC32(void *buffer, unsigned int count, ULONG crc)
{
  /* table generated with the following code:
   *
   * #define CRC32_POLYNOMIAL 0xEDB88320L
   *
   * int i, j;
   *
   * for (i = 0; i <= 255; i++) {
   *   unsigned long crc = i;
   *   for (j = 8; j > 0; j--) {
   *     if (crc & 1)
   *       crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
   *     else
   *       crc >>= 1;
   *   }
   *   CRCTable[i] = crc;
   * }
   */
  static const unsigned long CRCTable[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
  };
  unsigned char *p = (unsigned char *)buffer;

  ENTER();

  // we calculate the crc32 now.
  while (count-- != 0)
  {
    ULONG temp1 = (crc >> 8) & 0x00FFFFFFL;
    ULONG temp2 = CRCTable[((int)crc ^ *p++) & 0xFF];

    crc = temp1 ^ temp2;
  }

  RETURN(crc);
  return crc;
}
///

/*** REXX interface support ***/
/// InsertAddresses
//  Appends an array of addresses to a string gadget
void InsertAddresses(Object *obj, char **addr, BOOL add)
{
  char *buf;

  ENTER();

  buf = (char *)xget(obj, MUIA_String_Contents);

  if(buf[0] != '\0' && add)
    DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
  else
    setstring(obj, "");

  DoMethod(obj, MUIM_BetterString_Insert, *addr, MUIV_BetterString_Insert_EndOfString);

  while(*++addr != NULL)
  {
    DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
    DoMethod(obj, MUIM_BetterString_Insert, *addr, MUIV_BetterString_Insert_EndOfString);
  }

  LEAVE();
}
///
/// AllocReqText
//  Prepare multi-line text for requesters, converts \n to line breaks
char *AllocReqText(char *s)
{
  char *reqtext;

  ENTER();

  if((reqtext = calloc(strlen(s) + 1, 1)) != NULL)
  {
    char *d = reqtext;

    while(*s != '\0')
    {
      if(s[0] == '\\' && s[1] == 'n')
      {
        *d++ = '\n';
        s++;
        s++;
      }
      else
        *d++ = *s++;
    }
  }

  RETURN(reqtext);
  return reqtext;
}
///
/// ToLowerCase
//  Change a complete string to lower case
void ToLowerCase(char *str)
{
  char c;

  ENTER();

  while ((c = *str) != '\0')
    *str++ = tolower(c);

  LEAVE();
}
///
/// WriteUInt32
//  write a 32bit variable to a stream, big endian style
int WriteUInt32(FILE *stream, ULONG value)
{
  int n;

  ENTER();

  // convert the value to big endian style
  value = htonl(value);

  n = fwrite(&value, sizeof(value), 1, stream);

  RETURN(n);
  return n;
}
///
/// ReadUInt32
//  read a 32bit variable from a stream, big endian style
int ReadUInt32(FILE *stream, ULONG *value)
{
  int n;

  ENTER();

  if((n = fread(value, sizeof(*value), 1, stream)) == 1)
  {
    // convert the value to big endian style
    *value = ntohl(*value);
  }

  RETURN(n);
  return n;
}
///

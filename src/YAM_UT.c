/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
#include <stdarg.h>

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
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/keymap.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>
#include <workbench/startup.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extra.h"
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
#include "classes/ClassesExtra.h"

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

/* local protos */
static int  Word_Length(const char *buf);
static int  Quoting_Chars(char *buf, int len, char *text, int *post_spaces);
static BOOL GetPackMethod(enum FolderMode fMode, char **method, int *eff);
static BOOL CompressMailFile(char *src, char *dst, char *passwd, char *method, int eff);
static BOOL UncompressMailFile(char *src, char *dst, char *passwd);
static void AppendToLogfile(int id, char *text, void *a1, void *a2, void *a3, void *a4);

#if !defined(__amigaos4__) || (INCLUDE_VERSION < 50)
struct PathNode
{
  BPTR pn_Next;
  BPTR pn_Lock;
};
#endif

/// CloneSearchPath()
// This returns a duplicated search path (preferable the workbench
// searchpath) usable for NP_Path of SystemTagList().
static BPTR CloneSearchPath(void)
{
  BPTR path = 0;

  if(WorkbenchBase && WorkbenchBase->lib_Version >= 44)
  {
    WorkbenchControl(NULL, WBCTRLA_DuplicateSearchPath, &path, TAG_DONE);
  }
  else
  {
    // We don't like this evil code in OS4 compile, as we should have
    // a recent enough workbench available
#ifndef __amigaos4__
    struct Process *pr;

    pr = (struct Process*)FindTask(NULL);

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

          // TODO: Check out if AllocVec() is correct, because this memory is
          // freed by the system later
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
#endif
  }
  
  return path;
}

///
/// FreeSearchPath()
// Free the memory returned by CloneSearchPath
static void FreeSearchPath(BPTR path)
{
  if(path == 0)
    return;

  if(WorkbenchBase && WorkbenchBase->lib_Version >= 44)
  {
    WorkbenchControl(NULL, WBCTRLA_FreeSearchPath, path, TAG_DONE);
  }
  else
  {
#ifndef __amigaos4__
    while (path)
    {
      struct PathNode *node = BADDR(path);
      path = node->pn_Next;
      UnLock(node->pn_Lock);
      FreeVec(node);
    }
#endif
  }
}
///

/*** Hooks ***/
/// AttachDspFunc
//  Attachment listview display hook
HOOKPROTONH(AttachDspFunc, long, char **array, struct Part *entry)
{
   if (entry)
   {
      static char dispnu[SIZE_SMALL], dispsz[SIZE_SMALL];
      array[0] = array[2] = "";

      if(entry->Nr > PART_RAW)
        snprintf(array[0] = dispnu, sizeof(dispnu), "%d", entry->Nr);

      if(*entry->Name) array[1] = entry->Name;
      else             array[1] = DescribeCT(entry->ContentType);

      if(entry->Size > 0)
      {
        array[2] = dispsz;

        if(entry->Decoded)
          FormatSize(entry->Size, dispsz, sizeof(dispsz));
        else
        {
          dispsz[0] = '~';
          FormatSize(entry->Size, &dispsz[1], sizeof(dispsz)-1);
        }
      }
   }
   else
   {
      array[0] = GetStr(MSG_ATTACH_NO);
      array[1] = GetStr(MSG_ATTACH_PART);
      array[2] = GetStr(MSG_Size);
   }

   return 0;
}
MakeStaticHook(AttachDspHook, AttachDspFunc);
///

/*** Requesters ***/
/// YAMMUIRequest
// Own -secure- implementation of MUI_Request with collecting and reissueing ReturnIDs
// We also have a wrapper #define MUI_Request for calling that function instead.
LONG STDARGS YAMMUIRequest(APTR app, APTR win, UNUSED LONG flags, char *title, char *gadgets, char *format, ...)
{
  LONG result = -1;
  char reqtxt[SIZE_LINE];
  Object *WI_YAMREQ;
  Object *BT_GROUP;
  va_list args;

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
       InnerSpacing(4,4),
       GroupSpacing(8),
       MUIA_Background,       MUII_RequesterBack,
       Child, HGroup,
          Child, TextObject,
            TextFrame,
            InnerSpacing(8,8),
            MUIA_Background,    MUII_TextBack,
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
    char fstring[] = "-capslock f1"; // by default we set it to "-capslock f1" so that we can press f1
    char *ul;
    BOOL active = FALSE, ie = TRUE;
    Object *BT_TEMP;

    set(app, MUIA_Application_Sleep, TRUE);
    DoMethod(app, OM_ADDMEMBER, WI_YAMREQ);

    // first we count how many gadget we have to create
    for(num_gads=1, token=gadgets; *token; token++)
    {
      if(*token == '|') num_gads++;
    }

    // prepare the BT_Group for the change.
    DoMethod(BT_GROUP, MUIM_Group_InitChange);

    // now we create the buttons for the requester
    for(token=gadgets, i=0; i < num_gads; i++, token=next)
    {
      if((next = strchr(token, '|'))) *next++ = '\0';
      if(*token == '*') { active=TRUE; token++; }
      if((ul = strchr(token, '_'))) ie = FALSE;

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
          fstring[11] = '0'+i+1;
          DoMethod(WI_YAMREQ, MUIM_Notify, MUIA_Window_InputEvent, fstring, app, 2, MUIM_Application_ReturnID, i+1);
        }

        DoMethod(BT_TEMP, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, i+1);
        if(active) { set(WI_YAMREQ, MUIA_Window_ActiveObject, BT_TEMP); active = FALSE; }
      }

      // write back what we took.
      if(next) *(next-1) = '|';
    }

    // signal a ExitChange now
    DoMethod(BT_GROUP, MUIM_Group_ExitChange);

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

  return result;
}
///
/// StringRequest
//  Puts up a string requester
int StringRequest(char *string, int size, char *title, char *body, char *yestext, char *alttext, char *notext, BOOL secret, Object *parent)
{
  Object *bt_okay;
  Object *bt_middle;
  Object *bt_cancel;
  Object *wi_sr;
  Object *st_in;
  int ret_code = -1;

  wi_sr = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
    MUIA_Window_RefWindow,  parent,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
    MUIA_Window_ID,         MAKE_ID('S','R','E','Q'),
    WindowContents, VGroup,
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
/// FolderRequest
//  Allows user to choose a folder from a list
struct Folder *FolderRequest(char *title, char *body, char *yestext, char *notext, struct Folder *exclude, APTR parent)
{
   static int lastactive;
   struct Folder **flist, *folder = (struct Folder *)-1;
   char *fname;
   APTR bt_okay, bt_cancel, wi_fr, lv_folder;

   wi_fr = WindowObject,
      MUIA_Window_Title, title ? title : "YAM",
      MUIA_Window_RefWindow, parent,
      MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
      MUIA_Window_ID,        MAKE_ID('F','R','E','Q'),
      WindowContents, VGroup,
         Child, LLabel(body),
         Child, lv_folder = ListviewObject,
            MUIA_CycleChain, 1,
            MUIA_Listview_DoubleClick, TRUE,
            MUIA_Listview_List, ListObject,
               InputListFrame,
               MUIA_List_AutoVisible, TRUE,
            End,
         End,
         Child, ColGroup(3),
            Child, bt_okay = MakeButton(yestext),
            Child, HSpace(0),
            Child, bt_cancel = MakeButton(notext),
         End,
      End,
   End;

   if (wi_fr)
   {
      int i;

      flist = FO_CreateList();
      for (i = 1; i <= (int)*flist; i++) if (flist[i] != exclude) if (flist[i]->Type != FT_GROUP)
         DoMethod(lv_folder, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
      free(flist);
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
            {
              folder = NULL;
            }
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
   return folder;
}
///
/// AttachRequest
//  Allows user to select a message part (attachment) from a list
struct Part *AttachRequest(char *title, char *body, char *yestext, char *notext, int mode, struct ReadMailData *rmData)
{
  struct Part *retpart = (struct Part *)-1, *part;
  APTR bt_okay, bt_cancel, wi_ar, lv_attach;

  // lets create the AttachSelection window now
  wi_ar = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
    MUIA_Window_RefWindow,  rmData->readWindow,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
//    MUIA_Window_ID,         MAKE_ID('A','R','E','Q'), // we don`t supply a windowID or otherwise the upper three attributes don`t work.
    WindowContents, VGroup,
      Child, LLabel(body),
        Child, lv_attach = NListviewObject,
          MUIA_CycleChain, TRUE,
          MUIA_NListview_NList, NListObject,
            InputListFrame,
            MUIA_NList_Title,       TRUE,
            MUIA_NList_DoubleClick, TRUE,
            MUIA_NList_MultiSelect, isMultiReq(mode) ? MUIV_NList_MultiSelect_Default : MUIV_NList_MultiSelect_None,
            MUIA_NList_DisplayHook, &AttachDspHook,
            MUIA_NList_Format,      "BAR,BAR,",
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
    strlcpy(spart[0].Name, GetStr(MSG_RE_Original), sizeof(spart[0].Name));
    spart[0].Size = rmData->mail->Size;
    spart[0].Decoded = TRUE;
    DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[0], MUIV_NList_Insert_Top);
    set(lv_attach, MUIA_NList_Active, MUIV_NList_Active_Top);

    // if this AttachRequest isn`t a DISPLAY request we show all the option to select the text we actually see
    if(!isDisplayReq(mode))
    {
      spart[1].Nr = PART_ALLTEXT;
      strlcpy(spart[1].Name, GetStr(MSG_RE_AllTexts), sizeof(spart[1].Name));
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

  return retpart;
}
///
/// InfoWindow
//  Displays a text in an own modeless window
void InfoWindow(char *title, char *body, char *oktext, APTR parent)
{
   Object *bt_okay;
   Object *wi_iw;

   if((wi_iw = WindowObject,
         MUIA_Window_Title,     title,
         MUIA_Window_RefWindow, parent,
         MUIA_Window_LeftEdge,  MUIV_Window_LeftEdge_Centered,
         MUIA_Window_TopEdge,   MUIV_Window_TopEdge_Centered,
         WindowContents, VGroup,
            MUIA_Background, MUII_RequesterBack,
            Child, VGroup,
               TextFrame,
               MUIA_Background, MUII_TextBack,
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
}
///

/*** String related ***/
/// itoa
//  Converts an integer into a string
char *itoa(int val)
{
  static char str[SIZE_SMALL];
  snprintf(str, sizeof(str), "%d", val);
  return str;
}
///
/// MatchNoCase
//  Case insensitive pattern matching
BOOL MatchNoCase(const char *string, const char *match)
{
   BOOL result=FALSE;
   LONG patternlen = strlen(match)*2+2; // ParsePattern() needs at least 2*source+2 bytes buffer
   char *pattern = malloc((size_t)patternlen);

   if(pattern)
   {
     if(ParsePatternNoCase((STRPTR)match, pattern, patternlen) != -1)
     {
        result = MatchPatternNoCase((STRPTR)pattern, (STRPTR)string);
     }

     free(pattern);
   }

   return result;
}
///
/// StripUnderscore
//  Removes underscore from button labels
char *StripUnderscore(char *label)
{
   if (strchr(label,'_'))
   {
      static char newlabel[SIZE_DEFAULT];
      char *p = newlabel;
      for (; *label; label++) if (*label != '_') *p++ = *label;
      *p = 0;
      return newlabel;
   }
   else return label;
}
///
/// GetNextLine
//  Reads next line from a multi-line string
char *GetNextLine(char *p1)
{
   static char *begin;
   char *p2;
   if (p1) begin = p1;
   if ((p1 = strchr(p2 = begin, '\n'))) { *p1 = 0; begin = ++p1; }
   return p2;
}
///
/// TrimStart
//  Strips leading spaces
char *TrimStart(char *s)
{
   while (*s && isspace(*s)) ++s;
   return s;
}
///
/// TrimEnd
//  Removes trailing spaces
char *TrimEnd(char *s)
{
   char *e = s+strlen(s)-1;
   while (e >= s && isspace(*e)) *e-- = 0;
   return s;
}
///
/// Trim
//  Removes leading and trailing spaces
char *Trim(char *s)
{
   if(s)
   {
      char *e = s+strlen(s)-1;
      while (*s && isspace(*s)) ++s;
      while (e >= s && isspace(*e)) *e-- = '\0';
   }
   return s;
}
///
/// stristr
//  Case insensitive version of strstr()
char *stristr(const char *a, const char *b)
{
   int l = strlen(b);
   for (; *a; a++) if(strnicmp(a, b, l) == 0) return (char *)a;
   return NULL;
}
///
/// MyStrChr
//  Searches for a character in string, ignoring text in quotes
char *MyStrChr(const char *s, int c)
{
   BOOL nested = FALSE;

   for (; *s; s++)
      if (*s == '\"') nested = !nested;
      else if (*s == (char)c && !nested) return (char *)s;
   return NULL;
}
///
/// AllocStrBuf
//  Allocates a dynamic buffer
char *AllocStrBuf(size_t initlen)
{
   size_t *strbuf = calloc(initlen+sizeof(size_t), sizeof(char));
   if(!strbuf) return NULL;
   *strbuf++ = initlen;
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
char *AppendToBuffer(char *buf, int *wptr, int *len, char *add)
{
   int nlen = *len, npos = (*wptr)+strlen(add);
   while (npos >= nlen-1) nlen = (nlen*3)/2;
   if (nlen != *len) buf = realloc(buf, *len = nlen);
   while (*add) buf[(*wptr)++] = *add++;
   buf[*wptr] = '\0'; // we have to make sure that the string is null terminated
   return buf;
}
///
/// AllocCopy
//  Duplicates a memory block
APTR AllocCopy(APTR source, int size)
{
   APTR dest = malloc(size);
   if (dest) memcpy(dest, source, size);
   return dest;
}
///
/// Decrypt
//  Decrypts passwords
char *Decrypt(char *source)
{
   static char buffer[SIZE_PASSWORD+2];
   char *write = &buffer[SIZE_PASSWORD];

   *write-- = 0;
   while (*source)
   {
      *write-- = ((char)atoi(source))^CRYPTBYTE;
      source += 4;
   }
   return ++write;
}
///
/// Encrypt
//  Encrypts passwords
char *Encrypt(char *source)
{
   static char buffer[4*SIZE_PASSWORD+2];
   char *read = source+strlen(source)-1;

   *buffer = 0;
   while (read >= source)
   {
      unsigned char c = (*read--)^CRYPTBYTE;
      int p = strlen(buffer);

      snprintf(&buffer[p], sizeof(buffer)-p, "%03d ", c);
   }
   return buffer;
}
///

/*** File related ***/
/// GetLine
//  Gets Null terminated line of a text file
char *GetLine(FILE *fh, char *buffer, int bufsize)
{
  char *ptr;

  // read in the next line or return NULL if
  // a problem occurrs. The caller then should
  // query ferror() to determine why exactly it
  // failed.
  if(fgets(buffer, bufsize, fh) == NULL)
  {
    // lets NUL-terminate the string at least.
    buffer[0] = '\0';
    return NULL;
  }

  // search for either a \r or \n and terminate there
  // if found.
  if((ptr = strpbrk(buffer, "\r\n")))
    *ptr = '\0';

  // now return the buffer.
  return buffer;
}       

///
/// FileExists
//  return true/false if file exists
BOOL FileExists(char *filename)
{
  BPTR lock = Lock((STRPTR)filename, ACCESS_READ);
  if(!lock)
    return FALSE;
  UnLock(lock);
  return TRUE;
}
///
/// FileSize
//  Returns size of a file
int FileSize(char *filename)
{
  BPTR lock;
  int size = -1;

  if((lock = Lock((STRPTR)filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        size = fib->fib_Size;
      }
      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return size;
}
///
/// FileProtection
//  Returns protection bits of a file
long FileProtection(const char *filename)
{
  BPTR lock;
  long prots = -1;

  if((lock = Lock((STRPTR)filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        prots = fib->fib_Protection;
      }
      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return prots;
}
///
/// FileType
//  Returns file type (file/directory)
int FileType(char *filename)
{
  BPTR lock;
  long type = 0;

  if((lock = Lock((STRPTR)filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        type = fib->fib_DirEntryType < 0 ? 1 : 2;
      }
      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return type;
}
///
/// FileComment
//  Returns file comment
char *FileComment(char *filename)
{
  BPTR lock;
  static char fileComment[80];
  char *comment = NULL;

  if((lock = Lock((STRPTR)filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        strlcpy(fileComment, fib->fib_Comment, sizeof(fileComment));
        comment = fileComment;
      }
      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return comment;
}
///
/// FileDate
//  Returns the date of the file
struct DateStamp *FileDate(char *filename)
{
  BPTR lock;
  static struct DateStamp ds;
  struct DateStamp *res = NULL;

  if((lock = Lock(filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        memcpy(&ds, &fib->fib_Date, sizeof(struct DateStamp));
        res = &ds;
      }
      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return res;
}
///
/// FileTime
//  Returns the date of the file in seconds since 1.1.1970
long FileTime(const char *filename)
{
  BPTR lock;
  long ret = 0;

  if((lock = Lock((STRPTR)filename, ACCESS_READ)))
  {
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB, NULL)))
    {
      if(Examine(lock, fib))
      {
        // this is the correct calculation to convert the
        // struct DateStamp entries which are based on the 1.1.1978
        // to a long variable for the seconds since 1.1.1970
        ret = ((fib->fib_Date.ds_Days + 2922) * 1440 +
                fib->fib_Date.ds_Minute) * 60 +
                fib->fib_Date.ds_Tick / TICKS_PER_SECOND;
      }

      FreeDosObject(DOS_FIB, fib);
    }

    UnLock(lock);
  }

  return ret;
}
///
/// RenameFile
//  Renames a file and restores the protection bits
BOOL RenameFile(char *oldname, char *newname)
{
   struct FileInfoBlock *fib;
   BPTR lock;
   BOOL result = FALSE;

   if(!Rename(oldname, newname))
     return FALSE;

   if((fib = AllocDosObject(DOS_FIB,NULL)))
   {
      if((lock = Lock(newname, ACCESS_READ)))
      {
         if(Examine(lock, fib))
         {
            UnLock(lock);
            if(SetProtection(newname, fib->fib_Protection & (~FIBF_ARCHIVE)))
            {
              result = TRUE;
            }
         }
         else UnLock(lock);
      }
      FreeDosObject(DOS_FIB,fib);
   }
   return result;
}
///
/// CopyFile
//  Copies a file
BOOL CopyFile(char *dest, FILE *destfh, char *sour, FILE *sourfh)
{
   BOOL success = FALSE;

   if(sour)
     sourfh = fopen(sour, "r");

   if(sourfh && dest)
     destfh = fopen(dest, "w");

   if(sourfh && destfh)
   {
      char buf[SIZE_LARGE];
      int len;

      while((len = fread(buf, 1, SIZE_LARGE, sourfh)))
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

   if(dest && destfh)
     fclose(destfh);

   if(sour && sourfh)
     fclose(sourfh);

   return success;
}
///
/// MoveFile
//  Moves a file (also from one partition to another)
BOOL MoveFile(char *oldfile, char *newfile)
{
  // we first try to rename the file with a standard Rename()
  // and if it doesn't work we do a raw copy
  if(!RenameFile(oldfile, newfile))
  {
    // a normal rename didn't work, so lets copy the file
    if(!CopyFile(newfile, 0, oldfile, 0) ||
       !DeleteFile(oldfile))
    {
      // also a copy didn't work, so lets return an error
      return FALSE;
    }
  }

  return TRUE;
}
///
/// ConvertCRLF
//  Converts line breaks from LF to CRLF or vice versa
BOOL ConvertCRLF(char *in, char *out, BOOL to)
{
   BOOL success = FALSE;
   char buf[SIZE_LINE];
   FILE *infh, *outfh;

   if ((infh = fopen(in, "r")))
   {
      if ((outfh = fopen(out, "w")))
      {
         while (GetLine(infh, buf, SIZE_LINE)) fprintf(outfh, "%s%s\n", buf, to?"\r":"");
         success = TRUE;
         fclose(outfh);
      }
      fclose(infh);
   }
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
static int Quoting_Chars(char *buf, int len, char *text, int *post_spaces)
{
  unsigned char c;
  BOOL quote_found = FALSE;
  int i=0;
  int last_bracket = 0;
  int skip_chars = 0;
  int pre_spaces = 0;

  (*post_spaces) = 0;

  while((c = *text++) && i < len-1)
  {
    if(c == '>')
    {
      if(pre_spaces > 0)
        break;

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
      else if(quote_found == TRUE || pre_spaces > 0 || skip_chars > 2)
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
  return last_bracket ? skip_chars : 0;
}

///
/// Quote_Text
//  Main mail text quotation function. It takes the source string "src" and
//  analyzes it concerning existing quoting characters. Depending on this
//  information it adds new quoting marks "prefix" to the start of each line
//  taking care of a correct word wrap if the line gets longs than "line_max".
//  All output is directly written to the already opened filehandle "out".
void Quote_Text(FILE *out, char *src, int len, int line_max, char *prefix)
{
  // make sure the output file handle is valid
  if(out)
  {
    static char temp_buf[128];
    int temp_len;
    BOOL newline = TRUE;
    BOOL wrapped = FALSE; // needed to implement automatic wordwrap while quoting
    BOOL lastwasspace = FALSE;
    int skip_on_next_newline = 0;
    int line_len = 0;
    int skip_chars;
    int post_spaces = 0;

    // find out how many quoting chars the next line has
    skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
    temp_len = strlen(temp_buf) - skip_chars;
    src += skip_chars;
    len -= skip_chars;

    while(len > 0)
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
        len--;
        continue;
      }

      // on a CR (newline)
      if(c == '\n')
      {
        src++;
        len--;

        // find out how many quoting chars the next line has
        skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
        src += (skip_chars + skip_on_next_newline);
        len -= (skip_chars + skip_on_next_newline);
        skip_on_next_newline = 0;

        if(temp_len == ((int)strlen(temp_buf)-skip_chars) && wrapped)
        {
          // the text has been wrapped previously and the quoting chars
          // are the same like the previous line, so the following text
          // probably belongs to the same paragraph

          len -= temp_len; // skip the quoting chars
          src += temp_len;
          wrapped = FALSE;

          // check whether the next char will be a newline or not, because
          // a newline indicates a new empty line, so there is no need to
          // cat something together at all
          if(*src != '\n')
          {
            // add a space to if this was the first quoting
            if(temp_len == 0 || (*src != ' ' && lastwasspace == FALSE))
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
          fputs(prefix, out);

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
          fputs(prefix, out);
          line_len+=strlen(prefix);
        }
        else
        {
          fputs(prefix, out);
          fputc(' ', out);
          line_len+=strlen(prefix)+1;
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
        len--;

        // output a newline to start a new line
        fputc('\n', out);

        // reset line_len
        line_len = 0;

        fputs(prefix, out);
        line_len += strlen(prefix);

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
      len--;
    }

    // check whether we finished the quoting with
    // a newline or otherwise the followed signature won`t fit correctly
    if(newline == FALSE)
      fputc('\n', out);
  }
}
///
/// SimpleWordWrap
//  Reformats a file to a new line length
void SimpleWordWrap(char *filename, int wrapsize)
{
   BPTR fh;
   if ((fh = Open((STRPTR)filename, MODE_OLDFILE)))
   {
      char ch;
      int p = 0, lsp = -1, sol = 0;

      while (Read(fh, &ch, 1))
      {
         if (p-sol > wrapsize && lsp >= 0)
         {
            ch = '\n';
            Seek(fh, (LONG)lsp-p-1, OFFSET_CURRENT);
            p = lsp;
            Write(fh, &ch, 1);
         }
         if (isspace(ch)) lsp = p;
         if (ch == '\n') { sol = p+1; lsp = -1; }
         p++;
      }
      Close(fh);
   }
}
///
/// ReqFile
//  Puts up a file requester
int ReqFile(enum ReqFileType num, Object *win, char *title, int mode, char *drawer, char *file)
{
  // the following arrays depend on the ReqFileType enumeration
  static const char *pattern[MAXASL] =
  {
    "#?.addressbook#?",               // ASL_ABOOK
    "#?.config#?",                    // ASL_CONFIG
    NULL,                             // ASL_DETACH
    "~(#?.info)",                     // ASL_ATTACH
    "#?.(yam|rexx)",                  // ASL_REXX
    "#?.(gif|jpg|jpeg|png|iff|ilbm)", // ASL_PHOTO
    "#?.((mbx|eml|dbx|msg)|#?,#?)",   // ASL_IMPORT
    "#?.mbx",                         // ASL_EXPORT
    NULL                              // ASL_FOLDER
  };

   static BOOL init[MAXASL] =
   {
     FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
   };
   char *postext = hasSaveModeFlag(mode) ? GetStr(MSG_UT_Save) : GetStr(MSG_UT_Load);
   int skip = *file ? 1 : 2;
   struct Window *truewin = (struct Window *)xget(win, MUIA_Window_Window);

   if (!init[num]) { init[num] = TRUE; skip = 0; }

   return MUI_AslRequestTags( G->ASLReq[num],
                              ASLFR_Window,        truewin,
                              ASLFR_TitleText,     title,
                              ASLFR_PositiveText,  postext,
                              ASLFR_InitialFile,   file,
                              ASLFR_DoSaveMode,    hasSaveModeFlag(mode),
                              ASLFR_DoMultiSelect, hasMultiSelectFlag(mode),
                              ASLFR_DrawersOnly,   hasDrawersOnlyFlag(mode),
                              ASLFR_DoPatterns,    pattern[num] != NULL,
                              skip ? TAG_DONE : ASLFR_InitialDrawer, drawer,
                              ASLFR_InitialPattern, pattern[num] ? pattern[num] : "#?",
                              TAG_DONE
                            );
}
///
/// OpenTempFile
//  Creates or opens a temporary file
struct TempFile *OpenTempFile(char *mode)
{
   static int count = 0;
   struct TempFile *tf;
   if ((tf = calloc(1, sizeof(struct TempFile))))
   {
      // the tempfile MUST be SIZE_MFILE long because we
      // also use this tempfile routine for showing temporary mails which
      // conform to SIZE_MFILE
      char buf[SIZE_MFILE];

      // now format our temporary filename according to our Application data
      // this format tries to make the temporary filename kinda unique.
      snprintf(buf, sizeof(buf), "YAMt%d%02d.tmp", G->RexxHost->portnumber, ++count);

      // now add the temporary path to the filename
      strmfp(tf->Filename, C->TempDir, buf);

      if (!mode) return tf;
      if ((tf->FP = fopen(tf->Filename, mode))) return tf;

      // on error we free everything
      free(tf);
      count--;
   }
   return NULL;
}
///
/// CloseTempFile
//  Closes a temporary file
void CloseTempFile(struct TempFile *tf)
{
   if (tf)
   {
      if (tf->FP) fclose(tf->FP);
      D(DBF_UTIL, "DeleteTempFile: %s\n", tf->Filename);
      DeleteFile(tf->Filename);
      free(tf);
   }
}
///
/// DumpClipboard
//  Exports contents of clipboard unit 0 to a file
#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')
BOOL DumpClipboard(FILE *out)
{
   struct IFFHandle *iff;
   struct ContextNode *cn;
   long   error, rlen;
   UBYTE  readbuf[SIZE_DEFAULT];
   BOOL   success = FALSE;

   if ((iff = AllocIFF()))
   {
      if ((iff->iff_Stream = (ULONG)OpenClipboard(PRIMARY_CLIP)))
      {
         InitIFFasClip(iff);
         if (!OpenIFF(iff, IFFF_READ))
         {
            if (!StopChunk(iff, ID_FTXT, ID_CHRS)) while (TRUE)
            {
               error = ParseIFF(iff, IFFPARSE_SCAN);
               if (error == IFFERR_EOC) continue; else if (error) break;
               cn = CurrentChunk(iff);
               if (cn) if (cn->cn_Type == ID_FTXT && cn->cn_ID == ID_CHRS)
               {
                  success = TRUE;
                  while ((rlen = ReadChunkBytes(iff, readbuf, SIZE_DEFAULT)) > 0)
                     fwrite(readbuf, 1, (size_t)rlen, out);
               }
            }
            CloseIFF(iff);
         }
         CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
      }
      FreeIFF(iff);
   }
   return success;
}
///
/// IsFolderDir
//  Checks if a directory is used as a mail folder
static BOOL IsFolderDir(char *dir)
{
  char *filename = (char *)FilePart(dir);
  int i;

  for(i = 0; i < 4; i++)
  {
    if(!stricmp(filename, FolderNames[i]))
      return TRUE;
  }

  return (BOOL)(PFExists(dir, ".fconfig") || PFExists(dir, ".index"));
}
///
/// AllFolderLoaded
//  Checks if all folder index are correctly loaded
BOOL AllFolderLoaded(void)
{
   BOOL allloaded = TRUE;
   struct Folder **flist;

   if((flist = FO_CreateList()))
   {
      int i;

      for (i = 1; i <= (int)*flist; i++)
      {
        if (flist[i]->LoadedMode != LM_VALID && flist[i]->Type != FT_GROUP)
        {
          allloaded = FALSE;
          break;
        }
      }
      free(flist);
   }
   else return FALSE;

   return allloaded;
}
///
/// PFExists
//  Checks if a file exists in the specified directory
BOOL PFExists(char *path, char *file)
{
   char fname[SIZE_PATHFILE];
   strmfp(fname, path, file);
   return FileExists(fname);
}
///
/// DeleteMailDir
//  Recursively deletes a mail directory
BOOL DeleteMailDir(char *dir, BOOL isroot)
{
  BPTR dirLock;
  BOOL result = TRUE;

  if((dirLock = Lock(dir, ACCESS_READ)))
  {
    struct ExAllControl *eac;

    if((eac = AllocDosObject(DOS_EXALLCONTROL, NULL)))
    {
      struct ExAllData *ead;
      struct ExAllData *eabuffer;
      LONG more;
      eac->eac_LastKey = 0;
      eac->eac_MatchString = NULL;
      eac->eac_MatchFunc = NULL;

      if((eabuffer = malloc(SIZE_EXALLBUF)))
      {
        do
        {
          more = ExAll(dirLock, eabuffer, SIZE_EXALLBUF, ED_TYPE, eac);
          if(!more && IoErr() != ERROR_NO_MORE_ENTRIES)
          {
            result = FALSE;
            break;
          }

          if(eac->eac_Entries == 0)
            continue;

          ead = (struct ExAllData *)eabuffer;
          do
          {
            BOOL isdir = isDrawer(ead->ed_Type);
            char *filename = (char *)ead->ed_Name;
            char fname[SIZE_PATHFILE];

            strmfp(fname, dir, filename);

            if(isroot)
            {
              if(isdir)
              {
                if(IsFolderDir(fname))
                  result = DeleteMailDir(fname, FALSE);
              }
              else
              {
                if(stricmp(filename, ".config")      == 0 ||
                   stricmp(filename, ".glossary")    == 0 ||
                   stricmp(filename, ".addressbook") == 0 ||
                   stricmp(filename, ".emailcache")  == 0)
                {
                  result = DeleteFile(fname);
                }
              }
            }
            else if(!isdir && (isValidMailFile(filename) ||
                    stricmp(filename, ".fconfig") == 0   ||
                    stricmp(filename, ".index") == 0)
                   )
            {
              result = DeleteFile(fname);
            }
          }
          while((ead = ead->ed_Next) && result);
        }
        while(more && result);

        free(eabuffer);
      }
      else result = FALSE;

      FreeDosObject(DOS_EXALLCONTROL, eac);
    }
    else result = FALSE;

    UnLock(dirLock);

    if(result)
      result = DeleteFile(dir);
  }
  else result = FALSE;

  return result;
}
///
/// FileToBuffer
//  Reads a complete file into memory
char *FileToBuffer(char *file)
{
  FILE *fh;
  char *text = NULL;
  int size = FileSize(file);

  if(size >= 0 && (text = malloc((size+1)*sizeof(char))))
  {
    text[size] = '\0'; // NUL-terminate the string

    if((fh = fopen(file, "r")))
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

  return text;
}
///
/// FileCount()
// returns the total number of files that are in a directory
// or -1 if an error occurred
long FileCount(char *directory)
{
  BPTR dirLock;
  long result = 0;

  if((dirLock = Lock(directory, ACCESS_READ)))
  {
    struct ExAllControl *eac;

    if((eac = AllocDosObject(DOS_EXALLCONTROL, NULL)))
    {
      struct ExAllData *eabuffer;
      LONG more;
      eac->eac_LastKey = 0;
      eac->eac_MatchString = NULL;
      eac->eac_MatchFunc = NULL;

      if((eabuffer = malloc(SIZE_EXALLBUF)))
      {
        do
        {
          more = ExAll(dirLock, eabuffer, SIZE_EXALLBUF, ED_NAME, eac);
          if(!more && IoErr() != ERROR_NO_MORE_ENTRIES)
          {
            result = -1;
            break;
          }

          // count the entries
          result += eac->eac_Entries;
        }
        while(more);

        free(eabuffer);
      }
      else result = -1;

      FreeDosObject(DOS_EXALLCONTROL, eac);
    }
    else result = -1;

    UnLock(dirLock);
  }
  else result = -1;

  return result;
}
///

/*** Mail related ***/
/// MyRemove
//  Removes a message from a message list
static void MyRemove(struct Mail **list, struct Mail *rem)
{
   struct Mail *mail;
   if (*list == rem) { *list = rem->Next; return; }
   for (mail = *list; mail->Next; mail = mail->Next)
      if (mail->Next == rem) { mail->Next = rem->Next; return; }
}
///
/// CreateFilename
//  Prepends mail directory to a file name
char *CreateFilename(const char * const file)
{
   static char buffer[SIZE_PATHFILE];
   strmfp(buffer, G->MA_MailDir, file);
   return buffer;
}
///
/// CreateDirectory
//  Makes a directory
BOOL CreateDirectory(char *dir)
{
   int t = FileType(dir);
   if (t == 2) return TRUE;
   if (t == 0)
   {
      BPTR fl = CreateDir((STRPTR)dir);
      if (fl)
      {
         UnLock(fl);
         return TRUE;
      }
   }
   if (G->MA) ER_NewError(GetStr(MSG_ER_CantCreateDir), dir);
   return FALSE;
}
///
/// GetFolderDir
//  Returns path of a folder directory
char *GetFolderDir(struct Folder *fo)
{
   static char buffer[SIZE_PATH];
   if (strchr(fo->Path, ':')) return fo->Path;
   strmfp(buffer, G->MA_MailDir, fo->Path);
   return buffer;
}
///
/// GetMailFile
//  Returns path of a message file
char *GetMailFile(char *string, struct Folder *folder, struct Mail *mail)
{
  static char buffer[SIZE_PATHFILE];

  if(!folder && mail)
    folder = mail->Folder;

  if(!string)
    string = buffer;

  strmfp(string, (folder == NULL || folder == (struct Folder *)-1) ? C->TempDir : GetFolderDir(folder), mail->MailFile);

  return string;
}
///
/// GetReturnAddress
//  Gets return address of a message
struct Person *GetReturnAddress(struct Mail *mail)
{
   if (mail->ReplyTo.Address[0]) return &mail->ReplyTo;
   return &mail->From;
}
///
/// BuildAddrName
//  Creates "Real Name" <E-mail> string
char *BuildAddrName(char *address, char *name)
{
  static char buffer[SIZE_ADDRESS+SIZE_REALNAME+4];
  char *delim;

  if(*name)
  {
    if(strpbrk(name, ",.()"))
      delim = "\"";
    else
      delim = "";

    snprintf(buffer, sizeof(buffer), "%s%s%s <%s>", delim, name, delim, address);
  }
  else
    snprintf(buffer, sizeof(buffer), "%s", address);

  return buffer;
}
///
/// ExtractAddress
//  Extracts e-mail address and real name
void ExtractAddress(char *line, struct Person *pe)
{
   char *p = line, *ra[4], *save;
   BOOL found = FALSE;

   ra[2] = ra[3] = NULL;
   save = strdup(line);
   pe->Address[0] = pe->RealName[0] = 0;
   while (isspace(*p)) p++;
   if ((ra[0] = MyStrChr(p,'<'))) if ((ra[1] = MyStrChr(ra[0],'>')))
   {
      *ra[0]++ = 0; *ra[1] = 0;
      for (ra[2] = p, ra[3] = ra[0]-2; isspace(*ra[3]) && ra[3] >= ra[2]; ra[3]--) *ra[3] = 0;
      found = TRUE;
   }
   if (!found)
   {
      for (ra[1] = ra[0] = p; *ra[1] && *ra[1] != '\t' && *ra[1] != ' ' && *ra[1] != ','; ra[1]++);
      if ((ra[2] = MyStrChr(ra[1],'('))) if ((ra[3] = MyStrChr(ra[2],')')))
      {
         ra[2]++; *ra[3]-- = 0;
         found = TRUE;
      }
      *ra[1] = 0;
      if (!found) ra[2] = ra[3] = "";
   }

   if (*ra[2] == '\"') ra[2]++;
   if (*ra[3] == '\"' && *(ra[3]-1) != '\\') *ra[3] = 0;

   strlcpy(pe->Address ,Trim(ra[0]), sizeof(pe->Address));
   strlcpy(pe->RealName, Trim(ra[2]), sizeof(pe->RealName));

   strcpy(line, save);

   free(save);
}
///
/// CompressMsgID
//  Creates a crc32 checksum of the MsgID, so that it can be used later
//  for the follow-up algorithms aso.
ULONG CompressMsgID(char *msgid)
{
  // if the MsgID is valid we calculate the CRC32 checksum and as it
  // consists only of one cycle through the crc function we call it
  // with -1
  return msgid && *msgid ? CRC32(msgid, strlen(msgid), -1L) : 0;
}
///
/// DescribeCT
//  Returns description of a content type
char *DescribeCT(const char *ct)
{
  char *ret = (char *)ct;

  ENTER();

  if(ct == NULL)
    ret = GetStr(MSG_CTunknown);
  else
  {
    struct MinNode *curNode;

    // first we search through the users' own MIME type list
    for(curNode = C->mimeTypeList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
    {
      struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;

      if(MatchNoCase(ct, mt->ContentType) && mt->Description[0] != '\0')
      {
        ret = mt->Description;
        break;
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
          ret = GetStr(IntMimeTypeArray[i].Description);
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

   // get the actual time
   DateStamp(&ds);

   return (ds.ds_Days*24*60*60 + ds.ds_Minute*60 + ds.ds_Tick/TICKS_PER_SECOND);
}
///
/// DateStampUTC
//  gets the current system time in UTC
void DateStampUTC(struct DateStamp *ds)
{
  DateStamp(ds);
  DateStampTZConvert(ds, TZC_UTC);
}
///
/// GetSysTimeUTC
//  gets the actual system time in UTC
void GetSysTimeUTC(struct TimeVal *tv)
{
  GetSysTime(TIMEVAL(tv));
  TimeValTZConvert(tv, TZC_UTC);
}
///
/// TimeValTZConvert
//  converts a supplied timeval depending on the TZConvert flag to be converted
//  to/from UTC
void TimeValTZConvert(struct TimeVal *tv, enum TZConvert tzc)
{
  if(tzc == TZC_LOCAL)
    tv->Seconds += (C->TimeZone+C->DaylightSaving*60)*60;
  else if(tzc == TZC_UTC)
    tv->Seconds -= (C->TimeZone+C->DaylightSaving*60)*60;
}
///
/// DateStampTZConvert
//  converts a supplied DateStamp depending on the TZConvert flag to be converted
//  to/from UTC
void DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc)
{
  // convert the DateStamp from local -> UTC or visa-versa
  if(tzc == TZC_LOCAL)    ds->ds_Minute += (C->TimeZone+C->DaylightSaving*60);
  else if(tzc == TZC_UTC) ds->ds_Minute -= (C->TimeZone+C->DaylightSaving*60);

  // we need to check the datestamp variable that it is still in it`s borders
  // after the UTC correction
  while(ds->ds_Minute < 0)     { ds->ds_Minute += 1440; ds->ds_Days--; }
  while(ds->ds_Minute >= 1440) { ds->ds_Minute -= 1440; ds->ds_Days++; }
}
///
/// TimeVal2DateStamp
//  converts a struct TimeVal to a struct DateStamp
void TimeVal2DateStamp(const struct TimeVal *tv, struct DateStamp *ds, enum TZConvert tzc)
{
  LONG seconds = (tv->Seconds+(tv->Microseconds/1000000));

  ds->ds_Days   = seconds/86400;       // calculate the days since 1.1.1978
  ds->ds_Minute = (seconds%86400)/60;
  ds->ds_Tick   = (tv->Seconds%60)*TICKS_PER_SECOND + (tv->Microseconds/20000);

  // if we want to convert from/to UTC we need to do this now
  if(tzc != TZC_NONE)
    DateStampTZConvert(ds, tzc);
}
///
/// DateStamp2TimeVal
//  converts a struct DateStamp to a struct TimeVal
void DateStamp2TimeVal(const struct DateStamp *ds, struct TimeVal *tv, enum TZConvert tzc)
{
  // check if the ptrs are set or not.
  if(ds == NULL || tv == NULL)
    return;

  // creates wrong timevals from DateStamps with year >= 2114 ...
  tv->Seconds = (ds->ds_Days*24*60 + ds->ds_Minute)*60 + ds->ds_Tick/TICKS_PER_SECOND;
  tv->Microseconds = (ds->ds_Tick % TICKS_PER_SECOND) * 1000000/TICKS_PER_SECOND;

  // if we want to convert from/to UTC we need to do this now
  if(tzc != TZC_NONE)
    TimeValTZConvert(tv, tzc);
}
///
/// TimeVal2String
//  Converts a timeval structure to a string with using DateStamp2String after a convert
BOOL TimeVal2String(char *dst, int dstlen, const struct TimeVal *tv, enum DateStampType mode, enum TZConvert tzc)
{
  struct DateStamp ds;

  // convert the timeval into a datestamp
  TimeVal2DateStamp(tv, &ds, TZC_NONE);

  // then call the DateStamp2String() function to get the real string
  return DateStamp2String(dst, dstlen, &ds, mode, tzc);
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
  dt.dat_Flags   = 0; // perhaps later we can add Weekday substitution
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
    {
      // calculate the beat time
      LONG beat = (((date->ds_Minute-C->TimeZone+(C->DaylightSaving?0:60)+1440)%1440)*1000)/1440;

      if(mode == DSS_DATEBEAT)
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
/// DateStamp2RFCString()
BOOL DateStamp2RFCString(char *dst, int dstlen, struct DateStamp *date, int timeZone, BOOL convert)
{
  struct DateStamp curDateStamp;
  struct ClockData cd;
  time_t seconds;
  int convertedTimeZone = (timeZone/60)*100 + (timeZone%60);

  ENTER();

  // if date == NULL we get the current time/date
  if(date == NULL)
    DateStamp(&curDateStamp);
  else
    memcpy(&curDateStamp, date, sizeof(struct DateStamp));

  // point at curDateStamp
  date = &curDateStamp;

  // if the user wants to convert the datestamp we have to make sure we
  // substract/add the timeZone
  if(convert && timeZone != 0)
  {
    date->ds_Minute += timeZone;

    // we need to check the datestamp variable that it is still in it`s borders
    // after adjustment
    while(date->ds_Minute < 0)     { date->ds_Minute += 1440; date->ds_Days--; }
    while(date->ds_Minute >= 1440) { date->ds_Minute -= 1440; date->ds_Days++; }
  }

  // lets form the seconds now for the Amiga2Date function
  seconds = (date->ds_Days*24*60*60 + date->ds_Minute*60 + date->ds_Tick/TICKS_PER_SECOND);

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
/// String2DateStamp()
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
            strlcpy(timestr, p+1, 8);

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
    {
      char *p;

      // copy the datestring
      if((p = strchr(string, ' ')))
      {
        strlcpy(datestr, string, p-string+1);
        strlcpy(timestr, p+1, sizeof(timestr));

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
/// String2TimeVal()
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
     char *TZname;
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
          D(DBF_UTIL, "TZtoMinutes: abbreviation '%s' NOT found!", tzone);
      }
   }

   return tzcorr == -1 ? 0 : (tzcorr/100)*60 + (tzcorr%100);
}
///
/// FormatSize
//  Displays large numbers using group separators
void FormatSize(LONG size, char *buf, int buflen)
{
  char *dp = G->Locale ? (char *)G->Locale->loc_DecimalPoint : ".";
  double dsize = (double)size;

  // we check what SizeFormat the user has choosen
  switch(C->SizeFormat)
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
    default:
    {
      char *gs = G->Locale ? (char *)G->Locale->loc_GroupSeparator : ",";

      // as we just split the size to another value, we redefine the KB/MB/GB values to base 10 variables
      enum { KB = 1000, MB = 1000 * 1000, GB = 1000 * 1000 * 1000 };

      if(size < KB)      snprintf(buf, buflen, "%ld", size);
      else if(size < MB) snprintf(buf, buflen, "%ld%s%03ld", size/KB, gs, size%KB);
      else if(size < GB) snprintf(buf, buflen, "%ld%s%03ld%s%03ld", size/MB, gs, (size%MB)/KB, gs, size%KB);
      else               snprintf(buf, buflen, "%ld%s%03ld%s%03ld%s%03ld", size/GB, gs, (size%GB)/MB, gs, (size%MB)/KB, gs, size%KB);
    }
    break;
  }
}
///
/// MailExists
//  Checks if a message still exists
BOOL MailExists(struct Mail *mailptr, struct Folder *folder)
{
   struct Mail *work;
   if (isVirtualMail(mailptr)) return TRUE;
   if (!folder) folder = mailptr->Folder;
   for (work = folder->Messages; work; work = work->Next) if (work == mailptr) return TRUE;
   return FALSE;
}
///
/// DisplayMailList
//  Lists folder contents in the message listview
void DisplayMailList(struct Folder *fo, APTR lv)
{
   struct Mail *work, **array;
   int lastActive = fo->LastActive;

   if ((array = (struct Mail **)calloc(fo->Total+1,sizeof(struct Mail *))))
   {
      int i = 0;

      BusyText(GetStr(MSG_BusyDisplayingList), "");
      for (work = fo->Messages; work; work = work->Next)
      {
         array[i++] = work;
      }

      // We do not encapsulate this Clear&Insert with a NList_Quiet because
      // this will speed up the Insert with about 3-4 seconds for ~6000 items
      DoMethod(lv, MUIM_NList_Clear);
      DoMethod(lv, MUIM_NList_Insert, array, fo->Total, MUIV_NList_Insert_Sorted,
                   C->AutoColumnResize ? MUIF_NONE : MUIV_NList_Insert_Flag_Raw);

      free(array);
      BusyEnd();
   }

   // Now we have to recove the LastActive or otherwise it will be -1 later
   fo->LastActive = lastActive;
}
///
/// AddMailToList
//  Adds a message to a folder
struct Mail *AddMailToList(struct Mail *mail, struct Folder *folder)
{
   struct Mail *new = malloc(sizeof(struct Mail));
   if (new)
   {
      memcpy(new, mail, sizeof(struct Mail));
      new->Folder = folder;

      // lets add the new Message to our message list
      new->Next = folder->Messages;
      folder->Messages = new;

      // lets summarize the stats
      folder->Total++;
      folder->Size += mail->Size;

      if(hasStatusNew(mail))
      {
        folder->New++;
        folder->Unread++;
      }
      else if(!hasStatusRead(mail))
      {
        folder->Unread++;
      }

      MA_ExpireIndex(folder);
   }
   return new;
}
///
/// RemoveMailFromList
//  Removes a message from a folder
void RemoveMailFromList(struct Mail *mail)
{
  struct Folder *folder = mail->Folder;

  // now we remove the mail from main mail
  // listviews in case the folder of it is the
  // currently active one.
  if(folder == FO_GetCurrentFolder())
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RemoveMail, mail);

  // lets decrease the folder statistics first
  folder->Total--;
  folder->Size -= mail->Size;

  if(hasStatusNew(mail))
  {
    folder->New--;
    folder->Unread--;
  }
  else if(!hasStatusRead(mail))
  {
    folder->Unread--;
  }

  // remove the mail from the folderlist now
  MyRemove(&(folder->Messages), mail);

  // then we have to mark the folder index as expired so
  // that it will be saved next time.
  MA_ExpireIndex(folder);

  // and last, but not least we have to free the mail
  free(mail);
}
///
/// ClearMailList
//  Removes all messages from a folder
void ClearMailList(struct Folder *folder, BOOL resetstats)
{
   struct Mail *work, *next;

   for(work = folder->Messages; work; work = next)
   {
      next = work->Next;
      free(work);
   }

   if(resetstats)
     folder->Total = folder->New = folder->Unread = folder->Size = 0;

   folder->Messages = NULL;
}
///
/// GetPackMethod
//  Returns packer type and efficiency
static BOOL GetPackMethod(enum FolderMode fMode, char **method, int *eff)
{
   BOOL result = TRUE;

   switch(fMode)
   {
      case FM_XPKCOMP:
        *method = C->XPKPack;
        *eff = C->XPKPackEff;
      break;

      case FM_XPKCRYPT:
        *method = C->XPKPackEncrypt;
        *eff = C->XPKPackEncryptEff;
      break;

      default:
        *method = NULL;
        *eff = 0;
        result = FALSE;
   }

   return result;
}
///
/// CompressMailFile
//  Shrinks a message file
static BOOL CompressMailFile(char *src, char *dst, char *passwd, char *method, int eff)
{
   D(DBF_UTIL, "CompressMailFile: %08lx - [%s] -> [%s] - [%s] - [%s] - %ld", XpkBase, src, dst, passwd, method, eff);

   if(!XpkBase)
     return FALSE;

   return (BOOL)!XpkPackTags(XPK_InName,      src,
                             XPK_OutName,     dst,
                             XPK_Password,    passwd,
                             XPK_PackMethod,  method,
                             XPK_PackMode,    eff,
                             TAG_DONE);
}
///
/// UncompressMailFile
//  Expands a compressed message file
static BOOL UncompressMailFile(char *src, char *dst, char *passwd)
{
   D(DBF_UTIL, "UncompressMailFile: %08lx - [%s] -> [%s] - [%s]", XpkBase, src, dst, passwd);

   if(!XpkBase)
      return FALSE;

   return (BOOL)!XpkUnpackTags(XPK_InName,    src,
                               XPK_OutName,   dst,
                               XPK_Password,  passwd,
                               TAG_DONE);
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
   int success = 0;

   D(DBF_UTIL, "TransferMailFile: %d->%d [%s]->[%s]", srcMode, dstMode, mail->MailFile, GetFolderDir(dstfolder));

   if(!MA_GetIndex(srcfolder))
     return 0;

   if(!MA_GetIndex(dstfolder))
     return 0;

   // get some information we require
   GetPackMethod(dstMode, &pmeth, &peff);
   GetMailFile(srcbuf, srcfolder, mail);

   // check if we can just take the exactly same filename in the destination
   // folder or if we require to increase the mailfile counter to make it
   // unique
   strlcpy(dstFileName, mail->MailFile, sizeof(dstFileName));
   strlcpy(dstbuf, GetFolderDir(dstfolder), sizeof(dstbuf));
   AddPart(dstbuf, dstFileName, SIZE_PATHFILE);

   if(FileExists(dstbuf))
   {
     int mCounter = atoi(&dstFileName[13]);

     do
     {
       if(mCounter < 1 || mCounter >= 999)
         return 0;

       mCounter++;

       snprintf(&dstFileName[13], sizeof(dstFileName)-13, "%03d", mCounter);
       dstFileName[16] = ','; // restore it

       strlcpy(dstbuf, GetFolderDir(dstfolder), sizeof(dstbuf));
       AddPart(dstbuf, dstFileName, SIZE_PATHFILE);
     }
     while(FileExists(dstbuf));

     // if we end up here we finally found a new mailfilename which we can use, so
     // lets copy it to our MailFile variable
     strlcpy(mail->MailFile, dstFileName, sizeof(mail->MailFile));
   }

   // now that we have the source and destination filename
   // we can go and do the file operation depending on some data we
   // acquired earlier
   if((srcMode == dstMode && srcfolder->Mode <= FM_SIMPLE) ||
      (srcfolder->Mode <= FM_SIMPLE && dstfolder->Mode <= FM_SIMPLE))
   {
      if(copyit)
         success = CopyFile(dstbuf, 0, srcbuf, 0) ? 1 : -1;
      else
         success = MoveFile(srcbuf, dstbuf) ? 1 : -1;
   }
   else if(isXPKFolder(srcfolder))
   {
      if(!isXPKFolder(dstfolder))
      {
         // if we end up here the source folder is a compressed folder but the
         // destination one not. so lets uncompress it
         if((success = UncompressMailFile(srcbuf, dstbuf, srcpw) ? 1 : -2) > 0)
         {
            if(!copyit)
            {
               success = DeleteFile(srcbuf) ? 1 : -1;
            }
         }
      }
      else
      {
         // here the source folder is a compressed+crypted folder and the
         // destination one also, so we have to uncompress the file to a
         // temporarly file and compress it immediatly with the destination
         // password again.
         struct TempFile *tf = OpenTempFile(NULL);

         if(tf)
         {
            if((success = UncompressMailFile(srcbuf, tf->Filename, srcpw) ? 1 : -2) > 0)
            {
               // compress it immediatly again
               if((success = CompressMailFile(tf->Filename, dstbuf, dstpw, pmeth, peff) ? 1 : -2) > 0)
               {
                  if(!copyit)
                  {
                     success = DeleteFile(srcbuf) ? 1 : -1;
                  }
               }
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
         if((success = CompressMailFile(srcbuf, dstbuf, dstpw, pmeth, peff) ? 1 : -2) > 0)
         {
            if(!copyit)
            {
               success = DeleteFile(srcbuf) ? 1 : -1;
            }
         }
      }
      else
      {
        // if we end up here then there is something seriously wrong
        success = -3;
      }
   }

   return success;
}
///
/// RepackMailFile
//  (Re/Un)Compresses a message file
//  Note: If dstMode is -1 and passwd is NULL, then this function packs
//        the current mail. It will assume it is plaintext and needs to be packed now
BOOL RepackMailFile(struct Mail *mail, enum FolderMode dstMode, char *passwd)
{
   char *pmeth = NULL, srcbuf[SIZE_PATHFILE], dstbuf[SIZE_PATHFILE];
   struct Folder *folder = mail->Folder;
   int peff = 0;
   enum FolderMode srcMode = folder->Mode;
   BOOL success = FALSE;

   ENTER();

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
            DeleteFile(srcbuf))
         {
            success = RenameFile(dstbuf, srcbuf);
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
           success = DeleteFile(dstbuf);
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
            DeleteFile(srcbuf))
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

   if(GetPackMethod(folder->Mode, &pmeth, &peff))
   {
     if(CompressMailFile(file, newfile, folder->Password, pmeth, peff) &&
        DeleteFile(file))
     {
        return TRUE;
     }
   }

   return FALSE;
}
///
/// StartUnpack
//  Unpacks a file to a temporary file
char *StartUnpack(char *file, char *newfile, struct Folder *folder)
{
   FILE *fh;
   BOOL xpk = FALSE;
   if ((fh = fopen(file, "r")))
   {
      if (fgetc(fh) == 'X') if (fgetc(fh) == 'P') if (fgetc(fh) == 'K') xpk = TRUE;
      fclose(fh);
      if (xpk)
      {
         char nfile[SIZE_FILE];

         snprintf(nfile, sizeof(nfile), "%s_%08lx.unp", FilePart(file), (ULONG)folder);

         strmfp(newfile, C->TempDir, nfile);
         if (FileSize(newfile) < 0) if (!UncompressMailFile(file, newfile, folder ? folder->Password : "")) return NULL;
      }
      else
        strcpy(newfile, file);

      return newfile;
   }
   return NULL;
}
///
/// FinishUnpack
//  Deletes temporary unpacked file
void FinishUnpack(char *file)
{
  // we just delete if this is really related to a unpack file
  if(strstr(file, ".unp"))
  {
    if(IsMinListEmpty(&G->readMailDataList) == FALSE)
    {
      // search through our ReadDataList
      struct MinNode *curNode;
      for(curNode = G->readMailDataList.mlh_Head; curNode->mln_Succ; curNode = curNode->mln_Succ)
      {
        struct ReadMailData *rmData = (struct ReadMailData *)curNode;
        if(stricmp(file, rmData->readFile) == 0)
          return;
      }
    }

    DeleteFile(file);
  }
}
///

/*** Editor related ***/
/// EditorToFile
//  Saves contents of a texteditor object to a file
BOOL EditorToFile(Object *editor, char *file)
{
  FILE *fh;
  BOOL result = FALSE;

  if((fh = fopen(file, "w")))
  {
    char *text = (char *)DoMethod((Object *)editor, MUIM_TextEditor_ExportText);

    // write out the whole text to the file
    if(fwrite(text, strlen(text), 1, fh) == 1)
      result = TRUE;

    FreeVec(text); // use FreeVec() because TextEditor.mcc uses AllocVec()
    fclose(fh);
  }

  return result;
}
///
/// FileToEditor
//  Loads a file into a texteditor object
BOOL FileToEditor(char *file, Object *editor)
{
  char *text = FileToBuffer(file);
  BOOL res = FALSE;

  if(text)
  {
    char *parsedText;

    // Parse the text and do some highlighting and stuff
    if((parsedText = ParseEmailText(text)))
    {
      set(editor, MUIA_TextEditor_Contents, parsedText);
      free(parsedText);

      res = TRUE;
    }

    free(text);
  }

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

  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
  if(var)
  {
    char buf[SIZE_SMALL];
    snprintf(buf, sizeof(buf), "0x%s", var);
    setstring(string, buf);
  }
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

   secret = str = (char *)xget(pop, MUIA_UserData);
   if (G->PGPVersion == 5)
   {
      retc = PGPCommand("pgpk", "-l +language=us", KEEPLOG);
   }
   else
   {
      strlcpy(buf, "-kv  ", sizeof(buf));
      if (secret)
      {
         GetVar("PGPPATH", &buf[4], SIZE_DEFAULT, 0);
         if((p = buf[strlen(buf)-1]) != ':' && p != '/')
           strlcat(buf, "/", sizeof(buf));

         strlcat(buf, "secring.pgp", sizeof(buf));
      }
      retc = PGPCommand("pgp", buf, KEEPLOG);
   }

   if(!retc && (fp = fopen(PGPLOGFILE, "r")))
   {
      str = (char *)xget(string, MUIA_String_Contents);
      DoMethod(pop, MUIM_List_Clear);

      while (GetLine(fp, buf, sizeof(buf)))
      {
         char entry[SIZE_DEFAULT];
         memset(entry, 0, SIZE_DEFAULT);
         if (G->PGPVersion == 5)
         {
            if (!strncmp(buf, "sec", 3) || (!strncmp(&buf[1], "ub", 2) && !secret))
            {
               memcpy(entry, &buf[12], 8);

               while (GetLine(fp, buf, sizeof(buf)))
               {
                 if(!strncmp(buf, "uid", 3))
                 {
                   strlcat(entry, &buf[4], sizeof(entry)-9);
                   break;
                 }
               }
            }
         }
         else
         {
            if (buf[9] == '/' && buf[23] == '/')
            {
               memcpy(entry, &buf[10], 8);
               strlcat(entry, &buf[29], sizeof(entry)-8);
            }
         }
         if (*entry)
         {
            DoMethod(pop, MUIM_List_InsertSingle, entry, MUIV_List_Insert_Bottom);
            if (!strncmp(entry, str, 8)) set(pop, MUIA_List_Active, keys);
            keys++;
         }
      }
      fclose(fp);
      DeleteFile(PGPLOGFILE);
   }
   if (!keys) ER_NewError(GetStr(MSG_ER_NoPublicKeys), "", "");
   return keys > 0;
}
MakeHook(PO_ListPublicKeysHook, PO_ListPublicKeys);
///

/*** MUI related ***/
/// ShortCut
//  Finds keyboard shortcut in text label
char ShortCut(char *label)
{
   char *ptr = strchr(label, '_');
   if (!ptr) return 0;
   return (char)ToLower((ULONG)(*++ptr));
}
///
/// RemoveCut
//  Removes shortcut character from text label
#if 0
static char *RemoveCut(char *label)
{
   static char lab[SIZE_DEFAULT];
   char *p;

   for (p = lab; *label; label++) if (*label != '_') *p++ = *label;
   *p = 0;
   return lab;
}
#endif
///
/// MakeCycle
//  Creates a MUI cycle object
Object *MakeCycle(char **labels, char *label)
{
   Object *obj = KeyCycle(labels, ShortCut(label));
   if (obj) set(obj, MUIA_CycleChain, 1);
   return obj;
}
///
/// MakeButton
//  Creates a MUI button
Object *MakeButton(char *txt)
{
   Object *obj = MUI_MakeObject(MUIO_Button,txt);
   if (obj) set(obj, MUIA_CycleChain, 1);
   return obj;
}
///
/// MakeCheck
//  Creates a MUI checkmark object
Object *MakeCheck(char *label)
{
   return
   ImageObject,
      ImageButtonFrame,
      MUIA_InputMode   , MUIV_InputMode_Toggle,
      MUIA_Image_Spec  , MUII_CheckMark,
      MUIA_Background  , MUII_ButtonBack,
      MUIA_ShowSelState, FALSE,
      MUIA_ControlChar , ShortCut(label),
      MUIA_CycleChain  , 1,
   End;
}
///
/// MakeCheckGroup
//  Creates a labelled MUI checkmark object
Object *MakeCheckGroup(Object **check, char *label)
{
   return
   HGroup,
      Child, *check = MakeCheck(label),
      Child, Label1(label),
      Child, HSpace(0),
   End;
}
///
/// MakeString
//  Creates a MUI string object
Object *MakeString(int maxlen, char *label)
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
Object *MakePassString(char *label)
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
Object *MakeInteger(int maxlen, char *label)
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
Object *MakePGPKeyList(Object **st, BOOL secret, char *label)
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
Object *MakeAddressField(Object **string, char *label, APTR help, int abmode, int winnum, BOOL allowmulti)
{
   Object *obj, *bt_adr;

   if ((obj = HGroup,
      GroupSpacing(1),
      Child, *string = RecipientstringObject,
         StringFrame,
         MUIA_CycleChain,                          TRUE,
         MUIA_String_AdvanceOnCR,                  TRUE,
         MUIA_Recipientstring_ResolveOnCR,         TRUE,
         MUIA_Recipientstring_MultipleRecipients,  allowmulti,
         MUIA_ControlChar, ShortCut(label),
      End,
      Child, bt_adr = PopButton(MUII_PopUp),
   End))
   {
      SetHelp(*string,help);
      SetHelp(bt_adr, MSG_HELP_WR_BT_ADR);
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnum);
      DoMethod(*string, MUIM_Notify, MUIA_Recipientstring_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnum);
      DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime,  bt_adr, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
   }

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
/// MakeMenuitem
//  Creates a menu item from a catalog string
Object *MakeMenuitem(const char *str, ULONG ud)
{
  if(str == NULL)
    return MenuitemObject,
             MUIA_Menuitem_Title, NM_BARLABEL,
           End;

  if(str[1] == '\0')
    return MenuitemObject,
             MUIA_Menuitem_Title,    str+2,
             MUIA_Menuitem_Shortcut, str,
             MUIA_UserData,          ud,
           End;

  return MenuitemObject,
           MUIA_Menuitem_Title, str,
           MUIA_UserData,       ud,
         End;
}
///
/// SetupToolbar
//  Initializes a single button in a MUI toolbar object
void SetupToolbar(struct MUIP_Toolbar_Description *tb, char *label, char *help, ULONG flags)
{
   tb->Type = label ? (*label ? TDT_BUTTON : TDT_SPACE) : TDT_END;
   tb->Flags = flags;
   tb->ToolText = tb->Type == TDT_BUTTON ? label : NULL;
   tb->HelpString = help;
   tb->MutualExclude = 0;
   tb->Key = 0;
}
///
/// SetupMenu
//  Initializes a MUI menu item
void SetupMenu(int type, struct NewMenu *menu, char *label, char *shortcut, int id)
{
   menu->nm_Type = type;
   menu->nm_Label = (STRPTR)label;
   menu->nm_CommKey = (STRPTR)shortcut;
   menu->nm_Flags = 0;
   menu->nm_MutualExclude = 0;
   menu->nm_UserData = (APTR)id;
}
///
/// DoSuperNew
//  Calls parent NEW method within a subclass
#if !defined(__MORPHOS__)
Object * STDARGS VARARGS68K DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  Object *rc;
  VA_LIST args;

  VA_START(args, obj);
  rc = (Object *)DoSuperMethod(cl, obj, OM_NEW, VA_ARG(args, ULONG), NULL);
  VA_END(args);

  return rc;
}
#endif
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
  ENTER();

  // make sure we open the window object
  set(obj, MUIA_Window_Open, TRUE);

  // now we check wheter the window was successfully
  // open or the application has been in iconify state
  if(xget(obj, MUIA_Window_Open) == TRUE ||
     xget(_app(obj), MUIA_Application_Iconified) == TRUE)
  {
    RETURN(TRUE);
    return TRUE;
  }

  // otherwise we perform a DisplaBeep()
  DisplayBeep(0);

  RETURN(FALSE);
  return FALSE;
}
///
/// DisposeModule
// Free resources of a MUI window
void DisposeModule(void *modptr)
{
   struct UniversalClassData **module = (struct UniversalClassData **)modptr;
   if (*module)
   {
      APTR window = (*module)->GUI.WI;
      set(window, MUIA_Window_Open, FALSE);
      DoMethod(G->App, OM_REMMEMBER, window);
      MUI_DisposeObject(window);
      free(*module);
      *module = NULL;
   }
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
   char *ls;
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
   if(!endptr || endptr == ls)
      G->Weights[0] = 30;

   ls = endptr;
   G->Weights[1] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[1] = 100;

   ls = endptr;
   G->Weights[2] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[2] = 25;

   ls = endptr;
   G->Weights[3] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[3] = 100;

   ls = endptr;
   G->Weights[4] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[4] = 30;

   ls = endptr;
   G->Weights[5] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[5] = 100;

   ls = endptr;
   G->Weights[6] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[6] = 25;

   ls = endptr;
   G->Weights[7] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[7] = 100;

   ls = endptr;
   G->Weights[8] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[8] = 5;

   ls = endptr;
   G->Weights[9] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[9] = 100;

   ls = endptr;
   G->Weights[10] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[10] = 5;

   ls = endptr;
   G->Weights[11] = strtol(ls, &endptr, 10);
   if(!endptr || endptr == ls)
      G->Weights[11] = 100;

   // lets set the weight factors to the corresponding GUI elements now
   // if they exist
   set(G->MA->GUI.LV_FOLDERS,  MUIA_HorizWeight, G->Weights[0]);
   set(G->MA->GUI.GR_MAILVIEW, MUIA_HorizWeight, G->Weights[1]);
   set(G->MA->GUI.PG_MAILLIST, MUIA_VertWeight,  G->Weights[6]);

   // if the embedded read pane is active we set its weight values
   if(C->EmbeddedReadPane)
   {
     SetAttrs(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_VertWeight,                 G->Weights[7],
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
   if(permanent)
   {
      struct Process *pr = (struct Process *)FindTask(NULL);
      APTR oldWindowPtr = pr->pr_WindowPtr;

      // this is for the people out there having their SYS: partition locked and whining about
      // YAM popping up a error requester upon the exit - so it`s their fault now if
      // the MUI objects aren`t saved correctly.
      pr->pr_WindowPtr = (APTR)-1;

      DoMethod(G->App, MUIM_Application_Save, MUIV_Application_Save_ENVARC);

      pr->pr_WindowPtr = oldWindowPtr; // restore the old windowPtr
   }

   LEAVE();
}
///
/// ConvertKey
//  Converts input event to key code
ULONG ConvertKey(struct IntuiMessage *imsg)
{
  struct InputEvent event;
  unsigned char code = 0;

  event.ie_NextEvent    = NULL;
  event.ie_Class        = IECLASS_RAWKEY;
  event.ie_SubClass     = 0;
  event.ie_Code         = imsg->Code;
  event.ie_Qualifier    = imsg->Qualifier;
  event.ie_EventAddress = (APTR *) *((ULONG *)imsg->IAddress);

  MapRawKey(&event, (STRPTR)&code, 1, NULL);

  return code;
}
///
/// isChildOfGroup()
// return TRUE if the supplied child object is part of the supplied group
BOOL isChildOfGroup(Object *group, Object *child)
{
  struct List *child_list;
  Object *curchild;
  Object *cstate;

  // get the child list of the group object
  child_list = (struct List *)xget(group, MUIA_Group_ChildList);
  if(child_list == NULL)
    return FALSE;

  // here we check whether the child is part of the supplied group
  cstate = (Object *)child_list->lh_Head;
  while((curchild = NextObject(&cstate)))
  {
    if(curchild == child)
    {
      return TRUE;
    }
  }

  return FALSE;
}

///

/*** GFX related ***/
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
/// MyBltMaskBitMap()
static void MyBltMaskBitMap(const struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct BitMap *destBitMap, LONG xDest, LONG yDest, LONG xSize, LONG ySize, struct BitMap *maskBitMap)
{
  BltBitMap(srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);
  BltBitMap(maskBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0xe2,~0,NULL);
  BltBitMap(srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);
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
/// MyBltMaskBitMapRastPort()
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
  {
    BltMaskBitMapRastPort(srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm, bltMask);
  }

  LEAVE();
}

///

/*** Miscellaneous stuff ***/
/// PGPGetPassPhrase
//  Asks user for the PGP passphrase
void PGPGetPassPhrase(void)
{
   if (!G->PGPPassPhrase[0])
   {
      G->PGPPassVolatile = FALSE;
      if(GetVar("PGPPASS", G->PGPPassPhrase, SIZE_DEFAULT, 0) < 0)
      {
         char pgppass[SIZE_DEFAULT];
         G->PGPPassVolatile = TRUE; *pgppass = 0;
         if (StringRequest(pgppass, SIZE_DEFAULT, "PGP", GetStr(MSG_UT_PGPPassReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->MA->GUI.WI))
            strlcpy(G->PGPPassPhrase, pgppass, sizeof(G->PGPPassPhrase));
      }
      else return;
   }
   SetVar("PGPPASS", G->PGPPassPhrase, -1, GVF_GLOBAL_ONLY);
}
///
/// PGPClearPassPhrase
//  Clears the ENV variable containing the PGP passphrase
void PGPClearPassPhrase(BOOL force)
{
   if (G->PGPPassVolatile) DeleteVar("PGPPASS", 0);
   if (force) G->PGPPassPhrase[0] = 0;
}
///
/// PGPCommand
//  Launches a PGP command
int PGPCommand(char *progname, char *options, int flags)
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
      BusyText(GetStr(MSG_BusyPGPrunning), "");
      strmfp(command, C->PGPCmdPath, progname);
      strlcat(command, " >" PGPLOGFILE " ", sizeof(command));
      strlcat(command, options, sizeof(command));

      // use SystemTags() for executing PGP
      error = SystemTags(command, SYS_Input,    fhi,
                                  SYS_Output,   fho,
                                  #if defined(__amigaos4__)
                                  SYS_Error,    NULL,
                                  #endif
                                  NP_StackSize, C->StackSize,
                                  NP_WindowPtr, NULL,
                                  TAG_DONE);

      BusyEnd();

      Close(fho);
    }

    Close(fhi);
  }

  if(error > 0 && !hasNoErrorsFlag(flags))
    ER_NewError(GetStr(MSG_ER_PGPreturnsError), command, PGPLOGFILE);

  if(error < 0)
    ER_NewError(GetStr(MSG_ER_PGPnotfound), C->PGPCmdPath);

  if(!error && !hasKeepLogFlag(flags))
    DeleteFile(PGPLOGFILE);

  RETURN(error);
  return error;
}
///
/// AppendToLogfile
//  Appends a line to the logfile
static void AppendToLogfile(int id, char *text, void *a1, void *a2, void *a3, void *a4)
{
   FILE *fh;
   char logfile[SIZE_PATHFILE], filename[SIZE_FILE];
   if (!C->LogAllEvents && (id < 30 || id > 49)) return;
   if (C->SplitLogfile)
   {
      struct ClockData cd;
      Amiga2Date(GetDateStamp(), &cd);
      snprintf(filename, sizeof(filename), "YAM-%s%d.log", months[cd.month-1], cd.year);
   }
   else
     strlcpy(filename, "YAM.log", sizeof(filename));

   strmfp(logfile, *C->LogfilePath ? C->LogfilePath : G->ProgDir, filename);

   if ((fh = fopen(logfile, "a")))
   {
      char datstr[64];
      DateStamp2String(datstr, sizeof(datstr), NULL, DSS_DATETIME, TZC_NONE);
      fprintf(fh, "%s [%02d] ", datstr, id);
      fprintf(fh, text, a1, a2, a3, a4);
      fprintf(fh, "\n");
      fclose(fh);
   }
}
///
/// AppendLog
//  Appends a line to the logfile, depending on log mode
void AppendLog(int id, char *text, void *a1, void *a2, void *a3, void *a4)
{
   if (C->LogfileMode != 0) AppendToLogfile(id, text, a1, a2, a3, a4);
}
void AppendLogNormal(int id, char *text, void *a1, void *a2, void *a3, void *a4)
{
   if (C->LogfileMode == 1) AppendToLogfile(id, text, a1, a2, a3, a4);
}
void AppendLogVerbose(int id, char *text, void *a1, void *a2, void *a3, void *a4)
{
   if (C->LogfileMode == 2) AppendToLogfile(id, text, a1, a2, a3, a4);
}
///
/// Busy
//  Displays busy message
void Busy(char *text, char *parameter, int cur, int max)
{
   // we can have different busy levels (defined BUSYLEVEL)
   static char infotext[BUSYLEVEL][SIZE_DEFAULT];
   static struct TimeVal last_move;

   if(text)
   {
      if(*text)
      {
        snprintf(infotext[BusyLevel], SIZE_DEFAULT, text, parameter);

        if(max > 0)
        {
          // initialize the InfoBar gauge
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

            GetSysTime(TIMEVAL(&last_move));
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
        {
          E(DBF_UTIL, "Error: reached highest BusyLevel!!!");
        }
      }
      else
      {
         if(BusyLevel)
           BusyLevel--;

         if(G->MA)
         {
            if(BusyLevel <= 0)
            {
              DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_HideBars);
            }
            else
            {
              DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowInfoText, infotext[BusyLevel-1]);
            }
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
          DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowGauge, NULL, cur, max);

        if(G->InStartupPhase)
        {
          struct TimeVal now;

          // then we update the gauge, but we take also care of not refreshing
          // it too often or otherwise it slows down the whole search process.
          GetSysTime(TIMEVAL(&now));
          if(-CmpTime(TIMEVAL(&now), TIMEVAL(&last_move)) > 0)
          {
            struct TimeVal delta;

            // how much time has passed exactly?
            memcpy(&delta, &now, sizeof(struct TimeVal));
            SubTime(TIMEVAL(&delta), TIMEVAL(&last_move));

            // update the display at least twice a second
            if(delta.Seconds > 0 || delta.Microseconds > 250000)
            {
              DoMethod(G->SplashWinObject, MUIM_Splashwindow_ProgressChange, NULL, cur, -1);
              memcpy(&last_move, &now, sizeof(struct TimeVal));
            }
          }
        }
      }
   }
}

///
/// DisplayAppIconStatistics
//  Calculates AppIconStatistic and update the AppIcon
void DisplayAppIconStatistics(void)
{
  static char apptit[SIZE_DEFAULT/2];
  struct Folder *fo;
  struct Folder **flist;
  char *src, dst[10];
  int mode;
  int new_msg = 0;
  int unr_msg = 0;
  int tot_msg = 0;
  int snt_msg = 0;
  int del_msg = 0;

  // if the user wants to show an AppIcon on the workbench,
  // we go and calculate the mail stats for all folders out there.
  if((flist = FO_CreateList()))
  {
    int i;

    for(i = 1; i <= (int)*flist; i++)
    {
      fo = flist[i];
      if(!fo)
        break;

      if(fo->Stats != 0)
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

  if(C->WBAppIcon)
  {
    // Lets create the label of the AppIcon now
    for(src = C->AppIconText; *src; src++)
    {
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
  if(G->TR && G->TR->Checking)
    mode = 3;
  else
    mode = tot_msg ? (new_msg ? 2 : 1) : 0;


  // We first have to remove the appicon before we can change it
  if(G->AppIcon)
  {
    RemoveAppIcon(G->AppIcon);
    G->AppIcon = NULL;
  }

  // Now we create the new AppIcon and display it
  if(G->DiskObj[mode])
  {
    struct DiskObject *dobj=G->DiskObj[mode];

    // NOTE:
    // 1.) Using the VARARGS version is better for GCC/68k and it doesn't
    //     hurt other compilers
    // 2.) Using "zero" as lock parameter avoids a header compatibility
    //     issue (old: "struct FileLock *"; new: "BPTR")
    if(C->WBAppIcon)
      G->AppIcon = AddAppIcon(0, 0, apptit, G->AppPort, 0, dobj, TAG_DONE);

    #if defined(__amigaos4__)
    // check if application.library is used and then
    // we also notify it about the AppIcon change
    if(G->applicationID)
    {
      static int lastIconID = -1;

      if(C->DockyIcon)
      {
        if(lastIconID != mode)
        {
          struct ApplicationIconInfo aii;

          aii.iconType = APPICONT_CustomIcon;
          aii.info.customIcon = dobj;

          SetApplicationAttrs(G->applicationID,
                              APPATTR_IconType, (uint32)&aii,
                              TAG_DONE);

          lastIconID = mode;
        }
      }
      else
        lastIconID = -1;
    }
    #endif
  }
}

///
/// DisplayStatistics
//  Calculates folder statistics and update mailbox status icon
void DisplayStatistics(struct Folder *fo, BOOL updateAppIcon)
{
   int pos;
   struct Mail *mail;
   struct MUI_NListtree_TreeNode *tn;
   struct Folder *actfo = FO_GetCurrentFolder();

   // If the parsed argument is NULL we want to show the statistics from the actual folder
   if (!fo) fo = actfo;
   else if (fo == (struct Folder *)-1)
   {
     fo = FO_GetFolderByType(FT_INCOMING, NULL);
   }

   // Get Position of Folder
   pos = FO_GetFolderPosition(fo, TRUE);
   if(pos < 0) return;

   // Now we recount the amount of Messages of this Folder
   for (mail = fo->Messages, fo->Unread = fo->New = fo->Total = fo->Sent = fo->Deleted = 0; mail; mail = mail->Next)
   {
      fo->Total++;

      if(hasStatusNew(mail))
        fo->New++;

      if(!hasStatusRead(mail))
        fo->Unread++;

      if(hasStatusSent(mail))
        fo->Sent++;

      if(hasStatusDeleted(mail))
        fo->Deleted++;
   }

   // if this folder hasn`t got any own folder image in the folder
   // directory and it is one of our standard folders we have to check which image we put in front of it
   if(fo->imageObject == NULL)
   {
      if(fo->Type == FT_INCOMING)      fo->ImageIndex = (fo->New+fo->Unread) ? 3 : 2;
      else if(fo->Type == FT_OUTGOING) fo->ImageIndex = (fo->Total > 0) ? 5 : 4;
      else if(fo->Type == FT_DELETED)  fo->ImageIndex = (fo->Total > 0) ? 7 : 6;
      else if(fo->Type == FT_SENT)     fo->ImageIndex = 8;
      else fo->ImageIndex = -1;
   }

   if (fo == actfo)
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
         struct Folder *fo_parent = (struct Folder *)tn_parent->tn_User;
         if(fo_parent)
         {
            int i;

            // clear the parent mailvariables first
            fo_parent->Unread = fo_parent->New = fo_parent->Total = fo_parent->Sent = fo_parent->Deleted = 0;

            // Now we scan every child of the parent and count the mails
            for(i=0;;i++)
            {
               struct MUI_NListtree_TreeNode *tn_child;
               struct Folder *fo_child;

               tn_child = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn_parent, i, MUIV_NListtree_GetEntry_Flag_SameLevel);
               if(!tn_child) break;

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
         else break;
      }
   }

   if (!G->AppIconQuiet && updateAppIcon) DisplayAppIconStatistics();
}

///
/// CheckPrinter
//  Checks if printer is ready
BOOL CheckPrinter(void)
{
   struct MsgPort *PrintPort;
   struct IOStdReq *PrintIO;
   char *error = NULL;

   if ((PrintPort = CreateMsgPort()))
   {
      //PrintPort->mp_Node.ln_Name = "YAM PrintPort";
      if ((PrintIO = (struct IOStdReq *)CreateIORequest(PrintPort, sizeof(struct IOStdReq))))
      {
         if (!(OpenDevice("printer.device", 0, (struct IORequest *)PrintIO, 0)))
         {
            UWORD Result = 0;
            PrintIO->io_Message.mn_ReplyPort = PrintPort;
            PrintIO->io_Command = PRD_QUERY;
            PrintIO->io_Data = &Result;
            DoIO((struct IORequest *)PrintIO);
            if(PrintIO->io_Actual == 1)      // parallel port printer?
            {
               if (((Result>>8) & 3) == 0) error = NULL;                   // no error
               else if ((Result>>8) & 01) error = GetStr(MSG_UT_NoPaper);  // /POUT asserted
               else error = GetStr(MSG_UT_NoPrinter);                      // /BUSY (hopefully no RingIndicator interference)
            } else error = NULL;               // can't determine status of serial printers
            CloseDevice((struct IORequest *)PrintIO);
         }
         DeleteIORequest((struct IORequest *)PrintIO);
      }
      DeleteMsgPort(PrintPort);
   }
   if (error && !MUI_Request(G->App, NULL, 0, GetStr(MSG_ErrorReq), GetStr(MSG_OkayCancelReq), error)) return FALSE;
   return TRUE;
}
///
/// PlaySound
//  Plays a sound file using datatypes
void PlaySound(char *filename)
{
  if(DataTypesBase)
  {
    // if we previously created a sound object
    // lets dispose it first.
    if(G->NewMailSound_Obj)
      DisposeDTObject(G->NewMailSound_Obj);

    // create the new datatype object
    G->NewMailSound_Obj = NewDTObject(filename, DTA_GroupID, GID_SOUND, TAG_DONE);
    if(G->NewMailSound_Obj)
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
}
///
/// MatchExtension
//  Matches a file extension against a list of extension
static BOOL MatchExtension(char *fileext, char *extlist)
{
  BOOL result = FALSE;

  ENTER();

  if(extlist)
  {
    char *s = extlist;

    // now we search for our delimiters step by step
    while(*s)
    {
      char *e;

      if((e = strpbrk(s, " |;,")) == NULL)
        e = s+strlen(s);

      // now check if the extension matches
      if(strnicmp(s, fileext, e-s) == 0)
      {
        result = TRUE;
        break;
      }

      // set the next start to our last search
      s = ++e;
    }
  }

  RETURN(result);
  return result;
}

///
/// IdentifyFile
// Tries to identify a file and returns its content-type if applicable
// otherwise NULL
char *IdentifyFile(char *fname)
{
  char ext[SIZE_FILE];
  char *ctype = NULL;

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
           ctype = IntMimeTypeArray[(FileProtection(fname) & FIBF_SCRIPT) ? MT_AP_SCRIPT : MT_TX_PLAIN].ContentType;
        else
        {
          D(DBF_MIME, "identifying file through datatypes.library");

          // per default we end up with an "application/octet-stream" content-type
          ctype = IntMimeTypeArray[MT_AP_OCTET].ContentType;

          if(DataTypesBase)
          {
            BPTR lock;

            if((lock = Lock(fname, ACCESS_READ)))
            {
              struct DataType *dtn;

              if((dtn = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL)))
              {
                char *type = NULL;
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
  BPTR lock;
  BOOL success = FALSE;
  static char buf[SIZE_PATHFILE];

  // lets try to get a Lock on the supplied path
  if((lock = Lock(path, SHARED_LOCK)))
  {
    // so, if it seems to exists, we get the "real" name out of
    // the lock again.
    if(NameFromLock(lock, buf, SIZE_PATHFILE) != DOSFALSE)
    {
      success = TRUE;
    }

    // And then we unlock the file/dir immediatly again.
    UnLock(lock);
  }

  // only on success we return the realpath.
  return success ? buf : path;
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
  path = CloneSearchPath();

  if(SystemTags(cmd,
                SYS_Input,    in,
                SYS_Output,   out,
                #if defined(__amigaos4__)
                SYS_Error,    err,
                #endif
                NP_Path,      path,
                NP_StackSize, C->StackSize,
                NP_WindowPtr, NULL,
                SYS_Asynch,   asynch,
                TAG_DONE) == -1)
  {
    // something went wrong.
    FreeSearchPath(path);
    path = 0;

    result = FALSE;
  }

  if(asynch == FALSE && outdef != OUT_DOS)
  {
    if(path != 0)
      FreeSearchPath(path);

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
void GotoURL(char *url)
{
  ENTER();

  if(C->RX[MACRO_URL].Script[0])
  {
    char newurl[SIZE_LARGE];
    snprintf(newurl, sizeof(newurl), "%c%s%c", '"', url, '"');
    MA_StartMacro(MACRO_URL, newurl);
  }
  else if((OpenURLBase=OpenLibrary("openurl.library", 1)))
  {
    if(GETINTERFACE("main", IOpenURL, OpenURLBase))
    {
      static const struct TagItem tags[] = { { URL_NewWindow, TRUE },
                                             { TAG_DONE, 0         } };

      // open the URL in a new window per default
      URL_OpenA(url, (struct TagItem *)&tags[0]);

      DROPINTERFACE(IOpenURL);
    }
    CloseLibrary(OpenURLBase);
    OpenURLBase = NULL;
  }

  LEAVE();
}
///
/// SWSSearch()
// Smith&Waterman 1981 extended string similarity search algorithm
// X, Y are the two strings that will be compared for similarity
// It will return a pattern which will reflect the similarity of str1 and str2
// in a Amiga suitable format. This is case-insensitive !
char *SWSSearch(char *str1, char *str2)
{
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

  // by calling this function with (NULL, NULL) someone wants
  // to signal us to free the destination string
  if(str1 == NULL || str2 == NULL)
  {
    if(Z) free(Z);
    Z = NULL;
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
  for(i=0; i < lx; i++)
  {
    if(!(L[i]   = calloc(ly, sizeof(int)))) goto abort;
    if(!(Ind[i] = calloc(ly, sizeof(int)))) goto abort;
  }

  // and allocate the result string separately
  if(Z) free(Z);
  if(!(Z = calloc(lz, sizeof(char)))) goto abort;

  // we copy str1&str2 into X and Y but have to copy a placeholder in front of them
  memcpy(&X[1], str1, lx);
  memcpy(&Y[1], str2, ly);

  for(i=0; i < lx; i++)
  {
    Ind[i][0] = DELX;
  }

  for(j=0; j < ly; j++)
  {
    Ind[0][j] = DELY;
  }

  Ind[0][0] = DONE;

  // Now we calculate the L matrix
  // this is the first step of the SW algorithm
  for(i=1; i < lx; i++)
  {
    for(j=1; j < ly; j++)
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
        if(j>0)
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
        if(i>0)
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
  if(X)   free(X);
  if(Y)   free(Y);

  // lets free our help matrixes
  for(i=0; i < lx; i++)
  {
    if(L[i])    free(L[i]);
    if(Ind[i])  free(Ind[i]);
  }
  if(L)   free(L);
  if(Ind) free(Ind);

  return success ? &(Z[lz]) : NULL;
}

///
/// CRC32()
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

  // we calculate the crc32 now.
  while (count-- != 0)
  {
    ULONG temp1 = (crc >> 8) & 0x00FFFFFFL;
    ULONG temp2 = CRCTable[((int)crc ^ *p++) & 0xFF];
    crc = temp1 ^ temp2;
  }

  return crc;
}

///

/*** REXX interface support ***/
/// InsertAddresses
//  Appends an array of addresses to a string gadget
void InsertAddresses(APTR obj, char **addr, BOOL add)
{
   char *buf = (char *)xget(obj, MUIA_String_Contents);

   if (*buf && add) DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
   else setstring(obj, "");

   DoMethod(obj, MUIM_BetterString_Insert, *addr, MUIV_BetterString_Insert_EndOfString);

   while (*++addr)
   {
      DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);
      DoMethod(obj, MUIM_BetterString_Insert, *addr, MUIV_BetterString_Insert_EndOfString);
   }
}
///
/// AllocReqText
//  Prepare multi-line text for requesters, converts \n to line breaks
char *AllocReqText(char *s)
{
   char *d, *reqtext;
   d = reqtext = calloc(strlen(s)+1, 1);
   while (*s)
      if (*s == '\\' && s[1] == 'n') { *d++ = '\n'; s++; s++; }
      else *d++ = *s++;
   return reqtext;
}
///

/// putCharFunc
//  Hook used by FormatString()
HOOKPROTONO(putCharFunc, void, int c)
{
  char **tmp;

  ((char *)hook->h_Data)[0] = c;
  tmp = (char **)(&hook->h_Data);
  (*tmp)++;
}
MakeStaticHook(putCharHook, putCharFunc);
///

/// SPrintF
//  sprintf() replacement with Locale support
void STDARGS VARARGS68K SPrintF(char *outstr, char *fmtstr, ...)
{
  struct Hook hook;
  VA_LIST args;

  // initialize the hook
  InitHook(&hook, putCharHook, outstr);

  VA_START(args, fmtstr);
  FormatString(G->Locale, fmtstr, VA_ARG(args, void *), &hook);
  VA_END(args);
}
///

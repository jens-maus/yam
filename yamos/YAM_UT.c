/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/exec.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/keymap.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>
#include <workbench/startup.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_debug.h"
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

#define CRYPTBYTE       164
#define MUIA_Dtpic_Name 0x80423d72

struct UniversalClassData
{
   struct UniversalGUIData { APTR WI; } GUI;
};

int BusyLevel = 0;

long PNum = 0;
unsigned char *PPtr[16];

/***************************************************************************
 Utilities
***************************************************************************/

/* local protos */
static int GetWord(char **rptr, char *wbuf, int max);
static char *ReflowParagraph(char *start, char *end, int lmax, char *dest);
static void RemoveQuoteString(char *start, char *end, char *quot, char *dest);
static char *InsertQuoteString(char *start, char *quote, FILE *out);
static void SaveParagraph(char *start, char *end, char *prefix, FILE *out);
static char *FileToBuffer(char *file);
static BOOL GetPackMethod(int xpktype, char **method, int *eff);
static BOOL CompressMailFile(char *src, char *dst, char *passwd, char *method, int eff);
static BOOL UncompressMailFile(char *src, char *dst, char *passwd);
static void AppendToLogfile(int id, char *text, void *a1, void *a2, void *a3, void *a4);
static char *IdentifyFileDT(char *fname);
static void TimeValTZConvert(struct timeval *tv, enum TZConvert tzc);
static void DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc);

struct PathNode
{
   BPTR next;
   BPTR dir;
};

/// CloneWorkbenchPath
static BPTR CloneWorkbenchPath(struct WBStartup *wbmsg)
{
   BPTR path = 0;

   Forbid();
   if (wbmsg->sm_Message.mn_ReplyPort)
   {
      if ((wbmsg->sm_Message.mn_ReplyPort->mp_Flags & PF_ACTION) == PA_SIGNAL)
      {
         struct Process *wbproc = wbmsg->sm_Message.mn_ReplyPort->mp_SigTask;

         if (wbproc->pr_Task.tc_Node.ln_Type == NT_PROCESS)
         {
            struct CommandLineInterface *cli = BADDR(wbproc->pr_CLI);

            if (cli)
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
                  if (!dir2) break;
                  node = AllocVec(sizeof(struct PathNode), MEMF_PUBLIC);
                  if (!node)
                  {
                     UnLock(dir2);
                     break;
                  }
                  node->next = 0;
                  node->dir = dir2;
                  *p = MKBADDR(node);
                  p = &node->next;
               }
            }
         }
      }
   }
   Permit();

   return path;
}

///
/// FreeWorkbenchPath
static void FreeWorkbenchPath(BPTR path)
{
   while (path)
   {
      struct PathNode *node = BADDR(path);
      path = node->next;
      UnLock(node->dir);
      FreeVec(node);
   }
}
///

/*** Requesters ***/
/// YAMMUIRequest
// Own -secure- implementation of MUI_Request with collecting and reissueing ReturnIDs
// We also have a wrapper #define MUI_Request for calling that function instead.
LONG STDARGS YAMMUIRequest(APTR app, APTR win, LONG flags, char *title, char *gadgets, char *format, ...)
{
  LONG result = -1;
  char reqtxt[SIZE_LINE];
  Object *WI_YAMREQ;
  Object *BT_GROUP;
  va_list args;

  // lets create the requester text
  va_start(args, format);
  vsprintf(reqtxt, format, args);
  va_end(args);

  // if the applicationpointer is NULL we fall back to a standard requester
  if(app == NULL)
  {
    if(IntuitionBase)
    {
      struct EasyStruct ErrReq = { sizeof (struct EasyStruct), 0, NULL, NULL, NULL };

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

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(!SafeOpenWindow(WI_YAMREQ)) result = 0;
    else do
    {
      ULONG signals;
      LONG ret = DoMethod(app, MUIM_Application_NewInput, &signals);

      // bail out if a button was hit
      if(ret > 0 && ret < num_gads) { result = ret; break; }
			if(ret == num_gads)           { result = 0;   break; }

      if(signals) Wait(signals);
    }
    while(1);

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    DoMethod(app, OM_REMMEMBER, WI_YAMREQ);
    set(app, MUIA_Application_Sleep, FALSE);
  }

  return result;
}
///
/// StringRequest
//  Puts up a string requester
int StringRequest(char *string, int size, char *title, char *body, char *yestext, char *alttext, char *notext, BOOL secret, APTR parent)
{
   APTR bt_okay, bt_middle, bt_cancel, wi_sr, st_in;
   int ret_code = -1;

   wi_sr = WindowObject,
      MUIA_Window_Title, title ? title : "YAM",
      MUIA_Window_RefWindow, parent,
      MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
      MUIA_Window_ID, MAKE_ID('S','R','E','Q'),
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
   setstring(st_in, string);
   set(wi_sr, MUIA_Window_ActiveObject, st_in);
   set(G->App, MUIA_Application_Sleep, TRUE);
   DoMethod(G->App, OM_ADDMEMBER, wi_sr);
   DoMethod(bt_okay  , MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
   DoMethod(bt_middle, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 2);
   DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
   DoMethod(st_in, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
   DoMethod(wi_sr, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

   // lets collect the waiting returnIDs now
   COLLECT_RETURNIDS;

   if (!SafeOpenWindow(wi_sr)) ret_code = 0;
   else while (ret_code == -1)
   {
      ULONG signals;
      switch (DoMethod(G->App, MUIM_Application_Input, &signals))
      {
         case 1: ret_code = 1; break;
         case 2: ret_code = 2; break;
         case 3: ret_code = 0; break;
      }
      if (ret_code == -1 && signals) Wait(signals);
   }

   // now lets reissue the collected returnIDs again
   REISSUE_RETURNIDS;

   if (ret_code > 0) GetMUIString(string, st_in);
   DoMethod(G->App, OM_REMMEMBER, wi_sr);
   set(G->App, MUIA_Application_Sleep, FALSE);
   return ret_code;
}
///
/// FolderRequest
//  Allows user to choose a folder from a list
struct Folder *FolderRequest(char *title, char *body, char *yestext, char *notext, struct Folder *exclude, APTR parent)
{
   static int lastactive;
   struct Folder **flist, *folder = (struct Folder *)-1;
   int act, i;
   char *fname;
   APTR bt_okay, bt_cancel, wi_fr, lv_folder;

   wi_fr = WindowObject,
      MUIA_Window_Title, title ? title : "YAM",
      MUIA_Window_RefWindow, parent,
      MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
      MUIA_Window_ID, MAKE_ID('F','R','E','Q'),
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

      if (!SafeOpenWindow(wi_fr)) folder = NULL;
      else while (folder == (struct Folder *)-1)
      {
         ULONG signals, oo = DoMethod(G->App, MUIM_Application_Input, &signals);
         switch (oo)
         {
            case 1:
               get(lv_folder, MUIA_List_Active, &act);
               DoMethod(lv_folder, MUIM_List_GetEntry, act, &fname);
               if ((folder = FO_GetFolderByName(fname, NULL))) lastactive = act;
               break;
            case 3: folder = NULL; break;
         }
         if (folder == (struct Folder *)-1 && signals) Wait(signals);
      }

      // now lets reissue the collected returnIDs again
      REISSUE_RETURNIDS;

      DoMethod(G->App, OM_REMMEMBER, wi_fr);
      set(G->App, MUIA_Application_Sleep, FALSE);
   }
   return folder;
}
///
/// AttachRequest
//  Allows user to select a message part (attachment) from a list
struct Part *AttachRequest(char *title, char *body, char *yestext, char *notext, int winnum, int mode, APTR parent)
{
  struct Part *retpart = (struct Part *)-1, *part;
  APTR bt_okay, bt_cancel, wi_ar, lv_attach;

  // lets create the AttachSelection window now
  wi_ar = WindowObject,
    MUIA_Window_Title,      title ? title : "YAM",
    MUIA_Window_RefWindow,  parent,
    MUIA_Window_LeftEdge,   MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,    MUIV_Window_TopEdge_Centered,
//    MUIA_Window_ID,         MAKE_ID('A','R','E','Q'), // we don`t supply a windowID or otherwise the upper three attributes don`t work.
    WindowContents, VGroup,
      Child, LLabel(body),
        Child, lv_attach = NListviewObject,
          MUIA_CycleChain,      1,
          MUIA_NListview_NList, NListObject,
            InputListFrame,
            MUIA_NList_Title,       TRUE,
            MUIA_NList_DoubleClick, TRUE,
            MUIA_NList_MultiSelect, isMultiReq(mode) ? MUIV_NList_MultiSelect_Default : MUIV_NList_MultiSelect_None,
            MUIA_NList_DisplayHook, &RE_LV_AttachDspFuncHook,
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
    // lets create the static parts of the Attachrequest entries in the NList
    static struct Part spart[2];
    spart[0].Nr = PART_ORIGINAL;
    strcpy(spart[0].Name, GetStr(MSG_RE_Original));
    spart[0].Size = G->RE[winnum]->Mail.Size;
    spart[0].Decoded = TRUE;

    DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[0], MUIV_NList_Insert_Bottom);

    // if this AttachRequest isn`t a DISPLAY request we show all the option to select the text we actually see
    if(!isDisplayReq(mode))
    {
      spart[1].Nr = PART_ALLTEXT;
      strcpy(spart[1].Name, GetStr(MSG_RE_AllTexts));
      spart[1].Size = 0;

      DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[1], MUIV_NList_Insert_Bottom);
    }

    // now we process the mail and pick every part out to the NListview
    for(part = G->RE[winnum]->FirstPart->Next; part; part = part->Next)
    {
      if(!isPrintReq(mode) || part->Printable)
      {
        DoMethod(lv_attach, MUIM_NList_InsertSingle, part, MUIV_NList_Insert_Bottom);
      }
    }

    // now lets create all other window dependencies (this have to be multithreaded later)
    set(wi_ar, MUIA_Window_ActiveObject, lv_attach);
    set(G->App, MUIA_Application_Sleep, TRUE);
    DoMethod(G->App, OM_ADDMEMBER, wi_ar);
    DoMethod(bt_okay  , MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(lv_attach, MUIM_Notify, MUIA_NList_DoubleClick, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_ar, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    // we open the window now and listen for some events.
    if (!SafeOpenWindow(wi_ar)) retpart = NULL;
    else while (retpart == (struct Part *)-1)
    {
      ULONG signals;
      switch(DoMethod(G->App, MUIM_Application_Input, &signals))
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

            if(retpart == (struct Part *)-1) retpart = part;
            else prevpart->NextSelected = part;
          }
        }
        break;

        case 3:
        {
          retpart = NULL;
        }
        break;
      }
      if (retpart == (struct Part *)-1 && signals) Wait(signals);
    }

    // now lets reissue the collected returnIDs again
    REISSUE_RETURNIDS;

    DoMethod(G->App, OM_REMMEMBER, wi_ar);
    set(G->App, MUIA_Application_Sleep, FALSE);
  }
  return retpart;
}
///
/// InfoWindow
//  Displays a text in an own window
void InfoWindow(char *title, char *body, char *oktext, APTR parent)
{
   APTR bt_okay, wi_iw;

   if ((wi_iw = WindowObject,
         MUIA_Window_Title, title,
         MUIA_Window_RefWindow, parent,
         MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
         MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
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
      DoMethod(bt_okay, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_CloseWindow, wi_iw);
      DoMethod(wi_iw  , MUIM_Notify, MUIA_Window_CloseRequest, TRUE, MUIV_Notify_Application, 5, MUIM_Application_PushMethod, parent, 2, MUIM_MainWindow_CloseWindow, wi_iw);
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
   sprintf(str, "%d", val);
   return str;
}
///
/// MatchNoCase
//  Case insensitive pattern matching
BOOL MatchNoCase(char *string, char *match)
{
   BOOL result=FALSE;
   LONG patternlen = strlen(match)*2+2; // ParsePattern() needs at least 2*source+2 bytes buffer
   char *pattern = malloc((size_t)patternlen);

   if(pattern)
   {
     if(ParsePatternNoCase(match, pattern, patternlen) != -1)
     {
        result = MatchPatternNoCase(pattern, string);
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
   while (*s && ISpace(*s)) ++s;
   return s;
}       
///
/// TrimEnd
//  Removes trailing spaces
char *TrimEnd(char *s)
{
   char *e = s+strlen(s)-1;
   while (e >= s && ISpace(*e)) *e-- = 0;
   return s;
}
///
/// Trim
//  Removes leading and trailing spaces
char *Trim(char *s)
{
   if(s)
   {
      char *e = s+strlen(s)-1;
      while (*s && ISpace(*s)) ++s;
      while (e >= s && ISpace(*e)) *e-- = 0;
   }
   return s;
}       

///
/// SParse
//  Translate string with parsers
void SParse(char *s)
{
   long ctr[16], n, Nmax, NGlob = 0, max, mid;
   unsigned char *tptr, *p = NULL, *tp, la, lb;
   BOOL gr = 0, lr = 0;
            
   if (!s || !PNum || !G->CO_AutoTranslateIn) return;

   while (*s)
   {
      for (n = 0; n != PNum; n++) ctr[n] = 0; 

      tp = s;

      do
      {
         Nmax = 0;
         mid = max = -466725766; 

         for (n = 0; n != PNum; n++)      
         { 
            tptr = PPtr[n]; 
            la = 0;
            p = tp;
 
            do
            {
               lb = (*p++) ^ 128;
               if ( !((la | lb) & 128) ) ctr[n] += (signed char)tptr[(la << 7) + lb];
               la = lb;
            }
            while ((*p) && (*p) != 0x0a); 

            if (max < ctr[n])
            {   
               mid = max;
               max = ctr[n];
               Nmax = n; 
            }

         }

         if (*p == 0x0a) p++;

         tp = p;        

         if ((max >= 500) && ((max-mid) >=1000)) 
         {
            lr = gr = 1;
            NGlob = Nmax;
         }

      }
      while ((*p) && (!gr));

      if (gr || ((!(*p)) && lr)) Nmax = NGlob; 

      tptr = PPtr[Nmax] + 16384;

      do
      {
         *s = tptr[(unsigned char)*s]; 
         s++;
      }
      while (s != (char*)p); 

      if (*p == 0x0a) gr = 0; 
   }
}

/// 
/// LoadParsers
//  Load a parser tables into memory
BOOL LoadParsers(void)
{
   char *temp;
   char dir[SIZE_PATH], file[SIZE_PATHFILE];
   struct FileInfoBlock *fib;
   FILE *fp;
   BPTR lock;
   BOOL result = TRUE;
   
   if(PNum) return TRUE;

   strmfp(dir, G->ProgDir, "parsers");

   if((lock = Lock((STRPTR)dir, ACCESS_READ)))
   {
      if((fib = AllocDosObject(DOS_FIB, NULL)))
      {
        if(Examine(lock, fib))
        {
          while ((PNum < ARRAY_SIZE(PPtr)) && ExNext(lock,fib) && (IoErr() != ERROR_NO_MORE_ENTRIES))
          {
            strmfp(file, dir, fib->fib_FileName);

            if ((fp = fopen(file, "rb")))
            {
              if ((PPtr[PNum] = calloc(1, 16640)))
              {
                fread(PPtr[PNum], 1, 16640, fp);

                if(isFlagSet(fib->fib_Protection, FIBF_PURE))
                {
                  temp = PPtr[PNum];
                  PPtr[PNum] = PPtr[0];
                  PPtr[0] = temp;
                }

                PNum++;

              }
              else result = FALSE;

              fclose(fp);
            }
            else result = FALSE;
          }
        }
        else result = FALSE;

        FreeDosObject(DOS_FIB, fib);
      }
      else result = FALSE;

      UnLock(lock);
   }
   else result = FALSE;

   return result;
}

///
/// stccat
//  Safe string concatenation
char *stccat(char *a, char *b, int n)
{
   int m = 1;
   char *p = a;
   while (*p) { p++; m++; }
   while (*b && m < n) { *p++ = *b++; m++; }
   *p = 0;
   return a;
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
char *StrBufCpy(char *strbuf, char *source)
{
   size_t oldlen, newlen, reqlen=strlen(source);
   char *newstrbuf;

   if (!strbuf)
     if (NULL == (strbuf = AllocStrBuf(reqlen+1))) return NULL;
   oldlen = ((size_t *)strbuf)[-1];
   newstrbuf = strbuf;
   for (newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT);
   if (newlen != oldlen)
   {
      FreeStrBuf(strbuf);
      newstrbuf = AllocStrBuf(newlen);
   }
   if(newstrbuf) strcpy(newstrbuf, source);
   return newstrbuf;
}
///
/// StrBufCat
//  String concatenation using a dynamic buffer
char *StrBufCat(char *strbuf, char *source)
{
   size_t oldlen, newlen, reqlen=strlen(source);
   char *newstrbuf;

   if (!strbuf)
     if (NULL == (strbuf = AllocStrBuf(reqlen+1))) return NULL;
   reqlen += strlen(strbuf);
   oldlen = ((size_t *)strbuf)[-1];
   newstrbuf = strbuf;
   for (newlen = oldlen; newlen <= reqlen; newlen += SIZE_DEFAULT);
   if (newlen != oldlen)
   {
      if(NULL == (newstrbuf = AllocStrBuf(newlen)))
      {
         FreeStrBuf(strbuf);
         return NULL;
      }
      strcpy(newstrbuf, strbuf);
      FreeStrBuf(strbuf);
   }
   strcat(newstrbuf, source);
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

/// FreeData2D
//  Frees dynamic two-dimensional array
void FreeData2D(struct Data2D *data)
{
   while(data->Used)
   {
      data->Used--;
      FreeStrBuf(data->Data[data->Used]);
   }

   if (data->Allocated) free(data->Data);
   data->Data = NULL; data->Allocated = 0;
}
///
/// AllocData2D
//  Allocates dynamic two-dimensional array
char *AllocData2D(struct Data2D *data, size_t initsize)
{
   if (data->Used >= data->Allocated)
   {
      data->Allocated += 10;
      if (data->Data) data->Data = realloc(data->Data, data->Allocated*sizeof(char *));
      else            data->Data = malloc(data->Allocated*sizeof(char *));
   }
   return data->Data[data->Used++] = AllocStrBuf(initsize);
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
      sprintf(&buffer[strlen(buffer)], "%03d ", c);
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
   memset(buffer, 0, bufsize);
   if (!fgets(buffer, bufsize, fh)) return NULL;
   if ((ptr = strpbrk(buffer, "\r\n"))) *ptr = 0;
   return buffer;
}       
///
/// FileInfo
//  Gets size, protection bits and type of a file/directory
BOOL FileInfo(char *filename, int *size, long *bits, long *type)
{
  BPTR lock;
  BOOL result = FALSE;

  if((lock = Lock((STRPTR)filename,ACCESS_READ)))
  {
    // only if the calling function needs more info
    // we go on so that we are faster :)
    if(size || bits || type)
    {
      struct FileInfoBlock *fib;

      if((fib = AllocDosObject(DOS_FIB, NULL)))
      {
        if(Examine(lock, fib))
        {
          if (size) *size = fib->fib_Size;
          if (bits) *bits = fib->fib_Protection;
          if (type) *type = fib->fib_DirEntryType;
          result = TRUE;
        }
        FreeDosObject(DOS_FIB, fib);
      }
    }
    else result = TRUE;

    UnLock(lock);
  }
  return result;
}
///
/// FileSize
//  Returns size of a file
int FileSize(char *filename)
{
   int size;
   if (FileInfo(filename, &size, NULL, NULL)) return size; else return -1;
}
///
/// FileProtection
//  Returns protection bits of a file
long FileProtection(char *filename)
{
   long bits;
   if (FileInfo(filename, NULL, &bits, NULL)) return bits; else return -1;
}
///
/// FileType
//  Returns file type (file/directory)
int FileType(char *filename)
{
   long type;
   if (FileInfo(filename, NULL, NULL, &type)) return (type < 0 ? 1 : 2); else return 0;
}
///
/// RenameFile
//  Renames a file and restores the protection bits
BOOL RenameFile(char *oldname, char *newname)
{
   struct FileInfoBlock *fib;
   BPTR lock;
   BOOL result = FALSE;

   if(!Rename(oldname, newname)) return FALSE;

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

   if (sour) sourfh = fopen(sour, "r");
   if (sourfh && dest) destfh = fopen(dest, "w");

   if (sourfh && destfh)
   {
      char buf[SIZE_LARGE];
      int len;

      while((len = fread(buf, 1, SIZE_LARGE, sourfh)))
      {
         if(fwrite(buf, 1, len, destfh) != len) break;
      }

      // if we arrived here because this was the eof of the sourcefile
      // and non of the two filehandles are in error state we can set
      // success to TRUE.
      if(feof(sourfh) && !ferror(sourfh) && !ferror(destfh)) success = TRUE;
   }

   if (dest && destfh) fclose(destfh);
   if (sour && sourfh) fclose(sourfh);

   return success;
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
/// GetWord
//  Word-wrapping algorithm: gets next word
static int GetWord(char **rptr, char *wbuf, int max)
{
   int c, i = 0;
   static int nonblanks = 0;

   if (!(c = *(*rptr)++)) { *wbuf = 0; return 0; }
   if (isSpace(c))
   {
      while (isSpace(c) && c != '\n')
      {
         if (i < max-1) wbuf[i++] = c;
         c = *(*rptr)++;
      }
      if (c == '\n' || !c) { i = 0; wbuf[i++] = '\n'; }
   }
   else
   {
      while (isGraph(c))
      {
         if (i < max-1) wbuf[i++] = c; else break;
         c = *(*rptr)++;
      }
      nonblanks += i;
      while (isSpace(c) && c != '\n') c = *(*rptr)++;
      if (c == '\n')
      {
         if (nonblanks <= 20) wbuf[i++] = '\n';
         nonblanks = 0;
      }
   }
   if (isGraph(c)) (*rptr)--;
   wbuf[i] = '\0'; return i;
}
///
/// ReflowParagraph
//  Word-wrapping algorithm: process a paragraph
static char *ReflowParagraph(char *start, char *end, int lmax, char *dest)
{
   int lword, lline = 0;
   char c, word[SIZE_LARGE], *p;
   BOOL dented = FALSE;

   while ((lword = GetWord(&start, word, SIZE_LARGE)))
   {
      if ((c = word[lword-1]) == '\n')  word[--lword] = '\0';
      if (!lword);
      else if (isSpace(*word))
      {
         if (lline) *dest++ = '\n';
         dented = TRUE; lline = lword;
         for (p = word; *p; p++) *dest++ = *p;
      }
      else
      {
         if (lline == 0 || dented) { for (p = word; *p; p++) *dest++ = *p; lline += lword; dented = FALSE; }
         else
         {
            if (lline+lword < lmax) { *dest++ = ' '; for (p = word; *p; p++) *dest++ = *p; lline += lword+1; }
            else { *dest++ = '\n'; for (p = word; *p; p++) *dest++ = *p; lline = lword; }
         }
      }
      if (c == '\n') { *dest++ = '\n'; lline = 0; }
      if (start > end) break;
   }
   if (lline) *dest++ = '\n';
   *dest-- = 0;
   return dest;
}
///
/// RemoveQuoteString
//  Removes reply prefix
static void RemoveQuoteString(char *start, char *end, char *quot, char *dest)
{
   int quotlen = strlen(quot);

   while (start <= end)
   {
      if (!strncmp(start, quot, quotlen)) start += quotlen;
      while (*start && *start != '\n') *dest++ = *start++;
      if (*start) *dest++ = *start++;
   }
   *dest = '\0'; // null-terminate
}
///
/// InsertQuoteString
//  Inserts reply prefix
static char *InsertQuoteString(char *start, char *quote, FILE *out)
{
   if ((*start != '\n' || C->QuoteEmptyLines) && strncmp(start, "<sb>", 4) && strncmp(start, "<tsb>", 5))
   {
      int level, i;
      for (i = level = 0; start[i] && start[i] != '\n' && i < 16; i++)
      {
         if (isSpace(start[i]) && (!level || start[i+1] == '>')) continue;
         if (start[i] == '>') level++; else if (!isAlNum(start[i]) || level) break;
      }
      if (level) start = &start[i];
      if (level > 8) level = 8;
      fputs(quote, out); while (level--) fputc('>', out);
      if (*start != ' ') fputc(' ', out);
   }
   return start;
}
///
/// SaveParagraph
//  Writes a paragraph and inserts reply prefixes
static void SaveParagraph(char *start, char *end, char *prefix, FILE *out)
{
   while (start <= end)
   {
      if (*prefix) start = InsertQuoteString(start, prefix, out);
      while (*start && *start != '\n') fputc(*start++, out);
      if (*start) fputc(*start++, out);
   }
}
///
/// QuoteWordWrap
//  Reformats quoted messages to a new line length
void QuoteWordWrap(char *rptr, int lmax, char *prefix, char *firstprefix, FILE *out)
{
   if (!prefix) prefix = firstprefix;
   while (*rptr)
   {
      char *p, *ps = rptr, *pe, quot[17], c;
      int lsm = 0;
      *quot = 0;
      while (TRUE)
      {
         int ls = 0;
         char *p = rptr;
         while (*rptr && *rptr != '\n') { rptr++; ls++; }
         if (ls && *p == '>' && !*quot)
         {
            int i = 0;
            while ((*p == ' ' || *p == '>') && i < 16) quot[i++] = *p++;
            quot[i] = 0;
            while (i > 1) if (quot[i-2] == ' ' && quot[i-1] == ' ') quot[--i] = 0; else break;
         }
         if (ls > lsm) lsm = ls;
         if (!*rptr) break;
         c = rptr[1];
         if (isSpace(c) || !c || c == '\n') break;
         if (!*quot && c == '>') break;
         if (*quot) if (strncmp(&rptr[1], quot, strlen(quot))) break;
         rptr++;
      }
      pe = rptr;
      if (lsm > lmax)
      {
         char *buf = calloc((size_t)(2*(pe-ps+2)), sizeof(char));
         if (buf)
         {
           if (*quot)
           {
              char newprefix[SIZE_DEFAULT];
              RemoveQuoteString(ps, pe, quot, buf);
              strcpy(newprefix, firstprefix); strcat(newprefix, TrimEnd(quot));
              QuoteWordWrap(buf, lmax-strlen(quot), newprefix, firstprefix, out);
           }
           else
           {
              p = ReflowParagraph(ps, pe, lmax, buf);
              SaveParagraph(buf, p, prefix, out);
           }

           free(buf);
         }
      }
      else SaveParagraph(ps, pe, prefix, out);
      rptr = pe+1;
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
         if (ISpace(ch)) lsp = p;
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
     "#?.addressbook#?", "#?.config#?", NULL, "~(#?.info)", "#?.(yam|rexx)", "#?.(gif|jpg|jpeg|png|iff|ilbm)", NULL, NULL
   };

   static BOOL init[MAXASL] =
   {
     FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE
   };
   char *postext = hasSaveModeFlag(mode) ? GetStr(MSG_UT_Save) : GetStr(MSG_UT_Load);
   int skip = *file ? 1 : 2;
   struct Window *truewin;

   get(win, MUIA_Window_Window, &truewin);
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
      sprintf(buf, "YAMt%d%02d.tmp", G->RexxHost->portnumber, ++count);

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
      DB(kprintf("DeleteTempFile: %s\n", tf->Filename);)
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
   char *filename = FilePart(dir);
   int i;
   for (i = 0; i < 4; i++) if (!stricmp(filename, FolderNames[i])) return TRUE;
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
   return FileInfo(fname, NULL, NULL, NULL);
}
///
/// DeleteMailDir
//  Recursively deletes a mail directory
void DeleteMailDir(char *dir, BOOL isroot)
{
   char fname[SIZE_PATHFILE], *filename, dirname[SIZE_PATHFILE];
   struct FileInfoBlock *fib;
   BOOL cont, isdir;
   BPTR lock;

   if((fib = AllocDosObject(DOS_FIB,NULL)))
   {
      if((lock = Lock(dir, ACCESS_READ)))
      {
        strcpy(dirname, dir);
        if(Examine(lock, fib))
        {
          cont = (ExNext(lock,fib) && IoErr() != ERROR_NO_MORE_ENTRIES);
          while (cont)
          {
            strmfp(fname, dir, fib->fib_FileName);
            filename = FilePart(fname);
            isdir = isDrawer(fib);
            cont = (ExNext(lock,fib) && IoErr() != ERROR_NO_MORE_ENTRIES);
            if (isroot)
            {
              if (isdir)
              {
                if (IsFolderDir(fname)) DeleteMailDir(fname, FALSE);
              }
              else
              {
                if (!stricmp(filename, ".config") || !stricmp(filename, ".glossary") || !stricmp(filename, ".addressbook")) DeleteFile(fname);
              }
            }
            else if (!isdir) if (!stricmp(filename, ".fconfig") || !stricmp(filename, ".index") || IsValidMailFile(filename)) DeleteFile(fname);
          }
        }
        UnLock(lock);
        DeleteFile(dirname);
      }
      FreeDosObject(DOS_FIB,fib);
   }
}
///
/// FileToBuffer
//  Reads a complete file into memory
static char *FileToBuffer(char *file)
{
   char *text;
   int size = FileSize(file);
   FILE *fh;

   if (size >= 0) if ((text = calloc(size+1,1)))
   {
      if ((fh = fopen(file, "r")))
      {
         fread(text, 1, size, fh);
         fclose(fh);
         return text;
      }
      free(text);
   }
   return NULL;
}
///

/*** Mail related ***/
/// MyAddTail
//  Adds a message to a message list
void MyAddTail(struct Mail **list, struct Mail *new)
{
   struct Mail *mail;
   new->Next = NULL;
   if (!*list) { *list = new; return; }
   for (mail = *list; mail->Next; mail = mail->Next);
   mail->Next = new;
}
///
/// MyRemove
//  Removes a message from a message list
void MyRemove(struct Mail **list, struct Mail *rem)
{
   struct Mail *mail;
   if (*list == rem) { *list = rem->Next; return; }
   for (mail = *list; mail->Next; mail = mail->Next)
      if (mail->Next == rem) { mail->Next = rem->Next; return; }
}
///
/// WhichLV
//  Returns pointer to message listview if folder is active
APTR WhichLV(struct Folder *folder)
{
   if (folder == FO_GetCurrentFolder()) return G->MA->GUI.NL_MAILS; else return NULL;
}
///
/// CreateFilename
//  Prepends mail directory to a file name
char *CreateFilename(char *file)
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
   if (G->MA) ER_NewError(GetStr(MSG_ER_CantCreateDir), dir, NULL);
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
   if (!folder && mail) folder = mail->Folder;
   if (!string) string = buffer;
   strmfp(string, (folder == NULL || folder == (struct Folder *)-1) ? C->TempDir : GetFolderDir(folder), mail->MailFile);
   return string;
}
///
/// GetMailInfo
//  Returns location of a message
struct MailInfo *GetMailInfo(struct Mail *smail)
{
   static struct MailInfo mi;
   int i;
   
   mi.Display = (smail->Folder == FO_GetCurrentFolder());
   mi.Pos = -1;
   mi.FName = GetMailFile(NULL, smail->Folder, smail);

   if (mi.Display)
   {
      for(i=0; mi.Pos == -1; i++)
      {
        struct Mail *mail;
        DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, i, &mail);
        if (!mail) break;
        if (mail == smail) mi.Pos = i;
      }
   }
   return &mi;
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

   if (*name)
   {
      if (strpbrk(name, ",.()")) delim = "\""; else delim = "";
      sprintf(buffer, "%s%s%s <%s>", delim, name, delim, address);
   }
   else sprintf(buffer, "%s", address);
   return buffer;
}
///
/// ExtractAddress
//  Extracts e-mail address and real name
void ExtractAddress(char *line, struct Person *pe)
{
   char *p = line, *ra[4], *save = malloc(strlen(line)+1);
   BOOL found = FALSE;

   ra[2] = ra[3] = NULL;
   strcpy(save, line);
   pe->Address[0] = pe->RealName[0] = 0;
   while (ISpace(*p)) p++;
   if ((ra[0] = MyStrChr(p,'<'))) if ((ra[1] = MyStrChr(ra[0],'>')))
   {
      *ra[0]++ = 0; *ra[1] = 0;
      for (ra[2] = p, ra[3] = ra[0]-2; ISpace(*ra[3]) && ra[3] >= ra[2]; ra[3]--) *ra[3] = 0;
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
   MyStrCpy(pe->Address ,Trim(ra[0]));
   MyStrCpy(pe->RealName, Trim(ra[2]));
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
/// ExpandText
//  Replaces variables with values
char *ExpandText(char *src, struct ExpandTextData *etd)
{
   char buf[SIZE_ADDRESS], *p, *p2, *dst = AllocStrBuf(SIZE_DEFAULT);
   struct DateStamp adate;
  
   for (; *src; src++)
      if (*src == '\\')
      {
         src++;
         switch (*src)
         {
            case '\\':  dst = StrBufCat(dst, "\\"); break;
            case 'n':  dst = StrBufCat(dst, "\n"); break;
         }
      }
      else if (*src == '%' && etd)
      {
         if (!etd->OM_Date) etd->OM_Date = DateStamp(&adate);
         src++;
         switch (*src)
         {
            case 'n': dst = StrBufCat(dst, etd->OS_Name); break;
            case 'f': MyStrCpy(buf, etd->OS_Name);
                      if ((p = strchr(buf, ','))) p = Trim(++p);
                      else { for (p = buf; *p && *p != ' '; p++); *p = 0; p = buf; }
                      dst = StrBufCat(dst, p); break;
            case 's': dst = StrBufCat(dst, etd->OM_Subject); break;
            case 'e': dst = StrBufCat(dst, etd->OS_Address); break;
            case 'd': dst = StrBufCat(dst, DateStamp2String(etd->OM_Date, DSS_DATE, TZC_NONE)); break;
            case 't': dst = StrBufCat(dst, DateStamp2String(etd->OM_Date, DSS_TIME, TZC_NONE)); break;
            case 'w': dst = StrBufCat(dst, DateStamp2String(etd->OM_Date, DSS_WEEKDAY, TZC_NONE)); break;
            case 'm': dst = StrBufCat(dst, etd->OM_MessageID); break;
            case 'r': dst = StrBufCat(dst, etd->R_Name); break;
            case 'v': strcpy(buf, etd->R_Name);
                      if ((p = strchr(buf, ','))) p = Trim(++p);
                      else { for (p = buf; *p && *p != ' '; p++); *p = 0; p = buf; }
                      dst = StrBufCat(dst, p); break;
            case 'a': dst = StrBufCat(dst, etd->R_Address); break;
            case 'i': strcpy(buf, etd->OS_Name);
                      for (p = p2 = &buf[1]; *p; p++)
                         if (*p == ' ' && p[1] && p[1] != ' ') *p2++ = *++p;
                      *p2 = 0;
                      dst = StrBufCat(dst, buf); break;
            case 'j': strcpy(buf, etd->OS_Name);
                      for (p2 = &buf[1], p = &buf[strlen(buf)-1]; p > p2; p--)
                         if (p[-1] == ' ') { *p2++ = *p; break; }
                      *p2 = 0;
                      dst = StrBufCat(dst, buf); break;
            case 'h': if ((p = FileToBuffer(etd->HeaderFile)))
                      {
                         dst = StrBufCat(dst, p); free(p);
                      }
                      break;
        }
      }
      else
      {
         static char chr[2] = { 0,0 };
         chr[0] = *src;
         dst = StrBufCat(dst, chr);
      }
   return dst;
}              
///
/// DescribeCT
//  Returns description of a content type
char *DescribeCT(char *ct)
{
   int i;

   for(i = 0; ContType[i]; i++)
   {
      if(!stricmp(ct, ContType[i]))
      {
        ct = GetStr(ContTypeDesc[i]);
        break;
      }
   }

   return ct;
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
void GetSysTimeUTC(struct timeval *tv)
{
  GetSysTime(tv);
  TimeValTZConvert(tv, TZC_UTC);
}
///
/// TimeValTZConvert
//  converts a supplied timeval depending on the TZConvert flag to be converted
//  to/from UTC
static void TimeValTZConvert(struct timeval *tv, enum TZConvert tzc)
{
  if(tzc == TZC_LOCAL)    tv->tv_secs += (C->TimeZone+C->DaylightSaving*60)*60;
  else if(tzc == TZC_UTC) tv->tv_secs -= (C->TimeZone+C->DaylightSaving*60)*60;
}
///
/// DateStampTZConvert
//  converts a supplied DateStamp depending on the TZConvert flag to be converted
//  to/from UTC
static void DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc)
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
//  converts a struct timeval to a struct DateStamp
void TimeVal2DateStamp(const struct timeval *tv, struct DateStamp *ds, enum TZConvert tzc)
{
   LONG seconds = (tv->tv_secs+tv->tv_micro/1000000);

   ds->ds_Days   = seconds/86400;       // calculate the days since 1.1.1978
   ds->ds_Minute = (seconds%86400)/60;
   ds->ds_Tick   = (tv->tv_secs%60)*TICKS_PER_SECOND + (tv->tv_micro/20000);

   // if we want to convert from/to UTC we need to do this now
   if(tzc != TZC_NONE) DateStampTZConvert(ds, tzc);
}
///
/// DateStamp2TimeVal
//  converts a struct DateStamp to a struct timeval
void DateStamp2TimeVal(const struct DateStamp *ds, struct timeval *tv, enum TZConvert tzc)
{
   /* creates wrong timevals from DateStamps with year >= 2114 ... */

   tv->tv_secs = (ds->ds_Days*24*60 + ds->ds_Minute)*60 + ds->ds_Tick/TICKS_PER_SECOND;
   tv->tv_micro = (ds->ds_Tick % TICKS_PER_SECOND) * 1000000/TICKS_PER_SECOND;

   // if we want to convert from/to UTC we need to do this now
   if(tzc != TZC_NONE) TimeValTZConvert(tv, tzc);
}
///
/// TimeVal2String
//  Converts a timeval structure to a string with using DateStamp2String after a convert
char *TimeVal2String(const struct timeval *tv, enum DateStampType mode, enum TZConvert tzc)
{
   struct DateStamp ds;

   // convert the timeval into a datestamp
   TimeVal2DateStamp(tv, &ds, TZC_NONE);

   // then call the DateStamp2String() function to get the real string
   return DateStamp2String(&ds, mode, tzc);
}
///
/// DateStamp2String
//  Converts a datestamp to a string
char *DateStamp2String(struct DateStamp *date, enum DateStampType mode, enum TZConvert tzc)
{
   static char resstr[64];                    // allocate enough space as OS3.1 is buggy here.
   char datestr[32], timestr[32], daystr[32]; // we don`t use LEN_DATSTRING as OS3.1 anyway ignores it.
   struct DateTime dt;
   struct DateStamp dsnow;

   // if this argument is not set we get the actual time
   if(!date) date = DateStamp(&dsnow);

   // now we fill the DateTime structure with the data for our request.
   dt.dat_Stamp   = *date;
   dt.dat_Format  = (mode == DSS_USDATETIME || mode == DSS_UNIXDATE) ? FORMAT_USA : FORMAT_DEF;
   dt.dat_Flags   = 0; // perhaps later we can add Weekday substitution
   dt.dat_StrDate = datestr;
   dt.dat_StrTime = timestr;
   dt.dat_StrDay  = daystr;

   // now we check wheter we have to convert the datestamp to a specific TZ or not
   if(tzc != TZC_NONE) DateStampTZConvert(&dt.dat_Stamp, tzc);

   // lets terminate the strings as OS 3.1 is strange
   datestr[31] = '\0';
   timestr[31] = '\0';
   daystr[31]  = '\0';

   // lets convert the DateStamp now to a string
   if(DateToStr(&dt) == FALSE) return NULL;

   switch (mode)
   {
      case DSS_UNIXDATE:
      {
        int y = atoi(&datestr[6]);

        // this is a Y2K patch
        // if less then 8035 days has passed since 1.1.1978 then we are in the 20th century
        if (date->ds_Days < 8035) y += 1900;
        else y += 2000;

        sprintf(resstr, "%s %s %02d %s %d\n", wdays[dt.dat_Stamp.ds_Days%7], months[atoi(datestr)-1], atoi(&datestr[3]), timestr, y);
      }
      break;

      case DSS_DATETIME:
      case DSS_USDATETIME:
      {
        sprintf(resstr, "%s %s", datestr, timestr);
      }
      break;

      case DSS_WEEKDAY:
      {
        strcpy(resstr, daystr);
      }
      break;

      case DSS_DATE:
      {
        strcpy(resstr, datestr);
      }
      break;

      case DSS_TIME:
      {
        strcpy(resstr, timestr);
      }
      break;

      case DSS_BEAT:
      case DSS_DATEBEAT:
      {
        // calculate the beat time
        LONG beat = (((date->ds_Minute-C->TimeZone+(C->DaylightSaving?0:60)+1440)%1440)*1000)/1440;

        if(mode == DSS_DATEBEAT) sprintf(resstr, "%s @%03ld", datestr, beat);
        else                     sprintf(resstr, "@%03ld", beat);
      }
      break;
   }
   return resstr;
}
///
/// DateStamp2Long
// Converts a datestamp to a numeric value
long DateStamp2Long(struct DateStamp *date)
{
   char *s, datestr[LEN_DATSTRING];
   struct DateStamp dsnow;
   struct DateTime dt;
   int y;

   if (!date) date = DateStamp(&dsnow);
   memset(&dt, 0, sizeof(struct DateTime));
   dt.dat_Stamp   = *date;
   dt.dat_Format  = FORMAT_USA;
   dt.dat_StrDate = datestr;

   DateToStr(&dt);
   s = Trim(datestr);

   // get the year
   y = atoi(&s[6]);

   // this is a Y2K patch
   // if less then 8035 days has passed since 1.1.1978 then we are in the 20th century
   if (date->ds_Days < 8035) y += 1900;
   else y += 2000;

   return((100*atoi(&s[3])+atoi(s))*10000+y);
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
            DB(kprintf("TZtoMinutes: found abbreviation '%s' (%ld)\n", time_zone_table[i].TZname, tzcorr);)
            break;
          }
        }
        DB(if(tzcorr == -1) kprintf("TZtoMinutes: abbreviation '%s' NOT found!\n", tzone);)
      }
   }

   return tzcorr == -1 ? 0 : (tzcorr/100)*60 + (tzcorr%100);
}
///
/// FormatSize
//  Displays large numbers using group separators
void FormatSize(LONG size, char *buffer)
{
  char *dp = G->Locale ? (char *)G->Locale->loc_DecimalPoint : ".";
  char *p = &buffer[strlen(buffer)];
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
      if(size < KB)       sprintf(p, "%ld B", size);
      else if(size < MB)  sprintf(p, "%.1f KB", dsize/KB);
      else if(size < GB)  sprintf(p, "%.1f MB", dsize/MB);
      else                sprintf(p, "%.1f GB", dsize/GB);

      if((p = strchr(p, '.'))) *p = *dp;
    }
    break;

    /*
    ** TWO Precision mode
    ** This will result in the following output:
    ** 1.23 GB - 12.34 MB - 123.45 KB - 1234 B
    */
    case SF_2PREC:
    {
      if(size < KB)       sprintf(p, "%ld B", size);
      else if(size < MB)  sprintf(p, "%.2f KB", dsize/KB);
      else if(size < GB)  sprintf(p, "%.2f MB", dsize/MB);
      else                sprintf(p, "%.2f GB", dsize/GB);

      if((p = strchr(p, '.'))) *p = *dp;
    }
    break;

    /*
    ** THREE Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.345 MB - 123.456 KB - 1234 B
    */
    case SF_3PREC:
    {
      if(size < KB)       sprintf(p, "%ld B", size);
      else if(size < MB)  sprintf(p, "%.3f KB", dsize/KB);
      else if(size < GB)  sprintf(p, "%.3f MB", dsize/MB);
      else                sprintf(p, "%.3f GB", dsize/GB);

      if((p = strchr(p, '.'))) *p = *dp;
    }
    break;

    /*
    ** MIXED Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.34 MB - 123.4 KB - 1234 B
    */
    case SF_MIXED:
    {
      if(size < KB)       sprintf(p, "%ld B", size);
      else if(size < MB)  sprintf(p, "%.1f KB", dsize/KB);
      else if(size < GB)  sprintf(p, "%.2f MB", dsize/MB);
      else                sprintf(p, "%.3f GB", dsize/GB);

      if((p = strchr(p, '.'))) *p = *dp;
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

      if(size < KB)      sprintf(p, "%ld", size);
      else if(size < MB) sprintf(p, "%ld%s%03ld", size/KB, gs, size%KB);
      else if(size < GB) sprintf(p, "%ld%s%03ld%s%03ld", size/MB, gs, (size%MB)/KB, gs, size%KB);
      else               sprintf(p, "%ld%s%03ld%s%03ld%s%03ld", size/GB, gs, (size%GB)/MB, gs, (size%MB)/KB, gs, size%KB);
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
/// SelectMessage
//  Activates a message in the message listview
int SelectMessage(struct Mail *mail)
{
   struct MailInfo *mi;
   MA_ChangeFolder(mail->Folder, TRUE);
   mi = GetMailInfo(mail);
   if (mi->Pos >= 0) set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, mi->Pos);
   return mi->Pos;
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
      BusyEnd;
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
      if (mail->Status == STATUS_NEW) { folder->New++; folder->Unread++; }
      if (mail->Status == STATUS_UNR) folder->Unread++;
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

   // lets decrease the folder statistics first
   folder->Total--;
   folder->Size -= mail->Size;
   if (mail->Status == STATUS_NEW)      { folder->New--; folder->Unread--; }
   else if (mail->Status == STATUS_UNR) folder->Unread--;

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
   for (work = folder->Messages; work; work = next)
   {
      next = work->Next;
      free(work);
   }
   if (resetstats) folder->Total = folder->New = folder->Unread = folder->Size = 0;
   folder->Messages = NULL;
}
///
/// GetPackMethod
//  Returns packer type and efficiency
static BOOL GetPackMethod(int xpktype, char **method, int *eff)
{
   if (xpktype == 2) { *method = C->XPKPack; *eff = C->XPKPackEff; return TRUE; }
   if (xpktype == 3) { *method = C->XPKPackEncrypt; *eff = C->XPKPackEncryptEff; return TRUE; }
   return FALSE;
}
///
/// CompressMailFile
//  Shrinks a message file
static BOOL CompressMailFile(char *src, char *dst, char *passwd, char *method, int eff)
{
   if (!XpkBase) return FALSE;
   return (BOOL)!XpkPackTags(XPK_InName, src, XPK_OutName, dst, XPK_Password, passwd, XPK_PackMethod, method, XPK_PackMode, eff, TAG_DONE);
}
///
/// UncompressMailFile
//  Expands a compressed message file
static BOOL UncompressMailFile(char *src, char *dst, char *passwd)
{
   if (!XpkBase) return FALSE;
   return (BOOL)!XpkUnpackTags(XPK_InName, src, XPK_OutName, dst, XPK_Password, passwd, TAG_DONE);
}
///
/// TransferMailFile
//  Copies or moves a message file, handles compression
BOOL TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder)
{
   char *pmeth, srcbuf[SIZE_PATHFILE], dstbuf[SIZE_PATHFILE];
   struct Folder *srcfolder = mail->Folder;
   int peff = 0;
   int srcxpk = srcfolder->XPKType, dstxpk = dstfolder->XPKType;
   char *srcpw = srcfolder->Password, *dstpw = dstfolder->Password;
   BOOL one2one, needuncomp, needcomp, done = FALSE, success = FALSE;

   if (!MA_GetIndex(srcfolder)) return FALSE;
   if (!MA_GetIndex(dstfolder)) return FALSE;
   one2one = (srcxpk == dstxpk) && (srcxpk != 3);
   needuncomp = srcxpk > 1;
   needcomp   = dstxpk > 1;
   GetPackMethod(dstxpk, &pmeth, &peff);
   GetMailFile(srcbuf, srcfolder, mail);
   strcpy(dstbuf, MA_NewMailFile(dstfolder, mail->MailFile, atoi(mail->MailFile)));
   if (one2one && !copyit) if ((done = RenameFile(srcbuf, dstbuf))) success = TRUE;
   if (!done)
   {
      if(needuncomp)
      {
        if(needcomp)
        {
          if(one2one)
          {
            success = CopyFile(dstbuf, 0, srcbuf, 0);
          }
          else
          {
            struct TempFile *tf = OpenTempFile(NULL);
            if(UncompressMailFile(srcbuf, tf->Filename, srcpw))
            {
              success = CompressMailFile(tf->Filename, dstbuf, dstpw, pmeth, peff);
              CloseTempFile(tf);
            }
          }
        }
        else
        {
          success = UncompressMailFile(srcbuf, dstbuf, srcpw);
        }
      }
      else
      {
        if(needcomp)
        {
          success = CompressMailFile(srcbuf, dstbuf, dstpw, pmeth, peff);
        }
        else
        {
          success = CopyFile(dstbuf, 0, srcbuf, 0);
        }
      }

      if (success && !copyit) DeleteFile(srcbuf);
      if (success) SetComment(dstbuf, Status[mail->Status]);
   }
   return success;
}
///
/// RepackMailFile
//  (Re/Un)Compresses a message file
//  Note: If dstxpk is -1 and passwd is NULL, then this function packs
//        the current mail. It will assume it is plaintext and needs to be packed now
BOOL RepackMailFile(struct Mail *mail, int dstxpk, char *passwd)
{
   char *pmeth = NULL, srcbuf[SIZE_PATHFILE], dstbuf[SIZE_PATHFILE];
   struct Folder *folder = mail->Folder;
   int peff = 0, srcxpk = folder->XPKType;
   BOOL success = TRUE;

   // if this function was called with dstxpk=-1 and passwd=NULL then
   // we assume we need to pack the file from plain text to the currently
   // selected pack method of the folder
   if(dstxpk == -1 && passwd == NULL)
   {
      srcxpk = XPK_OFF;
      dstxpk = folder->XPKType;
      passwd = folder->Password;
   }

   MA_GetIndex(folder);
   GetMailFile(srcbuf, folder, mail);
   GetPackMethod(dstxpk, &pmeth, &peff);
   sprintf(dstbuf, "%s.tmp", srcbuf);
   switch (4*srcxpk+dstxpk)
   {
      case  0: case  5: case 10: case 15:
      case  1: case  4:                   return TRUE;
      case  2: case  3: case  6: case  7: if ((success = CompressMailFile(srcbuf, dstbuf, passwd, pmeth, peff)))
                                          {
                                             DeleteFile(srcbuf);
                                             success = RenameFile(dstbuf, srcbuf);
                                          }
                                          break;
      case  8: case 9:  case 12: case 13: if ((success = UncompressMailFile(srcbuf, dstbuf, folder->Password)))
                                          {
                                             DeleteFile(srcbuf);
                                             success = RenameFile(dstbuf, srcbuf);
                                          }
                                          break;
      case 11: case 14:                   if ((success = UncompressMailFile(srcbuf, dstbuf, folder->Password)))
                                          {
                                             success = CompressMailFile(dstbuf, srcbuf, passwd, pmeth, peff);
                                             DeleteFile(dstbuf);
                                          }
                                          break;
   }
   MA_SetMailStatus(mail, mail->Status);
   return success;
}
///
/// DoPack
//  Compresses a file
BOOL DoPack(char *file, char *newfile, struct Folder *folder)
{
   char *pmeth = NULL;
   int peff = 0;

   GetPackMethod(folder->XPKType, &pmeth, &peff);
   if (!CompressMailFile(file, newfile, folder->Password, pmeth, peff)) return FALSE;
   DeleteFile(file);
   return TRUE;
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

         sprintf(nfile, "%s_%08lx.unp", FilePart(file), folder);

         strmfp(newfile, C->TempDir, nfile);
         if (FileSize(newfile) < 0) if (!UncompressMailFile(file, newfile, folder ? folder->Password : "")) return NULL;
      }
      else strcpy(newfile, file);
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
   if (strstr(file, ".unp"))
   {
      int i;

      // now we check if the file is still in use by a read window somewhere
      for(i=0; i < MAXRE; i++)
      {
        // if this file is still in use we cannot delete it.
        if(G->RE[i] && strcmp(file, G->RE[i]->File) == 0) return;
      }

      DeleteFile(file);
    }
}
///
/// IsValidMailFile
//  Checks if a message file name is valid
BOOL IsValidMailFile(char *fname)
{
   int l = strlen(fname);
   if (l < 7 || l > 10 || fname[5] != '.') return FALSE;
   while (--l >= 0) if (l != 5 && !isdigit(fname[l])) return FALSE;
   return TRUE;
}
///

/*** Editor related ***/
/// EditorToFile
//  Saves contents of a texteditor object to a file
BOOL EditorToFile(Object *editor, char *file, struct TranslationTable *tt)
{
   char *text;
   UBYTE *p;
   FILE *fh;

   if (!(fh = fopen(file, "w"))) return FALSE;
   text = (char *)DoMethod((Object *)editor, MUIM_TextEditor_ExportText);
   if (tt) for (p = text; *p; ++p) *p = tt->Table[*p];
   fputs(text, fh);
   fclose(fh);
   FreeVec(text);
   return TRUE;
}
///
/// FileToEditor
//  Loads a file into a texteditor object
BOOL FileToEditor(char *file, Object *editor)
{
   char *text = FileToBuffer(file);
   char *parsedText;

   if (!text) return FALSE;

   parsedText = ParseEmailText(text);
   set(editor, MUIA_TextEditor_Contents, parsedText);

   free(parsedText);
   free(text);

   return TRUE;
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
HOOKPROTONH(PO_SetPublicKey, void, APTR pop, APTR string)
{
   char *var, buf[SIZE_SMALL];

   DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
   if (var)
   {
      strcpy(buf, "0x");
      strncat(buf, var, 8);
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

   get(pop, MUIA_UserData, &str); secret = str;
   if (G->PGPVersion == 5)
      retc = PGPCommand("pgpk", "-l +language=us", KEEPLOG);
   else
   {
      strcpy(buf, "-kv  ");
      if (secret)
      {
         GetVar("PGPPATH", &buf[4], SIZE_DEFAULT, 0);
         if ((p = buf[strlen(buf)-1]) != ':' && p != '/') strcat(buf, "/");
         strcat(buf, "secring.pgp");
      }
      retc = PGPCommand("pgp", buf, KEEPLOG);
   }
   if (!retc) if ((fp = fopen(PGPLOGFILE, "r")))
   {
      get(string, MUIA_String_Contents, &str);
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
                  if (!strncmp(buf, "uid", 3)) { strncat(entry, &buf[4], SIZE_DEFAULT-9); break; }
            }
         }
         else
         {
            if (buf[9] == '/' && buf[23] == '/')
            {
               memcpy(entry, &buf[10], 8);
               strncat(entry, &buf[29], SIZE_DEFAULT-8);
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
#ifdef UNUSED
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
      MUIA_String_MaxLen     , maxlen,
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_ControlChar       , ShortCut(label),
      MUIA_CycleChain        , 1,
   End;
}
///
/// MakePassString
//  Creates a MUI string object with hidden text
Object *MakePassString(char *label)
{
   return BetterStringObject,
      StringFrame,
      MUIA_String_MaxLen     , SIZE_PASSWORD,
      MUIA_String_Secret     , TRUE,
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_ControlChar       , ShortCut(label),
      MUIA_CycleChain        , 1,
   End;
}
///
/// MakeInteger
//  Creates a MUI string object for numeric input
Object *MakeInteger(int maxlen, char *label)
{
   Object *str = MakeString(maxlen+1, label);
   if (str)
   {
      SetAttrs(str, MUIA_String_Integer,  0,
                    MUIA_String_Accept,   "0123456789",
                    TAG_DONE);
   }
   return str;
}
///
/// MakePGPKeyList
//  Creates a PGP id popup list
Object *MakePGPKeyList(APTR *st, BOOL secret, char *label)
{
   APTR po, lv;

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
      DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
   return po;
}
///
/// MakePicture
//  Creates a MUI image object that uses image datatypes
Object *MakePicture(char *fname)
{
   return G->DtpicSupported ?
      MUI_NewObject("Dtpic.mui", MUIA_Dtpic_Name, fname, End :
      NewObject(CL_BodyChunk->mcc_Class,NULL, MUIA_Bodychunk_File, fname, End;
}
///
/// MakeStatusFlag
//  Creates a MUI object for status images
Object *MakeStatusFlag(char *fname)
{
   return NewObject(CL_BodyChunk->mcc_Class,NULL,
      MUIA_Bodychunk_File, fname,
      MUIA_Bodychunk_UseOld, TRUE,
      MUIA_Bitmap_Transparent, 0,
   End;
}
///
/// MakeFolderImage
//  Creates a MUI object for a folder image
Object *MakeFolderImage(char *fname)
{
   return NewObject(CL_BodyChunk->mcc_Class,NULL,
      MUIA_Bodychunk_File, fname,
      MUIA_Bodychunk_UseOld, TRUE,
      MUIA_Bitmap_Transparent, 0,
   End;
}
///
/// MakeAddressField
//  Creates a recipient field
Object *MakeAddressField(APTR *string, char *label, APTR help, int abmode, int winnum, BOOL allowmulti)
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
   return
   NumericbuttonObject,
      MUIA_Numeric_Min, min,
      MUIA_Numeric_Max, max,
      MUIA_Numeric_Format, percent ? "%ld%%" : "%ld",
      MUIA_CycleChain, 1,
   End;
}
///
/// MakeMenuitem
//  Creates a menu item from a catalog string
Object *MakeMenuitem(const UBYTE *str, ULONG ud)
{
   if (str == NULL) return (MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End);

   if (str[1] == '\0')
      return (MenuitemObject,
         MUIA_Menuitem_Title, str+2,
         MUIA_Menuitem_Shortcut, str,
         MUIA_UserData, ud,
         End);
   else
      return (MenuitemObject,
         MUIA_Menuitem_Title, str,
         MUIA_UserData, ud,
         End);
}
///
/// SetupToolbar
//  Initializes a single button in a MUI toolbar object
void SetupToolbar(struct MUIP_Toolbar_Description *tb, char *label, char *help, UWORD flags)
{
   tb->Type = label ? (*label ? TDT_BUTTON : TDT_SPACE) : TDT_END;
   tb->Flags = flags;
   tb->ToolText = tb->Type == TDT_BUTTON ? label : NULL;
   tb->HelpString = help;
   tb->MutualExclude = 0; tb->Key = 0;
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
Object * STDARGS DoSuperNew(struct IClass *cl, Object *obj, ...)
{
  return (Object *)DoSuperMethod(cl, obj, OM_NEW, (&obj+1), NULL);
}
#endif
///
/// xget()
//  Gets an attribute value from a MUI object
ULONG xget(Object *obj, ULONG attr)
{
   ULONG b = 0;
   get(obj, attr, &b);
   return b;
}
///
/// GetMUIString
//  Returns the value of a MUI string object
void GetMUIString(char *a,Object *obj)
{
   strcpy(a,(char*)xget(obj,MUIA_String_Contents));
}
///
/// GetMUIText
//  Returns the value of a MUI text object
void GetMUIText(char *a,Object *obj)
{
   strcpy(a,(char*)xget(obj,MUIA_Text_Contents));
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
   int isopen, isicon;
   set(obj, MUIA_Window_Open, TRUE);
   get(obj, MUIA_Window_Open, &isopen);
   get(_app(obj), MUIA_Application_Iconified, &isicon);
   if (isopen || isicon) return TRUE;
   DisplayBeep(0);
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
   DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENV);
   if (!*(ls = (STRPTR)xget(G->MA->GUI.ST_LAYOUT, MUIA_String_Contents))) ls = "35 100 25 100 30 100";
   sscanf(ls, "%ld %ld %ld %ld %ld %ld", &G->Weights[0], &G->Weights[1], &G->Weights[2], &G->Weights[3], &G->Weights[4], &G->Weights[5]);
}
///
/// SaveLayout
//  Saves column widths to ENV(ARC):MUI/YAM.cfg
void SaveLayout(BOOL permanent)
{
   char buf[SIZE_DEFAULT];
   sprintf(buf, "%ld %ld %ld %ld %ld %ld", G->Weights[0], G->Weights[1], G->Weights[2], G->Weights[3], G->Weights[4], G->Weights[5]);
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
}
///
/// ConvertKey
//  Converts input event to key code
ULONG ConvertKey(struct IntuiMessage *imsg)
{
   struct InputEvent event;
   UBYTE code = 0;
   event.ie_NextEvent    = NULL;
   event.ie_Class        = IECLASS_RAWKEY;
   event.ie_SubClass     = 0;
   event.ie_Code         = imsg->Code;
   event.ie_Qualifier    = imsg->Qualifier;
   event.ie_EventAddress = (APTR *) *((ULONG *)imsg->IAddress);
   MapRawKey(&event, &code, 1, NULL);
   return code;
}
///

/**** BodyChunk ****/
/// FreeBCImage
//  Frees a bodychunk image
void FreeBCImage(struct BodyChunkData *bcd)
{
   if (bcd)
   {
      if (bcd->Colors) free(bcd->Colors);
      if (bcd->Body) free(bcd->Body);
      free(bcd);
   }
}
///
/// GetBCImage
//  Searches for a bodychunk image by filename
struct BodyChunkData *GetBCImage(char *fname)
{
   int i;

   for (i = 0; i < MAXIMAGES; i++)
   {
      if (G->BImage[i] && !strcmp(G->BImage[i]->File, fname)) return G->BImage[i];
   }

   return NULL;
}
///
/// LoadBCImage
//  Loads a bodychunk image from disk
struct BodyChunkData *LoadBCImage(char *fname)
{
   struct BodyChunkData *bcd=calloc(1,sizeof(struct BodyChunkData));
   if (bcd)
   {
      struct IFFHandle *iff=AllocIFF();
      if (iff)
      {
         if ((iff->iff_Stream = Open(fname, MODE_OLDFILE)))
         {
            InitIFFasDOS(iff);
            if (!OpenIFF(iff, IFFF_READ))
            {
               if (!ParseIFF(iff, IFFPARSE_STEP))
               {
                  struct ContextNode *cn=CurrentChunk(iff);
                  if (cn && (cn->cn_ID == ID_FORM) && (cn->cn_Type == ID_ILBM))
                  {
                     if (!PropChunk (iff, ID_ILBM, ID_BMHD) &&
                         !PropChunk (iff, ID_ILBM, ID_CMAP) &&
                         !StopChunk (iff, ID_ILBM, ID_BODY) &&
                         !StopOnExit(iff, ID_ILBM, ID_FORM) &&
                         !ParseIFF  (iff, IFFPARSE_SCAN))
                     {
                        struct StoredProperty *sp;
                        if ((sp = FindProp(iff, ID_ILBM, ID_CMAP)))
                        {
                           bcd->Colors = calloc((size_t)sp->sp_Size, sizeof(ULONG));
                           if (bcd->Colors)
                           {
                              int i;
                              for (i = 0; i < sp->sp_Size; i++)
                              {
                                 ULONG c = ((UBYTE *)sp->sp_Data)[i];
                                 bcd->Colors[i] = (c *= 0x01010101);
                              }
                           }
                        }
                        if ((sp = FindProp(iff,ID_ILBM,ID_BMHD)))
                        {
                           struct BitMapHeader *bmhd = (struct BitMapHeader *)sp->sp_Data;
                           if (bmhd->bmh_Compression == cmpNone || bmhd->bmh_Compression==cmpByteRun1)
                           {
                              LONG size = CurrentChunk(iff)->cn_Size;
                              if ((bcd->Body = calloc((size_t)size,1)))
                              {
                                 if (ReadChunkBytes(iff, bcd->Body, size) == size)
                                 {
                                    bcd->Width  = bmhd->bmh_Width;
                                    bcd->Height = bmhd->bmh_Height;
                                    bcd->Depth = bmhd->bmh_Depth;
                                    bcd->Compression = bmhd->bmh_Compression;
                                    bcd->Masking = bmhd->bmh_Masking;
                                    strcpy(bcd->File, FilePart(fname));
                                 }
                              }
                           }
                        }
                     }
                  }
               }

               CloseIFF(iff);
            }

            Close(iff->iff_Stream);
         }

         FreeIFF(iff);
      }

      if (bcd->Depth) return bcd;

      FreeBCImage(bcd);
   }

   return NULL;
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
      if (GetVar("PGPPASS", G->PGPPassPhrase, SIZE_DEFAULT, 0) < 0)
      {
         char pgppass[SIZE_DEFAULT];
         G->PGPPassVolatile = TRUE; *pgppass = 0;
         if (StringRequest(pgppass, SIZE_DEFAULT, "PGP", GetStr(MSG_UT_PGPPassReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->MA->GUI.WI))
            strcpy(G->PGPPassPhrase, pgppass);
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
   BPTR fhi,fho;
   int error = -1;
   char command[SIZE_LARGE];

   DB(kprintf("PGPCommand: [%s] [%s] - %ld error: ", progname, options, flags);)

   if ((fhi = Open("NIL:", MODE_OLDFILE)))
   {
      if ((fho = Open("NIL:", MODE_NEWFILE)))
      {
         BusyText(GetStr(MSG_BusyPGPrunning), "");
         strmfp(command, C->PGPCmdPath, progname);
         strcat(command, " >" PGPLOGFILE " ");
         strcat(command, options);
         error = SystemTags(command, SYS_Input, fhi, SYS_Output, fho, NP_StackSize, C->StackSize, TAG_DONE);
         Close(fho);
         BusyEnd;
      }
      Close(fhi);
   }
   if (error > 0 && !hasNoErrorsFlag(flags)) ER_NewError(GetStr(MSG_ER_PGPreturnsError), command, PGPLOGFILE);
   if (error < 0) ER_NewError(GetStr(MSG_ER_PGPnotfound), C->PGPCmdPath, NULL);
   if (!error && !hasKeepLogFlag(flags)) DeleteFile(PGPLOGFILE);

   DB(kprintf("%ld\n", error);)
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
      sprintf(filename, "YAM-%s%d.log", months[cd.month-1], cd.year);
   }
   else strcpy(filename, "YAM.log");
   strmfp(logfile, *C->LogfilePath ? C->LogfilePath : G->ProgDir, filename);
   if ((fh = fopen(logfile, "a")))
   {
      fprintf(fh, "%s [%02d] ", DateStamp2String(NULL, DSS_DATETIME, TZC_NONE), id);
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

   if(text)
   {
      if(*text)
      {
        sprintf(infotext[BusyLevel], text, parameter);

        if(G->MA)
        {
          if(max > 0)
          {
            DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowGauge, infotext[BusyLevel], cur, max);
          }
          else
          {
            DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowInfoText, infotext[BusyLevel]);
          }
        }

        if(BusyLevel < BUSYLEVEL-1) BusyLevel++;
        DB(else kprintf("Error: reached highest BusyLevel!!!\n");)
      }
      else
      {
         if(BusyLevel) BusyLevel--;

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
      if (G->MA && BusyLevel > 0)
      {
        DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_ShowGauge, NULL, cur, max);
      }
   }
}

///
/// DisplayAppIconStatistics
//  Calculates AppIconStatistic and update the AppIcon
void DisplayAppIconStatistics(void)
{
  struct Folder *fo;
  struct Folder **flist;
  char *src, dst[10];
  int i, mode;
  static char apptit[SIZE_DEFAULT/2];
  int new_msg = 0;
  int unr_msg = 0;
  int tot_msg = 0;
  int snt_msg = 0;
  int del_msg = 0;

  if(!(flist = FO_CreateList())) return;

  for (i = 1; i <= (int)*flist; i++)
  {
    fo = flist[i];
    if(!fo) break;

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

  // we set the mode accordingly to the status of the folder (new/check/old)
  if(G->TR && G->TR->Checking) mode = 3;
  else mode = tot_msg ? (new_msg ? 2 : 1) : 0;

  // clear AppIcon Label first before we create it new
  apptit[0] = '\0';

  // Lets create the label of the AppIcon now
  for (src = C->AppIconText; *src; src++)
  {
    if (*src == '%')
    {
      switch (*++src)
      {
        case '%': strcpy(dst, "%");            break;
        case 'n': sprintf(dst, "%d", new_msg); break;
        case 'u': sprintf(dst, "%d", unr_msg); break;
        case 't': sprintf(dst, "%d", tot_msg); break;
        case 's': sprintf(dst, "%d", snt_msg); break;
        case 'd': sprintf(dst, "%d", del_msg); break;
      }
    }
    else
    {
      sprintf(dst, "%c", *src);
    }

    strcat(apptit, dst);
  }

  // We first have to remove the appicon before we can change it
  if (G->AppIcon)
  {
    RemoveAppIcon(G->AppIcon);
    G->AppIcon = NULL;
  }

  // Now we create the new AppIcon and display it
  if (G->DiskObj[mode])
  {
    struct DiskObject *dobj=G->DiskObj[mode];
    G->AppIcon = AddAppIcon(0, 0, (STRPTR)apptit, G->AppPort, NULL, dobj, TAG_DONE);
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

      switch(mail->Status)
      {
        case STATUS_NEW:  { fo->New++; fo->Unread++;  } break;
        case STATUS_UNR:  { fo->Unread++;             } break;
        case STATUS_SNT:  { fo->Sent++;               } break;
        case STATUS_DEL:  { fo->Deleted++;            } break;
      }
   }

   // if this folder hasn`t got any own folder image in the folder
   // directory and it is one of our standard folders we have to check which image we put in front of it
   if (!fo->BC_FImage)
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
      if ((PrintIO = CreateIORequest(PrintPort, sizeof(struct IOStdReq))))
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
    if((G->NewMailSound_Obj = NewDTObject(filename,
	  		DTA_GroupID, GID_SOUND,
		  	TAG_DONE)))
  	{
      // create a datatype trigger
	  	struct dtTrigger dtt;

  		// Fill the trigger
	  	dtt.MethodID     = DTM_TRIGGER;
		  dtt.dtt_GInfo    = NULL;
  		dtt.dtt_Function = STM_PLAY;
	  	dtt.dtt_Data     = NULL;

  		// Play the sound by calling DoDTMethodA()
      DoDTMethodA(G->NewMailSound_Obj, NULL, NULL, (Msg)&dtt);
	  }
  }
}
///
/// MatchExtension
//  Matches a file extension against a list of extension
BOOL MatchExtension(char *fileext, char *extlist)
{
   while ((extlist = strtok(extlist, " ")))
   {
      if (!stricmp(extlist, fileext)) return TRUE;
      extlist = NULL;
   }
   return FALSE;
}
///
/// IdentifyFileDT
//  Detects the file type using datatypes.library
static char *IdentifyFileDT(char *fname)
{
   static char ctype[SIZE_CTYPE];

   strcpy(ctype, "application/octet-stream");
   if (DataTypesBase)
   {
      BPTR lock = Lock(fname, ACCESS_READ);
      if (lock)
      {
         struct DataType *dtn = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL);
         if (dtn)
         {
            char *type = NULL;
            struct DataTypeHeader *dth = dtn->dtn_Header;
            switch (dth->dth_GroupID)
            {
               case GID_SYSTEM:     break;
               case GID_DOCUMENT:   type = "application"; break;
               case GID_TEXT:       type = "text"; break;
               case GID_SOUND:
               case GID_INSTRUMENT: type = "audio"; break;
               case GID_PICTURE:    type = "image"; break;
               case GID_MOVIE:
               case GID_ANIMATION:  type = "video"; break;
            }
            if (type) sprintf(ctype, "%s/x-%s", type, dth->dth_BaseName);
            ReleaseDataType(dtn);
         }
         UnLock (lock);
      }
   }
   return ctype;
}
///
/// IdentifyFile
//  Detects the file type by analyzing file extension and contents
char *IdentifyFile(char *fname)
{
   char *ctype = "";
   FILE *fh;

   if ((fh = fopen(fname, "r")))
   {
      int i, len;
      char buffer[SIZE_LARGE], *ext;

      len = fread(buffer, 1, SIZE_LARGE-1, fh);
      buffer[len] = 0;
      fclose(fh);
      if ((ext = strrchr(fname, '.'))) ++ext;
      else ext = "--";

      if (!stricmp(ext, "htm") || !stricmp(ext, "html"))                          ctype = ContType[CT_TX_HTML];
      else if (!strnicmp(buffer, "@database", 9) || !stricmp(ext, "guide"))       ctype = ContType[CT_TX_GUIDE];
      else if (!stricmp(ext, "ps") || !stricmp(ext, "eps"))                       ctype = ContType[CT_AP_PS];
      else if (!stricmp(ext, "pdf") || !strncmp(buffer, "%PDF-", 5))              ctype = ContType[CT_AP_PDF];
      else if (!stricmp(ext, "rtf"))                                              ctype = ContType[CT_AP_RTF];
      else if (!stricmp(ext, "lha") || !strncmp(&buffer[2], "-lh5-", 5))          ctype = ContType[CT_AP_LHA];
      else if (!stricmp(ext, "lzx") || !strncmp(buffer, "LZX", 3))                ctype = ContType[CT_AP_LZX];
      else if (!stricmp(ext, "zip"))                                              ctype = ContType[CT_AP_ZIP];
      else if (*((long *)buffer) >= HUNK_UNIT && *((long *)buffer) <= HUNK_INDEX) ctype = ContType[CT_AP_AEXE];
      else if (!stricmp(ext, "rexx") || !stricmp(ext+strlen(ext)-2, "rx"))        ctype = ContType[CT_AP_REXX];
      else if (!strncmp(&buffer[6], "JFIF", 4))                                   ctype = ContType[CT_IM_JPG];
      else if (!strncmp(buffer, "GIF8", 4))                                       ctype = ContType[CT_IM_GIF];
      else if (!strnicmp(ext, "png",4) || !strncmp(&buffer[1], "PNG", 3))         ctype = ContType[CT_IM_PNG];
      else if (!strnicmp(ext, "tif",4))                                           ctype = ContType[CT_IM_TIFF];
      else if (!strncmp(buffer, "FORM", 4) && !strncmp(&buffer[8], "ILBM", 4))    ctype = ContType[CT_IM_ILBM];
      else if (!stricmp(ext, "au") || !stricmp(ext, "snd"))                       ctype = ContType[CT_AU_AU];
      else if (!strncmp(buffer, "FORM", 4) && !strncmp(&buffer[8], "8SVX", 4))    ctype = ContType[CT_AU_8SVX];
      else if (!stricmp(ext, "wav"))                                              ctype = ContType[CT_AU_WAV];
      else if (!stricmp(ext, "mpg") || !stricmp(ext, "mpeg"))                     ctype = ContType[CT_VI_MPG];
      else if (!stricmp(ext, "qt") || !stricmp(ext, "mov"))                       ctype = ContType[CT_VI_MOV];
      else if (!strncmp(buffer, "FORM", 4) && !strncmp(&buffer[8], "ANIM", 4))    ctype = ContType[CT_VI_ANIM];
      else if (!stricmp(ext, "avi"))                                              ctype = ContType[CT_VI_AVI];
      else if (stristr(buffer, "\nFrom:"))                                        ctype = ContType[CT_ME_EMAIL];
      else
      {
         for (i = 1; i < MAXMV; i++) if (C->MV[i])
         {
            if (MatchExtension(ext, C->MV[i]->Extension)) ctype = C->MV[i]->ContentType;
         }
      }

      if (!*ctype)
      {
         int c, notascii = 0;
         for (i = 0; i < len; i++)
            if (c=(int)buffer[i],c < 32 || c > 127)
               if (c != '\t' && c != '\n') notascii++;
         if (notascii < len/10) ctype =  ContType[(FileProtection(fname)&FIBF_SCRIPT) ? CT_AP_SCRIPT : CT_TX_PLAIN];
         else ctype = IdentifyFileDT(fname);
      }
   }
   return ctype;
}
///
/// LoadTranslationTable
//  Load a translation table into memory
BOOL LoadTranslationTable(struct TranslationTable **tt, char *file)
{
   FILE *fp;
   if (*tt) free(*tt);
   *tt = NULL;
   if (!file || !*file) return FALSE;
   if (!(*tt = calloc(1,sizeof(struct TranslationTable)))) return FALSE;
   if ((fp = fopen(file, "r")))
   {
      UBYTE buf[SIZE_DEFAULT], *p;
      int i;
      for (i = 0; i < 256; i++) (*tt)->Table[i] = (UBYTE)i;
      MyStrCpy((*tt)->File, file);
      fgets(buf, SIZE_DEFAULT, fp);
      if (!strncmp(buf, "YCT1", 4))
      {
         fgets((*tt)->Name, SIZE_DEFAULT, fp);
         if ((p = strchr((*tt)->Name,'\n'))) *p = 0;
         while (fgets(buf, SIZE_DEFAULT, fp))
            if (!strnicmp(buf, "from", 4))
            {
              MyStrCpy((*tt)->SourceCharset, Trim(&buf[5]));
            }
            else if (!strnicmp(buf, "to", 2))
            {
              MyStrCpy((*tt)->DestCharset, Trim(&buf[3]));
            }
            else if (!strnicmp(buf, "header", 6)) (*tt)->Header = TRUE;
            else if (!strnicmp(buf, "author", 6));
            else if (strchr(buf, '='))
            {
               int source, dest;
               p = buf;
               if (*p == '$') sscanf(&p[1], "%x", &source); else source = (int)*p;
               while (*p++ != '=');
               if (*p == '$') sscanf(&p[1], "%x", &dest); else dest = (int)*p;
               if (source >= 0 && source <= 0xFF && dest >= 0 && dest <= 0xFF) (*tt)->Table[source] = (UBYTE)dest;
            }
         fclose(fp);
         return TRUE;
      }
      fclose(fp);
   }
   ER_NewError(GetStr(MSG_ER_ErrorTTable), file, NULL);
   free(*tt); *tt = NULL;
   return FALSE;
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
BOOL ExecuteCommand(char *cmd, BOOL asynch, BPTR outdef)
{
   BPTR in, out;
   int ret;

   DB(kprintf("Execute cmd: [%s]", cmd);)

   switch (outdef)
   {
      case OUT_DOS: in = Input(); out = Output(); break;
      case OUT_NIL: out = Open("NIL:", MODE_NEWFILE); in = Open("NIL:", MODE_OLDFILE); break;
      default:      out = outdef; in = Open("NIL:", MODE_OLDFILE); break;
   }

   if (!outdef) asynch = FALSE;

   if (WBmsg)
   {
      BPTR path = CloneWorkbenchPath(WBmsg);

      DB(kprintf(" with CloneWBPath()\n");)

      if ((ret = SystemTags(cmd,
                            SYS_Input,    in,
                            SYS_Output,   out,
                            NP_Path,      path,
                            NP_StackSize, C->StackSize,
                            SYS_Asynch,   asynch,
                            TAG_DONE)) == -1)
      {
        FreeWorkbenchPath(path);
      }
   }
   else
   {
      DB(kprintf(" without CloneWBPath()\n");)

      ret = SystemTags(cmd,
                       SYS_Input,     in,
                       SYS_Output,    out,
                       NP_StackSize,  C->StackSize,
                       SYS_Asynch,    asynch,
                       TAG_DONE);
   }

   if (ret == -1 && asynch && outdef) { Close(out); Close(in); }

   return (BOOL)(!ret);
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
   if (C->RX[MACRO_URL].Script[0])
   {
      char newurl[SIZE_LARGE];
      sprintf(newurl, "%c%s%c", '"', url, '"');
      MA_StartMacro(MACRO_URL, newurl);
   }
   else if ((OpenURLBase = OpenLibrary("openurl.library", 1)))
   {
      URL_Open(url, TAG_DONE);
      CloseLibrary(OpenURLBase);
   }
}
///
/// strtok_r()
// Reentrant version of stdlib strtok()
// Call like this:
// char *next=input, *token, breakstring[]=", ";
// do { token = strtok_r(&next,breakstring); /* ... */ } while(next);
char *strtok_r(char **s, char *brk)
{
char *p,*ret;

  if((s == NULL) || (*s == NULL) || (brk == NULL))
    return NULL;

  /* find break character */
  if((p = strpbrk(*s,brk)))
  {
    /* if found, terminate string there */
    *p = '\0';

    /* scan forward to next non-break */
    do { p++; } while(*p && (strchr(brk,*p) != NULL));

    /* if *p is a nullbyte, then no more tokens */
    if(*p == '\0') p = NULL;

    /* save current *s to return it */
    ret = *s;

    /* and let *s point to first non-break */
    *s = p;

    return ret;
  }

  /* no break character found - *s gets NULL */
  ret = *s;
  *s = NULL;

  return ret;
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
   char *buf;
   get(obj, MUIA_String_Contents, &buf);
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
HOOKPROTO(putCharFunc, void, struct Locale *locale, int c)
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
void STDARGS SPrintF(char *outstr, char *fmtstr, ...)
{
  struct Hook hook;
  va_list args;

  InitHook(&hook, putCharHook, outstr);
  va_start(args, fmtstr);
#if defined(__MORPHOS__)
  FormatString(G->Locale, fmtstr, args->overflow_arg_area, &hook);
#else
  FormatString(G->Locale, fmtstr, args, &hook);
#endif
  va_end(args);
}
///

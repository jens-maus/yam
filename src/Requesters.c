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

#include <clib/alib_protos.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "classes/Classes.h"

#include "FolderList.h"
#include "Locale.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

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
  if(tit != NULL)
    title = strdup(tit);

  if(gad != NULL)
    gadgets = strdup(gad);

  // lets create the requester text
  va_start(args, format);
  vsnprintf(reqtxt, sizeof(reqtxt), format, args);
  va_end(args);

  // if the applicationpointer is NULL we fall back to a standard requester
  if(app == NULL)
  {
    if(IntuitionBase != NULL)
    {
      struct EasyStruct ErrReq;

      ErrReq.es_StructSize   = sizeof(struct EasyStruct);
      ErrReq.es_Flags        = 0;
      ErrReq.es_Title        = title;
      ErrReq.es_TextFormat   = reqtxt;
      ErrReq.es_GadgetFormat = gadgets;

      result = EasyRequestArgs(NULL, &ErrReq, NULL, NULL);
    }

    if(title != NULL)
      free(title);

    if(gadgets != NULL)
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
  if(WI_YAMREQ != NULL)
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
        if((next = strchr(token, '|')) != NULL)
          *next++ = '\0';

        if(*token == '*')
        {
          active=TRUE;
          token++;
        }

        if((ul = strchr(token, '_')) != NULL)
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

          if(ie == TRUE && num_gads == 2)
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

          if(i <= 8)
          {
            // by default we set it to "-capslock f1" so that we can press f1
            // even if the capslock is on.
            char fstring[13];

            snprintf(fstring, sizeof(fstring), "-capslock f%d", i+1);
            DoMethod(WI_YAMREQ, MUIM_Notify, MUIA_Window_InputEvent, fstring, app, 2, MUIM_Application_ReturnID, i+1);
          }

          DoMethod(BT_TEMP, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_Application_ReturnID, i+1);

          if(active == TRUE)
          {
            set(WI_YAMREQ, MUIA_Window_ActiveObject, BT_TEMP);
            active = FALSE;
          }
        }

        // write back what we took.
        if(next != NULL)
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

    if(SafeOpenWindow(WI_YAMREQ) == FALSE)
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

  if(title != NULL)
    free(title);

  if(gadgets != NULL)
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

  ENTER();

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

  if(wi_sr != NULL)
  {
    setstring(st_in, string);
    set(wi_sr, MUIA_Window_ActiveObject, st_in);
    set(G->App, MUIA_Application_Sleep, TRUE);

    DoMethod(G->App, OM_ADDMEMBER, wi_sr);

    DoMethod(bt_okay,   MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
    DoMethod(st_in,     MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
    DoMethod(wi_sr,     MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);

    if(alttext != NULL)
      DoMethod(bt_middle, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 2);

    // lets collect the waiting returnIDs now
    COLLECT_RETURNIDS;

    if(SafeOpenWindow(wi_sr) == FALSE)
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

  RETURN(ret_code);
  return ret_code;
}

///
/// PassphraseRequest
//  Puts up a string requester for entering a PGP passphrase
int PassphraseRequest(char *string, int size, Object *parent)
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

  if(wi_sr != NULL)
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

    if(SafeOpenWindow(wi_sr) == FALSE)
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

  if(wi_fr != NULL)
  {
    char *fname;
    static int lastactive;

    LockFolderListShared(G->folders);

    if(IsFolderListEmpty(G->folders) == FALSE)
    {
      struct FolderNode *fnode;

      ForEachFolderNode(G->folders, fnode)
      {
        if(fnode->folder != exclude && !isGroupFolder(fnode->folder))
          DoMethod(lv_folder, MUIM_List_InsertSingle, fnode->folder->Name, MUIV_List_Insert_Bottom);
      }
    }

    UnlockFolderList(G->folders);

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

    if(SafeOpenWindow(wi_fr) == FALSE)
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

          if((folder = FO_GetFolderByName(fname, NULL)) != NULL)
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
/// AttachDspFunc
//  Attachment listview display hook
HOOKPROTONHNO(AttachDspFunc, LONG, struct NList_DisplayMessage *msg)
{
  ENTER();

  if(msg != NULL)
  {
    // now we set our local variables to the DisplayMessage structure ones
    struct Part *entry = (struct Part *)msg->entry;
    char **array = msg->strings;

    if(entry != NULL)
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
      if(isAlternativePart(entry) == TRUE && entry->Parent != NULL && entry->Parent->MainAltPart != entry)
        msg->preparses[1] = (char *)MUIX_I;

      if(entry->Size > 0)
      {
        array[2] = dispsz;

        if(isDecoded(entry))
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
  }

  RETURN(0);
  return 0;
}
MakeStaticHook(AttachDspHook, AttachDspFunc);

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
  if(wi_ar != NULL)
  {
    static struct Part spart[2];

    // add the window to our application object
    DoMethod(G->App, OM_ADDMEMBER, wi_ar);

    // lets create the static parts of the Attachrequest entries in the NList
    spart[0].Nr = PART_ORIGINAL;
    strlcpy(spart[0].Name, tr(MSG_RE_Original), sizeof(spart[0].Name));
    spart[0].Size = rmData->mail->Size;
    SET_FLAG(spart[0].Flags, PFLAG_DECODED);
    DoMethod(lv_attach, MUIM_NList_InsertSingle, &spart[0], MUIV_NList_Insert_Top);
    set(lv_attach, MUIA_NList_Active, MUIV_NList_Active_Top);

    // if this AttachRequest isn't a DISPLAY request we show all the option to select the text we actually see
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
      if(!isPrintReq(mode) || isPrintable(part))
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
    if(SafeOpenWindow(wi_ar) == FALSE)
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
  if(tit != NULL)
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

  if(wi_cb != NULL)
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

    if(SafeOpenWindow(wi_cb) == FALSE)
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

  if(title != NULL)
    free(title);

  RETURN(result);
  return result;
}

///

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

#include "YAM.h"

/***************************************************************************
 Private MUI classes
***************************************************************************/

/// Definitions
struct MUI_CustomClass *CL_TextEditor;
struct MUI_CustomClass *CL_BodyChunk;
struct MUI_CustomClass *CL_FolderList;
struct MUI_CustomClass *CL_AddressList;
struct MUI_CustomClass *CL_AttachList;
struct MUI_CustomClass *CL_DDString;
struct MUI_CustomClass *CL_DDList;
struct MUI_CustomClass *CL_MainWin;
struct MUI_CustomClass *CL_PageList;
///
/// BC_Dispatcher (BodyChunk)
//  Subclass of BodyChunk, can load images from files
ULONG SAVEDS ASM BC_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   struct BC_Data *data;
   struct TagItem *tags, *tag;
   int useold;

   switch (msg->MethodID)
   {
      case OM_NEW:
         tags = ((struct opSet *)msg)->ops_AttrList;
         obj = (Object *)DoSuperNew(cl, obj,
            MUIA_FixWidth, 16,
            MUIA_FixHeight, 16,
            MUIA_InnerBottom, 0,
            MUIA_InnerLeft, 0,
            MUIA_InnerRight, 0,
            MUIA_InnerTop, 0,
            TAG_MORE, tags);
         if (obj)
         {
            char fname[SIZE_PATHFILE];
            useold = FALSE;
            *fname = 0;
            data = INST_DATA(cl,obj);
            while (tag = NextTagItem(&tags))
            {
               switch (tag->ti_Tag)
               {
                  case MUIA_Bodychunk_UseOld:
                     if (tag->ti_Data) useold = (BOOL)tag->ti_Data;
                     break;
                  case MUIA_Bodychunk_File:
                     if (tag->ti_Data) stccpy(fname, (char *)tag->ti_Data, SIZE_PATHFILE);
                     break;
               }
            }
            if (*fname)
            {
               if (useold) data->BCD = GetBCImage(fname);
                      else data->BCD = LoadBCImage(fname);

               if (data->BCD)
               {
                  set(obj, MUIA_FixWidth,             data->BCD->Width);
                  set(obj, MUIA_FixHeight,            data->BCD->Height);
                  set(obj, MUIA_Bitmap_Width,         data->BCD->Width);
                  set(obj, MUIA_Bitmap_Height,        data->BCD->Height);
                  set(obj, MUIA_Bitmap_SourceColors,  data->BCD->Colors);
                  set(obj, MUIA_Bodychunk_Depth,      data->BCD->Depth);
                  set(obj, MUIA_Bodychunk_Body,       data->BCD->Body);
                  set(obj, MUIA_Bodychunk_Compression,data->BCD->Compression);
                  set(obj, MUIA_Bodychunk_Masking,    data->BCD->Masking);
                  set(obj, MUIA_UserData,             useold);
               }
               else return 0;
            }
         }
         return (ULONG)obj;

      case OM_DISPOSE:
         data = INST_DATA(cl,obj);
         get(obj, MUIA_UserData, &useold);
         if (!useold) FreeBCImage(data->BCD);
         break;
   }
   return DoSuperMethodA(cl, obj, msg);
}
///
/// WS_Dispatcher (Recipient String)
//  Subclass of Betterstring, handles alias auto-completion, drag&drop from address book
ULONG SAVEDS ASM WS_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   ULONG result = 0;
   UBYTE code;
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
   struct WS_Data *data = (struct WS_Data *)INST_DATA(cl,obj);
   struct MUI_NListtree_TreeNode *active;
   struct MUIP_HandleEvent *hmsg;
   
   switch(msg->MethodID)
   {
      case OM_NEW:
         result = DoSuperNew(cl, obj, StringFrame, MUIA_CycleChain, 1, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);
         break;
      case MUIM_Setup:
         if(DoSuperMethodA(cl, obj, msg))
         {
            data->ehnode.ehn_Priority = 1;
            data->ehnode.ehn_Flags    = 0;
            data->ehnode.ehn_Object   = obj;
            data->ehnode.ehn_Class    = cl;
            data->ehnode.ehn_Events   = IDCMP_RAWKEY;
            result = TRUE;
         }
         break;
      case MUIM_GoActive:
         DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
         result = DoSuperMethodA(cl, obj, msg);
         break;
      case MUIM_GoInactive:
         DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
         set(obj, MUIA_BetterString_SelectSize, 0);
         result = DoSuperMethodA(cl, obj, msg);
         break;
      case MUIM_HandleEvent:
         hmsg = (struct MUIP_HandleEvent *)msg;
         if (hmsg->imsg && hmsg->imsg->Class == IDCMP_RAWKEY)
         {
            char *contents, *newcontents, *completed = NULL, *comma;
            int pos, allowmulti;
            if (hmsg->imsg->Code == 95 && (hmsg->imsg->Qualifier & IEQUALIFIER_CONTROL))
            {
               DoSuperMethodA(cl, obj, msg);
               get(obj, MUIA_String_Contents, &contents);
               get(obj, MUIA_UserData, &allowmulti);
               if (completed = WR_ExpandAddresses(-1, contents, FALSE, !allowmulti))
               {
                  setstring(obj, completed);
                  FreeStrBuf(completed);
               }
               result = MUI_EventHandlerRC_Eat;
            }
            else
            {
               if (hmsg->imsg->Code == 65) DoMethod(obj, MUIM_BetterString_ClearSelected);
               code = ConvertKey(hmsg->imsg);
               if ((((code >= 32 && code <= 126) || code >= 160) && !(hmsg->imsg->Qualifier & IEQUALIFIER_RCOMMAND)) || (code && hmsg->imsg->Qualifier & IEQUALIFIER_CONTROL))
               {
                  DoSuperMethodA(cl, obj, msg);
                  get(obj, MUIA_String_Contents, &contents);
                  get(obj, MUIA_String_BufferPos, &pos);
                  if (strlen(contents) > 1)
                  {
                     if (comma = strrchr(contents,','))
                     {
                        while (*++comma == ' ');
                        if (strlen(comma) > 1) completed = AB_CompleteAlias(comma);
                     }
                     else completed = AB_CompleteAlias(contents);
                     if (completed)
                     {
                        newcontents = malloc(strlen(contents)+strlen(completed)+1);
                        strcpy(newcontents, contents);
                        strcpy(&newcontents[pos], completed);
                        SetAttrs(obj, MUIA_String_Contents,newcontents, MUIA_String_BufferPos,pos, MUIA_BetterString_SelectSize,strlen(newcontents)-pos, TAG_DONE);
                        free(newcontents);
                     }
                  }
                  result = MUI_EventHandlerRC_Eat;
               }
            }
         }
         break;
      case MUIM_DragQuery:
         result = MUIV_DragQuery_Refuse;
         if (d->obj == G->MA->GUI.NL_MAILS) result = MUIV_DragQuery_Accept;
         else if (d->obj == G->AB->GUI.LV_ADRESSES)
            if (active = (struct MUI_NListtree_TreeNode *)DoMethod(d->obj, MUIM_NListtree_GetEntry, NULL, MUIV_NListtree_GetEntry_Position_Active, 0))
               if (!(active->tn_Flags & TNF_LIST)) result = MUIV_DragQuery_Accept;
         break;
      case MUIM_DragDrop:
         if (d->obj == G->MA->GUI.NL_MAILS)
         {
            struct Mail *mail;
            DoMethod(d->obj, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
            if (OUTGOING(mail->Folder->Type)) AB_InsertAddress(obj, "", mail->To.RealName, mail->To.Address);
            else AB_InsertAddress(obj, "", mail->From.RealName, mail->From.Address);
         }
         else if (d->obj == G->AB->GUI.LV_ADRESSES)
         {
            struct MUI_NListtree_TreeNode *active = (struct MUI_NListtree_TreeNode *)DoMethod(d->obj, MUIM_NListtree_GetEntry, NULL, MUIV_NListtree_GetEntry_Position_Active, 0);
            struct ABEntry *addr = (struct ABEntry *)(active->tn_User);
            AB_InsertAddress(obj, addr->Alias, addr->RealName, "");
         }
         break;
      default:
         result = DoSuperMethodA(cl, obj, msg);
         break;
   }
   return result;

}
///
/// WL_Dispatcher (Attachment List)
//  Subclass of List, adds Drag&Drop from message list
ULONG SAVEDS ASM WL_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

   switch (msg->MethodID)
   {
      case OM_NEW:
         return DoSuperNew(cl, obj, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);
      case MUIM_Setup:
         if (!DoSuperMethodA(cl, obj, msg)) return FALSE;
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         return TRUE;
      case MUIM_Cleanup:
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         break;
      case MUIM_DragQuery:
         if (d->obj == G->MA->GUI.NL_MAILS) return MUIV_DragQuery_Accept;
         break;
      case MUIM_DragDrop:
         if (d->obj == G->MA->GUI.NL_MAILS)
         {
            struct Attach attach;
            struct Mail *mail;
            int id = MUIV_NList_NextSelected_Start;
            while (TRUE)
            {
               DoMethod(d->obj, MUIM_NList_NextSelected, &id); if (id == MUIV_NList_NextSelected_End) break;
               DoMethod(d->obj, MUIM_NList_GetEntry, id, &mail);
               clear(&attach, sizeof(struct Attach));
               GetMailFile(attach.FilePath, NULL, mail);
               stccpy(attach.Description, mail->Subject, SIZE_DEFAULT);
               strcpy(attach.ContentType, "message/rfc822");
               attach.Size = mail->Size;
               attach.IsMIME = TRUE;
               DoMethod(obj, MUIM_List_InsertSingle, &attach, MUIV_List_Insert_Bottom);
            }
            return 0;
         }
         break;
   }
   return DoSuperMethodA(cl, obj, msg);
}
///
/// FL_Dispatcher (Folder Listtree)
//  Subclass of NList, adds Drag&Drop from message list
ULONG SAVEDS ASM FL_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   struct MUIP_DragQuery *dq = (struct MUIP_DragQuery *)msg;
//   struct MUIP_NList_DropType *dt = (struct MUIP_NList_DropType *)msg;
   struct Folder *srcfolder, *dstfolder;
   struct MUI_NListtree_TreeNode *tn_src, *tn_dst;
   int pos = 0;

   switch (msg->MethodID)
   {
      case MUIM_DragQuery:
      {
         if (dq->obj == G->MA->GUI.NL_MAILS) return MUIV_DragQuery_Accept;
			}
    	break;

      case MUIM_DragDrop:
      {
         // if a folder is dragged on a folder we break here and the SuperClass should handle the msg
         if (dq->obj == obj) break;

         // if this is a drag&drop from one folder to another we get the source and dest
         get(obj, MUIA_NList_DropMark, &pos);

         tn_dst = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, pos, 0, TAG_DONE);
         if(!tn_dst) return 0;
				 dstfolder = tn_dst->tn_User;

         tn_src = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, MUIV_NListtree_GetEntry_Position_Active, 0, TAG_DONE);
         if(!tn_src) return 0;
				 srcfolder = tn_src->tn_User;

         if (dstfolder->Type != FT_GROUP) MA_MoveCopy(NULL, srcfolder, dstfolder, FALSE);
         return 0;
      }
    	break;
   }

   return DoSuperMethodA(cl,obj,msg);
}
///
/// EL_Dispatcher (Member List)
//  Subclass of List, adds Drag&Drop from address book window
ULONG SAVEDS ASM EL_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
   struct MUI_NListtree_TreeNode *active;

   switch (msg->MethodID)
   {
      case MUIM_DragQuery:
         if (d->obj == obj) break;
         if (d->obj == G->AB->GUI.LV_ADRESSES && d->obj != obj)
            if (active = (struct MUI_NListtree_TreeNode *)DoMethod(d->obj, MUIM_NListtree_GetEntry, NULL, MUIV_NListtree_GetEntry_Position_Active, 0))
               if (!((struct ABEntry *)(active->tn_User))->Members) return MUIV_DragQuery_Accept;
         return MUIV_DragQuery_Refuse;
      case MUIM_DragDrop:
         if (d->obj == obj) break;
         if (d->obj == G->AB->GUI.LV_ADRESSES && d->obj != obj)
            if (active = (struct MUI_NListtree_TreeNode *)DoMethod(d->obj, MUIM_NListtree_GetEntry, NULL, MUIV_NListtree_GetEntry_Position_Active, 0))
               if (active->tn_Flags & TNF_LIST) EA_AddMembers(obj, active);
               else EA_AddSingleMember(obj, active);
         return 0;
   }
   return DoSuperMethodA(cl,obj,msg);
}
///
/// AL_Dispatcher (Address book Listtree)
//  Subclass of Listtree, supports inline images and Drag&Drop from message list
ULONG SAVEDS ASM AL_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   struct AL_Data *data;
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

   switch (msg->MethodID)
   {
      case OM_NEW:
			{
         obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);

         if (obj)
         {
            struct AL_Data *data = INST_DATA(cl, obj);
	    			InitHook(&data->DisplayHook, AB_LV_DspFunc, data);
            set(obj, MUIA_NListtree_DisplayHook, &data->DisplayHook);

         }
         return (ULONG)obj;
      }
    	break;

      case MUIM_Setup:
 			{
         if (!DoSuperMethodA(cl, obj, msg)) return FALSE;
         data = INST_DATA(cl, obj);
         data->Object = NewObject(CL_BodyChunk->mcc_Class,NULL,
                                  MUIA_Bodychunk_File, "status_group",
                                  MUIA_Bodychunk_UseOld, TRUE,
                                  MUIA_Bitmap_Transparent, 0,
                              End;
         data->Image = (APTR)DoMethod(obj, MUIM_List_CreateImage, data->Object, 0);
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         return TRUE;
      }
    	break;

      case MUIM_Cleanup: 
			{
         data = INST_DATA(cl, obj);
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         DoMethod(obj, MUIM_List_DeleteImage, data->Image);
         if (data->Object) MUI_DisposeObject(data->Object);
      }
    	break;

      case MUIM_DragQuery:
			{
         if (d->obj == G->MA->GUI.NL_MAILS) return MUIV_DragQuery_Accept;
			}
      break;

      case MUIM_DragDrop:
			{
   			if (d->obj == G->MA->GUI.NL_MAILS)
        {
        	struct Mail **mlist = MA_CreateMarkedList(d->obj);
          if (mlist) { MA_GetAddress(mlist); free(mlist); }
        }
    	}
      break;
   }

   return DoSuperMethodA(cl,obj,msg);
}
///
/// MW_Dispatcher (Main Window)
//  Subclass of Windows, used to dispose subwindows on exit
struct MUIP_MainWindow_CloseWindow { ULONG MethodID; APTR Window; };
ULONG SAVEDS ASM MW_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   if (msg->MethodID == MUIM_MainWindow_CloseWindow)
   {
      APTR app, win = ((struct MUIP_MainWindow_CloseWindow *)msg)->Window;
      set(win, MUIA_Window_Open, FALSE);
      get(win, MUIA_ApplicationObject, &app);
      DoMethod(app, OM_REMMEMBER, win);
      MUI_DisposeObject(win);
   }
   else return DoSuperMethodA(cl, obj, (Msg)msg);

   return 0;
}
///
/// TE_Dispatcher (Text Editor)
//  Subclass of Texteditor, adds error requester, Drag&Drop capabilities and multi-color support
ULONG SAVEDS ASM TE_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,struct MUIP_TextEditor_HandleError *msg))
{
   switch (msg->MethodID)
   {
      case MUIM_DragQuery:
      {
         struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;
         return (ULONG)(drop_msg->obj == G->AB->GUI.LV_ADRESSES);
      }
      case MUIM_DragDrop:
      {
         struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;
         if (drop_msg->obj == G->AB->GUI.LV_ADRESSES)
         {
            struct MUI_NListtree_TreeNode *tn;
            if (tn = (struct MUI_NListtree_TreeNode *)DoMethod(drop_msg->obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Active, MUIV_NListtree_GetEntry_Position_Active, 0))
            {
               struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
               if (ab->Type != AET_GROUP)
               {
                  char *adr = AllocStrBuf(SIZE_DEFAULT);
                  WR_ResolveName(-1, ab->Alias, &adr, FALSE);
                  DoMethod(obj, MUIM_TextEditor_InsertText, adr, MUIV_TextEditor_InsertText_Cursor);
                  FreeStrBuf(adr);
               }
            }
         }
         break;
      }
      case MUIM_TextEditor_HandleError:
      {
         char *errortxt = NULL;
         switch (msg->errorcode)
         {
            case Error_ClipboardIsEmpty:  errortxt = GetStr(MSG_CL_ErrorEmptyCB); break;
            case Error_ClipboardIsNotFTXT:errortxt = GetStr(MSG_CL_ErrorNotFTXT); break;
            case Error_NoAreaMarked:      errortxt = GetStr(MSG_CL_ErrorNoArea); break;
            case Error_NothingToRedo:     errortxt = GetStr(MSG_CL_ErrorNoRedo); break;
            case Error_NothingToUndo:     errortxt = GetStr(MSG_CL_ErrorNoUndo); break;
            case Error_NotEnoughUndoMem:  errortxt = GetStr(MSG_CL_ErrorNoUndoMem); break;
         }
         if (errortxt) MUI_Request(_app(obj), _win(obj), 0L, NULL, GetStr(MSG_OkayReq), errortxt);
         break;
      }
      case MUIM_Show:
      {
         G->EdColMap[6] = MUI_ObtainPen(muiRenderInfo(obj), &C->ColoredText, 0);
         G->EdColMap[7] = MUI_ObtainPen(muiRenderInfo(obj), &C->Color2ndLevel, 0);
         break;
      }
      case MUIM_Hide:
      {
         if (G->EdColMap[6] >= 0) MUI_ReleasePen(muiRenderInfo(obj), G->EdColMap[6]);
         if (G->EdColMap[7] >= 0) MUI_ReleasePen(muiRenderInfo(obj), G->EdColMap[7]);
         break;
      }
   }
   return DoSuperMethodA(cl, obj, (Msg)msg);
}
///
/// PL_Dispatcher (Config Window Page List)
//  Subclass of List, adds small images to configuration menu
ULONG SAVEDS ASM PL_Dispatcher(REG(a0,struct IClass *cl), REG(a2,Object *obj), REG(a1,Msg msg))
{
   extern UBYTE PL_IconBody[MAXCPAGES][240];
   const ULONG PL_Colors[24] = {
      0x95959595,0x95959595,0x95959595, 0x00000000,0x00000000,0x00000000,
      0xffffffff,0xffffffff,0xffffffff, 0x3b3b3b3b,0x67676767,0xa2a2a2a2,
      0x7b7b7b7b,0x7b7b7b7b,0x7b7b7b7b, 0xafafafaf,0xafafafaf,0xafafafaf,
      0xaaaaaaaa,0x90909090,0x7c7c7c7c, 0xffffffff,0xa9a9a9a9,0x97979797 };
   struct PL_Data *data;
   int i;

   switch (msg->MethodID)
   {
      case OM_NEW:
         obj = (Object *)DoSuperNew(cl, obj, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);
         if (obj)
         {
	    struct PL_Data *data = INST_DATA(cl,obj);
	    InitHook(&data->DisplayHook, CO_PL_DspFunc, data);
            set(obj, MUIA_List_DisplayHook, &data->DisplayHook);
         }
         return (ULONG)obj;
      case MUIM_Setup:
         if (!DoSuperMethodA(cl, obj, msg)) return FALSE;
         data = INST_DATA(cl, obj);
         for (i = 0; i < MAXCPAGES; i++)
         {
            data->Object[i] = BodychunkObject,
               MUIA_FixWidth             , 23,
               MUIA_FixHeight            , 15,
               MUIA_Bitmap_Width         , 23,
               MUIA_Bitmap_Height        , 15,
               MUIA_Bitmap_SourceColors  , PL_Colors,
               MUIA_Bodychunk_Depth      , 3,
               MUIA_Bodychunk_Body       , PL_IconBody[i],
               MUIA_Bodychunk_Compression, 1,
               MUIA_Bodychunk_Masking    , 2,
               MUIA_Bitmap_Transparent   , 0,
            End;
            data->Image[i] = (APTR)DoMethod(obj, MUIM_List_CreateImage, data->Object[i], 0);
         }
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         return TRUE;
      case MUIM_Cleanup:
         data = INST_DATA(cl, obj);
         MUI_RequestIDCMP(obj, IDCMP_MOUSEBUTTONS|IDCMP_RAWKEY);
         for (i = 0; i < MAXCPAGES; i++)
         {
            DoMethod(obj, MUIM_List_DeleteImage, data->Image[i]);
            if (data->Object[i]) MUI_DisposeObject(data->Object[i]);
         }
   }
   return DoSuperMethodA(cl, obj, msg);
}
///

/// ExitClasses
//  Remove custom MUI classes
void ExitClasses(void)
{
   if (CL_PageList   ) MUI_DeleteCustomClass(CL_PageList   );
   if (CL_MainWin    ) MUI_DeleteCustomClass(CL_MainWin    );
   if (CL_TextEditor ) MUI_DeleteCustomClass(CL_TextEditor );
   if (CL_BodyChunk  ) MUI_DeleteCustomClass(CL_BodyChunk  );
   if (CL_FolderList ) MUI_DeleteCustomClass(CL_FolderList );
   if (CL_AddressList) MUI_DeleteCustomClass(CL_AddressList);
   if (CL_DDString   ) MUI_DeleteCustomClass(CL_DDString   );
   if (CL_DDList     ) MUI_DeleteCustomClass(CL_DDList     );
   if (CL_AttachList ) MUI_DeleteCustomClass(CL_AttachList );
}
///
/// InitClasses
//  Initialize custom MUI classes
BOOL InitClasses(void)
{
	CL_AttachList  = MUI_CreateCustomClass(NULL, MUIC_NList        , NULL, sizeof(struct DumData), ENTRY(WL_Dispatcher));
	CL_DDList      = MUI_CreateCustomClass(NULL, MUIC_List         , NULL, sizeof(struct DumData), ENTRY(EL_Dispatcher));
	CL_DDString    = MUI_CreateCustomClass(NULL, MUIC_BetterString , NULL, sizeof(struct WS_Data), ENTRY(WS_Dispatcher));
	CL_AddressList = MUI_CreateCustomClass(NULL, MUIC_NListtree    , NULL, sizeof(struct AL_Data), ENTRY(AL_Dispatcher));
	CL_FolderList  = MUI_CreateCustomClass(NULL, MUIC_NListtree    , NULL, sizeof(struct DumData), ENTRY(FL_Dispatcher));
	CL_BodyChunk   = MUI_CreateCustomClass(NULL, MUIC_Bodychunk    , NULL, sizeof(struct BC_Data), ENTRY(BC_Dispatcher));
	CL_TextEditor  = MUI_CreateCustomClass(NULL, MUIC_TextEditor   , NULL, sizeof(struct DumData), ENTRY(TE_Dispatcher));
	CL_MainWin     = MUI_CreateCustomClass(NULL, MUIC_Window       , NULL, sizeof(struct DumData), ENTRY(MW_Dispatcher));
	CL_PageList    = MUI_CreateCustomClass(NULL, MUIC_List         , NULL, sizeof(struct PL_Data), ENTRY(PL_Dispatcher));

	return (BOOL)(CL_AttachList && CL_DDList && CL_DDString && CL_AddressList && CL_FolderList && CL_BodyChunk &&
                CL_TextEditor && CL_MainWin && CL_PageList);
}
///

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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/pm.h>
#include <proto/utility.h>

#include "extra.h"
#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_classes.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

/***************************************************************************
 Module: Private MUI classes
***************************************************************************/

struct DumData { long dummy; };

struct BC_Data
{
   struct BodyChunkData *BCD;
};

/*** Definitions ***/
struct MUI_CustomClass *CL_TextEditor;
struct MUI_CustomClass *CL_BodyChunk;
struct MUI_CustomClass *CL_FolderList;
struct MUI_CustomClass *CL_MailList;
struct MUI_CustomClass *CL_AddressList;
struct MUI_CustomClass *CL_AttachList;
struct MUI_CustomClass *CL_DDList;
struct MUI_CustomClass *CL_MainWin;
struct MUI_CustomClass *CL_PageList;

/// BC_Dispatcher (BodyChunk)
/*** BC_Dispatcher (BodyChunk) - Subclass of BodyChunk, can load images from files ***/
DISPATCHERPROTO(BC_Dispatcher)
{
   struct BC_Data *data;
   struct TagItem *tags, *tag;
   int useold;

   switch (msg->MethodID)
   {
      case OM_NEW:
      {
         tags = ((struct opSet *)msg)->ops_AttrList;

         obj = DoSuperNew(cl, obj,
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
            }
         }
         return (ULONG)obj;
      }
      break;


      case OM_DISPOSE:
      {
         data = INST_DATA(cl,obj);
         get(obj, MUIA_UserData, &useold);
         if (!useold && data->BCD) FreeBCImage(data->BCD);
      }
      break;
   }
   return DoSuperMethodA(cl, obj, msg);
}

///
/// WL_Dispatcher (Attachment List)
/*** WL_Dispatcher (Attachment List) - Subclass of List, adds Drag&Drop from message list ***/
DISPATCHERPROTO(WL_Dispatcher)
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

   switch (msg->MethodID)
   {
      case OM_NEW:
         return (ULONG)DoSuperNew(cl, obj, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);
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
               memset(&attach, 0, sizeof(struct Attach));
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
/// FL_Dispatcher (Folder NListtree)
/*** FL_Dispatcher (Folder NListtree) - Subclass of NList, adds Drag&Drop from message list ***/
DISPATCHERPROTO(FL_Dispatcher)
{
   struct Folder *srcfolder, *dstfolder;
   struct MUI_NListtree_TreeNode *tn_src, *tn_dst;

//   DB(if(msg->MethodID != 0x9d510090) kprintf("%lx\n", msg->MethodID);)

   switch (msg->MethodID)
   {
      // we catch MUIM_DragReport because we want to restrict some dragging for some special objects
      case MUIM_DragReport:
      {
        struct MUIP_DragReport *dr = (struct MUIP_DragReport *)msg;
        struct MUI_NListtree_TestPos_Result res;
        struct MUI_NListtree_TreeNode *tn;

        DoMethod(obj, MUIM_NListtree_TestPos, dr->x, dr->y, &res);

        if(tn = res.tpr_TreeNode)
        {
          struct Folder *folder = (struct Folder *)tn->tn_User;

          // If we drag a folder on a folder we reject it immediatly because only below or above
          // is allowed
          if(dr->obj == obj)
          {
            if(folder->Type != FT_GROUP && res.tpr_Type == MUIV_NListtree_TestPos_Result_Onto)
            {
              return(MUIV_DragReport_Abort);
            }
          }
          else
          {
            // If we drag a mail onto a folder we allow only dragging on and not below or above
            if(folder->Type == FT_GROUP || res.tpr_Type != MUIV_NListtree_TestPos_Result_Onto)
            {
              return(MUIV_DragReport_Abort);
            }
          }

          // to rescue the dropping we call the SuperMethod now
          return(DoSuperMethodA(cl, obj, msg));
        }

        return(MUIV_DragReport_Abort);

      }
      break;

      case MUIM_DragQuery:
      {
         struct MUIP_DragQuery *dq = (struct MUIP_DragQuery *)msg;

         if (dq->obj == G->MA->GUI.NL_MAILS) return MUIV_DragQuery_Accept;
      }
      break;

      case MUIM_DragDrop:
      {
         struct MUIP_DragDrop *dd = (struct MUIP_DragDrop *)msg;

         // if a folder is dragged on a folder we break here and the SuperClass should handle the msg
         if (dd->obj == obj) break;

         tn_dst = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget);
         if(!tn_dst) return 0;
         dstfolder = tn_dst->tn_User;

         tn_src = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_Active);
         if(!tn_src) return 0;
         srcfolder = tn_src->tn_User;

         if (dstfolder->Type != FT_GROUP) MA_MoveCopy(NULL, srcfolder, dstfolder, FALSE);
         return 0;
      }
      break;

      // If the user tried to open a ContextMenu we check if
      // popupmenu.library was open and then display the ContextMenu
      case MUIM_ContextMenuBuild:
      {
         if(PopupMenuBase && C->FolderCntMenu) return(MA_FolderContextMenu((APTR)msg));
         else return(0);
      }
      break;
   }

   return DoSuperMethodA(cl,obj,msg);
}

///
/// ML_Dispatcher (Mail NListview)
/*** ML_Dispatcher (Mail NListview) - Subclass of NList, adds ContextMenuBuild to Message List ***/
DISPATCHERPROTO(ML_Dispatcher)
{
  switch (msg->MethodID)
  {
    // If the user tried to open a ContextMenu we check if
    // popupmenu.library was open and then display the ContextMenu
    case MUIM_ContextMenuBuild:
    {
      if(PopupMenuBase) return(MA_MailListContextMenu((APTR)msg));
      else return(0);
    }
    break;
  }

  return DoSuperMethodA(cl,obj,msg);
}

///
/// EL_Dispatcher (Member List)
/*** EL_Dispatcher (Member List) - Subclass of List, adds Drag&Drop from address book window ***/
DISPATCHERPROTO(EL_Dispatcher)
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
   struct MUI_NListtree_TreeNode *active;

   switch (msg->MethodID)
   {
      case MUIM_DragQuery:
         if (d->obj == obj) break;
         if (d->obj == G->AB->GUI.LV_ADDRESSES && d->obj != obj)
            if (active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active))
               if (!((struct ABEntry *)(active->tn_User))->Members) return MUIV_DragQuery_Accept;
         return MUIV_DragQuery_Refuse;
      case MUIM_DragDrop:
         if (d->obj == obj) break;
         if (d->obj == G->AB->GUI.LV_ADDRESSES && d->obj != obj)
            if (active = (struct MUI_NListtree_TreeNode *)xget(d->obj, MUIA_NListtree_Active))
               if (isFlagSet(active->tn_Flags, TNF_LIST)) EA_AddMembers(obj, active);
               else EA_AddSingleMember(obj, active);
         return 0;
   }
   return DoSuperMethodA(cl,obj,msg);
}

///
/// AL_Dispatcher (Address book NListtree)
/*** AL_Dispatcher (Address book NListtree) -
     Subclass of Listtree, supports inline images and Drag&Drop from message list ***/
DISPATCHERPROTO(AL_Dispatcher)
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

   switch (msg->MethodID)
   {
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

struct MUIP_MainWindow_CloseWindow { ULONG MethodID; APTR Window; };

/*** MW_Dispatcher (Main Window) - Subclass of Windows, used to dispose subwindows on exit ***/
DISPATCHERPROTO(MW_Dispatcher)
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
/*** TE_Dispatcher (Text Editor) - Subclass of Texteditor, adds
     error requester, Drag&Drop capabilities and multi-color support ***/
DISPATCHERPROTO(TE_Dispatcher)
{
   switch (msg->MethodID)
   {
      case MUIM_DragQuery:
      {
         struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;
         return (ULONG)(drop_msg->obj == G->AB->GUI.LV_ADDRESSES);
      }
      case MUIM_DragDrop:
      {
         struct MUIP_DragDrop *drop_msg = (struct MUIP_DragDrop *)msg;
         if (drop_msg->obj == G->AB->GUI.LV_ADDRESSES)
         {
            struct MUI_NListtree_TreeNode *tn;
            if (tn = (struct MUI_NListtree_TreeNode *)xget(drop_msg->obj, MUIA_NListtree_Active))
            {
               struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
               if (ab->Type != AET_GROUP)
               {
                  DoMethod(obj, MUIM_TextEditor_InsertText, AB_PrettyPrintAddress(ab), MUIV_TextEditor_InsertText_Cursor);
               }
            }
         }
         break;
      }
      case MUIM_TextEditor_HandleError:
      {
         char *errortxt = NULL;
         switch(((struct MUIP_TextEditor_HandleError *)msg)->errorcode)
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

   return DoSuperMethodA(cl, obj, msg);
}
///

/// Images
//  Images for section listview in ILBM/BODY format
static const UBYTE PL_IconBody[MAXCPAGES][240] = {
{ /* 0 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0xff,0x00,0x01,0x10,0x00,0xff,0x00,0x01,0xf0,
0x00,0xff,0x00,0x01,0x18,0x00,0xff,0x00,0x01,0xf8,0x00,0x03,0x00,0x01,0x88,
0x00,0xfd,0x00,0x03,0x00,0x01,0xf0,0x00,0x03,0x00,0x03,0xc0,0x00,0xff,0x00,
0x01,0x0c,0x00,0x03,0x00,0x02,0xe0,0x00,0x03,0x00,0x06,0x80,0x00,0x03,0x00,
0x01,0x18,0x00,0x03,0x00,0x04,0xc0,0x00,0x03,0x00,0x0c,0x00,0x00,0x03,0x00,
0x03,0x30,0x00,0x03,0x11,0x09,0x80,0x00,0x03,0x1f,0x1c,0x00,0x00,0x03,0x11,
0x06,0x60,0x00,0x03,0x0f,0x93,0x00,0x00,0x03,0x00,0x3c,0x00,0x00,0x03,0x00,
0x8c,0xc0,0x00,0x03,0x05,0xf6,0x00,0x00,0x03,0x03,0x38,0x00,0x00,0x03,0x01,
0x59,0x80,0x00,0x03,0x03,0xfc,0x00,0x00,0x03,0x01,0xf0,0x00,0x00,0x03,0x01,
0xb3,0x00,0x00,0x03,0x01,0xb8,0x00,0x00,0x03,0x00,0xe0,0x00,0x00,0x03,0x00,
0xa6,0x00,0x00,0x03,0x00,0xb0,0x00,0x00,0x03,0x00,0x40,0x00,0x00,0x03,0x00,
0x0c,0x00,0x00,0x03,0x00,0x60,0x00,0x00,0xfd,0x00,0x03,0x00,0x18,0x00,0x00,
0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 1 */
0x03,0x1c,0x00,0x70,0x00,0xfd,0x00,0x03,0x1c,0x00,0x70,0x00,0x03,0x22,0x00,
0x88,0x00,0xfd,0x00,0x03,0x3e,0x00,0xf8,0x00,0x03,0x57,0xff,0x5c,0x00,0x03,
0x1e,0x00,0x78,0x00,0x03,0x75,0xff,0xd4,0x00,0x03,0x4f,0xff,0x3c,0x00,0x03,
0x1d,0xff,0x70,0x00,0x03,0x6c,0x00,0xb4,0x00,0x03,0x5a,0xfe,0xec,0x00,0x03,
0x18,0x00,0xe0,0x00,0x03,0x7c,0xfe,0x74,0x00,0x03,0x3f,0x7d,0x78,0x00,0x03,
0x13,0x01,0x48,0x00,0x03,0x20,0xfe,0x00,0x00,0x03,0x3f,0x45,0xf0,0x00,0x03,
0x08,0x44,0x20,0x00,0x03,0x37,0x29,0xd0,0x00,0x03,0x3f,0xd7,0xf0,0x00,0x03,
0x08,0x10,0x20,0x00,0x03,0x37,0xef,0xd0,0x00,0x03,0x23,0x45,0x88,0x00,0x03,
0x00,0x44,0x00,0x00,0x03,0x3f,0x29,0xf8,0x00,0x03,0x57,0x7d,0xdc,0x00,0x03,
0x1f,0x01,0xf8,0x00,0x03,0x74,0xfe,0x54,0x00,0x03,0x4f,0x7f,0x3c,0x00,0x03,
0x1d,0x7f,0x70,0x00,0x03,0x6c,0x80,0xb4,0x00,0x03,0x5b,0xff,0x6c,0x00,0x03,
0x18,0x00,0x60,0x00,0x03,0x7d,0xff,0xf4,0x00,0x03,0x3f,0x00,0xf8,0x00,0x03,
0x12,0x00,0x48,0x00,0x03,0x21,0x00,0x80,0x00,0x03,0x1e,0x00,0x70,0x00,0xfd,
0x00,0x03,0x1e,0x00,0x70,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 2 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x3f,0xff,
0xf8,0x00,0x03,0x3f,0xff,0xf8,0x00,0xfd,0x00,0x03,0x20,0x00,0x18,0x00,0x03,
0x3f,0xff,0xe0,0x00,0xff,0x00,0x01,0x14,0x00,0x03,0x2e,0x01,0x58,0x00,0x03,
0x31,0xff,0xe0,0x00,0x03,0x0e,0x01,0xd4,0x00,0x03,0x24,0x01,0x98,0x00,0x03,
0x3b,0xff,0xe0,0x00,0x03,0x04,0x01,0xd4,0x00,0x03,0x20,0x01,0x18,0x00,0x03,
0x3f,0xff,0xe0,0x00,0x03,0x00,0x01,0xd4,0x00,0x03,0x20,0x6c,0x18,0x00,0x03,
0x3f,0x93,0xe0,0x00,0x03,0x00,0x6c,0x14,0x00,0x03,0x20,0x55,0x18,0x00,0x03,
0x3f,0xaa,0xe0,0x00,0x03,0x00,0x55,0x14,0x00,0x03,0x20,0x6b,0x18,0x00,0x03,
0x3f,0x94,0xe0,0x00,0x03,0x00,0x6b,0x14,0x00,0x03,0x20,0x00,0x18,0x00,0x03,
0x3f,0xff,0xe0,0x00,0xff,0x00,0x01,0x14,0x00,0x03,0x3f,0xff,0xf8,0x00,0x00,
0x20,0xfe,0x00,0x03,0x1f,0xff,0xf4,0x00,0x03,0x3f,0xff,0xf8,0x00,0xfd,0x00,
0xff,0x00,0x01,0x04,0x00,0xfd,0x00,0xfd,0x00,0x03,0x1f,0xff,0xfc,0x00,0xfd,
0x00,0xfd,0x00,0xfd,0x00, },
{ /* Folders
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x03,0xce,0x70,0x00,0x03,0x03,0xce,0x70,
0x00,0x03,0x00,0x21,0x08,0x00,0x03,0xfa,0x21,0x00,0x00,0x03,0xfb,0xef,0x70,
0x00,0x03,0x00,0x10,0x8c,0x00,0x03,0x86,0xd6,0xb4,0x00,0x03,0xff,0x7b,0xdc,
0x00,0x03,0x00,0xc6,0x30,0x00,0x03,0xbb,0xff,0x1c,0x00,0x03,0xcf,0xff,0x34,
0x00,0x03,0x38,0x00,0xda,0x00,0x03,0x80,0x00,0x7c,0x00,0xff,0xff,0x01,0x68,
0x00,0xff,0x00,0x01,0xba,0x00,0x03,0x80,0x00,0xfc,0x00,0xff,0xff,0x01,0x10,
0x00,0xff,0x00,0x01,0xba,0x00,0x03,0x9d,0x54,0xfc,0x00,0x03,0xea,0xab,0x28,
0x00,0x03,0x1d,0x54,0xba,0x00,0x03,0x8b,0xac,0xfc,0x00,0x03,0xf5,0x57,0x10,
0x00,0x03,0x0b,0xac,0xba,0x00,0x03,0x95,0x54,0xfc,0x00,0x03,0xfa,0xab,0x28,
0x00,0x03,0x15,0x54,0xba,0x00,0x03,0x8a,0xac,0xfc,0x00,0x03,0xfd,0x57,0x10,
0x00,0x03,0x0a,0xac,0xba,0x00,0x03,0x80,0x00,0xfc,0x00,0xff,0xff,0xff,0x00,
0xff,0x00,0x01,0x82,0x00,0xff,0xff,0x01,0xc0,0x00,0x00,0x80,0xfe,0x00,0x03,
0x7f,0xff,0xbe,0x00,0xff,0xff,0x01,0xc0,0x00,0xfd,0x00,0xff,0x00,0x01,0x20,
0x00,0xfd,0x00,0xfd,0x00,0x03,0x7f,0xff,0xe0,0x00, },
{  3 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x01,0xff,0x00,0x00,0x03,0x01,0xff,0x00,
0x00,0xfd,0x00,0x03,0x0e,0xa0,0xe0,0x00,0x03,0x0e,0x08,0xe0,0x00,0x03,0x01,
0xfe,0x00,0x00,0x03,0x12,0x2a,0x18,0x00,0x03,0x10,0x00,0x98,0x00,0x03,0x0a,
0x7f,0xe0,0x00,0x03,0x06,0x80,0x10,0x00,0x03,0x00,0x2a,0x00,0x00,0x03,0x1f,
0xff,0xe8,0x00,0x03,0x0c,0x28,0x30,0x00,0x03,0x08,0x02,0xa0,0x00,0x03,0x04,
0x7f,0xc8,0x00,0x03,0x02,0x80,0x30,0x00,0x03,0x00,0x2a,0x10,0x00,0x03,0x0f,
0xff,0xc0,0x00,0x03,0x06,0x20,0x60,0x00,0x03,0x04,0x0a,0xc0,0x00,0x03,0x02,
0x7f,0x90,0x00,0x03,0x00,0x80,0x60,0x00,0x03,0x00,0x2a,0x20,0x00,0x03,0x06,
0xff,0x80,0x00,0x03,0x02,0x20,0xc0,0x00,0x03,0x02,0x08,0x80,0x00,0x03,0x00,
0x7f,0x20,0x00,0xff,0x00,0x01,0xc0,0x00,0x03,0x00,0x2a,0x40,0x00,0x03,0x03,
0x7f,0x00,0x00,0xff,0x01,0x01,0x80,0x00,0x03,0x01,0x09,0x00,0x00,0x03,0x00,
0x5e,0x40,0x00,0x03,0x00,0xff,0x80,0x00,0x03,0x00,0x80,0x80,0x00,0x00,0x01,
0xfe,0x00,0xfd,0x00,0xfd,0x00,0x03,0x00,0xff,0x80,0x00,0xfd,0x00,0xfd,0x00,
0xfd,0x00, },
{ /* 4 */
0x03,0x00,0xfe,0x00,0x00,0x03,0x00,0x28,0x00,0x00,0xfd,0x00,0x03,0x01,0xff,
0x00,0x00,0x03,0x00,0x82,0x00,0x00,0x03,0x02,0x00,0x80,0x00,0x03,0x01,0x93,
0x00,0x00,0x03,0x00,0x7c,0x00,0x00,0x03,0x02,0x7c,0x80,0x00,0x03,0x01,0xbb,
0x00,0x00,0x03,0x01,0x45,0x00,0x00,0x03,0x02,0xaa,0x80,0x00,0x03,0x19,0x55,
0x30,0x00,0x03,0x09,0xab,0x20,0x00,0x03,0x02,0x00,0x88,0x00,0x03,0x1f,0xbb,
0xf0,0x00,0x03,0x12,0x44,0x90,0x00,0x03,0x08,0xaa,0x28,0x00,0x03,0x12,0x38,
0x90,0x00,0x03,0x0c,0x54,0x60,0x00,0x03,0x03,0xd7,0x88,0x00,0x03,0x38,0x10,
0x38,0x00,0x03,0x17,0xef,0xd0,0x00,0x03,0x40,0x10,0x04,0x00,0x03,0x55,0xf7,
0x54,0x00,0x03,0x3e,0x08,0xf8,0x00,0x03,0x39,0xf7,0x3a,0x00,0x03,0x55,0x11,
0x54,0x00,0x03,0x3e,0xaa,0xf8,0x00,0x03,0x39,0x55,0x3a,0x00,0x03,0x39,0xbb,
0x38,0x00,0x03,0x16,0x44,0xd0,0x00,0x03,0x41,0xbb,0x04,0x00,0xff,0x11,0x01,
0x30,0x00,0x03,0x0f,0xab,0xc0,0x00,0x03,0x01,0x55,0x28,0x00,0x03,0x0c,0x00,
0x60,0x00,0x03,0x03,0xff,0x80,0x00,0x03,0x14,0x00,0x50,0x00,0x03,0x07,0xff,
0xc0,0x00,0x00,0x04,0xfe,0x00,0xff,0x00,0x01,0x20,0x00,0xfd,0x00,0xfd,0x00,
0x03,0x03,0xff,0xc0,0x00, },
{ /* 5 */
0xff,0x00,0x01,0x1a,0x00,0xff,0x00,0x01,0x1e,0x00,0xff,0x00,0x01,0x0c,0x00,
0x03,0x00,0x60,0x24,0x00,0x03,0x00,0x60,0x38,0x00,0xff,0x00,0x01,0x08,0x00,
0x03,0x00,0x90,0x58,0x00,0x03,0x00,0xf0,0x70,0x00,0xff,0x00,0x01,0x10,0x00,
0x03,0x01,0x28,0xb0,0x00,0x03,0x01,0xf8,0xe0,0x00,0x03,0x00,0x20,0x20,0x00,
0x03,0x02,0x55,0x60,0x00,0x03,0x03,0xed,0xc0,0x00,0x03,0x00,0x50,0x40,0x00,
0x03,0x04,0xaa,0xc0,0x00,0x03,0x07,0xd5,0x80,0x00,0x03,0x00,0xa8,0x80,0x00,
0x03,0x09,0x55,0x84,0x00,0x03,0x0f,0xab,0x04,0x00,0x03,0x01,0x51,0x18,0x00,
0x03,0x12,0xab,0xa8,0x00,0x03,0x1f,0x5e,0x28,0x00,0x03,0x02,0xa2,0x54,0x00,
0x03,0x25,0x5e,0x40,0x00,0x03,0x3e,0xbc,0x00,0x00,0x03,0x05,0x4d,0xb0,0x00,
0x03,0x4a,0xbd,0x60,0x00,0x03,0x7d,0x58,0x20,0x00,0x03,0x0a,0xb3,0xc0,0x00,
0x03,0x15,0x7b,0x50,0x00,0x03,0x7a,0x88,0xb0,0x00,0x03,0x15,0x67,0x40,0x00,
0x03,0x2a,0xaa,0xa8,0x00,0x03,0x75,0x55,0x58,0x00,0x03,0x2a,0xaa,0xa0,0x00,
0xff,0x55,0x01,0x54,0x00,0x03,0x6a,0xaa,0xac,0x00,0xff,0x55,0x01,0x50,0x00,
0xff,0x55,0x01,0x56,0x00,0xff,0x00,0x01,0x02,0x00,0x03,0x7f,0xff,0xfc,0x00,
0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 6 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0xff,0x00,0x01,0x20,0x00,0xff,0x00,0x01,0x40,
0x00,0xfd,0x00,0x03,0x00,0x80,0x10,0x00,0x03,0x00,0x7f,0xc0,0x00,0x03,0x00,
0x80,0x20,0x00,0x03,0x01,0x00,0x08,0x00,0x03,0x00,0xf4,0x00,0x00,0x03,0x01,
0x7f,0xf0,0x00,0x03,0x02,0x00,0x18,0x00,0x03,0x01,0xfa,0x10,0x00,0x03,0x02,
0xff,0xe4,0x00,0x03,0x00,0x33,0xf0,0x00,0x03,0x03,0xdd,0x20,0x00,0x03,0x01,
0xcc,0x4c,0x00,0x03,0x00,0x69,0x20,0x00,0x03,0x03,0xa6,0x40,0x00,0x03,0x01,
0x9e,0x98,0x00,0x03,0x08,0x61,0x00,0x00,0x03,0x13,0xee,0x00,0x00,0x03,0x01,
0x96,0xf0,0x00,0x03,0x18,0xa1,0x00,0x00,0x03,0x37,0x5c,0x00,0x00,0x03,0x10,
0xee,0x80,0x00,0x03,0x3d,0x02,0x00,0x00,0x03,0x62,0xf8,0x00,0x00,0x03,0x3f,
0xfd,0x80,0x00,0x03,0x7e,0xce,0x00,0x00,0x03,0x69,0x3a,0x00,0x00,0x03,0x3f,
0xf1,0x00,0x00,0x03,0x3f,0xf8,0x00,0x00,0x03,0x30,0x08,0x00,0x00,0x03,0x10,
0x06,0x00,0x00,0x00,0x18,0xfe,0x00,0x00,0x10,0xfe,0x00,0x03,0x27,0xf8,0x00,
0x00,0xfd,0x00,0xfd,0x00,0x00,0x1c,0xfe,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 7 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x01,0x20,0x24,0x00,0x03,0x00,0x20,0x24,
0x00,0x03,0x00,0x10,0x00,0x00,0x03,0x01,0x20,0x24,0x00,0x00,0x01,0xfe,0x00,
0x00,0x02,0xfe,0x00,0x03,0x02,0x60,0x4c,0x00,0x03,0x02,0x60,0x4c,0x00,0x03,
0x01,0x00,0x20,0x00,0x03,0x02,0x40,0x48,0x00,0xfd,0x00,0xfd,0x00,0x03,0x04,
0x80,0x98,0x00,0xff,0x00,0x01,0x90,0x00,0xff,0x00,0x01,0x40,0x00,0x03,0x15,
0xa4,0x96,0x00,0x03,0x15,0x84,0x02,0x00,0x03,0x08,0x02,0x08,0x00,0x03,0x1d,
0x64,0xa2,0x00,0x03,0x08,0x20,0xa2,0x00,0x03,0x00,0x01,0x10,0x00,0x03,0x1f,
0xc9,0x24,0x00,0x03,0x1d,0x41,0x00,0x00,0xff,0x00,0x01,0x8a,0x00,0x03,0x3e,
0x5d,0xc8,0x00,0x03,0x2a,0x08,0x80,0x00,0x03,0x00,0x22,0x14,0x00,0x03,0x6c,
0x76,0x8e,0x00,0x03,0x44,0x54,0x0c,0x00,0x03,0x02,0x08,0x40,0x00,0x00,0x4c,
0xfe,0x00,0x00,0x08,0xfe,0x00,0xfd,0x00,0x00,0xd8,0xfe,0x00,0x00,0xd0,0xfe,
0x00,0x00,0x04,0xfe,0x00,0x00,0xa8,0xfe,0x00,0x00,0x88,0xfe,0x00,0x00,0x40,
0xfe,0x00,0x00,0xc0,0xfe,0x00,0x00,0x40,0xfe,0x00,0xfd,0x00, },
{ /* 8 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x00,0x78,0xfe,0x00,0x00,0x48,0xfe,0x00,0x00,
0x04,0xfe,0x00,0x00,0x8e,0xfe,0x00,0x00,0xb0,0xfe,0x00,0x00,0x49,0xfe,0x00,
0x03,0x83,0xe0,0x00,0x00,0x03,0x7d,0x60,0x00,0x00,0x03,0x02,0x10,0x00,0x00,
0x03,0xbb,0xfc,0x00,0x00,0x03,0x7e,0xec,0x00,0x00,0x03,0x3f,0x62,0x00,0x00,
0x03,0xc1,0xfb,0x78,0x00,0x03,0x3f,0xc2,0x48,0x00,0x03,0x40,0xdc,0x80,0x00,
0x03,0xff,0xf0,0x4c,0x00,0x03,0x81,0xdf,0x0c,0x00,0x03,0x00,0xb0,0xb0,0x00,
0x03,0x03,0xd9,0xbc,0x00,0x03,0x02,0xbe,0x10,0x00,0x03,0x7c,0x19,0x78,0x00,
0x03,0x03,0xe5,0xa4,0x00,0x03,0x02,0x5e,0x08,0x00,0x03,0x01,0x06,0xd0,0x00,
0x03,0x07,0x9b,0xd4,0x00,0x03,0x07,0x06,0x28,0x00,0x03,0x03,0x6a,0x82,0x00,
0x03,0x3e,0x8d,0xec,0x00,0x03,0x26,0x02,0x90,0x00,0x03,0x07,0x74,0x4a,0x00,
0x03,0x05,0x04,0xfc,0x00,0x03,0x04,0x02,0x04,0x00,0x03,0x0a,0xe9,0x72,0x00,
0x03,0x0e,0x03,0xf8,0x00,0x03,0x08,0x03,0x48,0x00,0x03,0x01,0x85,0x06,0x00,
0xfd,0x00,0xfd,0x00,0x03,0x00,0x03,0xfc,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 9 */
0x03,0x00,0x7c,0x00,0x00,0x03,0x00,0x7c,0x00,0x00,0x03,0x00,0x82,0x00,0x00,
0xff,0x01,0xff,0x00,0x03,0x01,0xf9,0x00,0x00,0xfd,0x00,0x03,0x00,0x79,0x00,
0x00,0x03,0x01,0xbc,0x00,0x00,0x00,0x02,0xfe,0x00,0x03,0x02,0x85,0x00,0x00,
0x03,0x03,0x06,0x00,0x00,0x03,0x00,0x40,0x80,0x00,0x03,0x02,0x85,0x00,0x00,
0x03,0x03,0x06,0x00,0x00,0xff,0x00,0x01,0x80,0x00,0x03,0x00,0x85,0x00,0x00,
0x03,0x01,0x06,0x00,0x00,0x03,0x02,0x00,0x80,0x00,0x03,0x05,0xbb,0x80,0x00,
0x03,0x07,0x2a,0x00,0x00,0x03,0x05,0xbb,0xc0,0x00,0x03,0x03,0x00,0x40,0x00,
0x03,0x06,0x7e,0x00,0x00,0x03,0x0b,0x7e,0xa0,0x00,0x03,0x02,0x10,0x40,0x00,
0x03,0x04,0xd6,0x00,0x00,0x03,0x0a,0xef,0xa0,0x00,0x03,0x07,0x38,0x40,0x00,
0x03,0x04,0x6e,0x00,0x00,0x03,0x0f,0x46,0xa0,0x00,0x03,0x06,0x10,0x40,0x00,
0x03,0x04,0xc6,0x00,0x00,0x03,0x0e,0xef,0xa0,0x00,0x03,0x07,0x38,0x40,0x00,
0x03,0x04,0x6e,0x00,0x00,0x03,0x0f,0x46,0xa0,0x00,0x03,0x06,0x00,0x40,0x00,
0x03,0x04,0xfe,0x00,0x00,0x03,0x0e,0xff,0xa0,0x00,0x03,0x03,0xff,0xc0,0x00,
0xff,0x00,0x01,0x40,0x00,0x03,0x04,0x00,0x20,0x00,0xfd,0x00,0xfd,0x00,0x03,
0x03,0xff,0xc0,0x00, },
{ /* 10 */
0x03,0x00,0x04,0x00,0x00,0x03,0x00,0x04,0x00,0x00,0x03,0x00,0xfb,0x00,0x00,
0x03,0x00,0xf9,0x00,0x00,0x03,0x00,0x01,0x00,0x00,0x03,0x03,0xfa,0x80,0x00,
0x03,0x01,0x1c,0x80,0x00,0x03,0x00,0xf8,0x80,0x00,0x03,0x03,0x1d,0x40,0x00,
0x03,0x02,0x7e,0xc0,0x00,0x03,0x01,0xc0,0xc0,0x00,0x03,0x06,0x7e,0x20,0x00,
0x03,0x02,0xfe,0x40,0x00,0x03,0x01,0x80,0x00,0x00,0x03,0x06,0xfe,0xa0,0x00,
0x03,0x02,0xc2,0x40,0x00,0x00,0x01,0xfe,0x00,0x03,0x06,0xe6,0xa0,0x00,0x03,
0x03,0xe6,0x40,0x00,0x03,0x01,0x24,0x00,0x00,0x03,0x07,0xf7,0xa0,0x00,0x03,
0x03,0xf5,0xc0,0x00,0x03,0x02,0x91,0x40,0x00,0x03,0x01,0xe4,0x20,0x00,0x03,
0x01,0x61,0x80,0x00,0x03,0x01,0x00,0x80,0x00,0x03,0x00,0x6e,0x40,0x00,0x03,
0x00,0xdf,0x00,0x00,0x03,0x00,0xe5,0x00,0x00,0x03,0x00,0x50,0x80,0x00,0x03,
0x00,0xf7,0x00,0x00,0x03,0x00,0xcd,0x00,0x00,0x03,0x00,0x14,0x80,0x00,0x03,
0x00,0xdf,0x00,0x00,0x03,0x00,0xe5,0x00,0x00,0x03,0x00,0x50,0x80,0x00,0x03,
0x00,0x5b,0x00,0x00,0x03,0x00,0x49,0x00,0x00,0x03,0x00,0xbc,0x80,0x00,0x03,
0x00,0x7e,0x00,0x00,0x03,0x00,0x62,0x00,0x00,0x03,0x00,0x01,0x00,0x00,0xfd,
0x00,0xfd,0x00,0xfd,0x00, },
{ /* 11 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x00,0xfe,0x00,0x00,0x03,0x00,0xfe,0x00,
0x00,0x03,0x00,0xfe,0x00,0x00,0x03,0x03,0xeb,0x80,0x00,0x03,0x03,0xeb,0x80,
0x00,0x03,0x03,0xeb,0x80,0x00,0x03,0x04,0xfe,0x40,0x00,0x03,0x04,0x82,0x40,
0x00,0x03,0x05,0x00,0x40,0x00,0x03,0x0b,0x9f,0xe0,0x00,0x03,0x0a,0xb3,0x60,
0x00,0x03,0x0c,0x00,0x20,0x00,0x03,0x1a,0xdc,0xe0,0x00,0x03,0x18,0x78,0xd0,
0x00,0x03,0x15,0x01,0x18,0x00,0x03,0x32,0xf4,0xd8,0x00,0x03,0x32,0x10,0xd8,
0x00,0x03,0x21,0x0b,0x44,0x00,0x03,0x62,0xf8,0x08,0x00,0x03,0x6e,0x80,0x64,
0x00,0x03,0x41,0x05,0x04,0x00,0x03,0xc1,0x00,0x30,0x00,0x03,0xdd,0x22,0xe0,
0x00,0x03,0xa0,0xf7,0x28,0x00,0x03,0x90,0x80,0x60,0x00,0x03,0x9e,0x85,0xc0,
0x00,0x03,0x90,0x56,0x50,0x00,0x03,0x3c,0x61,0x40,0x00,0x03,0xbf,0x63,0x00,
0x00,0x03,0xc4,0x19,0xa0,0x00,0x03,0x0f,0xa9,0x80,0x00,0x03,0x0b,0xfd,0x00,
0x00,0x03,0x31,0xa8,0x40,0x00,0x03,0x03,0xff,0x00,0x00,0x03,0x02,0x01,0x00,
0x00,0x03,0x0c,0x00,0x80,0x00,0x03,0x00,0x7c,0x00,0x00,0x03,0x00,0x7c,0x00,
0x00,0x03,0x03,0x82,0x00,0x00,0xfd,0x00,0xfd,0x00,0xfd,0x00, },
{ /* 12 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x07,0xff,0xc0,0x00,0x03,0x07,0xff,0xc0,
0x00,0x03,0x08,0x00,0x20,0x00,0x03,0x0a,0x00,0x20,0x00,0x03,0x0d,0xff,0x80,
0x00,0x00,0x02,0xfe,0x00,0x03,0x0e,0x00,0x20,0x00,0x03,0x0a,0x00,0x40,0x00,
0x03,0x05,0xff,0x80,0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0b,0xff,0x40,0x00,
0xff,0x00,0x01,0x80,0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0b,0xff,0x40,0x00,
0xff,0x00,0x01,0x80,0x00,0x03,0x0b,0xff,0xa0,0x00,0x03,0x0b,0x03,0xc0,0x00,
0xfd,0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0a,0x79,0x40,0x00,0x03,0x04,0x00,
0x80,0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0b,0x87,0x40,0x00,0xff,0x00,0x01,
0x80,0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0b,0x33,0x40,0x00,0x03,0x04,0x00,
0x80,0x00,0x03,0x0b,0xff,0xa0,0x00,0x03,0x0b,0x33,0xc0,0x00,0x00,0x04,0xfe,
0x00,0x03,0x0b,0xff,0x20,0x00,0x03,0x0b,0x03,0x40,0x00,0x03,0x04,0x00,0x80,
0x00,0x03,0x0b,0xff,0xa0,0x00,0x03,0x0b,0xff,0xc0,0x00,0x00,0x04,0xfe,0x00,
0x03,0x0f,0xff,0xa0,0x00,0x03,0x0f,0xff,0xc0,0x00,0xfd,0x00,0x03,0x0f,0xff,
0xc0,0x00,0x00,0x08,0xfe,0x00,0xfd,0x00, },
{ /* 13 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x03,0x60,0x18,0x0c,0x00,0x03,0x00,0x38,0x00,
0x00,0x03,0x60,0x18,0x0c,0x00,0x03,0x20,0x28,0x04,0x00,0x03,0x60,0x18,0x0c,
0x00,0xff,0x20,0x01,0x06,0x00,0x03,0xf0,0x38,0x14,0x00,0x03,0x42,0x08,0x84,
0x00,0x03,0xe0,0x20,0x0a,0x00,0x03,0x59,0x0d,0xec,0x00,0x03,0x30,0x1c,0x1c,
0x00,0x03,0x40,0x00,0x82,0x00,0x03,0x1c,0xb6,0x4c,0x00,0x03,0x2a,0x15,0x24,
0x00,0x03,0x11,0x38,0x82,0x00,0x03,0x26,0x4c,0xdc,0x00,0x03,0x35,0x2a,0x54,
0x00,0x03,0x28,0x91,0x40,0x00,0x03,0x2b,0x3d,0x98,0x00,0x03,0x12,0x3d,0x90,
0x00,0x03,0x28,0xa2,0xa4,0x00,0x03,0x15,0xb7,0x38,0x00,0x03,0x1f,0xd2,0x68,
0x00,0x03,0x10,0x35,0x04,0x00,0x03,0x12,0x6c,0xf8,0x00,0x03,0x0a,0xba,0x68,
0x00,0x03,0x12,0x64,0xc4,0x00,0x03,0x06,0x9c,0xb0,0x00,0x03,0x09,0x79,0x20,
0x00,0x03,0x06,0x98,0x88,0x00,0x03,0x09,0x89,0x70,0x00,0x03,0x0d,0x91,0x50,
0x00,0x03,0x08,0x02,0x88,0x00,0x03,0x0a,0xba,0xf0,0x00,0x03,0x05,0x4d,0x10,
0x00,0x03,0x08,0x10,0x40,0x00,0x03,0x03,0x9e,0xe0,0x00,0x03,0x06,0x1c,0x00,
0x00,0x03,0x03,0xe1,0x10,0x00,0x03,0x03,0xff,0xe0,0x00,0x03,0x03,0xff,0xe0,
0x00,0xff,0x00,0x01,0x10,0x00, },
{ /* 14 */
0xfd,0x00,0xfd,0x00,0xfd,0x00,0x00,0x60,0xfe,0x00,0x00,0x20,0xfe,0x00,0x00,
0x10,0xfe,0x00,0x00,0x98,0xfe,0x00,0x00,0xf8,0xfe,0x00,0x00,0x04,0xfe,0x00,
0x00,0x6f,0xfe,0x00,0x00,0x51,0xfe,0x00,0x03,0x20,0x80,0x00,0x00,0x03,0x3f,
0xc0,0x00,0x00,0x03,0x24,0x40,0x00,0x00,0x03,0x00,0x20,0x00,0x00,0x03,0x0f,
0xf8,0x00,0x00,0x03,0x01,0x18,0x00,0x00,0xfd,0x00,0x03,0x07,0x7e,0x00,0x00,
0x03,0x04,0x06,0x00,0x00,0x03,0x00,0x80,0x00,0x00,0x03,0x01,0xdf,0x80,0x00,
0x03,0x01,0x40,0x80,0x00,0x03,0x02,0x20,0x00,0x00,0x03,0x00,0x67,0x60,0x00,
0x03,0x00,0x05,0x20,0x00,0x03,0x00,0x98,0x90,0x00,0x03,0x00,0x3e,0xd8,0x00,
0x03,0x00,0x2b,0x40,0x00,0x03,0x00,0x04,0xe0,0x00,0x03,0x00,0x0c,0x4c,0x00,
0x03,0x00,0x09,0x90,0x00,0x03,0x00,0x12,0x68,0x00,0x03,0x00,0x06,0x04,0x00,
0x03,0x00,0x06,0xb8,0x00,0x03,0x00,0x01,0x42,0x00,0x03,0x00,0x01,0xc4,0x00,
0x03,0x00,0x01,0x70,0x00,0x03,0x00,0x02,0x0a,0x00,0xff,0x00,0x01,0x78,0x00,
0xff,0x00,0x01,0x60,0x00,0xff,0x00,0x01,0x86,0x00,0xfd,0x00,0xfd,0x00,0xff,
0x00,0x01,0x3e,0x00, },
};
///

/// PL_Dispatcher (Config Window Page List)
/*** PL_Dispatcher (Config Window Page List) - Subclass of List, adds small images to configuration menu ***/
DISPATCHERPROTO(PL_Dispatcher)
{
   static const ULONG PL_Colors[24] = {
      0x95959595,0x95959595,0x95959595, 0x00000000,0x00000000,0x00000000,
      0xffffffff,0xffffffff,0xffffffff, 0x3b3b3b3b,0x67676767,0xa2a2a2a2,
      0x7b7b7b7b,0x7b7b7b7b,0x7b7b7b7b, 0xafafafaf,0xafafafaf,0xafafafaf,
      0xaaaaaaaa,0x90909090,0x7c7c7c7c, 0xffffffff,0xa9a9a9a9,0x97979797 };
   struct PL_Data *data;
   int i;

   switch (msg->MethodID)
   {
      case OM_NEW:
         obj = DoSuperNew(cl, obj, TAG_MORE, ((struct opSet *)msg)->ops_AttrList);
         if (obj)
         {
            struct PL_Data *data = INST_DATA(cl,obj);
            InitHook(&data->DisplayHook, CO_PL_DspFuncHook, data);
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

/// ExitClasses()
/*** ExitClasses - Remove custom MUI classes ***/
void ExitClasses(void)
{
   if (CL_PageList   ) MUI_DeleteCustomClass(CL_PageList   );
   if (CL_MainWin    ) MUI_DeleteCustomClass(CL_MainWin    );
   if (CL_TextEditor ) MUI_DeleteCustomClass(CL_TextEditor );
   if (CL_BodyChunk  ) MUI_DeleteCustomClass(CL_BodyChunk  );
   if (CL_MailList   ) MUI_DeleteCustomClass(CL_MailList   );
   if (CL_FolderList ) MUI_DeleteCustomClass(CL_FolderList );
   if (CL_AddressList) MUI_DeleteCustomClass(CL_AddressList);
   if (CL_DDList     ) MUI_DeleteCustomClass(CL_DDList     );
   if (CL_AttachList ) MUI_DeleteCustomClass(CL_AttachList );
}

///
/// InitClasses()
/*** InitClasses - Initialize custom MUI classes ***/
BOOL InitClasses(void)
{
   CL_AttachList  = MUI_CreateCustomClass(NULL, MUIC_NList        , NULL, sizeof(struct DumData), ENTRY(WL_Dispatcher));
   CL_DDList      = MUI_CreateCustomClass(NULL, MUIC_List         , NULL, sizeof(struct DumData), ENTRY(EL_Dispatcher));
   CL_AddressList = MUI_CreateCustomClass(NULL, MUIC_NListtree    , NULL, sizeof(struct DumData), ENTRY(AL_Dispatcher));
   CL_FolderList  = MUI_CreateCustomClass(NULL, MUIC_NListtree    , NULL, sizeof(struct DumData), ENTRY(FL_Dispatcher));
   CL_MailList    = MUI_CreateCustomClass(NULL, MUIC_NList        , NULL, sizeof(struct DumData), ENTRY(ML_Dispatcher));
   CL_BodyChunk   = MUI_CreateCustomClass(NULL, MUIC_Bodychunk    , NULL, sizeof(struct BC_Data), ENTRY(BC_Dispatcher));
   CL_TextEditor  = MUI_CreateCustomClass(NULL, MUIC_TextEditor   , NULL, sizeof(struct DumData), ENTRY(TE_Dispatcher));
   CL_MainWin     = MUI_CreateCustomClass(NULL, MUIC_Window       , NULL, sizeof(struct DumData), ENTRY(MW_Dispatcher));
   CL_PageList    = MUI_CreateCustomClass(NULL, MUIC_List         , NULL, sizeof(struct PL_Data), ENTRY(PL_Dispatcher));

   return (BOOL)(CL_AttachList && CL_DDList && CL_AddressList && CL_FolderList && CL_MailList &&
                 CL_BodyChunk && CL_TextEditor && CL_MainWin && CL_PageList);
}
///

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

 Superclass:  MUIC_BetterString
 Description: Auto-completes email addresses etc.

***************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <mui/NList_mcc.h>
#include "Classes.h"
#include "YAM_addressbook.h"
#include "YAM_debug.h"
#include "YAM_folderconfig.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

/* ---------------------------------- */
#define DECLARE(method) ULONG m_Recipientstring_## method (struct IClass *cl, Object *obj, struct MUIP_Recipientstring_## method *msg)
#define OVERLOAD(method) ULONG m_Recipientstring_## method (struct IClass *cl, Object *obj, Msg msg)
#define ATTR(attr) case MUIA_Recipientstring_## attr
/* ---------------------------------- */

struct Data
{
   struct MUI_EventHandlerNode ehnode;

	Object *Matchwindow, *Matchlist;
};

ULONG RecipientstringGetSize (VOID) { return sizeof(struct Data); }

OVERLOAD(OM_NEW)
{
	return (ULONG)DoSuperNew(cl, obj,
		StringFrame,
		MUIA_CycleChain, 1,
		TAG_MORE, inittags(msg));
}

OVERLOAD(OM_DISPOSE)
{
	GETDATA;

	if(data->Matchwindow)
	{
		DoMethod(_app(obj), OM_REMMEMBER, data->Matchwindow);
		MUI_DisposeObject(data->Matchwindow);
	}

	return DoSuperMethodA(cl, obj, msg);
}

OVERLOAD(MUIM_Setup)
{
	GETDATA;

	if(DoSuperMethodA(cl, obj, msg))
	{
	   data->ehnode.ehn_Priority = 1;
	   data->ehnode.ehn_Flags    = 0;
	   data->ehnode.ehn_Object   = obj;
	   data->ehnode.ehn_Class    = cl;
	   data->ehnode.ehn_Events   = IDCMP_RAWKEY;
	   return TRUE;
	}
	return FALSE;
}

OVERLOAD(MUIM_GoActive)
{
	GETDATA;
   DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
   return DoSuperMethodA(cl, obj, msg);
}

OVERLOAD(MUIM_GoInactive)
{
	GETDATA;
   DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
   set(obj, MUIA_BetterString_SelectSize, 0);
	set(data->Matchwindow, MUIA_Window_Open, FALSE);
   return DoSuperMethodA(cl, obj, msg);
}

OVERLOAD(MUIM_HandleEvent)
{
	GETDATA;
	ULONG result = 0;
   struct MUIP_HandleEvent *hmsg = (struct MUIP_HandleEvent *)msg;
   if (hmsg->imsg && hmsg->imsg->Class == IDCMP_RAWKEY)
   {
      STRPTR contents, newcontents, completed = NULL, comma;
      LONG pos, allowmulti;
      Object *next = obj;

      // If RETURN was pressed
      if(hmsg->imsg->Code == 68)
      {
         // we expand the entered string with realname if SHIFT was not pressed also
         BOOL withrname = !((hmsg->imsg->Qualifier & IEQUALIFIER_RSHIFT) || (hmsg->imsg->Qualifier & IEQUALIFIER_LSHIFT));

      	ULONG open = FALSE, active;
      	if((get(data->Matchwindow, MUIA_Window_Open, &open), open) && (get(data->Matchlist, MUIA_List_Active, &active), active != MUIV_List_Active_Off))
      	{
      		DisplayBeep(NULL); // FIXME! We need to pick the address from the popup list
			}

         // If RETURN was pressed WITH SHIFT we expand the adress WITHOUT realname
         DoSuperMethodA(cl, obj, msg);
         get(obj, MUIA_String_Contents, &contents);
         get(obj, MUIA_UserData, &allowmulti);
         if (completed = WR_ExpandAddresses(-1, contents, FALSE, !allowmulti, withrname))
         {
            setstring(obj, completed);
            FreeStrBuf(completed);
            get(obj, MUIA_UserData, &next);
         }
         else DisplayBeep(0);

         if(next) set(_win(next), MUIA_Window_ActiveObject, next);

         result = MUI_EventHandlerRC_Eat;
      }
      else if(hmsg->imsg->Code == 95 && (hmsg->imsg->Qualifier & IEQUALIFIER_CONTROL))
      {
			set(data->Matchwindow, MUIA_Window_Open, FALSE);

         // If CTRL+HELP is pressed we expand the entered string with the addressbook
         DoSuperMethodA(cl, obj, msg);
         get(obj, MUIA_String_Contents, &contents);
         get(obj, MUIA_UserData, &allowmulti);
         if (completed = WR_ExpandAddresses(-1, contents, FALSE, !allowmulti, TRUE))
         {
            setstring(obj, completed);
            FreeStrBuf(completed);
         }
         result = MUI_EventHandlerRC_Eat;
      }
      else if(hmsg->imsg->Code == 65) // Backspace
      {
			set(data->Matchwindow, MUIA_Window_Open, FALSE);

	      // Clear all marked text if BACKSPACE (65) is pressed
			DoMethod(obj, MUIM_BetterString_ClearSelected, TAG_DONE);
		}
      else if(hmsg->imsg->Code == 76 || hmsg->imsg->Code == 77) /* Arrow up/down */
      {
      	ULONG open = FALSE, active;
      	if(get(data->Matchwindow, MUIA_Window_Open, &open), open)
      	{
      		if(get(data->Matchlist, MUIA_List_Active, &active), active != MUIV_List_Active_Off)
      				set(data->Matchlist, MUIA_List_Active, hmsg->imsg->Code == 76 ? MUIV_List_Active_Up     : MUIV_List_Active_Down);
      		else	set(data->Matchlist, MUIA_List_Active, hmsg->imsg->Code == 76 ? MUIV_List_Active_Bottom : MUIV_List_Active_Top);

	         result = MUI_EventHandlerRC_Eat;
      	}
		}
		else
      {
         // Convert the RAWKEY with MapRawKey() to a real keycode
         UBYTE code = ConvertKey(hmsg->imsg);
         ULONG select_size;
         get(obj, MUIA_BetterString_SelectSize, &select_size);

			if (code == ',' && select_size != 0)
         {
				set(data->Matchwindow, MUIA_Window_Open, FALSE);

            // if the comma was pressed then we jump to the end of the string and continue
            set(obj, MUIA_String_BufferPos, MUIV_BetterString_BufferPos_End);
            DoSuperMethodA(cl, obj, msg);
            result = MUI_EventHandlerRC_Eat;
         }
         else if ((((code >= 32 && code <= 126) || code >= 160) && !(hmsg->imsg->Qualifier & IEQUALIFIER_RCOMMAND)) || (code && hmsg->imsg->Qualifier & IEQUALIFIER_CONTROL))
         {
				set(data->Matchwindow, MUIA_Window_Open, FALSE);

            // if the pressed key is a REAL key and NOT pressed with right AMIGA
            // or pressed with control then...
            DoSuperMethodA(cl, obj, msg);
            get(obj, MUIA_String_Contents, &contents);
            get(obj, MUIA_String_BufferPos, &pos);

            // we only try to complete the Alias if the entered string is
            // at least 2 characters long
            if(strlen(contents) > 1)
            {
               // if there is a comma in the entered string we only try to
               // complete the right sight of the last comma
               if (comma = strrchr(contents,','))
               {
                  while (*++comma == ' '); // skip any space between the comma and the real string
                  if (strlen(comma) > 1) completed = AB_CompleteAlias(comma);
               }
               else
               {
               	DoMethod(obj, MUIM_Recipientstring_ShowMatches, contents);
               	completed = AB_CompleteAlias(contents);
              	}

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
	return result;
}

void FindAllMatches (STRPTR text, Object *list, struct MUI_NListtree_TreeNode *root)
{
	int tl = strlen(text);
	struct MUI_NListtree_TreeNode *tn;
	for(tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, root, MUIV_NListtree_GetEntry_Position_Head, 0);tn ;tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, 0))
	{
		struct ABEntry *entry = (struct ABEntry *)tn->tn_User;
		if(entry->Type == AET_GROUP)
		{
			FindAllMatches(text, list, tn);
		}
		else
		{
			UBYTE buf[512] = "";
			if(!Strnicmp(entry->Alias, text, tl))
				sprintf(buf, "%s (%s)", entry->Alias, entry->RealName);
			else if(!Strnicmp(entry->RealName, text, tl))
				sprintf(buf, "%s", entry->RealName);
			else if(!Strnicmp(entry->Address, text, tl))
				sprintf(buf, "%s (%s)", entry->Address, entry->RealName);

			if(buf[0] != '\0')
				DoMethod(list, MUIM_List_InsertSingle, buf, MUIV_List_Insert_Sorted);
		}
	}
}

DECLARE(ShowMatches) // STRPTR address
{
	GETDATA;
	LONG entries;

	if(!data->Matchwindow)
	{
		LONG top, left;
		get(_win(obj), MUIA_Window_TopEdge, &top);
		get(_win(obj), MUIA_Window_LeftEdge, &left);
		data->Matchwindow = WindowObject,
			MUIA_Window_Activate,			FALSE,
			MUIA_Window_Borderless,			TRUE,
			MUIA_Window_CloseGadget,		FALSE,
			MUIA_Window_DepthGadget,		FALSE,
			MUIA_Window_DragBar,				FALSE,
			MUIA_Window_SizeGadget,			FALSE,
			MUIA_Window_TopEdge,				top + _bottom(obj) + 1,
			MUIA_Window_LeftEdge,			left + _left(obj),
			MUIA_Window_Width,				_width(obj),
			WindowContents, GroupObject,
				InnerSpacing(0, 0),
				Child, ListviewObject,
					MUIA_Listview_List, data->Matchlist = ListObject,
						InputListFrame,
						MUIA_List_AdjustHeight, TRUE,
						MUIA_List_ConstructHook, MUIV_List_ConstructHook_String, 
						MUIA_List_DestructHook, MUIV_List_DestructHook_String,
					End,
				End,
			End,
		End;

		if(data->Matchwindow)
			DoMethod(_app(obj), OM_ADDMEMBER, data->Matchwindow);
	}

	DoMethod(data->Matchlist, MUIM_List_Clear);
	FindAllMatches(msg->address, data->Matchlist, MUIV_NListtree_GetEntry_ListNode_Root);

	if(get(data->Matchlist, MUIA_List_Entries, &entries), entries > 0)
		SetAttrs(data->Matchwindow, MUIA_Window_Open, TRUE, TAG_DONE);

	return 0;
}

OVERLOAD(MUIM_DragQuery)
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
	ULONG result = MUIV_DragQuery_Refuse;
   if (d->obj == G->MA->GUI.NL_MAILS) result = MUIV_DragQuery_Accept;
   else if (d->obj == G->AB->GUI.LV_ADDRESSES)
   {
	   struct MUI_NListtree_TreeNode *active;
      if(active = (struct MUI_NListtree_TreeNode *)GetMUI(d->obj, MUIA_NListtree_Active))
      {
         if (!(active->tn_Flags & TNF_LIST))
         {
            result = MUIV_DragQuery_Accept;
         }
      }
   }
	return result;
}

OVERLOAD(MUIM_DragDrop)
{
   struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;
   if (d->obj == G->MA->GUI.NL_MAILS)
   {
      struct Mail *mail;
      DoMethod(d->obj, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &mail);
      if (OUTGOING(mail->Folder->Type)) AB_InsertAddress(obj, "", mail->To.RealName, mail->To.Address);
      else AB_InsertAddress(obj, "", mail->From.RealName, mail->From.Address);
   }
   else if (d->obj == G->AB->GUI.LV_ADDRESSES)
   {
      struct MUI_NListtree_TreeNode *active = (struct MUI_NListtree_TreeNode *)GetMUI(d->obj, MUIA_NListtree_Active);
      struct ABEntry *addr = (struct ABEntry *)(active->tn_User);
      AB_InsertAddress(obj, addr->Alias, addr->RealName, "");
   }
   return 0;
}

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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site :	http://www.yam.ch
 YAM OpenSource project		:	http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_BetterString
 Description: Auto-completes email addresses etc.

***************************************************************************/

#include "Classes.h"

/* ---------------------------------- */
#define DECLARE(method) ULONG m_Recipientstring_## method (struct IClass *cl, Object *obj, struct MUIP_Recipientstring_## method *msg)
#define OVERLOAD(method) ULONG m_Recipientstring_## method (struct IClass *cl, Object *obj, Msg msg)
#define ATTR(attr) case MUIA_Recipientstring_## attr
/* ---------------------------------- */

/* some private stuff... someone sucks... */
#define MUIA_List_CursorType					  0x8042c53e /* V4  is. LONG */
#define MUIV_List_CursorType_Bar 1

struct CustomABEntry
{
	LONG MatchField;
	STRPTR MatchString;
	struct ABEntry *MatchEntry;
};

struct Data
{
	struct MUI_EventHandlerNode ehnode;
	Object *Matchwindow, *Matchlist;
	Object *From, *ReplyTo; /* used when resolving a list address */
	BOOL MultipleRecipients;
   BOOL NoResolve;
};

ULONG RecipientstringGetSize (VOID) { return sizeof(struct Data); }

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	if(obj = DoSuperMethodA(cl, obj, msg))
	{
		GETDATA;

		struct TagItem *tags = inittags(msg), *tag;
		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
   			ATTR(NoResolve)          : data->NoResolve = tag->ti_Data          ; break;
				ATTR(MultipleRecipients) : data->MultipleRecipients = tag->ti_Data ; break;
				ATTR(FromString)         : data->From = (Object *)tag->ti_Data     ; break;
				ATTR(ReplyToString)      : data->ReplyTo = (Object *)tag->ti_Data  ; break;
			}
		}

		SetAttrs(obj,
			MUIA_String_Popup, obj,
			MUIA_String_Reject, data->MultipleRecipients ? NULL : ",",
			TAG_DONE);
	}
	return (ULONG)obj;
}
///
/// OVERLOAD(OM_DISPOSE)
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
///
/// OVERLOAD(OM_GET)
/* this is just so that we can notify the popup tag */
OVERLOAD(OM_GET)
{
	ULONG *store = ((struct opGet *)msg)->opg_Storage;
	switch(((struct opGet *)msg)->opg_AttrID)
	{
		ATTR(Popup) : *store = FALSE ; return TRUE;
	}
	return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
	GETDATA;

	struct TagItem *tags = inittags(msg), *tag;
	while((tag = NextTagItem(&tags)))
	{
		switch(tag->ti_Tag)
		{
			ATTR(NoResolve)          : data->NoResolve = tag->ti_Data          ; break;
			ATTR(MultipleRecipients) : data->MultipleRecipients = tag->ti_Data ; break;
			ATTR(FromString)         : data->From = (Object *)tag->ti_Data     ; break;
			ATTR(ReplyToString)      : data->ReplyTo = (Object *)tag->ti_Data  ; break;
		}
	}

	return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
	GETDATA;

	if(DoSuperMethodA(cl, obj, msg))
	{
		data->ehnode.ehn_Priority = 1;
		data->ehnode.ehn_Flags	  = 0;
		data->ehnode.ehn_Object	  = obj;
		data->ehnode.ehn_Class	  = cl;
		data->ehnode.ehn_Events	  = IDCMP_RAWKEY;
		return TRUE;
	}
	return FALSE;
}
///
/// OVERLOAD(MUIM_GoActive)
OVERLOAD(MUIM_GoActive)
{
	GETDATA;
	DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->ehnode);
	return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_GoInactive)
OVERLOAD(MUIM_GoInactive)
{
	GETDATA;
	DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->ehnode);
	set(data->Matchwindow, MUIA_Window_Open, FALSE);
	return DoSuperMethodA(cl, obj, msg);
}
///
/// OVERLOAD(MUIM_DragQuery)
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
///
/// OVERLOAD(MUIM_DragDrop)
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
///
/// OVERLOAD(MUIM_Popstring_Open)
/* this method is invoked when the MUI popup key is pressed, we let it trigger a notify, so that the address book will open -- in the future this should be removed and we should just use a Popupstring object */
OVERLOAD(MUIM_Popstring_Open)
{
	set(obj, MUIA_Recipientstring_Popup, TRUE);
	return 0;
}
///
/// OVERLOAD(MUIM_HandleEvent)
enum { IECODE_RETURN = 68, IECODE_HELP = 95, IECODE_BACKSPACE = 65, IECODE_UP = 76, IECODE_DOWN = 77 };

OVERLOAD(MUIM_HandleEvent)
{
	GETDATA;
	ULONG result = 0;
	struct IntuiMessage *imsg = ((struct MUIP_HandleEvent *)msg)->imsg;

	if(imsg && imsg->Class == IDCMP_RAWKEY)
	{
		STRPTR new_address = NULL;
		switch(imsg->Code)
		{
         case IECODE_RETURN:
         {
            if(data->NoResolve) return(DoSuperMethodA(cl, obj, msg));

            if((imsg->Qualifier & IEQUALIFIER_RSHIFT) || (imsg->Qualifier & IEQUALIFIER_LSHIFT))
            {
               DoMethod(obj, MUIM_Recipientstring_Resolve, MUIF_Recipientstring_Resolve_NoRealName);
            }
            else
            {
               DoMethod(obj, MUIM_Recipientstring_Resolve, 0);
            }

            set(_win(obj), MUIA_Window_ActiveObject, MUIV_Window_ActiveObject_Next);

            result = MUI_EventHandlerRC_Eat;
         }
         break;

			/* keys are sent to the popup-list if it's open */
			case IECODE_UP:
			case IECODE_DOWN:
			{
				if(GetMUI(data->Matchwindow, MUIA_Window_Open))
				{
					struct CustomABEntry *entry;
					if(GetMUI(data->Matchlist, MUIA_List_Active) != MUIV_List_Active_Off)
							set(data->Matchlist, MUIA_List_Active, imsg->Code == IECODE_UP ? MUIV_List_Active_Up     : MUIV_List_Active_Down);
					else	set(data->Matchlist, MUIA_List_Active, imsg->Code == IECODE_UP ? MUIV_List_Active_Bottom : MUIV_List_Active_Top);

					DoMethod(data->Matchlist, MUIM_List_GetEntry, GetMUI(data->Matchlist, MUIA_List_Active), &entry);
					new_address = entry->MatchString;

					DoMethod(obj, MUIM_BetterString_ClearSelected);

					result = MUI_EventHandlerRC_Eat;
				}
			}
			break;

			case IECODE_BACKSPACE:
				DoMethod(obj, MUIM_BetterString_ClearSelected);
			/* continue */

			default:
			{
				STRPTR old, new;

				if(GetMUI(obj, MUIA_BetterString_SelectSize) != 0 && ConvertKey(imsg) == ',')
					set(obj, MUIA_String_BufferPos, MUIV_BetterString_BufferPos_End);

				get(obj, MUIA_String_Contents, &old);
				old = strdup(old);

				DoSuperMethodA(cl, obj, msg);
				get(obj, MUIA_String_Contents, &new);

				if(strcmp(old, new)) /* if contents changed, check if something matches */
					new_address = (STRPTR)DoMethod(obj, MUIM_Recipientstring_ShowMatches);

				free(old);

				result = MUI_EventHandlerRC_Eat;
			}
			break;
		}

		if(new_address) /* this is the complete address of what the user is typing, so let's insert it (marked) */
		{
			LONG pos, start = DoMethod(obj, MUIM_Recipientstring_RecipientStart);;
			get(obj, MUIA_String_BufferPos, &pos);

			DoMethod(obj, MUIM_BetterString_Insert, &new_address[pos - start], pos);
			SetAttrs(obj, MUIA_String_BufferPos, pos, MUIA_BetterString_SelectSize, strlen(new_address) - (pos - start), TAG_DONE);
		}
	}
	return result;
}
///

/* Hooks */
/// ConstructHook()
HOOKPROTONH(ConstructFunc, struct CustomABEntry *, APTR pool, struct CustomABEntry *e)
{
	struct CustomABEntry *res;
	if(res = malloc(sizeof(*e)))
		*res = *e;
	return res;
}
MakeStaticHook(ConstructHook, ConstructFunc);
///
/// DisplayHook()
HOOKPROTONH(DisplayFunc, LONG, STRPTR *array, struct CustomABEntry *e)
{
	static TEXT buf[SIZE_ADDRESS + 4];

	array[0] = e->MatchEntry->Alias;
	array[1] = e->MatchEntry->RealName;
	array[2] = e->MatchEntry->Address;

	sprintf(buf, "\033b%." STR(SIZE_ADDRESS) "s", e->MatchString);
	array[e->MatchField] = buf;

   return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);
///
/// CompareHook()
HOOKPROTONH(CompareFunc, LONG, struct CustomABEntry *e2, struct CustomABEntry *e1)
{
	if(e1->MatchField == e2->MatchField)
			return Stricmp(e1->MatchString, e2->MatchString);
	else	return e1->MatchField < e2->MatchField ? -1 : +1;
}
MakeStaticHook(CompareHook, CompareFunc);
///
/// FindAddressHook()
HOOKPROTONH(FindAddressFunc, LONG, Object *obj, struct MUIP_NListtree_FindUserDataMessage *msg)
{
	struct ABEntry *entry;

   if(!msg) return(NULL);
   entry = (struct ABEntry *)msg->UserData;

	return (!Stricmp(msg->User, entry->Alias) || !Stricmp(msg->User, entry->RealName) || !Stricmp(msg->User, entry->Address)) ? 0 : ~0;
}
MakeStaticHook(FindAddressHook, FindAddressFunc);
///

/* Private Functions */
/// FindAllMatches()
/* helper function, recursively go through the address book to find entries which match the string given */
VOID FindAllMatches (STRPTR text, Object *list, struct MUI_NListtree_TreeNode *root)
{
	int tl = strlen(text);
	struct MUI_NListtree_TreeNode *tn;
	for(tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, root, MUIV_NListtree_GetEntry_Position_Head, 0);tn ;tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, 0))
	{
		if(tn->tn_Flags & TNF_LIST) /* it's a sublist */
		{
			FindAllMatches(text, list, tn);
		}
		else
		{
			struct ABEntry *entry = (struct ABEntry *)tn->tn_User;
			struct CustomABEntry e = { -1 };

			if(!Strnicmp(entry->Alias, text, tl))				{ e.MatchField = 0; e.MatchString = entry->Alias; }
			else if(!Strnicmp(entry->RealName, text, tl))	{ e.MatchField = 1; e.MatchString = entry->RealName; }
			else if(!Strnicmp(entry->Address, text, tl))		{ e.MatchField = 2; e.MatchString = entry->Address; }

			if(e.MatchField != -1) /* one of the fields matches, so let's insert it in the MUI list */
			{
				e.MatchEntry = entry;
				DoMethod(list, MUIM_List_InsertSingle, &e, MUIV_List_Insert_Sorted);
			}
		}
	}
}
///

/* Public Methods */
/// DECLARE(AddRecipient)
/* add a recipient to this string taking care of comma (if in multi-mode). */
DECLARE(AddRecipient) // STRPTR address
{
	GETDATA;
	STRPTR contents;

	if(!data->MultipleRecipients)
		set(obj, MUIA_String_Contents, NULL);

	if(get(obj, MUIA_String_Contents, &contents), contents[0] != '\0')
		DoMethod(obj, MUIM_BetterString_Insert, ", ", MUIV_BetterString_Insert_EndOfString);

	DoMethod(obj, MUIM_BetterString_Insert, msg->address, MUIV_BetterString_Insert_EndOfString);
	set(obj, MUIA_String_BufferPos, -1);

	return 0;
}
///
/// DECLARE(Resolve)
/* resolve all addresses */
DECLARE(Resolve) // ULONG flags
{
	GETDATA;
	BOOL list_expansion;
	LONG max_list_nesting = 5;
	STRPTR s, contents, tmp;
   BOOL res = TRUE;
   BOOL wrname = TRUE, checkvalids = TRUE;
   int int_flag = 0;

   // Lets check the flags first
   if(msg->flags & MUIF_Recipientstring_Resolve_NoRealName) wrname      = FALSE;
   if(msg->flags & MUIF_Recipientstring_Resolve_NoValids)   checkvalids = FALSE;

	set(G->AB->GUI.LV_ADDRESSES, MUIA_NListtree_FindUserDataHook, &FindAddressHook);

	do {

		struct MUI_NListtree_TreeNode *tn;
		list_expansion = FALSE;
		get(obj, MUIA_String_Contents, &s);
		if(!(contents = tmp = strdup(s)))
			break;
		set(obj, MUIA_String_Contents, NULL);

		DB(kprintf("Resolve this string: %s\n", tmp);)
		while(s = Trim(strtok(tmp, ","))) /* tokenize string and resolve each recipient */
		{
		   DB(kprintf("token: '%s'\n", s);)

         if(checkvalids == FALSE && (tmp = strchr(s, '@')))
         {
		   	DB(kprintf("Valid address found.. will not resolve it: %s\n", s);)
            int_flag = 1;
         }
         else if(tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_FindUserData, MUIV_NListtree_FindUserData_ListNode_Root, s, 0)) /* entry found in address book */
			{
				struct ABEntry *entry = (struct ABEntry *)tn->tn_User;
				DB(kprintf("Found match: %s\n", s);)
				if(entry->Type == AET_USER) /* it's a normal person */
				{
					DB(kprintf("\tPlain user: %s (%s, %s)\n", AB_PrettyPrintAddress(entry), entry->RealName, entry->Address);)
					DoMethod(obj, MUIM_Recipientstring_AddRecipient, wrname ? AB_PrettyPrintAddress(entry) : (STRPTR)entry->Address);
				}
				else if(entry->Type == AET_LIST) /* it's a list of persons */
				{
					if(data->MultipleRecipients)
					{
						STRPTR members, lf;
						if(members = strdup(entry->Members))
						{
							while(lf = strchr(members, '\n'))
							   lf[0] = ',';

							DB(kprintf("Found list: »%s«\n", entry->Members);)
							DoMethod(obj, MUIM_Recipientstring_AddRecipient, members);
							free(members);

							if(entry->RealName[0])	set(data->From, MUIA_String_Contents, AB_PrettyPrintAddress2(entry->RealName, C->EmailAddress));
							if(entry->Address[0])	set(data->ReplyTo, MUIA_String_Contents, entry->Address);

							list_expansion = TRUE;
						}
					}
					else
					{
						DB(kprintf("String doesn't allow multiple recipients\n");)
						DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
						DisplayBeep(NULL);
                  res = FALSE;
					}
				}
				else /* it's unknown... */
				{
					DB(kprintf("Unknown type: %ld\n", entry->Type);)
					DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
               set(_win(obj), MUIA_Window_ActiveObject, obj);
					DisplayBeep(NULL);
               res = FALSE;
				}
			}
			else int_flag = 2;

         // Lets check the flags now
			if(int_flag == 1 || (int_flag == 2 && (tmp = strchr(s, '@'))))
         {
		      DB(kprintf("Email address: %s\n", s);)
			   DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);

            /* email address lacks domain... */
   			if(tmp[1] == '\0')
            {
	   		   DoMethod(obj, MUIM_BetterString_Insert, strchr(C->EmailAddress, '@')+1, MUIV_BetterString_Insert_EndOfString);
		      }
			}
         else if(int_flag == 2)
         {
            DoMethod(obj, MUIM_Recipientstring_AddRecipient, s);
            set(_win(obj), MUIA_Window_ActiveObject, obj);
            DisplayBeep(NULL);
            res = FALSE;
         }

         // reset the values
         int_flag = 0;
			tmp = NULL;
		}
		free(contents);

	} while(list_expansion && max_list_nesting-- > 0);

   contents = (STRPTR)GetMUI(obj, MUIA_String_Contents);

	return((ULONG)(res ? contents : NULL));
}
///
/// DECALRE(RecipientStart)
/* return the index where current recipient start (from cursor pos), this is only useful for objects with more than one recipient */
DECLARE(RecipientStart)
{
	STRPTR buf;
	ULONG pos, start;
	get(obj, MUIA_String_Contents, &buf);
	get(obj, MUIA_String_BufferPos, &pos);

	start = pos;
	while(start > 0 && buf[start-1] != ',')	start--;
	while(ISpace(buf[start]))						start++;

	return start;
}
///
/// DECLARE(ShowMatches)
/* this compares the string for any matches in the address book. If some are found then a list popup and the first entry of this list is returned */
DECLARE(ShowMatches)
{
	GETDATA;
	LONG entries, state, pos;
	STRPTR res = NULL, buf, start = NULL, end;
	struct CustomABEntry *entry;
   LONG top, left;

	if(!data->Matchwindow)
	{
		data->Matchwindow = WindowObject,
			MUIA_Window_Activate,			FALSE,
			MUIA_Window_Borderless,			TRUE,
			MUIA_Window_CloseGadget,		FALSE,
			MUIA_Window_DepthGadget,		FALSE,
			MUIA_Window_DragBar,				FALSE,
			MUIA_Window_SizeGadget,			FALSE,
			WindowContents, GroupObject,
				InnerSpacing(0, 0),
				Child, data->Matchlist = ListObject,
					InputListFrame,
					MUIA_List_CompareHook,		&CompareHook,
					MUIA_List_ConstructHook,	&ConstructHook,
					MUIA_List_CursorType,		MUIV_List_CursorType_Bar,
					MUIA_List_DestructHook,		&GeneralDesHook,
					MUIA_List_DisplayHook,		&DisplayHook,
					MUIA_List_Format,				",,",
				End,
			End,
		End;

		if(data->Matchwindow) DoMethod(_app(obj), OM_ADDMEMBER, data->Matchwindow);
      else                  return(NULL);
	}

   // get the window position everytime because the window could be moved in the meantime
   get(_win(obj), MUIA_Window_TopEdge, &top);
	get(_win(obj), MUIA_Window_LeftEdge, &left);

   // now set the dimesions every time because they could have been changed.
   set(data->Matchwindow, MUIA_Window_TopEdge, top + _bottom(obj) + 1);
   set(data->Matchwindow, MUIA_Window_LeftEdge, left + _left(obj));
   set(data->Matchwindow, MUIA_Window_Width, _width(obj));

	/* we need to isolate the recipient currently being entered when the string contain several */
	set(data->Matchlist, MUIA_List_Quiet, TRUE);
	DoMethod(data->Matchlist, MUIM_List_Clear);
	get(obj, MUIA_String_Contents, &buf);
	get(obj, MUIA_String_BufferPos, &pos);
	if((buf[pos] == '\0' || buf[pos] == ',') && (start = strdup(&buf[DoMethod(obj, MUIM_Recipientstring_RecipientStart)])))
	{
		if(end = strchr(start, ','))
			end[0] = '\0';
		if(start[0] != '\0')
			FindAllMatches(start, data->Matchlist, MUIV_NListtree_GetEntry_ListNode_Root);
	}
	set(data->Matchlist, MUIA_List_Quiet, FALSE);

   /* is there more entries in the list and if only one, is it longer than what the user already typed... */
	if((get(data->Matchlist, MUIA_List_Entries, &entries), entries > 0) && (DoMethod(data->Matchlist, MUIM_List_GetEntry, 0, &entry), (entries != 1 || Stricmp(start, entry->MatchString))))
   	res = entry->MatchString;

   /* should we open the popup list (if not already shown) */
   if(!res || (get(data->Matchwindow, MUIA_Window_Open, &state), !state))
   	SetAttrs(data->Matchwindow, MUIA_Window_Open, res ? TRUE : FALSE, TAG_DONE);

   if(start)
   	free(start);

	return (ULONG)res;
}
///

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Window
 Description: Popup a list of addresses which match a given substring

***************************************************************************/

#include "Addrmatchlist_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
	struct MUI_EventHandlerNode ehnode;
	Object *Matchlist, *String;
	BOOL Open;
};
*/

/* EXPORT
struct CustomABEntry
{
	LONG MatchField;
	STRPTR MatchString;
	struct ABEntry *MatchEntry;
};
*/

/* Hooks */
/// ConstructHook
HOOKPROTONHNO(ConstructFunc, struct CustomABEntry *, struct CustomABEntry *e)
{
	struct CustomABEntry *res;
	if((res = malloc(sizeof(struct CustomABEntry))))
		*res = *e;
	return res;
}
MakeStaticHook(ConstructHook, ConstructFunc);

///
/// DisplayHook
HOOKPROTONH(DisplayFunc, LONG, STRPTR *array, struct CustomABEntry *e)
{
	static char buf[SIZE_ADDRESS + 4];

	array[0] = e->MatchEntry->Alias[0]    ? e->MatchEntry->Alias    : "-";
	array[1] = e->MatchEntry->RealName[0] ? e->MatchEntry->RealName : "-";
	array[2] = e->MatchEntry->Address[0]  ? e->MatchEntry->Address  : "-";

	sprintf(buf, "\033b%." STR(SIZE_ADDRESS) "s", e->MatchString);
	array[e->MatchField] = buf;

	return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

///
/// CompareHook
HOOKPROTONH(CompareFunc, LONG, struct CustomABEntry *e2, struct CustomABEntry *e1)
{
	if(e1->MatchField == e2->MatchField)
		return Stricmp(e1->MatchString, e2->MatchString);
	else
		return e1->MatchField < e2->MatchField ? -1 : +1;
}
MakeStaticHook(CompareHook, CompareFunc);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	Object *listview, *list;

	if((obj = DoSuperNew(cl, obj,
		MUIA_Window_Activate,         FALSE,
		MUIA_Window_Borderless,       TRUE,
		MUIA_Window_CloseGadget,      FALSE,
		MUIA_Window_DepthGadget,      FALSE,
		MUIA_Window_DragBar,          FALSE,
		MUIA_Window_SizeGadget,       FALSE,
		MUIA_Window_IsSubWindow,			TRUE,
		WindowContents, GroupObject,
			InnerSpacing(0, 0),
			Child, listview = ListviewObject,
				MUIA_Listview_ScrollerPos,	 MUIV_Listview_ScrollerPos_None,
				MUIA_Listview_List,	list = ListObject,
					InputListFrame,
					MUIA_List_CompareHook,     &CompareHook,
					MUIA_List_ConstructHook,   &ConstructHook,
					MUIA_List_CursorType,      MUIV_List_CursorType_Bar,
					MUIA_List_DestructHook,    &GeneralDesHook,
					MUIA_List_DisplayHook,     &DisplayHook,
					MUIA_List_Format,          ",,",
				End,
			End,
		End,
		TAG_MORE, inittags(msg))))
	{
		GETDATA;

		struct TagItem *tags = inittags(msg), *tag;
		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
				ATTR(String) : data->String = (Object *)tag->ti_Data ; break;
			}
		}

		data->Matchlist = list;

		if(!data->String)
		{
			E(DBF_GUI, "No MUIA_Addrmatchlist_String supplied");
			CoerceMethod(cl, obj, OM_DISPOSE);
			obj = NULL;
		}
		else
		{
			// we need to catch the RAWKEY events and forward them directly
			// to our string object
			data->ehnode.ehn_Priority = 1;
			data->ehnode.ehn_Flags	  = 0;
			data->ehnode.ehn_Object	  = data->String;
			data->ehnode.ehn_Class	  = 0;
			data->ehnode.ehn_Events	  = IDCMP_RAWKEY;

			DoMethod(obj, MUIM_Window_AddEventHandler, &data->ehnode);

			// set the doubleclick notify to signal the string to resolve a entry
			DoMethod(listview, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, data->String, 2, MUIM_Recipientstring_Resolve, MUIF_NONE);
			DoMethod(listview, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, obj, 3, MUIM_Set, MUIA_Window_Open, FALSE);
			DoMethod(list, MUIM_Notify, MUIA_List_Active, MUIV_EveryTime, obj, 2, MUIM_Addrmatchlist_ActiveChange, MUIV_TriggerValue);
		}
	}

	return (ULONG)obj;
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
			case MUIA_Window_Open:
			{
				if(data->Open != (BOOL)tag->ti_Data && _win(data->String))
					set(_win(data->String), MUIA_Window_DisableKeys, (data->Open = tag->ti_Data) ? 1 << MUIKEY_WINDOW_CLOSE : 0);
			}
			break;
		}
	}
	return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Class Methods */
/// DECLARE(ChangeWindow)
DECLARE(ChangeWindow)
{
	GETDATA;
	struct Window *match_win = (struct Window *)xget(obj, MUIA_Window_Window);
	struct Window *write_win = (struct Window *)xget(_win(data->String), MUIA_Window_Window);
	ULONG left = write_win->LeftEdge + _left(data->String);
	ULONG top = write_win->TopEdge + _bottom(data->String) + 1;

	// only when the window is close a set() is valid to change
	// the position of a MUI window. Otherwise we have to use ChangeWindowBox()
	if(match_win && xget(obj, MUIA_Window_Open))
	{
		// change the window position/sizes
		ChangeWindowBox(match_win, left, top, _width(data->String), match_win->Height);

		// also make sure the window is always in front of the write window
		MoveWindowInFrontOf(match_win, write_win);
	}
	else
	{
		// if there is currently no window open we can use set()
		// to change the position/sizes
		SetAttrs(obj,
			MUIA_Window_LeftEdge,   left,
			MUIA_Window_TopEdge,    top,
			MUIA_Window_Width,      _width(data->String),
			TAG_DONE);
	}

	return 0;
}

///
/// DECLARE(Event)
DECLARE(Event) // struct IntuiMessage *imsg
{
	GETDATA;

	if(xget(obj, MUIA_Window_Open))
	{
		struct IntuiMessage *imsg = msg->imsg;
		LONG direction = (imsg->Code == IECODE_UP || imsg->Code == NM_WHEEL_UP || imsg->Code == NM_WHEEL_LEFT) ? MUIV_List_Active_Up : MUIV_List_Active_Down;
		LONG position = xget(data->Matchlist, MUIA_List_Active);

		// to enable a circular selection model we have to make some checks.
		if(direction == MUIV_List_Active_Up)
		{
			if(position == 0) direction = MUIV_List_Active_Bottom;
		}
		else if(position == (LONG)xget(data->Matchlist, MUIA_List_Entries)-1)
		{
			direction = MUIV_List_Active_Top;
		}

		set(data->Matchlist, MUIA_List_Active, direction);

		return TRUE;
	}

	return FALSE;
}

///
/// DECLARE(Open)
DECLARE(Open) // STRPTR str
{
	GETDATA;
	STRPTR res = NULL;
	LONG entries;
	struct CustomABEntry *entry;

	D(DBF_GUI, "Match this: '%s'", msg->str);

	set(data->Matchlist, MUIA_List_Quiet, TRUE);

	DoMethod(data->Matchlist, MUIM_List_Clear);

	DoMethod(_app(obj), MUIM_YAM_FindEmailMatches, msg->str, data->Matchlist);

	/* is there more entries in the list and if only one, is it longer than what the user already typed... */
	entries = xget(data->Matchlist, MUIA_List_Entries);
	if(entries > 0 && (DoMethod(data->Matchlist, MUIM_List_GetEntry, 0, &entry), (entries != 1 || Stricmp(msg->str, entry->MatchString))))
	{
		res = entry->MatchString;
		nnset(data->Matchlist, MUIA_List_Active, MUIV_List_Active_Top);
	}

	// should we open the popup list (if not already shown)
	if(!xget(obj, MUIA_Window_Open))
	{
		if(res)
		{
			DoMethod(obj, MUIM_Addrmatchlist_ChangeWindow); // refresh the position
			set(obj, MUIA_Window_Open, TRUE);
		}
	}
	else if(!res) set(obj, MUIA_Window_Open, FALSE);

	set(data->Matchlist, MUIA_List_Quiet, FALSE);

	return (ULONG)res;
}

///
/// DECLARE(ActiveChange)
DECLARE(ActiveChange) // LONG active
{
	GETDATA;
	struct CustomABEntry *entry;
	STRPTR res;

	if(msg->active < 0) return 0;

	// get the active entry
	DoMethod(data->Matchlist, MUIM_List_GetEntry, msg->active, &entry);

	res = entry->MatchString;

	// Now we check if the match is because of the real name and the same name exists twice in
	// this list we have to return the email as matchstring
	if(entry->MatchField == 1)  // RealName
	{
		int i;
		LONG elen = (LONG)strlen(entry->MatchString);

		for(i=0;;i++)
		{
			struct CustomABEntry *compareEntry;

			DoMethod(data->Matchlist, MUIM_List_GetEntry, i, &compareEntry);
			if(!compareEntry) break;

			if(compareEntry != entry)
			{
				if(Strnicmp(compareEntry->MatchEntry->RealName, entry->MatchString, elen) == 0)
				{
					res = entry->MatchEntry->Address;
					break;
				}
			}
		}
	}

	// signal the string that we need to replace the selected part with
	// some new entry
	if(res) DoMethod(data->String, MUIM_Recipientstring_ReplaceSelected, res);

	return 0;
}

///

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

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Window
 Description: Popup a list of addresses which match a given substring

***************************************************************************/

#include "Addrmatchlist_cl.h"

/* CLASSDATA
struct Data
{
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

HOOKPROTONH(ConstructFunc, struct CustomABEntry *, APTR pool, struct CustomABEntry *e)
{
	struct CustomABEntry *res;
	if(res = malloc(sizeof(struct CustomABEntry)))
		*res = *e;
	return res;
}
MakeStaticHook(ConstructHook, ConstructFunc);

HOOKPROTONH(DisplayFunc, LONG, STRPTR *array, struct CustomABEntry *e)
{
	static TEXT buf[SIZE_ADDRESS + 4];

	array[0] = e->MatchEntry->Alias[0]    ? e->MatchEntry->Alias    : "-";
	array[1] = e->MatchEntry->RealName[0] ? e->MatchEntry->RealName : "-";
	array[2] = e->MatchEntry->Address[0]  ? e->MatchEntry->Address  : "-";

	sprintf(buf, "\033b%." STR(SIZE_ADDRESS) "s", e->MatchString);
	array[e->MatchField] = buf;

	return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);

HOOKPROTONH(CompareFunc, LONG, struct CustomABEntry *e2, struct CustomABEntry *e1)
{
	if(e1->MatchField == e2->MatchField)
			return Stricmp(e1->MatchString, e2->MatchString);
	else	return e1->MatchField < e2->MatchField ? -1 : +1;
}
MakeStaticHook(CompareHook, CompareFunc);

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	Object *list;
	if(obj = DoSuperNew(cl, obj,
		MUIA_Window_Activate,         FALSE,
		MUIA_Window_Borderless,       TRUE,
		MUIA_Window_CloseGadget,      FALSE,
		MUIA_Window_DepthGadget,      FALSE,
		MUIA_Window_DragBar,          FALSE,
		MUIA_Window_SizeGadget,       FALSE,
		WindowContents, GroupObject,
			InnerSpacing(0, 0),
			Child, list = ListObject,
				InputListFrame,
				MUIA_List_CompareHook,     &CompareHook,
				MUIA_List_ConstructHook,   &ConstructHook,
				MUIA_List_CursorType,      MUIV_List_CursorType_Bar,
				MUIA_List_DestructHook,    &GeneralDesHook,
				MUIA_List_DisplayHook,     &DisplayHook,
				MUIA_List_Format,          ",,",
			End,
		End,
		TAG_MORE, inittags(msg)))
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
			DB(kprintf("No MUIA_Addrmatchlist_String supplied\n");)
			CoerceMethod(cl, obj, OM_DISPOSE);
			obj = NULL;
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
				if(data->Open != tag->ti_Data && _win(data->String))
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
	ULONG left = xget(_win(data->String), MUIA_Window_LeftEdge) + _left(data->String);
	ULONG top = xget(_win(data->String), MUIA_Window_TopEdge) + _bottom(data->String) + 1;

	if(xget(obj, MUIA_Window_Open))
	{
		struct Window *win = (struct Window *)xget(obj, MUIA_Window_Window);
		ChangeWindowBox(win, left, top, (LONG)_width(data->String), (LONG)win->Height);
	}
	else
	{
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
	STRPTR res = NULL;

	if(xget(obj, MUIA_Window_Open))
	{
		struct CustomABEntry *entry;

		if(xget(data->Matchlist, MUIA_List_Active) != MUIV_List_Active_Off)
		{
				set(data->Matchlist, MUIA_List_Active, msg->imsg->Code == IECODE_UP ? MUIV_List_Active_Up     : MUIV_List_Active_Down);
		}
		else
		{
			set(data->Matchlist, MUIA_List_Active, msg->imsg->Code == IECODE_UP ? MUIV_List_Active_Bottom : MUIV_List_Active_Top);
		}

		DoMethod(data->Matchlist, MUIM_List_GetEntry, xget(data->Matchlist, MUIA_List_Active), &entry);

		res = entry->MatchString;

		// Now we check if the match is because of the real name and the same name exists twice in
		// this list we have to return the email as matchstring
		if(entry->MatchField == 1)  // RealName
		{
			int i;
			for(i=0;;i++)
			{
				struct CustomABEntry *compareEntry;

				DoMethod(data->Matchlist, MUIM_List_GetEntry, i, &compareEntry);
				if(!compareEntry) break;

				if(compareEntry != entry)
				{
					if(Stricmp(compareEntry->MatchString, entry->MatchString) == 0)
					{
						res = entry->MatchEntry->Address;
						break;
					}
				}
			}
		}
	}
	return (ULONG)res;
}

///
/// DECLARE(Open)
DECLARE(Open) // STRPTR str
{
	GETDATA;
	STRPTR res = NULL;
	LONG entries;
	struct CustomABEntry *entry;

	DB(kprintf("Match this: %s\n", msg->str);)

	set(data->Matchlist, MUIA_List_Quiet, TRUE);

	DoMethod(data->Matchlist, MUIM_List_Clear);

	DoMethod(_app(obj), MUIM_YAM_FindEmailMatches, msg->str, data->Matchlist);

	set(data->Matchlist, MUIA_List_Quiet, FALSE);

	/* is there more entries in the list and if only one, is it longer than what the user already typed... */
	entries = xget(data->Matchlist, MUIA_List_Entries);
	if(entries > 0 && (DoMethod(data->Matchlist, MUIM_List_GetEntry, 0, &entry), (entries != 1 || Stricmp(msg->str, entry->MatchString))))
		res = entry->MatchString;

	/* should we open the popup list (if not already shown) */
	if(!res || !xget(obj, MUIA_Window_Open))
		set(obj, MUIA_Window_Open, res ? TRUE : FALSE);

	return (ULONG)res;
}

///

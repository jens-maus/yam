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

 Superclass:  MUIC_Application
 Description: Application subclass handles all "global" stuff.

***************************************************************************/

#include "classes/YAM.h"

#define EMAILCACHENAME "PROGDIR:.emailcache"

/* CLASSDATA
struct Data
{
	struct List EMailCache;
	STRPTR EMailCacheName;
};
*/

struct EMailCacheNode
{
	struct Node ecn_Node;
	struct Person ecn_Person;
};

/// LoadEMailCache()
VOID LoadEMailCache (STRPTR name, struct List *list)
{
	BPTR fh;
	NewList(list);
	if(fh = Open(name, MODE_OLDFILE))
	{
		TEXT line[SIZE_REALNAME + SIZE_ADDRESS + 5]; /* should hold "name <addr>\n\0" */
		while(FGets(fh, line, sizeof(line)))
		{
			STRPTR addr, end;
			struct EMailCacheNode *node;
			if((addr = strchr(line, '<')) && (end = strchr(addr, '>')) && (node = calloc(1, sizeof(*node))))
			{
				if(addr != line)
				{
					addr[-1] = '\0';
					strncpy(node->ecn_Person.RealName, line, SIZE_REALNAME-1);
				}
				end[0] = '\0';
				strncpy(node->ecn_Person.Address, addr+1, SIZE_ADDRESS-1);

				AddTail(list, &node->ecn_Node);
			}
			else
			{
				D(DBF_ERROR, ("Error with '%s', parsing line:\n%s", name, line))
			}
		}
		Close(fh);
	}
	else
	{
		D(DBF_ERROR, ("Error opening file '%s' for reading", name))
	}
}

///

/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
   static STRPTR Classes[] = { "TextEditor.mcc", "Toolbar.mcc", "BetterString.mcc", "InfoText.mcc", "NListtree.mcc", "NList.mcc", "NListview.mcc", NULL };
	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Application_Author,         "YAM Open Source Team",
		MUIA_Application_Base,           "YAM",
		MUIA_Application_Title,          "YAM",
		MUIA_Application_Version,        yamversionstring,
		MUIA_Application_Copyright,      "© 2000-2001 by YAM Open Source Team",
		MUIA_Application_Description,    GetStr(MSG_AppDescription),
		MUIA_Application_UseRexx,        FALSE,
		MUIA_Application_SingleTask,     !getenv("MultipleYAM"),
		MUIA_Application_UsedClasses,    Classes,
		TAG_MORE,                        inittags(msg)))
	{
		GETDATA;
		struct TagItem *tags = inittags(msg), *tag;

		data->EMailCacheName = EMAILCACHENAME;

		while((tag = NextTagItem(&tags)))
		{
			switch(tag->ti_Tag)
			{
				ATTR(EMailCacheName) : data->EMailCacheName = (STRPTR)tag->ti_Data ; break;
			}
		}

		LoadEMailCache(data->EMailCacheName, &data->EMailCache);
	}
	return (ULONG)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
//	GETDATA;
	return DoSuperMethodA(cl, obj, msg);
}
///

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

#include "YAM_cl.h"

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
	struct ABEntry ecn_Person;
};

/* Private functions */
/// LoadEMailCache()
VOID LoadEMailCache(STRPTR name, struct List *list)
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
			if((addr = strchr(line, '<')) && (end = strchr(addr, '>')) && (node = calloc(1, sizeof(struct EMailCacheNode))))
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
/// SaveEMailCache()
VOID SaveEMailCache(STRPTR name, struct List *list)
{
	BPTR fh;

	if(fh = Open(name, MODE_NEWFILE))
	{
		int i;
		struct EMailCacheNode *node = (struct EMailCacheNode *)(list->lh_Head);
		TEXT line[SIZE_REALNAME + SIZE_ADDRESS + 5]; /* should hold "name <addr>\n\0" */

		for(i=0; i < C->EmailCache && ((struct Node *)node)->ln_Succ != NULL; i++, node = (struct EMailCacheNode *)((struct Node *)node)->ln_Succ)
		{
			struct ABEntry *entry = &node->ecn_Person;

			if(entry->RealName[0])
			{
				sprintf(line, "%s <%s>\n", entry->RealName, entry->Address);
			}
			else
			{
				sprintf(line, "<%s>\n", entry->Address);
			}

			FPuts(fh, line);
		}

		Close(fh);
	}
	else
	{
		D(DBF_ERROR, ("Error opening file '%s' for writing", name))
	}
}

///
/// FindAllABMatches()
// tries to find all matching addressbook entries and add them to the list
VOID FindAllABMatches (STRPTR text, Object *list, struct MUI_NListtree_TreeNode *root)
{
	LONG tl = strlen(text);
	struct MUI_NListtree_TreeNode *tn;

  // Now we try to find matches in the Addressbook Listtree
  tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, root, MUIV_NListtree_GetEntry_Position_Head, MUIF_NONE);

  for(;tn;)
	{
		if(isFlagSet(tn->tn_Flags, TNF_LIST)) /* it's a sublist */
		{
			FindAllABMatches(text, list, tn);
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

    tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);
	}
}

///
/// FindABPerson()
// tries to find a Person in a addressbook
BOOL FindABPerson(struct Person *person, struct MUI_NListtree_TreeNode *root)
{
	struct MUI_NListtree_TreeNode *tn;

	// Now we try to find matches in the Addressbook Listtree
	tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, root, MUIV_NListtree_GetEntry_Position_Head, MUIF_NONE);

	for(;tn;)
	{
		if(isFlagSet(tn->tn_Flags, TNF_LIST)) /* it's a sublist */
		{
			if(FindABPerson(person, tn)) return TRUE;
		}
		else
		{
			struct ABEntry *entry = (struct ABEntry *)tn->tn_User;

			// If the email matches a entry in the AB we already can return here with TRUE
			if(Stricmp(entry->Address, person->Address) == 0)
			{
				return TRUE;
			}
		}

		tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->AB->GUI.LV_ADDRESSES, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);
	}

  return FALSE;
}

///

/* Public Methods */
/// DECLARE(FindEmailMatches)
DECLARE(FindEmailMatches) // STRPTR matchText, Object *list
{
	GETDATA;

	if(msg->matchText && msg->matchText[0] != '\0')
	{
		// We first try to find matches in the Addressbook
		// and add them to the MUI list
		FindAllABMatches(msg->matchText, msg->list, MUIV_NListtree_GetEntry_ListNode_Root);

		// If the user has selected the EmailCache feature we also have to check this
		// list and add matches to the MUI List too
		if(C->EmailCache > 0 && !IsListEmpty(&data->EMailCache))
		{
			int i;
			LONG tl = strlen(msg->matchText);
			struct EMailCacheNode *node = (struct EMailCacheNode *)(data->EMailCache.lh_Head);

			for(i=0; i < C->EmailCache && ((struct Node *)node)->ln_Succ != NULL; i++, node = (struct EMailCacheNode *)((struct Node *)node)->ln_Succ)
			{
				struct ABEntry *entry = &node->ecn_Person;
				struct CustomABEntry e = { -1 };

				if(!Strnicmp(entry->RealName, msg->matchText, tl))      { e.MatchField = 1; e.MatchString = entry->RealName;  }
				else if(!Strnicmp(entry->Address, msg->matchText, tl))  { e.MatchField = 2; e.MatchString = entry->Address;   }

				if(e.MatchField != -1) /* one of the fields matches, so let's insert it in the MUI list */
				{
					e.MatchEntry = entry;
					DoMethod(msg->list, MUIM_List_InsertSingle, &e, MUIV_List_Insert_Bottom);
				}
			}
		}
	}

	return NULL;
}

///
/// DECLARE(FindEmailCacheMatch)
// Method that search in the email cache and return the found entry if not more than one
DECLARE(FindEmailCacheMatch) // STRPTR matchText
{
	GETDATA;

	if(C->EmailCache == 0 || IsListEmpty(&data->EMailCache)) return NULL;

	if(msg->matchText && msg->matchText[0] != '\0')
	{
		int i, matches = 0;
		LONG tl = strlen(msg->matchText);
		struct EMailCacheNode *node = (struct EMailCacheNode *)(data->EMailCache.lh_Head);
		struct ABEntry *foundentry = NULL;

		for(i=0; i < C->EmailCache && ((struct Node *)node)->ln_Succ != NULL; i++, node = (struct EMailCacheNode *)((struct Node *)node)->ln_Succ)
		{
			struct ABEntry *entry = &node->ecn_Person;

			if(!Strnicmp(entry->RealName, msg->matchText, tl) || !Strnicmp(entry->Address, msg->matchText, tl))
			{
				if(++matches > 1)
				{
					foundentry = NULL;
					break;
				}
				foundentry = entry;
			}
		}

		return (ULONG)foundentry;
	}

	return NULL;
}

///
/// DECLARE(AddToEmailCache)
// method that parses a string for addresses and add them to the emailcache if enabled
DECLARE(AddToEmailCache) // struct Person *person
{
	GETDATA;

	if(C->EmailCache == 0) return -1;

	// We first check the Addressbook if this Person already exists in the AB and if
	// so we cancel this whole operation.
	if(!FindABPerson(msg->person, MUIV_NListtree_GetEntry_ListNode_Root))
	{
		int i;
		BOOL found = FALSE;
		struct EMailCacheNode *node = (struct EMailCacheNode *)(data->EMailCache.lh_Head);

		// Ok, it doesn`t exists in the AB, now lets check the cache list
		// itself
		for(i=0; i < C->EmailCache && ((struct Node *)node)->ln_Succ != NULL; node = (struct EMailCacheNode *)((struct Node *)node)->ln_Succ)
		{
			struct ABEntry *entry = &node->ecn_Person;

			// If we find the same entry already in the list we just move it
			// up to the top
			if((msg->person->RealName[0] ? !Stricmp(entry->RealName, msg->person->RealName) : TRUE) && !Stricmp(entry->Address, msg->person->Address))
			{
				Remove((struct Node *)node);
				AddHead(&data->EMailCache, (struct Node *)node);
				found = TRUE;
				break;
			}
		}

		// if we didn`t find the person already in the list
		// we have to add it after the last node
		if(!found)
		{
			struct EMailCacheNode *newnode;

			// we alloc mem for this new node and add it behind the last node
			if(newnode = calloc(1, sizeof(struct EMailCacheNode)))
			{
				struct ABEntry *entry = &newnode->ecn_Person;

				// Lets copy the data in the new Person struct
				strcpy(entry->RealName, msg->person->RealName);
				strcpy(entry->Address, msg->person->Address);

				Insert(&data->EMailCache, (struct Node *)newnode, ((struct Node *)node)->ln_Pred->ln_Pred);
			}
		}

		// Now lets save the emailcache file again
		SaveEMailCache(data->EMailCacheName, &data->EMailCache);
	}

	return 0;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	static STRPTR Classes[] = { "TextEditor.mcc", "Toolbar.mcc", "BetterString.mcc", "InfoText.mcc", "NListtree.mcc", "NList.mcc", "NListview.mcc", NULL };
	if(obj = (Object *)DoSuperNew(cl, obj,
		MUIA_Application_Author,         "YAM Open Source Team",
		MUIA_Application_Base,           "YAM",
		MUIA_Application_Title,          "YAM",
		MUIA_Application_Version,        yamversionstring,
		MUIA_Application_Copyright,      "Copyright � 2000-2002 YAM Open Source Team",
		MUIA_Application_Description,    GetStr(MSG_APP_DESCRIPTION),
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

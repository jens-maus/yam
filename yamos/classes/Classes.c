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

#include <string.h>
#include "Classes.h"

struct MUI_CustomClass *YAMClasses[NumberOfClasses];

/// RecipientstringDispatcher()
DISPATCHERPROTO(RecipientstringDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW                                  : return(m_Recipientstring_OM_NEW(cl, obj, (APTR)msg));
		case OM_DISPOSE                              : return(m_Recipientstring_OM_DISPOSE(cl, obj, (APTR)msg));
		case MUIM_Setup                              : return(m_Recipientstring_MUIM_Setup(cl, obj, (APTR)msg));
		case MUIM_GoActive                           : return(m_Recipientstring_MUIM_GoActive(cl, obj, (APTR)msg));
		case MUIM_GoInactive                         : return(m_Recipientstring_MUIM_GoInactive(cl, obj, (APTR)msg));
		case MUIM_HandleEvent                        : return(m_Recipientstring_MUIM_HandleEvent(cl, obj, (APTR)msg));
		case MUIM_Recipientstring_ShowMatches        : return(m_Recipientstring_ShowMatches(cl, obj, (APTR)msg));
		case MUIM_DragQuery                          : return(m_Recipientstring_MUIM_DragQuery(cl, obj, (APTR)msg));
		case MUIM_DragDrop                           : return(m_Recipientstring_MUIM_DragDrop(cl, obj, (APTR)msg));
	}
	return DoSuperMethodA(cl, obj, msg);
}

///
/// SearchwindowDispatcher()
DISPATCHERPROTO(SearchwindowDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW                                  : return(m_Searchwindow_OM_NEW(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Open                  : return(m_Searchwindow_Open(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Close                 : return(m_Searchwindow_Close(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Search                : return(m_Searchwindow_Search(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Next                  : return(m_Searchwindow_Next(cl, obj, (APTR)msg));
	}
	return DoSuperMethodA(cl, obj, msg);
}

///

/// MCCInfo struct
const struct { STRPTR Name; STRPTR Superclass; LONG SuperMCC; ULONG (*GetSize) (VOID); APTR Dispatcher; } MCCInfo[NumberOfClasses] =
{
	{ MUIC_Recipientstring, MUIC_BetterString, -1, RecipientstringGetSize, ENTRY(RecipientstringDispatcher) },
	{ MUIC_Searchwindow, MUIC_Window, -1, SearchwindowGetSize, ENTRY(SearchwindowDispatcher) }
};

///

/// YAM_NewObject()
Object *YAM_NewObject (STRPTR class, ULONG tag, ...)
{
	int i;
	for(i = 0; i < NumberOfClasses; i++)
	{
		if(!strcmp(MCCInfo[i].Name, class))
			return NewObjectA(YAMClasses[i]->mcc_Class, NULL, (struct TagItem *)&tag);
	}
	return NULL;
}

///
/// YAM_SetupClasses()
int YAM_SetupClasses (VOID)
{
	int i;
	memset(YAMClasses, 0, sizeof(YAMClasses));
	for (i = 0; i < NumberOfClasses; i++)
	{
		struct MUI_CustomClass *superMCC = MCCInfo[i].SuperMCC == -1 ? NULL : YAMClasses[MCCInfo[i].SuperMCC];
		YAMClasses[i] = MUI_CreateCustomClass(NULL, MCCInfo[i].Superclass, superMCC, MCCInfo[i].GetSize(), MCCInfo[i].Dispatcher);
		if (!YAMClasses[i])
		{
			YAM_CleanupClasses();
			return 0;
		}
	}
	return 1;
}

///
/// YAM_CleanupClasses()
VOID YAM_CleanupClasses (VOID)
{
	int i;
	for (i = NumberOfClasses-1; i >= 0; i--)
	{
		MUI_DeleteCustomClass(YAMClasses[i]);
		YAMClasses[i] = NULL;
	}
}

///

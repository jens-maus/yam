#include <string.h>
#include "Classes.h"

DISPATCHERPROTO(RecipientstringDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW                                  : return(m_Recipientstring_OM_NEW(cl, obj, (APTR)msg));
		case OM_DISPOSE                              : return(m_Recipientstring_OM_DISPOSE(cl, obj, (APTR)msg));
		case MUIM_Setup                              : return(m_Recipientstring_MUIM_Setup(cl, obj, (APTR)msg));
		case MUIM_GoActive                           : return(m_Recipientstring_MUIM_GoActive(cl, obj, (APTR)msg));
		case MUIM_GoInactive                         : return(m_Recipientstring_MUIM_GoInactive(cl, obj, (APTR)msg));
		case MUIM_DragQuery                          : return(m_Recipientstring_MUIM_DragQuery(cl, obj, (APTR)msg));
		case MUIM_DragDrop                           : return(m_Recipientstring_MUIM_DragDrop(cl, obj, (APTR)msg));
		case OM_GET                                  : return(m_Recipientstring_OM_GET(cl, obj, (APTR)msg));
		case MUIM_Popstring_Open                     : return(m_Recipientstring_MUIM_Popstring_Open(cl, obj, (APTR)msg));
		case MUIM_Recipientstring_AddRecipient       : return(m_Recipientstring_AddRecipient(cl, obj, (APTR)msg));
		case MUIM_Recipientstring_Transform          : return(m_Recipientstring_Transform(cl, obj, (APTR)msg));
		case MUIM_Recipientstring_RecipientStart     : return(m_Recipientstring_RecipientStart(cl, obj, (APTR)msg));
		case MUIM_HandleEvent                        : return(m_Recipientstring_MUIM_HandleEvent(cl, obj, (APTR)msg));
		case MUIM_Recipientstring_ShowMatches        : return(m_Recipientstring_ShowMatches(cl, obj, (APTR)msg));
	}
	return DoSuperMethodA(cl, obj, msg);
}

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

const struct { STRPTR Name; STRPTR Superclass; LONG SuperMCC; ULONG (*GetSize) (VOID); APTR Dispatcher; } MCCInfo[NumberOfClasses] = 
{
	{ MUIC_Recipientstring, MUIC_BetterString, -1, RecipientstringGetSize, ENTRY(RecipientstringDispatcher) },
	{ MUIC_Searchwindow, MUIC_Window, -1, SearchwindowGetSize, ENTRY(SearchwindowDispatcher) }
};

struct MUI_CustomClass *YAMClasses[NumberOfClasses];

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

VOID YAM_CleanupClasses (VOID)
{
	int i;
	for (i = NumberOfClasses-1; i >= 0; i--)
	{
		MUI_DeleteCustomClass(YAMClasses[i]);
		YAMClasses[i] = NULL;
	}
}

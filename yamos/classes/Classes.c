#include <string.h>
#include "Classes.h"

DISPATCHERPROTO(SearchwindowDispatcher)
{
	switch (msg->MethodID)
	{
		case OM_NEW                                  : return(m_Searchwindow_OM_NEW(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Open                  : return(m_Searchwindow_Open(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Search                : return(m_Searchwindow_Search(cl, obj, (APTR)msg));
		case MUIM_Searchwindow_Next                  : return(m_Searchwindow_Next(cl, obj, (APTR)msg));
	}
	return DoSuperMethodA(cl, obj, msg);
}

const struct { STRPTR Name; STRPTR Superclass; LONG SuperMCC; ULONG (*GetSize) (VOID); APTR Dispatcher; } MCCInfo[NumberOfClasses] = 
{
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

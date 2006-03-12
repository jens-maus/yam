/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
 YAM OpenSource project		 : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_NList
 Description: List that manages the different pages of the configuration

***************************************************************************/

#include "ConfigPageList_cl.h"

/* CLASSDATA
struct Data
{
	struct Hook displayHook;
	Object 			*configIcon[MAXCPAGES];
};
*/

/* EXPORT
struct PageList
{
	int  Offset;
	APTR PageLabel;
};
*/

/* Hooks */
/// DisplayHook
//  Section listview displayhook
HOOKPROTONH(DisplayFunc, long, char **array, struct PageList *entry)
{
	static char page[SIZE_DEFAULT];
	
	sprintf(array[0] = page, "\033o[%d] %s", entry->Offset, GetStr(entry->PageLabel));
	
	return 0;
}
MakeStaticHook(DisplayHook, DisplayFunc);
///

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
	if((obj = DoSuperNew(cl, obj,
		TAG_MORE, inittags(msg))))
	{
		GETDATA;
		int i;

		InitHook(&data->displayHook, DisplayHook, data);
		set(obj, MUIA_NList_DisplayHook, &data->displayHook);

		// create/load all bodychunkimages of our config icons
		data->configIcon[0]  = MakeBCImage("config_firststep");
		data->configIcon[1]  = MakeBCImage("config_network");
		data->configIcon[2]  = MakeBCImage("config_newmail");
		data->configIcon[3]  = MakeBCImage("config_filters");
		data->configIcon[4]  = MakeBCImage("config_read");
		data->configIcon[5]  = MakeBCImage("config_write");
		data->configIcon[6]  = MakeBCImage("config_answer");
		data->configIcon[7]  = MakeBCImage("config_signature");
		data->configIcon[8]  = MakeBCImage("config_lists");
		data->configIcon[9]  = MakeBCImage("config_security");
		data->configIcon[10] = MakeBCImage("config_start");
		data->configIcon[11] = MakeBCImage("config_mime");
		data->configIcon[12] = MakeBCImage("config_abook");
		data->configIcon[13] = MakeBCImage("config_scripts");
		data->configIcon[14] = MakeBCImage("config_misc");
		data->configIcon[15] = MakeBCImage("config_update");

		// now we can add the config icon objects and use UseImage
		// to prepare it for displaying it in the NList
		for(i = 0; i < MAXCPAGES; i++)
		{
			DoMethod(obj, OM_ADDMEMBER, data->configIcon[i]);
			DoMethod(obj, MUIM_NList_UseImage, data->configIcon[i], i, MUIF_NONE);
		}
	}
	
	return (ULONG)obj;
}

///

/* Public Methods */

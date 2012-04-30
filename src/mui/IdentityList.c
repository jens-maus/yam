/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

 Superclass:  MUIC_NList
 Description: NList class to display all User identities

***************************************************************************/

#include "IdentityList_cl.h"

#include <mui/NList_mcc.h>

#include "YAM_utilities.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  char displayBuffer[SIZE_DEFAULT];
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  obj = DoSuperNew(cl, obj,

    InputListFrame,
    MUIA_NList_Title,        FALSE,
    MUIA_NList_DragType,     MUIV_NList_DragType_Immediate,
    MUIA_NList_DragSortable, TRUE,

    TAG_MORE, inittags(msg));

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_NList_Display)
//OVERLOAD(MUIM_NList_Display)
//{
//  struct MUIP_NList_Display *ndm = (struct MUIP_NList_Display *)msg;
//  struct MailServerNode *msn = (struct MailServerNode *)ndm->entry;
//
//  ENTER();
//
//  if(msn != NULL)
//  {
//    GETDATA;
//
//    snprintf(data->displayBuffer, sizeof(data->displayBuffer), "%d) %s%s" MUIX_N, (int)ndm->strings[-1]+1,
//                                                                                  isServerActive(msn) == FALSE ? MUIX_I : "",
//                                                                                  msn->description);
//
//    ndm->strings[0] = data->displayBuffer;
//  }
//
//  LEAVE();
//  return 0;
//}
//
///

/* Private Functions */

/* Public Methods */


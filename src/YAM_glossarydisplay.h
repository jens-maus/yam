#ifndef YAM_GLOSSARYDISPLAY_H
#define YAM_GLOSSARYDISPLAY_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

***************************************************************************/

#include "YAM_stringsizes.h"

struct Dict
{
   char  Alias[SIZE_NAME];
   char *Text;
};

struct DI_GUIData
{
   Object *WI;
   Object *GR_LIST;
   Object *GR_TEXT;
   Object *LV_ENTRIES;
   Object *ST_ALIAS;
   Object *TE_EDIT;
   Object *SL_EDIT;
   Object *BT_NEW;
   Object *BT_DELETE;
   Object *BT_ADDSELECT;
   Object *BT_PASTE;
};

struct DI_ClassData  /* glossary window */
{
  struct DI_GUIData GUI;
  struct Dict *     OldEntry;
  Object *          writeWindow;
  BOOL              Modified;
  char              ScreenTitle[SIZE_DEFAULT];
};

extern struct Hook DI_OpenHook;

#endif /* YAM_GLOSSARYDISPLAY_H */

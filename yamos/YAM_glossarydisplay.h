#ifndef YAM_GLOSSARYDISPLAY_H
#define YAM_GLOSSARYDISPLAY_H

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

struct Dict
{
   char *Text;
   char  Alias[SIZE_NAME];
};

struct DI_GUIData
{
   APTR WI;
   APTR GR_LIST;
   APTR GR_TEXT;
   APTR LV_ENTRIES;
   APTR ST_ALIAS;
   APTR TE_EDIT;
   APTR SL_EDIT;
   APTR BT_NEW;
   APTR BT_DELETE;
   APTR BT_ADDSELECT;
   APTR BT_PASTE;
};

struct DI_ClassData  /* glossary window */
{
   struct DI_GUIData GUI;
   struct Dict *     OldEntry;
   int               WrWin;
   BOOL              Modified;
};

extern struct Hook DI_OpenHook;

#endif /* YAM_GLOSSARYDISPLAY_H */

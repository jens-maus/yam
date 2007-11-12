/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project    : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_Group
 Description: Provides GUI elements and routines for showing the currently
              available themes for YAM.

***************************************************************************/

#include "ThemeListGroup_cl.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *NL_THEMELIST;
  Object *BT_ACTIVATE;
  Object *TX_AUTHOR;
  Object *TX_URL;

  char activeTheme[SIZE_FILE];
};
*/

/* Hooks */

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *themeListObject;
  Object *activateButtonObject;
  Object *authorTextObject;
  Object *urlTextObject;

  ENTER();

  obj = DoSuperNew(cl, obj,
          MUIA_Group_Horiz,       TRUE,
          MUIA_ContextMenu,       FALSE,

          Child, VGroup,
            MUIA_HorizWeight, 30,
            Child, NListviewObject,
              MUIA_CycleChain, TRUE,
              MUIA_NListview_NList, themeListObject = NListObject,
                InputListFrame,
                MUIA_NList_DragType,     MUIV_NList_DragType_None,
//                  MUIA_NList_Format,       "BAR",
//                  MUIA_NList_Title,        TRUE,
//                  MUIA_NList_DisplayHook2, &FilterDisplayHook,
              End,
            End,

            Child, activateButtonObject = MakeButton(tr(MSG_CO_THEME_ACTIVATE)),
          End,

          Child, BalanceObject, End,

          Child, VGroup,
            MUIA_HorizWeight, 70,
            Child, HGroup,
              Child, HSpace(0),
              Child, TextObject,
                MUIA_Text_Contents, tr(MSG_CO_THEME_PREVIEW),
                MUIA_Font,          MUIV_Font_Tiny,
                MUIA_HorizWeight,   0,
              End,
              Child, HSpace(0),
            End,

            Child, RectangleObject,
              MUIA_Rectangle_HBar, TRUE,
              MUIA_FixHeight,      4,
            End,

            Child, HVSpace,

            Child, RectangleObject,
              MUIA_Rectangle_HBar, TRUE,
              MUIA_FixHeight,      4,
            End,

            Child, ColGroup(2),
              Child, Label2(tr(MSG_CO_THEME_AUTHOR)),
              Child, authorTextObject = TextObject,
                TextFrame,
                MUIA_Background,  MUII_TextBack,
                MUIA_Text_SetMin, TRUE,
              End,

              Child, Label2(tr(MSG_CO_THEME_URL)),
              Child, urlTextObject = TextObject,
                TextFrame,
                MUIA_Background,  MUII_TextBack,
                MUIA_Text_SetMin, TRUE,
              End,
            End,
          End,

        TAG_MORE, inittags(msg));

  if(obj != NULL)
  {
    GETDATA;

    data->NL_THEMELIST = themeListObject;
    data->BT_ACTIVATE = activateButtonObject;
    data->TX_AUTHOR = authorTextObject;
    data->TX_URL = urlTextObject;
  }

  RETURN((ULONG)obj);
  return (ULONG)obj;
}
///
/// OVERLOAD(OM_GET)
/* this is just so that we can notify the popup tag */
OVERLOAD(OM_GET)
{
  GETDATA;
  ULONG *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    ATTR(Active) : *store = (ULONG)data->activeTheme; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
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
      ATTR(Active):
      {
        strlcpy(data->activeTheme, (char*)tag->ti_Data, sizeof(data->activeTheme));

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}
///

/* Public Methods */
/// DECLARE(Update)
#define isDrawer(etype)   (etype >= 0 && etype != ST_SOFTLINK && etype != ST_LINKDIR)

DECLARE(Update)
{
  BOOL result = FALSE;
  BPTR dirLock;
  char themesDir[SIZE_PATH];

  ENTER();

  // construct the themes directory path
  strmfp(themesDir, G->ProgDir, "Themes");

  if((dirLock = Lock(themesDir, ACCESS_READ)))
  {
    struct ExAllControl *eac;

    if((eac = AllocDosObject(DOS_EXALLCONTROL, NULL)))
    {
      struct ExAllData *ead;
      struct ExAllData *eabuffer;
      LONG more;
      eac->eac_LastKey = 0;
      eac->eac_MatchString = NULL;
      eac->eac_MatchFunc = NULL;

      if((eabuffer = malloc(SIZE_EXALLBUF)))
      {
        result = TRUE;

        do
        {
          more = ExAll(dirLock, eabuffer, SIZE_EXALLBUF, ED_TYPE, eac);
          if(!more && IoErr() != ERROR_NO_MORE_ENTRIES)
          {
            // fail with an error
            result = FALSE;
            break;
          }

          // ExAll() failed normally with no further entries
          if(eac->eac_Entries == 0)
            continue;

          // we received a filled ExAll buffer so we
          // parse through it now.
          ead = (struct ExAllData *)eabuffer;
          do
          {
            // check that this entry is a drawer
            // because we don't accept any file here
            if(isDrawer(ead->ed_Type))
            {
              D(DBF_CONFIG, "found dir '%s' in themes drawer", ead->ed_Name);

            }
            else
              W(DBF_CONFIG, "unknown file '%s' in themes directory ignored", ead->ed_Name);
          }
          while((ead = ead->ed_Next));
        }
        while(more);

        free(eabuffer);
      }
      else
        E(DBF_CONFIG, "couldn't allocate memory for ExAll buffer");

      FreeDosObject(DOS_EXALLCONTROL, eac);
    }
    else
      E(DBF_CONFIG, "couldn't allocate ExAll dos object!");

    UnLock(dirLock);
  }
  else
    E(DBF_CONFIG, "No themes directory found!");

  RETURN(result);
  return result;
}
///


/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

 Superclass:  MUIC_Popobject
 Description: Popobject to select a PGP key

***************************************************************************/

#include "PGPKeyPopup_cl.h"

#include <string.h>

#include <proto/dos.h>
#include <proto/muimaster.h>

#include "SDI_hook.h"

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_PGPKEY;
};
*/

/* Hooks */
/// SetPublicKey
//  Copies public PGP key from list to string gadget
HOOKPROTONH(SetPublicKey, void, Object *listview, Object *string)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
    if(var != NULL)
    {
      char buf[8 + 2 + 1]; // 8 chars + 2 extra chars required.

      snprintf(buf, sizeof(buf), "0x%s", var);
      setstring(string, buf);
    }
  }

  LEAVE();
}
MakeStaticHook(SetPublicKeyHook, SetPublicKey);

///
/// ListPublicKeys
//  Lists keys of public PGP keyring in a popup window
HOOKPROTONH(ListPublicKeys, long, Object *listview, Object *string)
{
  BOOL secret;
  int retc, keys = 0;
  FILE *fp;

  ENTER();

  // should we display secret keys?
  secret = (BOOL)xget(listview, MUIA_UserData);

  if(G->PGPVersion == 5)
    retc = PGPCommand("pgpk", "-l +language=us", KEEPLOG);
  else
  {
    char buf[SIZE_LARGE];

    strlcpy(buf, "-kv  ", sizeof(buf));
    if(secret == TRUE)
    {
      char p;

      GetVar("PGPPATH", &buf[4], sizeof(buf) - 4, 0);
      if((p = buf[strlen(buf) - 1]) != ':' && p != '/')
        strlcat(buf, "/", sizeof(buf));

      strlcat(buf, "secring.pgp", sizeof(buf));
    }
    retc = PGPCommand("pgp", buf, KEEPLOG);
  }

  if(retc == 0 && (fp = fopen(PGPLOGFILE, "r")) != NULL)
  {
    char *buf = NULL;
    size_t size = 0;
    char *str = (char *)xget(string, MUIA_String_Contents);
    Object *list = (Object *)xget(listview, MUIA_Listview_List);

    str = (char *)xget(string, MUIA_String_Contents);
    DoMethod(list, MUIM_List_Clear);

    setvbuf(fp, NULL, _IOFBF, SIZE_FILEBUF);

    while(GetLine(&buf, &size, fp) >= 0)
    {
      char entry[SIZE_DEFAULT];

      memset(entry, 0, sizeof(entry));
      if(G->PGPVersion == 5)
      {
        if(strncmp(buf, "sec", 3) == 0 || (strncmp(&buf[1], "ub", 2) == 0 && secret == FALSE))
        {
          memcpy(entry, &buf[12], 8);

          while(GetLine(&buf, &size, fp) >= 0)
          {
            if(strncmp(buf, "uid", 3) == 0)
            {
              strlcat(entry, &buf[4], sizeof(entry) - 9);
              break;
            }
          }
        }
      }
      else
      {
        if(buf[9] == '/' && buf[23] == '/')
        {
          memcpy(entry, &buf[10], 8);
          strlcat(entry, &buf[29], sizeof(entry) - 8);
        }
      }

      if(entry[0] != '\0')
      {
        DoMethod(list, MUIM_List_InsertSingle, entry, MUIV_List_Insert_Bottom);
        if(!strncmp(entry, str, 8))
          set(list, MUIA_List_Active, keys);

        keys++;
      }
    }

    fclose(fp);

    free(buf);

    if(DeleteFile(PGPLOGFILE) == 0)
      AddZombieFile(PGPLOGFILE);
  }

  if(keys == 0)
    ER_NewError(tr(MSG_ER_NoPublicKeys), "", "");

  RETURN(keys > 0);
  return keys > 0;
}
MakeStaticHook(ListPublicKeysHook, ListPublicKeys);

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *ST_PGPKEY;
  Object *lv;
  BOOL secret;
  char *label;

  ENTER();

  secret = GetTagData(ATTR(Secret), FALSE, inittags(msg));
  label = (char *)GetTagData(ATTR(Label), (ULONG)NULL, inittags(msg));

  if((obj = DoSuperNew(cl, obj,
    MUIA_Popstring_String, ST_PGPKEY = MakeString(SIZE_DEFAULT, label),
    MUIA_Popstring_Button, PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &ListPublicKeysHook,
    MUIA_Popobject_ObjStrHook, &SetPublicKeyHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, lv = ListviewObject,
      MUIA_UserData, secret,
      MUIA_Listview_List, ListObject,
        InputListFrame,
        MUIA_List_AdjustWidth, TRUE,
        MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
        MUIA_List_DestructHook, MUIV_List_DestructHook_String,
      End,
    End,
    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_PGPKEY = ST_PGPKEY;

    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, obj, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(ST_PGPKEY, MUIM_Notify, MUIA_String_Contents, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(PGPKeyChanged), TRUE);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;
  ULONG result;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(PGPKey):
      {
        nnset(data->ST_PGPKEY, MUIA_String_Contents, tag->ti_Data);
      }
      break;
    }
  }

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(PGPKeyChanged):
    {
      *store = TRUE;

      return TRUE;
    }
    break;

    case ATTR(PGPKey):
    {
      *store = xget(data->ST_PGPKEY, MUIA_String_Contents);

      return TRUE;
    }
    break;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_error.h"
#include "YAM_read.h"

#include "mui/ClassesExtra.h"
#include "mui/Recipientstring.h"

#include "Locale.h"
#include "MUIObjects.h"

#include "Debug.h"

/// MakeCycle
//  Creates a MUI cycle object
Object *MakeCycle(const char *const *labels, const char *label)
{
  return CycleObject,
           MUIA_CycleChain,    TRUE,
           MUIA_Font,          MUIV_Font_Button,
           MUIA_Cycle_Entries, labels,
           MUIA_ControlChar,   ShortCut(label),
         End;
}

///
/// MakeButton
//  Creates a MUI button
Object *MakeButton(const char *txt)
{
   Object *obj;

   if((obj = MUI_MakeObject(MUIO_Button,(IPTR)txt)) != NULL)
     set(obj, MUIA_CycleChain, TRUE);

   return obj;
}

///
/// MakeCheck
//  Creates a MUI checkmark object
Object *MakeCheck(const char *label)
{
  return ImageObject,
           ImageButtonFrame,
           MUIA_InputMode   , MUIV_InputMode_Toggle,
           MUIA_Image_Spec  , MUII_CheckMark,
           MUIA_Background  , MUII_ButtonBack,
           MUIA_ShowSelState, FALSE,
           MUIA_ControlChar , ShortCut(label),
           MUIA_CycleChain  , TRUE,
         End;
}

///
/// MakeCheckGroup
//  Creates a labelled MUI checkmark object
Object *MakeCheckGroup(Object **check, const char *label)
{
   return HGroup,
            Child, *check = MakeCheck(label),
            Child, Label1(label),
            Child, HSpace(0),
          End;
}

///
/// MakeString
//  Creates a MUI string object
Object *MakeString(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,      maxlen,
           MUIA_String_AdvanceOnCR, TRUE,
           MUIA_ControlChar,        ShortCut(label),
           MUIA_CycleChain,         TRUE,
         End;
}

///
/// MakePassString
//  Creates a MUI string object with hidden text
Object *MakePassString(const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       SIZE_PASSWORD,
           MUIA_String_Secret,       TRUE,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
         End;
}

///
/// MakeInteger
//  Creates a MUI string object for numeric input
Object *MakeInteger(int maxlen, const char *label)
{
  return BetterStringObject,
           StringFrame,
           MUIA_String_MaxLen,       maxlen+1,
           MUIA_String_AdvanceOnCR,  TRUE,
           MUIA_ControlChar,         ShortCut(label),
           MUIA_CycleChain,          TRUE,
           MUIA_String_Integer,      0,
           MUIA_String_Accept,       "0123456789",
         End;
}

///
/// PO_SetPublicKey
//  Copies public PGP key from list to string gadget
HOOKPROTONH(PO_SetPublicKey, void, Object *pop, Object *string)
{
  char *var = NULL;

  ENTER();

  DoMethod(pop, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
  if(var != NULL)
  {
    char buf[8 + 2 + 1]; // 8 chars + 2 extra chars required.

    strlcpy(buf, "0x", sizeof(buf));
    strlcat(buf, var, sizeof(buf));

    setstring(string, buf);
  }

  LEAVE();
}
MakeStaticHook(PO_SetPublicKeyHook, PO_SetPublicKey);

///
/// PO_ListPublicKeys
//  Lists keys of public PGP keyring in a popup window
HOOKPROTONH(PO_ListPublicKeys, long, APTR pop, APTR string)
{
  APTR secret;
  char *str;
  int retc, keys = 0;
  FILE *fp;

  ENTER();

  secret = str = (char *)xget(pop, MUIA_UserData);
  if(G->PGPVersion == 5)
  {
    retc = PGPCommand("pgpk", "-l +language=us", KEEPLOG);
  }
  else
  {
    char buf[SIZE_LARGE];

    strlcpy(buf, "-kv  ", sizeof(buf));
    if(secret != NULL)
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

    str = (char *)xget(string, MUIA_String_Contents);
    DoMethod(pop, MUIM_List_Clear);

    setvbuf(fp, NULL, _IOFBF, SIZE_FILEBUF);

    while(GetLine(&buf, &size, fp) >= 0)
    {
      char entry[SIZE_DEFAULT];

      memset(entry, 0, SIZE_DEFAULT);
      if(G->PGPVersion == 5)
      {
        if(strncmp(buf, "sec", 3) == 0 || (strncmp(&buf[1], "ub", 2) == 0 && secret == NULL))
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
        DoMethod(pop, MUIM_List_InsertSingle, entry, MUIV_List_Insert_Bottom);
        if(!strncmp(entry, str, 8))
          set(pop, MUIA_List_Active, keys);
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
MakeStaticHook(PO_ListPublicKeysHook, PO_ListPublicKeys);

///
/// MakePGPKeyList
//  Creates a PGP id popup list
Object *MakePGPKeyList(Object **st, BOOL secret, const char *label)
{
  Object *po, *lv;

  if ((po = PopobjectObject,
        MUIA_Popstring_String, *st = MakeString(SIZE_DEFAULT, label),
        MUIA_Popstring_Button, PopButton(MUII_PopUp),
        MUIA_Popobject_StrObjHook, &PO_ListPublicKeysHook,
        MUIA_Popobject_ObjStrHook, &PO_SetPublicKeyHook,
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
     End))
  {
    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
  }

  return po;
}

///
/// MakeAddressField
//  Creates a recipient field
Object *MakeAddressField(Object **string, const char *label, const Object *help, int abmode, int winnr, ULONG flags)
{
  Object *obj;
  Object *bt_adr;

  ENTER();

  if((obj = HGroup,

    GroupSpacing(1),
    Child, *string = RecipientstringObject,
      MUIA_CycleChain,                          TRUE,
      MUIA_String_AdvanceOnCR,                  TRUE,
      MUIA_Recipientstring_ResolveOnCR,         TRUE,
      MUIA_Recipientstring_MultipleRecipients,  isFlagSet(flags, AFF_ALLOW_MULTI),
      MUIA_BetterString_NoShortcuts,            isFlagSet(flags, AFF_EXTERNAL_SHORTCUTS),
      MUIA_ControlChar,                         ShortCut(label),
    End,
    Child, bt_adr = PopButton(MUII_PopUp),

  End))
  {
    SetHelp(*string,help);
    SetHelp(bt_adr, MSG_HELP_WR_BT_ADR);

    if(abmode == ABM_CONFIG)
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
      DoMethod(*string, MUIM_Notify, MUIA_Recipientstring_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, *string);
    }
    else
    {
      DoMethod(bt_adr, MUIM_Notify, MUIA_Pressed, FALSE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnr);
      DoMethod(*string, MUIM_Notify, MUIA_Recipientstring_Popup, TRUE, MUIV_Notify_Application, 4, MUIM_CallHook, &AB_OpenHook, abmode, winnr);
    }

    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime,  bt_adr, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN(obj);
  return obj;
}

///
/// MakeNumeric
//  Creates a MUI numeric slider
Object *MakeNumeric(int min, int max, BOOL percent)
{
  return NumericbuttonObject,
           MUIA_Numeric_Min, min,
           MUIA_Numeric_Max, max,
           MUIA_Numeric_Format, percent ? "%ld%%" : "%ld",
           MUIA_CycleChain, TRUE,
         End;
}

///
/// PO_CharsetOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTONH(PO_CharsetOpenFunc, BOOL, Object *list, Object *str)
{
  char *s;

  ENTER();

  if((s = (char *)xget(str, MUIA_Text_Contents)) != NULL)
  {
    int i;

    for(i=0;;i++)
    {
      char *x;

      DoMethod(list, MUIM_List_GetEntry, i, &x);
      if(x == NULL)
      {
        set(list, MUIA_List_Active, MUIV_List_Active_Off);
        break;
      }
      else if(stricmp(x, s) == 0)
      {
        set(list, MUIA_List_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(PO_CharsetOpenHook, PO_CharsetOpenFunc);

///
/// PO_CharsetCloseHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTONH(PO_CharsetCloseFunc, void, Object *list, Object *txt)
{
  char *var = NULL;

  ENTER();

  DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &var);
  if(var != NULL)
    set(txt, MUIA_Text_Contents, var);

  LEAVE();
}
MakeStaticHook(PO_CharsetCloseHook, PO_CharsetCloseFunc);

///
/// PO_CharsetListDisplayHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTONH(PO_CharsetListDisplayFunc, LONG, const char **array, STRPTR str)
{
  ENTER();

  if(str != NULL)
  {
    struct codeset *cs;

    // the standard name is always in column 0
    array[0] = str;

    // try to find the codeset via codesets.library and
    // display some more information about it.
    if((cs = CodesetsFind(str,
                          CSA_CodesetList,       G->codesetsList,
                          CSA_FallbackToDefault, FALSE,
                          TAG_DONE)) != NULL)
    {
      if(cs->characterization && stricmp(cs->characterization, str) != 0)
        array[1] = cs->characterization;
      else
        array[1] = "";
    }
    else
      array[1] = "";
  }
  else
  {
    array[0] = "";
    array[1] = "";
  }

  RETURN(0);
  return 0;
}
MakeStaticHook(PO_CharsetListDisplayHook, PO_CharsetListDisplayFunc);

///
/// MakeCharsetPop
//  Creates a popup list of available charsets supported by codesets.library
Object *MakeCharsetPop(Object **string, Object **pop)
{
  Object *lv;
  Object *po;

  ENTER();

  if((po = PopobjectObject,

    MUIA_Popstring_String, *string = TextObject,
      TextFrame,
      MUIA_Background,  MUII_TextBack,
    End,

    MUIA_Popstring_Button, *pop = PopButton(MUII_PopUp),
    MUIA_Popobject_StrObjHook, &PO_CharsetOpenHook,
    MUIA_Popobject_ObjStrHook, &PO_CharsetCloseHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, lv = ListviewObject,
       MUIA_Listview_ScrollerPos, MUIV_Listview_ScrollerPos_Right,
       MUIA_Listview_List, ListObject,
          InputListFrame,
          MUIA_List_Format,        "BAR,",
          MUIA_List_AutoVisible,   TRUE,
          MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
          MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
          MUIA_List_DisplayHook,   &PO_CharsetListDisplayHook,
       End,
    End,

  End) != NULL)
  {
    struct codeset *codeset;
    STRPTR *array;

    set(*pop, MUIA_CycleChain,TRUE);
    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);

    // Build list of available codesets
    if((array = CodesetsSupported(CSA_CodesetList, G->codesetsList,
                                  TAG_DONE)) != NULL)
    {
      DoMethod(lv, MUIM_List_Insert, array, -1, MUIV_List_Insert_Sorted);
      CodesetsFreeA(array, NULL);
    }
    else
      set(po, MUIA_Disabled, TRUE);

    // Use the system's default codeset
    if((codeset = CodesetsFindA(NULL, NULL)) != NULL)
      set(*string, MUIA_String_Contents, codeset->name);
  }
  else
  {
    *string = NULL;
    *pop = NULL;
  }

  RETURN(po);
  return po;
}

///
/// ShortCut
//  Finds keyboard shortcut in text label
char ShortCut(const char *label)
{
  char scut = '\0';
  char *ptr;

  ENTER();

  if((ptr = strchr(label, '_')) != NULL)
    scut = (char)ToLower((ULONG)(*++ptr));

  RETURN(scut);
  return scut;
}

///
/// GetMUIInteger
//  Returns the numeric value of a MUI string object
int GetMUIInteger(Object *obj)
{
   return (int)xget(obj,MUIA_String_Integer);
}

///
/// GetMUICheck
//  Returns the value of a MUI checkmark object
BOOL GetMUICheck(Object *obj)
{
   return (BOOL)xget(obj, MUIA_Selected);
}

///
/// GetMUICycle
//  Returns the value of a MUI cycle object
int GetMUICycle(Object *obj)
{
   return (int)xget(obj, MUIA_Cycle_Active);
}

///
/// GetMUIRadio
//  Returns the value of a MUI radio object
int GetMUIRadio(Object *obj)
{
   return (int)xget(obj, MUIA_Radio_Active);
}

///
/// GetMUINumer
//  Returns the value of a MUI numeric slider
int GetMUINumer(Object *obj)
{
   return (int)xget(obj, MUIA_Numeric_Value);
}

///
/// isChildOfList
// check if the supplied object is a member of the list
static BOOL isChildOfList(struct List *list, Object *child)
{
  BOOL isChild = FALSE;

  ENTER();

  if(list != NULL)
  {
    Object *curchild;
    Object *cstate;

    // here we check whether the child is part of the supplied group
    cstate = (Object *)list->lh_Head;
    while((curchild = NextObject(&cstate)) != NULL)
    {
      if(curchild == child)
      {
        isChild = TRUE;
        break;
      }
    }
  }

  RETURN(isChild);
  return isChild;
}

///
/// isChildOfGroup
// return TRUE if the supplied child object is part of the supplied group
BOOL isChildOfGroup(Object *group, Object *child)
{
  BOOL isChild;

  ENTER();

  // get the child list of the group object
  isChild = isChildOfList((struct List *)xget(group, MUIA_Group_ChildList), child);

  RETURN(isChild);
  return isChild;
}

///
/// isChildOfFamily
// return TRUE if the supplied child object is part of the supplied family/menu object
BOOL isChildOfFamily(Object *family, Object *child)
{
  BOOL isChild = FALSE;

  ENTER();

  // get the child list of the family object
  isChild = isChildOfList((struct List *)xget(family, MUIA_Family_List), child);

  RETURN(isChild);
  return isChild;
}

///

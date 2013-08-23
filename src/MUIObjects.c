/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include <proto/asl.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_read.h"

#include "mui/CharsetPopupList.h"
#include "mui/ClassesExtra.h"
#include "mui/PlaceholderPopupList.h"
#include "mui/Recipientstring.h"

#include "Locale.h"
#include "MimeTypes.h"
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
HOOKPROTONH(PO_SetPublicKey, void, Object *listview, Object *string)
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

      strlcpy(buf, "0x", sizeof(buf));
      strlcat(buf, var, sizeof(buf));

      setstring(string, buf);
    }
  }

  LEAVE();
}
MakeStaticHook(PO_SetPublicKeyHook, PO_SetPublicKey);

///
/// PO_ListPublicKeys
//  Lists keys of public PGP keyring in a popup window
HOOKPROTONH(PO_ListPublicKeys, long, Object *listview, Object *string)
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

      memset(entry, 0, SIZE_DEFAULT);
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
MakeStaticHook(PO_ListPublicKeysHook, PO_ListPublicKeys);

///
/// MakePGPKeyList
//  Creates a PGP id popup list
Object *MakePGPKeyList(Object **st, BOOL secret, const char *label)
{
  Object *po;
  Object *lv;

  ENTER();

  if((po = PopobjectObject,
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

  RETURN(po);
  return po;
}

///
/// MakeAddressField
//  Creates a recipient field
Object *MakeAddressField(Object **string, const char *label, const void *help, int abmode, int winnr, ULONG flags)
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
      MUIA_Recipientstring_NoFullName,          isFlagSet(flags, AFF_NOFULLNAME),
      MUIA_Recipientstring_NoCache,             isFlagSet(flags, AFF_NOCACHE),
      MUIA_Recipientstring_NoValid,             isFlagSet(flags, AFF_NOVALID),
      MUIA_Recipientstring_ResolveOnInactive,   isFlagSet(flags, AFF_RESOLVEINACTIVE),
      MUIA_BetterString_NoShortcuts,            isFlagSet(flags, AFF_EXTERNAL_SHORTCUTS),
      MUIA_ControlChar,                         ShortCut(label),
    End,
    Child, bt_adr = PopButton(MUII_PopUp),

  End))
  {
    if(help != NULL)
      SetHelp(*string, help);
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
HOOKPROTONH(PO_CharsetOpenFunc, BOOL, Object *listview, Object *str)
{
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_Text_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    int i;

    for(i=0;;i++)
    {
      char *x;

      DoMethod(list, MUIM_NList_GetEntry, i, &x);
      if(x == NULL)
      {
        set(list, MUIA_NList_Active, MUIV_NList_Active_Off);
        break;
      }
      else if(stricmp(x, s) == 0)
      {
        set(list, MUIA_NList_Active, i);
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
HOOKPROTONH(PO_CharsetCloseFunc, void, Object *listview, Object *txt)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &var);
    if(var != NULL)
      set(txt, MUIA_Text_Contents, var);
  }

  LEAVE();
}
MakeStaticHook(PO_CharsetCloseHook, PO_CharsetCloseFunc);

///
/// MakeCodesetPop
//  Creates a popup list of available codesets supported by codesets.library
Object *MakeCodesetPop(Object **string, Object **pop)
{
  Object *listview;
  Object *list;
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
    MUIA_Popobject_Object, listview = NListviewObject,
      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
      MUIA_NListview_NList, list = CharsetPopupListObject,
      End,
    End,

  End) != NULL)
  {
    struct codeset *codeset;

    set(*pop, MUIA_CycleChain,TRUE);
    DoMethod(list, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);

    // disable the popup button in case there are no charsets available
    if(xget(list, MUIA_NList_Entries) == 0)
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
/// PO_HandleVarHook
//  Pastes an entry from variable listview into string gadget
HOOKPROTONH(PO_HandleVarFunc, void, Object *listview, Object *string)
{
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_NListview_NList)) != NULL)
  {
    char *var = NULL;

    DoMethod(list, MUIM_NList_GetEntry, MUIV_NList_GetEntry_Active, &var);
    if(var != NULL)
    {
      char addstr[3];
      char *str = (char *)xget(string, MUIA_String_Contents);
      LONG pos = xget(string, MUIA_String_BufferPos);

      strlcpy(addstr, var, sizeof(addstr));

      if(str != NULL && str[0] != '\0')
      {
        int len = strlen(str)+sizeof(addstr);
        char *buf;

        if((buf = calloc(1, len)) != NULL)
        {
          // append the addstr to the right position

          if(pos > 0)
            strlcpy(buf, str, MIN(len, pos + 1));

          strlcat(buf, addstr, len);

          if(pos >= 0)
            strlcat(buf, str + pos, len);

          set(string, MUIA_String_Contents, buf);

          free(buf);
        }
      }
      else
        set(string, MUIA_String_Contents, addstr);
    }
  }

  LEAVE();
}
MakeStaticHook(PO_HandleVarHook, PO_HandleVarFunc);

///
/// MakeVarPop
//  Creates a popup list containing variables and descriptions for phrases etc.
Object *MakeVarPop(Object **string, Object **popButton, Object **list, const int mode, const int size, const char *shortcut)
{
  Object *po;

  ENTER();

  if((po = PopobjectObject,

    MUIA_Popstring_String, *string = MakeString(size, shortcut),
    MUIA_Popstring_Button, *popButton = PopButton(MUII_PopUp),
    MUIA_Popobject_ObjStrHook, &PO_HandleVarHook,
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, NListviewObject,
      MUIA_FixHeightTxt, "\n\n\n\n\n\n\n\n",
      MUIA_NListview_Horiz_ScrollBar, MUIV_NListview_HSB_None,
      MUIA_NListview_Vert_ScrollBar, MUIV_NListview_VSB_FullAuto,
      MUIA_NListview_NList, *list = PlaceholderPopupListObject,
        MUIA_PlaceholderPopupList_Mode, mode,
      End,
    End,

  End))
  {
    DoMethod(*list, MUIM_Notify, MUIA_NList_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime, po, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }

  RETURN(po);
  return po;
}

///
/// PO_MimeTypeListOpenHook
//  Sets the popup listview accordingly to the string gadget
HOOKPROTO(PO_MimeTypeListOpenFunc, BOOL, Object *listview, Object *str)
{
  BOOL isConfigWindow = (BOOL)(hook->h_Data != NULL);
  char *s;
  Object *list;

  ENTER();

  if((s = (char *)xget(str, MUIA_String_Contents)) != NULL &&
     (list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    int i;

    // we build the list totally from ground up.
    DoMethod(list, MUIM_List_Clear);

    // populate the list with the user's own defined MIME types but only if the source
    // string isn't the one in the YAM config window.
    if(isConfigWindow == FALSE)
    {
      struct MimeTypeNode *mt;

      IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
      {
        DoMethod(list, MUIM_List_InsertSingle, mt->ContentType, MUIV_List_Insert_Sorted);
      }
    }

    // populate the MUI list with our internal MIME types but check that
    // we don't add duplicate names
    for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
    {
      BOOL duplicateFound = FALSE;

      if(isConfigWindow == FALSE)
      {
        struct MimeTypeNode *mt;

        IterateList(&C->mimeTypeList, struct MimeTypeNode *, mt)
        {
          if(stricmp(mt->ContentType, IntMimeTypeArray[i].ContentType) == 0)
          {
            duplicateFound = TRUE;
            break;
          }
        }
      }

      if(duplicateFound == FALSE)
        DoMethod(list, MUIM_List_InsertSingle, IntMimeTypeArray[i].ContentType, MUIV_List_Insert_Sorted);
    }

    // make sure to make the current entry active
    for(i=0;;i++)
    {
      char *c;

      DoMethod(list, MUIM_List_GetEntry, i, &c);
      if(c == NULL || s[0] == '\0')
      {
        set(list, MUIA_List_Active, MUIV_List_Active_Off);
        break;
      }
      else if(!stricmp(c, s))
      {
        set(list, MUIA_List_Active, i);
        break;
      }
    }
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHook(PO_MimeTypeListOpenHook, PO_MimeTypeListOpenFunc);

///
/// PO_MimeTypeListCloseHook
//  Pastes an entry from the popup listview into string gadget
HOOKPROTO(PO_MimeTypeListCloseFunc, void, Object *listview, Object *str)
{
  struct MimeTypeCloseObjects *objs = (struct MimeTypeCloseObjects *)hook->h_Data;
  Object *list;

  ENTER();

  if((list = (Object *)xget(listview, MUIA_Listview_List)) != NULL)
  {
    char *entry = NULL;

    DoMethod(list, MUIM_List_GetEntry, MUIV_List_GetEntry_Active, &entry);
    if(entry != NULL)
    {
      set(str, MUIA_String_Contents, entry);

      // in case that this close function is used with the
      // string gadget in the YAM config window we have to do a deeper search
      // as we also want to set the file extension and description gadgets
      if(objs != NULL)
      {
        int i;

        for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
        {
          struct IntMimeType *mt = (struct IntMimeType *)&IntMimeTypeArray[i];

          if(stricmp(mt->ContentType, entry) == 0)
          {
            // we also set the file extension
            if(mt->Extension != NULL)
              set(objs->extension, MUIA_String_Contents, mt->Extension);

            // we also set the mime description
            set(objs->description, MUIA_String_Contents, tr(mt->Description));

            break;
          }
        }
      }
    }
  }

  LEAVE();
}
MakeHook(PO_MimeTypeListCloseHook, PO_MimeTypeListCloseFunc);

///
/// MakeMimeTypePop
//  Creates a popup list of available internal MIME types
Object *MakeMimeTypePop(Object **string, const char *desc)
{
  Object *lv;
  Object *po;
  Object *bt;

  ENTER();

  if((po = PopobjectObject,

    MUIA_Popstring_String, *string = BetterStringObject,
      StringFrame,
      MUIA_String_Accept,      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-/#?*",
      MUIA_String_MaxLen,      SIZE_CTYPE,
      MUIA_ControlChar,        ShortCut(desc),
      MUIA_String_AdvanceOnCR, TRUE,
      MUIA_CycleChain,         TRUE,
    End,
    MUIA_Popstring_Button, bt = PopButton(MUII_PopUp),
    MUIA_Popobject_WindowHook, &PO_WindowHook,
    MUIA_Popobject_Object, lv = ListviewObject,
      MUIA_Listview_ScrollerPos, MUIV_Listview_ScrollerPos_Right,
      MUIA_Listview_List, ListObject,
        InputListFrame,
        MUIA_List_AutoVisible, TRUE,
        MUIA_List_ConstructHook, MUIV_List_ConstructHook_String,
        MUIA_List_DestructHook,  MUIV_List_DestructHook_String,
      End,
    End,

  End))
  {
    set(bt, MUIA_CycleChain,TRUE);
    DoMethod(lv, MUIM_Notify, MUIA_Listview_DoubleClick, TRUE, po, 2, MUIM_Popstring_Close, TRUE);
    DoMethod(*string, MUIM_Notify, MUIA_Disabled, MUIV_EveryTime, po, 3, MUIM_Set, MUIA_Disabled, MUIV_TriggerValue);
  }
  else
    *string = NULL;

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

  if(label != NULL && (ptr = strchr(label, '_')) != NULL)
    scut = (char)ToLower((ULONG)(*++ptr));

  RETURN(scut);
  return scut;
}

///
/// FilereqStartFunc
//  Will be executed as soon as the user wants to popup a file requester
//  for selecting files
HOOKPROTONO(FilereqStartFunc, BOOL, struct TagItem *tags)
{
  Object *strObj = (Object *)hook->h_Data;
  char *str;

  ENTER();

  str = (char *)xget(strObj, MUIA_String_Contents);
  if(str != NULL && str[0] != '\0')
  {
    int i=0;
    static char buf[SIZE_PATHFILE];
    char *p;

    // make sure the string is unquoted.
    strlcpy(buf, str, sizeof(buf));
    UnquoteString(buf, FALSE);

    if((p = PathPart(buf)))
    {
      static char drawer[SIZE_PATHFILE];

      strlcpy(drawer, buf, MIN(sizeof(drawer), (unsigned int)(p - buf + 1)));

      tags[i].ti_Tag = ASLFR_InitialDrawer;
      tags[i].ti_Data= (ULONG)drawer;
      i++;
    }

    tags[i].ti_Tag = ASLFR_InitialFile;
    tags[i].ti_Data = (ULONG)FilePart(buf);
    i++;

    tags[i].ti_Tag = TAG_DONE;
  }

  RETURN(TRUE);
  return TRUE;
}
MakeHook(FilereqStartHook, FilereqStartFunc);

///
/// FilereqStopFunc
//  Will be executed as soon as the user selected a file
HOOKPROTONO(FilereqStopFunc, void, struct FileRequester *fileReq)
{
  Object *strObj = (Object *)hook->h_Data;

  ENTER();

  // check if a file was selected or not
  if(fileReq->fr_File != NULL && fileReq->fr_File[0] != '\0')
  {
    char buf[SIZE_PATHFILE];

    AddPath(buf, fileReq->fr_Drawer, fileReq->fr_File, sizeof(buf));

    // check if there is any space in the path
    if(strchr(buf, ' ') != NULL)
    {
      int len = strlen(buf);

      memmove(&buf[1], buf, len+1);
      buf[0] = '"';
      buf[len+1] = '"';
      buf[len+2] = '\0';
    }

    set(strObj, MUIA_String_Contents, buf);
  }

  LEAVE();
}
MakeHook(FilereqStopHook, FilereqStopFunc);

///
/// GetMUIString
// copy the contents of a MUI string object to a string
void GetMUIString(char *s, Object *o, size_t len)
{
  char *c;

  ENTER();

  if((c = (char *)xget(o, MUIA_String_Contents)) != NULL)
    strlcpy(s, c, len);
  else
  {
    E(DBF_GUI, "NULL string contents of object %08lx", o);
    s[0] = '\0';
  }

  LEAVE();
}

///
/// GetMUIText
// copy the contents of a MUI text object to a string
void GetMUIText(char *s, Object *o, size_t len)
{
  char *c;

  ENTER();

  if((c = (char *)xget(o, MUIA_Text_Contents)) != NULL)
    strlcpy(s, c, len);
  else
  {
    E(DBF_GUI, "NULL text contents of object %08lx", o);
    s[0] = '\0';
  }

  LEAVE();
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

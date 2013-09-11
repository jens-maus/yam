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

 Superclass:  MUIC_NListtree
 Description: NListtree class of the addressbook

***************************************************************************/

#include "AddressBookListtree_cl.h"

#include <ctype.h>
#include <string.h>

#include <proto/dos.h>
#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"

#include "AVLTree.h"
#include "Config.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "mui/ImageArea.h"
#include "mui/MainMailListGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *listImage;
  ULONG sortBy;
  BOOL modified;
  struct AVL_Tree *avlTree;        // the address book as an AVL tree
  char dateStr[SIZE_SMALL];
  char aliasStr[SIZE_DEFAULT];
  char pattern[SIZE_PATTERN+1];
  struct MUI_EventHandlerNode eh;
};
*/

/* EXPORT
#define MUIV_AddressBookListtree_SortBy_Alias      0
#define MUIV_AddressBookListtree_SortBy_FirstName  1
#define MUIV_AddressBookListtree_SortBy_Coment     2
#define MUIV_AddressBookListtree_SortBy_Address    3
#define MUIV_AddressBookListtree_SortBy_Street     4
#define MUIV_AddressBookListtree_SortBy_City       5
#define MUIV_AddressBookListtree_SortBy_Country    6
#define MUIV_AddressBookListtree_SortBy_Phone      7
#define MUIV_AddressBookListtree_SortBy_Birthday   8
#define MUIV_AddressBookListtree_SortBy_PGPId      9
#define MUIV_AddressBookListtree_SortBy_LastName  10 // artificial column generated from the Name column

#define NUMBER_ABOOK_COLUMNS 9
*/

/* Private Functions */
/// ComparePersons
// compare two person entries in the AVL tree
static int ComparePersons(const void *p1, const void *p2)
{
  const struct Person *entry1 = (const struct Person *)p1;
  const struct Person *entry2 = (const struct Person *)p2;

  return Stricmp(entry1->Address, entry2->Address);
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct AVL_Tree *avlTree;

  ENTER();

  if((avlTree = CreateAVLTree(ComparePersons)) != NULL)
  {
    if((obj = DoSuperNew(cl, obj,

      InputListFrame,
      MUIA_NListtree_DragDropSort,     TRUE,
      MUIA_NListtree_Title,            TRUE,
      MUIA_NListtree_EmptyNodes,       TRUE,
      MUIA_NListtree_MultiSelect,      MUIV_NListtree_MultiSelect_Default,
      MUIA_NList_AutoVisible,          TRUE,
      MUIA_NList_TitleClick,           TRUE,
      MUIA_NList_TitleClick2,          TRUE,
      MUIA_NList_TitleSeparator,       TRUE,
      MUIA_NList_DefaultObjectOnClick, FALSE,
      MUIA_Font,                       C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,

      TAG_MORE, inittags(msg))) != NULL)
    {
      GETDATA;

      // prepare the group image
      data->listImage = MakeImageObject("status_group", G->theme.statusImages[SI_GROUP]);
      DoMethod(obj, MUIM_NList_UseImage, data->listImage, 0, MUIF_NONE);

      DoMethod(obj, METHOD(MakeFormat));

      DoMethod(obj, MUIM_Notify, MUIA_NList_TitleClick, MUIV_EveryTime, MUIV_Notify_Self, 2, METHOD(SortBy), MUIV_TriggerValue);

      data->avlTree = avlTree;
    }
    else
      DeleteAVLTree(avlTree);
  }
  else
    obj = NULL;

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;

  // make sure that we free our group image
  DoMethod(obj, MUIM_NList_UseImage, NULL, 0, MUIF_NONE);
  MUI_DisposeObject(data->listImage);
  data->listImage = NULL;

  // delete the AVL tree and erase the pointer, because disposing
  // an NListtree object will invoke MUIM_NListtree_Clear during
  // the disposal
  DeleteAVLTree(data->avlTree);
  data->avlTree = NULL;

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags, *tag;
  IPTR result;

  ENTER();

  tags = inittags(msg);
  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Modified):
      {
        data->modified = (tag->ti_Data != 0) ? TRUE : FALSE;

        // make the superMethod call ignore those tags
        tag->ti_Tag = TAG_IGNORE;
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
// get some stuff of our instance data
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;
  IPTR result = FALSE;

  ENTER();

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Modified): *store = (ULONG)data->modified; result = TRUE; break;
  }

  if(result == FALSE)
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Setup)
OVERLOAD(MUIM_Setup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if((result = DoSuperMethodA(cl, obj, msg)))
  {
    data->eh.ehn_Class  = cl;
    data->eh.ehn_Object = obj;
    data->eh.ehn_Events = IDCMP_RAWKEY;
    data->eh.ehn_Flags  = MUI_EHF_GUIMODE;
    data->eh.ehn_Priority = -1;

    if(_win(obj) != NULL)
      DoMethod(_win(obj), MUIM_Window_AddEventHandler, &data->eh);
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_Cleanup)
OVERLOAD(MUIM_Cleanup)
{
  GETDATA;
  IPTR result;

  ENTER();

  if(_win(obj) != NULL)
    DoMethod(_win(obj), MUIM_Window_RemEventHandler, &data->eh);

  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_HandleEvent)
OVERLOAD(MUIM_HandleEvent)
{
  struct MUIP_HandleEvent *mhe = (struct MUIP_HandleEvent *)msg;
  IPTR result = 0;

  ENTER();

  if(mhe->imsg->Class == IDCMP_RAWKEY)
  {
    // check for DEL key without CAPS LOCK
    if(mhe->imsg->Code == 0x46 && isFlagClear(mhe->imsg->Qualifier, IEQUALIFIER_CAPSLOCK))
    {
      DoMethod(obj, METHOD(DeleteEntry));
      // eat the key press in any case
      result = MUI_EventHandlerRC_Eat;
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NListtree_Construct)
OVERLOAD(MUIM_NListtree_Construct)
{
  struct MUIP_NListtree_Construct *ncm = (struct MUIP_NListtree_Construct *)msg;
  struct ABEntry *addr = (struct ABEntry *)ncm->UserData;
  struct ABEntry *entry;
  GETDATA;

  ENTER();

  if((entry = memdup(addr, sizeof(*addr))) != NULL)
  {
    // accept real users only
    if(entry->Type == AET_USER)
    {
      // insert the person part in the AVL tree
      InsertInAVLTree(data->avlTree, &entry->Address[0]);
    }
    else if(addr->Members != NULL)
    {
      // clone the member list of groups
      if((entry->Members = strdup(addr->Members)) == NULL)
      {
        // if strdup() failed then we let the whole function fail
        free(entry);
        entry = NULL;
      }
    }
  }

  RETURN(entry);
  return (IPTR)entry;
}

///
/// OVERLOAD(MUIM_NListtree_Destruct)
OVERLOAD(MUIM_NListtree_Destruct)
{
  struct MUIP_NListtree_Destruct *ndm = (struct MUIP_NListtree_Destruct *)msg;
  struct ABEntry *entry = (struct ABEntry *)ndm->UserData;
  GETDATA;

  ENTER();

  if(entry != NULL)
  {
    // remove users only
    if(entry->Type == AET_USER && data->avlTree != NULL)
    {
      // remove the person part from the AVL tree
      RemoveFromAVLTree(data->avlTree, &entry->Address[0]);
    }

    free(entry->Members);
    free(entry);
  }

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_NListtree_Compare)
OVERLOAD(MUIM_NListtree_Compare)
{
  GETDATA;
  struct MUIP_NListtree_Compare *ncm = (struct MUIP_NListtree_Compare *)msg;
  struct ABEntry *ab1 = (struct ABEntry *)ncm->TreeNode1->tn_User;
  struct ABEntry *ab2 = (struct ABEntry *)ncm->TreeNode2->tn_User;
  LONG cmp = 0;

  ENTER();

  switch(data->sortBy)
  {
    case MUIV_AddressBookListtree_SortBy_Alias:
    {
      cmp = 0;
    }
    break;

    case MUIV_AddressBookListtree_SortBy_FirstName:
    {
      cmp = Stricmp(ab1->RealName, ab2->RealName);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_Coment:
    {
      cmp = Stricmp(ab1->Comment, ab2->Comment);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_Street:
    {
      cmp = Stricmp(ab1->Street, ab2->Street);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_City:
    {
      cmp = Stricmp(ab1->City, ab2->City);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_Country:
    {
      cmp = Stricmp(ab1->Country, ab2->Country);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_Phone:
    {
      cmp = Stricmp(ab1->Phone, ab2->Phone);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_Birthday:
    {
      cmp = ab1->BirthDay - ab2->BirthDay;
    }
    break;

    case MUIV_AddressBookListtree_SortBy_PGPId:
    {
      cmp = Stricmp(ab1->PGPId, ab2->PGPId);
    }
    break;

    case MUIV_AddressBookListtree_SortBy_LastName:
    {
      char *n1, *n2;

      if((n1 = strrchr(ab1->RealName,' ')) == NULL)
        n1 = ab1->RealName;
      if((n2 = strrchr(ab2->RealName,' ')) == NULL)
        n2 = ab2->RealName;
      cmp = Stricmp(n1, n2);
    }
    break;
  }

  if(cmp == 0)
    cmp = Stricmp(ab1->Alias, ab2->Alias);

  RETURN(cmp);
  return cmp;
}

///
/// OVERLOAD(MUIM_NListtree_Display)
OVERLOAD(MUIM_NListtree_Display)
{
  struct MUIP_NListtree_Display *ndm = (struct MUIP_NListtree_Display *)msg;

  ENTER();

  if(ndm->TreeNode != NULL)
  {
    GETDATA;
    struct ABEntry *entry = (struct ABEntry *)ndm->TreeNode->tn_User;

    AB_ExpandBD(entry->BirthDay, data->dateStr, sizeof(data->dateStr));

    ndm->Array[0]  = entry->Alias;
    ndm->Array[1]  = entry->RealName;
    ndm->Array[2]  = entry->Comment;
    ndm->Array[3]  = entry->Address;
    ndm->Array[4]  = entry->Street;
    ndm->Array[5]  = entry->City;
    ndm->Array[6]  = entry->Country;
    ndm->Array[7]  = entry->Phone;
    ndm->Array[8]  = data->dateStr;
    ndm->Array[9]  = entry->PGPId;
    ndm->Array[10] = entry->Homepage;

    switch(entry->Type)
    {
       case AET_LIST:
       {
         snprintf(data->aliasStr, sizeof(data->aliasStr), "\033o[0]%s", entry->Alias);
         ndm->Array[0] = data->aliasStr;
       }
       break;

       case AET_GROUP:
       {
         ndm->Preparse[0] = (char *)MUIX_B;
         ndm->Preparse[2] = (char *)MUIX_B;
       }
       break;

       default:
         // nothing
       break;
    }
  }
  else
  {
    ndm->Array[0]  = (STRPTR)tr(MSG_AB_TitleAlias);
    ndm->Array[1]  = (STRPTR)tr(MSG_AB_TitleName);
    ndm->Array[2]  = (STRPTR)tr(MSG_AB_TitleDescription);
    ndm->Array[3]  = (STRPTR)tr(MSG_AB_TitleAddress);
    ndm->Array[4]  = (STRPTR)tr(MSG_AB_TitleStreet);
    ndm->Array[5]  = (STRPTR)tr(MSG_AB_TitleCity);
    ndm->Array[6]  = (STRPTR)tr(MSG_AB_TitleCountry);
    ndm->Array[7]  = (STRPTR)tr(MSG_AB_TitlePhone);
    ndm->Array[8]  = (STRPTR)tr(MSG_AB_TitleBirthDate);
    ndm->Array[9]  = (STRPTR)tr(MSG_AB_TitlePGPId);
    ndm->Array[10] = (STRPTR)tr(MSG_AB_TitleHomepage);
  }

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_DragQuery)
OVERLOAD(MUIM_DragQuery)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
    return MUIV_DragQuery_Accept;

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragQuery *d = (struct MUIP_DragQuery *)msg;

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    struct MailList *mlist;

    if((mlist = MA_CreateMarkedList(d->obj, FALSE)) != NULL)
    {
      MA_GetAddress(mlist);
      DeleteMailList(mlist);
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_NListtree_DropType)
OVERLOAD(MUIM_NListtree_DropType)
{
  struct MUIP_NListtree_DropType *dt = (struct MUIP_NListtree_DropType *)msg;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  // get the current drop target
  if((tn = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget)) != NULL)
  {
    struct ABEntry *entry;

    if((entry = (struct ABEntry *)tn->tn_User) != NULL)
    {
      // If we drag an ABEntry on another ABEntry we abort the
      // DragReport immediately because we want to support drag operations
      // between ABEntry elements and groups is allowed
      if(*dt->Type == MUIV_NListtree_DropType_Onto && entry->Type != AET_GROUP)
         *dt->Type = MUIV_NListtree_DropType_None;
    }
    else
      *dt->Type = MUIV_NListtree_DropType_None;
  }
  else
    *dt->Type = MUIV_NListtree_DropType_None;

  RETURN(0);
  return 0;
}

///

/* Public Methods */
/// DECLARE(SortBy)
DECLARE(SortBy) // ULONG sortBy;
{
  GETDATA;

  ENTER();

  data->sortBy = msg->sortBy;
  DoMethod(obj, MUIM_NListtree_Sort, MUIV_NListtree_Sort_ListNode_Root, MUIV_NListtree_Sort_Flag_RecursiveAll);
  data->modified = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(FindPerson)
DECLARE(FindPerson) // struct Person *person
{
  GETDATA;
  struct Person *result;

  ENTER();

  result = (struct Person *)FindInAVLTree(data->avlTree, msg->person);

  RETURN(result);
  return (IPTR)result;
}

///
/// DECLARE(MakeFormat)
//  Creates format definition for address book listtree
DECLARE(MakeFormat)
{
  int i;
  char format[SIZE_LARGE];
  BOOL first = TRUE;

  ENTER();

  format[0] = '\0';

  // start at zero here, otherwise the tree column will be missing
  for(i = 0; i < NUMBER_ABOOK_COLUMNS; i++)
  {
    if(isFlagSet(C->AddrbookCols, (1<<i)))
    {
      int p;

      if(first)
        first = FALSE;
      else
        strlcat(format, " BAR,", sizeof(format));

      p = strlen(format);

      snprintf(&format[p], sizeof(format)-p, "COL=%d W=-1", i);
    }
  }

  set(obj, MUIA_NListtree_Format, format);

  RETURN(0);
  return 0;
}

///
/// DECLARE(FoldTree)
// (un)fold the complete tree
DECLARE(FoldTree) // ULONG unfold
{
  ENTER();

  if(msg->unfold == TRUE)
    DoMethod(obj, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Root, MUIV_NListtree_Open_TreeNode_All, MUIF_NONE);
  else
    DoMethod(obj, MUIM_NListtree_Close, MUIV_NListtree_Close_ListNode_Root, MUIV_NListtree_Close_TreeNode_All, MUIF_NONE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ClearTree)
// clears entire address book
DECLARE(ClearTree)
{
  ENTER();

  DoMethod(obj, MUIM_NListtree_Clear, NULL, MUIF_NONE);
  set(obj, MUIA_AddressBookListtree_Modified, TRUE);

  RETURN(0);
  return 0;
}

///
/// DECLARE(AddEntry)
// add a new entry to the address book
DECLARE(AddEntry) // ULONG type
{
  ENTER();

  EA_Init(msg->type, NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(EditEntry)
// edit the selected address book entry
DECLARE(EditEntry)
{
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_Active)) != NULL)
  {
    struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
    int winnum;

    if((winnum = EA_Init(ab->Type, ab)) >= 0)
      EA_Setup(winnum, ab);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DuplicateEntry)
// duplicate the selected address book entry
DECLARE(DuplicateEntry)
{
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_Active)) != NULL)
  {
    struct ABEntry *ab = (struct ABEntry *)(tn->tn_User);
    int winnum;

    if((winnum = EA_Init(ab->Type, NULL)) >= 0)
    {
      char buf[SIZE_NAME];
      int len;

      EA_Setup(winnum, ab);
      strlcpy(buf, ab->Alias, sizeof(buf));
      if((len = strlen(buf)) > 0)
      {
        if(isdigit(buf[len - 1]))
          buf[len - 1]++;
        else if(len < SIZE_NAME - 1)
          strlcat(buf, "2", sizeof(buf));
        else
          buf[len - 1] = '2';

        setstring(G->EA[winnum]->GUI.ST_ALIAS, buf);
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(FindEntry)
// searches address book
DECLARE(FindEntry)
{
  GETDATA;

  ENTER();

  if(StringRequest(data->pattern, SIZE_PATTERN, tr(MSG_AB_FindEntry), tr(MSG_AB_FindEntryReq), tr(MSG_AB_StartSearch), NULL, tr(MSG_Cancel), FALSE, _win(obj)) != 0)
  {
    char searchPattern[SIZE_PATTERN+5];

    snprintf(searchPattern, sizeof(searchPattern), "#?%s#?", data->pattern);

    if(AB_FindEntry(searchPattern, ABF_USER, NULL) == 0)
      MUI_Request(_app(obj), _win(obj), MUIF_NONE, tr(MSG_AB_FindEntry), tr(MSG_OkayReq), tr(MSG_AB_NoneFound));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(DeleteEntry)
// delete the selected address book entry
DECLARE(DeleteEntry)
{
  ENTER();

  DoMethod(obj, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_Selected, MUIF_NONE);
  set(obj, MUIA_AddressBookListtree_Modified, TRUE);

  RETURN(0);
  return 0;
}

///

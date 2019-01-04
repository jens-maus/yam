/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

#include "mui/AddressBookEditWindow.h"
#include "mui/ImageArea.h"
#include "mui/MainMailListGroup.h"

#include "Config.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *listImage;
  ULONG sortBy;
  BOOL selfReorder;
  struct MUI_NListtree_TreeNode *activeTN;
  char dateStr[SIZE_SMALL];
  char aliasStr[SIZE_DEFAULT];
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

#define MUIV_AddressBookListtree_Search_User          0
#define MUIV_AddressBookListtree_Search_RX            1
#define MUIV_AddressBookListtree_Search_RXName        2
#define MUIV_AddressBookListtree_Search_RXAddress     3
#define MUIV_AddressBookListtree_Search_RXNameAddress 4
*/

/* Private Functions */
/// BuildTreeEntry
struct BuildTreeStuff
{
  Object *obj;
  struct MUI_NListtree_TreeNode *parent[8];
  LONG nestLevel;
};

static BOOL BuildTreeEntry(const struct ABookNode *abn, ULONG flags, void *userData)
{
  struct BuildTreeStuff *stuff = (struct BuildTreeStuff *)userData;
  BOOL success = TRUE;

  ENTER();

  switch(abn->type)
  {
    case ABNT_USER:
    {
      if((APTR)DoMethod(stuff->obj, MUIM_NListtree_Insert, abn->Alias[0] != '\0' ? abn->Alias : abn->RealName, abn, stuff->parent[stuff->nestLevel], MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE) == NULL)
        success = FALSE;
    }
    break;

    case ABNT_GROUP:
    {
      if(isFlagSet(flags, IABF_FIRST_GROUP_VISIT))
      {
        if((stuff->parent[stuff->nestLevel+1] = (struct MUI_NListtree_TreeNode *)DoMethod(stuff->obj, MUIM_NListtree_Insert, abn->Alias, abn, stuff->parent[stuff->nestLevel], MUIV_NListtree_Insert_PrevNode_Tail, TNF_LIST)) == NULL)
          success = FALSE;
        else
          stuff->nestLevel++;
      }
      else
      {
        stuff->nestLevel--;
      }
    }
    break;

    case ABNT_LIST:
    {
      if((APTR)DoMethod(stuff->obj, MUIM_NListtree_Insert, abn->Alias, abn, stuff->parent[stuff->nestLevel], MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE) == NULL)
        success = FALSE;
    }
    break;
  }

  RETURN(success);
  return success;
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

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
    DoMethod(obj, MUIM_Notify, MUIA_NListtree_Active, MUIV_EveryTime, MUIV_Notify_Self, 3, MUIM_Set, ATTR(ActiveTreeNode), MUIV_TriggerValue);

    data->activeTN = (struct MUI_NListtree_TreeNode *)MUIV_NListtree_Active_Off;
    data->selfReorder = TRUE;
  }

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
      case ATTR(ActiveEntry):
      {
        struct ABookNode *abn = (struct ABookNode *)tag->ti_Data;

        data->activeTN = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_FindName, MUIV_NListtree_FindName_ListNode_Root, abn->Alias, MUIF_NONE);
        if(data->activeTN != NULL)
        {
          DoMethod(obj, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, data->activeTN);
          nnset(obj, MUIA_NListtree_Active, data->activeTN);
        }
      }
      break;

      case ATTR(ActiveTreeNode):
      {
        data->activeTN = (struct MUI_NListtree_TreeNode *)tag->ti_Data;
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
    case ATTR(DeleteEntryRequest): *store = TRUE; result = TRUE; break;
    case ATTR(ActiveGroup):
    {
      *store = (IPTR)NULL;
      if(data->activeTN != (struct MUI_NListtree_TreeNode *)MUIV_NListtree_ActiveList_Off)
      {
        struct MUI_NListtree_TreeNode *tn;

        tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, data->activeTN, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
        if(tn != NULL)
          *store = (IPTR)tn->tn_User;
      }
    }
    break;
    case ATTR(ActiveEntry):        *store = (IPTR)(data->activeTN != NULL ? data->activeTN->tn_User : NULL); result = TRUE; break;
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
      set(obj, ATTR(DeleteEntryRequest), TRUE);
      // eat the key press in any case
      result = MUI_EventHandlerRC_Eat;
    }
  }

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_NListtree_Compare)
OVERLOAD(MUIM_NListtree_Compare)
{
  GETDATA;
  struct MUIP_NListtree_Compare *ncm = (struct MUIP_NListtree_Compare *)msg;
  struct ABookNode *ab1 = (struct ABookNode *)ncm->TreeNode1->tn_User;
  struct ABookNode *ab2 = (struct ABookNode *)ncm->TreeNode2->tn_User;
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
      cmp = ab1->Birthday - ab2->Birthday;
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
    struct ABookNode *entry = (struct ABookNode *)ndm->TreeNode->tn_User;

    BirthdayToString(entry->Birthday, data->dateStr, sizeof(data->dateStr));

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

    switch(entry->type)
    {
      case ABNT_GROUP:
      {
        ndm->Preparse[0] = (char *)MUIX_B;
        ndm->Preparse[2] = (char *)MUIX_B;
      }
      break;

      case ABNT_LIST:
      {
        snprintf(data->aliasStr, sizeof(data->aliasStr), "\033o[0]%s", entry->Alias);
        ndm->Array[0] = data->aliasStr;
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
  ULONG result;

  ENTER();

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
    result = MUIV_DragQuery_Accept;
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(MUIM_DragDrop)
OVERLOAD(MUIM_DragDrop)
{
  struct MUIP_DragDrop *d = (struct MUIP_DragDrop *)msg;
  ULONG result = 0;

  ENTER();

  if(DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_IsMailList, d->obj) == TRUE)
  {
    struct MailList *mlist;

    if((mlist = MA_CreateMarkedList(d->obj, FALSE)) != NULL)
    {
      MA_GetAddress(mlist, (struct MUI_NListtree_TreeNode *)xget(obj, MUIA_NListtree_DropTarget),
                           xget(obj, MUIA_NListtree_DropType));

      DeleteMailList(mlist);
    }
  }
  else
    result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
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
    struct ABookNode *entry;

    if((entry = (struct ABookNode *)tn->tn_User) != NULL)
    {
      // If we drag an ABookNode on another ABookNode we abort the
      // DragReport immediately because we want to support drag operations
      // between ABookNode elements and groups is allowed
      if(*dt->Type == MUIV_NListtree_DropType_Onto && entry->type != ABNT_GROUP)
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
/// OVERLOAD(MUIM_NListtree_Insert)
OVERLOAD(MUIM_NListtree_Insert)
{
  GETDATA;
  struct MUIP_NListtree_Insert *mi = (struct MUIP_NListtree_Insert *)msg;
  struct MUI_NListtree_TreeNode *thisTN;

  ENTER();

  // first let the list tree class do the actual insertion of the tree nodes
  if((thisTN = (struct MUI_NListtree_TreeNode *)DoSuperMethodA(cl, obj, msg)) != NULL)
  {
    // reorder the address book only if we are explicitly told to do so
    if(data->selfReorder == TRUE)
    {
      struct ABookNode *groupABN;
      struct ABookNode *thisABN;
      struct MUI_NListtree_TreeNode *predTN;
      struct ABookNode *predABN;

      groupABN = (mi->ListNode != NULL && mi->ListNode->tn_User != NULL) ? (struct ABookNode *)mi->ListNode->tn_User : &G->abook.rootGroup;
      thisABN = (struct ABookNode *)thisTN->tn_User;

      // ideally we could use the mi->TreeNode pointer directly, but this might
      // be one of the special MUIV_#? values. Hence we better obtain the predecessor
      // node in the traditional way
      predTN = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, thisTN, MUIV_NListtree_GetEntry_Position_Previous, MUIF_NONE);
      predABN = (predTN != NULL) ? (struct ABookNode *)predTN->tn_User : NULL;

      // finally insert the node into the address book
      D(DBF_ABOOK, "insert entry '%s' behind entry '%s', group '%s'", thisABN->Alias, predABN != NULL ? predABN->Alias : "<head>", groupABN->Alias);
      AddABookNode(groupABN, thisABN, predABN);
      G->abook.modified = TRUE;
    }
  }

  RETURN(thisTN);
  return (IPTR)thisTN;
}

///
/// OVERLOAD(MUIM_NListtree_Move)
OVERLOAD(MUIM_NListtree_Move)
{
  IPTR result;
  struct MUIP_NListtree_Move *mv = (struct MUIP_NListtree_Move *)msg;
  struct ABookNode *groupABN;
  struct ABookNode *thisABN;
  struct MUI_NListtree_TreeNode *predTN;
  struct ABookNode *predABN;

  ENTER();

  // first let the list tree class do the actual movement of the tree nodes
  result = DoSuperMethodA(cl, obj, msg);

  groupABN = (mv->NewListNode->tn_User != NULL) ? (struct ABookNode *)mv->NewListNode->tn_User : &G->abook.rootGroup;
  thisABN = (struct ABookNode *)mv->OldTreeNode->tn_User;

  // ideally we could use the mv->NewTreeNode pointer directly, but this might
  // be one of the special MUIV_#? values. Hence we better obtain the predecessor
  // node in the traditional way
  predTN = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, mv->OldTreeNode, MUIV_NListtree_GetEntry_Position_Previous, MUIF_NONE);
  predABN = (predTN != NULL) ? (struct ABookNode *)predTN->tn_User : NULL;

  // finally move the node within the address book
  D(DBF_ABOOK, "move entry '%s' behind entry '%s', group '%s'", thisABN->Alias, predABN != NULL ? predABN->Alias : "<head>", groupABN->Alias);
  MoveABookNode(groupABN, thisABN, predABN);
  G->abook.modified = TRUE;

  RETURN(result);
  return result;
}

///

/* Public Methods */
/// DECLARE(BuildTree)
// (re)build the NListtree from the global address book
DECLARE(BuildTree)
{
  GETDATA;
  struct BuildTreeStuff stuff;

  ENTER();

  data->selfReorder = FALSE;

  set(obj, MUIA_NListtree_Quiet, TRUE);
  DoMethod(obj, MUIM_NListtree_Clear);
  stuff.obj = obj;
  stuff.parent[0] = MUIV_NListtree_Insert_ListNode_Root;
  stuff.nestLevel = 0;
  IterateABook(&G->abook, IABF_VISIT_GROUPS_TWICE, BuildTreeEntry, &stuff);
  set(obj, MUIA_NListtree_Quiet, FALSE);

  data->selfReorder = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(BuildABook)
// (re)build the global address book from the NListtree
DECLARE(BuildABook)
{
  struct ABook tempABook;
  struct MUI_NListtree_TreeNode *tn;
  struct MUI_NListtree_TreeNode *parentTN[8];
  struct ABookNode *groupABN[8];
  struct ABookNode *afterThisABN[8];
  ULONG nestLevel;
  ULONG i;

  ENTER();

  // we need a temporary address book, because me move all entries
  // from the global book to the temporary one
  InitABook(&tempABook, NULL);

  parentTN[0] = MUIV_NListtree_GetEntry_ListNode_Root;
  groupABN[0] = &tempABook.rootGroup;
  afterThisABN[0] = NULL;
  nestLevel = 0;
  i = 0;
  while((tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
  {
    struct ABookNode *abn = (struct ABookNode *)tn->tn_User;

    if(i == 0)
    {
      // if this is the first iteration we must obtain the parent tree node
      // of the current one to make sure we have a valid pointer to compare
      // instead of the artificial internal number.
      parentTN[0] = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
    }

    if(abn->type == ABNT_GROUP)
    {
      // move group nodes before bumping the nesting level
      MoveABookNode(groupABN[nestLevel], abn, afterThisABN[nestLevel]);
      afterThisABN[nestLevel] = abn;

      // bump the nesting level and remember the new group details
      nestLevel++;
      parentTN[nestLevel] = tn;
      groupABN[nestLevel] = abn;
      afterThisABN[nestLevel] = NULL;
    }
    else
    {
      // check if our parent treenode changed
      struct MUI_NListtree_TreeNode *parent;

      parent = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
      if(parent != parentTN[nestLevel] && nestLevel != 0)
      {
        // the parent treenode has changed, so go back until we find the correct treenode
        do
        {
          nestLevel--;
        }
        while(nestLevel != 0 && parent != parentTN[nestLevel]);

        // replace the artificial MUIV_#? root pointer by the true root pointer
        if(nestLevel == 0 && parentTN[0] == NULL)
          parentTN[0] = parent;
      }

      // move user and list nodes after bumping the nesting level
      MoveABookNode(groupABN[nestLevel], abn, afterThisABN[nestLevel]);
      afterThisABN[nestLevel] = abn;
    }

    i++;
  }

  // finally move all nodes back to the global address book and mark it as modified
  MoveABookNodes(&G->abook, &tempABook);
  G->abook.modified = TRUE;

  RETURN(0);
  return 0;
}

///
/// DECLARE(SortBy)
DECLARE(SortBy) // ULONG sortBy
{
  GETDATA;

  ENTER();

  data->sortBy = msg->sortBy;
  DoMethod(obj, MUIM_NListtree_Sort, MUIV_NListtree_Sort_ListNode_Root, MUIV_NListtree_Sort_Flag_RecursiveAll);
  // rebuild the address book from the sorted listtree
  DoMethod(obj, METHOD(BuildABook));

  RETURN(0);
  return 0;
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

      if(first == TRUE)
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
/// DECLARE(IncrementalSearch)
// incrementally searches the address book node for a given pattern
DECLARE(IncrementalSearch) // const char *pattern, ULONG *iterator
{
  struct ABookNode *foundABN = NULL;
  struct MUI_NListtree_TreeNode *tn;
  ULONG i;

  ENTER();

  D(DBF_ABOOK, "searching for pattern '%s'", msg->pattern);

  i = *msg->iterator;

  do
  {
    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct ABookNode *abn = (struct ABookNode *)tn->tn_User;

      if(abn->type == ABNT_USER || abn->type == ABNT_LIST)
      {
        BOOL found;

        D(DBF_ABOOK, "match entry with alias '%s'", abn->Alias);

        found = MatchNoCase(abn->Alias, msg->pattern) ||
                MatchNoCase(abn->Comment, msg->pattern);
        if(found == FALSE && abn->type == ABNT_USER)
        {
          found = MatchNoCase(abn->RealName, msg->pattern) ||
                  MatchNoCase(abn->Address, msg->pattern)  ||
                  MatchNoCase(abn->Homepage, msg->pattern) ||
                  MatchNoCase(abn->Street, msg->pattern)   ||
                  MatchNoCase(abn->City, msg->pattern)     ||
                  MatchNoCase(abn->Country, msg->pattern)  ||
                  MatchNoCase(abn->Phone, msg->pattern);
        }

        if(found == TRUE)
        {
          D(DBF_ABOOK, "found pattern '%s' in entry with address '%s'", msg->pattern, abn->Address);

          DoMethod(obj, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);
          set(obj, MUIA_NListtree_Active, tn);

          foundABN = abn;
          // return the next to be examined index
          *msg->iterator = i+1;

          break;
        }
      }
    }

    i++;
  }
  while(tn != NULL);

  RETURN(foundABN);
  return (IPTR)foundABN;
}

///

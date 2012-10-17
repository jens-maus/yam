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

 Superclass:  MUIC_NListtree
 Description: NListtree class of the addressbook

***************************************************************************/

#include "AddrBookListtree_cl.h"

#include <string.h>

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_addressbookEntry.h"
#include "YAM_config.h"

#include "AVLTree.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"

#include "mui/ImageArea.h"
#include "mui/MainMailListGroup.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *listImage;
  struct AVL_Tree *avlTree;        // the address book as an AVL tree
  char dateStr[SIZE_SMALL];
  char aliasStr[SIZE_DEFAULT];
};
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
      MUIA_NList_DefaultObjectOnClick, FALSE,
      MUIA_Font,                       C->FixedFontList ? MUIV_NList_Font_Fixed : MUIV_NList_Font,

      TAG_MORE, inittags(msg))) != NULL)
    {
      GETDATA;

      // prepare the group image
      data->listImage = MakeImageObject("status_group", G->theme.statusImages[SI_GROUP]);
      DoMethod(obj, MUIM_NList_UseImage, data->listImage, 0, MUIF_NONE);

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

  LEAVE();
  return 0;
}

///
/// OVERLOAD(MUIM_NListtree_Compare)
OVERLOAD(MUIM_NListtree_Compare)
{
  struct MUIP_NListtree_Compare *ncm = (struct MUIP_NListtree_Compare *)msg;
  struct ABEntry *ab1 = (struct ABEntry *)ncm->TreeNode1->tn_User;
  struct ABEntry *ab2 = (struct ABEntry *)ncm->TreeNode2->tn_User;
  char *n1, *n2;
  int cmp = 0;

  switch(G->AB->SortBy)
  {
    case 1:
      if((n1 = strrchr(ab1->RealName,' ')) == NULL)
        n1 = ab1->RealName;
      if((n2 = strrchr(ab2->RealName,' ')) == NULL)
        n2 = ab2->RealName;
      cmp = Stricmp(n1, n2);
    break;

    case 2:
      cmp = Stricmp(ab1->RealName, ab2->RealName);
    break;

    case 3:
      cmp = Stricmp(ab1->Comment, ab2->Comment);
    break;

    case 4:
      cmp = Stricmp(ab1->Address, ab2->Address);
    break;
  }

  if(cmp == 0)
    cmp = Stricmp(ab1->Alias, ab2->Alias);

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

  LEAVE();
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

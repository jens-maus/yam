/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <proto/muimaster.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_addressbookEntry.h"

#include "AVLTree.h"
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
/// OVERLOAD(MUIM_NListtree_Clear)
OVERLOAD(MUIM_NListtree_Clear)
{
  GETDATA;

  if(data->avlTree != NULL)
    ClearAVLTree(data->avlTree);

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(MUIM_NListtree_Insert)
OVERLOAD(MUIM_NListtree_Insert)
{
  struct MUI_NListtree_TreeNode *tn;

  // update the AVL tree on every insertion
  if((tn = (struct MUI_NListtree_TreeNode *)DoSuperMethodA(cl, obj, msg)) != NULL)
  {
    GETDATA;
    // use the user data from the inserted treenode, because the user data in the
    // message may be a pointer to a structure on the stack and will be void after
    // this call
    struct ABEntry *ab = (struct ABEntry *)tn->tn_User;

    // accept real users only
    if(ab->Type == AET_USER)
    {
      // insert the person part in the AVL tree
      InsertInAVLTree(data->avlTree, &ab->Address[0]);
    }
  }

  return (IPTR)tn;
}

///
/// OVERLOAD(MUIM_NListtree_Remove)
OVERLOAD(MUIM_NListtree_Remove)
{
  GETDATA;
  struct MUIP_NListtree_Remove *nrm = (struct MUIP_NListtree_Remove *)msg;
  struct MUI_NListtree_TreeNode *tn;

  // update the AVL tree on every removal
  if((LONG)nrm->TreeNode == MUIV_NListtree_Remove_TreeNode_Head)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, nrm->ListNode, MUIV_NListtree_GetEntry_Position_Head, MUIF_NONE);
  }
  else if((LONG)nrm->TreeNode == MUIV_NListtree_Remove_TreeNode_Tail)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, nrm->ListNode, MUIV_NListtree_GetEntry_Position_Tail, MUIF_NONE);
  }
  else if((LONG)nrm->TreeNode == MUIV_NListtree_Remove_TreeNode_Active || (LONG)nrm->TreeNode == MUIV_NListtree_Remove_TreeNode_Selected)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(obj, MUIM_NListtree_GetEntry, nrm->ListNode, MUIV_NListtree_GetEntry_Position_Active, MUIF_NONE);
  }
  else if((LONG)nrm->TreeNode == MUIV_NListtree_Remove_TreeNode_All)
  {
    ClearAVLTree(data->avlTree);
    tn = NULL;
  }
  else
  {
    tn = nrm->TreeNode;
  }

  if(tn != NULL)
  {
    struct ABEntry *ab = (struct ABEntry *)tn->tn_User;

    // remove users only
    if(ab->Type == AET_USER)
    {
      // remove the person part from the AVL tree
      RemoveFromAVLTree(data->avlTree, &ab->Address[0]);
    }
  }

  return DoSuperMethodA(cl, obj, msg);
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

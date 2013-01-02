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

 Superclass:  MUIC_Cycle
 Description: Cycle object to choose a signature

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "SignatureChooser_cl.h"

#include "YAM_config.h"
#include "YAM_utilities.h"

#include "Locale.h"
#include "MUIObjects.h"
#include "Signature.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  struct MinList *signatureList;      // list of signatures
  struct SignatureNode *curSignature; // ptr to currently active signature
  char **signatureArray;              // titles for the different signatures that can be selected
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  ENTER();

  if((obj = DoSuperNew(cl, obj,

    MUIA_CycleChain,    TRUE,
    MUIA_Font,          MUIV_Font_Button,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    // we must know the list on which we operate as there is a difference
    // between the global config and the config being edited
    // default to the global configration
    data->signatureList = (struct MinList *)GetTagData(ATTR(SignatureList), (IPTR)&C->signatureList, inittags(msg));
    data->curSignature = (struct SignatureNode *)GetTagData(ATTR(Signature), (IPTR)NULL, inittags(msg));

    // set up the full description of all active signatures
    DoMethod(obj, METHOD(UpdateSignatures));

    // notify ourselves about changed active items
    DoMethod(obj, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime, obj, 3, MUIM_Set, ATTR(SignatureIndex), MUIV_TriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_DISPOSE)
OVERLOAD(OM_DISPOSE)
{
  GETDATA;
  ULONG result;

  ENTER();

  // free the string array
  FreeStrArray(data->signatureArray);

  // signal the super class to dispose as well
  result = DoSuperMethodA(cl, obj, msg);

  RETURN(result);
  return result;
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(SignatureList):
      {
        data->signatureList = (struct MinList *)tag->ti_Data;
        DoMethod(obj, METHOD(UpdateSignatures));
      }
      break;

      case ATTR(Signature):
      {
        struct SignatureNode *newSignature = (struct SignatureNode *)tag->ti_Data;

        if(newSignature != data->curSignature && data->signatureList != NULL)
        {
          int j = 0;

          // find the new signature and set it as active entry
          if(newSignature != NULL)
          {
            int i = 1;
            struct Node *curNode;

            IterateList(data->signatureList, curNode)
            {
              struct SignatureNode *sn = (struct SignatureNode *)curNode;

              if(sn->id == newSignature->id)
              {
                j = i;
                break;
              }
              else if(sn->active == TRUE)
                i++;
            }
          }

          data->curSignature = newSignature;

          // set the new active item without triggering notifications
          nnset(obj, MUIA_Cycle_Active, j);
        }
      }
      break;

      case ATTR(SignatureIndex):
      {
        if(data->signatureList != NULL)
        {
          struct SignatureNode *newSignature;

          // the first item is always "no signature"
          if(tag->ti_Data != 0)
            newSignature = GetSignature(data->signatureList, tag->ti_Data-1, TRUE);
          else
            newSignature = NULL;

          // set the new signature and trigger possible notifications
          set(obj, ATTR(Signature), newSignature);
        }
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  GETDATA;
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Signature): *store = (IPTR)data->curSignature; return TRUE;
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Private Functions */

/* Public Methods */
/// DECLARE(UpdateSignatures)
// updates the str array containing all signatures
DECLARE(UpdateSignatures)
{
  GETDATA;

  ENTER();

  // we have to sync the content of the signatures
  // with the GUI elements of the write window
  FreeStrArray(data->signatureArray);

  if(data->signatureList != NULL)
  {
    struct Node *curNode;
    int numSignatures = 0;

    // first we find out how many entries the signature list
    // has
    IterateList(data->signatureList, curNode)
    {
      struct SignatureNode *sn = (struct SignatureNode *)curNode;

      if(sn->active == TRUE)
        numSignatures++;
    }

    // allocate enough space +1 for NUL termination and another +1
    // for "no entry"
    if((data->signatureArray = calloc(numSignatures+2, sizeof(char *))) != NULL)
    {
      int i;

      // add a "No Signature" at the front (index 0)
      data->signatureArray[0] = strdup(tr(MSG_CO_IDENTITY_NOSIGNATURE));

      // now we walk through the signatureList again
      // and clone the address string
      i = 1;
      IterateList(data->signatureList, curNode)
      {
        struct SignatureNode *sn = (struct SignatureNode *)curNode;

        if(sn->active == TRUE)
        {
          data->signatureArray[i] = strdup(sn->description);

          i++;
        }
      }

      // update the entry strings and set the active entry
      nnset(obj, MUIA_Cycle_Entries, data->signatureArray);
    }
  }

  RETURN(0);
  return 0;
}

///

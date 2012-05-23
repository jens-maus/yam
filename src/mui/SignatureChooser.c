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

 Superclass:  MUIC_Cycle
 Description: Cycle object to choose a signature

***************************************************************************/

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
  char **signatureArray;            // titles for the different signatures that can be selected
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
    // set up the full description of all active signatures
    DoMethod(obj, METHOD(UpdateSignatures));
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

/* Private Functions */

/* Public Methods */
/// DECLARE(UpdateSignatures)
// updates the str array containing all signatures
DECLARE(UpdateSignatures)
{
  GETDATA;
  struct Node *curNode;
  int numSignatures = 0;

  ENTER();

  // we have to sync the content of the signatures
  // with the GUI elements of the write window
  FreeStrArray(data->signatureArray);

  // first we find out how many entries the signature list
  // has
  IterateList(&C->signatureList, curNode)
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
    IterateList(&C->signatureList, curNode)
    {
      struct SignatureNode *sn = (struct SignatureNode *)curNode;

      if(sn->active == TRUE)
      {
        // construct the new string via asprintf() so that the necessary
        // memory is automatically allocated.
        data->signatureArray[i] = strdup(sn->description);

        i++;
      }
    }

    // update the entry strings and set the active entry
    nnset(obj, MUIA_Cycle_Entries, data->signatureArray);
  }

  RETURN(0);
  return 0;
}

///

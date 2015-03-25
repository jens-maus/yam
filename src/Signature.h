#ifndef SIGNATURE_H
#define SIGNATURE_H

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

***************************************************************************/

#include "YAM_stringsizes.h"

// forward declarations

// signature data structure
struct SignatureNode
{
  struct MinNode node;            // required for placing it into struct Config

  int id;                         // unique id for the signature
  BOOL active;                    // is this signature currently active?
  BOOL useSignatureFile;          // use signature file?

  char description[SIZE_LARGE];   // user definable description
  char filename[SIZE_FILE];       // external signature file
  char *signature;                // signature text
};

// public functions
struct SignatureNode *CreateNewSignature(void);
void FreeSignatureList(struct MinList *signatureList);
BOOL CompareSignatureLists(const struct MinList *sl1, const struct MinList *sl2);
struct SignatureNode *GetSignature(const struct MinList *signatureList, const unsigned int num, const BOOL activeOnly);
BOOL IsUniqueSignatureID(const struct MinList *signatureList, const int id);
struct SignatureNode *FindSignatureByID(const struct MinList *signatureList, const int id);
char *ImportSignature(const char *src);
char *ExportSignature(const char *src);
struct SignatureNode *CreateSignatureFromFile(const char *file, const char *description);

#endif // SIGNATURE_H

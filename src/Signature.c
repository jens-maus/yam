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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <proto/exec.h>

#include "extrasrc.h"

#if !defined(__amigaos4__)
#include <clib/alib_protos.h>
#endif

#include "YAM_utilities.h"

#include "FileInfo.h"
#include "Signature.h"
#include "StrBuf.h"

#include "Debug.h"

/***************************************************************************
 Module: Signature related routines
***************************************************************************/

/// CreateNewSignature
//  Initializes a new SignatureNode
struct SignatureNode *CreateNewSignature(void)
{
  struct SignatureNode *sn;

  ENTER();

  if((sn = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*sn),
                                         ASONODE_Min, TRUE,
                                         TAG_DONE)) != NULL)
  {
    // initialize all variables as AllocSysObject() does not clear the memory
    memset(sn, 0, sizeof(*sn));

    // now we fill the SignatureNode structure with some sensible defaults
    sn->active = TRUE;
    // default to use a signature file to be compatible with YAM <= 2.7
    sn->useSignatureFile = TRUE;
  }

  RETURN(sn);
  return sn;
}

///
/// FreeSignatureList
void FreeSignatureList(struct MinList *signatureList)
{
  struct Node *curNode;

  ENTER();

  // we have to free the signatureList
  while((curNode = RemHead((struct List *)signatureList)) != NULL)
  {
    struct SignatureNode *sn = (struct SignatureNode *)curNode;

    free(sn->signature);
    FreeSysObject(ASOT_NODE, sn);
  }

  NewMinList(signatureList);

  LEAVE();
}

///
/// CompareSignatureNodes
static BOOL CompareSignatureNodes(const struct Node *n1, const struct Node *n2)
{
  BOOL equal = TRUE;
  const struct SignatureNode *sn1 = (const struct SignatureNode *)n1;
  const struct SignatureNode *sn2 = (const struct SignatureNode *)n2;

  ENTER();

  // compare every single member of the structure
  if(sn1->id != sn2->id ||
     sn1->active != sn2->active ||
     sn1->useSignatureFile != sn2->useSignatureFile ||
     strcmp(sn1->description, sn2->description) != 0 ||
     strcmp(sn1->filename, sn2->filename) != 0 ||
     strcmp(sn1->signature != NULL ? sn1->signature : "", sn2->signature != NULL ? sn2->signature : "") != 0)
  {
    // something does not match
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// CompareSignatureLists
// compare two Signature lists
BOOL CompareSignatureLists(const struct MinList *sl1, const struct MinList *sl2)
{
  BOOL equal;

  ENTER();

  equal = CompareLists((const struct List *)sl1, (const struct List *)sl2, CompareSignatureNodes);

  RETURN(equal);
  return equal;
}

///
/// GetSignature
// function to extract the structure of a signature from our signature list
struct SignatureNode *GetSignature(const struct MinList *signatureList,
                                   const unsigned int num, const BOOL activeOnly)
{
  struct SignatureNode *result = NULL;
  unsigned int count = 0;
  struct Node *curNode;

  ENTER();

  IterateList(signatureList, curNode)
  {
    struct SignatureNode *sn = (struct SignatureNode *)curNode;

    if(activeOnly == FALSE || sn->active == TRUE)
    {
      if(count == num)
      {
        result = sn;
        break;
      }

      count++;
    }
  }

  RETURN(result);
  return result;
}

///
/// IsUniqueSignatureID
// check if the ID is unique within the list of signatures
BOOL IsUniqueSignatureID(const struct MinList *signatureList, const int id)
{
  BOOL isUnique = TRUE;
  struct Node *curNode;

  ENTER();

  IterateList(signatureList, curNode)
  {
    struct SignatureNode *sn = (struct SignatureNode *)curNode;

    if(sn->id == id)
    {
      // we found exactly this ID, this is bad
      isUnique = FALSE;
      break;
    }
  }

  RETURN(isUnique);
  return isUnique;
}

///
/// FindSignatureByID
// find a signature by a given ID
struct SignatureNode *FindSignatureByID(const struct MinList *signatureList, const int id)
{
  struct SignatureNode *result = NULL;

  ENTER();

  if(id > 0)
  {
    struct Node *curNode;

    IterateList(signatureList, curNode)
    {
      struct SignatureNode *sn = (struct SignatureNode *)curNode;

      // check if we found exactly this ID
      if(id == sn->id)
      {
        result = sn;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// ImportSignature
// import a signature text, replace all '\n' by LF
char *ImportSignature(const char *src)
{
  char *sig = NULL;

  ENTER();

  if(src != NULL)
  {
    char c;

    while((c = *src++) != '\0')
    {
      if(c == '\\')
      {
        // peek at the next character and convert it if possible
        switch(*src)
        {
          case 'n':
          {
            c = '\n';
            src++;
          }
          break;

          case 'r':
          {
            c = '\r';
            src++;
          }
          break;

          case 't':
          {
            c = '\t';
            src++;
          }
          break;

          case '\\':
          {
            c = '\\';
            src++;
          }
          break;

          case 'x':
          case 'X':
          {
            // convert \xHH to a character
            if(src[1] != '\0' && src[2] != '\0')
            {
              // don't use strtol() on the source string directly as it may contain
              // valid hex characters after the \xHH sequence when then would get
              // swallowed.
              char hex[3];

              hex[0] = src[1];
              hex[1] = src[2];
              hex[2] = '\0';
              c = (char)strtol(hex, NULL, 16);
              src += 3;
            }
  		  }
  		  break;

  		  default:
  		  {
            // unsupported backslash sequence, use character "as it is"
  		  }
  		  break;
        }
      }

      if(c != '\0')
      {
        // append a plain ASCII character
        char cat[2] = {c, '\0'};

        StrBufCat(&sig, cat);
      }
    }

    if(sig != NULL)
    {
      char *tmp = strdup(sig);

      FreeStrBuf(sig);
      sig = tmp;
    }
  }

  RETURN(sig);
  return sig;
}

///
/// ExportSignature
// export a signature text, replace all special chars like LF by '\n'
char *ExportSignature(const char *src)
{
  char *sig = NULL;

  ENTER();

  if(src != NULL)
  {
    char c;

    while((c = *src++) != '\0')
    {
      // replace non-ASCII characters by well known backslash sequences
      switch(c)
      {
        case '\n':
        {
          StrBufCat(&sig, "\\n");
        }
        break;

        case '\r':
        {
          StrBufCat(&sig, "\\r");
        }
        break;

        case '\t':
        {
          StrBufCat(&sig, "\\t");
        }
        break;

        case '\\':
        {
          StrBufCat(&sig, "\\\\");
        }
        break;

        default:
        {
          if(isprint((int)c))
          {
            // printable ASCII characters are used unmodified
            char cat[2] = {c, '\0'};

            StrBufCat(&sig, cat);
          }
          else
          {
            char xchar[6];

            // use the typical \xHH representation
            snprintf(xchar, sizeof(xchar), "\\x%02x", c & 0xff);
            StrBufCat(&sig, xchar);
  	      }
        }
        break;
      }
    }

    if(sig != NULL)
    {
      char *tmp = strdup(sig);

      FreeStrBuf(sig);
      sig = tmp;
    }
  }

  RETURN(sig);
  return sig;
}

///
/// CreateSignatureFromFile
// create a new signature node using a signature file
struct SignatureNode *CreateSignatureFromFile(const char *file, const char *description)
{
  struct SignatureNode *sn = NULL;
  char sigPath[SIZE_PATHFILE];

  ENTER();

  // check if the signature file exists
  CreateFilename(file, sigPath, sizeof(sigPath));
  if(FileExists(sigPath) == TRUE)
  {
    // create a new signature which uses the file as signature text
    if((sn = CreateNewSignature()) != NULL)
    {
      // default description
      strlcpy(sn->description, description, sizeof(sn->description));
      strlcpy(sn->filename, sigPath, sizeof(sn->filename));
      sn->useSignatureFile = TRUE;
    }
  }

  RETURN(sn);
  return sn;
}

///

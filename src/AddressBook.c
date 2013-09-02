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
#include <stdio.h>
#include <string.h>

#include <proto/codesets.h>
#include <proto/exec.h>
#include <proto/expat.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "mime/base64.h"
#include "mui/ClassesExtra.h"

#include "AddressBook.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "Locale.h"
#include "Logfile.h"
#include "Requesters.h"

#include "Debug.h"

/// CreateABookNode
struct ABookNode *CreateABookNode(enum ABookNodeType type)
{
  struct ABookNode *abn;

  ENTER();

  if((abn = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, sizeof(*abn),
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    InitABookNode(abn, type);
  }

  RETURN(abn);
  return abn;
}

///
/// InitABookNode
void InitABookNode(struct ABookNode *abn, enum ABookNodeType type)
{
  ENTER();

  memset(abn, 0, sizeof(*abn));
  abn->type = type;

  switch(type)
  {
    case AET_GROUP:
    {
      NewMinList(&abn->content.group.Members);
    }
    break;

    default:
    {
      // nothing else to do
    }
    break;
  }

  LEAVE();
}

///
/// DeleteABookNode
void DeleteABookNode(struct ABookNode *abn)
{
  ENTER();

  FreeSysObject(ASOT_NODE, abn);

  LEAVE();
}

///
/// InitAddressBook
void InitAddressBook(struct AddressBook *abook)
{
  ENTER();

  NewMinList(&abook->root);
  abook->modified = FALSE;

  LEAVE();
}

///
/// ClearAddressBookNode
static BOOL ClearAddressBookNode(const struct ABookNode *abn, BOOL first, UNUSED const void *userData)
{
  ENTER();

  if(first == TRUE)
  {
    if(abn->type == AET_LIST)
      free(abn->content.list.Members);

    DeleteABookNode((struct ABookNode *)abn);
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// ClearAddressBook
void ClearAddressBook(struct AddressBook *abook)
{
  ENTER();

  IterateAddressBook(abook, ClearAddressBookNode, NULL);
  InitAddressBook(abook);

  LEAVE();
}

///
/// SetDefaultAlias
//  Creates an alias from the real name if user left it empty
static void SetDefaultAlias(struct ABookNode *abn)
{
  char *p;

  ENTER();

  memset(abn->Alias, 0, sizeof(abn->Alias));
  p = abn->Alias;
  if(abn->content.user.RealName[0] != '\0')
  {
    char *ln;

    if((ln = strrchr(abn->content.user.RealName, ' ')) != NULL)
    {
      if(isAlNum(abn->content.user.RealName[0]))
      {
        *p++ = abn->content.user.RealName[0];
        *p++ = '_';
      }
      ln++;
    }
    else
      ln = abn->content.user.RealName;

    for(; strlen(abn->Alias) < SIZE_NAME-2 && ln[0] != '\0'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }
  else
  {
    char *ln;

    for(ln = abn->content.user.Address; strlen(abn->Alias) < SIZE_NAME-2 && *ln != '\0' && *ln != '@'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }

  LEAVE();
}

///

/// IterateAddressBookTree
static BOOL IterateAddressBookTree(const struct MinList *root, BOOL (*nodeFunc)(const struct ABookNode *abn, BOOL first, const void *userData), const void *userData)
{
  struct ABookNode *abn;
  struct ABookNode *next;
  BOOL result = TRUE;

  ENTER();

  // safe iteration over all nodes, because this function might be used for cleanup
  // purposes, too
  SafeIterateList(root, struct ABookNode *, abn, next)
  {
    result = nodeFunc(abn, TRUE, userData);
    if(result == FALSE)
      break;

    // handle groups in a recursive fashion first
    if(abn->type == AET_GROUP)
    {
      result = IterateAddressBookTree(&abn->content.group.Members, nodeFunc, userData);
      if(result == FALSE)
        break;
    }

    result = nodeFunc(abn, FALSE, userData);
    if(result == FALSE)
      break;
  }

  RETURN(result);
  return result;
}

///
/// IterateAddressBook
BOOL IterateAddressBook(struct AddressBook *abook, BOOL (*nodeFunc)(const struct ABookNode *abn, BOOL first, const void *userData), const void *userData)
{
  BOOL result;

  ENTER();

  result = IterateAddressBookTree(&abook->root, nodeFunc, userData);

  RETURN(result);
  return result;
}

///
/// LoadAddressBook
BOOL LoadAddressBook(const char *filename, struct AddressBook *abook, BOOL append)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "r")) != NULL)
  {
    char *buffer = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(GetLine(&buffer, &size, fh) >= 0)
    {
      int nested = 0;
      struct MinList *parent[8];

      parent[0] = &abook->root;

      if(strncmp(buffer,"YAB",3) == 0)
      {
        int version = buffer[3] - '0';

        if(append == FALSE)
          InitAddressBook(abook);

        while(GetLine(&buffer, &size, fh) >= 0)
        {
          struct ABookNode *abn;

          if(strncmp(buffer, "@USER", 5) == 0)
          {
            if((abn = CreateABookNode(AET_USER)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[6]), sizeof(abn->Alias));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->content.user.Address, Trim(buffer), sizeof(abn->content.user.Address));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->content.user.RealName, Trim(buffer), sizeof(abn->content.user.RealName));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              if(version > 2)
              {
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.Phone, Trim(buffer), sizeof(abn->content.user.Phone));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.Street, Trim(buffer), sizeof(abn->content.user.Street));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.City, Trim(buffer), sizeof(abn->content.user.City));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.Country, Trim(buffer), sizeof(abn->content.user.Country));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.PGPId, Trim(buffer), sizeof(abn->content.user.PGPId));
                GetLine(&buffer, &size, fh);
                abn->content.user.Birthday = atol(Trim(buffer));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.user.Photo, Trim(buffer), sizeof(abn->content.user.Photo));
                GetLine(&buffer, &size, fh);
                if(strcmp(buffer, "@ENDUSER") == 0)
                  strlcpy(abn->content.user.Homepage, Trim(buffer), sizeof(abn->content.user.Homepage));
              }
              if(version > 3)
              {
                GetLine(&buffer, &size, fh);
                abn->content.user.DefSecurity = atoi(Trim(buffer));
              }

              // skip any additional lines
              do
              {
                if(strcmp(buffer, "@ENDUSER") == 0)
                  break;
              }
              while(GetLine(&buffer, &size, fh) >= 0);

              AddTail((struct List *)parent[nested], (struct Node *)abn);
            }
          }
          else if(strncmp(buffer, "@LIST", 5) == 0)
          {
            if((abn = CreateABookNode(AET_LIST)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[6]), sizeof(abn->Alias));
              if(version > 2)
              {
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.list.Address, Trim(buffer), sizeof(abn->content.list.Address));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->content.list.RealName, Trim(buffer), sizeof(abn->content.list.RealName));
              }
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              while(GetLine(&buffer, &size, fh) >= 0)
              {
                if(strcmp(buffer, "@ENDLIST") == 0)
                  break;

                if(*buffer == '\0')
                  continue;

                dstrcat(&abn->content.list.Members, buffer);
                dstrcat(&abn->content.list.Members, "\n");
              }

              AddTail((struct List *)parent[nested], (struct Node *)abn);
            }
          }
          else if(strncmp(buffer, "@GROUP", 6) == 0)
          {
            if((abn = CreateABookNode(AET_GROUP)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[7]), sizeof(abn->Alias));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              AddTail((struct List *)parent[nested], (struct Node *)abn);

              nested++;
              parent[nested] = &abn->content.group.Members;
            }
          }
          else if(strcmp(buffer,"@ENDGROUP") == 0)
          {
            nested--;
          }
        }

        // no errors happened
        result = TRUE;
      }
      else
      {
        // ask the user if he really wants to read out a non YAM
        // Addressbook file.
        if(MUI_Request(G->App, NULL, MUIF_NONE, NULL, tr(MSG_AB_NOYAMADDRBOOK_GADS), tr(MSG_AB_NOYAMADDRBOOK), filename))
        {
          if(append == FALSE)
            InitAddressBook(abook);

          fseek(fh, 0, SEEK_SET);
          while(GetLine(&buffer, &size, fh) >= 0)
          {
            struct ABookNode *abn;

            if((abn = CreateABookNode(AET_USER)) != NULL)
            {
              char *p, *p2;

              if((p = strchr(buffer, ' ')) != NULL)
                *p = '\0';
              strlcpy(abn->content.user.Address, buffer, sizeof(abn->content.user.Address));
              if(p != NULL)
              {
                strlcpy(abn->content.user.RealName, ++p, sizeof(abn->content.user.RealName));
                if((p2 = strchr(p, ' ')) != NULL)
                   *p2 = '\0';
              }
              else
              {
                p = buffer;
                if((p2 = strchr(p, '@')) != NULL)
                   *p2 = '\0';
              }
              strlcpy(abn->Alias, p, sizeof(abn->Alias));

              AddTail((struct List *)parent[nested], (struct Node *)abn);
            }
          }
        }

        // no errors happened
        result = TRUE;
      }
    }
    else
      ER_NewError(tr(MSG_ER_ADDRBOOKLOAD), filename);

    fclose(fh);

    free(buffer);
  }
  else
  {
    // show an error message only if the .addressbook file exists but could not be opened
    if(FileExists(filename) == TRUE)
      ER_NewError(tr(MSG_ER_ADDRBOOKLOAD), filename);
  }

  if(result == TRUE)
  {
    abook->modified = append;
  }

  RETURN(result);
  return result;
}

///
/// SaveAddressBookEntry
static BOOL SaveAddressBookEntry(const struct ABookNode *abn, BOOL first, const void *userData)
{
  FILE *fh = (FILE *)userData;

  ENTER();

  switch(abn->type)
  {
    case AET_USER:
    {
      if(first == TRUE)
      {
        fprintf(fh, "@USER %s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%08ld\n%s\n%s\n%d\n@ENDUSER\n",
                    abn->Alias, abn->content.user.Address, abn->content.user.RealName, abn->Comment,
                    abn->content.user.Phone, abn->content.user.Street, abn->content.user.City, abn->content.user.Country, abn->content.user.PGPId, abn->content.user.Birthday, abn->content.user.Photo, abn->content.user.Homepage, abn->content.user.DefSecurity);
      }
    }
    break;

    case AET_LIST:
    {
      if(first == TRUE)
      {
        fprintf(fh, "@LIST %s\n%s\n%s\n%s\n%s\n@ENDLIST\n", abn->Alias, abn->content.user.Address, abn->content.user.RealName, abn->Comment, abn->content.list.Members != NULL ? abn->content.list.Members : "");
      }
    }
    break;

    case AET_GROUP:
    {
      if(first == TRUE)
        fprintf(fh, "@GROUP %s\n%s\n", abn->Alias, abn->Comment);
      else
        fputs("@ENDGROUP\n", fh);
    }
    break;
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// SaveAddressBook
BOOL SaveAddressBook(const char *filename, const struct AddressBook *abook)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fputs("YAB4 - YAM Addressbook\n", fh);
    result = IterateAddressBook((struct AddressBook *)abook, SaveAddressBookEntry, fh);
    fclose(fh);
    AppendToLogfile(LF_VERBOSE, 70, tr(MSG_LOG_SavingABook), filename);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), filename);

  RETURN(result);
  return result;
}

///
/// ImportAddressBookLDIF
//  Imports an address book in LDIF format
BOOL ImportAddressBookLDIF(const char *filename, struct AddressBook *abook, BOOL append)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "r")) != NULL)
  {
    char *buffer = NULL;
    size_t size = 0;
    struct ABookNode abn;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(append == FALSE)
      InitAddressBook(abook);

    InitABookNode(&abn, AET_USER);

    while(GetLine(&buffer, &size, fh) >= 0)
    {
      // an empty line separates two user entries
      if(buffer[0] == '\0')
      {
        // we need at least an EMail address
        if(abn.content.user.Address[0] != '\0')
        {
          struct ABookNode *node;

          // set up an alias only if none is given
          if(abn.Alias[0] == '\0')
            SetDefaultAlias(&abn);

          if((node = DuplicateNode(&abn, sizeof(abn))) != NULL)
          {
            // put it into the tree
            AddTail((struct List *)&abook->root, (struct Node *)node);
            result = TRUE;
          }
        }
      }
      else
      {
        char *key, *value;

        // every line has the pattern "key: value"
        // now separate these two
        key = buffer;
        if((value = strpbrk(key, ":")) != NULL)
        {
          BOOL utf8;

          *value++ = '\0';

          // a leading colon in the value marks a base64 encoded string
          if(value[0] == ':')
          {
            char *b64buffer = NULL;

            // first decode it
            base64decode(&b64buffer, &value[2], strlen(&value[2]));

            // now convert this prossible UTF8 string to a normal string
            value = CodesetsUTF8ToStr(CSA_Source,          Trim(b64buffer),
                                      CSA_DestCodeset,     G->localCodeset,
                                      CSA_MapForeignChars, C->MapForeignChars,
                                      TAG_DONE);
            utf8 = TRUE;

            free(b64buffer);
          }
          else
          {
            // take the value as it is
            value = Trim(value);
            utf8 = FALSE;
          }

          if(value != NULL)
          {
            // this is the very first line a new entry,
            // so clear the structure for further actions now
            if(stricmp(key, "dn") == 0)
            {
              InitABookNode(&abn, AET_USER);
            }
            else if(stricmp(key, "cn") == 0)                         // complete name
              strlcpy(abn.content.user.RealName, value, sizeof(abn.content.user.RealName));
            else if(stricmp(key, "mail") == 0)                       // mail address
              strlcpy(abn.content.user.Address, value, sizeof(abn.content.user.Address));
            else if(stricmp(key, "mozillaNickname") == 0)            // alias
              strlcpy(abn.Alias, value, sizeof(abn.Alias));
            else if(stricmp(key, "telephoneNumber") == 0)            // phone number
            {
              if(abn.content.user.Phone[0] != '\0')
                strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, value, sizeof(abn.content.user.Phone));
            }
            else if(stricmp(key, "homePhone") == 0)                  // phone number
            {
              if(abn.content.user.Phone[0] != '\0')
                strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, value, sizeof(abn.content.user.Phone));
            }
            else if(stricmp(key, "fax") == 0)                        // fax number
            {
              if(abn.content.user.Phone[0] != '\0')
              strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, value, sizeof(abn.content.user.Phone));
            }
            else if(stricmp(key, "pager") == 0)                      // pager number
            {
              if(abn.content.user.Phone[0] != '\0')
                strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, value, sizeof(abn.content.user.Phone));
            }
            else if(stricmp(key, "mobile") == 0)                     // mobile number
            {
              if(abn.content.user.Phone[0] != '\0')
                strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, value, sizeof(abn.content.user.Phone));
            }
            else if(stricmp(key, "homeStreet") == 0)                 // office street
            {
              if(abn.content.user.Street[0] != '\0')
                strlcat(abn.content.user.Street, ", ", sizeof(abn.content.user.Street));
              strlcat(abn.content.user.Street, value, sizeof(abn.content.user.Street));
            }
            else if(stricmp(key, "mozillaHomeStreet2") == 0)         // home street
            {
              if(abn.content.user.Street[0] != '\0')
                strlcat(abn.content.user.Street, ", ", sizeof(abn.content.user.Street));
              strlcat(abn.content.user.Street, value, sizeof(abn.content.user.Street));
            }
            else if(stricmp(key, "l") == 0)                          // office locality
            {
              if(abn.content.user.City[0] != '\0')
                strlcat(abn.content.user.City, ", ", sizeof(abn.content.user.City));
              strlcat(abn.content.user.City, value, sizeof(abn.content.user.City));
            }
            else if(stricmp(key, "mozillaHomeLocalityName") == 0)    // home locality
            {
              if(abn.content.user.City[0] != '\0')
                strlcat(abn.content.user.City, ", ", sizeof(abn.content.user.City));
              strlcat(abn.content.user.City, value, sizeof(abn.content.user.City));
            }
            else if(stricmp(key, "postalCode") == 0)                 // office postal code
            {
              if(abn.content.user.City[0] != '\0')
                strlcat(abn.content.user.City, ", ", sizeof(abn.content.user.City));
              strlcat(abn.content.user.City, value, sizeof(abn.content.user.City));
            }
            else if(stricmp(key, "mozillaHomePostalCode") == 0)      // home postal code
            {
              if(abn.content.user.City[0] != '\0')
                strlcat(abn.content.user.City, ", ", sizeof(abn.content.user.City));
              strlcat(abn.content.user.City, value, sizeof(abn.content.user.City));
            }
            else if(stricmp(key, "c") == 0)                          // office country
            {
              if(abn.content.user.Country[0] != '\0')
                strlcat(abn.content.user.Country, ", ", sizeof(abn.content.user.Country));
              strlcat(abn.content.user.Country, value, sizeof(abn.content.user.Country));
            }
            else if(stricmp(key, "mozillaHomeCountryName") == 0)     // home country
            {
              if(abn.content.user.Country[0] != '\0')
                strlcat(abn.content.user.Country, ", ", sizeof(abn.content.user.Country));
              strlcat(abn.content.user.Country, value, sizeof(abn.content.user.Country));
            }
            else if(stricmp(key, "mozillaWorkUrl") == 0)             // working home page
            {
              if(abn.content.user.Homepage[0] != '\0')
                strlcat(abn.content.user.Homepage, ", ", sizeof(abn.content.user.Homepage));
              strlcat(abn.content.user.Homepage, value, sizeof(abn.content.user.Homepage));
            }
            else if(stricmp(key, "mozillaHomeUrl") == 0)             // private homepage
            {
              if(abn.content.user.Homepage[0] != '\0')
                strlcat(abn.content.user.Homepage, ", ", sizeof(abn.content.user.Homepage));
              strlcat(abn.content.user.Homepage, value, sizeof(abn.content.user.Homepage));
            }
          }

          // if the value string has been converted from UTF8 before we need to free it now
          if(utf8 == TRUE)
            CodesetsFreeA(value, NULL);
        }
      }
    }

    fclose(fh);

    free(buffer);
  }
  else
     ER_NewError(tr(MSG_ER_ADDRBOOKIMPORT), filename);

  if(result == TRUE)
  {
    // now remember the "modified" state
    abook->modified = append;
  }

  RETURN(result);
  return result;
}

///
/// WriteLDIFLine
// writes a line to an LDIF address book file according to RFC 2849
static void WriteLDIFLine(FILE *fh, const char *key, const char *valueFmt, ...)
{
  ENTER();

  if(key[0] != '\0')
  {
    char *buffer = NULL;
    va_list args;

    // put the arguments into the value string
    va_start(args, valueFmt);
    if(vasprintf(&buffer, valueFmt, args) != -1)
    {
      // now check if the value string must be UTF8/base64 encoded
      char *p = buffer;
      unsigned char c;
      BOOL initChar = TRUE;
      BOOL mustBeEncoded = FALSE;

      while((c = *p++) != '\0' && mustBeEncoded == FALSE)
      {
        BOOL safeChar;

        // these characters are safe, everything else must be encoded
        // see RFC 2849
        if(initChar == TRUE)
        {
          // safe init character
          safeChar = ((c >= 0x01 && c <= 0x09) ||
                      (c >= 0x0b && c <= 0x0c) ||
                      (c >= 0x0e && c <= 0x1f) ||
                      (c >= 0x21 && c <= 0x39) ||
                      (c == 0x3b)              ||
                      (c >= 0x3d && c <= 0x7f));

          initChar = FALSE;
        }
        else
        {
          // safe characters
          safeChar = ((c >= 0x01 && c <= 0x09) ||
                      (c >= 0x0b && c <= 0x0c) ||
                      (c >= 0x0e && c <= 0x7f));
        }

        // yes, we have to encode this string
        if(safeChar == FALSE)
          mustBeEncoded = TRUE;
      }

      if(mustBeEncoded == TRUE)
      {
        UTF8 *utf8;
        size_t utf8len = 0;

        // convert the value string to UTF8
        if((utf8 = CodesetsUTF8Create(CSA_Source, buffer,
                                      CSA_SourceCodeset, G->localCodeset,
                                      CSA_DestLenPtr, &utf8len,
                                      TAG_DONE)) != NULL)
        {
          char *b64_buffer = NULL;

          // we can reuse the former buffer here again, because we have a copy of the string
          // in utf8
          if(base64encode(&b64_buffer, (char *)utf8, utf8len) > 0)
          {
            // write the key and encoded value strings
            // these are separated by a double colon
            fprintf(fh, "%s:: %s\n", key, b64_buffer);

            free(b64_buffer);
          }

          CodesetsFreeA(utf8, NULL);
        }
      }
      else
      {
        // write the unencoded key and value strings
        // these are separated by a single colon
        fprintf(fh, "%s: %s\n", key, buffer);
      }

      free(buffer);
    }
    va_end(args);
  }
  else
  {
    // just write the end marker (a blank line)
    fprintf(fh, "\n");
  }

  LEAVE();
}

///
/// ExportAddressBookLDIFEntry
//  Exports an address book as LDIF file
static BOOL ExportAddressBookLDIFEntry(const struct ABookNode *abn, BOOL first, const void *userData)
{
  FILE *fh = (FILE *)userData;

  ENTER();

  switch(abn->type)
  {
    case AET_USER:
    {
      if(first == TRUE)
      {
        WriteLDIFLine(fh, "dn", "cn=%s,mail=%s", abn->content.user.RealName, abn->content.user.Address);
        WriteLDIFLine(fh, "objectClass", "top");
        WriteLDIFLine(fh, "objectClass", "person");
        WriteLDIFLine(fh, "objectClass", "organizationalPerson");
        WriteLDIFLine(fh, "objectClass", "inetOrdPerson");
        WriteLDIFLine(fh, "objectClass", "mozillaAbPersonAlpha");
        WriteLDIFLine(fh, "cn", "%s", abn->content.user.RealName);
        WriteLDIFLine(fh, "mail", "%s", abn->content.user.Address);
        if(abn->Alias[0] != '\0')
          WriteLDIFLine(fh, "mozillaNickname", "%s", abn->Alias);
        if(abn->content.user.Phone[0] != '\0')
          WriteLDIFLine(fh, "telephoneNumber", "%s", abn->content.user.Phone);
        if(abn->content.user.Street[0] != '\0')
          WriteLDIFLine(fh, "street", "%s", abn->content.user.Street);
        if(abn->content.user.City[0] != '\0')
          WriteLDIFLine(fh, "l", "%s", abn->content.user.City);
        if(abn->content.user.Country[0] != '\0')
          WriteLDIFLine(fh, "c", "%s", abn->content.user.Country);
        if(abn->content.user.Homepage[0] != '\0')
          WriteLDIFLine(fh, "mozillaHomeUrl", "%s", abn->content.user.Homepage);
        WriteLDIFLine(fh, "", "");
      }
    }
    break;

    case AET_GROUP:
    {
      // groups are handled by IterateAddressBook() already
    }
    break;

    default:
    {
      // lists cannot be exported to LDIF
    }
    break;
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// ExportAddressBookLDIF
//  Exports an address book as LDIF file
BOOL ExportAddressBookLDIF(const char *filename, const struct AddressBook *abook)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    result = IterateAddressBook((struct AddressBook *)abook, ExportAddressBookLDIFEntry, fh);

    fclose(fh);
    result = TRUE;
  }
  else
    ER_NewError(tr(MSG_ER_ADDRBOOKEXPORT), filename);

  RETURN(result);
  return result;
}

///

struct CSVStuff
{
  FILE *fh;
  char delimiter;
};

/// ImportAddressBookCSV
//  Imports an address book with comma or tab separated entries
BOOL ImportAddressBookCSV(const char *filename, struct AddressBook *abook, BOOL append, char delimiter)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "r")) != NULL)
  {
    char *buffer = NULL;
    size_t size = 0;
    char delimStr[2];

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(append == FALSE)
      InitAddressBook(abook);

    delimStr[0] = delimiter;
    delimStr[1] = '\0';

    while(GetLine(&buffer, &size, fh) >= 0)
    {
      struct ABookNode abn;
      char *item = buffer;
      int itemNumber = 0;

      InitABookNode(&abn, AET_USER);

      do
      {
        char *next;

        // first check if the current item begins with a quote
        if(item[0] == '"')
        {
          // now we have to search for the next quote and treat all
          // characters inbetween as one item
          if((next = strpbrk(&item[1], "\"")) != NULL)
          {
            // skip the leading quote
            item++;
            // erase the trailing quote
            *next++ = '\0';
            // and look for the next delimiter starting right after the just erased quote
            if((next = strpbrk(next, delimStr)) != NULL)
              *next++ = '\0';
          }
          else
          {
            // no closing quote found, abort this line
            item[0] = '\0';
            // to make sure this item doesn't make it into YAM's address book clear all values again
            abn.content.user.Address[0] = '\0';
          }
        }
        else
        {
          // do a normal search for the separating character
          if((next = strpbrk(item, delimStr)) != NULL)
            *next++ = '\0';
        }

        itemNumber++;

        // remove any nonsense like leading or trailing spaces
        item = Trim(item);

        if(item[0] != '\0')
        {
          // Thunderbird 1.5 exports 36 items, let's look which
          switch(itemNumber)
          {
            // first name
            case 1:
            {
              strlcat(abn.content.user.RealName, item, sizeof(abn.content.user.RealName));
            }
            break;

            // last name
            case 2:
            {
              if(abn.content.user.RealName[0] != '\0')
                strlcat(abn.content.user.RealName, " ", sizeof(abn.content.user.RealName));
              strlcat(abn.content.user.RealName, item, sizeof(abn.content.user.RealName));
            }
            break;

            // complete name, preferred, if available
            case 3:
            {
              strlcpy(abn.content.user.RealName, item, sizeof(abn.content.user.RealName));
            }
            break;

            // nickname
            case 4:
            {
              strlcpy(abn.Alias, item, sizeof(abn.Alias));
            }
            break;

            // EMail address
            case 5:
            {
              strlcpy(abn.content.user.Address, item, sizeof(abn.content.user.Address));
            }
            break;

            // second EMail address, ignored
            case 6:
              // nothing
            break;

            case 7:   // office phone number
            case 8:   // private phone number
            case 9:   // fax number
            case 10:  // pager number
            case 11:  // mobile phone
            {
              if(abn.content.user.Phone[0] != '\0')
                strlcat(abn.content.user.Phone, ", ", sizeof(abn.content.user.Phone));
              strlcat(abn.content.user.Phone, item, sizeof(abn.content.user.Phone));
            }
            break;

            case 12: // address, part 1
            case 13: // address, part 2
            {
              if(abn.content.user.Street[0] != '\0')
                strlcat(abn.content.user.Street, " ", sizeof(abn.content.user.Street));
              strlcat(abn.content.user.Street, item, sizeof(abn.content.user.Street));
            }
            break;

            // city
            case 14:
            {
              strlcpy(abn.content.user.City, item, sizeof(abn.content.user.City));
            }
            break;

            // province, ignored
            case 15:
              // nothing
            break;

            // ZIP code, append it to the city name
            case 16:
            {
              if(abn.content.user.City[0] != '\0')
                strlcat(abn.content.user.City, ", ", sizeof(abn.content.user.City));
              strlcat(abn.content.user.City, item, sizeof(abn.content.user.City));
            }
            break;

            // country
            case 17:
            {
              strlcpy(abn.content.user.Country, item, sizeof(abn.content.user.Country));
            }
            break;

            case 27: // office web address
            case 28: // private web address
            {
              strlcpy(abn.content.user.Homepage, item, sizeof(abn.content.user.Homepage));
            }
            break;

            default: // everything else is ignored
              break;
          }
        }

        item = next;
      }
      while(item != NULL);

      // we need at least an EMail address
      if(abn.content.user.Address[0] != '\0')
      {
        struct ABookNode *node;

        // set up an alias only if none is given
        if(abn.Alias[0] == '\0')
          SetDefaultAlias(&abn);


        if((node = DuplicateNode(&abn, sizeof(abn))) != NULL)
        {
          AddTail((struct List *)&abook->root, (struct Node *)node);
          result = TRUE;
        }
      }
    }

    free(buffer);

    fclose(fh);
  }
  else
    ER_NewError(tr(MSG_ER_ADDRBOOKIMPORT), filename);

  if(result == TRUE)
  {
    // now remember the "modified" state
    abook->modified = append;
  }

  RETURN(result);
  return result;
}

///
/// WriteCSVItem
// writes TAB or comma separated item to an address book file
static void WriteCSVItem(struct CSVStuff *stuff, const char *value)
{
  ENTER();

  if(value != NULL)
  {
    char c;
    char *p = (char *)value;
    BOOL addQuotes = FALSE;

    // check if we need to add quotes to the item because it contains the separating character
    while((c = *p++) != '\0' && addQuotes == FALSE)
    {
      if(c == ',' || c == stuff->delimiter)
        addQuotes = TRUE;
    }

    if(addQuotes)
    {
      // surround the value by quotes and add the delimiter
      fprintf(stuff->fh, "\"%s\"%c", value, stuff->delimiter);
    }
    else
    {
      // just write the value and the delimiter
      fprintf(stuff->fh, "%s%c", value, stuff->delimiter);
    }
  }
  else
    // no value given, that means this was the final item in the line
    fprintf(stuff->fh, "\n");

  LEAVE();
}

///
/// ExportAddressBookCSVEntry
//  Exports an address book with comma or tab separated entries
static BOOL ExportAddressBookCSVEntry(const struct ABookNode *abn, BOOL first, const void *userData)
{
  struct CSVStuff *stuff = (struct CSVStuff *)userData;

  ENTER();

  switch(abn->type)
  {
    case AET_USER:
    {
      if(first == TRUE)
      {
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, abn->content.user.RealName);
        WriteCSVItem(stuff, abn->Alias);
        WriteCSVItem(stuff, abn->content.user.Address);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, abn->content.user.Phone);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, abn->content.user.Street);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, abn->content.user.City);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, ""); // postal code
        WriteCSVItem(stuff, abn->content.user.Country);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, abn->content.user.Homepage);
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, "");
        WriteCSVItem(stuff, NULL);
      }
    }
    break;

    case AET_GROUP:
    {
      // groups are handled by IterateAddressBook() already
    }
    break;

    default:
    {
      // lists cannot be exported to LDIF
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// ExportAddressBookCSV
//  Exports an address book with comma or tab separated entries
BOOL ExportAddressBookCSV(const char *filename, const struct AddressBook *abook, char delimiter)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    struct CSVStuff stuff;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    stuff.fh = fh;
    stuff.delimiter = delimiter;
    result = IterateAddressBook((struct AddressBook *)abook, ExportAddressBookCSVEntry, &stuff);

    fclose(fh);
    result = TRUE;
  }
  else
    ER_NewError(tr(MSG_ER_ADDRBOOKEXPORT), filename);

  RETURN(result);
  return result;
}

///

enum XMLSection
{
  xs_Unknown = 0,
  xs_Group,
  xs_Contact,
};

enum XMLData
{
  xd_Unknown = 0,
  xd_GroupName,
  xd_ContactName,
  xd_Description,
  xd_Address,
  xd_Alias,
  xd_PGPId,
  xd_Homepage,
  xd_Portrait,
  xd_Birthday,
  xd_Street,
  xd_City,
  xd_ZIPCode,
  xd_State,
  xd_Country,
  xd_Phone,
};

struct XMLUserData
{
  enum XMLSection section;
  enum XMLData dataType;
  struct AddressBook *abook;
  struct ABookNode abn;
  XML_Char xmlData[SIZE_LARGE];
  size_t xmlDataSize;
  BOOL success;
};

/// XMLStartHandler
// handle an XML start tag
static void XMLStartHandler(void *userData, const XML_Char *name, UNUSED const XML_Char **atts)
{
  struct XMLUserData *xmlUserData = userData;

  ENTER();

  if(strcmp(name, "newgroup") == 0)
  {
    xmlUserData->section = xs_Group;
  }
  else if(strcmp(name, "newcontact") == 0)
  {
    xmlUserData->section = xs_Contact;
  }
  else if(strcmp(name, "name") == 0)
  {
    switch(xmlUserData->section)
    {
      default:
      case xs_Group:
        // not yet supported
        xmlUserData->dataType = xd_Unknown;
        InitABookNode(&xmlUserData->abn, AET_GROUP);
      break;

      case xs_Contact:
        xmlUserData->dataType = xd_ContactName;
        InitABookNode(&xmlUserData->abn, AET_USER);
      break;
    }
  }
  else if(strcmp(name, "description") == 0)
  {
    xmlUserData->dataType = xd_Description;
  }
  else if(strcmp(name, "email") == 0)
  {
    xmlUserData->dataType = xd_Address;
  }
  else if(strcmp(name, "alias") == 0)
  {
    xmlUserData->dataType = xd_Alias;
  }
  else if(strcmp(name, "pgpid") == 0)
  {
    xmlUserData->dataType = xd_PGPId;
  }
  else if(strcmp(name, "homepage") == 0)
  {
    xmlUserData->dataType = xd_Homepage;
  }
  else if(strcmp(name, "portrait") == 0)
  {
    xmlUserData->dataType = xd_Portrait;
  }
  else if(strcmp(name, "birthday") == 0)
  {
    xmlUserData->dataType = xd_Birthday;
  }
  else if(strcmp(name, "street") == 0)
  {
    xmlUserData->dataType = xd_Street;
  }
  else if(strcmp(name, "city") == 0)
  {
    xmlUserData->dataType = xd_City;
  }
  else if(strcmp(name, "zip") == 0)
  {
    xmlUserData->dataType = xd_ZIPCode;
  }
  else if(strcmp(name, "state") == 0)
  {
    xmlUserData->dataType = xd_State;
  }
  else if(strcmp(name, "country") == 0)
  {
    xmlUserData->dataType = xd_Country;
  }
  else if(strcmp(name, "phone") == 0 || strcmp(name, "mobil") == 0 || strcmp(name, "fax") == 0)
  {
    xmlUserData->dataType = xd_Phone;
  }
  else
  {
    xmlUserData->dataType = xd_Unknown;
  }

  xmlUserData->xmlData[0] = '\0';
  xmlUserData->xmlDataSize = 0;

  LEAVE();
}

///
/// XMLEndHandler
// handle an XML end tag
static void XMLEndHandler(void *userData, const XML_Char *name)
{
  struct XMLUserData *xmlUserData = userData;
  char *isoStr;

  ENTER();

  // add the terminating NUL character
  xmlUserData->xmlData[xmlUserData->xmlDataSize] = '\0';

  // now convert this prossible UTF8 string to a normal string
  if((isoStr = CodesetsUTF8ToStr(CSA_Source,          Trim(xmlUserData->xmlData),
                                 CSA_DestCodeset,     G->localCodeset,
                                 CSA_MapForeignChars, C->MapForeignChars,
                                 TAG_DONE)) != NULL)
  {
    if(xmlUserData->section == xs_Group)
    {
    }
    else if(xmlUserData->section == xs_Contact)
    {
      // now fill the converted string into the entry
      // possible double entries (private, work, etc) will be merged
      switch(xmlUserData->dataType)
      {
        case xd_ContactName:
        {
          strlcpy(xmlUserData->abn.content.user.RealName, isoStr, sizeof(xmlUserData->abn.content.user.RealName));
        }
        break;

        case xd_Description:
        {
          strlcpy(xmlUserData->abn.Comment, isoStr, sizeof(xmlUserData->abn.Comment));
        }
        break;

        case xd_Address:
        {
          strlcpy(xmlUserData->abn.content.user.Address, isoStr, sizeof(xmlUserData->abn.content.user.Address));
        }
        break;

        case xd_Alias:
        {
          strlcpy(xmlUserData->abn.Alias, isoStr, sizeof(xmlUserData->abn.Alias));
        }
        break;

        case xd_PGPId:
        {
          strlcpy(xmlUserData->abn.content.user.PGPId, isoStr, sizeof(xmlUserData->abn.content.user.PGPId));
        }
        break;

        case xd_Homepage:
        {
          strlcpy(xmlUserData->abn.content.user.Homepage, isoStr, sizeof(xmlUserData->abn.content.user.Homepage));
        }
        break;

        case xd_Portrait:
        {
          strlcpy(xmlUserData->abn.content.user.Photo, isoStr, sizeof(xmlUserData->abn.content.user.Photo));
        }
        break;

        case xd_Birthday:
        {
          LONG day = 0;
          LONG month = 0;
          LONG year = 0;
          char *p = isoStr;
          char *q;

          if((q = strchr(p, '/')) != NULL)
          {
            *q++ = '\0';
            month = atol(p);
            p = q;
          }
          if((q = strchr(p, '/')) != NULL)
          {
            *q++ = '\0';
            day = atol(p);
            p = q;
          }
          year = atol(p);

          xmlUserData->abn.content.user.Birthday = day * 1000000 + month * 10000 + year;
        }
        break;

        case xd_Street:
        {
          if(xmlUserData->abn.content.user.Street[0] == '\0')
          {
            strlcpy(xmlUserData->abn.content.user.Street, isoStr, sizeof(xmlUserData->abn.content.user.Street));
          }
          else
          {
            strlcat(xmlUserData->abn.content.user.Street, ", ", sizeof(xmlUserData->abn.content.user.Street));
            strlcat(xmlUserData->abn.content.user.Street, isoStr, sizeof(xmlUserData->abn.content.user.Street));
          }
        }
        break;

        case xd_City:
        {
          if(xmlUserData->abn.content.user.City[0] == '\0')
          {
            strlcpy(xmlUserData->abn.content.user.City, isoStr, sizeof(xmlUserData->abn.content.user.City));
          }
          else
          {
            strlcat(xmlUserData->abn.content.user.City, ", ", sizeof(xmlUserData->abn.content.user.City));
            strlcat(xmlUserData->abn.content.user.City, isoStr, sizeof(xmlUserData->abn.content.user.City));
          }
        }
        break;

        case xd_ZIPCode:
        {
          if(xmlUserData->abn.content.user.City[0] == '\0')
          {
            strlcpy(xmlUserData->abn.content.user.City, isoStr, sizeof(xmlUserData->abn.content.user.City));
          }
          else
          {
            char tmp[SIZE_DEFAULT];

            strlcpy(tmp, xmlUserData->abn.content.user.City, sizeof(tmp));
            snprintf(xmlUserData->abn.content.user.City, sizeof(xmlUserData->abn.content.user.City), "%s %s", isoStr, tmp);
          }
        }
        break;

        case xd_State:
        case xd_Country:
        {
          if(xmlUserData->abn.content.user.Country[0] == '\0')
          {
            strlcpy(xmlUserData->abn.content.user.Country, isoStr, sizeof(xmlUserData->abn.content.user.Country));
          }
          else
          {
            strlcat(xmlUserData->abn.content.user.Country, ", ", sizeof(xmlUserData->abn.content.user.Country));
            strlcat(xmlUserData->abn.content.user.Country, isoStr, sizeof(xmlUserData->abn.content.user.Country));
          }
        }
        break;

        case xd_Phone:
        {
          if(xmlUserData->abn.content.user.Phone[0] == '\0')
          {
            strlcpy(xmlUserData->abn.content.user.Phone, isoStr, sizeof(xmlUserData->abn.content.user.Phone));
          }
          else
          {
            strlcat(xmlUserData->abn.content.user.Phone, ", ", sizeof(xmlUserData->abn.content.user.Phone));
            strlcat(xmlUserData->abn.content.user.Phone, isoStr, sizeof(xmlUserData->abn.content.user.Phone));
          }
        }
        break;

        default:
        {
          // ignore this item
        }
        break;
      }
    }

    CodesetsFreeA(isoStr, NULL);
  }

  // add the entry to our address book if it was a normal contact information
  if(xmlUserData->section == xs_Contact && strcmp(name, "newcontact") == 0)
  {
    // we need at least an EMail address
    if(xmlUserData->abn.content.user.Address[0] != '\0')
    {
      struct ABookNode *abn;

      // set up an alias only if none is given
      if(xmlUserData->abn.Alias[0] == '\0')
        SetDefaultAlias(&xmlUserData->abn);

      if((abn = DuplicateNode(&xmlUserData->abn, sizeof(xmlUserData->abn))) != NULL)
      {
        // put it into the tree
        AddTail((struct List *)&xmlUserData->abook->root, (struct Node *)abn);
        xmlUserData->success = TRUE;
      }
    }
  }

  LEAVE();
}

///
/// XMLCharacterDataHandler
// handle the XML character data
static void XMLCharacterDataHandler(void *userData, const XML_Char *s, int len)
{
  struct XMLUserData *xmlUserData = userData;

  ENTER();

  // does the string still fit in our buffer?
  if(xmlUserData->xmlDataSize < sizeof(xmlUserData->xmlData))
  {
    strncpy(&xmlUserData->xmlData[xmlUserData->xmlDataSize], s, MIN(sizeof(xmlUserData->xmlData) - xmlUserData->xmlDataSize, (size_t)len));
  }
  // add the size nevertheless
  xmlUserData->xmlDataSize += len;

  LEAVE();
}

///
/// ImportAddressBookXML
// imports an address book in XML format (i.e. from SimpleMail)
BOOL ImportAddressBookXML(const char *filename, struct AddressBook *abook, BOOL append)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "r")) != NULL)
  {
    XML_Parser parser;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(append == FALSE)
      ClearAddressBook(abook);

    // create the XML parser
    if((parser = XML_ParserCreate(NULL)) != NULL)
    {
      struct XMLUserData xmlUserData;
      char *buffer = NULL;
      size_t size = 0;

      xmlUserData.section = xs_Unknown;
      xmlUserData.dataType = xd_Unknown;
      xmlUserData.abook = abook;
      xmlUserData.success = FALSE;

      XML_SetElementHandler(parser, XMLStartHandler, XMLEndHandler);
      XML_SetCharacterDataHandler(parser, XMLCharacterDataHandler);
      XML_SetUserData(parser, &xmlUserData);

      // now parse the file line by line
      while(GetLine(&buffer, &size, fh) >= 0)
      {
        if(XML_Parse(parser, buffer, strlen(buffer), FALSE) == XML_STATUS_ERROR)
        {
          result = FALSE;
          break;
        }
      }
      if(result == TRUE)
      {
        // if everything went fine we need one final parsing step in case
        // there are some characters left in the parsing pipeline.
        if(XML_Parse(parser, "", 0, TRUE) == XML_STATUS_ERROR)
        {
          result = FALSE;
        }
      }

      free(buffer);

      // free the parser again
      XML_ParserFree(parser);

      result = xmlUserData.success;
    }

    fclose(fh);
  }
  else
     ER_NewError(tr(MSG_ER_ADDRBOOKIMPORT), filename);

  if(result == TRUE)
  {
    // now remember the "modified" state
    abook->modified = append;
  }

  RETURN(result);
  return result;
}

///

struct SearchStuff
{
  const char *text;
  size_t textLen;
  int mode;
  int mode_type;
  struct ABookNode **result;
  int hits;
};

/// SearchAddressBookEntry
BOOL SearchAddressBookEntry(const struct ABookNode *abn, BOOL first, const void *userData)
{
  struct SearchStuff *stuff = (struct SearchStuff *)userData;
  BOOL result = TRUE;

  ENTER();

  if(first == TRUE)
  {
    BOOL found = FALSE;

    // now we check if this entry is one of the not wished entry types
    // and then we skip it.
    if(abn->type == AET_USER && !isUserSearch(stuff->mode))
      goto out;
    if(abn->type == AET_LIST && !isListSearch(stuff->mode))
      goto out;
    if(abn->type == AET_GROUP && !isGroupSearch(stuff->mode))
      goto out;

    if(isCompleteSearch(stuff->mode))
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(stuff->mode_type))
        found = !Strnicmp(abn->Alias, stuff->text, stuff->textLen);
      else if(isRealNameSearch(stuff->mode_type))
        found = !Strnicmp(abn->content.user.RealName, stuff->text, stuff->textLen);
      else if(isAddressSearch(stuff->mode_type))
        found = !Strnicmp(abn->content.user.Address, stuff->text, stuff->textLen);
    }
    else
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(isAliasSearch(stuff->mode_type))
        found = !Stricmp(abn->Alias, stuff->text);
      else if(isRealNameSearch(stuff->mode_type))
        found = !Stricmp(abn->content.user.RealName, stuff->text);
      else if(isAddressSearch(stuff->mode_type))
        found = !Stricmp(abn->content.user.Address, stuff->text);
    }

    if(found == TRUE)
    {
      *stuff->result = (struct ABookNode *)abn;
      stuff->hits++;

      // we scan until we are at the end of the list or
      // if we found more then one matching entry
      if(stuff->hits >= 2)
        result = FALSE;
    }
  }

out:
  RETURN(result);
  return result;
}

///
/// SearchAddressBook
//  Searches the address book by alias, name or address
//  it will break if there is more then one entry
int SearchAddressBook(const struct AddressBook *abook, const char *text, int mode, struct ABookNode **abn)
{
  struct SearchStuff stuff;
  int hits;

  ENTER();

  stuff.text = text;
  stuff.textLen = strlen(text);
  stuff.mode = mode;
  stuff.mode_type = mode & ASM_TYPEMASK;;
  stuff.result = abn;
  stuff.hits = 0;
  IterateAddressBook((struct AddressBook *)abook, SearchAddressBookEntry, &stuff);

  hits = stuff.hits;

  RETURN(hits);
  return hits;
}

///

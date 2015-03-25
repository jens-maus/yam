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

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include <clib/alib_protos.h>
#include <proto/codesets.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/expat.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/timer.h>
#include <proto/utility.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "mime/base64.h"
#include "mui/BirthdayRequestWindow.h"
#include "mui/ClassesExtra.h"

#include "AddressBook.h"
#include "Config.h"
#include "DynamicString.h"
#include "FileInfo.h"
#include "Locale.h"
#include "Logfile.h"
#include "Requesters.h"
#include "Timer.h"

#include "Debug.h"

static void ClearABookGroup(struct ABookNode *group);

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

  NewMinList(&abn->GroupMembers);

  LEAVE();
}

///
/// DeleteABookNode
void DeleteABookNode(struct ABookNode *abn)
{
  ENTER();

  if(abn->type == ABNT_GROUP)
  {
    ClearABookGroup(abn);
  }
  else if(abn->type == ABNT_LIST)
  {
    D(DBF_ABOOK, "free members of list '%s'", abn->Alias);
    dstrfree(abn->ListMembers);
  }

  FreeSysObject(ASOT_NODE, abn);

  LEAVE();
}

///
/// AddABookNode
void AddABookNode(struct ABookNode *group, struct ABookNode *member, struct ABookNode *afterThis)
{
  ENTER();

  if(afterThis == NULL)
    AddHead((struct List *)&group->GroupMembers, (struct Node *)member);
  else
    Insert((struct List *)&group->GroupMembers, (struct Node *)member, (struct Node *)afterThis);

  LEAVE();
}

///
/// RemoveABookNode
void RemoveABookNode(struct ABookNode *member)
{
  ENTER();

  Remove((struct Node *)member);

  LEAVE();
}

///
/// MoveABookNode
void MoveABookNode(struct ABookNode *group, struct ABookNode *member, struct ABookNode *afterThis)
{
  ENTER();

  D(DBF_ABOOK, "move '%s' behind '%s' in group '%s'", member->Alias, afterThis != NULL ? afterThis->Alias : "<NULL>", group->Alias);
  // first remove the node from the list
  RemoveABookNode(member);
  // then insert it again at the desired position
  AddABookNode(group, member, afterThis);

  LEAVE();
}

///
/// CompareABookNodes
BOOL CompareABookNodes(const struct ABookNode *abn1, const struct ABookNode *abn2)
{
  BOOL equal = TRUE;

  ENTER();

  if(abn1->type == abn2->type)
  {
    switch(abn1->type)
    {
      case ABNT_USER:
      {
        if(strcmp(abn1->Alias, abn2->Alias) != 0 ||
           strcmp(abn1->RealName, abn2->RealName) != 0 ||
           strcmp(abn1->Address, abn2->Address) != 0 ||
           strcmp(abn1->Comment, abn2->Comment) != 0 ||
           strcmp(abn1->PGPId, abn2->PGPId) != 0 ||
           strcmp(abn1->Street, abn2->Street) != 0 ||
           strcmp(abn1->City, abn2->City) != 0 ||
           strcmp(abn1->Country, abn2->Country) != 0 ||
           strcmp(abn1->Phone, abn2->Phone) != 0 ||
           strcmp(abn1->Photo, abn2->Photo) != 0 ||
           abn1->Birthday != abn2->Birthday ||
           abn1->DefSecurity != abn2->DefSecurity)
        {
          equal = FALSE;
        }
      }
      break;

      case ABNT_GROUP:
      {
        if(strcmp(abn1->Alias, abn2->Alias) != 0 ||
           strcmp(abn1->Comment, abn2->Comment) != 0)
        {
          equal = FALSE;
        }
      }
      break;

      case ABNT_LIST:
      {
        if(strcmp(abn1->Alias, abn2->Alias) != 0 ||
           strcmp(abn1->RealName, abn2->RealName) != 0 ||
           strcmp(abn1->Address, abn2->Address) != 0 ||
           strcmp(abn1->Comment, abn2->Comment) != 0 ||
           strcmp(abn1->ListMembers != NULL ? abn1->ListMembers : (char *)"", abn2->ListMembers != NULL ? abn2->ListMembers : (char *)"") != 0)
        {
          equal = FALSE;
        }
      }
      break;

      default:
      {
        equal = FALSE;
      }
      break;
    }
  }
  else
  {
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// InitABook
void InitABook(struct ABook *abook, const char *name)
{
  ENTER();

  InitABookNode(&abook->rootGroup, ABNT_GROUP);
  strlcpy(abook->rootGroup.Alias, name != NULL ? name : "root", sizeof(abook->rootGroup.Alias));
  abook->modified = FALSE;

  LEAVE();
}

///
/// ClearABookGroup
static void ClearABookGroup(struct ABookNode *group)
{
  struct ABookNode *abn;
  struct ABookNode *next;

  ENTER();

  D(DBF_ABOOK, "free members of group %08lx '%s'", group, group->Alias);
  SafeIterateList(&group->GroupMembers, struct ABookNode *, abn, next)
  {
    D(DBF_ABOOK, "free member %08lx '%s' '%s'", abn, abn->Alias, abn->Address);
    DeleteABookNode(abn);
  }
  NewMinList(&group->GroupMembers);

  LEAVE();
}

///
/// ClearABook
void ClearABook(struct ABook *abook)
{
  ENTER();

  ClearABookGroup(&abook->rootGroup);
  InitABook(abook, NULL);

  LEAVE();
}

///
/// MoveABookNodes
void MoveABookNodes(struct ABook *dst, struct ABook *src)
{
  ENTER();

  MoveList((struct List *)&dst->rootGroup.GroupMembers, (struct List *)&src->rootGroup.GroupMembers);

  LEAVE();
}

///
/// FixAlias
//  Avoids ambiguos aliases
void FixAlias(const struct ABook *abook, struct ABookNode *abn, const struct ABookNode *excludeThis)
{
  char alias[SIZE_NAME];
  int count = 1;
  struct ABookNode *found = NULL;
  BOOL useAlias = TRUE;

  ENTER();

  strlcpy(alias, abn->Alias, sizeof(alias));

  while(SearchABook(abook, alias, ASM_ALIAS|ASM_USER|ASM_LIST|ASM_GROUP, &found) != 0)
  {
    size_t len;

    if(found == excludeThis)
    {
      useAlias = FALSE;
      break;
    }

    if((len = strlen(abn->Alias)) > sizeof(alias)-2)
      len = sizeof(alias)-2;

    snprintf(&alias[len], sizeof(alias)-len, "%d", ++count);
  }

  if(useAlias == TRUE)
  {
    // copy the modified string back
    strlcpy(abn->Alias, alias, sizeof(abn->Alias));
  }

  LEAVE();
}

///
/// SetDefaultAlias
//  Creates an alias from the real name if user left it empty
void SetDefaultAlias(struct ABookNode *abn)
{
  char *p;

  ENTER();

  memset(abn->Alias, 0, sizeof(abn->Alias));
  p = abn->Alias;
  if(abn->RealName[0] != '\0')
  {
    char *ln;

    if((ln = strrchr(abn->RealName, ' ')) != NULL)
    {
      if(isAlNum(abn->RealName[0]))
      {
        *p++ = abn->RealName[0];
        *p++ = '_';
      }
      ln++;
    }
    else
      ln = abn->RealName;

    for(; strlen(abn->Alias) < SIZE_NAME-2 && ln[0] != '\0'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }
  else
  {
    char *ln;

    for(ln = abn->Address; strlen(abn->Alias) < SIZE_NAME-2 && *ln != '\0' && *ln != '@'; ln++)
    {
      if(isAlNum(*ln))
        *p++ = *ln;
    }
  }

  LEAVE();
}

///
/// IterateABookGroup
BOOL IterateABookGroup(const struct ABookNode *group, ULONG flags, BOOL (*nodeFunc)(const struct ABookNode *abn, ULONG flags, void *userData), void *userData)
{
  struct ABookNode *abn;
  BOOL result = TRUE;

  ENTER();

  IterateList(&group->GroupMembers, struct ABookNode *, abn)
  {
    // handle groups in a recursive fashion first
    if(abn->type == ABNT_GROUP)
    {
      if((result = nodeFunc(abn, IABF_FIRST_GROUP_VISIT, userData)) == FALSE)
        break;

      if((result = IterateABookGroup(abn, flags, nodeFunc, userData)) == FALSE)
        break;

      if(isFlagSet(flags, IABF_VISIT_GROUPS_TWICE))
      {
        if((result = nodeFunc(abn, IABF_SECOND_GROUP_VISIT, userData)) == FALSE)
          break;
      }
    }
    else
    {
      if((result = nodeFunc(abn, 0, userData)) == FALSE)
        break;
    }
  }

  RETURN(result);
  return result;
}

///
/// IterateABook
BOOL IterateABook(const struct ABook *abook, ULONG flags, BOOL (*nodeFunc)(const struct ABookNode *abn, ULONG flags, void *userData), void *userData)
{
  BOOL result;

  ENTER();

  result = IterateABookGroup(&abook->rootGroup, flags, nodeFunc, userData);

  RETURN(result);
  return result;
}

///
/// CreateEmptyABookFile
// create an empty address book
BOOL CreateEmptyABookFile(const char *filename)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    // write at least the header, this is required for a valid .addressbook file
    fputs("YAB4 - YAM Addressbook\n", fh);
    fclose(fh);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), filename);

  RETURN(result);
  return result;
}

///
/// LoadABook
BOOL LoadABook(const char *filename, struct ABook *abook, BOOL append)
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
      struct ABookNode *parent[8];
      struct ABookNode *afterThis[8];

      parent[0] = &abook->rootGroup;
      afterThis[0] = NULL;

      if(strncmp(buffer,"YAB",3) == 0)
      {
        int version = buffer[3] - '0';

        if(append == FALSE)
          ClearABook(abook);

        while(GetLine(&buffer, &size, fh) >= 0)
        {
          struct ABookNode *abn;

          if(strncmp(buffer, "@USER", 5) == 0)
          {
            if((abn = CreateABookNode(ABNT_USER)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[6]), sizeof(abn->Alias));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Address, Trim(buffer), sizeof(abn->Address));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->RealName, Trim(buffer), sizeof(abn->RealName));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              if(version > 2)
              {
                GetLine(&buffer, &size, fh);
                strlcpy(abn->Phone, Trim(buffer), sizeof(abn->Phone));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->Street, Trim(buffer), sizeof(abn->Street));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->City, Trim(buffer), sizeof(abn->City));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->Country, Trim(buffer), sizeof(abn->Country));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->PGPId, Trim(buffer), sizeof(abn->PGPId));
                GetLine(&buffer, &size, fh);
                abn->Birthday = atol(Trim(buffer));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->Photo, Trim(buffer), sizeof(abn->Photo));
                GetLine(&buffer, &size, fh);
                if(strcmp(buffer, "@ENDUSER") == 0)
                  strlcpy(abn->Homepage, Trim(buffer), sizeof(abn->Homepage));
              }
              if(version > 3)
              {
                GetLine(&buffer, &size, fh);
                abn->DefSecurity = atoi(Trim(buffer));
              }

              // skip any additional lines
              do
              {
                if(strcmp(buffer, "@ENDUSER") == 0)
                  break;
              }
              while(GetLine(&buffer, &size, fh) >= 0);

              AddABookNode(parent[nested], abn, afterThis[nested]);
              afterThis[nested] = abn;
            }
          }
          else if(strncmp(buffer, "@LIST", 5) == 0)
          {
            if((abn = CreateABookNode(ABNT_LIST)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[6]), sizeof(abn->Alias));
              if(version > 2)
              {
                GetLine(&buffer, &size, fh);
                strlcpy(abn->Address, Trim(buffer), sizeof(abn->Address));
                GetLine(&buffer, &size, fh);
                strlcpy(abn->RealName, Trim(buffer), sizeof(abn->RealName));
              }
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              while(GetLine(&buffer, &size, fh) >= 0)
              {
                if(strcmp(buffer, "@ENDLIST") == 0)
                  break;

                if(*buffer == '\0')
                  continue;

                dstrcat(&abn->ListMembers, buffer);
                dstrcat(&abn->ListMembers, "\n");
              }

              AddABookNode(parent[nested], abn, afterThis[nested]);
              afterThis[nested] = abn;
            }
          }
          else if(strncmp(buffer, "@GROUP", 6) == 0)
          {
            if((abn = CreateABookNode(ABNT_GROUP)) != NULL)
            {
              strlcpy(abn->Alias, Trim(&buffer[7]), sizeof(abn->Alias));
              GetLine(&buffer, &size, fh);
              strlcpy(abn->Comment, Trim(buffer), sizeof(abn->Comment));
              AddABookNode(parent[nested], abn, afterThis[nested]);
              afterThis[nested] = abn;

              nested++;
              parent[nested] = abn;
              afterThis[nested] = NULL;
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
          struct ABookNode *lastABN = NULL;

          if(append == FALSE)
            ClearABook(abook);

          fseek(fh, 0, SEEK_SET);
          while(GetLine(&buffer, &size, fh) >= 0)
          {
            struct ABookNode *abn;

            if((abn = CreateABookNode(ABNT_USER)) != NULL)
            {
              char *p, *p2;

              if((p = strchr(buffer, ' ')) != NULL)
                *p = '\0';
              strlcpy(abn->Address, buffer, sizeof(abn->Address));
              if(p != NULL)
              {
                strlcpy(abn->RealName, ++p, sizeof(abn->RealName));
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

              AddABookNode(parent[nested], abn, lastABN);
              lastABN = abn;
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
/// SaveABookEntry
static BOOL SaveABookEntry(const struct ABookNode *abn, ULONG flags, void *userData)
{
  FILE *fh = (FILE *)userData;

  ENTER();

  switch(abn->type)
  {
    case ABNT_USER:
    {
      fprintf(fh, "@USER %s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%08ld\n%s\n%s\n%d\n@ENDUSER\n",
                  abn->Alias, abn->Address, abn->RealName, abn->Comment,
                  abn->Phone, abn->Street, abn->City, abn->Country, abn->PGPId, abn->Birthday, abn->Photo, abn->Homepage, abn->DefSecurity);
    }
    break;

    case ABNT_LIST:
    {
      fprintf(fh, "@LIST %s\n%s\n%s\n%s\n%s\n@ENDLIST\n", abn->Alias, abn->Address, abn->RealName, abn->Comment, abn->ListMembers != NULL ? abn->ListMembers : "");
    }
    break;

    case ABNT_GROUP:
    {
      if(isFlagSet(flags, IABF_FIRST_GROUP_VISIT))
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
/// SaveABook
BOOL SaveABook(const char *filename, const struct ABook *abook)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fputs("YAB4 - YAM Addressbook\n", fh);
    result = IterateABook((struct ABook *)abook, IABF_VISIT_GROUPS_TWICE, SaveABookEntry, fh);
    fclose(fh);
    AppendToLogfile(LF_VERBOSE, 70, tr(MSG_LOG_SavingABook), filename);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), filename);

  RETURN(result);
  return result;
}

///
/// ImportLDIFABook
//  Imports an address book in LDIF format
BOOL ImportLDIFABook(const char *filename, struct ABook *abook, BOOL append)
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
      ClearABook(abook);

    InitABookNode(&abn, ABNT_USER);

    while(GetLine(&buffer, &size, fh) >= 0)
    {
      // an empty line separates two user entries
      if(buffer[0] == '\0')
      {
        // we need at least an EMail address
        if(abn.Address[0] != '\0')
        {
          struct ABookNode *node;

          // set up an alias only if none is given
          if(abn.Alias[0] == '\0')
            SetDefaultAlias(&abn);

          if((node = DuplicateNode(&abn, sizeof(abn))) != NULL)
          {
            // put it into the tree
            AddABookNode(&abook->rootGroup, node, NULL);
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
              InitABookNode(&abn, ABNT_USER);
            }
            else if(stricmp(key, "cn") == 0)                         // complete name
              strlcpy(abn.RealName, value, sizeof(abn.RealName));
            else if(stricmp(key, "mail") == 0)                       // mail address
              strlcpy(abn.Address, value, sizeof(abn.Address));
            else if(stricmp(key, "mozillaNickname") == 0)            // alias
              strlcpy(abn.Alias, value, sizeof(abn.Alias));
            else if(stricmp(key, "telephoneNumber") == 0)            // phone number
            {
              if(abn.Phone[0] != '\0')
                strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, value, sizeof(abn.Phone));
            }
            else if(stricmp(key, "homePhone") == 0)                  // phone number
            {
              if(abn.Phone[0] != '\0')
                strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, value, sizeof(abn.Phone));
            }
            else if(stricmp(key, "fax") == 0)                        // fax number
            {
              if(abn.Phone[0] != '\0')
              strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, value, sizeof(abn.Phone));
            }
            else if(stricmp(key, "pager") == 0)                      // pager number
            {
              if(abn.Phone[0] != '\0')
                strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, value, sizeof(abn.Phone));
            }
            else if(stricmp(key, "mobile") == 0)                     // mobile number
            {
              if(abn.Phone[0] != '\0')
                strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, value, sizeof(abn.Phone));
            }
            else if(stricmp(key, "homeStreet") == 0)                 // office street
            {
              if(abn.Street[0] != '\0')
                strlcat(abn.Street, ", ", sizeof(abn.Street));
              strlcat(abn.Street, value, sizeof(abn.Street));
            }
            else if(stricmp(key, "mozillaHomeStreet2") == 0)         // home street
            {
              if(abn.Street[0] != '\0')
                strlcat(abn.Street, ", ", sizeof(abn.Street));
              strlcat(abn.Street, value, sizeof(abn.Street));
            }
            else if(stricmp(key, "l") == 0)                          // office locality
            {
              if(abn.City[0] != '\0')
                strlcat(abn.City, ", ", sizeof(abn.City));
              strlcat(abn.City, value, sizeof(abn.City));
            }
            else if(stricmp(key, "mozillaHomeLocalityName") == 0)    // home locality
            {
              if(abn.City[0] != '\0')
                strlcat(abn.City, ", ", sizeof(abn.City));
              strlcat(abn.City, value, sizeof(abn.City));
            }
            else if(stricmp(key, "postalCode") == 0)                 // office postal code
            {
              if(abn.City[0] != '\0')
                strlcat(abn.City, ", ", sizeof(abn.City));
              strlcat(abn.City, value, sizeof(abn.City));
            }
            else if(stricmp(key, "mozillaHomePostalCode") == 0)      // home postal code
            {
              if(abn.City[0] != '\0')
                strlcat(abn.City, ", ", sizeof(abn.City));
              strlcat(abn.City, value, sizeof(abn.City));
            }
            else if(stricmp(key, "c") == 0)                          // office country
            {
              if(abn.Country[0] != '\0')
                strlcat(abn.Country, ", ", sizeof(abn.Country));
              strlcat(abn.Country, value, sizeof(abn.Country));
            }
            else if(stricmp(key, "mozillaHomeCountryName") == 0)     // home country
            {
              if(abn.Country[0] != '\0')
                strlcat(abn.Country, ", ", sizeof(abn.Country));
              strlcat(abn.Country, value, sizeof(abn.Country));
            }
            else if(stricmp(key, "mozillaWorkUrl") == 0)             // working home page
            {
              if(abn.Homepage[0] != '\0')
                strlcat(abn.Homepage, ", ", sizeof(abn.Homepage));
              strlcat(abn.Homepage, value, sizeof(abn.Homepage));
            }
            else if(stricmp(key, "mozillaHomeUrl") == 0)             // private homepage
            {
              if(abn.Homepage[0] != '\0')
                strlcat(abn.Homepage, ", ", sizeof(abn.Homepage));
              strlcat(abn.Homepage, value, sizeof(abn.Homepage));
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
/// ExportLDIFABookEntry
//  Exports an address book as LDIF file
static BOOL ExportLDIFABookEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  FILE *fh = (FILE *)userData;

  ENTER();

  switch(abn->type)
  {
    case ABNT_USER:
    {
      WriteLDIFLine(fh, "dn", "cn=%s,mail=%s", abn->RealName, abn->Address);
      WriteLDIFLine(fh, "objectClass", "top");
      WriteLDIFLine(fh, "objectClass", "person");
      WriteLDIFLine(fh, "objectClass", "organizationalPerson");
      WriteLDIFLine(fh, "objectClass", "inetOrdPerson");
      WriteLDIFLine(fh, "objectClass", "mozillaAbPersonAlpha");
      WriteLDIFLine(fh, "cn", "%s", abn->RealName);
      WriteLDIFLine(fh, "mail", "%s", abn->Address);
      if(abn->Alias[0] != '\0')
        WriteLDIFLine(fh, "mozillaNickname", "%s", abn->Alias);
      if(abn->Phone[0] != '\0')
        WriteLDIFLine(fh, "telephoneNumber", "%s", abn->Phone);
      if(abn->Street[0] != '\0')
        WriteLDIFLine(fh, "street", "%s", abn->Street);
      if(abn->City[0] != '\0')
        WriteLDIFLine(fh, "l", "%s", abn->City);
      if(abn->Country[0] != '\0')
        WriteLDIFLine(fh, "c", "%s", abn->Country);
      if(abn->Homepage[0] != '\0')
        WriteLDIFLine(fh, "mozillaHomeUrl", "%s", abn->Homepage);
      WriteLDIFLine(fh, "", "");
    }
    break;

    case ABNT_GROUP:
    {
      // groups are handled by IterateABook() already
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
/// ExportLDIFABook
//  Exports an address book as LDIF file
BOOL ExportLDIFABook(const char *filename, const struct ABook *abook)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    result = IterateABook(abook, 0, ExportLDIFABookEntry, fh);

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

/// ImportCSVABook
//  Imports an address book with comma or tab separated entries
BOOL ImportCSVABook(const char *filename, struct ABook *abook, BOOL append, char delimiter)
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
      ClearABook(abook);

    delimStr[0] = delimiter;
    delimStr[1] = '\0';

    while(GetLine(&buffer, &size, fh) >= 0)
    {
      struct ABookNode abn;
      char *item = buffer;
      int itemNumber = 0;

      InitABookNode(&abn, ABNT_USER);

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
            abn.Address[0] = '\0';
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
              strlcat(abn.RealName, item, sizeof(abn.RealName));
            }
            break;

            // last name
            case 2:
            {
              if(abn.RealName[0] != '\0')
                strlcat(abn.RealName, " ", sizeof(abn.RealName));
              strlcat(abn.RealName, item, sizeof(abn.RealName));
            }
            break;

            // complete name, preferred, if available
            case 3:
            {
              strlcpy(abn.RealName, item, sizeof(abn.RealName));
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
              strlcpy(abn.Address, item, sizeof(abn.Address));
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
              if(abn.Phone[0] != '\0')
                strlcat(abn.Phone, ", ", sizeof(abn.Phone));
              strlcat(abn.Phone, item, sizeof(abn.Phone));
            }
            break;

            case 12: // address, part 1
            case 13: // address, part 2
            {
              if(abn.Street[0] != '\0')
                strlcat(abn.Street, " ", sizeof(abn.Street));
              strlcat(abn.Street, item, sizeof(abn.Street));
            }
            break;

            // city
            case 14:
            {
              strlcpy(abn.City, item, sizeof(abn.City));
            }
            break;

            // province, ignored
            case 15:
              // nothing
            break;

            // ZIP code, append it to the city name
            case 16:
            {
              if(abn.City[0] != '\0')
                strlcat(abn.City, ", ", sizeof(abn.City));
              strlcat(abn.City, item, sizeof(abn.City));
            }
            break;

            // country
            case 17:
            {
              strlcpy(abn.Country, item, sizeof(abn.Country));
            }
            break;

            case 27: // office web address
            case 28: // private web address
            {
              strlcpy(abn.Homepage, item, sizeof(abn.Homepage));
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
      if(abn.Address[0] != '\0')
      {
        struct ABookNode *node;

        // set up an alias only if none is given
        if(abn.Alias[0] == '\0')
          SetDefaultAlias(&abn);


        if((node = DuplicateNode(&abn, sizeof(abn))) != NULL)
        {
          AddABookNode(&abook->rootGroup, node, NULL);
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
/// ExportCSVABookEntry
//  Exports an address book with comma or tab separated entries
static BOOL ExportCSVABookEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct CSVStuff *stuff = (struct CSVStuff *)userData;

  ENTER();

  switch(abn->type)
  {
    case ABNT_USER:
    {
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, abn->RealName);
      WriteCSVItem(stuff, abn->Alias);
      WriteCSVItem(stuff, abn->Address);
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, abn->Phone);
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, abn->Street);
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, abn->City);
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, ""); // postal code
      WriteCSVItem(stuff, abn->Country);
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
      WriteCSVItem(stuff, abn->Homepage);
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, "");
      WriteCSVItem(stuff, NULL);
    }
    break;

    case ABNT_GROUP:
    {
      // groups are handled by IterateABook() already
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
/// ExportCSVABook
//  Exports an address book with comma or tab separated entries
BOOL ExportCSVABook(const char *filename, const struct ABook *abook, char delimiter)
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
    result = IterateABook(abook, 0, ExportCSVABookEntry, &stuff);

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
  struct ABook *abook;
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
        InitABookNode(&xmlUserData->abn, ABNT_GROUP);
      break;

      case xs_Contact:
        xmlUserData->dataType = xd_ContactName;
        InitABookNode(&xmlUserData->abn, ABNT_USER);
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
          strlcpy(xmlUserData->abn.RealName, isoStr, sizeof(xmlUserData->abn.RealName));
        }
        break;

        case xd_Description:
        {
          strlcpy(xmlUserData->abn.Comment, isoStr, sizeof(xmlUserData->abn.Comment));
        }
        break;

        case xd_Address:
        {
          strlcpy(xmlUserData->abn.Address, isoStr, sizeof(xmlUserData->abn.Address));
        }
        break;

        case xd_Alias:
        {
          strlcpy(xmlUserData->abn.Alias, isoStr, sizeof(xmlUserData->abn.Alias));
        }
        break;

        case xd_PGPId:
        {
          strlcpy(xmlUserData->abn.PGPId, isoStr, sizeof(xmlUserData->abn.PGPId));
        }
        break;

        case xd_Homepage:
        {
          strlcpy(xmlUserData->abn.Homepage, isoStr, sizeof(xmlUserData->abn.Homepage));
        }
        break;

        case xd_Portrait:
        {
          strlcpy(xmlUserData->abn.Photo, isoStr, sizeof(xmlUserData->abn.Photo));
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

          xmlUserData->abn.Birthday = day * 1000000 + month * 10000 + year;
        }
        break;

        case xd_Street:
        {
          if(xmlUserData->abn.Street[0] == '\0')
          {
            strlcpy(xmlUserData->abn.Street, isoStr, sizeof(xmlUserData->abn.Street));
          }
          else
          {
            strlcat(xmlUserData->abn.Street, ", ", sizeof(xmlUserData->abn.Street));
            strlcat(xmlUserData->abn.Street, isoStr, sizeof(xmlUserData->abn.Street));
          }
        }
        break;

        case xd_City:
        {
          if(xmlUserData->abn.City[0] == '\0')
          {
            strlcpy(xmlUserData->abn.City, isoStr, sizeof(xmlUserData->abn.City));
          }
          else
          {
            strlcat(xmlUserData->abn.City, ", ", sizeof(xmlUserData->abn.City));
            strlcat(xmlUserData->abn.City, isoStr, sizeof(xmlUserData->abn.City));
          }
        }
        break;

        case xd_ZIPCode:
        {
          if(xmlUserData->abn.City[0] == '\0')
          {
            strlcpy(xmlUserData->abn.City, isoStr, sizeof(xmlUserData->abn.City));
          }
          else
          {
            char tmp[SIZE_DEFAULT];

            strlcpy(tmp, xmlUserData->abn.City, sizeof(tmp));
            snprintf(xmlUserData->abn.City, sizeof(xmlUserData->abn.City), "%s %s", isoStr, tmp);
          }
        }
        break;

        case xd_State:
        case xd_Country:
        {
          if(xmlUserData->abn.Country[0] == '\0')
          {
            strlcpy(xmlUserData->abn.Country, isoStr, sizeof(xmlUserData->abn.Country));
          }
          else
          {
            strlcat(xmlUserData->abn.Country, ", ", sizeof(xmlUserData->abn.Country));
            strlcat(xmlUserData->abn.Country, isoStr, sizeof(xmlUserData->abn.Country));
          }
        }
        break;

        case xd_Phone:
        {
          if(xmlUserData->abn.Phone[0] == '\0')
          {
            strlcpy(xmlUserData->abn.Phone, isoStr, sizeof(xmlUserData->abn.Phone));
          }
          else
          {
            strlcat(xmlUserData->abn.Phone, ", ", sizeof(xmlUserData->abn.Phone));
            strlcat(xmlUserData->abn.Phone, isoStr, sizeof(xmlUserData->abn.Phone));
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
    if(xmlUserData->abn.Address[0] != '\0')
    {
      struct ABookNode *abn;

      // set up an alias only if none is given
      if(xmlUserData->abn.Alias[0] == '\0')
        SetDefaultAlias(&xmlUserData->abn);

      if((abn = DuplicateNode(&xmlUserData->abn, sizeof(xmlUserData->abn))) != NULL)
      {
        // put it into the tree
        AddABookNode(&xmlUserData->abook->rootGroup, abn, NULL);
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
/// ImportXMLABook
// imports an address book in XML format (i.e. from SimpleMail)
BOOL ImportXMLABook(const char *filename, struct ABook *abook, BOOL append)
{
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  if((fh = fopen(filename, "r")) != NULL)
  {
    XML_Parser parser;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(append == FALSE)
      ClearABook(abook);

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

struct PlainSearchStuff
{
  const char *text;
  size_t textLen;
  ULONG mode;
  struct ABookNode **result;
  ULONG hits;
};

/// SearchABookEntry
BOOL SearchABookEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct PlainSearchStuff *stuff = (struct PlainSearchStuff *)userData;
  BOOL result = TRUE;
  BOOL doSearch;

  ENTER();

  // now we check if this entry is one of the not wished entry types
  // and then we skip it.
  if(abn->type == ABNT_USER && isUserTypeSearch(stuff->mode) == TRUE)
    doSearch = TRUE;
  else if(abn->type == ABNT_LIST && isListTypeSearch(stuff->mode) == TRUE)
    doSearch = TRUE;
  else if(abn->type == ABNT_GROUP && isGroupTypeSearch(stuff->mode) == TRUE)
    doSearch = TRUE;
  else
    doSearch = FALSE;

  if(doSearch == TRUE)
  {
    BOOL found = FALSE;

    if(isCompleteSearch(stuff->mode))
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(found == FALSE && isAliasSearch(stuff->mode))
        found = (Strnicmp(abn->Alias, stuff->text, stuff->textLen) == 0);
      if(found == FALSE && isRealNameSearch(stuff->mode))
        found = (Strnicmp(abn->RealName, stuff->text, stuff->textLen) == 0);
      if(found == FALSE && isAddressSearch(stuff->mode))
        found = (Strnicmp(abn->Address, stuff->text, stuff->textLen) == 0);
    }
    else
    {
      // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
      if(found == FALSE && isAliasSearch(stuff->mode))
        found = (Stricmp(abn->Alias, stuff->text) == 0);
      if(found == FALSE && isRealNameSearch(stuff->mode))
        found = (Stricmp(abn->RealName, stuff->text) == 0);
      if(found == FALSE && isAddressSearch(stuff->mode))
        found = (Stricmp(abn->Address, stuff->text) == 0);
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

  RETURN(result);
  return result;
}

///
/// SearchABook
//  Searches the address book by alias, name or address
//  it will break if there is more then one entry
ULONG SearchABook(const struct ABook *abook, const char *text, ULONG mode, struct ABookNode **abn)
{
  struct PlainSearchStuff stuff;
  ULONG hits;

  ENTER();

  stuff.text = text;
  stuff.textLen = strlen(text);
  stuff.mode = mode;
  stuff.result = abn;
  stuff.hits = 0;
  IterateABook(abook, 0, SearchABookEntry, &stuff);

  hits = stuff.hits;

  RETURN(hits);
  return hits;
}

///

struct PatternSearchStuff
{
  const char *pattern;
  ULONG mode;
  char **aliases;
  ULONG hits;
};

/// PatternSearchABookEntry
BOOL PatternSearchABookEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct PatternSearchStuff *stuff = (struct PatternSearchStuff *)userData;
  BOOL doSearch;

  ENTER();

  // now we check if this entry is one of the not wished entry types
  // and then we skip it.
  if(abn->type == ABNT_USER && isUserTypeSearch(stuff->mode) == TRUE)
    doSearch = TRUE;
  else if(abn->type == ABNT_LIST && isListTypeSearch(stuff->mode) == TRUE)
    doSearch = TRUE;
  else
    doSearch = FALSE;

  if(doSearch == TRUE)
  {
    BOOL found = FALSE;

    // Now we check for the ALIAS->REALNAME->ADDRESS, so only ONE mode is allowed at a time
    if(found == FALSE && isAliasSearch(stuff->mode))
      found = MatchNoCase(abn->Alias, stuff->pattern);
    if(found == FALSE && isRealNameSearch(stuff->mode))
      found = MatchNoCase(abn->RealName, stuff->pattern);
    if(found == FALSE && isAddressSearch(stuff->mode))
      found = MatchNoCase(abn->Address, stuff->pattern);
    if(found == FALSE && isCommentSearch(stuff->mode))
      found = MatchNoCase(abn->Comment, stuff->pattern);
    if(found == FALSE && isUserInfoSearch(stuff->mode) && abn->type == ABNT_USER)
      found = MatchNoCase(abn->Homepage, stuff->pattern) ||
              MatchNoCase(abn->Street, stuff->pattern)   ||
              MatchNoCase(abn->City, stuff->pattern)     ||
              MatchNoCase(abn->Country, stuff->pattern)  ||
              MatchNoCase(abn->Phone, stuff->pattern);

    if(found == TRUE)
    {
      D(DBF_ABOOK, "found pattern '%s' in entry with alias '%s'", stuff->pattern, abn->Alias);

      if(stuff->aliases != NULL)
        stuff->aliases[stuff->hits] = (char *)abn->Alias;

      // always count the number of hits
      stuff->hits++;
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// PatternSearchABook
//  Searches the address book by alias, name or address
//  it will break if there is more then one entry
ULONG PatternSearchABook(const struct ABook *abook, const char *pattern, ULONG mode, char **aliases)
{
  struct PatternSearchStuff stuff;
  ULONG hits;

  ENTER();

  stuff.pattern = pattern;
  stuff.mode = mode;
  stuff.aliases = aliases;
  stuff.hits = 0;
  IterateABook(abook, 0, PatternSearchABookEntry, &stuff);

  hits = stuff.hits;

  RETURN(hits);
  return hits;
}

///
/// CreateABookGroup
// create a new group, default to the root group
struct ABookNode *CreateABookGroup(struct ABook *abook, const char *name)
{
  struct ABookNode *result = &abook->rootGroup;

  ENTER();

  if(IsStrEmpty(name) == FALSE)
  {
    struct ABookNode *abn;

    if(SearchABook(abook, name, ASM_ALIAS|ASM_GROUP|ASM_COMPLETE, &abn) == 0)
    {
      if((abn = CreateABookNode(ABNT_GROUP)) != NULL)
      {
        strlcpy(abn->Alias, name, sizeof(abn->Alias));
        AddABookNode(&abook->rootGroup, abn, NULL);

        result = abn;
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// FindPersonInABook
struct ABookNode *FindPersonInABook(const struct ABook *abook, const struct Person *pe)
{
  struct ABookNode *result = NULL;
  struct ABookNode *abn;

  ENTER();

  if(SearchABook(abook, pe->Address, ASM_ADDRESS|ASM_USER|ASM_COMPLETE, &abn) == 1)
  {
    result = abn;
  }

  RETURN(result);
  return result;
}

///

struct BirthdayStuff
{
  ldiv_t today;
};

/// CheckABookEntryBirthday
static BOOL CheckABookEntryBirthday(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct BirthdayStuff *stuff = (struct BirthdayStuff *)userData;

  ENTER();

  if(abn->type == ABNT_USER && abn->Birthday != 0)
  {
    ldiv_t birthday = ldiv(abn->Birthday, 10000);

    if(birthday.quot == stuff->today.quot)
    {
      char question[SIZE_LARGE];
      const char *name = abn->RealName[0] != '\0' ? abn->RealName : abn->Alias;
      char dateString[64];

      DateStamp2String(dateString, sizeof(dateString), NULL, DSS_DATE, TZC_NONE);
      snprintf(question, sizeof(question), tr(MSG_AB_BirthdayReqBody), dateString, name, stuff->today.rem - birthday.rem);

      // show the Birthday Requester
      BirthdayRequestWindowObject,
        MUIA_BirthdayRequestWindow_Body, question,
        MUIA_BirthdayRequestWindow_Alias, abn->Alias,
      End;
    }
  }

  RETURN(TRUE);
  return TRUE;
}

///
/// CheckABookBirthdays
// searches the address book for todays birthdays
void CheckABookBirthdays(const struct ABook *abook, BOOL check)
{
  struct TimeVal nowTV;
  struct TimeVal nextTV;
  struct DateStamp nextDS;

  ENTER();

  // perform the check only if we are instructed to do it
  if(check == TRUE)
  {
    struct BirthdayStuff stuff;

    stuff.today = ldiv(DateStamp2Long(NULL), 10000);
    IterateABook(abook, 0, CheckABookEntryBirthday, &stuff);
  }

  // reschedule the birthday check for the configured check time
  DateStamp(&nextDS);
  nextDS.ds_Minute = C->BirthdayCheckTime.ds_Minute;
  nextDS.ds_Tick = 0;

  DateStamp2TimeVal(&nextDS, &nextTV, TZC_NONE);

  GetSysTime(TIMEVAL(&nowTV));
  if(CmpTime(TIMEVAL(&nowTV), TIMEVAL(&nextTV)) < 0)
  {
    // if the check time is already over for today we schedule the next check
    // for tomorrow
    nextDS.ds_Days++;
    DateStamp2TimeVal(&nextDS, &nextTV, TZC_NONE);
  }

  // calculate the remaining time until the next check
  SubTime(TIMEVAL(&nextTV), TIMEVAL(&nowTV));

  #if defined(DEBUG)
  {
  char dateString[64];

  DateStamp2String(dateString, sizeof(dateString), &nextDS, DSS_DATETIME, TZC_NONE);
  D(DBF_TIMER, "next birthday check @ %s", dateString);
  }
  #endif
  RestartTimer(TIMER_CHECKBIRTHDAYS, nextTV.Seconds, nextTV.Microseconds, TRUE);

  LEAVE();
}

///

struct PrintStuff
{
  FILE *prt;
  int mode;
};

/// PrintABookEntryField
//  Formats and prints a single field
static void PrintABookEntryField(const char *fieldname, const char *field, FILE *prt)
{
  ENTER();

  if(IsStrEmpty(field) == FALSE)
    fprintf(prt, "%-20.20s: %-50.50s\n", StripUnderscore(fieldname), field);

  LEAVE();
}

///
/// PrintShortABookEntry
//  Prints an address book entry in compact format
void PrintShortABookEntry(const struct ABookNode *abn, FILE *prt)
{
  static const char types[3] = { 'P','L','G' };

  ENTER();

  fprintf(prt, "%c %-12.12s %-20.20s %-36.36s\n", types[abn->type-ABNT_USER],
     abn->Alias, abn->RealName, abn->type == ABNT_USER ? abn->Address : abn->Comment);

  LEAVE();
}

///
/// PrintLongABookEntry
//  Prints an address book entry in detailed format
void PrintLongABookEntry(const struct ABookNode *abn, FILE *prt)
{
  ENTER();

  fputs("------------------------------------------------------------------------\n", prt);
  switch(abn->type)
  {
    case ABNT_USER:
    {
      char dateStr[SIZE_SMALL];

      BirthdayToString(abn->Birthday, dateStr, sizeof(dateStr));

      PrintABookEntryField(tr(MSG_AB_PersonAlias), abn->Alias, prt);
      PrintABookEntryField(tr(MSG_EA_RealName), abn->RealName, prt);
      PrintABookEntryField(tr(MSG_EA_EmailAddress), abn->Address, prt);
      PrintABookEntryField(tr(MSG_EA_PGPId), abn->PGPId, prt);
      PrintABookEntryField(tr(MSG_EA_Homepage), abn->Homepage, prt);
      PrintABookEntryField(tr(MSG_EA_Street), abn->Street, prt);
      PrintABookEntryField(tr(MSG_EA_City), abn->City, prt);
      PrintABookEntryField(tr(MSG_EA_Country), abn->Country, prt);
      PrintABookEntryField(tr(MSG_EA_Phone), abn->Phone, prt);
      PrintABookEntryField(tr(MSG_EA_DOB), dateStr, prt);
    }
    break;

    case ABNT_GROUP:
    {
      PrintABookEntryField(tr(MSG_AB_GroupAlias), abn->Alias, prt);
    }
    break;

    case ABNT_LIST:
    {
      PrintABookEntryField(tr(MSG_AB_ListAlias), abn->Alias, prt);
      PrintABookEntryField(tr(MSG_EA_MLName), abn->RealName, prt);
      PrintABookEntryField(tr(MSG_EA_ReturnAddress), abn->Address, prt);
      if(abn->ListMembers != NULL)
      {
        BOOL header = FALSE;
        char *ptr;

        for(ptr = abn->ListMembers; *ptr; ptr++)
        {
          char *nptr = strchr(ptr, '\n');

          if(nptr != NULL)
            *nptr = 0;
          else
            break;
          if(header == FALSE)
          {
            PrintABookEntryField(tr(MSG_EA_Members), ptr, prt);
            header = TRUE;
          }
          else
            fprintf(prt, "                      %s\n", ptr);
          *nptr = '\n';
          ptr = nptr;
        }
      }
    }
    break;
  }

  PrintABookEntryField(tr(MSG_EA_Description), abn->Comment, prt);

  LEAVE();
}

///
/// PrintABookEntry
static BOOL PrintABookEntry(const struct ABookNode *abn, UNUSED ULONG flags, void *userData)
{
  struct PrintStuff *stuff = (struct PrintStuff *)userData;

  ENTER();

  if(stuff->mode == 1)
    PrintLongABookEntry(abn, stuff->prt);
  else
    PrintShortABookEntry(abn, stuff->prt);

  RETURN(TRUE);
  return TRUE;
}

///
/// PrintABook
//  Recursively prints all address book entries
void PrintABook(const struct ABook *abook, FILE *prt, int mode)
{
  struct PrintStuff stuff;

  ENTER();

  stuff.prt = prt;
  stuff.mode = mode;
  IterateABook(abook, 0, PrintABookEntry, &stuff);

  LEAVE();
}

///
/// PrintABookGroup
//  Recursively prints an address book group
void PrintABookGroup(const struct ABookNode *group, FILE *prt, int mode)
{
  struct PrintStuff stuff;

  ENTER();

  stuff.prt = prt;
  stuff.mode = mode;
  IterateABookGroup(group, 0, PrintABookEntry, &stuff);

  LEAVE();
}

///

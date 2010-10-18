/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include <clib/alib_protos.h>
#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/xpkmaster.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "mui/Classes.h"

#include "FileInfo.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"

#include "Debug.h"

/* local protos */
static struct FO_ClassData *FO_New(void);

// According to the folder types we define the corresponding
// default folder names. Please note that order and length IS important here.
// check the "enum FolderType"
const char* const FolderName[FT_NUM] = { NULL,       // FT_CUSTOM
                                         "incoming", // FT_INCOMING
                                         "outgoing", // FT_OUTGOING
                                         "sent",     // FT_SENT
                                         "trash",    // FT_TRASH
                                         NULL,       // FT_GROUP
                                         NULL,       // FT_CUSTOMSENT
                                         NULL,       // FT_CUSTOMMIXED
                                         "spam",     // FT_SPAM
                                       };

/***************************************************************************
 Module: Folder Configuration
***************************************************************************/

/// FO_GetCurrentFolder
//  Returns pointer to active folder
struct Folder *FO_GetCurrentFolder(void)
{
  struct Folder *folder = NULL;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active)) != NULL)
    folder = ((struct FolderNode *)tn->tn_User)->folder;

  RETURN(folder);
  return folder;
}

///
/// FO_SetCurrentFolder
//  Set the passed folder as the active one
void FO_SetCurrentFolder(const struct Folder *fo)
{
  ENTER();

  if(fo != NULL)
  {
    // make sure the tree is opened to display it
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, fo->Treenode, MUIF_NONE);

    nnset(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, fo->Treenode);
  }

  LEAVE();
}
///
/// FO_GetFolderRexx
//  Finds a folder by its name, type or position
struct Folder *FO_GetFolderRexx(const char *arg, int *pos)
{
  struct Folder *fo = NULL;

  ENTER();

  LockFolderListShared(G->folders);

  if(IsFolderListEmpty(G->folders) == FALSE)
  {
    struct FolderNode *foundNode = NULL;
    int listIndex = 0;
    const char *p = arg;
    BOOL numeric = TRUE;

    // lets find out if the user wants to have the folder identified by it's position
    while(*p != '\0')
    {
      int c = (int)*p++;

      if(isdigit(c) == FALSE)
      {
        numeric = FALSE;
        break;
      }
    }

    if(numeric == TRUE)
    {
      // a numeric search, find the folder by index
      int wantedIndex = atoi(arg);
      int findIndex = 0;

      if(wantedIndex >= 0 && wantedIndex < (int)G->folders->count)
      {
        struct FolderNode *fnode;

        listIndex = 0;
        ForEachFolderNode(G->folders, fnode)
        {
          // if the current one is a FT_GROUP we go to the next one until we find
          // the correct one
          if(isGroupFolder(fnode->folder) == FALSE)
          {
            if(findIndex == wantedIndex)
            {
              foundNode = fnode;
              break;
            }
            // count real folders only
            findIndex++;
          }
          // count folders and groups
          listIndex++;
        }
      }
    }
    else
    {
      // find the folder by name
      struct FolderNode *fnode;

      listIndex = 0;
      ForEachFolderNode(G->folders, fnode)
      {
        struct Folder *folder = fnode->folder;

        if((!Stricmp(arg, folder->Name) && !isGroupFolder(folder))              ||
           (!stricmp(arg, FolderName[FT_INCOMING]) && isIncomingFolder(folder)) ||
           (!stricmp(arg, FolderName[FT_OUTGOING]) && isOutgoingFolder(folder)) ||
           (!stricmp(arg, FolderName[FT_SENT]) && isSentFolder(folder))         ||
           (!stricmp(arg, FolderName[FT_TRASH]) && isTrashFolder(folder))       ||
           (!stricmp(arg, FolderName[FT_SPAM]) && isSpamFolder(folder)))
        {
          foundNode = fnode;
          break;
        }

        listIndex++;
      }
    }

    if(foundNode != NULL)
    {
      fo = foundNode->folder;
      if(pos != NULL)
        *pos = listIndex;
    }
  }

  UnlockFolderList(G->folders);

  RETURN(fo);
  return fo;
}
///
/// FO_GetFolderByAttribute
//  Generalized find-folder function
static struct Folder *FO_GetFolderByAttribute(BOOL (*cmpf)(const struct Folder*, const void*), const void *attr, int *pos)
{
  int i = 0;
  struct Folder *folder = NULL;

  ENTER();

  LockFolderListShared(G->folders);

  if(IsFolderListEmpty(G->folders) == FALSE)
  {
    struct FolderNode *fnode;

    ForEachFolderNode(G->folders, fnode)
    {
      if(cmpf(fnode->folder, attr) == TRUE)
      {
        folder = fnode->folder;
        break;
      }
      i++;
    }
  }

  UnlockFolderList(G->folders);

  if(pos != NULL)
    *pos = i;

  RETURN(folder);
  return folder;
}
///
/// FO_GetFolderByType
// comparison function for FO_GetFolderByType
static BOOL FO_GetFolderByType_cmp(const struct Folder *f, const enum FolderType *type)
{
  return (BOOL)(f->Type == *type);
}
//  Finds a folder by its type
struct Folder *FO_GetFolderByType(const enum FolderType type, int *pos)
{
   return FO_GetFolderByAttribute((BOOL (*)(const struct Folder *, const void *))&FO_GetFolderByType_cmp, (const void *)&type, pos);
}
///
/// FO_GetFolderByName
// comparison function for FO_GetFolderByName
static BOOL FO_GetFolderByName_cmp(const struct Folder *f, const char *name)
{
  return (BOOL)(strcmp(f->Name, name) == 0 && !isGroupFolder(f));
}
//  Finds a folder by its name
struct Folder *FO_GetFolderByName(const char *name, int *pos)
{
  return FO_GetFolderByAttribute((BOOL (*)(const struct Folder *, const void *))&FO_GetFolderByName_cmp, (const void *)name, pos);
}
///
/// FO_GetFolderByPath
// comparison function for FO_GetFolderByPath
static BOOL FO_GetFolderByPath_cmp(const struct Folder *f, const char *path)
{
  return (BOOL)(stricmp(f->Path, path) == 0);
}
//  Finds a folder by its path
struct Folder *FO_GetFolderByPath(const char *path, int *pos)
{
  return FO_GetFolderByAttribute((BOOL (*)(const struct Folder *, const void *))&FO_GetFolderByPath_cmp, (const void *)path, pos);
}
///
/// FO_GetFolderPosition
//  Gets the position of a folder in the list
//  !! must be called with a locked folder list !!
int FO_GetFolderPosition(struct Folder *findfo, BOOL withGroups)
{
  int pos = -1;

  ENTER();

  if(IsFolderListEmpty(G->folders) == FALSE)
  {
    struct FolderNode *fnode;
    int p = 0;

    ForEachFolderNode(G->folders, fnode)
    {
      if(fnode->folder == findfo)
      {
        // success
        pos = p;
        break;
      }

      if(withGroups == TRUE || isGroupFolder(fnode->folder) == FALSE)
      {
        // count all folders, or even groups if allowed
        p++;
      }
    }
  }

  RETURN(pos);
  return pos;
}
///
/// FO_LoadConfig
//  Loads folder configuration from .fconfig file
BOOL FO_LoadConfig(struct Folder *fo)
{
  BOOL success = FALSE;
  FILE *fh;
  char fname[SIZE_PATHFILE];

  ENTER();

  AddPath(fname, fo->Fullpath, ".fconfig", sizeof(fname));
  if((fh = fopen(fname, "r")) != NULL)
  {
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(getline(&buf, &buflen, fh) >= 3 && strnicmp(buf, "YFC", 3) == 0)
    {
      BOOL statsproc = FALSE;

      // pick a default value for ML support parameters
      fo->MLSignature  = 1;
      fo->MLSupport    = TRUE;

      while(getline(&buf, &buflen, fh) > 0)
      {
        char *p;
        char *value;

        if((value = strchr(buf, '=')) != NULL)
        {
          for(++value; isspace(*value); value++)
            ;
        }

        if((p = strpbrk(buf, "\r\n")) != NULL)
          *p = '\0';

        for(p = buf; *p != '\0' && isspace(*p) == FALSE; p++)
          ;

        *p = '\0';

        if(*buf != '\0' && value != NULL)
        {
          if(stricmp(buf, "Name") == 0)                strlcpy(fo->Name, value, sizeof(fo->Name));
          else if(stricmp(buf, "MaxAge") == 0)         fo->MaxAge = atoi(value);
          else if(stricmp(buf, "Password") == 0)       strlcpy(fo->Password, Decrypt(value), sizeof(fo->Password));
          else if(stricmp(buf, "Type") == 0)           fo->Type = atoi(value);
          else if(stricmp(buf, "XPKType") == 0)        fo->Mode = atoi(value); // valid < v2.4
          else if(stricmp(buf, "Mode") == 0)           fo->Mode = atoi(value);
          else if(stricmp(buf, "Sort1") == 0)          fo->Sort[0] = atoi(value);
          else if(stricmp(buf, "Sort2") == 0)          fo->Sort[1] = atoi(value);
          else if(stricmp(buf, "Stats") == 0)          { fo->Stats = Txt2Bool(value); statsproc = TRUE; }
          else if(stricmp(buf, "ExpireUnread") == 0)   fo->ExpireUnread = Txt2Bool(value);
          else if(stricmp(buf, "MLSupport") == 0)      fo->MLSupport = Txt2Bool(value);
          else if(stricmp(buf, "MLFromAddr") == 0)     strlcpy(fo->MLFromAddress, value, sizeof(fo->MLFromAddress));
          else if(stricmp(buf, "MLRepToAddr") == 0)    strlcpy(fo->MLReplyToAddress, value, sizeof(fo->MLReplyToAddress));
          else if(stricmp(buf, "MLAddress") == 0)      strlcpy(fo->MLAddress, value, sizeof(fo->MLAddress));
          else if(stricmp(buf, "MLPattern") == 0)      strlcpy(fo->MLPattern, value, sizeof(fo->MLPattern));
          else if(stricmp(buf, "MLSignature") == 0)    fo->MLSignature = atoi(value);
          else if(stricmp(buf, "WriteIntro") == 0)     strlcpy(fo->WriteIntro, value, sizeof(fo->WriteIntro));
          else if(stricmp(buf, "WriteGreetings") == 0) strlcpy(fo->WriteGreetings, value, sizeof(fo->WriteGreetings));
        }
      }

      success = TRUE;

      if(statsproc == FALSE)
        fo->Stats = !isIncomingFolder(fo);

      if(isTrashFolder(fo) || isSpamFolder(fo))
        fo->ExpireUnread = TRUE;

      // check for non custom folder
      // and set some values which shouldn't be changed
      if(isDefaultFolder(fo))
      {
        fo->MLSignature  = -1;
        fo->MLSupport    = FALSE;
      }

      // mark an old spam folder as custom folder, so it can be deleted
      if(fo->Type == FT_SPAM && C->SpamFilterEnabled == FALSE)
        fo->Type = FT_CUSTOM;
    }
    else
      E(DBF_FOLDER, "couldn't find folder config header");

    fclose(fh);

    free(buf);
  }

  RETURN(success);
  return success;
}

///
/// FO_SaveConfig
//  Saves folder configuration to .fconfig file
BOOL FO_SaveConfig(struct Folder *fo)
{
  char fname[SIZE_PATHFILE];
  FILE *fh;
  BOOL result = FALSE;

  ENTER();

  AddPath(fname, fo->Fullpath, ".fconfig", sizeof(fname));
  if((fh = fopen(fname, "w")) != NULL)
  {
    struct DateStamp ds;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fprintf(fh, "YFC2 - YAM Folder Configuration\n");
    fprintf(fh, "Name           = %s\n", fo->Name);
    fprintf(fh, "MaxAge         = %d\n", fo->MaxAge);
    fprintf(fh, "Password       = %s\n", Encrypt(fo->Password));
    fprintf(fh, "Type           = %d\n", fo->Type);
    fprintf(fh, "Mode           = %d\n", fo->Mode);
    fprintf(fh, "Sort1          = %d\n", fo->Sort[0]);
    fprintf(fh, "Sort2          = %d\n", fo->Sort[1]);
    fprintf(fh, "Stats          = %s\n", Bool2Txt(fo->Stats));
    fprintf(fh, "ExpireUnread   = %s\n", Bool2Txt(fo->ExpireUnread));
    fprintf(fh, "MLSupport      = %s\n", Bool2Txt(fo->MLSupport));
    fprintf(fh, "MLFromAddr     = %s\n", fo->MLFromAddress);
    fprintf(fh, "MLRepToAddr    = %s\n", fo->MLReplyToAddress);
    fprintf(fh, "MLPattern      = %s\n", fo->MLPattern);
    fprintf(fh, "MLAddress      = %s\n", fo->MLAddress);
    fprintf(fh, "MLSignature    = %d\n", fo->MLSignature);
    fprintf(fh, "WriteIntro     = %s\n", fo->WriteIntro);
    fprintf(fh, "WriteGreetings = %s\n", fo->WriteGreetings);
    fclose(fh);

    AddPath(fname, fo->Fullpath, ".index", sizeof(fname));

    if(!isModified(fo))
      SetFileDate(fname, DateStamp(&ds));

    result = TRUE;
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), fname);

  RETURN(result);
  return result;
}

///
/// FO_NewFolder
//  Initializes a new folder and creates its directory
struct Folder *FO_NewFolder(enum FolderType type, const char *path, const char *name)
{
  struct Folder *folder;

  ENTER();

  if((folder = calloc(1, sizeof(*folder))) != NULL)
  {
    if((folder->messages = CreateMailList()) != NULL)
    {
      folder->Sort[0] = 1;
      folder->Sort[1] = 3;
      folder->Type = type;
      folder->LastActive = -1;

      // set the standard icon images, or none for a custom folder
      switch(type)
      {
        case FT_INCOMING: folder->ImageIndex = FICON_ID_INCOMING; break;
        case FT_OUTGOING: folder->ImageIndex = FICON_ID_OUTGOING; break;
        case FT_TRASH:    folder->ImageIndex = FICON_ID_TRASH;    break;
        case FT_SENT:     folder->ImageIndex = FICON_ID_SENT;     break;
        case FT_SPAM:     folder->ImageIndex = FICON_ID_SPAM;     break;
        default:          folder->ImageIndex = -1;                break;
      }

      strlcpy(folder->Path, path, sizeof(folder->Path));
      strlcpy(folder->Name, name, sizeof(folder->Name));

      if(strchr(path, ':') != NULL)
        strlcpy(folder->Fullpath, path, sizeof(folder->Fullpath));
      else
        AddPath(folder->Fullpath, G->MA_MailDir, path, sizeof(folder->Fullpath));

      if(CreateDirectory(folder->Fullpath) == FALSE)
      {
        DeleteMailList(folder->messages);
        free(folder);
        folder = NULL;
      }
    }
    else
    {
      free(folder);
      folder = NULL;
    }
  }

  RETURN(folder);
  return folder;
}

///
/// FO_FreeFolder
//  frees all resources previously allocated on creation time of the folder
BOOL FO_FreeFolder(struct Folder *folder)
{
  BOOL result = FALSE;

  ENTER();

  if(folder != NULL)
  {
    D(DBF_FOLDER, "freeing folder '%s'", folder->Name);
    // remove the image of this folder from the objectlist at the application
    if(folder->imageObject != NULL)
    {
      // Here we cannot remove the BC_FImage from the BC_GROUP because the
      // destructor of the Folder Listtree will call this function and then
      // this BC_GROUP doesn't exists anymore. -> Enforcer hit !
      // so if the user is going to remove this folder by hand we will remove
      // the BC_FImage of it in the FO_DeleteFolderFunc() before the destructor
      // is going to call this function.

      // remove the bodychunk object from the nlist
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

      MUI_DisposeObject(folder->imageObject);
      // let's set it to NULL so that the destructor doesn't do the work again.
      folder->imageObject = NULL;
    }

    if(!isGroupFolder(folder))
    {
      // free all the mail pointers in the list
      LockMailListShared(folder->messages);

      if(IsMailListEmpty(folder->messages) == FALSE)
      {
        struct MailNode *mnode;

        ForEachMailNode(folder->messages, mnode)
        {
          free(mnode->mail);
          mnode->mail = NULL;
        }
      }

      UnlockMailList(folder->messages);
    }

    // free the mail list itself
    DeleteMailList(folder->messages);

    D(DBF_FOLDER, "freed folder '%s'", folder->Name);

    // now it's time to deallocate the folder itself
    free(folder);
    result = TRUE;
  }

  RETURN(result);
  return result;
}

///
/// FO_CreateFolder
//  Adds a new entry to the folder list
BOOL FO_CreateFolder(enum FolderType type, const char * const path, const char *name)
{
  struct Folder *folder;
  BOOL result = FALSE;

  ENTER();

  if((folder = FO_NewFolder(type, path, name)) != NULL)
  {
    struct FolderNode *fnode;

    LockFolderList(G->folders);
    fnode = AddNewFolderNode(G->folders, folder);
    UnlockFolderList(G->folders);

    if(fnode != NULL)
    {
      struct MUI_NListtree_TreeNode *tn;

      if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder->Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE)) != NULL)
      {
        if(FO_SaveConfig(folder) == TRUE)
        {
          folder->Treenode = tn;

          // only if we reach here everything was fine and we can return TRUE
          result = TRUE;
        }
        else
        {
          // If we reach here the SaveConfig() returned FALSE and we need to remove the folder again
          // from the listtree. But we MUST pass the treenode and NOT the folder, because the folder
          // pointer is no valid treenode but just the user data!!
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, tn, MUIF_NONE);
        }
      }
    }
  }

  RETURN(result);
  return result;
}

///
/// FO_LoadFolderImage
//  Loads the images for the folder that should be displayed in the NListtree
static BOOL FO_LoadFolderImage(struct Folder *folder)
{
  BOOL success = FALSE;

  ENTER();

  // first we make sure that valid data is underway.
  if(folder != NULL)
  {
    if(folder->ImageIndex >= MAX_FOLDERIMG+1)
    {
      char fname[SIZE_PATHFILE];
      Object *lv = G->MA->GUI.NL_FOLDERS;

      AddPath(fname, folder->Fullpath, ".fimage", sizeof(fname));
      if(FileExists(fname) == TRUE)
      {
        // Now we say that this image could be used by this Listtree
        if((folder->imageObject = MakeImageObject(fname, fname)) != NULL)
        {
          DoMethod(lv, MUIM_NList_UseImage, folder->imageObject, folder->ImageIndex, MUIF_NONE);

          D(DBF_FOLDER, "successfully loaded folder image '%s'", fname);
          success = TRUE;
        }
        else
          E(DBF_FOLDER, "error while trying to create imageareaobejct for '%s'", fname);
      }
      else
      {
        D(DBF_FOLDER, "no folder image '%s' found", fname);
        folder->imageObject = NULL;
      }
    }
    else
    {
      W(DBF_FOLDER, "imageIndex of folder < MAX_FOLDERIMG (%ld < %ld)", folder->ImageIndex, MAX_FOLDERIMG+1);
      folder->imageObject = NULL;
    }
  }
  else
    E(DBF_FOLDER, "folder == NULL");

  RETURN(success);
  return success;
}

///
/// FO_LoadTree
//  Loads folder list from a file
BOOL FO_LoadTree(void)
{
  char foldersPath[SIZE_PATHFILE];
  BOOL success = FALSE;
  char *fname;
  int nested = 0, i = 0, j = MAX_FOLDERIMG+1;
  FILE *fh;
  APTR lv = G->MA->GUI.NL_FOLDERS;
  struct MUI_NListtree_TreeNode *tn_root = MUIV_NListtree_Insert_ListNode_Root;

  ENTER();

  fname = CreateFilename(".folders", foldersPath, sizeof(foldersPath));

  if((fh = fopen(fname, "r")) != NULL)
  {
    char *buffer = NULL;
    size_t size = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(GetLine(&buffer, &size, fh) >= 3 && strncmp(buffer, "YFO", 3) == 0)
    {
      DoMethod(lv, MUIM_NListtree_Clear, NULL, 0);
      set(lv, MUIA_NListtree_Quiet, TRUE);
      while(GetLine(&buffer, &size, fh) >= 0)
      {
        if(strncmp(buffer, "@FOLDER", 7) == 0)
        {
          struct Folder *fo;

          if((fo = calloc(1, sizeof(*fo))) != NULL)
          {
            fo->Type = FT_CUSTOM;
            fo->Sort[0] = 1;
            fo->Sort[1] = 3;
            fo->LastActive = -1;
            strlcpy(fo->Name, Trim(&buffer[8]), sizeof(fo->Name));
            GetLine(&buffer, &size, fh);
            strlcpy(fo->Path, Trim(buffer), sizeof(fo->Path));

            // set up the full path to the folder
            if(strchr(fo->Path, ':') != NULL)
            {
              // the path is an absolute path already
              strlcpy(fo->Fullpath, fo->Path, sizeof(fo->Fullpath));
            }
            else
            {
              // concatenate the default mail dir and the folder's relative path to an absolute path
              strlcpy(fo->Fullpath, G->MA_MailDir, sizeof(fo->Fullpath));
              AddPart(fo->Fullpath, fo->Path, sizeof(fo->Fullpath));
            }

            if((fo->messages = CreateMailList()) != NULL)
            {
              if(CreateDirectory(fo->Fullpath) == TRUE)
              {
                struct FolderNode *fnode;
                struct MUI_NListtree_TreeNode *tn;

                // if there doesn't exist any .fconfig configuration in the folder
                // we do have to generate it and we do that by analyzing its name,
                // comparing it to the default folder names we know.
                if(FO_LoadConfig(fo) == FALSE)
                {
                  char *folderpath = (char *)FilePart(fo->Path);

                  // check if this is a so-called "standard" folder (INCOMING/OUTGOING etc.)
                  if(stricmp(folderpath, FolderName[FT_INCOMING]) == 0)
                    fo->Type = FT_INCOMING;
                  else if(stricmp(folderpath, FolderName[FT_OUTGOING]) == 0)
                    fo->Type = FT_OUTGOING;
                  else if(stricmp(folderpath, FolderName[FT_SENT]) == 0)
                    fo->Type = FT_SENT;
                  else if(stricmp(folderpath, FolderName[FT_TRASH]) == 0)
                    fo->Type = FT_TRASH;
                  else if(C->SpamFilterEnabled == TRUE && stricmp(folderpath, FolderName[FT_SPAM]) == 0)
                    fo->Type = FT_SPAM;

                  // Save the config now because it could be changed in the meantime
                  if(FO_SaveConfig(fo) == FALSE)
                  {
                    fclose(fh);
                    free(buffer);
                    free(fo);

                    RETURN(FALSE);
                    return FALSE;
                  }
                }

                fo->SortIndex = i++;
                fo->ImageIndex = j;

                // Now we load the folder images if they exists
                if(FO_LoadFolderImage(fo) == TRUE)
                {
                  j++;
                }
                else
                {
                  // the new/unread statistics are still at zero, so this will result
                  // in the "no new mails" images
                  FO_SetFolderImage(fo);
                }

                LockFolderList(G->folders);
                fnode = AddNewFolderNode(G->folders, fo);
                UnlockFolderList(G->folders);

                if(fnode == NULL)
                {
                  fclose(fh);
                  free(buffer);
                  free(fo);

                  RETURN(FALSE);
                  return FALSE;
                }

                // Now we add this folder to the folder listtree
                if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE)) == NULL)
                {
                  fclose(fh);
                  free(buffer);
                  free(fo);

                  RETURN(FALSE);
                  return FALSE;
                }

                // remember the treenode
                fo->Treenode = tn;
              }
            }
            else
            {
              fclose(fh);
              free(buffer);
              free(fo);

              RETURN(FALSE);
              return FALSE;
            }
            do
            {
              if(strcmp(buffer, "@ENDFOLDER") == 0)
                break;
            }
            while(GetLine(&buffer, &size, fh) >= 0);
          }
        }
        else if(strncmp(buffer, "@SEPARATOR", 10) == 0)
        {
          struct Folder *fo;

          if((fo = calloc(1, sizeof(*fo))) != NULL)
          {
            long tnflags = TNF_LIST;
            struct FolderNode *fnode;
            struct MUI_NListtree_TreeNode *tn;

            // SEPARATOR support is obsolete since the folder hierachical order
            // that's why we handle SEPARATORs as GROUPs now for backward compatibility
            fo->Type = FT_GROUP;
            strlcpy(fo->Name, Trim(&buffer[11]), sizeof(fo->Name));
            do
            {
              if(strcmp(buffer, "@ENDSEPARATOR") == 0)
                break;
            }
            while(GetLine(&buffer, &size, fh) >= 0);
            fo->SortIndex = i++;

            // Now we check if the foldergroup image was loaded and if not we enable the standard NListtree image
            if(IsImageInCache("folder_fold") &&
               IsImageInCache("folder_unfold"))
            {
              SET_FLAG(tnflags, TNF_NOSIGN);
            }

            LockFolderList(G->folders);
            fnode = AddNewFolderNode(G->folders, fo);
            UnlockFolderList(G->folders);

            if(fnode == NULL)
            {
              fclose(fh);
              free(buffer);
              free(fo);

              RETURN(FALSE);
              return FALSE;
            }

            if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags)) == NULL)
            {
              fclose(fh);
              free(buffer);
              free(fo);

              RETURN(FALSE);
              return FALSE;
            }

            // remember the treenode
            fo->Treenode = tn;
          }
        }
        else if(strncmp(buffer, "@GROUP", 6) == 0)
        {
          struct Folder *fo;

          if((fo = calloc(1, sizeof(*fo))) != NULL)
          {
            long tnflags = TNF_LIST;
            struct FolderNode *fnode;

            fo->Type = FT_GROUP;
            strlcpy(fo->Name, Trim(&buffer[7]), sizeof(fo->Name));

            // now we check if the node should be open or not
            if(GetLine(&buffer, &size, fh) >= 0)
            {
              // if it is greater zero then the node should be displayed open
              if(atoi(buffer) > 0)
                SET_FLAG(tnflags, TNF_OPEN);
            }

            // Now we check if the foldergroup image was loaded and if not we enable the standard NListtree image
            if(IsImageInCache("folder_fold") &&
               IsImageInCache("folder_unfold"))
            {
              SET_FLAG(tnflags, TNF_NOSIGN);
            }

            LockFolderList(G->folders);
            fnode = AddNewFolderNode(G->folders, fo);
            UnlockFolderList(G->folders);

            if(fnode == NULL)
            {
              fclose(fh);
              free(buffer);
              free(fo);

              RETURN(FALSE);
              return FALSE;
            }

            // now we are going to add this treenode to the list
            if((tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags)) == NULL)
            {
              fclose(fh);
              free(buffer);
              free(fo);

              RETURN(FALSE);
              return FALSE;
            }

            // remember the treenode
            fo->Treenode = tn_root;

            nested++;
          }
        }
        else if(strcmp(buffer,"@ENDGROUP") == 0)
        {
          nested--;

          // now we check if the nested is zero and if yes then we set tn_root = MUIV_NListtree_Insert_ListNode_Root
          // otherwise we go back to the root of the root
          if(nested == 0)
            tn_root = MUIV_NListtree_Insert_ListNode_Root;
          else
            tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
        }
      }

      xset(lv, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible,
               MUIA_NListtree_Quiet,  FALSE);
    }

    fclose(fh);

    free(buffer);

    success = TRUE;
  }

  // if nested is still greater zero we have a misconfiguration
  if(nested > 0)
    success = FALSE;

  RETURN(success);
  return success;
}

///
/// FO_SaveSubTree
//  Saves a sub folder list to a file
static BOOL FO_SaveSubTree(FILE *fh, struct MUI_NListtree_TreeNode *subtree)
{
  BOOL success = TRUE;
  BOOL noendgroup = FALSE;
  struct Folder *fo;
  struct MUI_NListtree_TreeNode *tn, *tn_root, *tn_parent;
  APTR lv = G->MA->GUI.NL_FOLDERS;
  int i;

  ENTER();

  // The root-Treenode is the subtree at the start
  tn_root = subtree;

  for(i = 0;; i++)
  {
    if(tn_root == MUIV_NListtree_GetEntry_ListNode_Root)
    {
      tn = tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, 0, MUIF_NONE);
      tn_parent = subtree = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
      noendgroup = TRUE;
    }
    else
    {
      // get the next treenode
      if(i == 0)
        tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, i, MUIF_NONE);
      else
        tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);

      // get the parent node of the treenode
      tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
    }

    // if tn is null or the parent of the next is not the same like the caller of this function
    // we are going to print ENDGROUP and return
    if(tn == NULL || tn_parent != subtree)
    {
      // if we reach here it's just the end of a GROUP
      if(noendgroup == FALSE)
        fputs("@ENDGROUP\n", fh);

      break;
    }
    else
    {
      fo = ((struct FolderNode *)tn->tn_User)->folder;
      if(fo == NULL)
        break;

      fo->SortIndex = i;

      if(isGroupFolder(fo))
      {
        fprintf(fh, "@GROUP %s\n%d\n", fo->Name, isFlagSet(tn->tn_Flags, TNF_OPEN));

        // Now we recursively save this subtree first
        success = FO_SaveSubTree(fh, tn);
      }
      else
        fprintf(fh, "@FOLDER %s\n%s\n@ENDFOLDER\n", fo->Name, fo->Path);

      tn_root = tn;
    }
  }

  RETURN(success);
  return success;
}

///
/// FO_SaveTree
//  Saves folder list to a file
BOOL FO_SaveTree(void)
{
  char foldersPath[SIZE_PATHFILE];
  char *fname;
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

  fname = CreateFilename(".folders", foldersPath, sizeof(foldersPath));

  if((fh = fopen(fname, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fputs("YFO1 - YAM Folders\n", fh);

    success = FO_SaveSubTree(fh, MUIV_NListtree_GetEntry_ListNode_Root);

    fclose(fh);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), fname);

  RETURN(success);
  return success;
}

///
/// FO_SetFolderImage
// set a folder's image depending on its type and the number of new/unread mails
void FO_SetFolderImage(struct Folder *folder)
{
  ENTER();

  // if this folder hasn't got any own folder image in the folder
  // directory and it is one of our standard folders we have to check which image we put in front of it
  if(folder->imageObject == NULL)
  {
    if(isIncomingFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FICON_ID_INCOMING_NEW : FICON_ID_INCOMING;
    else if(isOutgoingFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FICON_ID_OUTGOING_NEW : FICON_ID_OUTGOING;
    else if(isTrashFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FICON_ID_TRASH_NEW : FICON_ID_TRASH;
    else if(isSentFolder(folder))
      folder->ImageIndex = FICON_ID_SENT;
    else if(isSpamFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FICON_ID_SPAM_NEW : FICON_ID_SPAM;
    else
      folder->ImageIndex = -1;
  }

  LEAVE();
}

///
/// FO_UpdateStatistics
// recalculate the number of new/unread/etc mails in a folder
void FO_UpdateStatistics(struct Folder *folder)
{
  ENTER();

  if(isGroupFolder(folder) == FALSE)
  {
    struct MailNode *mnode;

    D(DBF_FOLDER, "updating stats of folder '%s'", folder->Name);

    folder->Unread = 0;
    folder->New = 0;
    folder->Total = 0;
    folder->Sent = 0;
    folder->Deleted = 0;

    LockMailListShared(folder->messages);

    // now we recount the amount of messages of this folder
    ForEachMailNode(folder->messages, mnode)
    {
      struct Mail *mail = mnode->mail;

      folder->Total++;

      if(hasStatusNew(mail))
        folder->New++;

      if(!hasStatusRead(mail))
        folder->Unread++;

      if(hasStatusSent(mail))
        folder->Sent++;

      if(hasStatusDeleted(mail))
        folder->Deleted++;
    }

    UnlockMailList(folder->messages);

    // finally update the image based on the new numbers
    FO_SetFolderImage(folder);
  }

  LEAVE();
}

///
/// FO_UpdateTreeStatistics
// recalculate the number of new/unread/etc mails in a folder and update the parent treenodes
void FO_UpdateTreeStatistics(const struct Folder *folder, const BOOL redraw)
{
  struct MUI_NListtree_TreeNode *tn;
  struct MUI_NListtree_TreeNode *tn_parent;

  ENTER();

  tn = folder->Treenode;

  if(redraw == TRUE)
  {
    // let's redraw the folderentry in the listtree
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, tn, MUIF_NONE);
  }

  // Now we have to recalculate all parent and grandparents treenodes to
  // set their status accordingly.
  while((tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE)))
  {
    if(tn_parent->tn_User != NULL)
    {
      struct Folder *fo_parent = ((struct FolderNode *)tn_parent->tn_User)->folder;
      int i;

      // clear the parent mailvariables first
      fo_parent->Unread = 0;
      fo_parent->New = 0;
      fo_parent->Total = 0;
      fo_parent->Sent = 0;
      fo_parent->Deleted = 0;
      fo_parent->Size = 0;

      // Now we scan every child of the parent and count the mails
      for(i=0;;i++)
      {
        struct MUI_NListtree_TreeNode *tn_child;
        struct Folder *fo_child;

        tn_child = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, tn_parent, i, MUIV_NListtree_GetEntry_Flag_SameLevel);
        if(tn_child == NULL)
          break;

        fo_child = ((struct FolderNode *)tn_child->tn_User)->folder;

        fo_parent->Unread    += fo_child->Unread;
        fo_parent->New       += fo_child->New;
        fo_parent->Total     += fo_child->Total;
        fo_parent->Sent      += fo_child->Sent;
        fo_parent->Deleted   += fo_child->Deleted;
        fo_parent->Size      += fo_child->Size;
      }

      if(redraw == TRUE)
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, tn_parent, MUIF_NONE);

      // for the next step we set tn to the current parent so that we get the
      // grandparents ;)
      tn = tn_parent;
    }
    else
      break;
  }

  LEAVE();
}

///
/// FO_MoveFolderDir
//  Moves a folder to a new directory
static BOOL FO_MoveFolderDir(struct Folder *fo, struct Folder *oldfo)
{
  BOOL success = TRUE;

  ENTER();

  BusyGauge(tr(MSG_BusyMoving), itoa(fo->Total), fo->Total);

  LockMailListShared(fo->messages);

  if(IsMailListEmpty(fo->messages) == FALSE)
  {
    struct MailNode *mnode;
    ULONG i = 0;

    ForEachMailNode(fo->messages, mnode)
    {
      struct Mail *mail = mnode->mail;
      char srcbuf[SIZE_PATHFILE];
      char dstbuf[SIZE_PATHFILE];

      if(BusySet(++i) == FALSE)
      {
        success = FALSE;
        break;
      }

      GetMailFile(dstbuf, sizeof(dstbuf), fo, mail);
      GetMailFile(srcbuf, sizeof(srcbuf), oldfo, mail);

      if(MoveFile(srcbuf, dstbuf) == TRUE)
        RepackMailFile(mail, fo->Mode, fo->Password);
      else
        success = FALSE;
    }
  }

  UnlockMailList(fo->messages);

  if(success == TRUE)
  {
    char srcbuf[SIZE_PATHFILE];
    char dstbuf[SIZE_PATHFILE];

    // now we try to move an existing .index file
    AddPath(srcbuf, oldfo->Fullpath, ".index", sizeof(srcbuf));
    AddPath(dstbuf, fo->Fullpath, ".index", sizeof(dstbuf));
    if(FileExists(srcbuf) == TRUE && MoveFile(srcbuf, dstbuf) == FALSE)
    {
      success = FALSE;
    }
    else
    {
      // now we try to move the .fimage file aswell
      AddPath(srcbuf, oldfo->Fullpath, ".fimage", sizeof(srcbuf));
      AddPath(dstbuf, fo->Fullpath, ".fimage", sizeof(dstbuf));
      if(FileExists(srcbuf) == TRUE && MoveFile(srcbuf, dstbuf) == FALSE)
      {
        success = FALSE;
      }
      else
      {
        // if we were able to successfully move all files
        // we can also delete the source directory. However,
        // we are NOT doing any error checking here as the
        // source may be a VOLUME and as such not deleteable
        DeleteMailDir(oldfo->Fullpath, FALSE);
      }
    }
  }

  BusyEnd();

  RETURN(success);
  return success;
}

///
/// FO_EnterPassword
//  Sets password for a protected folder
static BOOL FO_EnterPassword(struct Folder *fo)
{
  BOOL result = FALSE;

  ENTER();

  do
  {
    char passwd[SIZE_PASSWORD];
    char passwd2[SIZE_PASSWORD];

    passwd[0] = '\0';
    passwd2[0] = '\0';

    if(StringRequest(passwd, SIZE_PASSWORD, tr(MSG_Folder), tr(MSG_CO_ChangeFolderPass), tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->FO->GUI.WI) == 0)
      break;

    if(passwd[0] != '\0' && StringRequest(passwd2, SIZE_PASSWORD, tr(MSG_Folder), tr(MSG_CO_RetypePass), tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->FO->GUI.WI) == 0)
      break;

    if(Stricmp(passwd, passwd2) == 0)
    {
      strlcpy(fo->Password, passwd, sizeof(fo->Password));
      result = TRUE;
      break;
    }
    else
      DisplayBeep(NULL);
  }
  while(TRUE);

  RETURN(result);
  return result;
}

///
/// FO_GetFolder
//  Fills form with data from folder structure
static void FO_GetFolder(struct Folder *folder)
{
  struct FO_GUIData *gui = &G->FO->GUI;
  BOOL isdefault = isDefaultFolder(folder);
  static const int type2cycle[9] = { FT_CUSTOM, FT_CUSTOM, FT_INCOMING, FT_INCOMING, FT_OUTGOING, -1, FT_INCOMING, FT_OUTGOING, FT_CUSTOM };
  int i;

  ENTER();

  set(gui->ST_FNAME,  MUIA_String_Contents, folder->Name);
  set(gui->ST_FPATH,  MUIA_String_Contents, folder->Path);
  set(gui->NM_MAXAGE, MUIA_Numeric_Value,   folder->MaxAge);

  xset(gui->CH_EXPIREUNREAD, MUIA_Selected, folder->ExpireUnread,
                             MUIA_Disabled, isTrashFolder(folder) || isSpamFolder(folder) || folder->MaxAge == 0);

  xset(gui->CY_FTYPE,  MUIA_Cycle_Active, type2cycle[folder->Type],
                       MUIA_Disabled,     isdefault);

  xset(gui->CY_FMODE,  MUIA_Cycle_Active, folder->Mode,
                       MUIA_Disabled,     isdefault);

  for(i = 0; i < 2; i++)
  {
    set(gui->CY_SORT[i], MUIA_Cycle_Active, (folder->Sort[i] < 0 ? -folder->Sort[i] : folder->Sort[i])-1);
    set(gui->CH_REVERSE[i], MUIA_Selected, folder->Sort[i] < 0);
  }

  set(gui->CH_STATS,       MUIA_Selected, folder->Stats);
  set(gui->BT_AUTODETECT,  MUIA_Disabled, !folder->MLSupport || isdefault);
  set(gui->ST_HELLOTEXT,   MUIA_String_Contents, folder->WriteIntro);
  set(gui->ST_BYETEXT,     MUIA_String_Contents, folder->WriteGreetings);

  // for ML-Support
  xset(gui->CH_MLSUPPORT,
       MUIA_Selected, isdefault ? FALSE : folder->MLSupport,
       MUIA_Disabled, isdefault);

  xset(gui->ST_MLADDRESS,
       MUIA_String_Contents, folder->MLAddress,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(gui->ST_MLPATTERN,
       MUIA_String_Contents, folder->MLPattern,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(gui->ST_MLFROMADDRESS,
       MUIA_String_Contents, folder->MLFromAddress,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(gui->ST_MLREPLYTOADDRESS,
       MUIA_String_Contents, folder->MLReplyToAddress,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(gui->CY_MLSIGNATURE,
       MUIA_Cycle_Active,    folder->MLSignature,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  if(!isTrashFolder(folder) && !isSpamFolder(folder))
  {
    // disable the "also unread" check mark whenever the max age is set to 0 days,
    // but only for folders other than the Trash and Spam folders
    DoMethod(G->FO->GUI.NM_MAXAGE, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, G->FO->GUI.CH_EXPIREUNREAD, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  // we make sure the window is at the front if it
  // is already open
  if(xget(G->FO->GUI.WI, MUIA_Window_Open) == TRUE)
    DoMethod(G->FO->GUI.WI, MUIM_Window_ToFront);

  LEAVE();
}

///
/// FO_PutFolder
//  Updates folder structure with form data
static void FO_PutFolder(struct Folder *folder)
{
  static const int cycle2type[3] = { FT_CUSTOM, FT_CUSTOMSENT, FT_CUSTOMMIXED };
  struct FO_GUIData *gui;
  int i;

  ENTER();

  gui = &G->FO->GUI;

  GetMUIString(folder->Name, gui->ST_FNAME, sizeof(folder->Name));
  GetMUIString(folder->Path, gui->ST_FPATH, sizeof(folder->Path));

  // we have to correct the folder path because we shouldn't allow a last / in the
  // path
  if(folder->Path[strlen(folder->Path) - 1] == '/')
    folder->Path[strlen(folder->Path) - 1] = '\0';

  folder->MaxAge = GetMUINumer(gui->NM_MAXAGE);
  if(!isDefaultFolder(folder))
  {
    folder->Type = cycle2type[GetMUICycle(gui->CY_FTYPE)];
    folder->Mode = GetMUICycle(gui->CY_FMODE);
  }

  for(i = 0; i < 2; i++)
  {
    folder->Sort[i] = GetMUICycle(gui->CY_SORT[i]) + 1;
    if (GetMUICheck(gui->CH_REVERSE[i]))
      folder->Sort[i] = -folder->Sort[i];
  }

  folder->ExpireUnread = GetMUICheck(gui->CH_EXPIREUNREAD);
  folder->Stats = GetMUICheck(gui->CH_STATS);
  folder->MLSupport = GetMUICheck(gui->CH_MLSUPPORT);

  GetMUIString(folder->WriteIntro, gui->ST_HELLOTEXT, sizeof(folder->WriteIntro));
  GetMUIString(folder->WriteGreetings, gui->ST_BYETEXT, sizeof(folder->WriteGreetings));

  GetMUIString(folder->MLPattern, gui->ST_MLPATTERN, sizeof(folder->MLPattern));

  // resolve the addresses first, in case someone entered an alias
  DoMethod(gui->ST_MLADDRESS, MUIM_Recipientstring_Resolve, MUIF_NONE);
  DoMethod(gui->ST_MLFROMADDRESS, MUIM_Recipientstring_Resolve, MUIF_NONE);
  DoMethod(gui->ST_MLREPLYTOADDRESS, MUIM_Recipientstring_Resolve, MUIF_NONE);

  GetMUIString(folder->MLAddress, gui->ST_MLADDRESS, sizeof(folder->MLAddress));
  GetMUIString(folder->MLFromAddress, gui->ST_MLFROMADDRESS, sizeof(folder->MLFromAddress));
  GetMUIString(folder->MLReplyToAddress, gui->ST_MLREPLYTOADDRESS, sizeof(folder->MLReplyToAddress));
  folder->MLSignature = GetMUICycle(gui->CY_MLSIGNATURE);

  LEAVE();
}

///
/// FO_NewFolderGroupFunc
//  Creates a new separator
HOOKPROTONHNONP(FO_NewFolderGroupFunc, void)
{
  struct Folder folder;

  ENTER();

  memset(&folder, 0, sizeof(struct Folder));
  folder.Type = FT_GROUP;

  if(StringRequest(folder.Name, SIZE_NAME, tr(MSG_FO_NEWFGROUP), tr(MSG_FO_NEWFGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, G->MA->GUI.WI) != 0)
  {
    LONG tnflags = TNF_LIST | TNF_OPEN;
    struct FolderNode *fnode;

    // Now we check if the foldergroup image was loaded and if not we enable the standard NListtree image
    if(IsImageInCache("folder_fold") == TRUE &&
       IsImageInCache("folder_unfold") == TRUE)
    {
      SET_FLAG(tnflags, TNF_NOSIGN);
    }

    LockFolderList(G->folders);
    fnode = AddNewFolderNode(G->folders, memdup(&folder, sizeof(folder)));
    UnlockFolderList(G->folders);

    if(fnode != NULL)
    {
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder.Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags);

      FO_SaveTree();
    }
  }

  LEAVE();
}
MakeHook(FO_NewFolderGroupHook, FO_NewFolderGroupFunc);

///
/// FO_NewFolderFunc
//  Creates a new folder
HOOKPROTONHNONP(FO_NewFolderFunc, void)
{
  int mode = MUI_Request(G->App, G->MA->GUI.WI, 0, tr(MSG_MA_NewFolder), tr(MSG_FO_NewFolderGads), tr(MSG_FO_NewFolderReq));
  // must be static, otherwise the GUI will access random memory
  static struct Folder folder;

  ENTER();

  // reset the folder struct and set some default values.
  memset(&folder, 0, sizeof(struct Folder));
  folder.Sort[0] = 1;
  folder.Sort[1] = 3;
  folder.Type = FT_CUSTOM;
  folder.ImageIndex = -1;

  switch (mode)
  {
    case 1: break;
    case 2:
    {
      struct Folder *currfolder;

      if((currfolder = FO_GetCurrentFolder()) == NULL)
      {
        LEAVE();
        return;
      }

      // as the user decided to use the settings from the current folder, wie copy
      // the current one to our new one.
      memcpy(&folder, currfolder, sizeof(struct Folder));

      if(isGroupFolder(&folder))
      {
        FO_NewFolderGroupFunc();
        LEAVE();
        return;
      }
      else if(isIncomingFolder(&folder) || isTrashFolder(&folder))
        folder.Type = FT_CUSTOM;
      else if(isOutgoingFolder(&folder) || isSentFolder(&folder))
        folder.Type = FT_CUSTOMSENT;

      // now that we have the correct folder type, we set some default values for the new
      // folder
      folder.Path[0]     = '\0';
      folder.Name[0]     = '\0';
      folder.imageObject = NULL;
      // erase the message list which might have been copied from the current folder
      folder.messages    = NULL;
      // no image for the folder by default
      folder.ImageIndex  = -1;
    }
    break;

    case 3:
    {
      struct FileReqCache *frc;

      if((frc = ReqFile(ASL_FOLDER, G->MA->GUI.WI, tr(MSG_FO_SelectDir), REQF_DRAWERSONLY, G->MA_MailDir, "")) != NULL)
      {
        strlcpy(folder.Path, frc->drawer, sizeof(folder.Path));

        FO_LoadConfig(&folder);
      }
      else
      {
        LEAVE();
        return;
      }
    }
    break;

    default:
    {
      LEAVE();
      return;
    }
  }

  if(G->FO == NULL)
  {
    if((G->FO = FO_New()) == NULL)
    {
      LEAVE();
      return;
    }
    if(SafeOpenWindow(G->FO->GUI.WI) == FALSE)
    {
      DisposeModulePush(&G->FO);
      LEAVE();
      return;
    }
  }

  // there is no "old" folder which could be edited, just the new one
  G->FO->EditFolder = NULL;
  FO_GetFolder(&folder);
  set(G->FO->GUI.WI, MUIA_Window_ActiveObject, G->FO->GUI.ST_FNAME);

  LEAVE();
}
MakeHook(FO_NewFolderHook, FO_NewFolderFunc);

///
/// FO_EditFolderFunc
//  Opens folder window to edit the settings of the active folder
HOOKPROTONHNONP(FO_EditFolderFunc, void)
{
  struct Folder *folder = FO_GetCurrentFolder();

  ENTER();

  if(folder != NULL)
  {
    if(isGroupFolder(folder))
    {
      if(StringRequest(folder->Name, SIZE_NAME, tr(MSG_FO_EDIT_FGROUP), tr(MSG_FO_EDIT_FGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, G->MA->GUI.WI))
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
    }
    else
    {
      if(G->FO == NULL)
      {
        if((G->FO = FO_New()) == NULL)
        {
          LEAVE();
          return;
        }

        if(SafeOpenWindow(G->FO->GUI.WI) == FALSE)
        {
          DisposeModulePush(&G->FO);

          LEAVE();
          return;
        }
      }

      G->FO->EditFolder = folder;
      FO_GetFolder(folder);
    }
  }

  LEAVE();
}
MakeHook(FO_EditFolderHook, FO_EditFolderFunc);

///
/// FO_DeleteFolderFunc
//  Removes the active folder
HOOKPROTONHNONP(FO_DeleteFolderFunc, void)
{
  struct Folder *folder;

  ENTER();

  if((folder = FO_GetCurrentFolder()) != NULL)
  {
    BOOL delete_folder = FALSE;
    Object *lv = G->MA->GUI.NL_FOLDERS;

    switch (folder->Type)
    {
      case FT_CUSTOM:
      case FT_CUSTOMSENT:
      case FT_CUSTOMMIXED:
      {
        if(MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_ConfirmDelete)) != 0)
        {
          // check if the folder that is about to be deleted is part
          // of an active filter and if so remove it from it
          if(FolderIsUsedByFilters(folder->Name) == TRUE)
            RemoveFolderFromFilters(folder->Name);

          delete_folder = TRUE;
          DeleteMailDir(folder->Fullpath, FALSE);
          ClearFolderMails(folder, TRUE);

          // Here we dispose the folderimage Object because the destructor
          // of the Folder Listtree can't do this without throwing enforcer hits
          if(folder->imageObject != NULL)
          {
            // we make sure that the NList also doesn't use the image in future anymore
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

            // and last, but not least we free the BC object here, so that this Object is also gone
            MUI_DisposeObject(folder->imageObject);
            folder->imageObject = NULL; // let's set it to NULL so that the destructor doesn't do the work again.
          }
        }
      }
      break;

      case FT_GROUP:
      {
        struct MUI_NListtree_TreeNode *tn_sub;
        struct MUI_NListtree_TreeNode *tn_group = (struct MUI_NListtree_TreeNode *)xget(lv, MUIA_NListtree_Active);

        // check if the active treenode is a list and if it is empty
        // we have to do this like the following because there is no other way to
        // get known if the active entry has subentries.
        if((tn_sub = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_group, MUIV_NListtree_GetEntry_Position_Head, MUIF_NONE)) != NULL)
        {
          // Now we popup a requester and if this requester is confirmed we move the subentries to the parent node.
          if(MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_YesNoReq2), tr(MSG_FO_GROUP_CONFDEL)))
          {
            struct MUI_NListtree_TreeNode *tn_sub_next = tn_sub;

            delete_folder = TRUE;

            set(lv, MUIA_NListtree_Quiet, TRUE);

            while(tn_sub_next != NULL)
            {
              tn_sub_next = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_sub, MUIV_NListtree_GetEntry_Position_Next, MUIV_NListtree_GetEntry_Flag_SameLevel);

              // move entry to the parent of the group
              DoMethod(lv, MUIM_NListtree_Move, tn_group, tn_sub, MUIV_NListtree_Move_NewListNode_Active, MUIV_NListtree_Move_NewTreeNode_Tail, MUIF_NONE);

              tn_sub = tn_sub_next;
            }

            set(lv, MUIA_NListtree_Quiet, FALSE);
          }
        }
        else
          delete_folder = TRUE;
      }
      break;

      default:
      {
        DisplayBeep(NULL);
      }
      break;
    }

    if(delete_folder == TRUE)
    {
      D(DBF_FOLDER, "deleting folder \"%s\"", folder->Name);

      // remove the entry from the listtree now
      DoMethod(lv, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_Active, MUIF_NONE);

      // Save the Tree to the folder config now
      FO_SaveTree();

      // update the statistics in case the just deleted folder contained new or unread mail
      DisplayStatistics(NULL, TRUE);
    }
    else
      D(DBF_FOLDER, "keeping folder \"%s\"", folder->Name);
  }

  LEAVE();
}
MakeHook(FO_DeleteFolderHook, FO_DeleteFolderFunc);

///
/// FO_CloseFunc
//  Closes folder configuration window
HOOKPROTONHNONP(FO_CloseFunc, void)
{
  ENTER();

  DisposeModulePush(&G->FO);

  LEAVE();
}
MakeStaticHook(FO_CloseHook, FO_CloseFunc);

///
/// FO_SaveFunc
//  Saves modified folder configuration
HOOKPROTONHNONP(FO_SaveFunc, void)
{
  struct FO_GUIData *gui = &G->FO->GUI;
  APTR lv = G->MA->GUI.NL_FOLDERS;
  struct Folder folder;
  struct Folder *oldfolder = G->FO->EditFolder;
  BOOL success = FALSE;
  BOOL isNewFolder;

  ENTER();

  D(DBF_FOLDER, "oldfolder=%08lx '%s'", oldfolder, oldfolder != NULL ? SafeStr(oldfolder->Name) : SafeStr(NULL));
  // if this is a edit folder request we separate here.
  if(oldfolder != NULL)
  {
    BOOL nameChanged;

    isNewFolder = FALSE;
    memcpy(&folder, oldfolder, sizeof(folder));
    FO_PutFolder(&folder);

    // check if something has changed and if not we exit here immediately
    if(memcmp(&folder, oldfolder, sizeof(struct Folder)) == 0)
    {
      DisposeModulePush(&G->FO);

      LEAVE();
      return;
    }

    nameChanged = (stricmp(oldfolder->Name, folder.Name) != 0);

    // lets first check for a valid folder name
    // if the foldername is empty or it was changed and the new name already exists it's invalid
    if(folder.Name[0] == '\0' || (nameChanged == TRUE && FO_GetFolderByName(folder.Name, NULL) != NULL))
    {
      MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERNAMEINVALID));

      LEAVE();
      return;
    }

    // check if the filter name has changed and if it is part of
    // an active filter and if so rename it in the filter definition
    // as well.
    if(nameChanged == TRUE && FolderIsUsedByFilters(oldfolder->Name) == TRUE)
      RenameFolderInFilters(oldfolder->Name, folder.Name);

    // copy the new Folder name
    strlcpy(oldfolder->Name, folder.Name, sizeof(oldfolder->Name));

    // if the folderpath string has changed
    if(stricmp(oldfolder->Path, folder.Path) != 0)
    {
      // check if the full pathes are different
      if(stricmp(oldfolder->Fullpath, folder.Fullpath) != 0)
      {
        int result;

        // check if the new folder already exists or not.
        if(FileExists(folder.Fullpath) == FALSE)
        {
          result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDEREXISTS));
        }
        else
        {
          result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDERMOVE));
        }

        // If the user really wants to proceed
        if(result == 1)
        {
          if(Rename(oldfolder->Fullpath, folder.Fullpath) == FALSE)
          {
            if(!(CreateDirectory(folder.Fullpath) && FO_MoveFolderDir(&folder, oldfolder)))
            {
              ER_NewError(tr(MSG_ER_MOVEFOLDERDIR), folder.Name, folder.Fullpath);

              LEAVE();
              return;
            }
          }
        }
        else
        {
          LEAVE();
          return;
        }
      }

      strlcpy(oldfolder->Path, folder.Path, sizeof(oldfolder->Path));
      strlcpy(oldfolder->Fullpath, folder.Fullpath, sizeof(oldfolder->Path));
    }

    strlcpy(oldfolder->WriteIntro,       folder.WriteIntro, sizeof(oldfolder->WriteIntro));
    strlcpy(oldfolder->WriteGreetings,   folder.WriteGreetings, sizeof(oldfolder->WriteGreetings));
    strlcpy(oldfolder->MLFromAddress,    folder.MLFromAddress, sizeof(oldfolder->MLFromAddress));
    strlcpy(oldfolder->MLReplyToAddress, folder.MLReplyToAddress, sizeof(oldfolder->MLReplyToAddress));
    strlcpy(oldfolder->MLAddress,        folder.MLAddress, sizeof(oldfolder->MLAddress));
    strlcpy(oldfolder->MLPattern,        folder.MLPattern, sizeof(oldfolder->MLPattern));
    oldfolder->MLSignature  = folder.MLSignature;
    oldfolder->Sort[0]      = folder.Sort[0];
    oldfolder->Sort[1]      = folder.Sort[1];
    oldfolder->MaxAge       = folder.MaxAge;
    oldfolder->ExpireUnread = folder.ExpireUnread;
    oldfolder->Stats        = folder.Stats;
    oldfolder->MLSupport    = folder.MLSupport;

    if(xget(gui->CY_FTYPE, MUIA_Disabled) == FALSE)
    {
      enum FolderMode oldmode = oldfolder->Mode;
      enum FolderMode newmode = folder.Mode;
      BOOL changed = TRUE;

      if(oldmode == newmode || (newmode > FM_SIMPLE && XpkBase == NULL))
      {
        changed = FALSE;
      }
      else if(!isProtectedFolder(&folder) && isProtectedFolder(oldfolder) &&
              oldfolder->LoadedMode != LM_VALID)
      {
        if((changed = MA_PromptFolderPassword(&folder, gui->WI)) == FALSE)
        {
          LEAVE();
          return;
        }
      }
      else if(isProtectedFolder(&folder) && !isProtectedFolder(oldfolder))
      {
        if((changed = FO_EnterPassword(&folder)) == FALSE)
        {
          LEAVE();
          return;
        }
      }

      if(isProtectedFolder(&folder) && isProtectedFolder(oldfolder))
         strlcpy(folder.Password, oldfolder->Password, sizeof(folder.Password));

      if(changed == TRUE)
      {
        if(!isProtectedFolder(&folder))
          folder.Password[0] = '\0';

        if(folder.Mode != oldmode)
        {
          BusyGauge(tr(MSG_BusyUncompressingFO), "", folder.Total);

          LockMailListShared(folder.messages);

          if(IsMailListEmpty(folder.messages) == FALSE)
          {
            struct MailNode *mnode;
            ULONG i = 0;

            ForEachMailNode(folder.messages, mnode)
            {
              BusySet(++i);
              RepackMailFile(mnode->mail, folder.Mode, folder.Password);
            }
          }

          UnlockMailList(folder.messages);

          BusyEnd();

          oldfolder->Mode = newmode;
        }

        strlcpy(oldfolder->Password, folder.Password, sizeof(oldfolder->Password));
      }
      oldfolder->Type = folder.Type;
    }

    if(FO_SaveConfig(&folder) == TRUE)
      success = TRUE;
  }
  else // if not then a new folder should be generated
  {
    int result;

    D(DBF_FOLDER, "new folder");
    isNewFolder = TRUE;
    memset(&folder, 0, sizeof(struct Folder));
    folder.ImageIndex = -1;

    if((folder.messages = CreateMailList()) != NULL)
    {
      FO_PutFolder(&folder);
      D(DBF_FOLDER, "new folder '%s'", folder.Name);

      // set up the full path to the folder
      if(strchr(folder.Path, ':') != NULL)
      {
        // the path is an absolute path already
        strlcpy(folder.Fullpath, folder.Path, sizeof(folder.Fullpath));
      }
      else
      {
        // concatenate the default mail dir and the folder's relative path to an absolute path
        strlcpy(folder.Fullpath, G->MA_MailDir, sizeof(folder.Fullpath));
        AddPart(folder.Fullpath, folder.Path, sizeof(folder.Fullpath));
      }

      // lets first check for a valid folder name
      // if the foldername is empty or the new name already exists it's invalid
      if(folder.Name[0] == '\0' || FO_GetFolderByName(folder.Name, NULL) != NULL)
      {
        MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERNAMEINVALID));

        LEAVE();
        return;
      }

      // lets check if entered folder path is valid or not
      if(folder.Fullpath[0] == '\0')
      {
        MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERPATHINVALID));

        LEAVE();
        return;
      }
      else if(FileExists(folder.Fullpath) == TRUE) // check if something with folder.Path already exists
      {
        result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDEREXISTS));
      }
      else
        result = TRUE;

      // only if the user want to proceed we go on.
      if(result == TRUE)
      {
        if(isProtectedFolder(&folder) && FO_EnterPassword(&folder) == FALSE)
        {
          LEAVE();
          return;
        }

        if(CreateDirectory(folder.Fullpath) == TRUE)
        {
          if(FO_SaveConfig(&folder) == TRUE)
          {
            // allocate memory for the new folder
            if((oldfolder = memdup(&folder, sizeof(folder))) != NULL)
            {
              struct FolderNode *fnode;

              // finally add the new folder to the global list
              LockFolderList(G->folders);
              fnode = AddNewFolderNode(G->folders, oldfolder);
              UnlockFolderList(G->folders);

              if(fnode != NULL)
              {
                struct Folder *prevFolder;

                // allow the listtree to reorder our folder list
                set(lv, MUIA_MainFolderListtree_ReorderFolderList, TRUE);

                prevFolder = FO_GetCurrentFolder();
                if(prevFolder != NULL && isGroupFolder(prevFolder))
                {
                  // add the folder to the end of the current folder group
                  DoMethod(lv, MUIM_NListtree_Insert, oldfolder->Name, fnode, prevFolder->Treenode, MUIV_NListtree_Insert_PrevNode_Tail, MUIV_NListtree_Insert_Flag_Active);
                }
                else
                {
                  // add the folder after the current folder
                  DoMethod(lv, MUIM_NListtree_Insert, oldfolder->Name, fnode, MUIV_NListtree_Insert_ListNode_Active, MUIV_NListtree_Insert_PrevNode_Active, MUIV_NListtree_Insert_Flag_Active);
                }

                // the MainFolderListtree class has catched the insert operation and
                // move the new folder node within the folder list to the correct position.
                set(lv, MUIA_MainFolderListtree_ReorderFolderList, FALSE);

                success = TRUE;
              }
            }
          }
        }
        else
        {
          LEAVE();
          return;
        }
      }
      else
      {
        LEAVE();
        return;
      }
    }
    else
    {
      LEAVE();
      return;
    }
  }

  set(gui->WI, MUIA_Window_Open, FALSE);

  if(success == TRUE)
  {
    MA_SetSortFlag();
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_Title);
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Sort);
    MA_ChangeFolder(FO_GetFolderByName(folder.Name, NULL), FALSE);

    // Save the folder tree only if we just created a new folder, otherwise
    // a temporarily modified open/close state of folder groups will be saved
    // as well, even if the user didn't want this.
    if(isNewFolder == TRUE)
      FO_SaveTree();

    DisplayStatistics(oldfolder, TRUE);
  }

  DisposeModulePush(&G->FO);

  LEAVE();
}
MakeStaticHook(FO_SaveHook, FO_SaveFunc);

///
/// FO_SetOrderFunc
//  Saves or resets folder order
HOOKPROTONHNO(FO_SetOrderFunc, void, enum SetOrder *arg)
{
  ENTER();

  switch (*arg)
  {
    case SO_SAVE:
    {
      FO_SaveTree();
    }
    break;

    case SO_RESET:
    {
      struct FolderNode *fnode;

      // before we reset/reload the foldertree we have to
      // make sure everything is freed correctly.
      LockFolderList(G->folders);

      while((fnode = TakeFolderNode(G->folders)) != NULL)
      {
        struct Folder *folder = fnode->folder;

        if(folder == NULL)
          break;

        // we do not have to call FreeFolder manually, because the
        // destructor of the Listtree will do this for us. But we
        // have to free the FImage of the folder if it exists
        if(folder->imageObject != NULL)
        {
          // we make sure that the NList also doesn't use the image in future anymore
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

          // and last, but not least we free the BC object here, so that this Object is also gone
          MUI_DisposeObject(folder->imageObject);
          // let's set it to NULL so that the destructor doesn't do the work again.
          folder->imageObject = NULL;
        }

        // free this folder
        FO_FreeFolder(folder);
        // and free its node
        DeleteFolderNode(fnode);
      }

      // all folder nodes have been freed, now initialize the list again
      InitFolderList(G->folders);

      UnlockFolderList(G->folders);

      FO_LoadTree();
    }
    break;
  }

  LEAVE();
}
MakeHook(FO_SetOrderHook, FO_SetOrderFunc);
///
/// FO_MLAutoDetectFunc
//  Tries to autodetect the Mailinglist support parameters
HOOKPROTONHNONP(FO_MLAutoDetectFunc, void)
{
  #define SCANMSGS  5

  char *toPattern;
  char *toAddress;
  char *res=NULL;
  const char *notRecog;
  BOOL takePattern = TRUE;
  BOOL takeAddress = TRUE;
  struct Folder *folder;
  struct MailNode *mnode;
  int i;
  BOOL success;

  ENTER();

  folder = G->FO->EditFolder;
  if(folder == NULL)
  {
    LEAVE();
    return;
  }

  if(IsMailListEmpty(folder->messages) == TRUE)
  {
    LEAVE();
    return;
  }

  mnode = FirstMailNode(folder->messages);
  toPattern = mnode->mail->To.Address;
  toAddress = mnode->mail->To.Address;

  LockMailListShared(folder->messages);

  i = 0;
  success = TRUE;
  ForEachMailNode(folder->messages, mnode)
  {
    // skip the first mail as this has already been processed before
    if(i > 0)
    {
      struct Mail *mail = mnode->mail;
      char *result;

      D(DBF_FOLDER, "SWS: [%s] [%s]", toPattern, mail->To.Address);

      // Analyze the toAdress through the Smith&Waterman algorithm
      if(takePattern == TRUE && (result = SWSSearch(toPattern, mail->To.Address)) != NULL)
      {
        free(res);

        if((res = strdup(result)) == NULL)
        {
          success = FALSE;
          break;
        }

        toPattern = res;

        // If we reached a #? pattern then we break here
        if(strcmp(toPattern, "#?") == 0)
          takePattern = FALSE;
      }

      // Lets check if the toAddress kept the same and then we can use
      // it for the TOADDRESS string gadget
      if(takeAddress == TRUE && stricmp(toAddress, mail->To.Address) != 0)
        takeAddress = FALSE;
    }

    if(++i > SCANMSGS)
      break;
  }

  UnlockMailList(folder->messages);

  if(success == TRUE)
  {
    // lets make a pattern out of the found SWS string
    if(takePattern == TRUE)
    {
      if(strlen(toPattern) >= 2 && !(toPattern[0] == '#' && toPattern[1] == '?'))
      {
        if(res != NULL)
          res = realloc(res, strlen(res)+3);
        else if((res = malloc(strlen(toPattern)+3)) != NULL)
          strlcpy(res, toPattern, strlen(toPattern));

        if(res != NULL)
        {
          // move the actual string to the back and copy the wildcard in front of it.
          memmove(&res[2], res, strlen(res)+1);
          res[0] = '#';
          res[1] = '?';

          toPattern = res;
        }
        else
          success = FALSE;
      }

      if(success == TRUE && strlen(toPattern) >= 2 && !(toPattern[strlen(toPattern)-2] == '#' && toPattern[strlen(toPattern)-1] == '?'))
      {
        if(res != NULL)
          res = realloc(res, strlen(res)+3);
        else if((res = malloc(strlen(toPattern)+3)) != NULL)
          strlcpy(res, toPattern, strlen(toPattern));

        if(res != NULL)
        {
          // and now copy also the wildcard at the back of the string
          strcat(res, "#?");

          toPattern = res;
        }
        else
          success = FALSE;
      }
    }

    if(success == TRUE)
    {
      D(DBF_FOLDER, "ML-Pattern: [%s]", toPattern);

      // Now we set the new pattern & address values to the string gadgets
      notRecog = tr(MSG_FO_NOTRECOGNIZED);
      setstring(G->FO->GUI.ST_MLPATTERN, takePattern && toPattern[0] ? toPattern : notRecog);
      setstring(G->FO->GUI.ST_MLADDRESS, takeAddress ? toAddress : notRecog);
    }
  }

  // lets free all resources now
  free(res);

  SWSSearch(NULL, NULL);

  LEAVE();
}
MakeStaticHook(FO_MLAutoDetectHook, FO_MLAutoDetectFunc);

///
/// FolderPathFunc
// set the user's mail directory path as default path instead of YAM's directory
HOOKPROTONHNO(FolderPathFunc, LONG, struct TagItem *tags)
{
  struct TagItem *tag;

  ENTER();

  // search for an already existing drawer tag item
  if((tag = FindTagItem(ASLFR_InitialDrawer, tags)) != NULL)
  {
    // set the initial drawer to the user's mail directory
    tag->ti_Data = (ULONG)G->MA_MailDir;
  }
  else
  {
    // MUI allows us to add up to 15 own tags
    // add the tag for the initial drawer
    tags->ti_Tag = ASLFR_InitialDrawer;
    tags->ti_Data = (ULONG)G->MA_MailDir;
    tags++;
    // terminate the list
    tags->ti_Tag = TAG_DONE;
  }

  RETURN(TRUE);
  return TRUE;
}
MakeStaticHook(FolderPathHook, FolderPathFunc);

///

/// FO_New
//  Creates folder configuration window
static struct FO_ClassData *FO_New(void)
{
  struct FO_ClassData *data;

  ENTER();

  if((data = calloc(1, sizeof(struct FO_ClassData))) != NULL)
  {
    static const char *ftypes[4];
    static const char *fmodes[5];
    static const char *sortopt[8];
    static const char *fsignat[5];

    ftypes[0]  = tr(MSG_FO_FTRcvdMail);
    ftypes[1]  = tr(MSG_FO_FTSentMail);
    ftypes[2]  = tr(MSG_FO_FTBothMail);
    ftypes[3]  = NULL;

    fmodes[0]  = tr(MSG_FO_FMNormal);
    fmodes[1]  = tr(MSG_FO_FMSimple);
    // compression and encryption are only available if XPK is available
    fmodes[2]  = (XpkBase != NULL) ? tr(MSG_FO_FMPack) : NULL;
    fmodes[3]  = (XpkBase != NULL) ? tr(MSG_FO_FMEncPack) : NULL;
    fmodes[4]  = NULL;

    sortopt[0] = tr(MSG_FO_MessageDate);
    sortopt[1] = tr(MSG_FO_DateRecvd);
    sortopt[2] = tr(MSG_Sender);
    sortopt[3] = tr(MSG_Recipient);
    sortopt[4] = tr(MSG_Subject);
    sortopt[5] = tr(MSG_Size);
    sortopt[6] = tr(MSG_Status);
    sortopt[7] = NULL;

    fsignat[0] = tr(MSG_WR_NoSig);
    fsignat[1] = tr(MSG_WR_DefSig);
    fsignat[2] = tr(MSG_WR_AltSig1);
    fsignat[3] = tr(MSG_WR_AltSig2);
    fsignat[4] = NULL;

    data->GUI.WI = WindowObject,
       MUIA_Window_Title, tr(MSG_FO_EditFolder),
       MUIA_HelpNode,  "FO_W",
       MUIA_Window_ID, MAKE_ID('F','O','L','D'),
       MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
       MUIA_Window_TopEdge,  MUIV_Window_TopEdge_Centered,
       WindowContents, VGroup,
          Child, ColGroup(2), GroupFrameT(tr(MSG_FO_Properties)),
             Child, Label2(tr(MSG_CO_Name)),
             Child, data->GUI.ST_FNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),
             Child, Label2(tr(MSG_Path)),
             Child, PopaslObject,
                MUIA_Popasl_Type, ASL_FileRequest,
                MUIA_Popasl_StartHook, &FolderPathHook,
                MUIA_Popstring_String, data->GUI.ST_FPATH = MakeString(SIZE_PATH, ""),
                MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
                ASLFR_DrawersOnly, TRUE,
             End,
             Child, Label2(tr(MSG_FO_MaxAge)),
             Child, HGroup,
                Child, data->GUI.NM_MAXAGE = NumericbuttonObject,
                  MUIA_CycleChain,      1,
                  MUIA_Numeric_Min,     0,
                  MUIA_Numeric_Max,     730,
                  MUIA_Numeric_Format,  tr(MSG_FO_MAXAGEFMT),
                End,
                Child, data->GUI.CH_EXPIREUNREAD = MakeCheck(tr(MSG_FO_EXPIREUNREAD)),
                Child, LLabel1(tr(MSG_FO_EXPIREUNREAD)),
                Child, HSpace(0),
             End,
             Child, Label1(tr(MSG_FO_FolderType)),
             Child, data->GUI.CY_FTYPE = MakeCycle(ftypes,tr(MSG_FO_FolderType)),
             Child, Label1(tr(MSG_FO_FolderMode)),
             Child, data->GUI.CY_FMODE = MakeCycle(fmodes,tr(MSG_FO_FolderMode)),
             Child, Label1(tr(MSG_FO_SortBy)),
             Child, HGroup,
                Child, data->GUI.CY_SORT[0] = MakeCycle(sortopt,tr(MSG_FO_SortBy)),
                Child, data->GUI.CH_REVERSE[0] = MakeCheck(tr(MSG_FO_Reverse)),
                Child, LLabel1(tr(MSG_FO_Reverse)),
            End,
             Child, Label1(tr(MSG_FO_ThenBy)),
             Child, HGroup,
                Child, data->GUI.CY_SORT[1] = MakeCycle(sortopt,tr(MSG_FO_ThenBy)),
                Child, data->GUI.CH_REVERSE[1] = MakeCheck(tr(MSG_FO_Reverse)),
                Child, LLabel1(tr(MSG_FO_Reverse)),
             End,
             Child, Label2(tr(MSG_FO_Welcome)),
             Child, data->GUI.ST_HELLOTEXT = MakeString(SIZE_INTRO,tr(MSG_FO_Welcome)),
             Child, Label2(tr(MSG_FO_Greetings)),
             Child, data->GUI.ST_BYETEXT = MakeString(SIZE_INTRO,tr(MSG_FO_Greetings)),
             Child, Label1(tr(MSG_FO_DSTATS)),
             Child, HGroup,
                Child, data->GUI.CH_STATS = MakeCheck(tr(MSG_FO_DSTATS)),
                Child, HSpace(0),
             End,
          End,
          Child, ColGroup(2), GroupFrameT(tr(MSG_FO_MLSupport)),
             Child, Label2(tr(MSG_FO_MLSUPPORT)),
             Child, HGroup,
                Child, data->GUI.CH_MLSUPPORT = MakeCheck(tr(MSG_FO_MLSUPPORT)),
                Child, HVSpace,
                Child, data->GUI.BT_AUTODETECT = MakeButton(tr(MSG_FO_AUTODETECT)),
             End,
             Child, Label2(tr(MSG_FO_TO_PATTERN)),
             Child, data->GUI.ST_MLPATTERN = MakeString(SIZE_PATTERN,tr(MSG_FO_TO_PATTERN)),
             Child, Label2(tr(MSG_FO_TO_ADDRESS)),
             Child, MakeAddressField(&data->GUI.ST_MLADDRESS, tr(MSG_FO_TO_ADDRESS), MSG_HELP_FO_ST_MLADDRESS, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
             Child, Label2(tr(MSG_FO_FROM_ADDRESS)),
             Child, MakeAddressField(&data->GUI.ST_MLFROMADDRESS, tr(MSG_FO_FROM_ADDRESS), MSG_HELP_FO_ST_MLFROMADDRESS, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
             Child, Label2(tr(MSG_FO_REPLYTO_ADDRESS)),
             Child, MakeAddressField(&data->GUI.ST_MLREPLYTOADDRESS, tr(MSG_FO_REPLYTO_ADDRESS), MSG_HELP_FO_ST_MLREPLYTOADDRESS, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
             Child, Label1(tr(MSG_WR_Signature)),
             Child, data->GUI.CY_MLSIGNATURE = MakeCycle(fsignat, tr(MSG_WR_Signature)),
          End,
          Child, ColGroup(3),
             Child, data->GUI.BT_OKAY = MakeButton(tr(MSG_Okay)),
             Child, HSpace(0),
             Child, data->GUI.BT_CANCEL = MakeButton(tr(MSG_Cancel)),
          End,
       End,
    End;

    if(data->GUI.WI != NULL)
    {
      DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);

      set(data->GUI.ST_FPATH, MUIA_String_Reject, " \";#?(|)");
      set(data->GUI.CH_STATS, MUIA_Disabled, C->WBAppIcon == FALSE && C->DockyIcon == FALSE);

      SetHelp(data->GUI.ST_FNAME,            MSG_HELP_FO_ST_FNAME            );
      SetHelp(data->GUI.ST_FPATH,            MSG_HELP_FO_TX_FPATH            );
      SetHelp(data->GUI.NM_MAXAGE,           MSG_HELP_FO_ST_MAXAGE           );
      SetHelp(data->GUI.CY_FMODE,            MSG_HELP_FO_CY_FMODE            );
      SetHelp(data->GUI.CY_FTYPE,            MSG_HELP_FO_CY_FTYPE            );
      SetHelp(data->GUI.CY_SORT[0],          MSG_HELP_FO_CY_SORT0            );
      SetHelp(data->GUI.CY_SORT[1],          MSG_HELP_FO_CY_SORT1            );
      SetHelp(data->GUI.CH_REVERSE[0],       MSG_HELP_FO_CH_REVERSE          );
      SetHelp(data->GUI.CH_REVERSE[1],       MSG_HELP_FO_CH_REVERSE          );
      SetHelp(data->GUI.ST_MLPATTERN,        MSG_HELP_FO_ST_MLPATTERN        );
      SetHelp(data->GUI.CY_MLSIGNATURE,      MSG_HELP_FO_CY_MLSIGNATURE      );
      SetHelp(data->GUI.CH_STATS,            MSG_HELP_FO_CH_STATS            );
      SetHelp(data->GUI.CH_EXPIREUNREAD,     MSG_HELP_FO_CH_EXPIREUNREAD     );
      SetHelp(data->GUI.CH_MLSUPPORT,        MSG_HELP_FO_CH_MLSUPPORT        );
      SetHelp(data->GUI.BT_AUTODETECT,       MSG_HELP_FO_BT_AUTODETECT       );
      SetHelp(data->GUI.ST_HELLOTEXT,        MSG_HELP_FO_ST_HELLOTEXT        );
      SetHelp(data->GUI.ST_BYETEXT,          MSG_HELP_FO_ST_BYETEXT          );

      DoMethod(data->GUI.BT_OKAY,        MUIM_Notify, MUIA_Pressed,            FALSE,  MUIV_Notify_Application, 2,  MUIM_CallHook,  &FO_SaveHook);
      DoMethod(data->GUI.BT_CANCEL,      MUIM_Notify, MUIA_Pressed,            FALSE,  MUIV_Notify_Application, 2,  MUIM_CallHook,  &FO_CloseHook);
      DoMethod(data->GUI.BT_AUTODETECT,  MUIM_Notify, MUIA_Pressed,            FALSE,  MUIV_Notify_Application, 2,  MUIM_CallHook,  &FO_MLAutoDetectHook);
      DoMethod(data->GUI.WI,             MUIM_Notify, MUIA_Window_CloseRequest,TRUE,   MUIV_Notify_Application, 2,  MUIM_CallHook,  &FO_CloseHook);

      // Now we connect the TriggerValues of the MLSUPPORT Checkbox
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.BT_AUTODETECT,       3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.ST_MLPATTERN,        3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.ST_MLADDRESS,        3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.ST_MLFROMADDRESS,    3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.ST_MLREPLYTOADDRESS, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
      DoMethod(data->GUI.CH_MLSUPPORT,   MUIM_Notify, MUIA_Selected, MUIV_EveryTime,  data->GUI.CY_MLSIGNATURE,      3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    }
    else
    {
      free(data);
      data = NULL;
    }
  }

  RETURN(data);
  return data;
}
///


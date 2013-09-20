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
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/ImageArea.h"

#include "Busy.h"
#include "Config.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "ImageCache.h"
#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Signature.h"
#include "UserIdentity.h"

#include "Debug.h"

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
                                         "drafts",   // FT_DRAFTS
                                         "archive",  // FT_ARCHIVE
                                       };

/***************************************************************************
 Module: Folder Configuration
***************************************************************************/

/// GetCurrentFolder
// returns the currently active folder
struct Folder *GetCurrentFolder(void)
{
  struct Folder *folder;

  ENTER();

  // obtain the semaphore is shared mode as we reading the variable only
  ObtainSemaphoreShared(G->globalSemaphore);
  folder = G->currentFolder;
  ReleaseSemaphore(G->globalSemaphore);

  RETURN(folder);
  return folder;
}

///
/// SetCurrentFolder
// set the currently active folder
void SetCurrentFolder(const struct Folder *folder)
{
  ENTER();

  // obtain the semaphore is exclusive mode as we modifying the variable
  ObtainSemaphore(G->globalSemaphore);
  G->currentFolder = (struct Folder *)folder;
  ReleaseSemaphore(G->globalSemaphore);

  LEAVE();
}

///
/// ActivateFolder
// set the passed folder as the active one
void ActivateFolder(const struct Folder *fo)
{
  ENTER();

  if(fo != NULL)
  {
    // make sure the tree is opened to display it
    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, fo->Treenode, MUIF_NONE);

    nnset(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, fo->Treenode);

    // and remember the new current folder
    SetCurrentFolder(fo);
  }

  LEAVE();
}

///
/// UpdateAllFolderSettings
// update all embedded identity and signature pointers of all folders
void UpdateAllFolderSettings(const struct Config *co)
{
  struct FolderNode *fnode;

  ENTER();

  LockFolderListShared(G->folders);

  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    // replace the old identity and signature pointers with current
    // ones pointing to the new configuration
    if(folder->MLIdentity != NULL)
      folder->MLIdentity = FindUserIdentityByID(&co->userIdentityList, folder->MLIdentity->id);

    if(folder->MLSignature != NULL)
      folder->MLSignature = FindSignatureByID(&co->signatureList, folder->MLSignature->id);
  }

  UnlockFolderList(G->folders);

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
           (!stricmp(arg, FolderName[FT_SPAM]) && isSpamFolder(folder))         ||
           (!stricmp(arg, FolderName[FT_DRAFTS]) && isDraftsFolder(folder)))
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
  struct FolderNode *fnode;

  ENTER();

  LockFolderListShared(G->folders);

  ForEachFolderNode(G->folders, fnode)
  {
    if(cmpf(fnode->folder, attr) == TRUE)
    {
      folder = fnode->folder;
      break;
    }
    i++;
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
  return (BOOL)(!isGroupFolder(f) && strcmp(f->Name, name) == 0);
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
/// FO_GetFolderGroup
// comparison function for FO_GetFolderGroup
static BOOL FO_GetFolderGroup_cmp(const struct Folder *f, const char *name)
{
  return (BOOL)(isGroupFolder(f) && strcmp(f->Name, name) == 0);
}
//  Finds a folder group
struct Folder *FO_GetFolderGroup(const char *name, int *pos)
{
  return FO_GetFolderByAttribute((BOOL (*)(const struct Folder *, const void *))&FO_GetFolderGroup_cmp, (const void *)name, pos);
}
///
/// FO_GetFolderPosition
//  Gets the position of a folder in the list
//  !! must be called with a locked folder list !!
int FO_GetFolderPosition(struct Folder *findfo, BOOL withGroups)
{
  int pos = -1;
  struct FolderNode *fnode;
  int p = 0;

  ENTER();

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
  D(DBF_FOLDER, "load config file '%s' of folder '%s'", fname, fo->Name);
  if((fh = fopen(fname, "r")) != NULL)
  {
    char *buf = NULL;
    size_t buflen = 0;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(getline(&buf, &buflen, fh) >= 3 && strnicmp(buf, "YFC", 3) == 0)
    {
      BOOL statsproc = FALSE;

      // pick a default value for ML support parameters
      fo->MLSignature = GetSignature(&C->signatureList, 0, TRUE);
      fo->MLSupport   = TRUE;

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
          else if(stricmp(buf, "MLIdentityID") == 0)   fo->MLIdentity = FindUserIdentityByID(&C->userIdentityList, strtol(value, NULL, 16));
          else if(stricmp(buf, "MLRepToAddr") == 0)    strlcpy(fo->MLReplyToAddress, value, sizeof(fo->MLReplyToAddress));
          else if(stricmp(buf, "MLAddress") == 0)      strlcpy(fo->MLAddress, value, sizeof(fo->MLAddress));
          else if(stricmp(buf, "MLPattern") == 0)      strlcpy(fo->MLPattern, value, sizeof(fo->MLPattern));
          else if(stricmp(buf, "MLSignatureID") == 0)  fo->MLSignature = FindSignatureByID(&C->signatureList, strtol(value, NULL, 16));
          else if(stricmp(buf, "WriteIntro") == 0)     strlcpy(fo->WriteIntro, value, sizeof(fo->WriteIntro));
          else if(stricmp(buf, "WriteGreetings") == 0) strlcpy(fo->WriteGreetings, value, sizeof(fo->WriteGreetings));
          // obsolete config parameters, convert these to the current stuff and features
          else if(stricmp(buf, "MLFromAddr") == 0)     fo->MLIdentity = FindUserIdentityByAddress(&C->userIdentityList, value);
          else if(stricmp(buf, "MLSignature") == 0)
          {
            int num = atoi(value);

            // zero means no signature, all other values are converted the (n-1)-th signature in the signature list
            if(num <= 0)
              fo->MLSignature = NULL;
            else
              fo->MLSignature = GetSignature(&C->signatureList, num-1, TRUE);
          }
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
        fo->MLSignature  = NULL;
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
  else
    E(DBF_FOLDER, "couldn't open folder config file '%s' of folder '%s' (%s)", fname, fo->Name, strerror(errno));

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
  D(DBF_FOLDER, "save config file '%s' of folder '%s'", fname, fo->Name);
  if((fh = fopen(fname, "w")) != NULL)
  {
    struct DateStamp ds;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fprintf(fh, "YFC3 - YAM Folder Configuration\n");
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
    fprintf(fh, "MLIdentityID   = %08x\n", fo->MLIdentity != NULL ? fo->MLIdentity->id : 0);
    fprintf(fh, "MLRepToAddr    = %s\n", fo->MLReplyToAddress);
    fprintf(fh, "MLPattern      = %s\n", fo->MLPattern);
    fprintf(fh, "MLAddress      = %s\n", fo->MLAddress);
    fprintf(fh, "MLSignatureID  = %08x\n", fo->MLSignature != NULL ? fo->MLSignature->id : 0);
    fprintf(fh, "WriteIntro     = %s\n", fo->WriteIntro);
    fprintf(fh, "WriteGreetings = %s\n", fo->WriteGreetings);
    fclose(fh);

    AddPath(fname, fo->Fullpath, ".index", sizeof(fname));

    if(!isModified(fo))
      SetFileDate(fname, DateStamp(&ds));

    result = TRUE;
  }
  else
  {
    E(DBF_FOLDER, "couldn't open folder config file '%s' of folder '%s' (%s)", fname, fo->Name, strerror(errno));
    ER_NewError(tr(MSG_ER_CantCreateFile), fname);
  }

  RETURN(result);
  return result;
}

///
/// FO_LoadFolderImage
//  Loads the images for the folder that should be displayed in the NListtree
BOOL FO_LoadFolderImage(struct Folder *folder)
{
  BOOL success = FALSE;

  ENTER();

  // first we make sure that valid data is underway.
  if(folder != NULL)
  {
    if(folder->ImageIndex >= FI_MAX)
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
      W(DBF_FOLDER, "imageIndex of folder < FICON_ID_MAX (%ld < %ld)", folder->ImageIndex, FI_MAX);
      folder->imageObject = NULL;
    }
  }
  else
    E(DBF_FOLDER, "folder == NULL");

  RETURN(success);
  return success;
}

///
/// FO_UnloadFolderImage
// unloads an image used by a folder
void FO_UnloadFolderImage(struct Folder *folder)
{
  ENTER();

  if(folder != NULL)
  {
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
  }

  LEAVE();
}

///
/// FO_NewFolder
//  Initializes a new folder and creates its directory
struct Folder *FO_NewFolder(enum FolderType type, const char *path, const char *name)
{
  struct Folder *folder;

  ENTER();

  if((folder = AllocFolder()) != NULL)
  {
    InitFolder(folder, type);

    // set the standard icon images, or none for a custom folder
    switch(type)
    {
      case FT_INCOMING: folder->ImageIndex = FI_INCOMING; break;
      case FT_OUTGOING: folder->ImageIndex = FI_OUTGOING; break;
      case FT_TRASH:    folder->ImageIndex = FI_TRASH;    break;
      case FT_SENT:     folder->ImageIndex = FI_SENT;     break;
      case FT_SPAM:     folder->ImageIndex = FI_SPAM;     break;
      case FT_DRAFTS:   folder->ImageIndex = FI_DRAFTS;   break;
      case FT_ARCHIVE:  folder->ImageIndex = FI_ARCHIVE;  break;
      default:          folder->ImageIndex = -1;          break;
    }

    strlcpy(folder->Path, path, sizeof(folder->Path));
    strlcpy(folder->Name, name, sizeof(folder->Name));

    // set up the full path to the folder
    BuildFolderPath(folder->Fullpath, path, sizeof(folder->Fullpath));
    if(CreateDirectory(folder->Fullpath) == FALSE)
    {
      FreeFolder(folder);
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

    // unload the folder's image
    FO_UnloadFolderImage(folder);

    if(!isGroupFolder(folder))
      ClearMailList(folder->messages);

    D(DBF_FOLDER, "freed folder '%s'", folder->Name);

    // now it's time to deallocate the folder itself
    FreeFolder(folder);
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
/// StripMailDir
// strip the user's mail directory from a path if it is contained
static const char *StripMailDir(const char *path)
{
  ENTER();

  if(G->MA_MailDir[0] != '\0' && strnicmp(path, G->MA_MailDir, strlen(G->MA_MailDir)) == 0)
  {
    // strip the user's mail directory from the folder path
    path = &path[strlen(G->MA_MailDir)];
    if(*path == '/')
      path++;
  }

  RETURN(path);
  return path;
}

///
/// BuildFolderPath
// build a full path to a folder
char *BuildFolderPath(char *fullpath, const char *path, size_t fullpathSize)
{
  ENTER();

  CreateFilename("Folders", fullpath, fullpathSize);
  AddPart(fullpath, path, fullpathSize);

  RETURN(fullpath);
  return fullpath;
}

///
/// FO_LoadTree
//  Loads folder list from a file
enum LoadTreeResult FO_LoadTree(void)
{
  char dotFoldersPath[SIZE_PATHFILE];
  char foldersPath[SIZE_PATHFILE];
  enum LoadTreeResult result = LTR_Failure;
  int nested = 0;
  int i = 0;
  int j = FI_MAX;
  FILE *fh;
  APTR lv = G->MA->GUI.NL_FOLDERS;
  struct MUI_NListtree_TreeNode *tn_root = MUIV_NListtree_Insert_ListNode_Root;
  struct FolderNode *fnode_root = NULL; // NULL == root

  ENTER();

  CreateFilename(".folders", dotFoldersPath, sizeof(dotFoldersPath));
  D(DBF_FOLDER, "load folder tree from file '%s'", dotFoldersPath);

  CreateFilename("Folders", foldersPath, sizeof(foldersPath));
  if(CreateDirectory(foldersPath) == TRUE)
  {
    if((fh = fopen(dotFoldersPath, "r")) != NULL)
    {
      char *buffer = NULL;
      size_t size = 0;
      ULONG version = 0;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      if(GetLine(&buffer, &size, fh) >= 3 && strncmp(buffer, "YFO", 3) == 0)
      {
        version = atoi(&buffer[3]);

        D(DBF_FOLDER, "found version V%ld folder tree file", version);

        if(version == 1)
        {
          LONG res;

          // ask the user if moving the folders is desired
          res = MUI_Request(G->App, NULL, MUIF_NONE,
                            tr(MSG_FOLDER_MOVE_WARNING_TITLE),
                            tr(MSG_FOLDER_MOVE_WARNING_GADS),
                            tr(MSG_FOLDER_MOVE_WARNING),
                            G->MA_MailDir);
          if(res == 0)
          {
            result = LTR_QuitYAM;
            goto failure;
          }
        }

        set(lv, MUIA_NListtree_Quiet, TRUE);
        while(GetLine(&buffer, &size, fh) >= 0)
        {
          if(strncmp(buffer, "@FOLDER", 7) == 0)
          {
            struct Folder *fo;

            if((fo = calloc(1, sizeof(*fo))) != NULL)
            {
              InitFolder(fo, FT_CUSTOM);
              strlcpy(fo->Name, Trim(&buffer[8]), sizeof(fo->Name));

              GetLine(&buffer, &size, fh);
              if(version == 1)
              {
                // V1 tree files might contain full folder path names in other locations than <MAILDIR>/Folders
                strlcpy(fo->Path, StripMailDir(Trim(buffer)), sizeof(fo->Path));
              }
              else
              {
                // V2 tree files contain the directory name only
                strlcpy(fo->Path, Trim(buffer), sizeof(fo->Path));
              }

              if((fo->messages = CreateMailList()) != NULL)
              {
                // set up the full path to the folder
                BuildFolderPath(fo->Fullpath, FilePart(fo->Path), sizeof(fo->Fullpath));

                if(version == 1)
                {
                  char v1path[SIZE_PATHFILE];

                  if(strchr(fo->Path, ':') != NULL)
                  {
                    // the path is an absolute path already
                    strlcpy(v1path, fo->Path, sizeof(v1path));
                  }
                  else
                  {
                    // concatenate the default mail dir and the folder's relative path to an absolute path
                    AddPath(v1path, G->MA_MailDir, fo->Path, sizeof(v1path));
                  }

                  D(DBF_FOLDER, "old V1 path '%s'", v1path);
                  D(DBF_FOLDER, "new V2 path '%s'", fo->Fullpath);

                  // first check if the new path differs from the old one
                  if(strcasecmp(v1path, fo->Fullpath) != 0)
                  {
                    // then check if the new folder directory already exists
                    if(FileExists(fo->Fullpath) == TRUE)
                    {
                      char path[SIZE_PATH];
                      ULONG count;

                      // create a new folder directory name by appending a unique number
                      D(DBF_FOLDER, "directory '%s' already exists, creating new unique path", fo->Fullpath);
                      strlcpy(path, fo->Fullpath, sizeof(path));
                      count = 0;
                      do
                      {
                        count++;
                        snprintf(fo->Fullpath, sizeof(fo->Fullpath), "%s_%ld", path, count);
                      }
                      while(FileExists(fo->Fullpath) == TRUE);
                    }

                    D(DBF_FOLDER, "move folder contents from '%s' to '%s'", v1path, fo->Fullpath);
                    MoveDirectory(v1path, fo->Fullpath);
                  }

                  // use the unique directory name only as path from now on
                  strlcpy(fo->Path, FilePart(fo->Fullpath), sizeof(fo->Path));
                }

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
                    else if(stricmp(folderpath, FolderName[FT_DRAFTS]) == 0)
                      fo->Type = FT_DRAFTS;

                    // Save the config now because it could be changed in the meantime
                    if(FO_SaveConfig(fo) == FALSE)
                    {
                      FreeFolder(fo);
                      goto failure;
                    }
                  }

                  fo->SortIndex = i++;
                  fo->ImageIndex = j;

                  // now we load the folder images if they exists
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

                  // now we add this folder to the folder listtree
                  if(fnode == NULL ||
                     (tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE)) == NULL)
                  {
                    FreeFolder(fo);
                    goto failure;
                  }

                  // remember the treenode and folder root
                  fo->Treenode = tn;
                  fo->parent = fnode_root;
                }
              }
              else
              {
                FreeFolder(fo);
                goto failure;
              }

              // skip all lines until we find the folder end marker
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
              InitFolder(fo, FT_GROUP);
              strlcpy(fo->Name, Trim(&buffer[11]), sizeof(fo->Name));
              do
              {
                if(strcmp(buffer, "@ENDSEPARATOR") == 0)
                  break;
              }
              while(GetLine(&buffer, &size, fh) >= 0);
              fo->SortIndex = i++;

              LockFolderList(G->folders);
              fnode = AddNewFolderNode(G->folders, fo);
              UnlockFolderList(G->folders);

              if(fnode == NULL ||
                 (tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags)) == NULL)
              {
                FreeFolder(fo);
                goto failure;
              }

              // remember the treenode and folder root
              fo->Treenode = tn;
              fo->parent = NULL;
            }
          }
          else if(strncmp(buffer, "@GROUP", 6) == 0)
          {
            struct Folder *fo;

            if((fo = calloc(1, sizeof(*fo))) != NULL)
            {
              long tnflags = TNF_LIST;
              struct FolderNode *fnode;

              InitFolder(fo, FT_GROUP);
              strlcpy(fo->Name, Trim(&buffer[7]), sizeof(fo->Name));

              // now we check if the node should be open or not
              if(GetLine(&buffer, &size, fh) >= 0)
              {
                // if it is greater zero then the node should be displayed open
                if(atoi(buffer) > 0)
                  setFlag(tnflags, TNF_OPEN);
              }

              LockFolderList(G->folders);
              fnode = AddNewFolderNode(G->folders, fo);
              UnlockFolderList(G->folders);

              // now we are going to add this treenode to the list
              if(fnode == NULL ||
                 (tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo->Name, fnode, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags)) == NULL)
              {
                FreeFolder(fo);
                goto failure;
              }

              // remember the treenode and folder parent
              fo->Treenode = tn_root;
              fo->parent = fnode_root;

              // set the new parent folder to this one
              fnode_root = fnode;

              nested++;
            }
          }
          else if(strcmp(buffer,"@ENDGROUP") == 0)
          {
            nested--;

            // now we check if the nested is zero and if yes then we set tn_root = MUIV_NListtree_Insert_ListNode_Root
            // otherwise we go back to the root of the root
            if(nested == 0)
            {
              tn_root = MUIV_NListtree_Insert_ListNode_Root;
              fnode_root = NULL;
            }
            else
            {
              tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
              if(tn_root != NULL)
                fnode_root = (struct FolderNode *)tn_root->tn_User;
              else
                fnode_root = NULL;
            }
          }
        }

        xset(lv, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible,
                 MUIA_NListtree_Quiet,  FALSE);

        result = LTR_Success;
      }

failure:
      fclose(fh);

      free(buffer);

      // in case of an error we didn't leave the loop above in a normal fashion and hence have to
      // revert the listtree's quiet state
      if(result == LTR_Failure)
        set(lv, MUIA_NListtree_Quiet,  FALSE);

      // save the new V2 tree file in case we just successfully loaded a V1 tree file
      if(result == LTR_Success && version == 1)
      {
        D(DBF_FOLDER, "save V2 folder tree file");
        FO_SaveTree();
      }
    }
    else
      E(DBF_FOLDER, "failed to open folder tree file '%s'", dotFoldersPath);

    // if the nest count is still greater zero we have a misconfiguration
    if(result == LTR_Success && nested > 0)
    {
      E(DBF_FOLDER, "malformed folder tree file '%s', nest counter %ld", dotFoldersPath, nested);
      result = LTR_Failure;
    }
  }
  else
  {
    E(DBF_FOLDER, "failed to create '%s' directory", foldersPath);
    result = LTR_Failure;
  }

  RETURN(result);
  return result;
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
        fprintf(fh, "@GROUP %s\n"
                    "%d\n",
                    fo->Name, isFlagSet(tn->tn_Flags, TNF_OPEN));

        // Now we recursively save this subtree first
        success = FO_SaveSubTree(fh, tn);
      }
      else
      {
        fprintf(fh, "@FOLDER %s\n"
                    "%s\n"
                    "@ENDFOLDER\n",
                    fo->Name, fo->Path);
      }

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
  char dotFoldersPath[SIZE_PATHFILE];
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

  CreateFilename(".folders", dotFoldersPath, sizeof(dotFoldersPath));

  if((fh = fopen(dotFoldersPath, "w")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fputs("YFO2 - YAM Folders\n", fh);

    success = FO_SaveSubTree(fh, MUIV_NListtree_GetEntry_ListNode_Root);

    fclose(fh);
  }
  else
    ER_NewError(tr(MSG_ER_CantCreateFile), dotFoldersPath);

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
      folder->ImageIndex = (folder->Unread != 0) ? FI_INCOMING_NEW : FI_INCOMING;
    else if(isOutgoingFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FI_OUTGOING_NEW : FI_OUTGOING;
    else if(isTrashFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FI_TRASH_NEW : FI_TRASH;
    else if(isSentFolder(folder))
      folder->ImageIndex = FI_SENT;
    else if(isSpamFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FI_SPAM_NEW : FI_SPAM;
    else if(isDraftsFolder(folder))
      folder->ImageIndex = (folder->Unread != 0) ? FI_DRAFTS_NEW : FI_DRAFTS;
    else if(isArchiveFolder(folder))
      folder->ImageIndex = FI_ARCHIVE;
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

  // make sure we don't deal with a group folder and
  // the folder's index is valid. There is no point in
  // updating the stats of a folder with a flushed index.
  if(isGroupFolder(folder) == FALSE && folder->LoadedMode == LM_VALID)
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
BOOL FO_MoveFolderDir(struct Folder *fo, struct Folder *oldfo)
{
  BOOL success = TRUE;
  char totalStr[SIZE_SMALL];
  struct BusyNode *busy;
  struct MailNode *mnode;
  ULONG i;

  ENTER();

  busy = BusyBegin(BUSY_PROGRESS_ABORT);
  snprintf(totalStr, sizeof(totalStr), "%d", fo->Total);
  BusyText(busy, tr(MSG_BusyMoving), totalStr);

  LockMailListShared(fo->messages);

  i = 0;
  ForEachMailNode(fo->messages, mnode)
  {
    struct Mail *mail = mnode->mail;
    char srcbuf[SIZE_PATHFILE];
    char dstbuf[SIZE_PATHFILE];

    if(BusyProgress(busy, ++i, fo->Total) == FALSE)
    {
      success = FALSE;
      break;
    }

    GetMailFile(dstbuf, sizeof(dstbuf), fo, mail);
    GetMailFile(srcbuf, sizeof(srcbuf), oldfo, mail);

    D(DBF_FOLDER, "move mail file '%s' to '%s'", srcbuf, dstbuf);
    if(MoveFile(srcbuf, dstbuf) == TRUE)
    {
      success = RepackMailFile(mail, fo->Mode, fo->Password);
    }
    else
    {
      W(DBF_FOLDER, "failed to move file '%s' to '%s'", srcbuf, dstbuf);
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
      W(DBF_FOLDER, "failed to move file '%s' to '%s'", srcbuf, dstbuf);
      success = FALSE;
    }
    else
    {
      // now we try to move the .fimage file aswell
      AddPath(srcbuf, oldfo->Fullpath, ".fimage", sizeof(srcbuf));
      AddPath(dstbuf, fo->Fullpath, ".fimage", sizeof(dstbuf));
      if(FileExists(srcbuf) == TRUE && MoveFile(srcbuf, dstbuf) == FALSE)
      {
        W(DBF_FOLDER, "failed to copy file '%s' to '%s'", srcbuf, dstbuf);
        success = FALSE;
      }
      else
      {
        // if we were able to successfully move all files
        // we can also delete the source directory. However,
        // we are NOT doing any error checking here as the
        // source may be a VOLUME and as such not deletable.
        DeleteMailDir(oldfo->Fullpath, FALSE);
      }
    }
  }

  BusyEnd(busy);

  RETURN(success);
  return success;
}

///

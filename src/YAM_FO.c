/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

#include "ImageCache.h"
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

/// FO_CreateList
//  Creates a linked list of all folders
struct Folder **FO_CreateList(void)
{
  int max;
  struct Folder **flist;
  APTR lv;

  ENTER();

  lv = G->MA->GUI.NL_FOLDERS;
  max = DoMethod(lv, MUIM_NListtree_GetNr, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_GetNr_Flag_CountAll);

  if((flist = calloc(max + 1, sizeof(struct Folder *))) != NULL)
  {
    int i;
    struct Folder **fPtr;

    flist[0] = (struct Folder *)max;
    fPtr = &flist[1];

    for(i = 0; i < max; i++)
    {
      struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);

      if(tn == NULL)
      {
        free(flist);
        flist = NULL;
        break;
      }
      else
      {
        // put the folder in the list
        *fPtr++ = tn->tn_User;
      }
    }
  }

  RETURN(flist);
  return flist;
}

///
/// FO_GetCurrentFolder
//  Returns pointer to active folder
struct Folder *FO_GetCurrentFolder(void)
{
  struct Folder *folder = NULL;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  if((tn = (struct MUI_NListtree_TreeNode *)xget(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active)) != NULL)
    folder = (struct Folder *)tn->tn_User;

  RETURN(folder);
  return folder;
}

///
/// FO_SetCurrentFolder
//  Set the passed folder as the active one
BOOL FO_SetCurrentFolder(struct Folder *fo)
{
  BOOL result = FALSE;

  ENTER();

  if(fo != NULL)
  {
    int i;

    for(i = 0;; i++)
    {
      struct MUI_NListtree_TreeNode *tn;

      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
      if(tn != NULL && tn->tn_User == fo)
      {
        // make sure the tree is opened to display it
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Open, MUIV_NListtree_Open_ListNode_Parent, tn, MUIF_NONE);

        nnset(G->MA->GUI.NL_FOLDERS, MUIA_NListtree_Active, tn);
        result = TRUE;
        break;
      }
    }
  }

  RETURN(result);
  return result;
}
///
/// FO_GetFolderRexx
//  Finds a folder by its name, type or position
struct Folder *FO_GetFolderRexx(const char *arg, int *pos)
{
  struct Folder *fo = NULL;
  struct Folder **flist;

  ENTER();

  if((flist = FO_CreateList()) != NULL)
  {
    int nr = 0;
    const char *p = arg;
    BOOL numeric = TRUE;

    // lets find out if the user wants to have the folder identified by it`s position
    while(*p != '\0')
    {
      int c = (int)*p++;

      if(!isdigit(c))
      {
        numeric = FALSE;
        break;
      }
    }

    // if this is a numeric search we go on.
    if(numeric)
    {
      int i = atoi(arg);
      int k = 0;

      if(i >= 0 && i < (int)*flist)
      {
        int j;

        for(j = 1; j <= (int)*flist; j++)
        {
          // if the current one is a FT_GROUP we go to the next one until we find
          // the correct one
          if(isGroupFolder(flist[j]) == FALSE)
          {
            if(k == i)
            {
              nr = j;
              break;
            }
            k++;
          }
        }
      }
    }

    // for string folder search
    if(nr == 0)
    {
      int i;

      for(i = 1; i <= (int)*flist; i++)
      {
        if((!Stricmp(arg, flist[i]->Name) && !isGroupFolder(flist[i]))    ||
           (!stricmp(arg, FolderName[FT_INCOMING]) && isIncomingFolder(flist[i]))  ||
           (!stricmp(arg, FolderName[FT_OUTGOING]) && isOutgoingFolder(flist[i]))  ||
           (!stricmp(arg, FolderName[FT_SENT]) && isSentFolder(flist[i]))      ||
           (!stricmp(arg, FolderName[FT_TRASH]) && isTrashFolder(flist[i]))   ||
           (!stricmp(arg, FolderName[FT_SPAM]) && isSpamFolder(flist[i])))
        {
          nr = i;
          break;
        }
      }
    }

    if(nr != 0)
    {
      fo = flist[nr];
      if(pos != NULL)
        *pos = --nr;
    }

    free(flist);
  }

  RETURN(fo);
  return fo;
}
///
/// FO_GetFolderByAttribute
//  Generalized find-folder function
static struct Folder *FO_GetFolderByAttribute(BOOL (*cmpf)(const struct Folder*, const void*), const void *attr, int *pos)
{
  int i;
  struct Folder *folder = NULL;

  ENTER();

  for(i=0; ;i++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct Folder *fo;

      if((fo = tn->tn_User) != NULL)
      {
        if(cmpf(fo, attr))
        {
          folder = fo;
          break;
        }
      }
      else
        break;
    }
    else
      break;
  }

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
  return (BOOL)(!strcmp(f->Name, name) && (!isGroupFolder(f)));
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
  return (BOOL)(!stricmp(f->Path, path));
}
//  Finds a folder by its path
struct Folder *FO_GetFolderByPath(const char *path, int *pos)
{
  return FO_GetFolderByAttribute((BOOL (*)(const struct Folder *, const void *))&FO_GetFolderByPath_cmp, (const void *)path, pos);
}
///
/// FO_GetFolderPosition
//  Gets the position of a folder in the list
int FO_GetFolderPosition(struct Folder *findfo, BOOL withGroups)
{
  int pos = -1;
  int i, j;

  ENTER();

  for(i = 0, j = 0; ;i++, j++)
  {
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE)) != NULL)
    {
      struct Folder *fo;

      if((fo = tn->tn_User) != NULL)
      {
        if(withGroups == FALSE && isGroupFolder(fo))
          j--;
        if(fo == findfo)
        {
          // success
          pos = j;
          break;
        }
      }
      else
        break;
    }
    else
      break;
  }

  RETURN(pos);
  return pos;
}
///
/// FO_GetFolderTreeNode
//  Gets the tree node of a folder
struct MUI_NListtree_TreeNode *FO_GetFolderTreeNode(struct Folder *findfo)
{
  int i;
  struct Folder *fo;
  struct MUI_NListtree_TreeNode *tn;

  ENTER();

  for(i=0;;i++)
  {
    tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIF_NONE);
    if(tn == NULL || tn->tn_User == NULL)
    {
      // no more treenodes or invalid user data, bail out
      tn = NULL;
      break;
    }

    fo = tn->tn_User;
    if(fo == findfo)
    {
      // we just found the desired folder, so break out of the loop and return the treenode
      break;
    }
  }

  RETURN(tn);
  return tn;
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

  strlcpy(fname, GetFolderDir(fo), sizeof(fname));
  AddPart(fname, ".fconfig", sizeof(fname));

  if((fh = fopen(fname, "r")) != NULL)
  {
    char buffer[SIZE_LARGE];

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    if(fgets(buffer, SIZE_LARGE, fh) && strnicmp(buffer, "YFC", 3) == 0)
    {
      BOOL statsproc = FALSE;

      // pick a default value for ML support parameters
      fo->MLSignature  = 1;
      fo->MLSupport    = TRUE;

      while(fgets(buffer, sizeof(buffer), fh))
      {
        char *p;
        char *value;

        if((value = strchr(buffer, '=')) != NULL)
          for(++value; isspace(*value); value++);

        if((p = strpbrk(buffer,"\r\n")) != NULL)
          *p = '\0';

        for(p = buffer; *p != '\0' && !isspace(*p); p++);
        *p = '\0';

        if(*buffer != '\0' && value != NULL)
        {
          if(!stricmp(buffer, "Name"))              strlcpy(fo->Name, value, sizeof(fo->Name));
          else if(!stricmp(buffer, "MaxAge"))       fo->MaxAge = atoi(value);
          else if(!stricmp(buffer, "Password"))     strlcpy(fo->Password, Decrypt(value), sizeof(fo->Password));
          else if(!stricmp(buffer, "Type"))         fo->Type = atoi(value);
          else if(!stricmp(buffer, "XPKType"))      fo->Mode = atoi(value); // valid < v2.4
          else if(!stricmp(buffer, "Mode"))         fo->Mode = atoi(value);
          else if(!stricmp(buffer, "Sort1"))        fo->Sort[0] = atoi(value);
          else if(!stricmp(buffer, "Sort2"))        fo->Sort[1] = atoi(value);
          else if(!stricmp(buffer, "Stats"))        { fo->Stats = Txt2Bool(value); statsproc = TRUE; }
          else if(!stricmp(buffer, "MLSupport"))    fo->MLSupport = Txt2Bool(value);
          else if(!stricmp(buffer, "MLFromAddr"))   strlcpy(fo->MLFromAddress, value, sizeof(fo->MLFromAddress));
          else if(!stricmp(buffer, "MLRepToAddr"))  strlcpy(fo->MLReplyToAddress, value, sizeof(fo->MLReplyToAddress));
          else if(!stricmp(buffer, "MLAddress"))    strlcpy(fo->MLAddress, value, sizeof(fo->MLAddress));
          else if(!stricmp(buffer, "MLPattern"))    strlcpy(fo->MLPattern, value, sizeof(fo->MLPattern));
          else if(!stricmp(buffer, "MLSignature"))  fo->MLSignature = atoi(value);
          else if(!stricmp(buffer, "WriteIntro"))     strlcpy(fo->WriteIntro, value, sizeof(fo->WriteIntro));
          else if(!stricmp(buffer, "WriteGreetings")) strlcpy(fo->WriteGreetings, value, sizeof(fo->WriteGreetings));
        }
      }
      success = TRUE;

      if(!statsproc)
        fo->Stats = !isIncomingFolder(fo);

      // check for non custom folder
      // and set some values which shouldn`t be changed
      if(isDefaultFolder(fo))
      {
        fo->MLSignature  = -1;
        fo->MLSupport    = FALSE;
      }

      // mark an old spam folder as custom folder, so it can be deleted
      if(fo->Type == FT_SPAM && !C->SpamFilterEnabled)
        fo->Type = FT_CUSTOM;
    }
    fclose(fh);
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

  strlcpy(fname, GetFolderDir(fo), sizeof(fname));
  AddPart(fname, ".fconfig", sizeof(fname));

  if((fh = fopen(fname, "w")) != NULL)
  {
    struct DateStamp ds;

    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    fprintf(fh, "YFC2 - YAM Folder Configuration\n");
    fprintf(fh, "Name        = %s\n",  fo->Name);
    fprintf(fh, "MaxAge      = %d\n",  fo->MaxAge);
    fprintf(fh, "Password    = %s\n",  Encrypt(fo->Password));
    fprintf(fh, "Type        = %d\n",  fo->Type);
    fprintf(fh, "Mode        = %d\n",  fo->Mode);
    fprintf(fh, "Sort1       = %d\n",  fo->Sort[0]);
    fprintf(fh, "Sort2       = %d\n",  fo->Sort[1]);
    fprintf(fh, "Stats       = %s\n",  Bool2Txt(fo->Stats));
    fprintf(fh, "MLSupport   = %s\n",  Bool2Txt(fo->MLSupport));
    fprintf(fh, "MLFromAddr  = %s\n",  fo->MLFromAddress);
    fprintf(fh, "MLRepToAddr = %s\n",  fo->MLReplyToAddress);
    fprintf(fh, "MLPattern   = %s\n",  fo->MLPattern);
    fprintf(fh, "MLAddress   = %s\n",  fo->MLAddress);
    fprintf(fh, "MLSignature = %d\n",  fo->MLSignature);
    fprintf(fh, "WriteIntro  = %s\n",  fo->WriteIntro);
    fprintf(fh, "WriteGreetings = %s\n",  fo->WriteGreetings);
    fclose(fh);

    strlcpy(fname, GetFolderDir(fo), sizeof(fname));
    AddPart(fname, ".index", sizeof(fname));

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

  if((folder = calloc(1, sizeof(struct Folder))) != NULL)
  {
    folder->Sort[0] = 1;
    folder->Sort[1] = 3;
    folder->Type = type;
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
    if(!CreateDirectory(GetFolderDir(folder)))
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
    // remove the image of this folder from the objectlist at the application
    if(folder->imageObject != NULL)
    {
      // Here we cannot remove the BC_FImage from the BC_GROUP because the
      // destructor of the Folder Listtree will call this function and then
      // this BC_GROUP doesn`t exists anymore. -> Enforcer hit !
      // so if the user is going to remove this folder by hand we will remove
      // the BC_FImage of it in the FO_DeleteFolderFunc() before the destructor
      // is going to call this function.

      // remove the bodychunk object from the nlist
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

      // the image object itself will be freed by
      // the folder object itself as it is part of the hierarchy since we
      // added it with OM_ADDMEMBER.
    }

    // now it`s time to deallocate the folder itself
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
    struct MUI_NListtree_TreeNode *tn;

    if((tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder->Name, folder, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE)) != NULL)
    {
      if(FO_SaveConfig(folder))
      {
        // only if we reach here everything was fine and we can return TRUE
        result = TRUE;
      }
      else
      {
        // If we reach here the SaveConfig() returned FALSE and we need to remove the folder again
        // from the listtree. But we MUST pass the treenode and NOT the folder, because the folder
        // pointer is no valid treenode but just the user data!!
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Insert_ListNode_Root, tn, MUIF_NONE);
      }
    }

    free(folder);
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

      strlcpy(fname, GetFolderDir(folder), sizeof(fname));
      AddPart(fname, ".fimage", sizeof(fname));

      if(FileExists(fname))
      {
        folder->imageObject = ImageAreaObject,
                                MUIA_ImageArea_Filename, fname,
                              End;

        // Now we say that this image could be used by this Listtree
        if(folder->imageObject != NULL)
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
BOOL FO_LoadTree(char *fname)
{
  static struct Folder fo;
  BOOL success = FALSE;
  char buffer[SIZE_LARGE];
  int nested = 0, i = 0, j = MAX_FOLDERIMG+1;
  FILE *fh;
  APTR lv = G->MA->GUI.NL_FOLDERS;
  struct MUI_NListtree_TreeNode *tn_root = MUIV_NListtree_Insert_ListNode_Root;

  ENTER();

  if((fh = fopen(fname, "r")) != NULL)
  {
    setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

    GetLine(fh, buffer, sizeof(buffer));
    if(strncmp(buffer, "YFO", 3) == 0)
    {
      DoMethod(lv, MUIM_NListtree_Clear, NULL, 0);
      set(lv, MUIA_NListtree_Quiet, TRUE);
      while(GetLine(fh, buffer, sizeof(buffer)))
      {
        memset(&fo, 0, sizeof(struct Folder));
        if(strncmp(buffer, "@FOLDER", 7) == 0)
        {
          fo.Type = FT_CUSTOM;
          fo.Sort[0] = 1;
          fo.Sort[1] = 3;
          strlcpy(fo.Name, Trim(&buffer[8]), sizeof(fo.Name));
          strlcpy(fo.Path, Trim(GetLine(fh, buffer, sizeof(buffer))), sizeof(fo.Path));

          if(CreateDirectory(GetFolderDir(&fo)) == TRUE)
          {
            // if there doesn't exist any .fconfig configuration in the folder
            // we do have to generate it and we do that by analyzing its name,
            // comparing it to the default folder names we know.
            if(FO_LoadConfig(&fo) == FALSE)
            {
              char *folderpath = (char *)FilePart(fo.Path);

              // check if this is a so-called "standard" folder (INCOMING/OUTGOING etc.)
              if(stricmp(folderpath, FolderName[FT_INCOMING]) == 0)
                fo.Type = FT_INCOMING;
              else if(stricmp(folderpath, FolderName[FT_OUTGOING]) == 0)
                fo.Type = FT_OUTGOING;
              else if(stricmp(folderpath, FolderName[FT_SENT]) == 0)
                fo.Type = FT_SENT;
              else if(stricmp(folderpath, FolderName[FT_TRASH]) == 0)
                fo.Type = FT_TRASH;
              else if(C->SpamFilterEnabled && stricmp(folderpath, FolderName[FT_SPAM]) == 0)
                fo.Type = FT_SPAM;

              // Save the config now because it could be changed in the meantime
              if(FO_SaveConfig(&fo) == FALSE)
              {
                fclose(fh);

                RETURN(FALSE);
                return FALSE;
              }
            }

            fo.SortIndex = i++;
            fo.ImageIndex = j;

            // Now we load the FolderImages if they exists
            if(FO_LoadFolderImage(&fo))
              j++;
            else
            {
              // we cannot find out if there is new/unread mail in the folder,
              // so we initialize the folder with the std ImageIndex.
              if(isIncomingFolder(&fo))      fo.ImageIndex = FICON_ID_INCOMING;
              else if(isOutgoingFolder(&fo)) fo.ImageIndex = FICON_ID_OUTGOING;
              else if(isTrashFolder(&fo))    fo.ImageIndex = FICON_ID_TRASH;
              else if(isSentFolder(&fo))     fo.ImageIndex = FICON_ID_SENT;
              else if(isSpamFolder(&fo))     fo.ImageIndex = FICON_ID_SPAM;
              else fo.ImageIndex = -1; // or with -1 for a non std folder.
            }

            // Now we add this folder to the folder listtree
            if(!(DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, MUIF_NONE)))
            {
              fclose(fh);

              RETURN(FALSE);
              return FALSE;
            }
          }
          do
            if(strcmp(buffer, "@ENDFOLDER") == 0)
              break;
          while(GetLine(fh, buffer, sizeof(buffer)));
        }
        else if(strncmp(buffer, "@SEPARATOR", 10) == 0)
        {
          long tnflags = TNF_LIST;

          // SEPARATOR support is obsolete since the folder hierachical order
          // that`s why we handle SEPARATORs as GROUPs now for backward compatibility
          fo.Type = FT_GROUP;
          strlcpy(fo.Name, Trim(&buffer[11]), sizeof(fo.Name));
          do
            if(strcmp(buffer, "@ENDSEPARATOR") == 0)
              break;
          while(GetLine(fh, buffer, sizeof(buffer)));
          fo.SortIndex = i++;

          // Now we check if the foldergroup image was loaded and if not we enable the standard NListtree image
          if(IsImageInCache("folder_fold") &&
             IsImageInCache("folder_unfold"))
          {
            SET_FLAG(tnflags, TNF_NOSIGN);
          }

          if((Object *)DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags) == NULL)
          {
            fclose(fh);

            RETURN(FALSE);
            return FALSE;
          }
        }
        else if(strncmp(buffer, "@GROUP", 6) == 0)
        {
          long tnflags = (TNF_LIST);

          fo.Type = FT_GROUP;
          strlcpy(fo.Name, Trim(&buffer[7]), sizeof(fo.Name));

          // now we check if the node should be open or not
          if(GetLine(fh, buffer, sizeof(buffer)))
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

          // now we are going to add this treenode to the list
          if(!(tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags)))
          {
            fclose(fh);

            RETURN(FALSE);
            return FALSE;
          }

          nested++;
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

      SetAttrs(lv, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible,
                   MUIA_NListtree_Quiet,  FALSE,
                   TAG_DONE);
    }
    fclose(fh);
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
      if(i == 0) tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, i, MUIF_NONE);
      else tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Next, MUIF_NONE);

      // get the parent node of the treenode
      tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, MUIF_NONE);
    }

    // if tn is null or the parent of the next is not the same like the caller of this function
    // we are going to print ENDGROUP and return
    if (!tn || tn_parent != subtree)
    {
      // if we reach here it`s just the end of a GROUP
      if(!noendgroup)
        fputs("@ENDGROUP\n", fh);

      break;
    }
    else
    {
      fo = tn->tn_User;
      if (!fo) break;
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
BOOL FO_SaveTree(char *fname)
{
  BOOL success = FALSE;
  FILE *fh;

  ENTER();

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
/// FO_UpdateStatistics
// recalculate the number of new/unread/etc mails in a folder
void FO_UpdateStatistics(struct Folder *folder)
{
  struct Mail *mail;

  ENTER();

  if(folder == (struct Folder *)-1)
    folder = FO_GetFolderByType(FT_INCOMING, NULL);

  folder->Unread = 0;
  folder->New = 0;
  folder->Total = 0;
  folder->Sent = 0;
  folder->Deleted = 0;

  // now we recount the amount of messages of this folder
  for(mail = folder->Messages; mail != NULL; mail = mail->Next)
  {
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

  LEAVE();
}
///
/// FO_MoveFolderDir
//  Moves a folder to a new directory
static BOOL FO_MoveFolderDir(struct Folder *fo, struct Folder *oldfo)
{
  struct Mail *mail;
  char srcbuf[SIZE_PATHFILE], dstbuf[SIZE_PATHFILE];
  BOOL success = TRUE;
  int i;

  ENTER();

  BusyGauge(tr(MSG_BusyMoving), itoa(fo->Total), fo->Total);
  strlcpy(srcbuf, GetFolderDir(oldfo), sizeof(srcbuf));
  strlcpy(dstbuf, GetFolderDir(fo), sizeof(dstbuf));

  for(i = 0, mail = fo->Messages; mail && success; mail = mail->Next, i++)
  {
    BusySet(i+1);
    GetMailFile(dstbuf, fo, mail);
    GetMailFile(srcbuf, oldfo, mail);

    if(MoveFile(srcbuf, dstbuf))
      RepackMailFile(mail, fo->Mode, fo->Password);
    else
      success = FALSE;
  }

  if(success)
  {
    // now we try to move an existing .index file
    strlcpy(srcbuf, GetFolderDir(oldfo), sizeof(srcbuf));
    AddPart(srcbuf, ".index", sizeof(srcbuf));
    strlcpy(dstbuf, GetFolderDir(fo), sizeof(dstbuf));
    AddPart(dstbuf, ".index", sizeof(dstbuf));

    if(FileExists(srcbuf) && !MoveFile(srcbuf, dstbuf))
    {
      success = FALSE;
    }
    else
    {
      // now we try to mvoe the .fimage file aswell
      strlcpy(srcbuf, GetFolderDir(oldfo), sizeof(srcbuf));
      AddPart(srcbuf, ".fimage", sizeof(srcbuf));
      strlcpy(dstbuf, GetFolderDir(fo), sizeof(dstbuf));
      AddPart(dstbuf, ".fimage", sizeof(dstbuf));

      if(FileExists(srcbuf) && !MoveFile(srcbuf, dstbuf))
      {
        success = FALSE;
      }
      else
      {
        // if we were able to successfully move all files
        // we can also delete the source directory. However,
        // we are NOT doing any error checking here as the
        // source may be a VOLUME and as such not deleteable
        DeleteMailDir(GetFolderDir(oldfo), FALSE);
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
  char passwd[SIZE_PASSWORD], passwd2[SIZE_PASSWORD];

  for(*passwd = 0;;)
  {
    *passwd = *passwd2 = 0;

    if(!StringRequest(passwd, SIZE_PASSWORD, tr(MSG_Folder), tr(MSG_CO_ChangeFolderPass), tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->FO->GUI.WI))
      return FALSE;

    if(*passwd && !StringRequest(passwd2, SIZE_PASSWORD, tr(MSG_Folder), tr(MSG_CO_RetypePass), tr(MSG_Okay), NULL, tr(MSG_Cancel), TRUE, G->FO->GUI.WI))
      return FALSE;

    if(!Stricmp(passwd, passwd2))
      break;
    else
      DisplayBeep(NULL);
  }

  if(!*passwd)
    return FALSE;

  strlcpy(fo->Password, passwd, sizeof(fo->Password));

  return TRUE;
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

  SetAttrs(gui->CY_FTYPE,  MUIA_Cycle_Active, type2cycle[folder->Type],
                           MUIA_Disabled,     isdefault,
                           TAG_DONE);

  SetAttrs(gui->CY_FMODE,  MUIA_Cycle_Active, folder->Mode,
                           MUIA_Disabled,     isdefault,
                           TAG_DONE);

  for(i = 0; i < 2; i++)
  {
    set(gui->CY_SORT[i], MUIA_Cycle_Active, (folder->Sort[i] < 0 ? -folder->Sort[i] : folder->Sort[i])-1);
    set(gui->CH_REVERSE[i], MUIA_Selected, folder->Sort[i] < 0);
  }

  set(gui->CH_STATS,       MUIA_Selected, folder->Stats);
  set(gui->BT_AUTODETECT,  MUIA_Disabled, !folder->MLSupport || isdefault);

  SetAttrs(gui->ST_HELLOTEXT,
           MUIA_String_Contents, folder->WriteIntro,
           TAG_DONE);

  SetAttrs(gui->ST_BYETEXT,
           MUIA_String_Contents, folder->WriteGreetings,
           TAG_DONE);

  // for ML-Support
  SetAttrs(gui->CH_MLSUPPORT,
           MUIA_Selected, isdefault ? FALSE : folder->MLSupport,
           MUIA_Disabled, isdefault,
           TAG_DONE);

  SetAttrs(gui->ST_MLADDRESS,
           MUIA_String_Contents, folder->MLAddress,
           MUIA_Disabled, !folder->MLSupport || isdefault,
           TAG_DONE);

  SetAttrs(gui->ST_MLPATTERN,
           MUIA_String_Contents, folder->MLPattern,
           MUIA_Disabled, !folder->MLSupport || isdefault,
           TAG_DONE);

  SetAttrs(gui->ST_MLFROMADDRESS,
           MUIA_String_Contents, folder->MLFromAddress,
           MUIA_Disabled, !folder->MLSupport || isdefault,
           TAG_DONE);

  SetAttrs(gui->ST_MLREPLYTOADDRESS,
           MUIA_String_Contents, folder->MLReplyToAddress,
           MUIA_Disabled, !folder->MLSupport || isdefault,
           TAG_DONE);

  SetAttrs(gui->CY_MLSIGNATURE,
           MUIA_Cycle_Active,    folder->MLSignature,
           MUIA_Disabled, !folder->MLSupport || isdefault,
           TAG_DONE);

  // we make sure the window is at the front if it
  // is already open
  if(xget(G->FO->GUI.WI, MUIA_Window_Open))
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

  // we have to correct the folder path because we shouldn`t allow a last / in the
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

  if(StringRequest(folder.Name, SIZE_NAME, tr(MSG_FO_NEWFGROUP), tr(MSG_FO_NEWFGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, G->MA->GUI.WI))
  {
    long tnflags = (TNF_LIST | TNF_OPEN);

    // Now we check if the foldergroup image was loaded and if not we enable the standard NListtree image
    if(IsImageInCache("folder_fold") &&
       IsImageInCache("folder_unfold"))
    {
      SET_FLAG(tnflags, TNF_NOSIGN);
    }

    DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder.Name, &folder, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags);

    FO_SaveTree(CreateFilename(".folders"));
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
   static struct Folder folder;

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
         struct Folder *currfolder = FO_GetCurrentFolder();
         if(!currfolder) return;

         // as the user decided to use the settings from the current folder, wie copy
         // the current one to our new one.
         memcpy(&folder, currfolder, sizeof(struct Folder));

         if(isGroupFolder(&folder))
         {
           FO_NewFolderGroupFunc();
           return;
         }
         else if(isIncomingFolder(&folder) || isTrashFolder(&folder))
           folder.Type = FT_CUSTOM;
         else if(isOutgoingFolder(&folder) || isSentFolder(&folder))
           folder.Type = FT_CUSTOMSENT;

         // now that we have the correct folder type, we set some default values for the new
         // folder
         *folder.Path       = 0;
         *folder.Name       = 0;
         folder.imageObject = NULL;
         folder.Messages    = NULL;
         folder.ImageIndex  = -1;  // No Image for the folder by default.
      }
      break;

      case 3:
      {
        struct FileReqCache *frc;

        if((frc = ReqFile(ASL_FOLDER, G->MA->GUI.WI, tr(MSG_FO_SelectDir), REQF_DRAWERSONLY, G->MA_MailDir, "")))
        {
          strlcpy(folder.Path, frc->drawer, sizeof(folder.Path));

          FO_LoadConfig(&folder);
        }
        else
          return;
      }
      break;

      default:
      {
        return;
      }
   }

   if (!G->FO)
   {
      if (!(G->FO = FO_New())) return;
      if (!SafeOpenWindow(G->FO->GUI.WI)) { DisposeModulePush(&G->FO); return; }
   }

   FO_GetFolder(&folder);
}
MakeHook(FO_NewFolderHook, FO_NewFolderFunc);

///
/// FO_EditFolderFunc
//  Opens folder window to edit the settings of the active folder
HOOKPROTONHNONP(FO_EditFolderFunc, void)
{
  struct Folder *folder = FO_GetCurrentFolder();

  ENTER();

  if(folder)
  {
    if(isGroupFolder(folder))
    {
      if(StringRequest(folder->Name, SIZE_NAME, tr(MSG_FO_EDIT_FGROUP), tr(MSG_FO_EDIT_FGROUPREQ), tr(MSG_Okay), NULL, tr(MSG_Cancel), FALSE, G->MA->GUI.WI))
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, MUIF_NONE);
    }
    else
    {
      if(!G->FO)
      {
        if(!(G->FO = FO_New()))
        {
          LEAVE();
          return;
        }

        if(!SafeOpenWindow(G->FO->GUI.WI))
        {
          DisposeModulePush(&G->FO);

          LEAVE();
          return;
        }
      }

      FO_GetFolder(G->FO->EditFolder = folder);
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
    APTR lv = G->MA->GUI.NL_FOLDERS;

    switch (folder->Type)
    {
      case FT_CUSTOM:
      case FT_CUSTOMSENT:
      case FT_CUSTOMMIXED:
      {
        if((delete_folder = MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_CO_ConfirmDelete))))
        {
          DeleteMailDir(GetFolderDir(folder), FALSE);
          ClearMailList(folder, TRUE);

          // Here we dispose the folderimage Object because the destructor
          // of the Folder Listtree can`t do this without throwing enforcer hits
          if(folder->imageObject)
          {
            // we make sure that the NList also doesn`t use the image in future anymore
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

            // and last, but not least we free the BC object here, so that this Object is also gone
            MUI_DisposeObject(folder->imageObject);
            folder->imageObject = NULL; // let`s set it to NULL so that the destructor doesn`t do the work again.
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
          if((delete_folder = MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_GROUP_CONFDEL))))
          {
            struct MUI_NListtree_TreeNode *tn_sub_next = tn_sub;

            set(lv, MUIA_NListtree_Quiet, TRUE);

            while(tn_sub_next)
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
        // nothing
      break;
    }

    if(delete_folder)
    {
      D(DBF_FOLDER, "deleting folder \"%s\"", folder->Name);

      // remove the entry from the listtree now
      DoMethod(lv, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, MUIV_NListtree_Remove_TreeNode_Active, MUIF_NONE);

      // Save the Tree to the folder config now
      FO_SaveTree(CreateFilename(".folders"));

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
  struct Folder folder, *oldfolder = G->FO->EditFolder;
  BOOL success = FALSE;

  ENTER();

  // if this is a edit folder request we separate here.
  if(oldfolder)
  {
    memcpy(&folder, oldfolder, sizeof(struct Folder));
    FO_PutFolder(&folder);

    // check if something has changed and if not we immediatly exit here
    if(memcmp(&folder, oldfolder, sizeof(struct Folder)) == 0)
    {
      DisposeModulePush(&G->FO);

      LEAVE();
      return;
    }

    // lets first check for a valid folder name
    // if the foldername is empty or it was changed and the new name already exists it`s invalid
    if(*folder.Name == '\0' || (stricmp(oldfolder->Name, folder.Name) != 0 && FO_GetFolderByName(folder.Name, NULL)))
    {
      MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERNAMEINVALID));

      LEAVE();
      return;
    }

    strlcpy(oldfolder->Name, folder.Name, sizeof(oldfolder->Name));

    // if the folderpath string has changed
    if(stricmp(oldfolder->Path, folder.Path) != 0)
    {
      char realpath_old[SIZE_PATH];
      char realpath_new[SIZE_PATH];

      // lets get the real pathes so that we can compare them later on
      strlcpy(realpath_old, GetRealPath(oldfolder->Path), sizeof(realpath_old));
      strlcpy(realpath_new, GetRealPath(folder.Path), sizeof(realpath_new));

      // then let`s check if the realPathes (after lock/unlock) is also different
      if(stricmp(realpath_old, realpath_new) != 0)
      {
        int result;

        // check if the new folder already exists or not.
        if(FileExists(folder.Path))
        {
          result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDEREXISTS));
        }
        else
        {
          result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDERMOVE));
        }

        // If the user really wants to proceed
        if(result)
        {
          if(Rename(oldfolder->Path, folder.Path) == FALSE)
          {
            if(!(CreateDirectory(GetFolderDir(&folder)) && FO_MoveFolderDir(&folder, oldfolder)))
            {
              ER_NewError(tr(MSG_ER_MOVEFOLDERDIR), folder.Name, folder.Path);

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
    }

    strlcpy(oldfolder->WriteIntro,       folder.WriteIntro, sizeof(oldfolder->WriteIntro));
    strlcpy(oldfolder->WriteGreetings,   folder.WriteGreetings, sizeof(oldfolder->WriteGreetings));
    strlcpy(oldfolder->MLFromAddress,    folder.MLFromAddress, sizeof(oldfolder->MLFromAddress));
    strlcpy(oldfolder->MLReplyToAddress, folder.MLReplyToAddress, sizeof(oldfolder->MLReplyToAddress));
    strlcpy(oldfolder->MLAddress,        folder.MLAddress, sizeof(oldfolder->MLAddress));
    strlcpy(oldfolder->MLPattern,        folder.MLPattern, sizeof(oldfolder->MLPattern));
    oldfolder->MLSignature = folder.MLSignature;
    oldfolder->Sort[0]   = folder.Sort[0];
    oldfolder->Sort[1]   = folder.Sort[1];
    oldfolder->Stats     = folder.Stats;
    oldfolder->MaxAge    = folder.MaxAge;
    oldfolder->MLSupport = folder.MLSupport;

    if(!xget(gui->CY_FTYPE, MUIA_Disabled))
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
        if(!(changed = MA_PromptFolderPassword(&folder, gui->WI)))
        {
          LEAVE();
          return;
        }
      }
      else if(isProtectedFolder(&folder) && !isProtectedFolder(oldfolder))
      {
        if(!(changed = FO_EnterPassword(&folder)))
        {
          LEAVE();
          return;
        }
      }

      if(isProtectedFolder(&folder) && isProtectedFolder(oldfolder))
         strlcpy(folder.Password, oldfolder->Password, sizeof(folder.Password));

      if(changed)
      {
        if(!isProtectedFolder(&folder))
          folder.Password[0] = '\0';

        if(folder.Mode != oldmode)
        {
          struct Mail *mail;
          int i;

          BusyGauge(tr(MSG_BusyUncompressingFO), "", folder.Total);
          for(i = 0, mail = folder.Messages; mail; mail = mail->Next, i++)
          {
            BusySet(i+1);
            RepackMailFile(mail, folder.Mode, folder.Password);
          }
          BusyEnd();

          oldfolder->Mode = newmode;
        }

        strlcpy(oldfolder->Password, folder.Password, sizeof(oldfolder->Password));
      }
      oldfolder->Type = folder.Type;
    }

    if(FO_SaveConfig(&folder))
      success = TRUE;
  }
  else // if not then a new folder should be generated
  {
    int result;

    memset(&folder, 0, sizeof(struct Folder));
    folder.ImageIndex = -1;

    FO_PutFolder(&folder);

    // lets first check for a valid folder name
    // if the foldername is empty or the new name already exists it`s invalid
    if(folder.Name[0] == '\0' || FO_GetFolderByName(folder.Name, NULL))
    {
      MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERNAMEINVALID));

      LEAVE();
      return;
    }

    // lets check if entered folder path is valid or not
    if(folder.Path[0] == '\0')
    {
      MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_OkayReq), tr(MSG_FO_FOLDERPATHINVALID));

      LEAVE();
      return;
    }
    else if(FileExists(folder.Path)) // check if something with folder.Path already exists
    {
      result = MUI_Request(G->App, G->FO->GUI.WI, 0, NULL, tr(MSG_YesNoReq), tr(MSG_FO_FOLDEREXISTS));
    }
    else
      result = TRUE;

    // only if the user want to proceed we go on.
    if(result)
    {
      if(isProtectedFolder(&folder) && FO_EnterPassword(&folder) == FALSE)
      {
        LEAVE();
        return;
      }

      if(CreateDirectory(GetFolderDir(&folder)))
      {
        if(FO_SaveConfig(&folder))
        {
          DoMethod(lv, MUIM_NListtree_Insert, folder.Name, &folder, MUIV_NListtree_Insert_ListNode_Active, MUIV_NListtree_Insert_PrevNode_Active, MUIV_NListtree_Insert_Flag_Active);
          oldfolder = &folder;
          success = TRUE;
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

  if(success)
  {
    MA_SetSortFlag();
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Redraw, MUIV_NList_Redraw_Title);
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_NList_Sort);
    MA_ChangeFolder(FO_GetFolderByName(folder.Name, NULL), FALSE);
    FO_SaveTree(CreateFilename(".folders"));
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
      FO_SaveTree(CreateFilename(".folders"));
    }
    break;

    case SO_RESET:
    {
      struct Folder **flist;

      // before we reset/reload the foldertree we have to
      // make sure everything is freed correctly.
      if((flist = FO_CreateList()) != NULL)
      {
        int i;

        for(i=1; i <= (int)*flist; i++)
        {
          struct Folder *folder = flist[i];

          if(folder == NULL)
            break;

          // we do not have to call FreeFolder manually, because the
          // destructor of the Listtree will do this for us. But we
          // have to free the FImage of the folder if it exists
          if(folder->imageObject)
          {
            // we make sure that the NList also doesn`t use the image in future anymore
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, folder->ImageIndex, MUIF_NONE);

            // and last, but not least we free the BC object here, so that this Object is also gone
            MUI_DisposeObject(folder->imageObject);
            folder->imageObject = NULL; // let`s set it to NULL so that the destructor doesn`t do the work again.
          }
        }

        free(flist);
      }

      FO_LoadTree(CreateFilename(".folders"));
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
  struct Mail *mail;
  int i;

  folder = G->FO->EditFolder;
  if(!folder) return;

  mail = folder->Messages;
  if(!mail) return;

  toPattern = mail->To.Address;
  toAddress = mail->To.Address;

  for(i=0, mail=mail->Next; mail && i < SCANMSGS; i++, mail = mail->Next)
  {
    char *result;

    D(DBF_FOLDER, "SWS: [%s] [%s]", toPattern, mail->To.Address);

    // Analyze the ToAdress through the Smith&Waterman algorithm
    if(takePattern && (result = SWSSearch(toPattern, mail->To.Address)))
    {
      if(res)
        free(res);

      res = strdup(result);
      if(!res)
        return;

      toPattern = res;

      // If we reached a #? pattern then we break here
      if(strcmp(toPattern, "#?") == 0)
        takePattern = FALSE;
    }

    // Lets check if the toAddress kept the same and then we can use
    // it for the TOADDRESS string gadget
    if(takeAddress && stricmp(toAddress, mail->To.Address) != 0)
      takeAddress = FALSE;
  }

  // lets make a pattern out of the found SWS string
  if(takePattern)
  {
    if(strlen(toPattern) >= 2 && !(toPattern[0] == '#' && toPattern[1] == '?'))
    {
      if(res)
        res = realloc(res, strlen(res)+3);
      else if((res = malloc(strlen(toPattern)+3)))
        strlcpy(res, toPattern, strlen(toPattern));

      if(!res)
        return;

      // move the actual string to the back and copy the wildcard in front of it.
      memmove(&res[2], res, strlen(res)+1);
      res[0] = '#';
      res[1] = '?';

      toPattern = res;
    }

    if(strlen(toPattern) >= 2 && !(toPattern[strlen(toPattern)-2] == '#' && toPattern[strlen(toPattern)-1] == '?'))
    {
      if(res)
        res = realloc(res, strlen(res)+3);
      else if((res = malloc(strlen(toPattern)+3)))
        strlcpy(res, toPattern, strlen(toPattern));

      if(!res) return;

      // and now copy also the wildcard at the back of the string
      strcat(res, "#?");

      toPattern = res;
    }
  }

  D(DBF_FOLDER, "ML-Pattern: [%s]", toPattern);

  // Now we set the new pattern & address values to the string gadgets
  notRecog = tr(MSG_FO_NOTRECOGNIZED);
  setstring(G->FO->GUI.ST_MLPATTERN, takePattern && toPattern[0] ? toPattern : notRecog);
  setstring(G->FO->GUI.ST_MLADDRESS, takeAddress ? toAddress : notRecog);

  // lets free all resources now
  if(res) free(res);
  SWSSearch(NULL, NULL);
}
MakeStaticHook(FO_MLAutoDetectHook, FO_MLAutoDetectFunc);

///

/// FO_New
//  Creates folder configuration window
static struct FO_ClassData *FO_New(void)
{
   struct FO_ClassData *data = calloc(1, sizeof(struct FO_ClassData));

   if (data)
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
               Child, Label2(tr(MSG_FO_ToPattern)),
               Child, data->GUI.ST_MLPATTERN = MakeString(SIZE_PATTERN,tr(MSG_FO_ToPattern)),
               Child, Label2(tr(MSG_FO_ToAddress)),
               Child, MakeAddressField(&data->GUI.ST_MLADDRESS, tr(MSG_FO_ToAddress), MSG_HELP_FO_ST_MLADDRESS, ABM_CONFIG, -1, TRUE),
               Child, Label2(tr(MSG_FO_FromAddress)),
               Child, MakeAddressField(&data->GUI.ST_MLFROMADDRESS, tr(MSG_FO_FromAddress), MSG_HELP_FO_ST_MLFROMADDRESS, ABM_CONFIG, -1, TRUE),
               Child, Label2(tr(MSG_FO_ReplyToAddress)),
               Child, MakeAddressField(&data->GUI.ST_MLREPLYTOADDRESS, tr(MSG_FO_ReplyToAddress), MSG_HELP_FO_ST_MLREPLYTOADDRESS, ABM_CONFIG, -1, TRUE),
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

      if (data->GUI.WI)
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

         return data;
      }
      free(data);
   }
   return NULL;
}
///


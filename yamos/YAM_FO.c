/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <clib/alib_protos.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <proto/xpkmaster.h>

#include "YAM.h"
#include "YAM_classes.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"

/* local protos */
static void FO_XPKUpdateFolder(struct Folder*, int);
static BOOL FO_Move(char*, char*);
static BOOL FO_MoveFolderDir(struct Folder*, struct Folder*);
static BOOL FO_EnterPassword(struct Folder*);
static BOOL FO_FoldernameRequest(char*);
static struct FO_ClassData *FO_New(void);
static BOOL FO_GetFolderByType_cmp(struct Folder*, int*);
static BOOL FO_GetFolderByName_cmp(struct Folder*, char*);
static struct Folder *FO_GetFolderByAttribute(BOOL(*)(struct Folder*,void*), void*, int*);
static BOOL FO_SaveSubTree(FILE *, struct MUI_NListtree_TreeNode *);

/***************************************************************************
 Module: Folder Configuration
***************************************************************************/

/// FO_CreateList
//  Creates a linked list of all folders
struct Folder **FO_CreateList(void)
{
   int max, i;
   struct Folder **flist;
   APTR lv = G->MA->GUI.NL_FOLDERS;

   get(lv, MUIA_NList_Entries, &max);
   if (flist = calloc(max+1, sizeof(struct Folder *)))
   {
      flist[0] = (struct Folder *)max;
      for (i = 0; i < max; i++)
      {
        struct MUI_NListtree_TreeNode *tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, 0, TAG_DONE);

        if(!tn)
        {
          free(flist);
          return NULL;
        }

        flist[i+1] = tn->tn_User;
      }
   }
   return flist;
}

///
/// FO_GetCurrentFolder
//  Returns pointer to active folder
struct Folder *FO_GetCurrentFolder(void)
{
   struct MUI_NListtree_TreeNode *tn;

   tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, MUIV_NListtree_GetEntry_Position_Active, 0, TAG_DONE);

   if(!tn) return NULL;

   return((struct Folder *)tn->tn_User);
}

///
/// FO_GetFolderRexx
//  Finds a folder by its name, type or position
struct Folder *FO_GetFolderRexx(char *arg, int *pos)
{
   int i, nr = 0;
   struct Folder *fo = NULL, **flist;
   char *p = arg;
   BOOL numeric = TRUE;

   if (flist = FO_CreateList())
   {
      while (*p) if (!isdigit((int)*p++)) numeric = FALSE;
      if (numeric) if ((i = atoi(arg)) >= 0 && i < (int)*flist) if (flist[++i]->Type != FT_GROUP) nr = i;
      if (!nr) for (i = 1; i <= (int)*flist; i++)
         if ((!Stricmp(arg, flist[i]->Name) && flist[i]->Type != FT_GROUP) ||
             (!stricmp(arg, "incoming")     && flist[i]->Type == FT_INCOMING) ||
             (!stricmp(arg, "outgoing")     && flist[i]->Type == FT_OUTGOING) ||
             (!stricmp(arg, "sent")         && flist[i]->Type == FT_SENT) ||
             (!stricmp(arg, "deleted")      && flist[i]->Type == FT_DELETED)) { nr = i; break; }
      if (nr)
      {
         fo = flist[nr];
         if (pos) *pos = --nr;
      }
      free(flist);
   }
   return fo;
}

///
/// FO_GetFolderByAttribute
//  Generalized find-folder function
static struct Folder *FO_GetFolderByAttribute(BOOL (*cmpf)(struct Folder*,void*), void *attr, int *pos)
{
   int i;
   struct Folder *fo = NULL;
   struct MUI_NListtree_TreeNode *tn;

   for(i = 0; ; i++)
   {
      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, 0, TAG_DONE);
      if (!tn) break;

      fo = tn->tn_User;
      if (!fo) break;

      if (cmpf(fo,attr)) break;
      fo = NULL;
   }
   if (pos) *pos = i;
   return fo;
}

///
/// FO_GetFolderByType
//  Finds a folder by its type
struct Folder *FO_GetFolderByType(enum FolderType type, int *pos)
{
   return FO_GetFolderByAttribute((BOOL (*)(struct Folder*,void*))&FO_GetFolderByType_cmp,&type,pos);
}
// comparison function for FO_GetFolderByType
static BOOL FO_GetFolderByType_cmp(struct Folder *f, enum FolderType *type)
{
   return (BOOL)(f->Type == *type);
}

///
/// FO_GetFolderByName
//  Finds a folder by its name
struct Folder *FO_GetFolderByName(char *name, int *pos)
{
   return FO_GetFolderByAttribute((BOOL (*)(struct Folder*,void*))&FO_GetFolderByName_cmp,name,pos);
}
// comparison function for FO_GetFolderByName
static BOOL FO_GetFolderByName_cmp(struct Folder *f, char *name)
{
   return (BOOL)(!strcmp(f->Name, name) && (f->Type != FT_GROUP));
}

///
/// FO_GetFolderPosition
//  Gets the position of a folder in the list
int FO_GetFolderPosition(struct Folder *findfo)
{
   int i;
   struct Folder *fo;
   struct MUI_NListtree_TreeNode *tn;

   for (i = 0;; i++)
   {
      tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, 0 , TAG_DONE);
      if (!tn) return -1;
 
      fo = tn->tn_User;
      if (!fo) return -1;

      if (fo == findfo) return i;
   }
}

///
/// FO_LoadConfig
//  Loads folder configuration from .fconfig file
BOOL FO_LoadConfig(struct Folder *fo)
{
   BOOL success = FALSE;
   FILE *fh;
   char buffer[SIZE_LARGE], fname[SIZE_PATHFILE];

   MyStrCpy(fname, GetFolderDir(fo)); AddPart(fname, ".fconfig", sizeof(fname));
   if (fh = fopen(fname, "r"))
   {
      fgets(buffer, SIZE_LARGE, fh);
      if (!strnicmp(buffer, "YFC", 3))
      {
         /* pick a default value */
         fo->MLSignature = 1;

         while (fgets(buffer, SIZE_LARGE, fh))
         {
            char *p, *value;
            if (value = strchr(buffer, '=')) for (++value; ISpace(*value); value++);
            if (p = strpbrk(buffer,"\r\n")) *p = 0;
            for (p = buffer; *p && !ISpace(*p); p++); *p = 0;
            if (*buffer && value)
            {
               if (!stricmp(buffer, "Name"))        MyStrCpy(fo->Name, value);
               if (!stricmp(buffer, "MaxAge"))      fo->MaxAge = atoi(value);
               if (!stricmp(buffer, "Password"))    MyStrCpy(fo->Password, Decrypt(value));
               if (!stricmp(buffer, "Type"))        fo->Type = atoi(value);
               if (!stricmp(buffer, "XPKType"))     fo->XPKType = atoi(value);
               if (!stricmp(buffer, "Sort1"))       fo->Sort[0] = atoi(value);
               if (!stricmp(buffer, "Sort2"))       fo->Sort[1] = atoi(value);
               if (!stricmp(buffer, "MLFromAddr"))  MyStrCpy(fo->MLFromAddress,value);
               if (!stricmp(buffer, "MLRepToAddr")) MyStrCpy(fo->MLReplyToAddress, value);
               if (!stricmp(buffer, "MLAddress"))   MyStrCpy(fo->MLAddress, value);
               if (!stricmp(buffer, "MLPattern"))   MyStrCpy(fo->MLPattern, value);
               if (!stricmp(buffer, "MLSignature")) fo->MLSignature = atoi(value);
            }
         }
         success = TRUE;
      }
      fclose(fh);
   }
   return success;
}

///
/// FO_SaveConfig
//  Saves folder configuration to .fconfig file
void FO_SaveConfig(struct Folder *fo)
{
   struct DateStamp ds;
   char fname[SIZE_PATHFILE];
   FILE *fh;

   MyStrCpy(fname, GetFolderDir(fo)); AddPart(fname, ".fconfig", sizeof(fname));
   if (fh = fopen(fname, "w"))
   {
      fprintf(fh, "YFC1 - YAM Folder Configuration\n");
      fprintf(fh, "Name        = %s\n",  fo->Name);
      fprintf(fh, "MaxAge      = %ld\n", fo->MaxAge);
      fprintf(fh, "Password    = %s\n",  Encrypt(fo->Password));
      fprintf(fh, "Type        = %ld\n", fo->Type);
      fprintf(fh, "XPKType     = %ld\n", fo->XPKType);
      fprintf(fh, "Sort1       = %ld\n", fo->Sort[0]);
      fprintf(fh, "Sort2       = %ld\n", fo->Sort[1]);
      fprintf(fh, "MLFromAddr  = %s\n",  fo->MLFromAddress);
      fprintf(fh, "MLRepToAddr = %s\n",  fo->MLReplyToAddress);
      fprintf(fh, "MLPattern   = %s\n",  fo->MLPattern);
      fprintf(fh, "MLAddress   = %s\n",  fo->MLAddress);
      fprintf(fh, "MLSignature = %ld\n", fo->MLSignature);
      fclose(fh);
      MyStrCpy(fname, GetFolderDir(fo)); AddPart(fname, ".index", sizeof(fname));
      if (!(fo->Flags&FOFL_MODIFY)) SetFileDate(fname, DateStamp(&ds));
   }
   else ER_NewError(GetStr(MSG_ER_CantCreateFile), fname, NULL);
}

///
/// FO_NewFolder
//  Initializes a new folder and creates its directory
struct Folder *FO_NewFolder(enum FolderType type, char *path, char *name)
{
   struct Folder *folder = calloc(1, sizeof(struct Folder));
   folder->Sort[0] = 1;
   folder->Sort[1] = 3;
   folder->Type = type;
   MyStrCpy(folder->Path, path);
   MyStrCpy(folder->Name, name);
   if (CreateDirectory(GetFolderDir(folder))) return folder;
   free(folder);
   return NULL;
}

///
/// FO_FreeFolder
//  frees all resources previously allocated on creation time of the folder
BOOL FO_FreeFolder(struct Folder *folder)
{
   if(!folder) return FALSE;

   // remove the image of this folder from the objectlist at the application
   if(!folder->FImage) MUI_DisposeObject(folder->FImage);

   // if we still have mails in the folder we have to clear the list
   if(folder->Messages) ClearMailList(folder, TRUE);

   // now it`s time to deallocate the folder itself
   free(folder);

   return TRUE;
}

///
/// FO_CreateFolder
//  Adds a new entry to the folder list
BOOL FO_CreateFolder(enum FolderType type, char *path, char *name)
{
   struct Folder *folder = FO_NewFolder(type, path, name);

   if (folder)
   {
      if(!(DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder->Name, folder, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, 0, TAG_DONE)))
      {
        free(folder);
        return FALSE;
      }
      FO_SaveConfig(folder);
      free(folder);
      return TRUE;
   }
   return FALSE;
}

///
/// FO_LoadTree
//  Loads folder list from a file
BOOL FO_LoadTree(char *fname)
{
   static struct Folder fo;
   BOOL success = FALSE;
   char buffer[SIZE_LARGE];
   int nested = 0, i = 0;
   FILE *fh;
   APTR lv = G->MA->GUI.NL_FOLDERS;
   struct MUI_NListtree_TreeNode *tn_root = MUIV_NListtree_Insert_ListNode_Root;
   
   if (fh = fopen(fname, "r"))
   {
      GetLine(fh, buffer, sizeof(buffer));
      if (!strncmp(buffer, "YFO", 3))
      {
         DoMethod(lv, MUIM_NListtree_Clear, NULL, 0, TAG_DONE);
         set(lv, MUIA_NListtree_Quiet, TRUE);
         while (GetLine(fh, buffer, sizeof(buffer)))
         {
            memset(&fo, 0, sizeof(struct Folder));
            if (!strncmp(buffer, "@FOLDER", 7))
            {
               fo.Type = FT_CUSTOM;
               fo.Sort[0] = 1; fo.Sort[1] = 3;
               MyStrCpy(fo.Name, Trim(&buffer[8]));
               MyStrCpy(fo.Path, Trim(GetLine(fh, buffer, sizeof(buffer))));

               if (CreateDirectory(GetFolderDir(&fo)))
               {
                  if (!FO_LoadConfig(&fo))
                  {
                     if (!stricmp(FilePart(fo.Path), FolderNames[0])) fo.Type = FT_INCOMING;
                     if (!stricmp(FilePart(fo.Path), FolderNames[1])) fo.Type = FT_OUTGOING;
                     if (!stricmp(FilePart(fo.Path), FolderNames[2])) fo.Type = FT_SENT;
                     if (!stricmp(FilePart(fo.Path), FolderNames[3])) fo.Type = FT_DELETED;

                     // Save the config now because it could be changed in the meantime
                     FO_SaveConfig(&fo);
                  }
                  fo.SortIndex = i++;

                  // Now we add this folder to the folder listtree
                  if(!(DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, 0, TAG_DONE)))
                  {
                     fclose(fh);
                     return FALSE;
                  }
               }
               do if (!strcmp(buffer, "@ENDFOLDER")) break;
               while (GetLine(fh, buffer, sizeof(buffer)));
            }
            else if (!strncmp(buffer, "@SEPARATOR", 10))
            {
               // SEPARATOR support is obsolete since the folder hierachical order
               // that`s why we handle SEPARATORs as GROUPs now for backward compatibility
               fo.Type = FT_GROUP;
               MyStrCpy(fo.Name, Trim(&buffer[11]));
               do if (!strcmp(buffer, "@ENDSEPARATOR")) break;
               while (GetLine(fh, buffer, sizeof(buffer)));
               fo.SortIndex = i++;
               if(!(DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, TNF_LIST, TAG_DONE)))
               {
                  fclose(fh);
                  return FALSE;
               }
            }
            else if (!strncmp(buffer, "@GROUP", 6))
            {
               long tnflags = (TNF_LIST);

               fo.Type = FT_GROUP;
               MyStrCpy(fo.Name, Trim(&buffer[7]));

               // now we check if the node should be open or not
               if(GetLine(fh, buffer, sizeof(buffer)))
               {
                  // if it is greater zero then the node should be displayed open
                  if(atoi(buffer) > 0) tnflags = (TNF_LIST | TNF_OPEN);
               }

               // now we are going to add this treenode to the list
               if(!(tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_Insert, fo.Name, &fo, tn_root, MUIV_NListtree_Insert_PrevNode_Tail, tnflags, TAG_DONE)))
               {
                  fclose(fh);
                  return FALSE;
               }

               nested++;
            }
            else if (!strcmp(buffer,"@ENDGROUP"))
            {
               nested--;

               // now we check if the nested is zero and if yes then we set tn_root = MUIV_NListtree_Insert_ListNode_Root
               // otherwise we go back to the root of the root
               if (nested == 0) tn_root = MUIV_NListtree_Insert_ListNode_Root;
               else tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Parent, 0, TAG_DONE);
            }
         }
         set(lv, MUIA_NListtree_Active, MUIV_NListtree_Active_FirstVisible);
         set(lv, MUIA_NListtree_Quiet, FALSE);
      }
      fclose(fh);
      success = TRUE;
   }

   // if nested is still greater zero we have a misconfiguration
   if (nested > 0) success = FALSE;

   return success;
}

///
/// FO_LoadTreeImage
//  Loads the image for the folder that should be displayed in the listtree
BOOL FO_LoadTreeImage(struct Folder *fo)
{
   char fname[SIZE_PATHFILE];
   APTR lv = G->MA->GUI.NL_FOLDERS;

   if(!fo) return FALSE;

   MyStrCpy(fname, GetFolderDir(fo)); AddPart(fname, ".fimage", sizeof(fname));

   fo->FImage = NewObject(CL_BodyChunk->mcc_Class, NULL,
      MUIA_Bodychunk_File,     fname,
      MUIA_Bodychunk_UseOld,   FALSE,
      MUIA_Bitmap_Transparent, 0,
   End;

   if(!fo->FImage) return FALSE;

#ifdef DEBUG
   kprintf("Loaded TreeImage: %s - %lx - %ld\n", fo->Name, fo->FImage, fo->SortIndex+1);
#endif

   // Now we say that this image could be used by this Listtree
   DoMethod(lv, MUIM_NList_UseImage, fo->FImage, fo->SortIndex+1, 0);

   return TRUE;
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

  // The root-Treenode is the subtree at the start
  tn_root = subtree;

  for (i = 0;; i++)
  {
    if(tn_root == MUIV_NListtree_GetEntry_ListNode_Root)
    {
      tn = tn_root = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, 0, 0, TAG_DONE);
      tn_parent = subtree = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, 0, TAG_DONE);
      noendgroup = TRUE;
    }
    else
    {
      // get the next treenode
      if(i == 0) tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, i, 0, TAG_DONE);
      else tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn_root, MUIV_NListtree_GetEntry_Position_Next, 0, TAG_DONE);

      // get the parent node of the treenode
      tn_parent = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, tn, MUIV_NListtree_GetEntry_Position_Parent, 0, TAG_DONE);
    }

    // if tn is null or the parent of the next is not the same like the caller of this function
    // we are going to print ENDGROUP and return
    if (!tn || tn_parent != subtree)
    {
      // if we reach here it`s just the end of a GROUP
      if(!noendgroup) fputs("@ENDGROUP\n", fh);

      break;
    }
    else
    {
      fo = tn->tn_User;
      if (!fo) break;
      fo->SortIndex = i;

      switch (fo->Type)
      {
        case FT_GROUP:
        {
          fprintf(fh, "@GROUP %s\n%ld\n", fo->Name, tn->tn_Flags & TNF_OPEN ? 1 : 0);

          // Now we recursively save this subtree first
          success = FO_SaveSubTree(fh, tn);
        }
        break;

        default:
        {
           fprintf(fh, "@FOLDER %s\n%s\n@ENDFOLDER\n", fo->Name, fo->Path);
        }
        break;
      }

      tn_root = tn;
    }
  }

  return success;
}

///
/// FO_SaveTree
//  Saves folder list to a file
BOOL FO_SaveTree(char *fname)
{
   BOOL success = TRUE;
   FILE *fh;

#ifdef DEBUG
   kprintf("SaveTree!!!\n");
#endif

   if (fh = fopen(fname, "w"))
   {
      fputs("YFO1 - YAM Folders\n", fh);

      if(!FO_SaveSubTree(fh, MUIV_NListtree_GetEntry_ListNode_Root)) success = FALSE;

      fclose(fh);
   }
   else ER_NewError(GetStr(MSG_ER_CantCreateFile), fname, NULL);

   return success;
}

///
/// FO_XPKUpdateFolder
//  Updates compression mode for a folder
static void FO_XPKUpdateFolder(struct Folder *fo, int oldtype)
{
   if (fo->XPKType != oldtype)
   {
      struct Mail *mail;
      int i;
      Busy(GetStr(MSG_BusyUncompressingFO), "", 0, fo->Total);
      for (i = 0, mail = fo->Messages; mail; mail = mail->Next, i++)
      {
         Busy(NULL, NULL, i, 0);
         RepackMailFile(mail, fo->XPKType, fo->Password);
      }
      BusyEnd;
   }
}

///
/// FO_Move
//  Moves a folder directory to a new destination
static BOOL FO_Move(char *srcbuf, char *dstbuf)
{
   if (!RenameFile(srcbuf, dstbuf)) if (!CopyFile(dstbuf, 0, srcbuf, 0)) return FALSE;
   return TRUE;
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
   Busy(GetStr(MSG_BusyMoving), itoa(fo->Total), 0, fo->Total);
   strcpy(srcbuf, GetFolderDir(oldfo));
   strcpy(dstbuf, GetFolderDir(fo));
   for (i = 0, mail = fo->Messages; mail && success; mail = mail->Next, i++)
   {
      Busy(NULL, NULL, i, 0);
      GetMailFile(dstbuf, fo, mail);
      GetMailFile(srcbuf, oldfo, mail);
      if (!FO_Move(srcbuf, dstbuf)) success = FALSE;
      else RepackMailFile(mail, fo->XPKType, fo->Password);
   }
   if (success)
   {
      MyStrCpy(srcbuf, GetFolderDir(oldfo)); AddPart(srcbuf, ".index", sizeof(srcbuf));
      MyStrCpy(dstbuf, GetFolderDir(fo)); AddPart(dstbuf, ".index", sizeof(dstbuf));
      FO_Move(srcbuf, dstbuf);
      DeleteMailDir(GetFolderDir(oldfo), FALSE);
   }
   BusyEnd;
   return success;
}

///
/// FO_EnterPassword
//  Sets password for a protected folder
static BOOL FO_EnterPassword(struct Folder *fo)
{
   char passwd[SIZE_PASSWORD], passwd2[SIZE_PASSWORD];

   for (*passwd = 0;;)
   {
      *passwd = *passwd2 = 0;
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_Folder), GetStr(MSG_CO_ChangeFolderPass), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->FO->GUI.WI)) return FALSE;
      if (*passwd) if (!StringRequest(passwd2, SIZE_PASSWORD, GetStr(MSG_Folder), GetStr(MSG_CO_RetypePass), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, G->FO->GUI.WI)) return FALSE;
      if (!Stricmp(passwd, passwd2)) break; else DisplayBeep(NULL);
   }
   if (!*passwd) return FALSE;
   strcpy(fo->Password, passwd);
   return TRUE;
}

///
/// FO_FoldernameRequest
//  Asks user for a folder name and path
static BOOL FO_FoldernameRequest(char *string)
{
   char *path;
   APTR bt_okay, bt_cancel, wi, st_pa, st_di;
   int ret_code = -1;

   wi = WindowObject,
      MUIA_Window_Title, GetStr(MSG_Folder),
      MUIA_Window_RefWindow, G->MA->GUI.WI,
      MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
      MUIA_Window_TopEdge, MUIV_Window_TopEdge_Centered,
      MUIA_Window_ID, MAKE_ID('N','F','R','Q'),
      WindowContents, VGroup,
         Child, VGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, LLabel(GetStr(MSG_CO_SelectDir)),
            Child, ColGroup(2),
               Child, Label2(GetStr(MSG_Path)),
               Child, PopaslObject,
                  MUIA_Popasl_Type, ASL_FileRequest,
                  MUIA_Popstring_String, st_pa = MakeString(SIZE_PATH, ""),
                  MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
                  ASLFR_DrawersOnly, TRUE,
               End,
               Child, Label2(GetStr(MSG_Directory)),
               Child, st_di = MakeString(SIZE_FILE, ""),
            End,
         End,
         Child, ColGroup(2),
            Child, bt_okay = MakeButton(GetStr(MSG_Okay)),
            Child, bt_cancel = MakeButton(GetStr(MSG_Cancel)),
         End,
      End,
   End;
   if (wi)
   {
      setstring(st_pa, G->MA_MailDir);
      setstring(st_di, string);
      set(st_di, MUIA_String_Reject, " \";:/#?(|)");
      set(wi, MUIA_Window_ActiveObject, st_di);
      set(G->App, MUIA_Application_Sleep, TRUE);
      DoMethod(G->App, OM_ADDMEMBER, wi);
      DoMethod(bt_okay  , MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 1);
      DoMethod(bt_cancel, MUIM_Notify, MUIA_Pressed, FALSE, G->App, 2, MUIM_Application_ReturnID, 3);
      DoMethod(st_di, MUIM_Notify, MUIA_String_Acknowledge, MUIV_EveryTime, G->App, 2, MUIM_Application_ReturnID, 1);
      DoMethod(wi, MUIM_Notify, MUIA_Window_CloseRequest, TRUE, G->App, 2, MUIM_Application_ReturnID, 3);
      if (!SafeOpenWindow(wi)) ret_code = 0;
      while (ret_code == -1)
      {
         ULONG signals;
         switch (DoMethod(G->App, MUIM_Application_Input, &signals))
         {
            case 1: if (*(GetMUIStringPtr(st_di))) ret_code = 1;
                    break;
            case 3: ret_code = 0; break;
         }
         if (ret_code == -1 && signals) Wait(signals);
      }
      path = GetMUIStringPtr(st_pa);
      if (ret_code > 0)
         if (!stricmp(path, G->MA_MailDir)) GetMUIString(string, st_di);
         else
         {
           MyStrCpy(string, path); AddPart(string, GetMUIStringPtr(st_di), sizeof(string));
         }
      DoMethod(G->App, OM_REMMEMBER, wi);
      set(G->App, MUIA_Application_Sleep, FALSE);
   }
   return (BOOL)ret_code;
}

///
/// FO_GetFolder
//  Fills form with data from folder structure
static void FO_GetFolder(struct Folder *folder, BOOL existing)
{
   struct FO_GUIData *gui = &G->FO->GUI;
   BOOL isdefault = (folder->Type != FT_CUSTOM && folder->Type != FT_CUSTOMSENT && folder->Type != FT_CUSTOMMIXED);
   static int type2cycle[9] = { 0,0,1,1,2,-1,1,2,-1 };
   int i;

   set(gui->ST_FNAME, MUIA_String_Contents, folder->Name);
   set(gui->TX_FPATH, MUIA_Text_Contents, folder->Path);
   set(gui->ST_MAXAGE, MUIA_String_Integer, folder->MaxAge);
   set(gui->CY_FTYPE, MUIA_Cycle_Active, type2cycle[folder->Type]);
   set(gui->CY_FMODE, MUIA_Cycle_Active, folder->XPKType);
   for (i = 0; i < 2; i++)
   {
      set(gui->CY_SORT[i], MUIA_Cycle_Active, (folder->Sort[i] < 0 ? -folder->Sort[i] : folder->Sort[i])-1);
      set(gui->CH_REVERSE[i], MUIA_Selected, folder->Sort[i] < 0);
   }
   set(gui->ST_MLADDRESS, MUIA_String_Contents, folder->MLAddress);
   set(gui->ST_MLPATTERN, MUIA_String_Contents, folder->MLPattern);
   set(gui->ST_MLFROMADDRESS, MUIA_String_Contents, folder->MLFromAddress);
   set(gui->ST_MLREPLYTOADDRESS, MUIA_String_Contents, folder->MLReplyToAddress);
   set(gui->CY_MLSIGNATURE, MUIA_Cycle_Active, folder->MLSignature);
   set(gui->CY_FTYPE, MUIA_Disabled, isdefault);
   set(gui->CY_FMODE, MUIA_Disabled, isdefault || existing);
   set(gui->BT_MOVE, MUIA_Disabled, existing);
}

///
/// FO_PutFolder
//  Updates folder structure with form data
static void FO_PutFolder(struct Folder *folder)
{
   struct FO_GUIData *gui = &G->FO->GUI;
   BOOL isdefault = (folder->Type != FT_CUSTOM && folder->Type != FT_CUSTOMSENT && folder->Type != FT_CUSTOMMIXED);
   static int cycle2type[3] = { FT_CUSTOM,FT_CUSTOMSENT,FT_CUSTOMMIXED };
   int i;

   GetMUIString(folder->Name, gui->ST_FNAME);
   GetMUIText(folder->Path, gui->TX_FPATH);
   folder->MaxAge = GetMUIInteger(gui->ST_MAXAGE);
   if (!isdefault)
   {
      folder->Type = cycle2type[GetMUICycle(gui->CY_FTYPE)];
      folder->XPKType = GetMUICycle(gui->CY_FMODE);
   }
   for (i = 0; i < 2; i++)
   {
      folder->Sort[i] = GetMUICycle(gui->CY_SORT[i])+1;
      if (GetMUICheck(gui->CH_REVERSE[i])) folder->Sort[i] = -folder->Sort[i];
   }
   GetMUIString(folder->MLPattern,        gui->ST_MLPATTERN);
   GetMUIString(folder->MLAddress,        gui->ST_MLADDRESS);
   GetMUIString(folder->MLFromAddress,    gui->ST_MLFROMADDRESS);
   GetMUIString(folder->MLReplyToAddress, gui->ST_MLREPLYTOADDRESS);
   folder->MLSignature = GetMUICycle(gui->CY_MLSIGNATURE);
}

///
/// FO_NewFolderGroupFunc
//  Creates a new separator
HOOKPROTONHNONP(FO_NewFolderGroupFunc, void)
{
   struct Folder folder;
   memset(&folder, 0, sizeof(struct Folder));
   folder.Type = FT_GROUP;
   if (StringRequest(folder.Name, SIZE_NAME, GetStr(MSG_MA_NewSeparator), GetStr(MSG_FO_NewSepReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), FALSE, G->MA->GUI.WI))
   {
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Insert, folder.Name, &folder, MUIV_NListtree_Insert_ListNode_Root, MUIV_NListtree_Insert_PrevNode_Tail, (TNF_LIST | TNF_OPEN), TAG_DONE);
   }
}
MakeHook(FO_NewFolderGroupHook, FO_NewFolderGroupFunc);

///
/// FO_NewFolderFunc
//  Creates a new folder
HOOKPROTONHNONP(FO_NewFolderFunc, void)
{
   int mode = MUI_Request(G->App, G->MA->GUI.WI, 0, GetStr(MSG_MA_NewFolder), GetStr(MSG_FO_NewFolderGads), GetStr(MSG_FO_NewFolderReq));
   static struct Folder folder;

   memset(&folder, 0, sizeof(struct Folder));
   folder.Sort[0] = 1; folder.Sort[1] = 3; folder.Type = FT_CUSTOM;
   switch (mode)
   {
      case 0: return;
      case 1: if (!FO_FoldernameRequest(folder.Path)) return;
              MyStrCpy(folder.Name, FilePart(folder.Path));
              break;
      case 2: memcpy(&folder, FO_GetCurrentFolder(), sizeof(struct Folder));
              if (folder.Type == FT_GROUP) { FO_NewFolderGroupFunc(); return; }
              else if (folder.Type == FT_INCOMING || folder.Type == FT_DELETED) folder.Type = FT_CUSTOM;
              else if (folder.Type == FT_OUTGOING || folder.Type == FT_SENT) folder.Type = FT_CUSTOMSENT;
              *folder.Path = 0;
              if (!FO_FoldernameRequest(folder.Path)) return;
              MyStrCpy(folder.Name, FilePart(folder.Path));
              break;
      case 3: if (!ReqFile(ASL_FOLDER, G->MA->GUI.WI, GetStr(MSG_FO_SelectDir), 4, G->MA_MailDir, "")) return;
              MyStrCpy(folder.Path, G->ASLReq[ASL_FOLDER]->fr_Drawer);
              FO_LoadConfig(&folder);
              break;
   }
   if (!G->FO)
   {
      if (!(G->FO = FO_New())) return;
      if (!SafeOpenWindow(G->FO->GUI.WI)) { DisposeModulePush(&G->FO); return; }
   }
   FO_GetFolder(&folder, mode==3);
}
MakeHook(FO_NewFolderHook, FO_NewFolderFunc);

///
/// FO_EditFolderFunc
//  Opens folder window to edit the settings of the active folder
HOOKPROTONHNONP(FO_EditFolderFunc, void)
{
  struct Folder *folder = FO_GetCurrentFolder();

  switch(folder->Type)
  {
    case FT_GROUP:
    {
      if (StringRequest(folder->Name, SIZE_NAME, GetStr(MSG_FO_EditFolder), GetStr(MSG_FO_NewSepReq), GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), FALSE, G->MA->GUI.WI))
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_Active, 0, TAG_DONE);
    }
    break;

    default:
    {
      if (!G->FO)
      {
        if (!(G->FO = FO_New())) return;
        if (!SafeOpenWindow(G->FO->GUI.WI)) { DisposeModulePush(&G->FO); return; }
      }
      FO_GetFolder(G->FO->EditFolder = folder, FALSE);
    }
  }
}
MakeHook(FO_EditFolderHook, FO_EditFolderFunc);

///
/// FO_DeleteFolderFunc
//  Removes the active folder
HOOKPROTONHNONP(FO_DeleteFolderFunc, void)
{
   APTR lv = G->MA->GUI.NL_FOLDERS;
   struct Folder *f, *folder = FO_GetCurrentFolder();
   struct MUI_NListtree_TreeNode *tn;
   int i, pos, used = 0;
   get(lv, MUIA_NListtree_Active, &pos);

   for (i = 0; ; i++)
   {
      tn = (struct MUI_NListtree_TreeNode *)DoMethod(lv, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIV_NListtree_GetEntry_Flag_Visible, TAG_DONE);
      if (!tn) break;

      f = tn->tn_User;
      if (!f) break; else if (!stricmp(f->Path, folder->Path)) used++;
   }
   switch (folder->Type)
   {
      case FT_CUSTOM:
      case FT_CUSTOMSENT:
      case FT_CUSTOMMIXED:
        if(used < 2)
        {
           char * a = GetStr(MSG_CO_ConfirmDelete);

           if(!MUI_Request(G->App, G->MA->GUI.WI, 0, NULL, GetStr(MSG_YesNoReq), a))
             return;
           DeleteMailDir(GetFolderDir(folder), FALSE);
        }
        ClearMailList(folder, TRUE);
      case FT_GROUP:
        DoMethod(lv, MUIM_NList_Remove, pos);
        FO_SaveTree(CreateFilename(".folders"));
        break;
      default:
        break;
   }
}
MakeHook(FO_DeleteFolderHook, FO_DeleteFolderFunc);

///
/// FO_MoveFunc
//  Asks user for the new destination
HOOKPROTONHNONP(FO_MoveFunc, void)
{
   char path[SIZE_PATH];
   GetMUIText(path, G->FO->GUI.TX_FPATH);
   if (FO_FoldernameRequest(path)) set(G->FO->GUI.TX_FPATH, MUIA_Text_Contents, path);
}
MakeStaticHook(FO_MoveHook, FO_MoveFunc);

///
/// FO_CloseFunc
//  Closes folder configuration window
HOOKPROTONHNONP(FO_CloseFunc, void)
{
   DisposeModulePush(&G->FO);
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

   if (oldfolder)
   {
      memcpy(&folder, oldfolder, sizeof(struct Folder));
      FO_PutFolder(&folder);
      if (stricmp(oldfolder->Path, folder.Path))
      {
         if (Rename(oldfolder->Path, folder.Path)) strcpy(oldfolder->Path, folder.Path);
         else
            if (CreateDirectory(GetFolderDir(&folder))) if (FO_MoveFolderDir(&folder, oldfolder))
               strcpy(oldfolder->Path, folder.Path);
      }
      strcpy(oldfolder->Name, folder.Name);
      strcpy(oldfolder->MLFromAddress,    folder.MLFromAddress);
      strcpy(oldfolder->MLReplyToAddress, folder.MLReplyToAddress);
      strcpy(oldfolder->MLAddress,        folder.MLAddress);
      strcpy(oldfolder->MLPattern,        folder.MLPattern);
      oldfolder->MLSignature = folder.MLSignature;
      oldfolder->Sort[0] = folder.Sort[0];
      oldfolder->Sort[1] = folder.Sort[1];
      oldfolder->MaxAge  = folder.MaxAge;
      if (!GetMUI(gui->CY_FTYPE, MUIA_Disabled))
      {
         int oldxpk = oldfolder->XPKType, newxpk = folder.XPKType;
         BOOL changed = TRUE;
         if (oldxpk == newxpk || (newxpk > 1 && !XpkBase)) changed = FALSE;
         else if (!(newxpk&1) && (oldxpk&1) && oldfolder->LoadedMode != 2) changed = MA_PromptFolderPassword(&folder, gui->WI);
         else if ((newxpk&1) && !(oldxpk&1)) changed = FO_EnterPassword(&folder);
         if ((newxpk&1) && (oldxpk&1)) strcpy(folder.Password, oldfolder->Password);
         if (changed)
         {
            if (!(newxpk&1)) *folder.Password = 0;
            FO_XPKUpdateFolder(&folder, oldxpk);
            oldfolder->XPKType = newxpk;
            strcpy(oldfolder->Password, folder.Password);
         }
         oldfolder->Type = folder.Type;
      }
      set(gui->WI, MUIA_Window_Open, FALSE);
      DoMethod(lv, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_All, 0, TAG_DONE);
      FO_SaveConfig(&folder);
      success = TRUE;
   }
   else
   {
      memset(&folder, 0, sizeof(struct Folder));
      FO_PutFolder(&folder);
      if (folder.XPKType&1) if (!FO_EnterPassword(&folder)) folder.XPKType &= ~1;
      set(gui->WI, MUIA_Window_Open, FALSE);
      if (CreateDirectory(GetFolderDir(&folder)))
      {
         int pos;
         get(lv, MUIA_NListtree_Active, &pos);
         DoMethod(lv, MUIM_NListtree_Insert, folder.Name, &folder, MUIV_NListtree_Insert_ListNode_Root, ++pos, MUIV_NListtree_Insert_Flag_Active, TAG_DONE);
         FO_SaveConfig(&folder);
         success = TRUE;
      }
   }
   if (success)
   {
      MA_SetSortFlag();
      DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_Sort);
      MA_ChangeFolder(FO_GetFolderByName(folder.Name, NULL));
      FO_SaveTree(CreateFilename(".folders"));
   }
   DisposeModulePush(&G->FO);
}
MakeStaticHook(FO_SaveHook, FO_SaveFunc);

///
/// FO_SetOrderFunc
//  Saves or resets folder order
HOOKPROTONHNO(FO_SetOrderFunc, void, enum SortOrder *arg)
{
   switch (*arg)
   {
      case SO_SAVE:  FO_SaveTree(CreateFilename(".folders")); break;
      case SO_RESET: DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Sort, MUIV_NListtree_Sort_ListNode_Root, MUIV_NListtree_Sort_Flag_RecursiveAll, TAG_DONE); break;
   }
}
MakeHook(FO_SetOrderHook, FO_SetOrderFunc);
///

/// FO_New
//  Creates folder configuration window
static struct FO_ClassData *FO_New(void)
{
   struct FO_ClassData *data;

   if (data = calloc(1, sizeof(struct FO_ClassData)))
   {
      APTR bt_okay, bt_cancel;
      static char *ftypes[4], *fmodes[5], *sortopt[8], *fsignat[5];
      fsignat[0] = GetStr(MSG_WR_NoSig);
      fsignat[1] = GetStr(MSG_WR_DefSig);
      fsignat[2] = GetStr(MSG_WR_AltSig1);
      fsignat[3] = GetStr(MSG_WR_AltSig2);
      fsignat[4] = NULL;
      sortopt[0] = GetStr(MSG_FO_MessageDate);
      sortopt[1] = GetStr(MSG_FO_DateRecvd);
      sortopt[2] = GetStr(MSG_Sender);
      sortopt[3] = GetStr(MSG_Recipient);
      sortopt[4] = GetStr(MSG_Subject);
      sortopt[5] = GetStr(MSG_Size);
      sortopt[6] = GetStr(MSG_Status);
      sortopt[7] = NULL;
      fmodes[0]  = GetStr(MSG_FO_FMNormal);
      fmodes[1]  = GetStr(MSG_FO_FMSimple);
      fmodes[2]  = GetStr(MSG_FO_FMPack);
      fmodes[3]  = GetStr(MSG_FO_FMEncPack);
      fmodes[4]  = NULL;
      ftypes[0]  = GetStr(MSG_FO_FTRcvdMail);
      ftypes[1]  = GetStr(MSG_FO_FTSentMail);
      ftypes[2]  = GetStr(MSG_FO_FTBothMail);
      ftypes[3]  = NULL;
      data->GUI.WI = WindowObject,
         MUIA_Window_Title, GetStr(MSG_FO_EditFolder),
         MUIA_HelpNode, "FO_W",
         MUIA_Window_ID, MAKE_ID('F','O','L','D'),
         WindowContents, VGroup,
            Child, ColGroup(2), GroupFrameT(GetStr(MSG_FO_Properties)),
               Child, Label2(GetStr(MSG_CO_Name)),
               Child, data->GUI.ST_FNAME = MakeString(SIZE_NAME,GetStr(MSG_CO_Name)),
               Child, Label2(GetStr(MSG_Path)),
               Child, HGroup,
                  MUIA_Group_HorizSpacing, 0,
                  Child, data->GUI.TX_FPATH = TextObject, MUIA_Background, MUII_TextBack, MUIA_Frame, MUIV_Frame_Text, End,
                  Child, data->GUI.BT_MOVE = PopButton(MUII_PopDrawer),
               End,
               Child, Label2(GetStr(MSG_FO_MaxAge)),
               Child, data->GUI.ST_MAXAGE = MakeInteger(4,GetStr(MSG_FO_MaxAge)),
               Child, Label1(GetStr(MSG_FO_FolderType)),
               Child, data->GUI.CY_FTYPE = MakeCycle(ftypes,GetStr(MSG_FO_FolderType)),
               Child, Label1(GetStr(MSG_FO_FolderMode)),
               Child, data->GUI.CY_FMODE = MakeCycle(fmodes,GetStr(MSG_FO_FolderMode)),
               Child, Label1(GetStr(MSG_FO_SortBy)),
               Child, HGroup,
                  Child, data->GUI.CY_SORT[0] = MakeCycle(sortopt,GetStr(MSG_FO_SortBy)),
                  Child, data->GUI.CH_REVERSE[0] = MakeCheck(GetStr(MSG_FO_Reverse)),
                  Child, LLabel1(GetStr(MSG_FO_Reverse)),
               End,
               Child, Label1(GetStr(MSG_FO_ThenBy)),
               Child, HGroup,
                  Child, data->GUI.CY_SORT[1] = MakeCycle(sortopt,GetStr(MSG_FO_ThenBy)),
                  Child, data->GUI.CH_REVERSE[1] = MakeCheck(GetStr(MSG_FO_Reverse)),
                  Child, LLabel1(GetStr(MSG_FO_Reverse)),
               End,
            End,
            Child, ColGroup(2), GroupFrameT(GetStr(MSG_FO_MLSupport)),
               Child, Label2(GetStr(MSG_FO_ToPattern)),
               Child, data->GUI.ST_MLPATTERN = MakeString(SIZE_PATTERN,GetStr(MSG_FO_ToPattern)),
               Child, Label2(GetStr(MSG_FO_ToAddress)),
               Child, data->GUI.ST_MLADDRESS = MakeString(SIZE_ADDRESS,GetStr(MSG_FO_ToAddress)),
               Child, Label2(GetStr(MSG_FO_FromAddress)),
               Child, data->GUI.ST_MLFROMADDRESS = MakeString(SIZE_ADDRESS,GetStr(MSG_FO_FromAddress)),
               Child, Label2(GetStr(MSG_FO_ReplyToAddress)),
               Child, data->GUI.ST_MLREPLYTOADDRESS = MakeString(SIZE_ADDRESS,GetStr(MSG_FO_ReplyToAddress)),
               Child, Label1(GetStr(MSG_WR_Signature)),
               Child, data->GUI.CY_MLSIGNATURE = MakeCycle(fsignat, GetStr(MSG_WR_Signature)),
            End,
            Child, ColGroup(3),
               Child, bt_okay = MakeButton(GetStr(MSG_Okay)),
               Child, HSpace(0),
               Child, bt_cancel = MakeButton(GetStr(MSG_Cancel)),
            End,
         End,
      End;
      if (data->GUI.WI)
      {
         DoMethod(G->App, OM_ADDMEMBER, data->GUI.WI);
         SetHelp(data->GUI.ST_FNAME,            MSG_HELP_FO_ST_FNAME            );
         SetHelp(data->GUI.TX_FPATH,            MSG_HELP_FO_TX_FPATH            );
         SetHelp(data->GUI.ST_MAXAGE,           MSG_HELP_FO_ST_MAXAGE           );
         SetHelp(data->GUI.CY_FMODE,            MSG_HELP_FO_CY_FMODE            );
         SetHelp(data->GUI.CY_FTYPE,            MSG_HELP_FO_CY_FTYPE            );
         SetHelp(data->GUI.CY_SORT[0],          MSG_HELP_FO_CY_SORT0            );
         SetHelp(data->GUI.CY_SORT[1],          MSG_HELP_FO_CY_SORT1            );
         SetHelp(data->GUI.CH_REVERSE[0],       MSG_HELP_FO_CH_REVERSE          );
         SetHelp(data->GUI.CH_REVERSE[1],       MSG_HELP_FO_CH_REVERSE          );
         SetHelp(data->GUI.ST_MLPATTERN,        MSG_HELP_FO_ST_MLPATTERN        );
         SetHelp(data->GUI.ST_MLADDRESS,        MSG_HELP_FO_ST_MLADDRESS        );
         SetHelp(data->GUI.ST_MLFROMADDRESS,    MSG_HELP_FO_ST_MLFROMADDRESS    );
         SetHelp(data->GUI.ST_MLREPLYTOADDRESS, MSG_HELP_FO_ST_MLREPLYTOADDRESS );
         SetHelp(data->GUI.CY_MLSIGNATURE,      MSG_HELP_WR_RA_SIGNATURE        );

         DoMethod(data->GUI.BT_MOVE  ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FO_MoveHook);
         DoMethod(bt_okay            ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FO_SaveHook);
         DoMethod(bt_cancel          ,MUIM_Notify,MUIA_Pressed             ,FALSE         ,MUIV_Notify_Application,2,MUIM_CallHook,&FO_CloseHook);
         DoMethod(data->GUI.WI       ,MUIM_Notify,MUIA_Window_CloseRequest ,TRUE          ,MUIV_Notify_Application,2,MUIM_CallHook,&FO_CloseHook);
         return data;
      }
      free(data);
   }
   return NULL;
}
///

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2002 by YAM Open Source Team

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
#include <libraries/gadtools.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include "extra.h"
#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_mime.h"
#include "YAM_read.h"
#include "YAM_rexx.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"
#include "classes/Classes.h"

/*
** The following structures are used to build the
** .index files of a folder.
**
** Alignment is very important here, because the
** structures are saved RAW to a file.
** Therefore we have to ensure that the alignment
** will be the same for 68k and PPC.
** That`s why we deal with the following #pragma
** calls
*/

#include "amiga-align.h"

/*
** structure of the compressed mail
**
** DO NOT CHANGE ALIGNMENT here or the .index
** files of a folder will be corrupt !
**
*/
struct ComprMail
{
   int              Flags;
   char             MailFile[SIZE_MFILE];
   struct DateStamp Date;      // the creation date of the mail within UTC
   struct timeval   transDate; // we need microseconds for received/sent Date
   char             Status;
   char             Importance;
   unsigned long    cMsgID;
   unsigned long    cIRTMsgID;
   long             Size;
   int              MoreBytes;
};

/*
** structure of the Folder Index
**
** DO NOT CHANGE ALIGNMENT here or the .index
** files of a folder will be corrupt !
**
*/
struct FIndex
{
   ULONG ID;
   int   Total;
   int   New;
   int   Unread;
   int   Size;
   long  reserved[2];
};

// whenever you change something up there (in FIndex or ComprMail) you
// need to increase this version ID!
#define FINDEX_VER  (MAKE_ID('Y','I','N','5'))

#include "default-align.h"

/* global variables */
struct Data2D Header = { 0, 0, NULL };

/* local protos */
static void MA_ValidateStatus(struct Folder*);
static char *MA_IndexFileName(struct Folder*);
static BOOL MA_DetectUUE(FILE*);
static void MA_GetRecipients(char*, struct Person**, int*);
static struct DateStamp *MA_ScanDate(char *date);

/***************************************************************************
 Module: Main - Folder handling
***************************************************************************/

/*** Index Maintenance ***/
/// MA_PromptFolderPassword
//  Asks user for folder password
BOOL MA_PromptFolderPassword(struct Folder *fo, APTR win)
{
   char passwd[SIZE_PASSWORD], prompt[SIZE_LARGE];
   struct User *user = US_GetCurrentUser();

   if (isFreeAccess(fo)) return TRUE;
   if (!Stricmp(fo->Password, user->Password)) return TRUE;
   sprintf(prompt, GetStr(MSG_MA_GetFolderPass), fo->Name);
   do {
      *passwd = 0;
      if (!StringRequest(passwd, SIZE_PASSWORD, GetStr(MSG_Folder), prompt, GetStr(MSG_Okay), NULL, GetStr(MSG_Cancel), TRUE, win)) return FALSE;
   } while (Stricmp(passwd, fo->Password));
   return TRUE;
}

///
/// MA_ValidateStatus
//  Avoids invalid status values
static void MA_ValidateStatus(struct Folder *folder)
{
   struct Mail *mail;

   if(C->UpdateNewMail            ||
      folder->Type == FT_OUTGOING ||
      folder->Type == FT_SENT
     )
   {
      DB(kprintf("Validating status of new msgs in folder %s\n", folder->Name);)

      for (mail = folder->Messages; mail; mail = mail->Next)
      {
        if (mail->Status == STATUS_NEW)
        {
          if (folder->Type == FT_OUTGOING)   MA_SetMailStatus(mail, STATUS_WFS);
          else if (folder->Type == FT_SENT)  MA_SetMailStatus(mail, STATUS_SNT);
          else                               MA_SetMailStatus(mail, STATUS_UNR);
        }
      }
   }
}

///
/// MA_IndexFileName
//  Returns file name of folder index
static char *MA_IndexFileName(struct Folder *folder)
{
   static char buffer[SIZE_PATHFILE];
   strcpy(buffer, GetFolderDir(folder));
   AddPart(buffer, ".index", SIZE_PATHFILE);
   return buffer;
}

///
/// MA_LoadIndex
//  Loads a folder index from disk
enum LoadedMode MA_LoadIndex(struct Folder *folder, BOOL full)
{
   FILE *fh;
   enum LoadedMode indexloaded = LM_UNLOAD;
   char buf[SIZE_LARGE];
   BOOL corrupt = FALSE;
   BOOL error = FALSE;

   DB( kprintf("Loading index for folder %s\n", folder->Name); )

   if ((fh = fopen(MA_IndexFileName(folder), "r")))
   {
      struct FIndex fi;

      BusyText(GetStr(MSG_BusyLoadingIndex), folder->Name);
      if(fread(&fi, sizeof(struct FIndex), 1, fh) != 1)
      {
        DB(kprintf("error while loading struct FIndex from .index file\n");)
        error = TRUE;
      }
      else if(fi.ID == FINDEX_VER)
      {
         folder->Total  = fi.Total;
         folder->New    = fi.New;
         folder->Unread = fi.Unread;
         folder->Size   = fi.Size;
         indexloaded = LM_FLUSHED;

         if (full)
         {
            ClearMailList(folder, TRUE);
            for(;;)
            {
               struct Mail mail;
               struct ComprMail cmail;
               memset(&mail, 0, sizeof(struct Mail));               
               if(fread(&cmail, sizeof(struct ComprMail), 1, fh) != 1)
               {
                  // check if we are here because of an error or EOF
                  if(ferror(fh) != 0 || feof(fh) == 0)
                  {
                    DB(kprintf("error while loading ComprMail struct from .index file\n");)
                    error = TRUE;
                  }

                  // if we end up here it is just a EOF and no error.
                  break;
               }

               if(cmail.MoreBytes > SIZE_LARGE)
               {
                  printf("WARNING: Index of folder '%s' CORRUPTED near mailfile '%s' (MoreBytes: 0x%x) - aborting!\n", folder->Name, cmail.MailFile, cmail.MoreBytes);
                  printf("File position: %ld\n", ftell(fh));

                  corrupt = TRUE;
                  break;
               }

               if(fread(buf, cmail.MoreBytes, 1, fh) != 1)
               {
                 DB(kprintf("fread error while reading index file\n");)
                 error = TRUE;
                 break;
               }

               strcpy(mail.Subject, GetNextLine(buf));
               strcpy(mail.From.Address, GetNextLine(NULL));
               strcpy(mail.From.RealName, GetNextLine(NULL));
               strcpy(mail.To.Address, GetNextLine(NULL));
               strcpy(mail.To.RealName, GetNextLine(NULL));
               strcpy(mail.ReplyTo.Address, GetNextLine(NULL));
               strcpy(mail.ReplyTo.RealName, GetNextLine(NULL));
               mail.Folder = folder;
               mail.Flags = cmail.Flags;
               setVOLValue(&mail, 0);  // we have to make sure that the volatile flag field isn`t loaded
               strcpy(mail.MailFile, cmail.MailFile);
               mail.Date = cmail.Date;
               mail.transDate = cmail.transDate;
               mail.Status = cmail.Status;
               mail.Importance = cmail.Importance;
               mail.cMsgID = cmail.cMsgID;
               mail.cIRTMsgID = cmail.cIRTMsgID;
               mail.Size = cmail.Size;

               // finally add the new mail structure to our mail list
               if(AddMailToList(&mail, folder) == NULL)
               {
                 DB(kprintf("AddMailToList returned NULL!\n");)
                 error = TRUE;
                 break;
               }
            }
         }
      }

      if(!error && ferror(fh) == 1)
        error = TRUE;

      BusyEnd;
      fclose(fh);
   }

   if(error)
   {
     DB(kprintf("an error occurred while trying to load the index file '%s'\n", MA_IndexFileName(folder));)
     ClearMailList(folder, TRUE);

     return LM_UNLOAD;
   }

   if(corrupt || indexloaded == LM_UNLOAD)
   {
     DB( kprintf("  corrupt or outdated .index file detected, %s\n", full ? "rebuilding..." : "skipping..."); )
     ClearMailList(folder, TRUE);
     if(full)
     {
        MA_ScanMailBox(folder);
        if(MA_SaveIndex(folder)) indexloaded = LM_VALID;
     }
   }
   else if(full)
   {
     indexloaded = LM_VALID;
     CLEAR_FLAG(folder->Flags, FOFL_MODIFY);
   }

   return indexloaded;
}

///
/// MA_SaveIndex
//  Saves a folder index to disk
BOOL MA_SaveIndex(struct Folder *folder)
{
   struct Mail *mail;
   FILE *fh;
   struct ComprMail cmail;
   char buf[SIZE_LARGE];
   struct FIndex fi;

   if (!(fh = fopen(MA_IndexFileName(folder), "w"))) return FALSE;
   BusyText(GetStr(MSG_BusySavingIndex), folder->Name);

   // lets prepare the Folder Index struct and write it out
   // we clear it first, so that the reserved field is also 0
   memset(&fi, 0, sizeof(struct FIndex));
   fi.ID = FINDEX_VER;
   fi.Total = folder->Total; fi.New = folder->New; fi.Unread = folder->Unread; fi.Size = folder->Size;
   fwrite(&fi, sizeof(struct FIndex), 1, fh);

   for (mail = folder->Messages; mail; mail = mail->Next)
   {
      memset(&cmail, 0, sizeof(struct ComprMail));
      sprintf(buf, "%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
         mail->Subject,
         mail->From.Address, mail->From.RealName,
         mail->To.Address, mail->To.RealName,
         mail->ReplyTo.Address, mail->ReplyTo.RealName);

      cmail.Flags = mail->Flags;
      setVOLValue(&cmail, 0);  // we have to make sure that the volatile flag field isn`t saved

      strcpy(cmail.MailFile, mail->MailFile);
      cmail.Date = mail->Date;
      cmail.transDate = mail->transDate;
      cmail.Status = mail->Status;
      cmail.Importance = mail->Importance;
      cmail.cMsgID = mail->cMsgID;
      cmail.cIRTMsgID = mail->cIRTMsgID;
      cmail.Size = mail->Size;
      cmail.MoreBytes = strlen(buf);
      fwrite(&cmail, sizeof(struct ComprMail), 1, fh);
      fwrite(buf, 1, cmail.MoreBytes, fh);
   }
   fclose(fh);
   CLEAR_FLAG(folder->Flags, FOFL_MODIFY);
   BusyEnd;
   return TRUE;
}

///
/// MA_GetIndex
//  Opens/unlocks a folder
BOOL MA_GetIndex(struct Folder *folder)
{
   if (!folder || folder->Type == FT_GROUP) return FALSE;

   if (folder->LoadedMode != LM_VALID)
   {
      if(isCryptedFolder(folder) && *folder->Password && !MA_PromptFolderPassword(folder, G->MA->GUI.WI))
        return FALSE;

      // load the index file
      folder->LoadedMode = MA_LoadIndex(folder, TRUE);

      // we need to refresh the entry in the folder listtree this folder belongs to
      // but only if there is a GUI already!
      if(folder->LoadedMode == LM_VALID)
      {
        MA_ValidateStatus(folder);

        if(G->MA)
        {
          struct MUI_NListtree_TreeNode *tn;
          int i;

          for(i=0;;i++)
          {
            tn = (struct MUI_NListtree_TreeNode *)DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_GetEntry, MUIV_NListtree_GetEntry_ListNode_Root, i, MUIV_NListtree_GetEntry_Flag_Visible);
            if(!tn) break;

            if(tn->tn_User == folder)
            {
              DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, i, MUIV_NListtree_Redraw_Flag_Nr);
              break;
            }
          }
        }
      }
   }

   return (BOOL)(folder->LoadedMode == LM_VALID);
}

///
/// MA_ExpireIndex
//  Invalidates a folder index
void MA_ExpireIndex(struct Folder *folder)
{
   if(!isModified(folder)) DeleteFile(MA_IndexFileName(folder));

   SET_FLAG(folder->Flags, FOFL_MODIFY);
}

///
/// MA_UpdateIndexes
//  Updates indices of all folders
void MA_UpdateIndexes(BOOL initial)
{
   int i;
   struct Folder **flist;

   if ((flist = FO_CreateList()))
   {
      for (i = 1; i <= (int)*flist; i++)
      {
        if (flist[i]->Type != FT_GROUP)
        {
         if (initial)
         {
            long dirdate = getft(GetFolderDir(flist[i]));
            long inddate = getft(MA_IndexFileName(flist[i]));
            if (dirdate > inddate+30 && inddate != -1)
            {
               DeleteFile(MA_IndexFileName(flist[i]));
               MA_GetIndex(flist[i]);
            }
         }
         else
         {
            if (flist[i]->LoadedMode == LM_VALID && isModified(flist[i])) MA_SaveIndex(flist[i]);
         }
        }
      }
      free(flist);
   }
}

///
/// MA_FlushIndexes
//  Removes loaded folder indices from memory and closes folders
void MA_FlushIndexes(BOOL all)
{
   int i;
   struct Folder *fo, **flist, *actfo = FO_GetCurrentFolder();

   if(!actfo) return;

   if ((flist = FO_CreateList()))
   {
      for (i = 1; i <= (int)*flist; i++)
      {
         fo = flist[i];

         if ((fo->Type == FT_SENT || fo->Type == FT_CUSTOM || fo->Type == FT_CUSTOMSENT) && fo != actfo  && fo->LoadedMode == LM_VALID && (all || isFreeAccess(fo)))
         {
            if(isModified(fo)) MA_SaveIndex(fo);
            ClearMailList(fo, FALSE);
            fo->LoadedMode = LM_FLUSHED;
            CLEAR_FLAG(fo->Flags, FOFL_FREEXS);
         }
      }
      free(flist);
      DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_All, MUIF_NONE);
   }
}

HOOKPROTONHNONP(MA_FlushIndexFunc, void)
{
   MA_FlushIndexes(TRUE);
}
MakeHook(MA_FlushIndexHook, MA_FlushIndexFunc);
///

/*** Private functions ***/
/// MA_ChangeFolder
//  Changes to another folder
void MA_ChangeFolder(struct Folder *folder, BOOL set_active)
{
  struct Folder *actfo = FO_GetCurrentFolder();
  struct MA_GUIData *gui = &G->MA->GUI;

  if(!actfo) return;

  if(!folder) folder = actfo;
  else if(actfo == folder) return;
  else if(set_active) FO_SetCurrentFolder(folder);

  // if this folder should be disabled, lets do it now
  if(folder->Type == FT_GROUP || MA_GetIndex(folder) == FALSE)
  {
    SetAttrs(gui->NL_MAILS, MUIA_Disabled, TRUE,
                            MUIA_ShortHelp, NULL,
                            TAG_DONE);

    DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);
  }
  else
  {
    // set the SortFlag in the NList accordingly
    MA_SetSortFlag();

    // Now we update the InfoBar accordingly
    DoMethod(gui->IB_INFOBAR, MUIM_InfoBar_SetFolder, folder);

    // Create the Mail List and display it
    DisplayMailList(folder, gui->NL_MAILS);

    // now we have to assure that the folder is enabled
    set(gui->NL_MAILS, MUIA_Disabled, FALSE);

    // Now we jump to messages that are NEW
    if(C->JumpToNewMsg) MA_JumpToNewMsg();
    else if(folder->LastActive >= 0) set(gui->NL_MAILS, MUIA_NList_Active, folder->LastActive);

    // if there is still no entry active in the NList we make the first one active
    if(xget(gui->NL_MAILS, MUIA_NList_Active) == MUIV_NList_Active_Off)
    {
      set(gui->NL_MAILS, MUIA_NList_Active, MUIV_NList_Active_Top);
    }
  }
}

HOOKPROTONHNONP(MA_ChangeFolderFunc, void)
{
   MA_ChangeFolder(NULL, FALSE);
}
MakeHook(MA_ChangeFolderHook, MA_ChangeFolderFunc);
///
/// MA_FLContextMenuBuild
// If the user chooses a item out of the ContextMenu we have to process it.
enum { CMN_EDITF=10, CMN_DELETEF, CMN_INDEX, CMN_NEWF, CMN_NEWFG, CMN_SNAPS, CMN_RELOAD };

ULONG MA_FLContextMenuBuild(struct IClass *cl, Object *obj, struct MUIP_NList_ContextMenuBuild *msg)
{
  struct FL_Data *data = (struct FL_Data *)INST_DATA(cl,obj);
  struct MUI_NListtree_TestPos_Result r;
  struct MUI_NListtree_TreeNode *tn;
  struct Folder *folder = NULL;
  struct MA_GUIData *gui = &G->MA->GUI;
  BOOL disable_delete   = FALSE;
  BOOL disable_edit     = FALSE;
  BOOL disable_update   = FALSE;

  // dispose the old context_menu if it still exists
  if(data->context_menu)
  {
    MUI_DisposeObject(data->context_menu);
    data->context_menu = NULL;
  }

  // if this was a RMB click on the titlebar we create our own special menu
  if(msg->ontop)
  {
  	data->context_menu = MenustripObject,
	  	Child, MenuObjectT(GetStr(MSG_MA_CTX_FOLDERLIST)),
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Folder), MUIA_UserData, 1, MUIA_Menuitem_Enabled, FALSE, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<0)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Total),  MUIA_UserData, 2, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<1)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Unread), MUIA_UserData, 3, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<2)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_New),    MUIA_UserData, 4, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<3)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_Size),   MUIA_UserData, 5, MUIA_Menuitem_Checked, isFlagSet(C->FolderCols, (1<<4)), MUIA_Menuitem_Checkit, TRUE, MUIA_Menuitem_Toggle, TRUE, End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
  			Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFWIDTH_THIS), MUIA_UserData, MUIV_NList_Menu_DefWidth_This, End,
	  		Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFWIDTH_ALL),  MUIA_UserData, MUIV_NList_Menu_DefWidth_All,  End,
		  	Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFORDER_THIS), MUIA_UserData, MUIV_NList_Menu_DefOrder_This, End,
			  Child, MenuitemObject, MUIA_Menuitem_Title, GetStr(MSG_MA_CTX_DEFORDER_ALL),  MUIA_UserData, MUIV_NList_Menu_DefOrder_All,  End,
			End,
		End;

    return (ULONG)data->context_menu;
  }

  // Now lets find out which entry is under the mouse pointer
  DoMethod(gui->NL_FOLDERS, MUIM_NListtree_TestPos, msg->mx, msg->my, &r);

  tn = r.tpr_TreeNode;

  if(!tn || !tn->tn_User)
  {
    disable_delete = TRUE;
    disable_edit   = TRUE;
    disable_update = TRUE;
  }
  else
  {
    folder = (struct Folder *)tn->tn_User;

    // Set this Treenode as activ
    if(tn != (struct MUI_NListtree_TreeNode *)xget(gui->NL_FOLDERS, MUIA_NListtree_Active))
    {
      set(gui->NL_FOLDERS, MUIA_NListtree_Active, tn);
    }

    // Now we have to set the disabled flag if this is not a custom folder
    if(folder->Type != FT_CUSTOM && folder->Type != FT_CUSTOMSENT && folder->Type != FT_CUSTOMMIXED && folder->Type != FT_GROUP)
    {
      disable_delete = TRUE;
    }

    if(folder->Type == FT_GROUP)
    {
      disable_update = TRUE;
    }
  }

  // We create the ContextMenu now
  data->context_menu = MenustripObject,
    Child, MenuObjectT(folder ? FolderName(folder) : GetStr(MSG_FOLDER_NONSEL)),
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_EDIT),           MUIA_Menuitem_Enabled, !disable_edit,   MUIA_UserData, CMN_EDITF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_DELETE),         MUIA_Menuitem_Enabled, !disable_delete, MUIA_UserData, CMN_DELETEF, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_MA_UpdateIndex),        MUIA_Menuitem_Enabled, !disable_update, MUIA_UserData, CMN_INDEX,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_NEWFOLDER),      MUIA_UserData, CMN_NEWF,   End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_NEWFOLDERGROUP), MUIA_UserData, CMN_NEWFG,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, NM_BARLABEL, End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_SNAPSHOT),       MUIA_UserData, CMN_SNAPS,  End,
      Child, MenuitemObject, MUIA_Menuitem_Title, GetStripStr(MSG_FOLDER_RELOAD),         MUIA_UserData, CMN_RELOAD, End,
    End,
  End;

  return (ULONG)data->context_menu;
}

///
/// MA_FLContextMenuChoice
// If the user chooses a item out of the ContextMenu we have to process it.
ULONG MA_FLContextMenuChoice(struct IClass *cl, Object *obj, struct MUIP_ContextMenuChoice *msg)
{
  switch(xget(msg->item, MUIA_UserData))
  {
    // if the user selected a TitleContextMenu item
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    {
      ULONG col = xget(msg->item, MUIA_UserData)-1;

      if(isFlagSet(C->FolderCols, (1<<col))) CLEAR_FLAG(C->FolderCols, (1<<col));
      else                                   SET_FLAG(C->FolderCols, (1<<col));

      MA_MakeFOFormat(G->MA->GUI.NL_FOLDERS);
    }
    break;

    // or other item out of the FolderListContextMenu
    case CMN_EDITF:   { DoMethod(G->App, MUIM_CallHook, &FO_EditFolderHook);          } break;
    case CMN_DELETEF: { DoMethod(G->App, MUIM_CallHook, &FO_DeleteFolderHook);        } break;
    case CMN_INDEX:   { DoMethod(G->App, MUIM_CallHook, &MA_RescanIndexHook);         } break;
    case CMN_NEWF:    { DoMethod(G->App, MUIM_CallHook, &FO_NewFolderHook);           } break;
    case CMN_NEWFG:   { DoMethod(G->App, MUIM_CallHook, &FO_NewFolderGroupHook);      } break;
    case CMN_SNAPS:   { DoMethod(G->App, MUIM_CallHook, &FO_SetOrderHook, SO_SAVE);   } break;
    case CMN_RELOAD:  { DoMethod(G->App, MUIM_CallHook, &FO_SetOrderHook, SO_RESET);  } break;

    default:
    {
      return DoSuperMethodA(cl, obj, (Msg)msg);
    }
  }

  return 0;
}

///
/// MA_JumpToNewMsg
// Function that jumps to the first or last unread mail in a folder,
// depending on sort order of the folder
BOOL MA_JumpToNewMsg(VOID)
{
  struct Folder *folder = FO_GetCurrentFolder();
  int i, incr, pos = -1;

  if(folder->Sort[0] < 0 || folder->Sort[1] < 0)
  {
    get(G->MA->GUI.NL_MAILS, MUIA_NList_Entries, &i);
    i--;
    incr = -1;
  }
  else
  {
    i = 0;
    incr = 1;
  }

  while(1)
  {
    struct Mail *mail;
    DoMethod(G->MA->GUI.NL_MAILS, MUIM_NList_GetEntry, i, &mail);
    if(!mail) break;

    if (mail->Status == STATUS_NEW || mail->Status == STATUS_UNR)
    {
      pos = i;
      break;
    }

    i += incr;
  }

  set(G->MA->GUI.NL_MAILS, MUIA_NList_Active, pos >= 0 ? pos : folder->LastActive);

  return TRUE;
}
///

/*** Mail header scanning ***/
/// MA_NewMailFile
//  Returns an unique name for a new mail file
char *MA_NewMailFile(struct Folder *folder, char *mailfile, int daynumber)
{
   static char buffer[SIZE_PATHFILE];
   char mfile[SIZE_MFILE];
   struct Mail *mail;
   int cnt, mcnt = 0;
   
   if (!mailfile) mailfile = mfile;
   if (!daynumber) { struct DateStamp ds; DateStamp(&ds); daynumber = ds.ds_Days; }
   MA_GetIndex(folder);
   for (mail = folder->Messages; mail; mail = mail->Next)
      if (atoi(mail->MailFile) == daynumber)
         if ((cnt = atoi(&(mail->MailFile)[6])) > mcnt) mcnt = cnt;
   do {
      sprintf(mailfile, "%05d.%03d", daynumber, ++mcnt);
      strcpy(buffer, GetFolderDir(folder));
      AddPart(buffer, mailfile, SIZE_PATHFILE);
   } while (access(buffer,F_OK) == 0);
   return buffer;
}

///
/// MA_DetectUUE
//  Checks if message contains an uuencoded file
static BOOL MA_DetectUUE(FILE *fh)
{
   char *buffer;
   BOOL found = FALSE;

   buffer = calloc(SIZE_LINE,1);

   // Now we process the whole mailfile and check if there is any line that
   // starts with "begin xxx"
   while(GetLine(fh, buffer, SIZE_LINE))
   {
      // lets check for digit first because this will throw out many others first
      if(isdigit((int)buffer[6]) && strncmp(buffer, "begin ", 6) == 0)
      {
         found = TRUE;
         break;
      }
   }

   free(buffer);
   return found;
}

///
/// MA_ReadHeader
//  Reads header lines of a message into memory
BOOL MA_ReadHeader(FILE *fh)
{
  char *buffer, *ptr;
  BOOL success = FALSE;
  BOOL finished = FALSE;
  int linesread = 0;

  if((buffer = calloc(SIZE_LINE, sizeof(char))))
  {
    FreeData2D(&Header);

    // we read out the whole header line by line and
    // concatenate lines that are belonging together.
    while(GetLine(fh, buffer, SIZE_LINE) && ++linesread && buffer[0] ||
          (finished == FALSE,finished = TRUE))
    {
      BOOL linestart = TRUE;

      // if the start of this line is a space or a tabulator sign
      // this line belongs to the last header also and we have to
      // add it to the last one.
      if((buffer[0] == ' ' || buffer[0] == '\t') &&
         Header.Used && finished==FALSE)
      {
        // move to the "real" start of the string so that we can copy
        // from there to our previous header.
        for(ptr = buffer; *ptr && isspace(*ptr); ptr++);

        // we want to preserve the last space so that this headerline
        // is correctly connected
        if(ptr != buffer) *(--ptr) = ' ';

        // now concatenate this new headerstring to our previous one
        ptr = StrBufCat(Header.Data[Header.Used-1], ptr);
      }
      else
      {
        // it seems that we have found another header line because
        // it didn`t start with a linear-white-space, so lets
        // first validate the previous one, if it exists.
        if(Header.Used)
        {
          char *prevHeader = Header.Data[Header.Used-1];
          int len;

          // we first decode the header according to RFC 2047 which
          // should give us the full charset interpretation
          if((len = rfc2047_decode(prevHeader, prevHeader, strlen(prevHeader),
             (G->TTin && G->TTin->Header) ? G->TTin : NULL)) == -1)
          {
            DB(kprintf("ERROR: malloc() error during rfc2047() decoding\n");)
            break; // break-out
          }
          else if(len == -2)
          {
            DB(kprintf("WARNING: unknown header encoding found\n");)

            // signal an error but continue.
            ER_NewError(GetStr(MSG_ER_UnknownHeaderEnc), prevHeader, NULL);
          }
          else if(len == -3)
          {
            DB(kprintf("WARNING: base64 header decoding failed\n");)
          }

          // now that we have decoded the headerline accoring to rfc2047
          // we have to strip out eventually existing ESC sequences as
          // this can be dangerous with MUI.
          for(ptr=prevHeader; *ptr; ptr++)
          {
            // if we find a ESC sequence, strip it!
            if(*ptr == 0x1b) { *ptr = ' '; linestart = FALSE; }
            else if(!success && linestart)
            {
              // we also need to analyse if we at least found one valid headerline
              // or not, because then wenn need to return FALSE
              if(*ptr == ':') success = TRUE;
              else if(isspace(*ptr)) linestart = FALSE;
            }
          }
        }

        if(finished)
          break;

        // now that we have finished the last header line
        // we can finally start processing the new one.
        ptr = StrBufCpy(AllocData2D(&Header, SIZE_DEFAULT), buffer);
      }

      Header.Data[Header.Used-1] = ptr;
    }

    free(buffer);
  }

  // we have to make sure that the Header data is empty if we
  // hadn`t success in getting some headers
  if(!success) FreeData2D(&Header);

  return (BOOL)(success == TRUE || linesread == 1);
}  

///
/// MA_FreeEMailStruct
//  Frees an extended email structure
void MA_FreeEMailStruct(struct ExtendedMail *email)
{
   if(email)
   {
      FreeStrBuf(email->SenderInfo);
      FreeStrBuf(email->Headers);
      if (email->NoSTo) free(email->STo);
      if (email->NoCC ) free(email->CC );
      if (email->NoBCC) free(email->BCC);
      free(email);
   }
}

///
/// MA_GetRecipients
//  Extracts recipients from a header field
static void MA_GetRecipients(char *h, struct Person **per, int *percnt)
{
   int cnt;
   char *p = h, *next;
   for (cnt = 0; *p;)
   {
      cnt++;
      if ((p = MyStrChr(p, ','))) p++; else break;
   }
   *percnt = cnt;
   if (cnt)
   {
      *per = calloc(cnt, sizeof(struct Person));
      for (cnt = 0, p = h; *p; cnt++)
      {
         if ((next = MyStrChr(p, ','))) *next++ = 0;
         ExtractAddress(p, (struct Person *)((ULONG)*per+cnt*sizeof(struct Person)));
         if (!(p = next)) break;
      }
   }
}

///
/// MA_ExamineMail
//  Parses the header lines of a message and fills email structure
struct ExtendedMail *MA_ExamineMail(struct Folder *folder, char *file, char *statstr, BOOL deep)
{
   struct ExtendedMail *email;
   static struct Person pe;
   struct Mail *mail;
   char *p, fullfile[SIZE_PATHFILE];
   int ok, i;
   struct DateStamp *foundDate = NULL;
   FILE *fh;

   // first we generate a new ExtendedMail buffer
   if(!(email = calloc(1, sizeof(struct ExtendedMail)))) return NULL;

   mail = &email->Mail;
   stccpy(mail->MailFile, file, SIZE_MFILE);
   email->DelSend = !C->SaveSent;
   if ((fh = fopen(GetMailFile(fullfile, folder, mail), "r")))
   {
      BOOL xpk = FALSE;
      if (fgetc(fh) == 'X') if (fgetc(fh) == 'P') if (fgetc(fh) == 'K') xpk = TRUE;
      if (xpk)
      {
         fclose(fh);
         if (!StartUnpack(GetMailFile(NULL, folder, mail), fullfile, folder)) return NULL;
         fh = fopen(fullfile, "r");
      }
      else rewind(fh);
   }

   if (fh)
   {
      MA_ReadHeader(fh);

      // Now we process the read header to set all flags accordingly
      for (ok=i=0; i < Header.Used; i++)
      {
         char *value, *field = Header.Data[i];
         if ((value = strchr(field, ':')))
         {
            *value++ = 0;
            if (!stricmp(field, "from"))
            {
               SET_FLAG(ok, 1);
               SParse(value);
               ExtractAddress(value, &pe);
               mail->From = pe;
            }
            else if (!stricmp(field, "reply-to"))
            {
               SET_FLAG(ok, 8);
               SParse(value);
               ExtractAddress(value, &pe);
               mail->ReplyTo = pe;
            }
            else if (!stricmp(field, "original-recipient"))
            {
               ExtractAddress(value, &pe);
               email->OriginalRcpt = pe;
            }
            else if (!stricmp(field, "disposition-notification-to"))
            {
               ExtractAddress(value, &pe);
               email->ReceiptTo = pe;
               email->ReceiptType = RCPT_TYPE_ALL;
               SET_FLAG(mail->Flags, MFLAG_SENDMDN);
            }
            else if (!stricmp(field, "return-view-to"))
            {
               ExtractAddress(value, &pe);
               email->ReceiptTo = pe;
               email->ReceiptType = RCPT_TYPE_READ;
            }
            else if (!stricmp(field, "return-receipt-to"))
            {
               email->RetRcpt = TRUE;
            }
            else if (!stricmp(field, "to"))
            {
               if(!(ok & 2))
               {
                  SET_FLAG(ok, 2);
                  SParse(value);
                  if ((p = MyStrChr(value, ','))) *p++ = 0;
                  ExtractAddress(value, &pe);
                  mail->To = pe;
                  if (p)
                  {
                    SET_FLAG(mail->Flags, MFLAG_MULTIRCPT);
                    if (deep && !email->NoSTo) MA_GetRecipients(p, &(email->STo), &(email->NoSTo));
                  }
               }
            }
            else if (!stricmp(field, "cc"))
            {
               SET_FLAG(mail->Flags, MFLAG_MULTIRCPT);
               if (deep && !email->NoCC) MA_GetRecipients(value, &(email->CC), &(email->NoCC));
            }
            else if (!stricmp(field, "bcc"))
            {
               SET_FLAG(mail->Flags, MFLAG_MULTIRCPT);
               if (deep && !email->NoBCC) MA_GetRecipients(value, &(email->BCC), &(email->NoBCC));
            }
            else if (!stricmp(field, "subject"))
            {
               SET_FLAG(ok, 4);
               SParse(value);
               stccpy(mail->Subject, Trim(value), SIZE_SUBJECT);
            }
            else if (!stricmp(field, "message-id"))
            {
               mail->cMsgID = CompressMsgID(p = Trim(value));
               stccpy(email->MsgID, p, SIZE_MSGID);
            }
            else if (!stricmp(field, "in-reply-to"))
            {
               mail->cIRTMsgID = CompressMsgID(p = Trim(value));
               stccpy(email->IRTMsgID, p, SIZE_MSGID);
            }
            else if (!stricmp(field, "date"))
            {
               foundDate = MA_ScanDate(value);
            }
            else if (!stricmp(field, "importance"))
            {
               p = Trim(value);
               if (!stricmp(p, "high")) mail->Importance = 1;
               else if(!stricmp(p, "low")) mail->Importance = -1;
            }
            else if (!stricmp(field, "priority"))
            {
               if(!mail->Importance)
               {
                  p = Trim(value);
                  if(!stricmp(p, "urgent")) mail->Importance = 1;
                  else if(!stricmp(p, "non-urgent")) mail->Importance = -1;
               }
            }
            else if (!stricmp(field, "content-type"))
            {
               p = Trim(value);
               if (!strnicmp(p, "multipart/mixed", 15))         SET_FLAG(mail->Flags, MFLAG_MULTIPART);
               else if(!strnicmp(p, "multipart/report", 16))    SET_FLAG(mail->Flags, MFLAG_REPORT);
               else if(!strnicmp(p, "multipart/encrypted", 19)) SET_FLAG(mail->Flags, MFLAG_CRYPT);
               else if(!strnicmp(p, "multipart/signed", 16))    SET_FLAG(mail->Flags, MFLAG_SIGNED);
            }
            else if (!stricmp(field, "x-senderinfo"))
            {
               SET_FLAG(mail->Flags, MFLAG_SENDERINFO);
               SParse(value);
               if (deep) email->SenderInfo = StrBufCpy(email->SenderInfo, value);
            }
            else if(deep) // and if we end up here we check if we really have to go further
            {
              if(!stricmp(field, "x-yam-options"))
              {
                enum Security sec;

                if (strstr(value, "delsent")) email->DelSend = TRUE;
                if ((p = strstr(value, "sigfile"))) email->Signature = p[7]-'0'+1;
                for(sec = SEC_SIGN; sec <= SEC_SENDANON; sec++)
                {
                  if(strstr(value, SecCodes[sec]))
                  email->Security = sec;
                }
              }
              else if(!strnicmp(field, "x-yam-header-", 13))
              {
                email->Headers = StrBufCat(StrBufCat(email->Headers, &field[13]), ":");
                email->Headers = StrBufCat(StrBufCat(email->Headers, value), "\\n");
              }
            }
         }
      }

      // if now the mail is still not MULTIPART we have to check for uuencoded attachments
      if(!isMultiPartMail(mail))
      {
         if (MA_DetectUUE(fh)) SET_FLAG(mail->Flags, MFLAG_MULTIPART);
      }

      // And now we close the Mailfile
      fclose(fh);

      FreeData2D(&Header);
      if ((ok & 8) && !mail->ReplyTo.RealName[0] && !stricmp(mail->ReplyTo.Address, mail->From.Address)) strcpy(mail->ReplyTo.RealName, mail->From.RealName);

      // then we check the fileComment field and set the mail status
      // aswell as the Date
      if(statstr && *statstr)
      {
         // by default this mail should be new
         for (mail->Status = STATUS_NEW, i = 0; i < 9; i++)
         {
            if (statstr[0] == *Status[i])
            {
              mail->Status = i;
              break;
            }
         }

         // now we check if second char is present and if we set the mailfag
         // as "marked" and the arexx permanent flag
         if(statstr[1] && statstr[1] != ' ')
         {
            // we have to check for the permanent flag also.
            if(statstr[1] >= 'M' && statstr[1] <= 'M'+7)
            {
              setPERValue(mail, statstr[1]-'M');
              SET_FLAG(mail->Flags, MFLAG_MARK);
            }
            else if(statstr[1] >= '1' && statstr[1] <= '7')
            {
              setPERValue(mail, statstr[1]-'1'+1);
            }
         }

         // now we check if this comment also has the transfer Date included
         // we only take the string if it is exactly 12bytes long or
         // otherwise it could be some weird data in the Comment string
         if(statstr[2] && statstr[14] == '\0')
         {
            // we need a buffer for the base64decoding
            // or we risk to let our mail structure to get overwritten.
            char buf[sizeof(struct timeval)+1]; // +1 because the b64decode does set a NUL byte

            // lets decode the base64 encoded timestring directly
            // into the mail->transDate timeval structure
            if(base64decode(&buf[0], &statstr[2], 12) <= 0)
            {
              DB(kprintf("WARNING: failure in decoding the encoded date in mailfile: '%s'\n", mail->MailFile);)

              // if we weren`t able to decode the base64 encoded comment string
              // we have to validate the transDate so that the calling function
              // recognizes to rewrite the comment with a valid string.
              mail->transDate.tv_micro = 0;
              mail->transDate.tv_secs  = 0;
            }
            else
            {
              // everything seems to worked so lets copy the binary data in our
              // transDate structure
              memcpy(&mail->transDate, &buf[0], sizeof(struct timeval));
            }
         }
      }

      // if we found the Date in the mail itself and it was convertable we
      // copy it out of the foundDate struct
      if(foundDate)
      {
        memcpy(&mail->Date, foundDate, sizeof(struct DateStamp));
      }
      else if(mail->transDate.tv_secs > 0) // if not we take the transfered Date (if found)
      {
        // convert the UTC transDate to a UTC mail Date
        TimeVal2DateStamp(&mail->transDate, &mail->Date, TZC_NONE);
      }
      else
      {
        // and as a fallback we take the name of the MailFile as the days
        mail->Date.ds_Days = atoi(mail->MailFile);
      }

      // lets calculate the mailSize out of the FileSize() function
      mail->Size = FileSize(fullfile);

      FinishUnpack(fullfile);

      return email;
   }

   FinishUnpack(fullfile);
   return NULL;
}

///
/// MA_ScanMailBox
//  Scans for message files in a folder directory
void MA_ScanMailBox(struct Folder *folder)
{
   struct ExtendedMail *email;
   struct FileInfoBlock *fib;
   BPTR lock;

   BusyText(GetStr(MSG_BusyScanning), folder->Name);
   ClearMailList(folder, TRUE);

   DB(kprintf("Recanning index for folder: %s...\n", folder->Name);)

   if((lock = Lock(GetFolderDir(folder), ACCESS_READ)))
   {
      if((fib = AllocDosObject(DOS_FIB, NULL)))
      {
          if(Examine(lock, fib) != DOSFALSE && ExNext(lock, fib) != DOSFALSE)
          {
            BOOL finish = FALSE;

            while(!finish)
            {
              struct FileInfoBlock fib_copy = *fib;

              // lets get the next fib immediatly, because the the file for this fib
              // can be immediatly deleted somehow and then we loose the dirtree.
              // So we somehow "prescan" this dir with this method.
              if(ExNext(lock, fib) == DOSFALSE) finish = TRUE;

              DoMethod(G->App,MUIM_Application_InputBuffered);
              if(isFile(&fib_copy) && IsValidMailFile(fib_copy.fib_FileName))
              {
                if(fib_copy.fib_Size)
                {
                  DB(kprintf("  examining MailFile: %s\n", fib_copy.fib_FileName);)

                  if((email = MA_ExamineMail(folder, fib_copy.fib_FileName, fib_copy.fib_Comment, FALSE)))
                  {
                    struct Mail *newMail = AddMailToList(&email->Mail, folder);

                    // if this new mail hasn`t got a valid transDate we have to check if we
                    // have to take the fileDate as a fallback value.
                    if(newMail->transDate.tv_secs == 0)
                    {
                      // only if it is _not_ a "waitforsend" and "hold" message we can take the fib_Date
                      // as the fallback
                      if(newMail->Status != STATUS_WFS && newMail->Status != STATUS_HLD)
                      {
                         DB(kprintf("    no transfer Date information found in comment, taking fileDate...\n");)

                         // now convert the local TZ fib_Date to a UTC transDate
                         DateStamp2TimeVal(&fib_copy.fib_Date, &newMail->transDate, TZC_UTC);

                         // then we write back the filecomment
                         MA_SetMailComment(newMail);
                      }
                    }

                    MA_FreeEMailStruct(email);
                  }
                }
                else
                {
                  char path[SIZE_PATHFILE];
                  NameFromLock(lock, path, SIZE_PATHFILE);
                  AddPart(path, fib_copy.fib_FileName, SIZE_PATHFILE);
                  DeleteFile(path);
                }
              }
            }
          }
          FreeDosObject(DOS_FIB, fib);
      }
      UnLock(lock);
   }
   BusyEnd;
}
///
/// MA_ScanDate
//  Converts textual date header into datestamp format
static struct DateStamp *MA_ScanDate(char *date)
{
   int count = 0, day = 0, mon = 0, year = 0, hour = 0, min = 0, sec = 0;
   char *tzone = "", *p, tdate[SIZE_SMALL], ttime[SIZE_SMALL];
   static struct DateTime dt;
   struct DateStamp *ds = &dt.dat_Stamp;

   if((p = strchr(date, ','))) p++;
   else p = date;

   while(*p && isspace(*p)) p++;

   while((p = strtok(p, " \t")))
   {
      switch (count)
      {
         // get the day
         case 0:
         {
            if(!isdigit(*p) || (day = atoi(p)) > 31) return NULL;
         }
         break;

         // get the month
         case 1:
         {
            for(mon = 1; mon <= 12; mon++)
            {
              if(strnicmp(p, months[mon-1], 3) == 0) break;
            }
            if(mon > 12) return NULL;
         }
         break;

         // get the year
         case 2:
         {
            year = atoi(p);
         }
         break;

         // get the time values
         case 3:
         {
            if(sscanf(p, "%d:%d:%d", &hour, &min, &sec) == 3) ;
            else if (sscanf (p, "%d:%d", &hour, &min) == 2) sec = 0;
            else return NULL;
         }
         break;

         // get the time zone
         case 4:
         {
            while (*p && (isspace(*p) || *p == '(')) p++;
            tzone = p;
         }
         break;
      }

      // if we iterated until 4 we can break out
      if(count == 4) break;

      count++;
      p = NULL;
   }

   // then format a standard DateStamp string like string
   // so that we can use StrToDate()
   sprintf(tdate, "%02d-%02d-%02d", mon, day, year%100);
   sprintf(ttime, "%02d:%02d:%02d", hour, min, sec);
   dt.dat_Format  = FORMAT_USA;
   dt.dat_Flags   = 0;
   dt.dat_StrDate = (STRPTR)tdate;
   dt.dat_StrTime = (STRPTR)ttime;
   if(!StrToDate(&dt)) return NULL;

   // bring the date in relation to UTC
   ds->ds_Minute -= TZtoMinutes(tzone);

   // we need to check the datestamp variable that it is still in it`s borders
   // after the UTC correction
   while(ds->ds_Minute < 0)     { ds->ds_Minute += 1440; ds->ds_Days--; }
   while(ds->ds_Minute >= 1440) { ds->ds_Minute -= 1440; ds->ds_Days++; }

   return ds;
}
///

/*** Hooks ***/
/// PO_InitFolderList
//  Creates a popup list of all folders
HOOKPROTONHNP(PO_InitFolderList, long, Object *pop)
{  
   int i;
   struct Folder **flist;

   DoMethod(pop, MUIM_List_Clear);
   DoMethod(pop, MUIM_List_InsertSingle, GetStr(MSG_MA_Cancel), MUIV_List_Insert_Bottom);
   if ((flist = FO_CreateList()))
   {
      for (i = 1; i <= (int)*flist; i++) if (flist[i]->Type != FT_GROUP)
         DoMethod(pop, MUIM_List_InsertSingle, flist[i]->Name, MUIV_List_Insert_Bottom);
      free(flist);
   }
   return TRUE;
}
MakeHook(PO_InitFolderListHook, PO_InitFolderList);

///
/// MA_LV_FDspFunc
//  Folder listview display hook
HOOKPROTONHNO(MA_LV_FDspFunc, long, struct MUIP_NListtree_DisplayMessage *msg)
{
   if(msg == NULL)
    return 0;

   if(msg->TreeNode != NULL)
   {
      static char dispfold[SIZE_DEFAULT], disptot[SIZE_SMALL], dispunr[SIZE_SMALL], dispnew[SIZE_SMALL], dispsiz[SIZE_SMALL];

      struct Folder *entry = (struct Folder *)msg->TreeNode->tn_User;

      msg->Array[0] = msg->Array[1] = msg->Array[2] = msg->Array[3] = msg->Array[4] = "";
      *dispsiz = 0;

      switch(entry->Type)
      {
        case FT_GROUP:
        {
          sprintf(msg->Array[0] = dispfold, "\033o[%d] %s", (isFlagSet(msg->TreeNode->tn_Flags, TNF_OPEN) ? 1 : 0), entry->Name);
          msg->Preparse[0] = (entry->New+entry->Unread) ? (MUIX_B MUIX_PH) : MUIX_PH;
        }
        break;

        default:
        {
          if(entry->ImageIndex >= 0) sprintf(msg->Array[0] = dispfold, "\033o[%d] ", entry->ImageIndex);
          else strcpy(msg->Array[0] = dispfold, " ");

          if (entry->Name[0]) strcat(dispfold, entry->Name);
          else sprintf(dispfold, "(%s)", FilePart(entry->Path));

          if(isCryptedFolder(entry)) sprintf(dispfold, "%s \033o[%d]", dispfold, MAXBCFOLDERIMG);

          if(entry->LoadedMode != LM_UNLOAD)
          {
            if(entry->New+entry->Unread) msg->Preparse[0] = MUIX_PH;
            if(C->FolderCols & (1<<1)) sprintf(msg->Array[1] = disptot, "%d", entry->Total);
            if(C->FolderCols & (1<<2)) sprintf(msg->Array[2] = dispunr, "%d", entry->Unread);
            if(C->FolderCols & (1<<3)) sprintf(msg->Array[3] = dispnew, "%d", entry->New);
            if(C->FolderCols & (1<<4)) FormatSize(entry->Size, msg->Array[4] = dispsiz);
          }
        }
      }
   }
   else 
   {
      msg->Array[0] = GetStr(MSG_Folder);
      msg->Array[1] = GetStr(MSG_Total);
      msg->Array[2] = GetStr(MSG_Unread);
      msg->Array[3] = GetStr(MSG_New);
      msg->Array[4] = GetStr(MSG_Size);
   }

   return 0;
}
MakeHook(MA_LV_FDspFuncHook,MA_LV_FDspFunc);

///
/// MA_MakeFOFormat
//  Creates format definition for folder listview
void MA_MakeFOFormat(APTR lv)
{
   static const int defwidth[FOCOLNUM] = { -1,-1,-1,-1,-1 };
   char format[SIZE_LARGE];
   BOOL first = TRUE;
   int i;

   *format = 0;
   for (i = 0; i < FOCOLNUM; i++) if (C->FolderCols & (1<<i))
   {
      if (first) first = FALSE; else strcat(format, " BAR,");
      sprintf(&format[strlen(format)], "COL=%d W=%d", i, defwidth[i]);
      if (i) strcat(format, " P=\033r");
   }
   strcat(format, " BAR");
   set(lv, MUIA_NListtree_Format, format);
}
///

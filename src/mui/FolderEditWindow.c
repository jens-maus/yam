
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

 Superclass:  MUIC_Window
 Description: Folder edit window

***************************************************************************/

#include "FolderEditWindow_cl.h"

#include <stdlib.h>

#include <libraries/asl.h>
#include <libraries/iffparse.h>
#include <proto/dos.h>
#include <proto/muimaster.h>
#include <proto/xpkmaster.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_mainFolder.h"
#include "YAM_stringsizes.h"

#include "Locale.h"
#include "MailList.h"
#include "MUIObjects.h"

#include "mui/IdentityChooser.h"
#include "mui/Recipientstring.h"
#include "mui/SignatureChooser.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *ST_FNAME;
  Object *ST_FPATH;
  Object *NM_MAXAGE;
  Object *CY_FMODE;
  Object *CY_FTYPE;
  Object *CY_SORT[2];
  Object *CH_REVERSE[2];
  Object *CH_EXPIREUNREAD;
  Object *ST_MLPATTERN;
  Object *CY_MLIDENTITY;
  Object *ST_MLREPLYTOADDRESS;
  Object *ST_MLADDRESS;
  Object *CY_MLSIGNATURE;
  Object *CH_STATS;
  Object *CH_MLSUPPORT;
  Object *BT_AUTODETECT;
  Object *BT_OKAY;
  Object *BT_CANCEL;
  Object *ST_HELLOTEXT;
  Object *ST_BYETEXT;
  struct Folder *folder;
  BOOL folderValid;
};
*/

/* Private Functions */

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  static const char *ftypes[4];
  static const char *fmodes[5];
  static const char *sortopt[8];
  Object *ST_FNAME;
  Object *ST_FPATH;
  Object *NM_MAXAGE;
  Object *CY_FMODE;
  Object *CY_FTYPE;
  Object *CY_SORT[2];
  Object *CH_REVERSE[2];
  Object *CH_EXPIREUNREAD;
  Object *ST_MLPATTERN;
  Object *CY_MLIDENTITY;
  Object *ST_MLREPLYTOADDRESS;
  Object *ST_MLADDRESS;
  Object *CY_MLSIGNATURE;
  Object *CH_STATS;
  Object *CH_MLSUPPORT;
  Object *BT_AUTODETECT;
  Object *BT_OKAY;
  Object *BT_CANCEL;
  Object *ST_HELLOTEXT;
  Object *ST_BYETEXT;

  ENTER();

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

  if((obj = DoSuperNew(cl, obj,
    MUIA_Window_Title, tr(MSG_FO_EditFolder),
    MUIA_HelpNode,  "Windows#Foldersettings",
    MUIA_Window_ID, MAKE_ID('F','O','L','D'),
    MUIA_Window_LeftEdge, MUIV_Window_LeftEdge_Centered,
    MUIA_Window_TopEdge,  MUIV_Window_TopEdge_Centered,
    WindowContents, VGroup,
      Child, ColGroup(2), GroupFrameT(tr(MSG_FO_Properties)),
        Child, Label2(tr(MSG_CO_Name)),
        Child, ST_FNAME = MakeString(SIZE_NAME,tr(MSG_CO_Name)),
        Child, Label2(tr(MSG_Path)),
        Child, PopaslObject,
          MUIA_Popasl_Type, ASL_FileRequest,
        //MUIA_Popasl_StartHook, &FolderPathHook,
          MUIA_Popstring_String, ST_FPATH = MakeString(SIZE_PATH, ""),
          MUIA_Popstring_Button, PopButton(MUII_PopDrawer),
          ASLFR_DrawersOnly, TRUE,
        End,
        Child, Label2(tr(MSG_FO_MaxAge)),
        Child, HGroup,
          Child, NM_MAXAGE = NumericbuttonObject,
            MUIA_CycleChain,      1,
            MUIA_Numeric_Min,     0,
            MUIA_Numeric_Max,     730,
            MUIA_Numeric_Format,  tr(MSG_FO_MAXAGEFMT),
          End,
          Child, CH_EXPIREUNREAD = MakeCheck(tr(MSG_FO_EXPIREUNREAD)),
          Child, LLabel1(tr(MSG_FO_EXPIREUNREAD)),
          Child, HSpace(0),
        End,
        Child, Label1(tr(MSG_FO_FolderType)),
        Child, CY_FTYPE = MakeCycle(ftypes,tr(MSG_FO_FolderType)),
        Child, Label1(tr(MSG_FO_FolderMode)),
        Child, CY_FMODE = MakeCycle(fmodes,tr(MSG_FO_FolderMode)),
        Child, Label1(tr(MSG_FO_SortBy)),
        Child, HGroup,
          Child, CY_SORT[0] = MakeCycle(sortopt,tr(MSG_FO_SortBy)),
          Child, CH_REVERSE[0] = MakeCheck(tr(MSG_FO_Reverse)),
          Child, LLabel1(tr(MSG_FO_Reverse)),
        End,
        Child, Label1(tr(MSG_FO_ThenBy)),
        Child, HGroup,
          Child, CY_SORT[1] = MakeCycle(sortopt,tr(MSG_FO_ThenBy)),
          Child, CH_REVERSE[1] = MakeCheck(tr(MSG_FO_Reverse)),
          Child, LLabel1(tr(MSG_FO_Reverse)),
        End,
        Child, Label2(tr(MSG_FO_Welcome)),
        Child, ST_HELLOTEXT = MakeString(SIZE_INTRO,tr(MSG_FO_Welcome)),
        Child, Label2(tr(MSG_FO_Greetings)),
        Child, ST_BYETEXT = MakeString(SIZE_INTRO,tr(MSG_FO_Greetings)),
        Child, Label1(tr(MSG_FO_DSTATS)),
        Child, HGroup,
          Child, CH_STATS = MakeCheck(tr(MSG_FO_DSTATS)),
          Child, HSpace(0),
        End,
      End,
      Child, ColGroup(2), GroupFrameT(tr(MSG_FO_MLSupport)),
        Child, Label2(tr(MSG_FO_MLSUPPORT)),
        Child, HGroup,
          Child, CH_MLSUPPORT = MakeCheck(tr(MSG_FO_MLSUPPORT)),
          Child, HVSpace,
          Child, BT_AUTODETECT = MakeButton(tr(MSG_FO_AUTODETECT)),
        End,
        Child, Label2(tr(MSG_FO_TO_PATTERN)),
        Child, ST_MLPATTERN = MakeString(SIZE_PATTERN,tr(MSG_FO_TO_PATTERN)),
        Child, Label2(tr(MSG_FO_TO_ADDRESS)),
        Child, MakeAddressField(&ST_MLADDRESS, tr(MSG_FO_TO_ADDRESS), MSG_HELP_FO_ST_MLADDRESS, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
        Child, Label2(tr(MSG_FO_FROM_ADDRESS)),
        Child, CY_MLIDENTITY = IdentityChooserObject,
          MUIA_ControlChar, ShortCut(tr(MSG_FO_FROM_ADDRESS)),
          End,
        Child, Label2(tr(MSG_FO_REPLYTO_ADDRESS)),
        Child, MakeAddressField(&ST_MLREPLYTOADDRESS, tr(MSG_FO_REPLYTO_ADDRESS), MSG_HELP_FO_ST_MLREPLYTOADDRESS, ABM_CONFIG, -1, AFF_ALLOW_MULTI),
        Child, Label1(tr(MSG_WR_Signature)),
        Child, CY_MLSIGNATURE = SignatureChooserObject,
          MUIA_ControlChar, ShortCut(tr(MSG_WR_Signature)),
        End,
      End,
      Child, ColGroup(3),
        Child, BT_OKAY = MakeButton(tr(MSG_Okay)),
        Child, HSpace(0),
        Child, BT_CANCEL = MakeButton(tr(MSG_Cancel)),
      End,
    End,

    TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->ST_FNAME            = ST_FNAME;
    data->ST_FPATH            = ST_FPATH;
    data->NM_MAXAGE           = NM_MAXAGE;
    data->CY_FMODE            = CY_FMODE;
    data->CY_FTYPE            = CY_FTYPE;
    data->CY_SORT[0]          = CY_SORT[0];
    data->CY_SORT[1]          = CY_SORT[1];
    data->CH_REVERSE[0]       = CH_REVERSE[0];
    data->CH_REVERSE[1]       = CH_REVERSE[1];
    data->CH_EXPIREUNREAD     = CH_EXPIREUNREAD;
    data->ST_MLPATTERN        = ST_MLPATTERN;
    data->CY_MLIDENTITY       = CY_MLIDENTITY;
    data->ST_MLREPLYTOADDRESS = ST_MLREPLYTOADDRESS;
    data->ST_MLADDRESS        = ST_MLADDRESS;
    data->CY_MLSIGNATURE      = CY_MLSIGNATURE;
    data->CH_STATS            = CH_STATS;
    data->CH_MLSUPPORT        = CH_MLSUPPORT;
    data->BT_AUTODETECT       = BT_AUTODETECT;
    data->BT_OKAY             = BT_OKAY;
    data->BT_CANCEL           = BT_CANCEL;
    data->ST_HELLOTEXT        = ST_HELLOTEXT;
    data->ST_BYETEXT          = ST_BYETEXT;

    data->folder = (struct Folder *)GetTagData(ATTR(Folder), (ULONG)NULL, inittags(msg));

    DoMethod(G->App, OM_ADDMEMBER, obj);

    set(ST_FPATH, MUIA_String_Reject, " \";#?*|()[]<>");
    set(CH_STATS, MUIA_Disabled, C->WBAppIcon == FALSE && C->DockyIcon == FALSE);

    SetHelp(ST_FNAME,        MSG_HELP_FO_ST_FNAME       );
    SetHelp(ST_FPATH,        MSG_HELP_FO_TX_FPATH       );
    SetHelp(NM_MAXAGE,       MSG_HELP_FO_ST_MAXAGE      );
    SetHelp(CY_FMODE,        MSG_HELP_FO_CY_FMODE       );
    SetHelp(CY_FTYPE,        MSG_HELP_FO_CY_FTYPE       );
    SetHelp(CY_SORT[0],      MSG_HELP_FO_CY_SORT0       );
    SetHelp(CY_SORT[1],      MSG_HELP_FO_CY_SORT1       );
    SetHelp(CH_REVERSE[0],   MSG_HELP_FO_CH_REVERSE     );
    SetHelp(CH_REVERSE[1],   MSG_HELP_FO_CH_REVERSE     );
    SetHelp(ST_MLPATTERN,    MSG_HELP_FO_ST_MLPATTERN   );
    SetHelp(CY_MLSIGNATURE,  MSG_HELP_FO_CY_MLSIGNATURE );
    SetHelp(CH_STATS,        MSG_HELP_FO_CH_STATS       );
    SetHelp(CH_EXPIREUNREAD, MSG_HELP_FO_CH_EXPIREUNREAD);
    SetHelp(CH_MLSUPPORT,    MSG_HELP_FO_CH_MLSUPPORT   );
    SetHelp(BT_AUTODETECT,   MSG_HELP_FO_BT_AUTODETECT  );
    SetHelp(ST_HELLOTEXT,    MSG_HELP_FO_ST_HELLOTEXT   );
    SetHelp(ST_BYETEXT,      MSG_HELP_FO_ST_BYETEXT     );

    DoMethod(BT_OKAY,       MUIM_Notify, MUIA_Pressed,             FALSE, obj, 3, MUIM_Set, ATTR(Modified), TRUE);
    DoMethod(BT_CANCEL,     MUIM_Notify, MUIA_Pressed,             FALSE, MUIV_Notify_Application, 2, MUIM_YAMApplication_DisposeWindow, obj);
    DoMethod(BT_AUTODETECT, MUIM_Notify, MUIA_Pressed,             FALSE, MUIV_Notify_Application, 1, METHOD(MLAutoDetect));
    DoMethod(obj,           MUIM_Notify, MUIA_Window_CloseRequest, TRUE,  MUIV_Notify_Application, 2, MUIM_YAMApplication_DisposeWindow, obj);

    DoMethod(CH_MLSUPPORT, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj, 2, METHOD(MLSupportUpdate), MUIV_NotTriggerValue);
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(OM_GET)
OVERLOAD(OM_GET)
{
  IPTR *store = ((struct opGet *)msg)->opg_Storage;

  switch(((struct opGet *)msg)->opg_AttrID)
  {
    case ATTR(Folder):
    {
      DoMethod(obj, METHOD(GUIToFolder), store);
      return TRUE;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///
/// OVERLOAD(OM_SET)
OVERLOAD(OM_SET)
{
  GETDATA;
  struct TagItem *tags = inittags(msg), *tag;

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Folder):
      {
        data->folder = (struct Folder *)tag->ti_Data;

        if(data->folder != NULL)
          DoMethod(obj, METHOD(FolderToGUI), data->folder);

        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case MUIA_Window_Open:
      {
        // make the folder name object the active one upon opening the
        // window in case the folder to be edited is a new one
        if(tag->ti_Data != FALSE && data->folderValid == FALSE)
          set(obj, MUIA_Window_ActiveObject, data->ST_FNAME);
      }
      break;
    }
  }

  return DoSuperMethodA(cl, obj, msg);
}

///

/* Public Methods */
/// DECLARE(FolderToGUI)
DECLARE(FolderToGUI) // struct Folder *folder
{
  GETDATA;
  struct Folder *folder = msg->folder;
  BOOL isdefault = isDefaultFolder(folder);
  static const int type2cycle[9] = { FT_CUSTOM, FT_CUSTOM, FT_INCOMING, FT_INCOMING, FT_OUTGOING, -1, FT_INCOMING, FT_OUTGOING, FT_CUSTOM };
  int i;

  ENTER();

  set(data->ST_FNAME,  MUIA_String_Contents, folder->Name);
  set(data->ST_FPATH,  MUIA_String_Contents, folder->Path);
  set(data->NM_MAXAGE, MUIA_Numeric_Value,   folder->MaxAge);

  xset(data->CH_EXPIREUNREAD, MUIA_Selected, folder->ExpireUnread,
                              MUIA_Disabled, isTrashFolder(folder) || isSpamFolder(folder) || folder->MaxAge == 0);

  xset(data->CY_FTYPE, MUIA_Cycle_Active, type2cycle[folder->Type],
                       MUIA_Disabled,     isdefault);

  xset(data->CY_FMODE, MUIA_Cycle_Active, folder->Mode,
                       MUIA_Disabled,     isdefault);

  for(i = 0; i < 2; i++)
  {
    set(data->CY_SORT[i], MUIA_Cycle_Active, (folder->Sort[i] < 0 ? -folder->Sort[i] : folder->Sort[i])-1);
    set(data->CH_REVERSE[i], MUIA_Selected, folder->Sort[i] < 0);
  }

  set(data->CH_STATS,       MUIA_Selected, folder->Stats);
  set(data->BT_AUTODETECT,  MUIA_Disabled, !folder->MLSupport || isdefault);
  set(data->ST_HELLOTEXT,   MUIA_String_Contents, folder->WriteIntro);
  set(data->ST_BYETEXT,     MUIA_String_Contents, folder->WriteGreetings);

  // for ML-Support
  xset(data->CH_MLSUPPORT,
       MUIA_Selected, isdefault ? FALSE : folder->MLSupport,
       MUIA_Disabled, isdefault);

  xset(data->ST_MLADDRESS,
       MUIA_String_Contents, folder->MLAddress,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(data->ST_MLPATTERN,
       MUIA_String_Contents, folder->MLPattern,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(data->ST_MLREPLYTOADDRESS,
       MUIA_String_Contents, folder->MLReplyToAddress,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(data->CY_MLSIGNATURE,
       MUIA_SignatureChooser_Signature, folder->MLSignature,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  xset(data->CY_MLIDENTITY,
       MUIA_IdentityChooser_Identity, folder->MLIdentity,
       MUIA_Disabled, !folder->MLSupport || isdefault);

  if(!isTrashFolder(folder) && !isSpamFolder(folder))
  {
    // disable the "also unread" check mark whenever the max age is set to 0 days,
    // but only for folders other than the Trash and Spam folders
    DoMethod(data->NM_MAXAGE, MUIM_Notify, MUIA_Numeric_Value, MUIV_EveryTime, data->CH_EXPIREUNREAD, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
  }

  // we make sure the window is at the front if it is already open
  if(xget(obj, MUIA_Window_Open) == TRUE)
    DoMethod(obj, MUIM_Window_ToFront);

  RETURN(0);
  return 0;
}

///
/// DECLARE(GUIToFolder)
DECLARE(GUIToFolder) // struct Folder *folder
{
  GETDATA;
  struct Folder *folder = msg->folder;
  static const int cycle2type[3] = { FT_CUSTOM, FT_CUSTOMSENT, FT_CUSTOMMIXED };
  int i;

  ENTER();

  GetMUIString(folder->Name, data->ST_FNAME, sizeof(folder->Name));
  GetMUIString(folder->Path, data->ST_FPATH, sizeof(folder->Path));

  // we have to correct the folder path because we shouldn't allow a last / in the
  // path
  if(folder->Path[strlen(folder->Path) - 1] == '/')
    folder->Path[strlen(folder->Path) - 1] = '\0';

  // set up the full path to the folder
  if(strchr(folder->Path, ':') != NULL)
  {
    // the path is an absolute path already
    strlcpy(folder->Fullpath, folder->Path, sizeof(folder->Fullpath));
  }
  else
  {
    // concatenate the default mail dir and the folder's relative path to an absolute path
    strlcpy(folder->Fullpath, G->MA_MailDir, sizeof(folder->Fullpath));
    AddPart(folder->Fullpath, folder->Path, sizeof(folder->Fullpath));
  }

  folder->MaxAge = GetMUINumer(data->NM_MAXAGE);
  if(!isDefaultFolder(folder))
  {
    folder->Type = cycle2type[GetMUICycle(data->CY_FTYPE)];
    folder->Mode = GetMUICycle(data->CY_FMODE);
  }

  for(i = 0; i < 2; i++)
  {
    folder->Sort[i] = GetMUICycle(data->CY_SORT[i]) + 1;
    if (GetMUICheck(data->CH_REVERSE[i]))
      folder->Sort[i] = -folder->Sort[i];
  }

  folder->ExpireUnread = GetMUICheck(data->CH_EXPIREUNREAD);
  folder->Stats = GetMUICheck(data->CH_STATS);
  folder->MLSupport = GetMUICheck(data->CH_MLSUPPORT);

  GetMUIString(folder->WriteIntro, data->ST_HELLOTEXT, sizeof(folder->WriteIntro));
  GetMUIString(folder->WriteGreetings, data->ST_BYETEXT, sizeof(folder->WriteGreetings));

  GetMUIString(folder->MLPattern, data->ST_MLPATTERN, sizeof(folder->MLPattern));

  // resolve the addresses first, in case someone entered an alias
  DoMethod(data->ST_MLADDRESS, MUIM_Recipientstring_Resolve, MUIF_NONE);
  DoMethod(data->ST_MLREPLYTOADDRESS, MUIM_Recipientstring_Resolve, MUIF_NONE);

  GetMUIString(folder->MLAddress, data->ST_MLADDRESS, sizeof(folder->MLAddress));
  GetMUIString(folder->MLReplyToAddress, data->ST_MLREPLYTOADDRESS, sizeof(folder->MLReplyToAddress));

  folder->MLSignature = (struct SignatureNode *)xget(data->CY_MLSIGNATURE, MUIA_SignatureChooser_Signature);
  folder->MLIdentity = (struct UserIdentityNode *)xget(data->CY_MLIDENTITY, MUIA_IdentityChooser_Identity);

  RETURN(0);
  return 0;
}

///
/// DECLARE(MLSupportUpdate)
DECLARE(MLSupportUpdate) // ULONG disabled
{
  GETDATA;

  ENTER();

  DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, msg->disabled,
    data->BT_AUTODETECT,
    data->ST_MLPATTERN,
    data->ST_MLADDRESS,
    data->CY_MLIDENTITY,
    data->ST_MLREPLYTOADDRESS,
    data->CY_MLSIGNATURE,
    NULL);

  RETURN(0);
  return 0;
}

///
/// DECLARE(MLAutoDetect)
DECLARE(MLAutoDetect)
{
  GETDATA;

  ENTER();

  if(data->folderValid == TRUE)
  {
    LockMailListShared(data->folder->messages);

    if(IsMailListEmpty(data->folder->messages) == FALSE)
    {
      #define SCANMSGS  5
      struct MailNode *mnode;
      char *toPattern;
      char *toAddress;
      char *res = NULL;
      BOOL takePattern = TRUE;
      BOOL takeAddress = TRUE;
      int i;
      BOOL success;

      mnode = FirstMailNode(data->folder->messages);
      toPattern = mnode->mail->To.Address;
      toAddress = mnode->mail->To.Address;

      i = 0;
      success = TRUE;
      ForEachMailNode(data->folder->messages, mnode)
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

      UnlockMailList(data->folder->messages);

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
          setstring(data->ST_MLPATTERN, takePattern == TRUE && toPattern[0] != '\0' ? toPattern : tr(MSG_FO_NOTRECOGNIZED));
          setstring(data->ST_MLADDRESS, takeAddress == TRUE ? toAddress : tr(MSG_FO_NOTRECOGNIZED));
        }
      }

      // free all resources
      free(res);

      SWSSearch(NULL, NULL);
    }
  }

  RETURN(0);
  return 0;
}

///

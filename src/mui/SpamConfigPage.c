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
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.   See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 YAM Official Support Site : http://www.yam.ch
 YAM OpenSource project     : http://sourceforge.net/projects/yamos/

 $Id$

 Superclass:  MUIC_ConfigPage
 Description: "Spam" configuration page

***************************************************************************/

#include "SpamConfigPage_cl.h"

#include <proto/muimaster.h>

#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>

#include "YAM.h"
#include "YAM_error.h"

#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "Requesters.h"

#include "mui/ConfigPage.h"
#include "mui/FilterChooser.h"
#include "mui/MainWindowToolbar.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *CH_SPAMFILTERENABLED;
  Object *TX_SPAMBADCOUNT;
  Object *TX_SPAMGOODCOUNT;
  Object *BT_SPAMRESETTRAININGDATA;
  Object *BT_OPTIMIZETRAININGDATA;
  Object *CH_SPAMFILTERFORNEWMAIL;
  Object *CH_SPAMABOOKISWHITELIST;
  Object *CH_SPAMMARKONMOVE;
  Object *CH_SPAMMARKASREAD;
  Object *CH_MOVEHAMTOINCOMING;
  Object *CH_FILTERHAM;
  Object *CH_SPAM_TRUSTEXTERNALFILTER;
  Object *CY_SPAM_EXTERNALFILTER;
};
*/

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  Object *CH_SPAMFILTERENABLED;
  Object *TX_SPAMBADCOUNT;
  Object *TX_SPAMGOODCOUNT;
  Object *BT_SPAMRESETTRAININGDATA;
  Object *BT_OPTIMIZETRAININGDATA;
  Object *CH_SPAMFILTERFORNEWMAIL;
  Object *CH_SPAMABOOKISWHITELIST;
  Object *CH_SPAMMARKONMOVE;
  Object *CH_SPAMMARKASREAD;
  Object *CH_MOVEHAMTOINCOMING;
  Object *CH_FILTERHAM;
  Object *CH_SPAM_TRUSTEXTERNALFILTER;
  Object *CY_SPAM_EXTERNALFILTER;

  ENTER();

  if((obj = DoSuperNew(cl, obj,
    MUIA_HelpNode, "Configuration#Spam",
    MUIA_ConfigPage_Page, cp_Spam,
    MUIA_ConfigPage_Contents, VGroup,
      Child, VGroup, GroupFrameT(tr(MSG_CO_SPAMFILTER)),
        Child, ColGroup(4),
          Child, CH_SPAMFILTERENABLED = MakeCheck(tr(MSG_CO_SPAM_FILTERENABLED)),
          Child, LLabel(tr(MSG_CO_SPAM_FILTERENABLED)),
          Child, HSpace(0),
          Child, HSpace(0),

          Child, HSpace(0),
          Child, Label2(tr(MSG_CO_SPAM_BADCOUNT)),
          Child, TX_SPAMBADCOUNT = TextObject,
            TextFrame,
            MUIA_Background,    MUII_TextBack,
            MUIA_Text_SetMin,   TRUE,
            MUIA_Text_PreParse, "\033r",
          End,
          Child, HSpace(0),

          Child, HSpace(0),
          Child, Label2(tr(MSG_CO_SPAM_GOODCOUNT)),
          Child, TX_SPAMGOODCOUNT = TextObject,
            TextFrame,
            MUIA_Background,    MUII_TextBack,
            MUIA_Text_SetMin,   TRUE,
            MUIA_Text_PreParse, "\033r",
          End,
          Child, HSpace(0),

          Child, HSpace(0),
          Child, BT_SPAMRESETTRAININGDATA = MakeButton(tr(MSG_CO_SPAM_RESETTRAININGDATA)),
          Child, BT_OPTIMIZETRAININGDATA = MakeButton(tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA)),
          Child, HSpace(0),
        End,
      End,

      Child, VGroup, GroupFrameT(tr(MSG_CO_SPAM_RECOGNITION)),
        Child, MakeCheckGroup(&CH_SPAMFILTERFORNEWMAIL, tr(MSG_CO_SPAM_FILTERFORNEWMAIL)),
        Child, MakeCheckGroup(&CH_SPAMABOOKISWHITELIST, tr(MSG_CO_SPAM_ADDRESSBOOKISWHITELIST)),
        Child, MakeCheckGroup(&CH_SPAMMARKONMOVE, tr(MSG_CO_SPAM_MARKONMOVE)),
        Child, MakeCheckGroup(&CH_SPAMMARKASREAD, tr(MSG_CO_SPAM_MARK_AS_READ)),
        Child, MakeCheckGroup(&CH_MOVEHAMTOINCOMING, tr(MSG_CO_MOVE_HAM_TO_INCOMING)),
        Child, ColGroup(2),
          Child, HSpace(5),
          Child, MakeCheckGroup(&CH_FILTERHAM, tr(MSG_CO_FILTER_HAM)),
        End,
        Child, HGroup,
          Child, MakeCheckGroup(&CH_SPAM_TRUSTEXTERNALFILTER, tr(MSG_SPAM_TRUSTHEADERLINES)),
          Child, CY_SPAM_EXTERNALFILTER = FilterChooserObject,
          End,
          Child, HSpace(0),
        End,
      End,
    End,
  TAG_MORE, inittags(msg))) != NULL)
  {
    GETDATA;

    data->CH_SPAMFILTERENABLED =        CH_SPAMFILTERENABLED;
    data->TX_SPAMBADCOUNT =             TX_SPAMBADCOUNT;
    data->TX_SPAMGOODCOUNT =            TX_SPAMGOODCOUNT;
    data->BT_SPAMRESETTRAININGDATA =    BT_SPAMRESETTRAININGDATA;
    data->BT_OPTIMIZETRAININGDATA =     BT_OPTIMIZETRAININGDATA;
    data->CH_SPAMFILTERFORNEWMAIL =     CH_SPAMFILTERFORNEWMAIL;
    data->CH_SPAMABOOKISWHITELIST =     CH_SPAMABOOKISWHITELIST;
    data->CH_SPAMMARKONMOVE =           CH_SPAMMARKONMOVE;
    data->CH_SPAMMARKASREAD =           CH_SPAMMARKASREAD;
    data->CH_MOVEHAMTOINCOMING =        CH_MOVEHAMTOINCOMING;
    data->CH_FILTERHAM =                CH_FILTERHAM;
    data->CH_SPAM_TRUSTEXTERNALFILTER = CH_SPAM_TRUSTEXTERNALFILTER;
    data->CY_SPAM_EXTERNALFILTER =      CY_SPAM_EXTERNALFILTER;

    DoMethod(obj, MUIM_MultiSet, MUIA_Disabled, TRUE, BT_SPAMRESETTRAININGDATA,
                                                      BT_OPTIMIZETRAININGDATA,
                                                      CH_SPAMFILTERFORNEWMAIL,
                                                      CH_SPAMABOOKISWHITELIST,
                                                      CH_SPAMMARKONMOVE,
                                                      CH_SPAMMARKASREAD,
                                                      CH_MOVEHAMTOINCOMING,
                                                      CH_FILTERHAM,
                                                      CH_SPAM_TRUSTEXTERNALFILTER,
                                                      CY_SPAM_EXTERNALFILTER,
                                                      NULL);

    SetHelp(CH_SPAMFILTERENABLED,     MSG_HELP_CH_SPAMFILTERENABLED);
    SetHelp(TX_SPAMBADCOUNT,          MSG_HELP_TX_SPAMBADCOUNT);
    SetHelp(TX_SPAMGOODCOUNT,         MSG_HELP_TX_SPAMGOODCOUNT);
    SetHelp(BT_SPAMRESETTRAININGDATA, MSG_HELP_BT_SPAMRESETTRAININGDATA);
    SetHelp(BT_OPTIMIZETRAININGDATA,  MSG_HELP_BT_OPTIMIZE_TRAININGDATA);
    SetHelp(CH_SPAMFILTERFORNEWMAIL,  MSG_HELP_CH_SPAMFILTERFORNEWMAIL);
    SetHelp(CH_SPAMABOOKISWHITELIST,  MSG_HELP_CH_SPAMABOOKISWHITELIST);
    SetHelp(CH_SPAMMARKONMOVE,        MSG_HELP_CH_SPAMMARKONMOVE);
    SetHelp(CH_SPAMMARKASREAD,        MSG_HELP_CH_SPAMMARKASREAD);
    SetHelp(CH_MOVEHAMTOINCOMING,     MSG_HELP_CH_MOVE_HAM_TO_INCOMING);
    SetHelp(CH_FILTERHAM,             MSG_HELP_CH_FILTER_HAM);

    DoMethod(CH_SPAMFILTERENABLED,        MUIM_Notify, MUIA_Selected, MUIV_EveryTime, obj,                    2, METHOD(ToggleSpamFilter), MUIV_TriggerValue);
    DoMethod(CH_MOVEHAMTOINCOMING,        MUIM_Notify, MUIA_Selected, MUIV_EveryTime, CH_FILTERHAM,           3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(CH_SPAM_TRUSTEXTERNALFILTER, MUIM_Notify, MUIA_Selected, MUIV_EveryTime, CY_SPAM_EXTERNALFILTER, 3, MUIM_Set, MUIA_Disabled, MUIV_NotTriggerValue);
    DoMethod(BT_SPAMRESETTRAININGDATA,    MUIM_Notify, MUIA_Pressed,  FALSE,          obj,                    1, METHOD(ResetSpamTrainingData));
    DoMethod(BT_OPTIMIZETRAININGDATA,     MUIM_Notify, MUIA_Pressed,  FALSE,          obj,                    1, METHOD(OptimizeSpamTrainingData));
  }

  RETURN((IPTR)obj);
  return (IPTR)obj;
}

///
/// OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
OVERLOAD(MUIM_ConfigPage_ConfigToGUI)
{
  GETDATA;

  ENTER();

  setcheckmark(data->CH_SPAMFILTERENABLED, CE->SpamFilterEnabled);
  setcheckmark(data->CH_SPAMFILTERFORNEWMAIL, CE->SpamFilterForNewMail);
  setcheckmark(data->CH_SPAMMARKONMOVE, CE->SpamMarkOnMove);
  setcheckmark(data->CH_SPAMMARKASREAD, CE->SpamMarkAsRead);
  setcheckmark(data->CH_SPAMABOOKISWHITELIST, CE->SpamAddressBookIsWhiteList);
  setcheckmark(data->CH_MOVEHAMTOINCOMING, CE->MoveHamToIncoming);
  setcheckmark(data->CH_FILTERHAM, CE->FilterHam);
  setcheckmark(data->CH_SPAM_TRUSTEXTERNALFILTER, CE->SpamTrustExternalFilter);
  set(data->CY_SPAM_EXTERNALFILTER, MUIA_FilterChooser_Filter, CE->SpamExternalFilter);

  DoMethod(obj, METHOD(UpdateStats));

  RETURN(0);
  return 0;
}

///
/// OVERLOAD(MUIM_ConfigPage_GUIToConfig)
OVERLOAD(MUIM_ConfigPage_GUIToConfig)
{
  GETDATA;

  ENTER();

  CE->SpamFilterEnabled = GetMUICheck(data->CH_SPAMFILTERENABLED);
  CE->SpamFilterForNewMail = GetMUICheck(data->CH_SPAMFILTERFORNEWMAIL);
  CE->SpamMarkOnMove = GetMUICheck(data->CH_SPAMMARKONMOVE);
  CE->SpamMarkAsRead = GetMUICheck(data->CH_SPAMMARKASREAD);
  CE->SpamAddressBookIsWhiteList = GetMUICheck(data->CH_SPAMABOOKISWHITELIST);
  CE->MoveHamToIncoming = GetMUICheck(data->CH_MOVEHAMTOINCOMING);
  CE->FilterHam = GetMUICheck(data->CH_FILTERHAM);
  CE->SpamTrustExternalFilter = GetMUICheck(data->CH_SPAM_TRUSTEXTERNALFILTER);
  strlcpy(CE->SpamExternalFilter, (char *)xget(data->CY_SPAM_EXTERNALFILTER, MUIA_FilterChooser_Filter), sizeof(CE->SpamExternalFilter));

  if(C->SpamFilterEnabled == TRUE && CE->SpamFilterEnabled == FALSE)
  {
    LONG mask;

    // raise a CheckboxRequest and ask the user which
    // operations he want to performed while disabling the
    // SPAM filter.
    mask = CheckboxRequest(obj, NULL, 3, tr(MSG_CO_SPAM_DISABLEFILTERASK),
                                         tr(MSG_CO_SPAM_RESETTDATA),
                                         tr(MSG_CO_SPAM_RESETMAILFLAGS),
                                         tr(MSG_CO_SPAM_DELETESPAMFOLDER));

    SHOWVALUE(DBF_CONFIG, mask);
    SHOWVALUE(DBF_ALWAYS, mask);
    // check if the user canceled the requester
    if(mask >= 0)
    {
      // reset training data
      if(mask & (1 << 0))
      {
        D(DBF_CONFIG, "resetting spam training data");
        BayesFilterResetTrainingData();
      }

      // reset spam state of all mails
      if(mask & (1 << 1))
      {
        struct FolderNode *fnode;

        D(DBF_CONFIG, "resetting spam state of all mails");

        LockFolderListShared(G->folders);

        ForEachFolderNode(G->folders, fnode)
        {
          struct Folder *folder = fnode->folder;

          if(!isGroupFolder(folder))
          {
            struct MailList *mlist;

            if((mlist = MA_CreateFullList(folder, FALSE)) != NULL)
            {
              struct MailNode *mnode;

              // clear all possible spam/ham flags from each mail
              ForEachMailNode(mlist, mnode)
              {
                struct Mail *mail = mnode->mail;

                if(mail != NULL)
                  MA_ChangeMailStatus(mail, SFLAG_NONE, SFLAG_USERSPAM|SFLAG_AUTOSPAM|SFLAG_HAM);
              }

              DeleteMailList(mlist);
            }
          }
        }

        UnlockFolderList(G->folders);
      }

      // delete spam folder
      if(mask & (1 << 2))
      {
        struct Folder *spamFolder;

        D(DBF_CONFIG, "deleting spam folder");

        // first locate the spam folder
        if((spamFolder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL)
        {
          // delete the folder on disk
          DeleteMailDir(spamFolder->Fullpath, FALSE);

          // remove all mails from our internal list
          ClearFolderMails(spamFolder, TRUE);

          // remove the folder from the folder list
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, spamFolder->Treenode, MUIF_NONE);

          // and finally save the modified tree to the folder config now
          FO_SaveTree();

          // update the statistics in case the spam folder contained new or unread mails
          DisplayStatistics(NULL, TRUE);

          // remove and delete the folder from our list
          LockFolderList(G->folders);
          RemoveFolder(G->folders, spamFolder);
          UnlockFolderList(G->folders);

          FO_FreeFolder(spamFolder);
        }
      }
      else
      {
        // the spam folder should be kept, but it must be "degraded" to a normal folder
        // to make it deleteable later
        struct Folder *spamFolder;

        // first locate the spam folder
        if((spamFolder = FO_GetFolderByType(FT_SPAM, NULL)) != NULL)
        {
          if(spamFolder->imageObject != NULL)
          {
            // we make sure that the NList also doesn't use the image in future anymore
            DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, spamFolder->ImageIndex, MUIF_NONE);
            spamFolder->imageObject = NULL;
            // we don't need to dispose the image, because it is one of the standard images and not
            // a custom image of the user.
          }

          // degrade it to a custom folder without folder image
          spamFolder->Type = FT_CUSTOM;
          spamFolder->ImageIndex = -1;

          // finally save the modified configuration
          FO_SaveConfig(spamFolder);

          // update the statistics in case the spam folder contained new or unread mails
          DisplayStatistics(spamFolder, TRUE);
        }
      }

      // update the toolbar to the new settings
      if(G->MA->GUI.TO_TOOLBAR != NULL)
        DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);
    }
    else
    {
      // the user canceled the requester so lets set the Spam filter
      // back online
      CE->SpamFilterEnabled = TRUE;
    }
  }
  else if(C->SpamFilterEnabled == FALSE && CE->SpamFilterEnabled == TRUE)
  {
    // the spam filter has been enabled, now try to create the mandatory spam folder
    char spamPath[SIZE_PATHFILE];
    BOOL createSpamFolder;
    enum FType type;

    if(ObtainFileInfo(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FI_TYPE, &type) == TRUE && type == FIT_NONEXIST)
    {
      // no directory named "spam" exists, so let's create it
      createSpamFolder = TRUE;
    }
    else
    {
      ULONG result;

      // the directory "spam" already exists, but it is not the standard spam folder
      // let the user decide what to do
      result = MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_ER_SPAMDIR_EXISTS_ANSWERS), tr(MSG_ER_SPAMDIR_EXISTS));
      switch(result)
      {
        default:
        case 0:
        {
          // the user has chosen to disable the spam filter, so we do it
          // or the requester was cancelled
          CE->SpamFilterEnabled = FALSE;
          createSpamFolder = FALSE;
        }
        break;

        case 1:
        {
          // delete everything in the folder, the directory itself can be kept
          DeleteMailDir(CreateFilename(FolderName[FT_SPAM], spamPath, sizeof(spamPath)), FALSE);
          createSpamFolder = TRUE;
        }
        break;

        case 2:
        {
          // keep the folder contents
          createSpamFolder = TRUE;
        }
        break;
      }
    }

    if(createSpamFolder == TRUE)
    {
      struct Folder *spamFolder;

      // if a folder named "spam" already exists, but a new spam folder should be
      // created, we need to remove the old folder from the tree view first
      if((spamFolder = FO_GetFolderByPath((STRPTR)FolderName[FT_SPAM], NULL)) != NULL)
      {
        // remove the folder from the folder list
        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Remove, MUIV_NListtree_Remove_ListNode_Root, spamFolder->Treenode, MUIF_NONE);
        spamFolder->Treenode = NULL;

        if(spamFolder->imageObject != NULL)
        {
          // we make sure that the NList also doesn't use the image in future anymore
          DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NList_UseImage, NULL, spamFolder->ImageIndex, MUIF_NONE);
          spamFolder->imageObject = NULL;
        }
      }

      // try to create the folder and save the new folder tree
      if(FO_CreateFolder(FT_SPAM, FolderName[FT_SPAM], tr(MSG_MA_SPAM)) == FALSE || FO_SaveTree() == FALSE)
      {
        // something failed, so we disable the spam filter again
        ER_NewError(tr(MSG_CO_ER_CANNOT_CREATE_SPAMFOLDER));
        CE->SpamFilterEnabled = FALSE;
      }
      else
      {
        // move the new spam folder after the trash folder
        struct Folder *this = FO_GetFolderByType(FT_SPAM, NULL);
        struct Folder *prev = FO_GetFolderByType(FT_TRASH, NULL);

        DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Move, MUIV_NListtree_Move_OldListNode_Root, this->Treenode, MUIV_NListtree_Move_NewListNode_Root, prev->Treenode, MUIF_NONE);

        // update the toolbar to the new settings
        if(G->MA->GUI.TO_TOOLBAR != NULL)
          DoMethod(G->MA->GUI.TO_TOOLBAR, MUIM_MainWindowToolbar_UpdateSpamControls);
      }
    }
  }

  if(CE->SpamFilterEnabled == TRUE && CE->SpamMarkAsRead == TRUE)
  {
    ULONG numberClassified = BayesFilterNumberOfHamClassifiedMails() + BayesFilterNumberOfSpamClassifiedMails();

    if(numberClassified < 100)
    {
      // Less than 100 mails have been classified so far.
      // Better ask the user if new spam mails really should be mark as "read", because
      // the filter is most probably not trained well enough an non-spam mails may be
      // marked as spam and read unnoticed.
      if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq), tr(MSG_ER_SPAM_NOT_ENOUGH_CLASSIFIED_MAILS), numberClassified))
      {
        CE->SpamMarkAsRead = FALSE;
      }
    }
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(UpdateStats)
DECLARE(UpdateStats)
{
  GETDATA;
  char buf[SIZE_DEFAULT];

  ENTER();

  snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfHamClassifiedMails(), BayesFilterNumberOfHamClassifiedWords());
  set(data->TX_SPAMGOODCOUNT, MUIA_Text_Contents, buf);
  snprintf(buf, sizeof(buf), tr(MSG_CO_SPAM_STATISTICS), BayesFilterNumberOfSpamClassifiedMails(), BayesFilterNumberOfSpamClassifiedWords());
  set(data->TX_SPAMBADCOUNT, MUIA_Text_Contents, buf);

  RETURN(0);
  return 0;
}

///
/// DECLARE(ToggleSpamFilter)
// enable/disable all spam filter relevant GUI elements according to the
// current spam filter settings
DECLARE(ToggleSpamFilter) // ULONG active
{
  GETDATA;

  ENTER();

  if(msg->active == TRUE)
  {
    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, FALSE, data->BT_SPAMRESETTRAININGDATA,
                                                          data->BT_OPTIMIZETRAININGDATA,
                                                          data->CH_SPAMFILTERFORNEWMAIL,
                                                          data->CH_SPAMABOOKISWHITELIST,
                                                          data->CH_SPAMMARKONMOVE,
                                                          data->CH_SPAMMARKASREAD,
                                                          data->CH_MOVEHAMTOINCOMING,
                                                          data->CH_SPAM_TRUSTEXTERNALFILTER,
                                                          NULL);
    set(data->CH_FILTERHAM, MUIA_Disabled, xget(data->CH_MOVEHAMTOINCOMING, MUIA_Selected) == FALSE);
    set(data->CY_SPAM_EXTERNALFILTER, MUIA_Disabled, xget(data->CH_SPAM_TRUSTEXTERNALFILTER, MUIA_Selected) == FALSE);
  }
  else
  {
    // disable all spam filter controls
    DoMethod(G->App, MUIM_MultiSet, MUIA_Disabled, TRUE, data->BT_SPAMRESETTRAININGDATA,
                                                         data->BT_OPTIMIZETRAININGDATA,
                                                         data->CH_SPAMFILTERFORNEWMAIL,
                                                         data->CH_SPAMABOOKISWHITELIST,
                                                         data->CH_SPAMMARKONMOVE,
                                                         data->CH_SPAMMARKASREAD,
                                                         data->CH_MOVEHAMTOINCOMING,
                                                         data->CH_FILTERHAM,
                                                         data->CH_SPAM_TRUSTEXTERNALFILTER,
                                                         data->CY_SPAM_EXTERNALFILTER,
                                                         NULL);
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(ResetSpamTrainingData)
// resets the spam training data
DECLARE(ResetSpamTrainingData)
{
  ENTER();

  if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_RESETTRAININGDATAASK)) != 0)
  {
    BayesFilterResetTrainingData();
    DoMethod(obj, METHOD(UpdateStats));
  }

  RETURN(0);
  return 0;
}

///
/// DECLARE(OptimizeSpamTrainingData)
// optimizes the spam training data
DECLARE(OptimizeSpamTrainingData)
{
  ENTER();

  if(MUI_Request(_app(obj), _win(obj), MUIF_NONE, NULL, tr(MSG_YesNoReq2), tr(MSG_CO_SPAM_OPTIMIZE_TRAININGDATA_ASK)) != 0)
  {
    set(_app(obj), MUIA_Application_Sleep, TRUE);
    BayesFilterOptimizeTrainingData();
    set(_app(obj), MUIA_Application_Sleep, FALSE);

    DoMethod(obj, METHOD(UpdateStats));
  }

  RETURN(0);
  return 0;
}

///

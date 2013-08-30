
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
 Description: Show the Birthday Requester

***************************************************************************/

#include "BirthdayRequestWindow_cl.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <proto/muimaster.h>
#include <libraries/iffparse.h>

#include "YAM.h"
#include "YAM_userlist.h"
#include "YAM_utilities.h"

#include "FileInfo.h"
#include "Locale.h"
#include "MUIObjects.h"

#include "mui/WriteWindow.h"
#include "mui/YAMApplication.h"

#include "Debug.h"

/* CLASSDATA
struct Data
{
  Object *skipTodayCheckbox;

  char screenTitle[SIZE_DEFAULT];
};
*/

#define BIRTHDAYCHECKFILE "PROGDIR:.birthdaycheck"

/* Private Functions */
/// CheckBirthdayCheckFile
// Load the .birthdaycheck file and return TRUE if the birthday requester
// for the current user and alias should be displayed again.
static BOOL CheckBirthdayCheckFile(const char *alias)
{
  struct User *user;
  char *userName = NULL;
  BOOL result = TRUE;

  ENTER();

  // without a user and user name it makes no sense to search for the user
  if((user = US_GetCurrentUser()) != NULL && (userName = user->Name) != NULL)
  {
    FILE *fh;
    char todayDateString[64];
    BOOL userFound = FALSE;
    size_t buflen = 0;

    // get the current date
    DateStamp2String(todayDateString, sizeof(todayDateString), NULL, DSS_DATE, TZC_NONE);

    if((fh = fopen(BIRTHDAYCHECKFILE, "r")) != NULL)
    {
      char *buf = NULL;
      size_t size = 0;

      setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);

      // check if we really work on a YAM BirthdayCheck file
      if(getline(&buf, &buflen, fh) >= 3 && strnicmp(buf, "YBC", 3) == 0)
      {
        // read in all lines
        while(GetLine(&buf, &size, fh) > 0)
        {
          char *ptr;
          char *value;

          SHOWSTRING(DBF_ALWAYS, buf);
          if((value = strchr(buf, '=')) != NULL && value != buf)
          {
            // skip spaces and equal signs backwards
            ptr = value;
            while(ptr != buf && (isspace(*ptr) || *ptr == '='))
            {
              *ptr-- = '\0';
            }
            // skip spaces
            ptr = value;
            while(*++ptr != '\0' && isspace(*ptr));
            value = ptr;

            if(*buf != '\0' && *value != '\0')
            {
              if(stricmp(buf, "DATE") == 0)
              {
                // if the date is not today we bail out here
                if(stricmp(value, todayDateString) != 0)
                {
                  break;
                }
              }
              else if(stricmp(buf, "USER") == 0)
              {
                if(stricmp(value, userName) == 0)
                {
                  userFound = TRUE;
                }
                else
                {
                  userFound = FALSE;
                }
              }
              else if(stricmp(buf, "ALIAS") == 0)
              {
                if(stricmp(value, alias) == 0)
                {
                  if(userFound == TRUE)
                  {
                    // alias found so we change the result and bail out
                    result = FALSE;
                    break;
                  }
                }
              }
            }
          }
        }
      }

      fclose(fh);

      free(buf);
    }
  }

  RETURN(result);
  return result;
}

///
/// SaveBirthdayCheckFile
// Load the .birthdaycheck file and compare the date in the file and the
// system date. If both are equal then append the user and alias to the file.
// If the date is not equal or the file doesn't exists then create a new file
// with a header line and the current system date and add the user and alias.
static void SaveBirthdayCheckFile(const char *alias)
{
  struct User *user;

  ENTER();

  // without a user and user name it makes no sense to search for the user
  if((user = US_GetCurrentUser()) != NULL && user->Name != NULL)
  {
    char *buf;
    FILE *fh = NULL;
    char todayDateString[64];
    BOOL writeNewFile = FALSE;
    BOOL appendToFile = FALSE;

    // get the current date
    DateStamp2String(todayDateString, sizeof(todayDateString), NULL, DSS_DATE, TZC_NONE);

    if((buf = FileToBuffer(BIRTHDAYCHECKFILE)) != NULL)
    {
      // check if we really work on a YAM BirthdayCheck file
      if(strnicmp(buf, "YBC", 3) == 0)
      {
        char *ptr;

        if((ptr = strchr(buf, '\n')) != NULL)
        {
          // skip to the first digit
          while(*++ptr != '\0' && !isdigit(*ptr));

          // now we compare the date
          if(strnicmp(ptr, todayDateString, strlen(todayDateString)) == 0)
          {
            // if the date is equal then we open the file in append mode
            appendToFile = TRUE;
          }
          else
          {
            // if the date is not equal so we write a new file
            writeNewFile = TRUE;
          }
        }
      }

      free(buf);
    }
    else
    {
      if(FileExists(BIRTHDAYCHECKFILE) == FALSE)
      {
        // the file does not exists so we write a new file
        writeNewFile = TRUE;
      }
      else
      {
        E(DBF_ALWAYS, "File '%s' exists but could not read in the buffer!", BIRTHDAYCHECKFILE);
      }
    }

    if(writeNewFile == TRUE || appendToFile == TRUE)
    {
      if((fh = fopen(BIRTHDAYCHECKFILE, appendToFile ? "a" : "w")) != NULL)
      {
        setvbuf(fh, NULL, _IOFBF, SIZE_FILEBUF);
        if(writeNewFile == TRUE)
        {
          fprintf(fh, "%s\n", "YBC1 - YAM BirthdayCheck");
          fprintf(fh, "DATE = %s\n", todayDateString);
        }
      }
    }

    // if the file is openend succesfully in the appropriate
    // mode we write the user and alias to the file
    if(fh != NULL)
    {
      fprintf(fh, "USER = %s\n", user->Name);
      fprintf(fh, "ALIAS= %s\n", alias);
      fclose(fh);
    }
  }

  LEAVE();
}

///

/* Overloaded Methods */
/// OVERLOAD(OM_NEW)
OVERLOAD(OM_NEW)
{
  struct TagItem *tags = inittags(msg), *tag;
  char *bodyText = NULL;
  char *alias = NULL;
  char *buf = NULL;
  char *yesText = NULL;
  char *noText = NULL;
  Object *yesButton = NULL;
  Object *noButton = NULL;
  Object *skipTodayCheckbox = NULL;

  ENTER();

  while((tag = NextTagItem((APTR)&tags)) != NULL)
  {
    switch(tag->ti_Tag)
    {
      case ATTR(Body):
      {
        bodyText = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;

      case ATTR(Alias):
      {
        alias = (char *)tag->ti_Data;
        tag->ti_Tag = TAG_IGNORE;
      }
      break;
    }
  }
  // check if we really should display the requester for this alias
  if(CheckBirthdayCheckFile(alias) == TRUE)
  {
    // split 'Yes|No' into yesText and noText
    noText = (char *)tr(MSG_YesNoReq);
    if((buf = strdup(noText)) != NULL)
    {
      yesText = buf;
      if((noText = strchr(buf, '|')) != NULL)
        *noText++ = '\0';

      if(*yesText == '*')
        yesText++;

      if((obj = DoSuperNew(cl, obj,

        MUIA_Window_LeftEdge,    MUIV_Window_LeftEdge_Centered,
        MUIA_Window_TopEdge,     MUIV_Window_TopEdge_Centered,
        MUIA_Window_Width,       MUIV_Window_Width_MinMax(0),
        MUIA_Window_Height,      MUIV_Window_Height_MinMax(0),
        MUIA_Window_ID,          MAKE_ID('B','R','E','Q'),
        MUIA_Window_CloseGadget, FALSE,
        MUIA_Window_SizeGadget,  FALSE,
        MUIA_Window_Activate,    TRUE,
        MUIA_Window_NoMenus,     TRUE,
        WindowContents, VGroup,
          MUIA_Background, MUII_RequesterBack,
          InnerSpacing(4, 4),
          Child, HGroup,
            GroupFrame,
            MUIA_Background, MUII_GroupBack,
            Child, HSpace(0),
            Child, TextObject,
              InnerSpacing(4, 4),
              MUIA_Text_Contents, bodyText,
              MUIA_Text_SetMax,   TRUE,
            End,
            Child, HSpace(0),
          End,
          Child, HGroup,
            Child, skipTodayCheckbox = MakeCheck(tr(MSG_BIRTHDAYREQUEST_DONTSHOWAGAIN)),
            Child, LLabel1(tr(MSG_BIRTHDAYREQUEST_DONTSHOWAGAIN)),
            Child, HVSpace,
          End,
          Child, HGroup,
            Child, yesButton = MakeButton(yesText),
            Child, VSpace(0),
            Child, noButton = MakeButton(noText),
          End,
        End,

        TAG_MORE, inittags(msg))) != NULL)
      {
        GETDATA;

        data->skipTodayCheckbox = skipTodayCheckbox;

        DoMethod(G->App, OM_ADDMEMBER, obj);

        // enable the checkbox by default
        set(skipTodayCheckbox, MUIA_Selected, TRUE);

        DoMethod(yesButton, MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, METHOD(FinishInput), alias, TRUE);
        DoMethod(noButton,  MUIM_Notify, MUIA_Pressed, FALSE, obj, 3, METHOD(FinishInput), alias, FALSE);

        xset(obj, MUIA_Window_Activate,    TRUE,
                  MUIA_Window_Title,       tr(MSG_AB_BirthdayReminder),
                  MUIA_Window_ScreenTitle, CreateScreenTitle(data->screenTitle, sizeof(data->screenTitle), tr(MSG_AB_BirthdayReminder)),
                  MUIA_Window_Open,        TRUE);

        free(buf);
      }
    }
  }

  RETURN(obj);
  return (IPTR)obj;
}

///

/* Public Methods */
/// DECLARE(FinishInput)
//
DECLARE(FinishInput) // const char *alias, ULONG writeMail
{
  GETDATA;
  BOOL skipToday;

  ENTER();

  // before we close the window we remember the checkbox status
  skipToday = xget(data->skipTodayCheckbox, MUIA_Selected);

  // close the requester window
  set(obj, MUIA_Window_Open, FALSE);

  // remove & dispose the requester object
  DoMethod(_app(obj), MUIM_Application_PushMethod, _app(obj), 2, MUIM_YAMApplication_DisposeSubWindow, obj);

  if(skipToday == TRUE)
  {
    // save the user and alias to the birthdaycheckfile
    SaveBirthdayCheckFile(msg->alias);
  }

  // if the user click the yesButton we open a NewWriteMailWindow
  if(msg->writeMail == TRUE)
  {
    struct WriteMailData *wmData;

    if((wmData = NewWriteMailWindow(NULL, 0)) != NULL)
    {
      xset(wmData->window,
           MUIA_WriteWindow_To,      msg->alias,
           MUIA_WriteWindow_Subject, tr(MSG_AB_HappyBirthday));
    }
  }

  RETURN(0);
  return 0;
}

///


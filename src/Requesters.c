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

#include <stdlib.h>
#include <strings.h>
#include <ctype.h>

#include <clib/alib_protos.h>
#include <libraries/iffparse.h>
#include <mui/NList_mcc.h>
#include <mui/NListview_mcc.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>

#include <proto/amissl.h>
#include <proto/amisslmaster.h>

#include "extrasrc.h"

#include "SDI_hook.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_configFile.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"

#include "mui/ClassesExtra.h"
#include "mui/AttachmentRequestWindow.h"
#include "mui/CheckboxRequestWindow.h"
#include "mui/FolderRequestWindow.h"
#include "mui/GenericRequestWindow.h"
#include "mui/PassphraseRequestWindow.h"
#include "mui/StringRequestWindow.h"
#include "mui/YAMApplication.h"

#include "DynamicStrings.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailServers.h"
#include "MethodStack.h"
#include "MUIObjects.h"
#include "Requesters.h"
#include "Threads.h"

#include "tcp/Connection.h"
#include "tcp/ssl.h"

#include "Debug.h"

#define REQUESTER_RETURNID          1000

/// YAMMUIRequest
// Own -secure- implementation of MUI_Request with collecting and reissueing ReturnIDs
// We also have a wrapper #define MUI_Request for calling that function instead.
LONG YAMMUIRequest(const Object *app, const Object *parent, LONG flags, const char *tit, const char *gad, const char *format, ...)
{
  LONG result = -1;
  va_list args;
  char *reqtxt = NULL;

  ENTER();

  // resolve the requester txt first
  va_start(args, format);
  if(vasprintf(&reqtxt, format, args) != -1)
  {
    // now call the YAMMUIRequestA() function which doesn't have a variable
    // arguments list anymore.
    result = YAMMUIRequestA(app, parent, flags, tit, gad, reqtxt);
  }
  va_end(args);

  // free the requester txt afterwards again
  free(reqtxt);

  RETURN(result);
  return result;
}

LONG YAMMUIRequestA(const Object *app, const Object *parent, LONG flags, const char *title, const char *gadgets, const char *reqtxt)
{
  LONG result = -1;

  ENTER();

  // we make sure that every thread in YAM can call this function. If this isn't the
  // main thread we simply push the message and wait until it returns.
  if(IsMainThread() == FALSE)
  {
    result = PushMethodOnStackWait(G->App, 7, MUIM_YAMApplication_MUIRequestA, app, parent, flags, title, gadgets, reqtxt);

    RETURN(result);
    return result;
  }

  // if the applicationpointer is NULL we fall back to a standard requester
  if(app == NULL)
  {
    if(IntuitionBase != NULL)
    {
      struct EasyStruct ErrReq;
      char *stripped_gadgets;
      char *p;

      // we have to strip any special characters from the gadgets
      // text (e.g. '_') so that intuition can open more nice looking requesters
      stripped_gadgets = strdup(gadgets);
      while((p = strchr(stripped_gadgets, '_')) != NULL)
        memmove(p, p+1, strlen(p)+1);

      ErrReq.es_StructSize   = sizeof(struct EasyStruct);
      ErrReq.es_Flags        = 0;
      ErrReq.es_Title        = (STRPTR)title;
      ErrReq.es_TextFormat   = (STRPTR)reqtxt;
      ErrReq.es_GadgetFormat = (STRPTR)stripped_gadgets;

      result = EasyRequestArgs(NULL, &ErrReq, NULL, NULL);

      free(stripped_gadgets);
    }
  }
  else
  {
    Object *win;

    win = GenericRequestWindowObject,
      MUIA_Window_Title,                    title != NULL ? title : tr(MSG_MA_ConfirmReq),
      MUIA_Window_RefWindow,                parent,
      MUIA_GenericRequestWindow_Body,       reqtxt,
      MUIA_GenericRequestWindow_Buttons,    gadgets,
      MUIA_GenericRequestWindow_Floattext,  isFlagSet(flags, MUIF_REQ_FLOATTEXT),
    End;

    // lets see if the WindowObject could be created perfectly
    if(win != NULL)
    {
      DoMethod(win, MUIM_Notify, MUIA_GenericRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

      set(G->App, MUIA_Application_Sleep, TRUE);

      if(SafeOpenWindow(win) == TRUE)
      {
        ULONG signals = 0;

        do
        {
          if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
          {
            result = xget(win, MUIA_GenericRequestWindow_Result);
            break;
          }

          if(signals != 0)
            signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

          // bail out if we receive a CTRL-C
          if(isFlagSet(signals, SIGBREAKF_CTRL_C))
            break;

          // show ourselves if we receive a CTRL-F
          if(isFlagSet(signals, SIGBREAKF_CTRL_F))
            PopUp();
        }
        while(TRUE);
      }

      // remove & dispose the requester object
      DoMethod(G->App, OM_REMMEMBER, win);
      MUI_DisposeObject(win);

      // wake up the application
      set(G->App, MUIA_Application_Sleep, FALSE);
    }
  }

  RETURN(result);
  return result;
}

///
/// StringRequest
//  Puts up a string requester
int StringRequest(char *string, int size, const char *title, const char *body,
                  const char *yestext, const char *alttext, const char *notext,
                  BOOL secret, Object *parent)
{
  int result = -1;
  Object *win;

  ENTER();

  if((win = StringRequestWindowObject,
    MUIA_Window_Title, title,
    MUIA_Window_RefWindow, parent,
    MUIA_StringRequestWindow_StringContents, string,
    MUIA_StringRequestWindow_MaxLength, size,
    MUIA_StringRequestWindow_Body, body,
    MUIA_StringRequestWindow_YesText, yestext,
    MUIA_StringRequestWindow_NoText, notext,
    MUIA_StringRequestWindow_AlternativeText, alttext,
    MUIA_StringRequestWindow_Secret, secret,
  End) != NULL)
  {
    DoMethod(win, MUIM_Notify, MUIA_StringRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

    set(G->App, MUIA_Application_Sleep, TRUE);

    if(SafeOpenWindow(win) == TRUE)
    {
      ULONG signals = 0;

      do
      {
        if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
        {
          if((result = xget(win, MUIA_StringRequestWindow_Result)) != 0)
          {
            get(win, MUIA_StringRequestWindow_StringContents, string);
          }

          break;
        }

        if(signals != 0)
          signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        // bail out if we receive a CTRL-C
        if(isFlagSet(signals, SIGBREAKF_CTRL_C))
          break;

        // show ourselves if we receive a CTRL-F
        if(isFlagSet(signals, SIGBREAKF_CTRL_F))
          PopUp();
      }
      while(TRUE);
    }

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, win);
    MUI_DisposeObject(win);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(result);
  return result;
}

///
/// PassphraseRequest
//  Puts up a string requester for entering a PGP passphrase
int PassphraseRequest(char *string, int size, Object *parent)
{
  int result = -1;
  Object *win;

  ENTER();

  if((win = PassphraseRequestWindowObject,
    MUIA_Window_RefWindow, parent,
    MUIA_PassphraseRequestWindow_StringContents, string,
    MUIA_PassphraseRequestWindow_MaxLength, size,
  End) != NULL)
  {
    DoMethod(win, MUIM_Notify, MUIA_PassphraseRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

    set(G->App, MUIA_Application_Sleep, TRUE);

    if(SafeOpenWindow(win) == TRUE)
    {
      ULONG signals = 0;

      do
      {
        if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
        {
          if((result = xget(win, MUIA_PassphraseRequestWindow_Result)) != 0)
          {
            get(win, MUIA_PassphraseRequestWindow_StringContents, string);
            if(xget(win, MUIA_PassphraseRequestWindow_RememberPhrase) == TRUE)
              C->PGPPassInterval = abs(C->PGPPassInterval);
            else if(C->PGPPassInterval > 0)
              C->PGPPassInterval = -C->PGPPassInterval;
          }

          break;
        }

        if(signals != 0)
          signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        // bail out if we receive a CTRL-C
        if(isFlagSet(signals, SIGBREAKF_CTRL_C))
          break;

        // show ourselves if we receive a CTRL-F
        if(isFlagSet(signals, SIGBREAKF_CTRL_F))
          PopUp();
      }
      while(TRUE);
    }

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, win);
    MUI_DisposeObject(win);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(result);
  return result;
}

///
/// FolderRequest
//  Allows user to choose a folder from a list
struct Folder *FolderRequest(const char *title, const char *body, const char *yestext, const char *notext,
                             struct Folder *exclude, Object *parent)
{
  struct Folder *folder = NULL;
  static struct Folder *prevFolder = NULL;
  Object *win;

  ENTER();

  if((win = FolderRequestWindowObject,
    MUIA_Window_Title, title,
    MUIA_Window_RefWindow, parent,
    MUIA_FolderRequestWindow_Body, body,
    MUIA_FolderRequestWindow_YesText, yestext,
    MUIA_FolderRequestWindow_NoText, notext,
    MUIA_FolderRequestWindow_Exclude, exclude,
    MUIA_FolderRequestWindow_Folder, prevFolder,
  End) != NULL)
  {
    DoMethod(win, MUIM_Notify, MUIA_FolderRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

    set(G->App, MUIA_Application_Sleep, TRUE);

    if(SafeOpenWindow(win) == TRUE)
    {
      ULONG signals = 0;

      do
      {
        if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
        {
          if(xget(win, MUIA_FolderRequestWindow_Result) != 0)
          {
            folder = (struct Folder *)xget(win, MUIA_FolderRequestWindow_Folder);
            // remember the selected folder for the next time
            prevFolder = folder;
          }

          break;
        }

        if(signals != 0)
          signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        // bail out if we receive a CTRL-C
        if(isFlagSet(signals, SIGBREAKF_CTRL_C))
          break;

        // show ourselves if we receive a CTRL-F
        if(isFlagSet(signals, SIGBREAKF_CTRL_F))
          PopUp();
      }
      while(TRUE);
    }

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, win);
    MUI_DisposeObject(win);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(folder);
  return folder;
}

///
/// AttachRequest
//  Allows user to select a message part (attachment) from a list
struct Part *AttachRequest(const char *title, const char *body, const char *yestext,
                           const char *notext, int mode, struct ReadMailData *rmData)
{
  struct Part *part = NULL;
  Object *win;

  ENTER();

  if((win = AttachmentRequestWindowObject,
    MUIA_Window_Title, title,
    MUIA_AttachmentRequestWindow_Body, body,
    MUIA_AttachmentRequestWindow_YesText, yestext,
    MUIA_AttachmentRequestWindow_NoText, notext,
    MUIA_AttachmentRequestWindow_Mode, mode,
    MUIA_AttachmentRequestWindow_ReadMailData, rmData,
  End) != NULL)
  {
    DoMethod(win, MUIM_Notify, MUIA_AttachmentRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

    set(G->App, MUIA_Application_Sleep, TRUE);

    if(SafeOpenWindow(win) == TRUE)
    {
      ULONG signals = 0;

      do
      {
        if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
        {
          if(xget(win, MUIA_AttachmentRequestWindow_Result) != 0)
          {
            part = (struct Part *)xget(win, MUIA_AttachmentRequestWindow_Part);
          }

          break;
        }

        if(signals != 0)
          signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

        // bail out if we receive a CTRL-C
        if(isFlagSet(signals, SIGBREAKF_CTRL_C))
          break;

        // show ourselves if we receive a CTRL-F
        if(isFlagSet(signals, SIGBREAKF_CTRL_F))
          PopUp();
      }
      while(TRUE);
    }

    // remove & dispose the requester object
    DoMethod(G->App, OM_REMMEMBER, win);
    MUI_DisposeObject(win);

    // wake up the application
    set(G->App, MUIA_Application_Sleep, FALSE);
  }

  RETURN(part);
  return part;
}

///
/// CheckboxRequest
// Displays a requester with a list of checkboxes
LONG CheckboxRequest(Object *parent, const char *tit, ULONG numBoxes, const char *text, ...)
{
  va_list args;
  LONG flags = -1;
  char **entries;

  ENTER();

  va_start(args, text);

  // we support 31 possible checkboxes at most
  numBoxes = MIN(31, numBoxes);

  // allocate memory for the entries plus one entry for the terminating NULL pointer
  if((entries = calloc(numBoxes + 1, sizeof(char *))) != NULL)
  {
    ULONG i;
    char **ptr = entries;
    Object *win;

    // copy the entries to our own array
    for(i = 0; i < numBoxes; i++)
      *ptr++ = va_arg(args, char *);

    // and add the terminating NULL pointer
    *ptr = NULL;

    if((win = CheckboxRequestWindowObject,
      MUIA_Window_Title, tit,
      MUIA_Window_RefWindow, parent,
      MUIA_CheckboxRequestWindow_Body, text,
      MUIA_CheckboxRequestWindow_Entries, entries,
    End) != NULL)
    {
      DoMethod(win, MUIM_Notify, MUIA_CheckboxRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

      set(G->App, MUIA_Application_Sleep, TRUE);

      if(SafeOpenWindow(win) == TRUE)
      {
        ULONG signals = 0;

        do
        {
          if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
          {
            flags = xget(win, MUIA_CheckboxRequestWindow_Flags);
            break;
          }

          if(signals != 0)
            signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

          // bail out if we receive a CTRL-C
          if(isFlagSet(signals, SIGBREAKF_CTRL_C))
            break;

          // show ourselves if we receive a CTRL-F
          if(isFlagSet(signals, SIGBREAKF_CTRL_F))
            PopUp();
        }
        while(TRUE);
      }

      // remove & dispose the requester object
      DoMethod(G->App, OM_REMMEMBER, win);
      MUI_DisposeObject(win);

      // wake up the application
      set(G->App, MUIA_Application_Sleep, FALSE);
    }

    free(entries);
  }

  va_end(args);

  RETURN(flags);
  return flags;
}

///
/// CertWarningRequest
// warns the user about a non-verified certificate
BOOL CertWarningRequest(struct Connection *conn, struct Certificate *cert)
{
  BOOL result = FALSE;

  ENTER();

  // we make sure that every thread in YAM can call this function. If this isn't the
  // main thread we simply push the message and wait until it returns.
  if(IsMainThread() == FALSE)
    result = PushMethodOnStackWait(G->App, 3, MUIM_YAMApplication_CertWarningRequest, conn, cert);
  else
  {
    Object *win;
    char *reqtxt = NULL;
    char *format = NULL;
    int failures = conn->sslCertFailures;

    // now we create the requester text
    dstrcpy(&format, tr(MSG_SSL_CERT_WARNING_INTRO));
    dstrcat(&format, "\n\n");

    if(isFlagSet(failures, SSL_CERT_ERR_UNTRUSTED))
    {
      dstrcat(&format, tr(MSG_SSL_CERT_WARNING_UNTRUSTED));
      dstrcat(&format, "\n");
    }

    if(isFlagSet(failures, SSL_CERT_ERR_IDMISMATCH))
    {
      dstrcat(&format, tr(MSG_SSL_CERT_WARNING_IDMISMATCH));
      dstrcat(&format, "\n");
    }

    if(isFlagSet(failures, SSL_CERT_ERR_NOTYETVALID))
    {
      dstrcat(&format, tr(MSG_SSL_CERT_WARNING_NOTYETVALID));
      dstrcat(&format, "\n");
    }

    if(isFlagSet(failures, SSL_CERT_ERR_EXPIRED))
    {
      dstrcat(&format, tr(MSG_SSL_CERT_WARNING_EXPIRED));
      dstrcat(&format, "\n");
    }

    if(isFlagSet(failures, SSL_CERT_ERR_OTHER))
    {
      dstrcat(&format, tr(MSG_SSL_CERT_WARNING_OTHER));
      dstrcat(&format, "\n");
    }

    dstrcat(&format, "\n");
    dstrcat(&format, tr(MSG_SSL_CERT_WARNING_INFO));

    // convert the format string now to a full string
    // with contents
    if(asprintf(&reqtxt, format, conn->server->hostname, conn->server->port, cert->identity, cert->notBefore, cert->notAfter, cert->issuerStr, cert->fingerprint) != -1)
    {
      // free the format template right now
      dfree(format);

      // create the window object now
      win = GenericRequestWindowObject,
        MUIA_Window_Title,                 tr(MSG_SSL_CERT_WARNING_TITLE),
        MUIA_Window_RefWindow,             G->MA->GUI.WI,
        MUIA_GenericRequestWindow_Body,    reqtxt,
        MUIA_GenericRequestWindow_Buttons, tr(MSG_SSL_CERT_WARNING_BUTTONS),
      End;

      // lets see if the WindowObject could be created perfectly
      if(win != NULL)
      {
        DoMethod(win, MUIM_Notify, MUIA_GenericRequestWindow_Result, MUIV_EveryTime, MUIV_Notify_Application, 2, MUIM_Application_ReturnID, REQUESTER_RETURNID);

        set(G->App, MUIA_Application_Sleep, TRUE);

        if(SafeOpenWindow(win) == TRUE)
        {
          ULONG signals = 0;

          do
          {
            if(DoMethod(G->App, MUIM_Application_NewInput, &signals) == REQUESTER_RETURNID)
            {
              int ret = xget(win, MUIA_GenericRequestWindow_Result);

              if(ret == 0) // user pressed 'Reject'
              {
                result = FALSE;
                break;
              }
              else if(ret == 1) // user pressed 'Accept'
              {
                result = TRUE;
                break;
              }
              else if(ret == 2) // user pressed 'Accept permanently'
              {
                // user wants to accept the SSL certificate permanently so lets
                // save the fingerprint and the failure bitmask in the config structure
                // of the MailServerNode
                strlcpy(conn->server->certFingerprint, cert->fingerprint, sizeof(conn->server->certFingerprint));
                conn->server->certFailures = failures;

                // make sure to save the config afterwards
                CO_SaveConfig(C, G->CO_PrefsFile);

                // signal NO error
                result = TRUE;

                break;
              }
              else if(ret == 3) // user pressed 'Show Certificate'
              {
                BIO* temp_memory_bio = BIO_new(BIO_s_mem());
                if(temp_memory_bio != NULL)
                {
                  char *buffer;
                  char *reqtitle;

                  X509_print_ex(temp_memory_bio, cert->subject, XN_FLAG_COMPAT, 0);
                  BIO_write(temp_memory_bio, "\0", 1);
                  BIO_get_mem_data(temp_memory_bio, &buffer);

                  // create the requester title string
                  if(asprintf(&reqtitle, tr(MSG_SSL_CERT_WARNING_SHOWTITLE), conn->server->hostname, conn->server->port) != -1)
                  {
                    // open an additional MUI requester now
                    MUI_Request(_app(win), win, MUIF_REQ_FLOATTEXT, reqtitle, tr(MSG_OkayReq), (char *)buffer);

                    free(reqtitle);
                  }
                  BIO_free(temp_memory_bio);
                }
                else
                  E(DBF_NET, "Failed to allocate temporary memory bio");
              }
            }

            if(signals != 0)
              signals = Wait(signals | SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_F);

            // bail out if we receive a CTRL-C
            if(isFlagSet(signals, SIGBREAKF_CTRL_C))
              break;

            // show ourselves if we receive a CTRL-F
            if(isFlagSet(signals, SIGBREAKF_CTRL_F))
              PopUp();
          }
          while(TRUE);
        }

        // remove & dispose the requester object
        DoMethod(G->App, OM_REMMEMBER, win);
        MUI_DisposeObject(win);

        // wake up the application
        set(G->App, MUIA_Application_Sleep, FALSE);
      }

      free(reqtxt);
    }
    else
    {
      // free the format template right now
      dfree(format);
    }
  }

  RETURN(result);
  return result;
}

///

#ifndef THREADS_H
#define THREADS_H

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

#include "SDI_compiler.h"

// special string tag item, the string supplied in ti_Data will be strdup()'ed
#define TAG_STRING (1 << 30)

enum ThreadAction
{
  TA_Startup,
  TA_Shutdown,
  TA_LaunchCommand,
  TA_FlushSpamTrainingData,
  TA_SendMails,
  TA_ImportMails,
  TA_ReceiveMails,
};

#define TT_Priority                                 0xf001 // priority of the thread

#define TT_LaunchCommand_Command     (TAG_STRING | (TAG_USER + 1))
#define TT_LaunchCommand_Output                    (TAG_USER + 2)

#define TT_SendMails_MailServer                    (TAG_USER + 1)
#define TT_SendMails_Mails                         (TAG_USER + 2)
#define TT_SendMails_Mode                          (TAG_USER + 3)

#define TT_ImportMails_File          (TAG_STRING | (TAG_USER + 1))
#define TT_ImportMails_Folder                      (TAG_USER + 2)
#define TT_ImportMails_Flags                       (TAG_USER + 3)

#define TT_ReceiveMails_MailServer                 (TAG_USER + 1)
#define TT_ReceiveMails_Flags                      (TAG_USER + 2)
#define TT_ReceiveMails_Result                     (TAG_USER + 3)

/*** Thread system init/cleanup functions ***/
BOOL InitThreads(void);
void CleanupThreads(void);
void HandleThreads(void);
void AbortWorkingThreads(void);
void PurgeIdleThreads(const BOOL purgeAll);
BOOL VARARGS68K DoAction(const enum ThreadAction action, ...);
BOOL IsMainThread(void);
APTR CurrentThread(void);
BOOL SleepThread(void);
void WakeupThread(APTR thread);
LONG ThreadAbortSignal(void);
LONG ThreadWakeupSignal(void);
LONG ThreadTimerSignal(void);
BOOL ThreadWasAborted(void);
const char *ThreadName(void);
BOOL InitThreadTimer(void);
void CleanupThreadTimer(void);
void StartThreadTimer(ULONG seconds, ULONG micros);
void StopThreadTimer(void);

#endif /* THREADS_H */


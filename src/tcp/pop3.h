#ifndef POP3_H
#define POP3_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2022 YAM Open Source Team

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

// forward declarations
struct Folder;
struct MailTransferNode;
struct MailServerNode;

struct DownloadResult
{
  long downloaded;
  long onServer;
  long dupeSkipped;
  long deleted;
  BOOL error;
};

#define RECEIVEF_USER            (1<<0) // transfer initiated by user
#define RECEIVEF_STARTUP         (1<<1) // transfer initiated by startup
#define RECEIVEF_TIMER           (1<<2) // transfer initiated by timer
#define RECEIVEF_AREXX           (1<<3) // transfer initiated by ARexx
#define RECEIVEF_SIGNAL          (1<<4) // wakeup a waiting thread after the transfer
#define RECEIVEF_TEST_CONNECTION (1<<5) // just test the connection, don't download any mails

// prototypes
BOOL ReceiveMails(struct MailServerNode *msn, const ULONG flags, struct DownloadResult *dlResult);

#endif /* POP3_H */

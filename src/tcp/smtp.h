#ifndef SMTP_H
#define SMTP_H

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

// forward declarations
struct MailList;
struct UserIdentityNode;

enum SendMailMode
{
  SENDMAIL_ALL_USER = 0,
  SENDMAIL_ALL_AUTO,
  SENDMAIL_ACTIVE_USER,
  SENDMAIL_ACTIVE_AUTO
};

#define SENDF_SIGNAL  (1<<0) // wakeup a waiting thread after the transfer

// prototypes
BOOL SendMails(struct UserIdentityNode *uin, struct MailList *mailsToSend, enum SendMailMode mode, const ULONG flags);

#endif /* SMTP_H */

#ifndef POP3_H
#define POP3_H

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

// forward declarations
struct Folder;
struct MailTransferNode;

// prototypes
void TR_GetMessageDetails(struct MailTransferNode *tnode, int lline);
BOOL TR_DeleteMessage(int number);
BOOL TR_LoadMessage(struct Folder *infolder, const int number);
void TR_GetMailFromNextPOP(BOOL isfirst, int singlepop, enum GUILevel guilevel);
BOOL TR_SendPOP3KeepAlive(void);

#endif /* POP3_H */

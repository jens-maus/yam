#ifndef YAM_REXX_H
#define YAM_REXX_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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

#include "YAM_rexx_rxif.h"

void ARexxDispatch(struct RexxHost *host);
void CloseDownARexxHost(struct RexxHost *host);
void DoRXCommand(struct RexxHost *host, struct RexxMsg *rexxmsg);
void FreeRexxCommand(struct RexxMsg *rxmsg);
void ReplyRexxCommand(struct RexxMsg *rxmsg, long prim, long sec, char *res);
struct RexxMsg *SendRexxCommand(struct RexxHost *host, char *buff, BPTR fh);
struct RexxHost *SetupARexxHost(char *basename, struct MsgPort *usrport);

#endif /* YAM_REXX_H */

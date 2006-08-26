#ifndef YAM_REXX_RXCL_H
#define YAM_REXX_RXCL_H

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

#define ARB_CF_ENABLED     (1L << 0)

#define ARB_HF_CMDSHELL    (1L << 0)
#define ARB_HF_USRMSGPORT  (1L << 1)

struct rxs_command
{
  const char *command;
  const char *args;
  const char *results;
  long resindex;
  void (*function)( struct RexxHost *, void **, long, struct RexxMsg * );
  long flags;
};

struct arb_p_link
{
  const char *str;
  int dst;
};

struct arb_p_state
{
  int cmd;
  struct arb_p_link *pa;
};

extern struct rxs_command rxs_commandlist[];
extern struct arb_p_state arb_p_state[];

#endif /* YAM_REXX_RXCL_H */

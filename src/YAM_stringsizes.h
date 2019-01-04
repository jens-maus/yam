#ifndef YAM_STRINGSIZES_H
#define YAM_STRINGSIZES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2019 YAM Open Source Team

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

#define SIZE_USERID       60
#define SIZE_PASSWORD     80
#define SIZE_ADDRESS     150
#define SIZE_HOST         80
#define SIZE_REALNAME     40
#define SIZE_SUBJECT     200
#define SIZE_MSGID       999 // RFC 4130 says 998 maximum + 1 NUL char
#define SIZE_MFILE        30
#define SIZE_COMMAND     120
#define SIZE_CTYPE        40
#define SIZE_NAME         30
#define SIZE_PATH        512 // max. possible directory path length (512 should be enough)
#define SIZE_FILE        108 // max. possible filename length (107+1)
#define SIZE_SMALL        16
#define SIZE_DEFAULT      80
#define SIZE_LARGE       512
#define SIZE_LINE       1001 // RFC 2821 says 1000 maximum + 1 NUL char
#define SIZE_RCPTS      4096
#define SIZE_INTRO       200
#define SIZE_PATTERN     160
#define SIZE_PATHFILE    (SIZE_PATH+SIZE_FILE)
#define SIZE_URL         (SIZE_HOST+SIZE_PATHFILE)
#define SIZE_EXALLBUF  32768
#define SIZE_FILEBUF   65536 // the buffer size for our fopen() file buffers
#define SIZE_STACK     65536 // stack size for main task and threads
#define SIZE_DSTRCHUNK  1024 // must be a power of 2

#define MAXP3_MENU        30  // the hardcoded max. for POP3 menu entries
#define MAXRX_MENU        10  // the hardcoded max. for Rexx menu entries
#define MAXSIG_MENU        8  // the hardcoded max. for signature menu entries

#define MAXERR            50
#define MAXUSERS          16
#define MAXEA              4

#endif /* YAM_STRINGSIZES_H */

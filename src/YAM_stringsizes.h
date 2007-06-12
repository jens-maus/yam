#ifndef YAM_STRINGSIZES_H
#define YAM_STRINGSIZES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

#define SIZE_USERID     60
#define SIZE_PASSWORD   80
#define SIZE_ADDRESS   150
#define SIZE_HOST       80
#define SIZE_REALNAME   40
#define SIZE_SUBJECT   200
#define SIZE_MSGID      80
#define SIZE_MFILE      30
#define SIZE_COMMAND   120
#define SIZE_CTYPE      40
#define SIZE_NAME       30
#define SIZE_PATH      512 // max. possible directory path length (512 should be enough)
#define SIZE_FILE      108 // max. possible filename length (107+1)
#define SIZE_SMALL      16
#define SIZE_DEFAULT    80
#define SIZE_LARGE     512
#define SIZE_LINE     1001
#define SIZE_RCPTS    4096
#define SIZE_INTRO     200
#define SIZE_PATTERN   160
#define SIZE_PATHFILE  (SIZE_PATH+SIZE_FILE)
#define SIZE_URL       (SIZE_HOST+SIZE_PATHFILE)
#define SIZE_EXALLBUF 32768
#define SIZE_FILEBUF  65535 // the buffer size for our fopen() file buffers

#define MAXP3           16
#define MAXRX           21

#define MAX_STATUSIMG   19
#define MAX_FOLDERIMG   11
#define MAX_CFGIMG      36
#define MAX_IMAGES      (MAX_STATUSIMG+MAX_FOLDERIMG+MAX_CFGIMG)

#define MAXICONS         4
#define MAXERR          50
#define MAXUSERS        16
#define MAXEA            4
#define MAXWR            2 /* BEWARE: Don't change this value - it's hardcoded many places! */

#endif /* YAM_STRINGSIZES_H */

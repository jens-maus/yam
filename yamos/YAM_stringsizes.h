#ifndef YAM_STRINGSIZES_H
#define YAM_STRINGSIZES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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
#define SIZE_ADDRESS   100
#define SIZE_HOST       80
#define SIZE_REALNAME   40
#define SIZE_SUBJECT   200
#define SIZE_MSGID      80
#define SIZE_MFILE      12
#define SIZE_COMMAND   120
#define SIZE_CTYPE      40
#define SIZE_NAME       20
#define SIZE_PATH      120
#define SIZE_FILE       32
#define SIZE_SMALL      16
#define SIZE_DEFAULT    80
#define SIZE_LARGE     512
#define SIZE_LINE     1001
#define SIZE_RCPTS    4096
#define SIZE_INTRO     200
#define SIZE_PATTERN   160
#define SIZE_PATHFILE  (SIZE_PATH+SIZE_FILE)
#define SIZE_URL       (SIZE_HOST+SIZE_PATHFILE)

#define MAXP3           16
#define MAXRU          100
#define MAXMV          100
#define MAXRX           21

#define MAXBCSTDIMAGES   9
#define MAXICONS         4
#define MAXIMAGES       26
#define MAXASL           8
#define MAXERR          50
#define MAXUSERS        16
#define MAXCTYPE        25
#define MAXCPAGES       15
#define MAXEA            4
#define MAXRE            4
#define MAXWR            2 /* BEWARE: Don't change this value - it's hardcoded many places! */

#endif /* YAM_STRINGSIZES_H */

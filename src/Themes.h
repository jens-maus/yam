#ifndef THEMES_H
#define THEMES_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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

#include <exec/types.h>

// forward declarations
struct DiskObject;

// All the image numbers MUST be declared using #define instead of defining
// distinct types using enum. Converting the latter to a constant string at
// compile time (see SI_STR() macro at the bottom) is not possible, hence
// the bunch of typeless #defines.

// images in the configuration window
#define CI_FIRST              0
#define CI_ABOOK              0
#define CI_ABOOKBIG           1
#define CI_ANSWER             2
#define CI_ANSWERBIG          3
#define CI_FILTERS            4
#define CI_FILTERSBIG         5
#define CI_FIRSTSTEP          6
#define CI_FIRSTSTEPBIG       7
#define CI_LISTS              8
#define CI_LISTSBIG           9
#define CI_LOOKFEEL          10
#define CI_LOOKFEELBIG       11
#define CI_MIME              12
#define CI_MIMEBIG           13
#define CI_MISC              14
#define CI_MISCBIG           15
#define CI_NETWORK           16
#define CI_NETWORKBIG        17
#define CI_NEWMAIL           18
#define CI_NEWMAILBIG        19
#define CI_READ              20
#define CI_READBIG           21
#define CI_SCRIPTS           22
#define CI_SCRIPTSBIG        23
#define CI_SECURITY          24
#define CI_SECURITYBIG       25
#define CI_SIGNATURE         26
#define CI_SIGNATUREBIG      27
#define CI_SPAM              28
#define CI_SPAMBIG           29
#define CI_START             30
#define CI_STARTBIG          31
#define CI_UPDATE            32
#define CI_UPDATEBIG         33
#define CI_WRITE             34
#define CI_WRITEBIG          35
#define CI_MAX               36

// images in the folder list
#define FI_FIRST              0
#define FI_FOLD               0
#define FI_UNFOLD             1
#define FI_INCOMING           2
#define FI_INCOMINGNEW        3
#define FI_OUTGOING           4
#define FI_OUTGOINGNEW        5
#define FI_SENT               6
#define FI_SPAM               7
#define FI_SPAMNEW            8
#define FI_TRASH              9
#define FI_TRASHNEW          10
#define FI_MAX               11

// the AppIcon images
#define II_FIRST              0
#define II_CHECK              0
#define II_EMPTY              1
#define II_NEW                2
#define II_OLD                3
#define II_MAX                4

// status images in the mail list
#define SI_FIRST              0
#define SI_ATTACH             0
#define SI_CRYPT              1
#define SI_DELETE             2
#define SI_DOWNLOAD           3
#define SI_ERROR              4
#define SI_FORWARD            5
#define SI_GROUP              6
#define SI_HOLD               7
#define SI_MARK               8
#define SI_NEW                9
#define SI_OLD               10
#define SI_REPLY             11
#define SI_REPORT            12
#define SI_SENT              13
#define SI_SIGNED            14
#define SI_SPAM              15
#define SI_UNREAD            16
#define SI_URGENT            17
#define SI_WAITSEND          18
#define SI_MAX               19

// the different images for a button in the toolbars
#define TBIM_FIRST            0
#define TBIM_NORMAL           0
#define TBIM_SELECTED         1
#define TBIM_GHOSTED          2
#define TBIM_MAX              3

// all the toolbar images
#define TBI_FIRST             0
#define TBI_READ              0
#define TBI_EDIT              1
#define TBI_MOVE              2
#define TBI_DELETE            3
#define TBI_GETADDR           4
#define TBI_NEWMAIL           5
#define TBI_REPLY             6
#define TBI_FORWARD           7
#define TBI_GETMAIL           8
#define TBI_SENDALL           9
#define TBI_SPAM             10
#define TBI_HAM              11
#define TBI_FILTER           12
#define TBI_FIND             13
#define TBI_ADDRBOOK         14
#define TBI_CONFIG           15
#define TBI_PREV             16
#define TBI_NEXT             17
#define TBI_PREVTHREAD       18
#define TBI_NEXTTHREAD       19
#define TBI_DISPLAY          20
#define TBI_SAVE             21
#define TBI_PRINT            22
#define TBI_EDITOR           23
#define TBI_INSERT           24
#define TBI_CUT              25
#define TBI_COPY             26
#define TBI_PASTE            27
#define TBI_UNDO             28
#define TBI_BOLD             29
#define TBI_ITALIC           30
#define TBI_UNDERLINE        31
#define TBI_COLORED          32
#define TBI_NEWUSER          33
#define TBI_NEWLIST          34
#define TBI_NEWGROUP         35
#define TBI_OPENTREE         36
#define TBI_CLOSETREE        37
#define TBI_MAX              38

// the main window's toolbar images
#define MWTBI_FIRST            0
#define MWTBI_READ             0
#define MWTBI_EDIT             1
#define MWTBI_MOVE             2
#define MWTBI_DELETE           3
#define MWTBI_GETADDR          4
#define MWTBI_NEWMAIL          5
#define MWTBI_REPLY            6
#define MWTBI_FORWARD          7
#define MWTBI_GETMAIL          8
#define MWTBI_SENDALL          9
#define MWTBI_SPAM            10
#define MWTBI_HAM             11
#define MWTBI_FILTER          12
#define MWTBI_FIND            13
#define MWTBI_ADDRBOOK        14
#define MWTBI_CONFIG          15
#define MWTBI_NULL            16
#define MWTBI_MAX             17

// the read window's toolbar images
#define RWTBI_FIRST            0
#define RWTBI_PREV             0
#define RWTBI_NEXT             1
#define RWTBI_PREVTHREAD       2
#define RWTBI_NEXTTHREAD       3
#define RWTBI_DISPLAY          4
#define RWTBI_SAVE             5
#define RWTBI_PRINT            6
#define RWTBI_DELETE           7
#define RWTBI_MOVE             8
#define RWTBI_REPLY            9
#define RWTBI_FORWARD         10
#define RWTBI_SPAM            11
#define RWTBI_HAM             12
#define RWTBI_NULL            13
#define RWTBI_MAX             14

// the write window's toolbar images
#define WWTBI_FIRST            0
#define WWTBI_EDITOR           0
#define WWTBI_INSERT           1
#define WWTBI_CUT              2
#define WWTBI_COPY             3
#define WWTBI_PASTE            4
#define WWTBI_UNDO             5
#define WWTBI_BOLD             6
#define WWTBI_ITALIC           7
#define WWTBI_UNDERLINE        8
#define WWTBI_COLORED          9
#define WWTBI_SEARCH          10
#define WWTBI_NULL            11
#define WWTBI_MAX             12

// the addressbook window's toolbar images
#define AWTBI_FIRST            0
#define AWTBI_SAVE             0
#define AWTBI_FIND             1
#define AWTBI_NEWUSER          2
#define AWTBI_NEWLIST          3
#define AWTBI_NEWGROUP         4
#define AWTBI_EDIT             5
#define AWTBI_DELETE           6
#define AWTBI_PRINT            7
#define AWTBI_OPENTREE         8
#define AWTBI_CLOSETREE        9
#define AWTBI_NULL            10
#define AWTBI_MAX             11

struct Theme
{
  char *name;
  char *author;
  char *url;
  char *version;
  char *configImages[CI_MAX];
  char *folderImages[FI_MAX];
  char *iconImages[II_MAX];
  char *statusImages[SI_MAX];
  char *mainWindowToolbarImages[TBIM_MAX][MWTBI_MAX];
  char *readWindowToolbarImages[TBIM_MAX][RWTBI_MAX];
  char *writeWindowToolbarImages[TBIM_MAX][WWTBI_MAX];
  char *abookWindowToolbarImages[TBIM_MAX][AWTBI_MAX];
  struct DiskObject *icons[II_MAX];

  char directory[SIZE_PATHFILE];
  BOOL loaded;
};

void AllocTheme(struct Theme *theme, const char *themeName);
void FreeTheme(struct Theme *theme);
LONG ParseThemeFile(const char *themeFile, struct Theme *theme);
void LoadTheme(struct Theme *theme, const char *themeName);
void UnloadTheme(struct Theme *theme);

// a macro to build the necessary string to use an image in an NList object
#define SI_STR(id)  SI_STR2(id)
#define SI_STR2(id) "\033o[" #id "]"

#endif /* THEMES_H */


#ifndef YAM_FIND_H
#define YAM_FIND_H

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

#include <dos/datetime.h>

#ifndef FORMAT_DEF
#define FORMAT_DEF 4
#endif

#include "YAM_mainFolder.h"

struct SearchGroup
{
   APTR PG_SRCHOPT;
   APTR CY_MODE;
   APTR ST_FIELD;
   APTR CY_COMP[5];
   APTR ST_MATCH[5];
   APTR BT_FILE[5];
   APTR BT_EDIT[5];
   APTR RA_ADRMODE;
   APTR CY_STATUS;
   APTR CH_CASESENS[5];
   APTR CH_SUBSTR[5];
};

struct FI_GUIData
{
   APTR WI;
   APTR LV_FOLDERS;
   struct SearchGroup GR_SEARCH;
   APTR LV_MAILS;
   APTR GR_PAGE;
   APTR GA_PROGRESS;
   APTR BT_SELECT;
   APTR BT_READ;
};

struct FI_ClassData  /* find window */
{
   struct FI_GUIData GUI;
   long              Abort;
};

enum FastSearch { FS_NONE=0, FS_FROM, FS_TO, FS_CC, FS_REPLYTO, FS_SUBJECT, FS_DATE, FS_SIZE };
enum SearchMode { SM_FROM=0, SM_TO, SM_CC, SM_REPLYTO, SM_SUBJECT, SM_DATE, SM_HEADLINE,
                  SM_SIZE, SM_HEADER, SM_BODY, SM_WHOLE, SM_STATUS };

struct Search
{
   char *          Pattern;
   struct Rule *   Rule;
   long            Size;
   enum SearchMode Mode;
   int             PersMode;
   int             Compare;
   enum MailStatus Status;
   enum FastSearch Fast;
   BOOL            CaseSens;
   BOOL            SubString;
   char            Match[SIZE_PATTERN+4];
   char            PatBuf[(SIZE_PATTERN+4)*2+2]; // ParsePattern() needs at least 2*source+2 bytes buffer
   char            Field[SIZE_DEFAULT];
   struct DateTime DT;
   struct Data2D   List;
};

extern struct Hook FI_OpenHook;
extern int         Mode2Group[12];

APTR FI_ConstructSearchGroup(struct SearchGroup *gdata, BOOL remote);
BOOL FI_DoComplexSearch(struct Search *search1, int combine, struct Search *search2, struct Mail *mail);
BOOL FI_PrepareSearch(struct Search *search, enum SearchMode mode, BOOL casesens, int persmode, int compar, enum MailStatus stat, BOOL substr, char *match, char *field);
void FI_SearchGhost(struct SearchGroup *gdata, BOOL disabled);

#endif /* YAM_FIND_H */

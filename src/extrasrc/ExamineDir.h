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

#ifndef EXAMINEDIR_H
#define EXAMINEDIR_H 1

#include <exec/types.h>

// This is just a minimal implementation of OS4's new directoy scanning API.
// We only provide the informations needed by YAM here. If more information
// is needed during directory scanning we will have to extend that.
struct ExamineData
{
  LONG Type;
  LONG FileSize;
  STRPTR Name;
};

#define EXD_IS_FILE(ed)              ((ed)->Type < 0)
#define EXD_IS_DIRECTORY(ed)         ((ed)->Type >= 0 && (ed)->Type != ST_SOFTLINK)

#define EX_Dummy                     (TAG_USER+4711)
#define EX_StringName                (EX_Dummy+1)
#define EX_DataFields                (EX_Dummy+2)
#define EX_MatchString               (EX_Dummy+3)
#define EX_DoCurrentDir              (EX_Dummy+4)

#define EXF_NAME                     (1<<0)

#endif /* EXAMINEDIR_H */

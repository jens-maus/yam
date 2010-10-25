#ifndef YAM_MAIL_LEX_H
#define YAM_MAIL_LEX_H

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

#include <exec/types.h>

enum tokenType
{
  tSPACE = 1,
  tTAB,
  tBEGINPAREN,
  tENDPAREN,
  tLESS,
  tGREATER,
  tAMPERSAND,
  tNEWLINE,
  tSTAR,
  tSLASH,
  tUNDERSCORE,
  tHASH,
  tSB,
  tTSB,
  tCITE,
  tEMAIL,
  tHTTP,
  tHTTPS,
  tFTP,
  tFILE,
  tGOPHER,
  tTELNET,
  tMAILTO,
  tNEWS,
  tURL,
  tNORMALTEXT,
  tSIGNATURE,
  tENDSIGNATURE,
  tBOLD,
  tITALIC,
  tUNDERLINE,
  tCOLORED,
  tNEXTPART
};

char *ParseEmailText(const char *mailTxt, const BOOL handleSigDash,
                     const BOOL useStyles, const BOOL useColors);

enum tokenType ExtractURL(const char *text, char **resultBuffer);

#endif /* YAM_MAIL_LEX_H */

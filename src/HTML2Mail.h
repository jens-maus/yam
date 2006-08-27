#ifndef HTML2MAIL_H
#define HTML2MAIL_H

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

 $Id: YAM_mail_lex.h 2046 2006-03-13 12:13:58Z damato $

***************************************************************************/

enum htmlTagType
{
  ht_PARAGRAPH = 1,
  ht_BR,
  ht_HR,
  ht_UNKNOWN,
  ht_NBSP,
  ht_AMP,
  ht_GT,
  ht_LT,
  ht_QUOT,
  ht_AUML,
  ht_OUML,
  ht_UUML,
  ht_SZLIG,
  ht_SUP2,
  ht_SUP3,
  ht_REG,
  ht_COPY,
  ht_UNKNOWN_CHAR,
  ht_NORMALTEXT
};

char *html2mail(char *htmlTxt);

#endif /* HTML2MAIL_H */

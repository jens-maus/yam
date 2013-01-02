#ifndef HTML2MAIL_H
#define HTML2MAIL_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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

enum htmlTagType
{
  ht_PARAGRAPH = 1,
  ht_BR,
  ht_DIV,
  ht_HR,
  ht_STYLE,
  ht_BOLD,
  ht_BOLD_END,
  ht_ITALIC,
  ht_ITALIC_END,
  ht_UNDERLINE,
  ht_UNDERLINE_END,
  ht_COMMENT,
  ht_HREF,
  ht_HREF_END,
  ht_DD,
  ht_DL,
  ht_DT,
  ht_UL,
  ht_LI,
  ht_LI_END,
  ht_LI_IN,
  ht_PRE,
  ht_HIGHLIGHT,
  ht_HIGHLIGHT_END,
  ht_TITLE,
  ht_TITLE_END,
  ht_SPACE,
  ht_UNKNOWN,

  ht_SP = 32, /* do not change! */
  ht_EXCL,
  ht_QUOT,
  ht_NUM,
  ht_DOLLAR,
  ht_PERCNT,
  ht_AMP,
  ht_APOS,
  ht_LPAR,
  ht_RPAR,
  ht_AST,
  ht_PLUS,
  ht_COMMA,
  ht_HYPHEN,
  ht_PERIOD,
  ht_SOL,
  ht_COLON = 58,
  ht_SEMI,
  ht_LT,
  ht_EQUALS,
  ht_GT,
  ht_QUEST,
  ht_COMMAT,
  ht_LSGB = 91,
  ht_BSOL,
  ht_RSGB,
  ht_CIRC,
  ht_LOWBAR,
  ht_GRAVE,
  ht_LCUB = 123,
  ht_VERBAR,
  ht_RCUB,
  ht_TILDE,

  ht_NBSP = 160,
  ht_IEXCL,
  ht_CENT,
  ht_POUND,
  ht_CURREN,
  ht_YEN,
  ht_BRKBAR,
  ht_SECT,
  ht_UML,
  ht_COPY,
  ht_ORDF,
  ht_LAQUO,
  ht_NOT,
  ht_SHY,
  ht_REG,
  ht_MACR,
  ht_DEG,
  ht_PLUSMN,
  ht_SUP2,
  ht_SUP3,
  ht_ACUTE,
  ht_MICRO,
  ht_PARA,
  ht_MIDDOT,
  ht_CEDIL,
  ht_SUP1,
  ht_ORDM,
  ht_RAQUO,
  ht_FRAC14,
  ht_FRAC12,
  ht_FRAC34,
  ht_IQUEST,

  ht_AGRAVE = 192,
  ht_AACUTE,
  ht_ACIRC,
  ht_ATILDE,
  ht_AUML,
  ht_ARING,
  ht_AELING,
  ht_CCEDIL,
  ht_EGRAVE,
  ht_EACUTE,
  ht_ECIRC,
  ht_EUML,
  ht_IGRAVE,
  ht_IACUTE,
  ht_ICIRC,
  ht_IUML,
  ht_ETH,
  ht_NTILDE,
  ht_OGRAVE,
  ht_OACUTE,
  ht_OCIRC,
  ht_OTILDE,
  ht_OUML,
  ht_TIMES,
  ht_OSLASH,
  ht_UGRAVE,
  ht_UACUTE,
  ht_UCIRC,
  ht_UUML,
  ht_YACUTE,
  ht_THORN,
  ht_SZLIG,

  ht_DIVIDE = 247,
  ht_YUML = 255,

  ht_TRADE,

  ht_ASCII_CHAR,
  ht_UNKNOWN_CHAR,

  ht_NORMALTEXT
};

char *html2mail(char *htmlTxt);

#endif /* HTML2MAIL_H */

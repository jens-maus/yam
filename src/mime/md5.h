#ifndef MD5_H
#define MD5_H

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

// forward declarations

// MD5 message digest routines
struct MD5Context
{
  unsigned long state[4];
  unsigned long count[2];
  unsigned char buffer[64];
};

void md5init(struct MD5Context *ctx);
void md5update(struct MD5Context *ctx, const void *buf, const unsigned int len);
void md5final(unsigned char digest[16], struct MD5Context *ctx);
void md5hmac(unsigned char *text, int text_len, unsigned char *key, int key_len, unsigned char digest[16]);
void md5digestToHex(const unsigned char digest[16], char *hex);

#endif // MD5_H

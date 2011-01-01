#ifndef QPRINTABLE_H
#define QPRINTABLE_H

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
struct codeset;

// quoted-printable encoding/decoding routines
long qpencode_file(FILE *in, FILE *out);
long qpdecode_file(FILE *in, FILE *out, struct codeset *srcCodeset);

// macros & static variables
static const char basis_hex[] = "0123456789ABCDEF";
static const unsigned char index_hex[128] =
{
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
   0,   1,  2,  3,  4,  5,  6, 7,   8,  9,255,255,255,255,255,255,
  255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255, 10, 11, 12, 13, 14, 15,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
};
#define hexchar(c)  index_hex[(c) & 0x7F]

// The following table and macros were taken from the GMIME project`s CVS
// and are used to help analyzing ASCII characters and to which special
// group of characters they belong. While decoding/endcoding MIME messages
// this table can be quiet helpful because it helps analyzing text
// in terms of RFC compliance more precisly.
static const unsigned short specials_table[256] =
{
//  0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
    5,  5,  5,  5,  5,  5,  5,  5,  5,103,  7,  5,  5, 39,  5,  5, // 0
    5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5, // 1
  242,448, 76,192,192,192,192,192, 76, 76,448,448, 76,448, 72,324, // 2
  448,448,448,448,448,448,448,448,448,448, 76, 76, 76,260, 76, 68, // 3
   76,448,448,448,448,448,448,448,448,448,448,448,448,448,448,448, // 4
  448,448,448,448,448,448,448,448,448,448,448,108,236,108,192,320, // 5
  192,448,448,448,448,448,448,448,448,448,448,448,448,448,448,448, // 6
  448,448,448,448,448,448,448,448,448,448,448,192,192,192,192,  5, // 7
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 8
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // 9
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // A
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // B
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // C
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // D
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, // E
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  // F
};

enum {
  IS_CTRL         = (1 << 0),
  IS_LWSP         = (1 << 1),
  IS_TSPECIAL     = (1 << 2),
  IS_SPECIAL      = (1 << 3),
  IS_SPACE        = (1 << 4),
  IS_DSPECIAL     = (1 << 5),
  IS_QPSAFE       = (1 << 6),
  IS_ESAFE        = (1 << 7), // encoded word safe
  IS_PSAFE        = (1 << 8)  // encode word in phrase safe
};

#define is_type(x, t)   ((specials_table[(unsigned char)(x)] & (t)) != 0)
#define is_ctrl(x)      ((specials_table[(unsigned char)(x)] & IS_CTRL) != 0)
#define is_lwsp(x)      ((specials_table[(unsigned char)(x)] & IS_LWSP) != 0)
#define is_tspecial(x)  ((specials_table[(unsigned char)(x)] & IS_TSPECIAL) != 0)
#define is_ttoken(x)    ((specials_table[(unsigned char)(x)] & (IS_TSPECIAL|IS_LWSP|IS_CTRL)) == 0)
#define is_atom(x)      ((specials_table[(unsigned char)(x)] & (IS_SPECIAL|IS_SPACE|IS_CTRL)) == 0)
#define is_dtext(x)     ((specials_table[(unsigned char)(x)] & IS_DSPECIAL) == 0)
#define is_fieldname(x) ((specials_table[(unsigned char)(x)] & (IS_CTRL|IS_SPACE)) == 0)
#define is_qpsafe(x)    ((specials_table[(unsigned char)(x)] & IS_QPSAFE) != 0)
#define is_esafe(x)     ((specials_table[(unsigned char)(x)] & IS_ESAFE) != 0)
#define is_psafe(x)     ((specials_table[(unsigned char)(x)] & IS_PSAFE) != 0)

#endif // QPRINTABLE_H

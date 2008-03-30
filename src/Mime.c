/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include <clib/alib_protos.h>

#include <proto/exec.h>
#include <proto/codesets.h>

#include "extrasrc.h"

#include "YAM.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_utilities.h"

#include "Locale.h"
#include "Mime.h"

#include "Debug.h"

/* local */
static int rfc2047_decode_int(const char *text,
                              int (*func)(const char *, unsigned int, const char *, const char *, void *),
                              void *arg);
static int rfc2047_dec_callback(const char *txt, unsigned int len, const char *chset,
                                const char *lang, void *arg);
INLINE char *rfc2047_search_quote(const char **ptr);
static int rfc2231_decode_int(char *dst, const char *src, struct codeset *srcCodeset);


/***************************************************************************
 Module: MIME I/O routines
***************************************************************************/

/// Global variables
static const char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char basis_hex[] = "0123456789ABCDEF";

static const unsigned char index_64[128] =
{
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
  255,255,255,255,255,255,255,255,255,255,255, 62,255,255,255, 63,
   52, 53, 54, 55, 56, 57, 58, 59, 60, 61,255,255,255,255,255,255,
  255,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,255,255,255,255,255,
  255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,255,255,255,255,255
};

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

// some defines that can be usefull
#define B64_LINELEN 72    // number of chars before the b64encode_file() issues a CRLF
#define B64DEC_BUF  4096  // bytes to use as a base64 file decoding buffer
#define B64ENC_BUF  4095  // bytes to use as a base64 file encoding buffer (must be a multiple of 3)

#define QP_LINELEN  76    // number of chars before qpencode_file() issues a CRLF
#define QPENC_BUF   4096  // bytes to use as a quoted-printable file encoding buffer
#define QPDEC_BUF   4096  // bytes to use as a quoted-printable file decoding buffer

#define UUENC_OWIDTH (60+3)                 // (60+3) length of the output string: +3 for len+checksum+newline char
#define UUENC_IWIDTH ((UUENC_OWIDTH-3)/4*3) // (45)   input length of data that should be uuencoded
#define UUENC_OBUF   (UUENC_OWIDTH*100)     // (6200) bytes to use as a uucode output buffer
#define UUENC_IBUF   (UUENC_IWIDTH*100)     // (4500) bytes to use as a uucode input buffer
#define UUDEC_IBUF   UUENC_OBUF             // bytes to use as a uucode input decoding buffer
#define UUDEC_OBUF   UUENC_IBUF             // bytes to use as a uucode output decoding buffer
#define UUDEC_BUF    4096                   // bytes to use as a uucode file decoding buffer
#define UUMAX_CHAR   64                     // the maximum value of a char to hit c + 32 = 96

#define UUENCODE_CHAR(c)  (((c) & 0x3F) ? ((c) & 0x3F) + ' ' : '`')
#define UUDECODE_CHAR(c)  (((c) - ' ') & 0x3F)

#define hexchar(c)    index_hex[(c) & 0x7F]

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

#define ENCWORD_LEN    75                  // max. encoded-word length (rfc2047 2.0)

#define CHARS_LWSP     " \t\n\r"           // linear whitespace chars
#define CHARS_TSPECIAL "()<>@,;:\\\"/[]?="
#define CHARS_SPECIAL  "()<>@,;:\\\".[]"
#define CHARS_CSPECIAL "()\\\r"            // not in comments
#define CHARS_DSPECIAL "[]\\\r \t"         // not in domains
#define CHARS_ESPECIAL "()<>@,;:\"/[]?.=_" // encoded word specials (rfc2047 5.1)
#define CHARS_PSPECIAL "!*+-/=_"           // encoded phrase specials (rfc2047 5.3)

///

/*** Helper functions for codesets.library ***/
/// strippedCharsetName()
// return the charset code stripped and without any white spaces
char *strippedCharsetName(const struct codeset* codeset)
{
  char *strStart = TrimStart(codeset->name);
  char *strEnd = strchr(strStart, ' ');

  if(strEnd > strStart || strStart > codeset->name)
  {
    static char strippedName[SIZE_CTYPE+1];

    if(strEnd > strStart && (size_t)(strEnd-strStart) < sizeof(strippedName))
      strlcpy(strippedName, strStart, strEnd-strStart+1);
    else
      strlcpy(strippedName, strStart, sizeof(strippedName));

    return strippedName;
  }
  else
    return codeset->name;
}

///

/*** BASE64 encode/decode routines (RFC 2045) ***/
/// base64encode()
// optimized base64 encoding function returning the length of the
// encoded string.
int base64encode(char *to, const unsigned char *from, unsigned int len)
{
  char *fromp = (char*)from;
  char *top = to;
  unsigned char cbyte;
  unsigned char obyte;
  char end[3];

  for (; len >= 3; len -= 3)
  {
    cbyte = *fromp++;
    *top++ = basis_64[(int)(cbyte >> 2)];
    obyte = (cbyte << 4) & 0x30;            /* 0011 0000 */

    cbyte = *fromp++;
    obyte |= (cbyte >> 4);                  /* 0000 1111 */
    *top++ = basis_64[(int)obyte];
    obyte = (cbyte << 2) & 0x3C;            /* 0011 1100 */

    cbyte = *fromp++;
    obyte |= (cbyte >> 6);                  /* 0000 0011 */
    *top++ = basis_64[(int)obyte];
    *top++ = basis_64[(int)(cbyte & 0x3F)]; /* 0011 1111 */
  }

  if(len)
  {
    end[0] = *fromp++;
    if(--len) end[1] = *fromp++; else end[1] = 0;
    end[2] = 0;

    cbyte = end[0];
    *top++ = basis_64[(int)(cbyte >> 2)];
    obyte = (cbyte << 4) & 0x30;            /* 0011 0000 */

    cbyte = end[1];
    obyte |= (cbyte >> 4);
    *top++ = basis_64[(int)obyte];
    obyte = (cbyte << 2) & 0x3C;            /* 0011 1100 */

    if(len) *top++ = basis_64[(int)obyte];
    else *top++ = '=';
    *top++ = '=';
  }

  *top = 0;
  return top - to;
}

///
/// base64decode()
// optimized base64 decoding function returning the length of the
// decoded string or 0 on an occurred error or a minus length integer as
// an indicator of a short count in the encoded string. The source
// string doesn`t have to be NUL-terminated and only 'len' characters
// are going to be decoded. The decoding also stops as soon as the
// ending padding '==' or '=' characters are found.
int base64decode(char *to, const unsigned char *from, unsigned int len)
{
  int result = 0;
  unsigned char *fromp = (unsigned char *)from;
  char *top = to;

  ENTER();

  while(len >= 4)
  {
    unsigned char x;
    unsigned char y;

    // decrease len in advance
    len--;

    // get the first char, check if it is a valid b64 char and
    // convert it accordingly to index_64[]
    x = *fromp++;
    if(x > 127 || (x = index_64[x]) == 255)
      break; // error

    // get the second char, check if it is a valid b64 char and
    // convert it accordingly to index_64[]
    y = *fromp++;
    if(y == '\0' || y > 127 || (y = index_64[y]) == 255)
      break; // error

    len--;

    // put the decoded b64 char into the output buffer.
    *top++ = (x << 2) | (y >> 4);

    // if we still have something left in the input buffer,
    // we go on with our decoding
    if(len > 0)
    {
      len--;

      // get next char
      x = *fromp++;

      // check char for the padding character '='
      if(x == '=')
      {
        // check if there is still something left
        // and if so it just have to be the padding char
        if((len > 0 && *fromp++ != '='))
          break; // error

        len--;

        // we received the padding string
        // lets break out here
        break; // everything fine
      }
      else
      {
        // it isn't the padding char, so is it a valid
        // b64 character instead?
        if(x > 127 || (x = index_64[x]) == 255)
          break; // error

        // put the second decoded b64 char into our output
        // buffer
        *top++ = (y << 4) | (x >> 2);

        // and check if there is something left again..
        if(len > 0)
        {
          len--;

          // get next char
          y = *fromp++;

          // is that char a padding char?
          if(y == '=')
          {
            // we received the padding string
            // lets break out here
            break; // everything fine
          }
          else if(y > 127 || (y = index_64[y]) == 255) // char valid b64?
            break; // error
          else
            *top++ = (x << 6) | y; // decode the third char as it is valid
        }
      }
    }
  }

  // make sure the string is
  // NUL-terminated
  *top = '\0';

  // if len is still > 0 it is a sign that the
  // base64 decoding aborted. So we return a minus
  // value to signal that short item count (error).
  if(len > 0)
    result = -(top-to);
  else
    result = top-to;

  RETURN(result);
  return result;
}

///
/// base64encode_file()
//  Encodes a file in base64 format. It reads in a file from a supplied FILE*
//  pointer stepwise by filling up a buffer, encoding it and writing it down
//  as soon as it reached the length of 72 characters. This makes sure the
//  base64 encoded parts can be embeded into an RFC822 compliant mail
//  It returns the total number of encoded characters written to the destination
//  file.
long base64encode_file(FILE *in, FILE *out, BOOL convLF)
{
  char inbuffer[B64ENC_BUF*2+2];  // we use a buffer of 8192 bytes here because we read out
                                  // data in 4095 byte chunks out of file 'in' and as we
                                  // probably need to convert each LF into a CRLF we have to
                                  // have a buffer with a maximum space of 8190 bytes.
                                  // the other 2 bytes are to be safe. :)
  char outbuffer[B64ENC_BUF*3];   // if we read in a maximum of 8190 bytes we will get out
                                  // a base64 encoded string with a maximum of 11064 bytes
                                  // but normally the routines shouldn`t occupy more than
                                  // ~5600 bytes because we normally won`t have tons of LF`s
                                  // in there. But by alloãating BUF*3 we should be on the safe
                                  // side.

  char *optr;
  BOOL eof_reached = FALSE;
  int next_unget = 0;
  int missing_chars = 0;
  int sumencoded = 0;
  int towrite;
  int encoded;
  size_t read = 0;

  ENTER();
  SHOWVALUE(DBF_MIME, convLF);

  while(eof_reached == FALSE)
  {
    // before we go on with reading in more data we move
    // the last next_unget characters of inbuffer to the start
    // of inbuffer
    if(next_unget > 0)
      memmove(inbuffer, &inbuffer[read], next_unget);

    // read in 4095 byte chunks
    read = fread(&inbuffer[0]+next_unget, 1, B64ENC_BUF-next_unget, in);
    read += next_unget;
    next_unget = 0;

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != B64ENC_BUF)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // now we check whether the user want to convert each LF into a CRLF
    // and if so we need to parse the whole read bytes for \n and convert
    // them to \r\n before the base64 encoding.
    if(convLF)
    {
      char convbuffer[B64ENC_BUF*2+2];
      char *sptr = convbuffer;
      char *dptr = inbuffer;
      long toconvert = read;
      long converted = 0;

      // lets fill the convbuffer with the data
      // of inbuffer first
      memcpy(convbuffer, inbuffer, toconvert);

      while(toconvert--)
      {
        if(*sptr == '\n')
        {
          // now write a \r first
          *dptr = '\r';
          dptr++;

          converted++;
        }

        // copy the current character;
        *dptr = *sptr;

        // increase the pointers
        dptr++;
        sptr++;
      }

      // increase the read counter
      read += converted;

      // now that we have converted something we have to
      // make sure that read is still a multiple of 3 if this
      // isn`t an EOF run.
      if(eof_reached == FALSE)
      {
        // lets check how many chars we have to skip and move
        // back later
        next_unget = read % 3;
        read -= next_unget;
      }
    }

    // now everything should be prepared so that we can call the
    // base64 encoding routine and let it convert our inbuffer to
    // the apropiate outbuffer
    encoded = base64encode(outbuffer, (unsigned char *)inbuffer, read);
    sumencoded += encoded;

    // if the base64encoding routine returns <= 0 then there is obviously
    // something wrong
    if(encoded <= 0)
    {
      E(DBF_MIME, "error on encoding data!");

      RETURN(-1);
      return -1;
    }

    // now that we seem to have everything encoded we write out
    // the encoded sting in 72 character long chunks followed by
    // a newline
    optr = outbuffer;
    towrite = encoded;

    while(towrite > 0)
    {
      size_t todo;

      // how many chars should be written?
      if(missing_chars == 0)
      {
        if(towrite >= B64_LINELEN)
        {
          todo = B64_LINELEN;
        }
        else
          todo = towrite;
      }
      else
        todo = towrite < missing_chars ? towrite : missing_chars;

      // now we do a binary write of the data
      if(fwrite(optr, 1, todo, out) != todo)
      {
        E(DBF_MIME, "error on writing data!");

        // an error must have occurred.
        RETURN(-1);
        return -1;
      }

      // lets modify our counters
      towrite -= todo;
      optr += todo;

      // then we have to check whether we have written
      // a full 72 char long line or not and if so we can attach
      // a newline.
      if(missing_chars == 0 &&
         todo < B64_LINELEN && eof_reached == FALSE)
      {
        // if we end up here we don`t write any newline,
        // but we remember how many characters we are
        // going to write in advance next time.
        missing_chars = B64_LINELEN-todo;
      }
      else if((towrite > 0 || eof_reached == FALSE) && fputc('\n', out) == EOF)
      {
        E(DBF_MIME, "error on writing newline");

        RETURN(-1);
        return -1;
      }
      else missing_chars = 0;
    }
  }

  RETURN(sumencoded);
  return sumencoded;
}

///
/// base64decode_file()
//  Decodes a file in base64 format. Takes care of an eventually specified translation
//  table as well as a CRLF->LF translation for printable text. It reads in the base64
//  strings line by line from the in file stream, decodes it and writes out the
//  decoded data with fwrite() to the out stream. It returns the total bytes of
//  written (decoded) data. In case of an error it returns -1 and in case it
//  found a short item count during decoding it return -2 asking the user
//  to still consider the string decoded (however it should be treated with
//  care)
long base64decode_file(FILE *in, FILE *out,
                       struct codeset *srcCodeset, BOOL convCRLF)
{
  char inbuffer[B64DEC_BUF+1];
  char outbuffer[B64DEC_BUF/4*3];
  char ungetbuf[3];
  long decodedChars = 0;
  size_t next_unget = 0;
  BOOL eof_reached = FALSE;
  BOOL problemDuringDecode = FALSE;

  ENTER();

  while(eof_reached == FALSE)
  {
    int outLength = 0;
    char *sptr;
    char *dptr;
    size_t read;
    size_t todo;

    // if we do have some unget chars lets copy them first at the
    // beginning of the inbuffer
    if(next_unget > 0)
      memcpy(inbuffer, ungetbuf, next_unget);

    // do a binary read of ~4096 chunks
    read = fread(&inbuffer[next_unget], sizeof(char), B64DEC_BUF-next_unget, in);

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != B64DEC_BUF-next_unget)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0 && next_unget == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // increase/reset the counters
    read += next_unget;
    next_unget = 0;

    // now that we have read 4096 bytes into our buffer
    // we have to iterate through this buffer and "eliminate"
    // white spaces which aren`t normally part of base64 encoded
    // string and can be safely skipped without
    // corrupting the decoded file.
    sptr = inbuffer;
    dptr = inbuffer;
    todo = read;

    while(todo > 0)
    {
      if(!isspace(*sptr))
      {
        *dptr = *sptr;
        dptr++;
      }
      else read--;

      sptr++;
      todo--;
    }

    // if we end up with read == 0 we had only spaces in our
    // source string, so lets skip to the next iteration
    if(read == 0)
      continue;

    // before we going to decode the string we have to make sure
    // that the encoded string is a multiple of 4 as 4 encoded
    // base64 chars will get out 2 unencoded ones.
    next_unget = read % 4;
    if(next_unget > 0)
    {
      if(eof_reached == FALSE)
      {
        read -= next_unget;
        memcpy(ungetbuf, &inbuffer[read], next_unget);
      }
      else
      {
        W(DBF_MIME, "unget chars at EOF???");

        problemDuringDecode = TRUE;
      }
    }

    // now that we have a whitespace free somewhat base64 encoded
    // string, we can call the base64decode() function to finally
    // decode the string
    if(read <= 0 ||
       (outLength = base64decode(outbuffer, (unsigned char *)inbuffer, read)) <= 0)
    {
      E(DBF_MIME, "error on decoding: %ld %ld", read, outLength);

      if(outLength < 0)
      {
        // we faced a short item count. That can actually be a sign that the text
        // in question is not a fully base64 compliant string. However, to
        // at least display the text to the user we redefine the outLength and
        // let the write function output that string (even if not correctly
        // decoded)
        outLength = -outLength;

        problemDuringDecode = TRUE;
      }
      else
      {
        // it should not happen that we face a shortCount
        // or error
        RETURN(-1);
        return -1;
      }
    }

    #if defined(DEBUG)
    if(outLength > B64DEC_BUF/4*3)
    {
      E(DBF_MIME, "Error: outLength exceeds outbuffer boundaries!");

      RETURN(-1);
      return -1;
    }
    #endif

    // in case the user wants us to detect the correct cyrillic codeset
    // we do it now, but just if the source codeset isn't UTF-8
    if(convCRLF && C->DetectCyrillic &&
       (srcCodeset == NULL || stricmp(srcCodeset->name, "utf-8") != 0))
    {
      struct codeset *cs = CodesetsFindBest(CSA_Source,         outbuffer,
                                            CSA_SourceLen,      outLength,
                                            CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                            TAG_DONE);

      if(cs != NULL && cs != srcCodeset)
        srcCodeset = cs;
    }

    // if the caller supplied a source codeset, we have to
    // make sure we convert our outbuffer before writing it out
    // to the file into our local charset
    if(srcCodeset)
    {
      ULONG strLen = 0;
      STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                      CSA_DestCodeset,   G->localCharset,
                                      CSA_Source,        outbuffer,
                                      CSA_SourceLen,     outLength,
                                      CSA_DestLenPtr,    &strLen,
                                      TAG_DONE);

      if(str && strLen > 0)
      {
        // if we end up here we successfully converted the
        // sourcebuffer to a destination buffer which complies to our local
        // charset
        dptr = str;
        outLength = strLen;
      }
      else
      {
        W(DBF_MIME, "error while trying to convert base64decoded string to local charset!");

        dptr = outbuffer;
      }
    }
    else
      dptr = outbuffer;

    // if the user also wants to convert CRLF to LF only,
    // we do it right now
    if(convCRLF)
    {
      long r;
      char *rc = dptr;
      char *wc = dptr;

      for(r=0; r < outLength; r++, rc++)
      {
        // check if this is a CRLF
        if(*rc == '\r' &&
           outLength-r > 1 && rc[1] == '\n')
        {
          // if so, skip the \r
          continue;
        }
        else
        {
          // if no CRLF is found, lets copy
          // the plain character
          *wc = *rc;

          // increase the write counter
          wc++;
        }
      }

      // make sure we reduce outLength by the
      // number of "overjumped" chars.
      outLength -= (rc-wc);
    }

    // now that we got the string decoded we write it into
    // our file
    if(fwrite(dptr, sizeof(char), (size_t)outLength, out) != (size_t)outLength)
    {
      E(DBF_MIME, "error on writing data!");

      // an error occurred while writing...
      RETURN(-1);
      return -1;
    }

    // in case the dptr buffer was allocated by codesets.library,
    // we have to free it now
    if(dptr != outbuffer)
      CodesetsFreeA(dptr, NULL);

    // increase the decodedChars counter
    decodedChars += outLength;
  }

  // if there was a problem during
  // the decoding phase we go and warn the user with a
  // return value of -2
  if(problemDuringDecode)
    decodedChars = -2;

  RETURN(decodedChars);
  return decodedChars;
}

///

/*** Quoted-Printable encode/decode routines (RFC 2045) ***/
/// qpencode_file()
// Encodes a whole file using the quoted-printable format defined in
// RFC 2045 (page 19)
long qpencode_file(FILE *in, FILE *out)
{
  unsigned char inbuffer[QPENC_BUF+1]; // we read out data in ~4096 byte chunks
  unsigned char outbuffer[QPENC_BUF+5];// the output buffer should not be more than
                                       // the input buffer with an additional space
                                       // for 5 chars which could be used during
                                       // encoding.

  unsigned char *iptr;
  unsigned char *optr = outbuffer;
  unsigned char c;
  int last = -1;
  long encoded_chars = 0;
  int line_len = 0;
  BOOL eof_reached = FALSE;
  size_t read;

  ENTER();

  while(eof_reached == FALSE)
  {
    // read in 4096 byte chunks
    read = fread(inbuffer, 1, QPENC_BUF, in);

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != QPENC_BUF)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // let us now parse through the inbuffer and encode it according
    // to RFC 2045
    iptr = inbuffer;

    while(read)
    {
      // decrease the read number and increase
      // out input buffer pointer
      c = *iptr++;
      read--;

      if(c == '\n')
      {
        // check if the previous char is a linear whitespace and
        // if so we have to put a soft break right before the
        // newline
        if(last != -1 && (last == ' ' || last == '\t'))
        {
          *optr++ = '=';
          *optr++ = '\n';
        }

        *optr++ = '\n';

        // reset the line_len counter
        line_len = 0;
        last = -1;
      }
        // we encode the current char if:
        // 1) it is an unsafe safe
        // 2) it is an upcoming "From " at the start of a line
      else if(!is_qpsafe(c) ||
              (last == -1 && c == 'F' && strncmp((char *)iptr, "rom ", 4) == 0))
      {

        // before we can encode the data we have to check
        // whether there is enough space left on the line
        // or if we have to put it on the next line
        if(line_len+3 >= QP_LINELEN-1) // one space for the trailing '='
        {
          *optr++ = '=';
          *optr++ = '\n';

          // reset the line_len counter
          line_len = 0;
        }

        // now put out the encoded char
        *optr++ = '=';
        *optr++ = basis_hex[(c >> 4) & 0xF];
        *optr++ = basis_hex[c & 0xF];

        // increase the line_len counter
        line_len += 3;

        // count the number of encoded chars
        encoded_chars++;
      }
      else
      {
        // so this char seems to be safe to be directly placed
        // in the output buffer without any encoding. We just
        // have to check whether this line is going to be longer
        // than the limit
        if(line_len+1 >= QP_LINELEN-1) // one space for the trailing '='
        {
          *optr++ = '=';
          *optr++ = '\n';

          // reset the line_len counter
          line_len = 0;
        }

        *optr++ = c;

        // increase the line_len counter
        line_len++;
      }

      // let us now check if our outbuffer is filled up so that we can write
      // out the data to our out stream.
      if(optr-outbuffer >= QPENC_BUF)
      {
        size_t todo = optr-outbuffer;

        // now we do a binary write of the data
        if(fwrite(outbuffer, 1, todo, out) != todo)
        {
          E(DBF_MIME, "error on writing data!");

          // an error must have occurred.
          RETURN(-1);
          return -1;
        }

        // now reset the outbuffer and stuff
        optr = outbuffer;
      }

      last = c;
    }
  }

  // check if there is something in the outbuffer that
  // hasn't been written out yet
  if(optr-outbuffer > 0)
  {
    size_t todo = optr-outbuffer;

    // now we do a binary write of the data
    if(fwrite(outbuffer, 1, todo, out) != todo)
    {
      E(DBF_MIME, "error on writing data!");

      // an error must have occurred.
      RETURN(-1);
      return -1;
    }
  }

  RETURN(encoded_chars);
  return encoded_chars;
}

///
/// qpdecode_file()
// Decodes a whole file using the quoted-printable format defined in
// RFC 2045 (page 19)
long qpdecode_file(FILE *in, FILE *out, struct codeset *srcCodeset)
{
  unsigned char inbuffer[QPDEC_BUF+1]; // lets use a 4096 byte large input buffer
  unsigned char outbuffer[QPDEC_BUF+1];// to speed things up we use the same amount
                                       // of memory for an output buffer as the
                                       // decoded string can't be larger than
                                       // the encoded one.

  unsigned char *iptr;
  unsigned char *optr = outbuffer;
  unsigned char c;
  size_t read = 0;
  size_t next_unget = 0;
  long decoded = 0;
  int result = 0;
  BOOL eof_reached = FALSE;

  ENTER();

  while(eof_reached == FALSE)
  {
    // do a binary read of ~4096 chunks
    read = fread(&inbuffer[next_unget], sizeof(char), QPDEC_BUF-next_unget, in);

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != QPDEC_BUF-next_unget)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0 && next_unget == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // increase/reset the counters
    read += next_unget;
    next_unget = 0;

    // now that we have read in our buffer we have to parse through
    // it and decode eventually existing quoted printable encoded
    // chunks. The routines also analyze the data and returns an
    // error if non quoted-printable safe data is found, however
    // it still tries to decode the data until the end. This fail-safe
    // behaviour is suggested in RFC 2045 on page 22.
    iptr = inbuffer;

    while(read)
    {
      c = *iptr++;
      read--;

      if(c == '=')
      {
        // check if the next char is a newline so that
        // we can skip the current =
        if(read && *iptr == '\n')
        {
          // skip the newline..
          iptr++;
          read--;

          continue;
        }

        // a '=' is the sign that a encoded string is following, so
        // let us check if we have enough space in our input buffer
        // and then decode it accordingly
        if(read >= 2)
        {
          unsigned char c1 = hexchar(*iptr);
          unsigned char c2 = hexchar(*(iptr+1));

          // so we have enough space, lets decode it, but let us
          // check if the two chars are really hexadecimal chars
          if(c1 != 255 && c2 != 255)
          {
            *optr++ = c1<<4 | c2;

            // increase the counters
            iptr += 2;
            read -= 2;

            // count the decoded chars
            decoded++;
          }
          else
          {
            // as suggested by RFC 2045 we keep the =XX sequence
            // and report a warning later to the user
            *optr++ = c;
            *optr++ = *iptr++;
            *optr++ = *iptr++;
            read -= 2;
            result = -3; // indicate a "decoding warning"
          }
        }
        else
        {
          // ok, there isn't enough space in the input buffer
          // so we break out here and parse the stuff on
          // the next iteration
          next_unget = read+1;
          memcpy(inbuffer, iptr-1, next_unget);

          break;
        }
      }
      else if(!isascii(c) ||
              (is_ctrl(c) && c != '\t' && c != '\n' &&
               c == '\r' && *iptr != '\n'))
      {
        // we found some not allowed char, so lets ignore it
        // but warn the user
        result = -4; // indicate a "unallowed control chars" warning
      }
      else
      {
        // the current char seems to be a normal
        // char, so lets output it
        *optr++ = c;
      }

      // let us now check if our outbuffer is filled up so that we can write
      // out the data to our out stream.
      if(optr-outbuffer >= QPDEC_BUF)
      {
        unsigned char *dptr = outbuffer;
        size_t todo = optr-outbuffer;

        // in case the user wants us to detect the correct cyrillic codeset
        // we do it now
        if(C->DetectCyrillic &&
           (srcCodeset == NULL || stricmp(srcCodeset->name, "utf-8") != 0))
        {
          struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                                CSA_SourceLen,      todo,
                                                CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                                TAG_DONE);

          if(cs != NULL && cs != srcCodeset)
            srcCodeset = cs;
        }

        // if the caller supplied a source codeset, we have to
        // make sure we convert our outbuffer before writing it out
        // to the file into our local charset
        if(srcCodeset)
        {
          ULONG strLen = 0;
          STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                          CSA_DestCodeset,   G->localCharset,
                                          CSA_Source,        dptr,
                                          CSA_SourceLen,     todo,
                                          CSA_DestLenPtr,    &strLen,
                                          TAG_DONE);

          if(str && strLen > 0)
          {
            // if we end up here we successfully converted the
            // sourcebuffer to a destination buffer which complies to our local
            // charset
            dptr = (unsigned char *)str;
            todo = strLen;
          }
          else
            W(DBF_MIME, "error while trying to convert qpdecoded string to local charset!");
        }

        // now we do a binary write of the data
        if(fwrite(dptr, 1, todo, out) != todo)
        {
          E(DBF_MIME, "error on writing data!");

          // in case the dptr buffer was allocated by codesets.library,
          // we have to free it now
          if(dptr != outbuffer)
            CodesetsFreeA(dptr, NULL);

          // an error must have occurred.
          RETURN(-1);
          return -1;
        }

        // in case the dptr buffer was allocated by codesets.library,
        // we have to free it now
        if(dptr != outbuffer)
          CodesetsFreeA(dptr, NULL);

        // now reset the outbuffer and stuff
        optr = outbuffer;
      }
    }
  }

  // check if there is something in the outbuffer that
  // hasn't been written out yet
  if(optr-outbuffer > 0)
  {
    unsigned char *dptr = outbuffer;
    size_t todo = optr-outbuffer;

    // in case the user wants us to detect the correct cyrillic codeset
    // we do it now
    if(C->DetectCyrillic &&
       (srcCodeset == NULL || stricmp(srcCodeset->name, "utf-8") != 0))
    {
      struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                            CSA_SourceLen,      todo,
                                            CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                            TAG_DONE);

      if(cs != NULL && cs != srcCodeset)
        srcCodeset = cs;
    }

    // if the caller supplied a source codeset, we have to
    // make sure we convert our outbuffer before writing it out
    // to the file into our local charset
    if(srcCodeset)
    {
      ULONG strLen = 0;
      STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                      CSA_DestCodeset,   G->localCharset,
                                      CSA_Source,        dptr,
                                      CSA_SourceLen,     todo,
                                      CSA_DestLenPtr,    &strLen,
                                      TAG_DONE);

      if(str && strLen > 0)
      {
        // if we end up here we successfully converted the
        // sourcebuffer to a destination buffer which complies to our local
        // charset
        dptr = (unsigned char *)str;
        todo = strLen;
      }
      else
        W(DBF_MIME, "error while trying to convert qpdecoded string to local charset!");
    }

    // now we do a binary write of the data
    if(fwrite(dptr, 1, todo, out) != todo)
    {
      E(DBF_MIME, "error on writing data!");

      // in case the dptr buffer was allocated by codesets.library,
      // we have to free it now
      if(dptr != outbuffer)
        CodesetsFreeA(dptr, NULL);

      // an error must have occurred.
      RETURN(-1);
      return -1;
    }

    // in case the dptr buffer was allocated by codesets.library,
    // we have to free it now
    if(dptr != outbuffer)
      CodesetsFreeA(dptr, NULL);
  }

  // if we end up here and read > 0 then the decoding wasn't finished
  // and we have to return an error
  if(read > 0)
  {
    RETURN(-2);
    return -2; // -2 means "unfinished decoding"
  }

  // on success lets return the number of decoded
  // chars
  RETURN(result == 0 ? decoded : result);
  return result == 0 ? decoded : result;
}

///

/*** UU encode/decode routines ***/
/// uuencode_file()
// Encodes a whole file using the good-old UUEncode algorithm which isn't
// suprisingly defined in any RFC out there. So here we use a slightly, but
// fully compatible approach of what is defined in the original BSD UUcode
// definition. In addition to the "normal" UU encoding we add a checksum
// char to every encoded line so that during decoding a mailer could check
// the integrity of the UU encoded string and warn the user accordingly.
long uuencode_file(FILE *in, FILE *out)
{
  unsigned char inbuffer[UUENC_IBUF+1]; // we read out data in ~4500 byte chunks
  unsigned char outbuffer[UUENC_OBUF+1];// the output buffer
  unsigned char *iptr;
  unsigned char *optr = outbuffer;
  long encoded_chars = 0;
  BOOL eof_reached = FALSE;
  size_t read;

  ENTER();

  while(eof_reached == FALSE)
  {
    // read in 4096 byte chunks
    read = fread(inbuffer, 1, UUENC_IBUF, in);

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != UUENC_IBUF)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // let us now parse through the inbuffer and encode it according
    // to UUEncoding rules
    iptr = inbuffer;

    while(read > 0)
    {
      int i;
      int checksum = 0;

      // check if we can parse a whole 45 chars long input line
      // or if we have to enough less than that.
      if(read/UUENC_IWIDTH > 0)
        *optr++ = UUENCODE_CHAR(UUENC_IWIDTH);
      else
        *optr++ = UUENCODE_CHAR(read);

      // then we encode by reading out 3 bytes each until we haven't
      // enough bytes left and have to do it different or if we
      // hit the line limit if 45 input chars.
      for(i=0; read/3 > 0 && i < UUENC_IWIDTH; i+=3)
      {
        unsigned char c1 = *iptr++;
        unsigned char c2 = *iptr++;
        unsigned char c3 = *iptr++;

        // encode the three chars (c1,c2,c3) into four output
        // chars by forming groups of 6 bits like the UUcode
        // definition defines it.
        *optr++ = UUENCODE_CHAR(c1 >> 2);
        *optr++ = UUENCODE_CHAR((c1 << 4) | ((c2 >> 4) & 0xF));
        *optr++ = UUENCODE_CHAR((c2 << 2) | ((c3 >> 6) & 0x3));
        *optr++ = UUENCODE_CHAR(c3);

        // calculate an incremental checksum
        checksum += (c1+c2+c3) % UUMAX_CHAR;

        encoded_chars += 4;
        read -= 3;
      }

      // check why we quit the above loop. If we quit it because
      // we hit the maximum input line length or if there was
      // nothing left to read we just write out the checksum
      // and a newline to finish the current line
      if(i >= UUENC_IWIDTH || read == 0)
      {
        *optr++ = UUENCODE_CHAR(checksum);
        *optr++ = '\n';
      }
      else if(read > 0)
      {
        unsigned char c1 = *iptr++;
        unsigned char c2 = 0;

        // if we hit this branch, then we have still some data to
        // process, but there are < 3 chars left, so we make it
        // easy and start encoding iterative.
        *optr++ = UUENCODE_CHAR(c1 >> 2);
        read--;

        // check if we still one byte left so that we can
        // include it into the decoding.
        if(read > 0)
        {
          c2 = *iptr++;
          read--;
        }

        // ok, and as it can't be more than 2 bytes we got, we
        // can simply encode the virtual 3 bytes into the rest
        // of the 3 bytes here.
        *optr++ = UUENCODE_CHAR((c1 << 4) | ((c2 >> 4) & 0xF));
        *optr++ = UUENCODE_CHAR(c2 << 2);
        *optr++ = '`';

        encoded_chars += 4;

        // put out a checksum char aswell and a finalizing newline
        checksum += (c1+c2) % UUMAX_CHAR;
        *optr++ = UUENCODE_CHAR(checksum);
        *optr++ = '\n';
      }

      // let us now check if our outbuffer is filled up so that we can write
      // out the data to our out stream.
      if(optr-outbuffer >= UUENC_OBUF ||
         eof_reached == TRUE)
      {
        size_t todo = optr-outbuffer;

        // make sure that we haven't overwritten the outbuffer
        // or otherwise some memory will be corrupted for sure.
        ASSERT(todo <= UUENC_OBUF);

        // now we do a binary write of the data
        if(fwrite(outbuffer, 1, todo, out) != todo)
        {
          E(DBF_MIME, "error on writing data!");

          // an error must have occurred.
          RETURN(-1);
          return -1;
        }

        // now reset the outbuffer and stuff
        optr = outbuffer;
      }
    }
  }

  RETURN(encoded_chars);
  return encoded_chars;
}

///
/// uudecode_file()
// Decode a UUencoded file using separate input/output buffers to speed up
// processing. It also takes respect of eventually existing checksums and
// tries to validate the UUencoded file to conform to the BSD standard or
// otherwise return an error/warning by returning negative values.
long uudecode_file(FILE *in, FILE *out, struct codeset *srcCodeset)
{
  unsigned char inbuffer[UUDEC_IBUF+1]; // we read out data in ~4500 byte chunks
  unsigned char outbuffer[UUDEC_OBUF+1];// the output buffer
  unsigned char *iptr;
  unsigned char *optr = outbuffer;
  size_t read;
  size_t next_unget = 0;
  long decoded = 0;
  int line_len = 0;
  int result = 0;
  int checksum = 0;
  BOOL eof_reached = FALSE;

  ENTER();

  // before we start with our decoding we have to search for
  // the starting "begin XXX" line
  do
  {
    if(fgets((char *)inbuffer, UUDEC_IBUF, in) != 0)
    {
      // check if this line start with "begin " and if so
      // break out and continue decoding the real data
      if(strncmp((char *)inbuffer, "begin ", 6) == 0)
        break;
    }
    else if(feof(in) != 0)
    {
      RETURN(-2);
      return -2; // -2 means "no UUcode start found"
    }
    else
    {
      RETURN(-1);
      return -1; // -1 means unexpected error
    }
  }
  while(TRUE);

  // start decoding the "real" data.
  while(eof_reached == FALSE)
  {
    // do a binary read of a multiple of UUDEC_IBUF
    read = fread(&inbuffer[next_unget], sizeof(char), UUDEC_IBUF-next_unget, in);

    // on a short item count we check for a potential
    // error and return immediatly.
    if(read != UUDEC_IBUF-next_unget)
    {
      if(feof(in) != 0)
      {
        D(DBF_MIME, "EOF file at %ld", ftell(in));

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0 && next_unget == 0)
          break;
      }
      else
      {
        E(DBF_MIME, "error on reading data!");

        // an error occurred, lets return -1
        RETURN(-1);
        return -1;
      }
    }

    // increase/reset the counters
    read += next_unget;
    next_unget = 0;

    // now that we have read in our buffer we have to parse through
    // it and decode all chars according to the uudecoding rules
    // of the UUcode encoding.
    iptr = inbuffer;

    while(read)
    {
      if(line_len == 0)
      {
        unsigned char c = *iptr++;
        read--;

        // skip whitespaces on a fresh line
        while(is_lwsp(c) && read)
        {
          c = *iptr++;
          read--;
        }

        // if the line length counter is zero we haven't read the
        // first byte to check how long the line is going to be
        line_len = UUDECODE_CHAR(c);
        if(line_len == 0)
        {
          unsigned char *cptr;

          // lets check whether this is just the sign that we
          // are at the end of our data or if it is an error
          if(*iptr == '\n' ||
            (*iptr == '`' && *(iptr+1) == '\n'))
          {
            // ok, we seem to have found the ending ' on a UUcode
            // line, so lets check if we have the finalizing "end"
            if(*iptr == '`')
            {
              iptr += 2;
              read -= 2;
            }
            else
            {
              iptr++;
              read--;
            }

            // check if there is enough space
            if(read < 3 && eof_reached == FALSE)
            {
              // copy back the rest of the stuff
              memcpy(inbuffer, iptr, read);

              // do a small binary read
              read += fread(&inbuffer[read], sizeof(char), 3-read, in);

              iptr = inbuffer;
            }

            // set the checkpointer
            cptr = iptr;

            // check again
            if(read < 3 || strncmp((char *)cptr, "end", 3) != 0)
            {
              // if we end up here then there isn't enough
              // data left for checking the finalizing "end"
              // or we just didn't find it, but somehow we were
              // able to decode all our data, so lets just drop
              // the user a warning
              result = -6; // -6 means "no end tag"
            }

            // set eof to let the outer loop terminate
            eof_reached = TRUE;
            break;
          }
          else
          {
            E(DBF_MIME, "error: invalid length ID");

            result = -3; // -3 means "invalid length ID"
          }
        }

        // clear our checksum
        checksum = 0;
      }
      else
      {
        while(line_len && read/4 > 0)
        {
          int tempsum;
          unsigned char c;
          unsigned char c1 = UUDECODE_CHAR(*iptr++);
          unsigned char c2 = UUDECODE_CHAR(*iptr++);
          unsigned char c3 = UUDECODE_CHAR(*iptr++);
          unsigned char c4 = UUDECODE_CHAR(*iptr++);

          // now that we have our input chars we can
          // decode them to our output buffer directly
          tempsum = c = c1 << 2 | c2 >> 4;
          *optr++ = c;
          line_len--;
          decoded++;

          if(line_len)
          {
            tempsum += c = c2 << 4 | c3 >> 2;
            *optr++ = c;
            line_len--;
            decoded++;

            if(line_len)
            {
              tempsum += c = c3 << 6 | c4;
              *optr++ = c;
              line_len--;
              decoded++;
            }
          }

          // calculate the checksum aswell
          checksum += tempsum % UUMAX_CHAR;

          // decrease the read counter
          read -= 4;
        }

        // let us now check if our outbuffer is filled up so that we can write
        // out the data to our out stream.
        if(optr-outbuffer >= UUDEC_OBUF)
        {
          unsigned char *dptr = outbuffer;
          size_t todo = optr-outbuffer;

          // in case the user wants us to detect the correct cyrillic codeset
          // we do it now
          if(C->DetectCyrillic &&
             (srcCodeset == NULL || stricmp(srcCodeset->name, "utf-8") != 0))
          {
            struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                                  CSA_SourceLen,      todo,
                                                  CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                                  TAG_DONE);

            if(cs != NULL && cs != srcCodeset)
              srcCodeset = cs;
          }

          // if the caller supplied a source codeset, we have to
          // make sure we convert our outbuffer before writing it out
          // to the file into our local charset
          if(srcCodeset)
          {
            ULONG strLen = 0;
            STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                            CSA_DestCodeset,   G->localCharset,
                                            CSA_Source,        dptr,
                                            CSA_SourceLen,     todo,
                                            CSA_DestLenPtr,    &strLen,
                                            TAG_DONE);

            if(str && strLen > 0)
            {
              // if we end up here we successfully converted the
              // sourcebuffer to a destination buffer which complies to our local
              // charset
              dptr = (unsigned char *)str;
              todo = strLen;
            }
            else
              W(DBF_MIME, "error while trying to convert uudecoded string to local charset!");
          }

          // now we do a binary write of the data
          if(fwrite(dptr, 1, todo, out) != todo)
          {
            E(DBF_MIME, "error on writing data!");

            // in case the dptr buffer was allocated by codesets.library,
            // we have to free it now
            if(dptr != outbuffer)
              CodesetsFreeA(dptr, NULL);

            // an error must have occurred.
            RETURN(-1);
            return -1;
          }

          // in case the dptr buffer was allocated by codesets.library,
          // we have to free it now
          if(dptr != outbuffer)
            CodesetsFreeA(dptr, NULL);

          // now reset the outbuffer and stuff
          optr = outbuffer;
        }

        // if line_len == 0 then we probably read through
        // our expected end of the line, so lets check if
        // the next char is either a newline or another
        // uuencoded char, which could be the checksum
        // of our input char
        if(line_len == 0)
        {
          unsigned char last = *iptr;

          if(last == '\n')
          {
            // there seems to be no checksum on this line
            // so lets go on without checking it.
            iptr++;
            read--;
          }
          else if(last > ' ' && last <= '`' &&
                  *(iptr+1) == '\n')
          {
            // check if our calculated checksum is
            // identical to the last char found
            if(UUENCODE_CHAR(checksum) != last)
            {
              E(DBF_MIME, "wrong checksum: %ld:%ld != %ld", checksum, UUENCODE_CHAR(checksum), last);

              // the checksum seems to be wrong
              // so lets signal it on exiting this
              // function
              result = -4; // -4 means "wrong checksum"
            }

            iptr += 2;
            read -= 2;
          }
          else
          {
            E(DBF_MIME, "error: no newline or no checksum found at end");

            // something serious must have happend
            // as either the last char isn't a newline
            // or a checksum is wrong, so lets exit with
            // an error immediatly
            RETURN(-5);
            return -5; // -5 means corrupted UUcode string found
          }
        }
        else if(read > 0)
        {
          // ok, there isn't enough studd in the input buffer
          // so we break out here and parse the stuff on
          // the next iteration
          next_unget = read;
          memcpy(inbuffer, iptr, next_unget);

          break;
        }
      }
    }
  }

  // check if there is something in the outbuffer that
  // hasn't been written out yet
  if(optr-outbuffer > 0)
  {
    unsigned char *dptr = outbuffer;
    size_t todo = optr-outbuffer;

    // in case the user wants us to detect the correct cyrillic codeset
    // we do it now
    if(C->DetectCyrillic &&
       (srcCodeset == NULL || stricmp(srcCodeset->name, "utf-8") != 0))
    {
      struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                            CSA_SourceLen,      todo,
                                            CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                            TAG_DONE);

      if(cs != NULL && cs != srcCodeset)
        srcCodeset = cs;
    }

    // if the caller supplied a source codeset, we have to
    // make sure we convert our outbuffer before writing it out
    // to the file into our local charset
    if(srcCodeset)
    {
      ULONG strLen = 0;
      STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                      CSA_DestCodeset,   G->localCharset,
                                      CSA_Source,        dptr,
                                      CSA_SourceLen,     todo,
                                      CSA_DestLenPtr,    &strLen,
                                      TAG_DONE);

      if(str && strLen > 0)
      {
        // if we end up here we successfully converted the
        // sourcebuffer to a destination buffer which complies to our local
        // charset
        dptr = (unsigned char *)str;
        todo = strLen;
      }
      else
        W(DBF_MIME, "error while trying to convert uudecoded string to local charset!");
    }

    // now we do a binary write of the data
    if(fwrite(dptr, 1, todo, out) != todo)
    {
      E(DBF_MIME, "error on writing data!");

      // in case the dptr buffer was allocated by codesets.library,
      // we have to free it now
      if(dptr != outbuffer)
        CodesetsFreeA(dptr, NULL);

      // an error must have occurred.
      RETURN(-1);
      return -1;
    }

    // in case the dptr buffer was allocated by codesets.library,
    // we have to free it now
    if(dptr != outbuffer)
      CodesetsFreeA(dptr, NULL);
  }

  // on success lets return the number of decoded
  // chars
  RETURN(result == 0 ? decoded : result);
  return result == 0 ? decoded : result;
}

///

/*** RFC 2047 MIME encoding/decoding routines ***/
/// RFC 2047 routines
// The following RFC2047 encoding/decoding routines are highly inspired and
// partly adopted from the Courier IMAP project available at:
//
//  http://www.sourceforge.net/projects/courier
//
// whereas the RFC2047 related code is available at:
//
//  http://courier.cvs.sourceforge.net/courier/libs/rfc822/rfc2047.c
//

#define ISSPACE(i) ((i)=='\t' || (i)=='\r' || (i)=='\n' || (i)==' ')
#define DOENCODE(i) (((i) & 0x80) || (i)=='"' || (i)=='=' || \
      ((unsigned char)(i) < 0x20 && !ISSPACE(i)) || \
      !(*qp_allow)(i))
#define DOENCODEWORD(c) \
  (((c) & 0x80) || (c) == '"' || (unsigned char)(c) <= 0x20 || \
   (c) == '_' || (c) == '=' || (c) == '?' || !(*qp_allow)(c))

///
/// rfc2047_encode_qp()
// RFC2047 quoted-printable string encoding routines. It takes a source string
// and encodes the string according to the RFC2047 standard in the quoted
// printable format.
static int rfc2047_encode_qp(const char *str, size_t len, const char *charset,
                             int (*qp_allow)(const char),
                             int (*func)(const char *, size_t, void *), void *arg,
                             size_t foldlen)
{
  int rc = 0;
  size_t maxlen;
  size_t i = len;
  size_t c = 0;
  size_t clen;

  ENTER();

  // Output mimeified text, insert spaces at 70+ character
  // boundaries for line wrapping.
  clen = strlen(charset);
  maxlen = clen+10;

  if(maxlen < 65)
    maxlen = foldlen + 4 - maxlen;
  else
    maxlen = 10;

  while(i > 0)
  {
    if(c == 0)
    {
      // begin a new encoded word
      if((rc = (*func)("=?", 2, arg)) != 0 ||
         (rc = (*func)(charset, clen, arg)) != 0 ||
         (rc = (*func)("?Q?", 3, arg)) != 0)
      {
        break;
      }

      c += clen+3+2;
    }

    if(DOENCODEWORD(*str))
    {
      char buf[3];

      buf[0] = '=';
      buf[1] = basis_hex[ ( *str >> 4) & 0x0F ];
      buf[2] = basis_hex[ *str & 0x0F ];

      if((rc = (*str == ' ') ? (*func)("_", 1, arg) : (*func)(buf, 3, arg)) != 0)
        break;

      c += (*str == ' ') ? 1 : 3;

      ++str;
      --i;
    }
    else
    {
      size_t j;

      for(j=0; j < i && DOENCODEWORD(str[j]) == FALSE; j++)
      {
        if((j + c) >= maxlen)
          break;
      }

      if((rc = (*func)(str, j, arg)) != 0)
        break;

      c += j;
      str += j;
      i -= j;
    }

    if(i == 0 || c >= maxlen)
    {
      if((rc = (*func)("?= ", (i != 0) ? 3 : 2, arg)) != 0)
        break;

      c=0;
    }
  }

  RETURN(rc);
  return rc;
}

///
/// rfc2047_encode_base64()
// RFC2047 base64 string encoding routines. It takes a source string and encodes
// the string according to the rules in RFC2047, but with the bas64 encoding
static int rfc2047_encode_base64(const char *ptr, size_t len, const char *charset,
                                 int (*func)(const char *, size_t, void *), void *arg,
                                 size_t foldlen)
{
  int rc = 0;
  size_t clen;

  ENTER();

  clen = strlen(charset);

  while(len)
  {
    size_t i;

    if((rc=(*func)("=?", 2, arg)) ||
       (rc=(*func)(charset, clen, arg))||
       (rc=(*func)("?B?", 3, arg)))
    {
      break;
    }

    i = 2 + clen + 3;

    while(len)
    {
      unsigned char ibuf[3];
      char obuf[4];
      size_t n=len > 3 ? 3:len;

      ibuf[0]= ptr[0];

      if(n>1)
        ibuf[1]=ptr[1];
      else
        ibuf[1]=0;

      if(n>2)
        ibuf[2]=ptr[2];
      else
        ibuf[2]=0;

      ptr += n;
      len -= n;

      obuf[0] = basis_64[ ibuf[0]        >>2 ];
      obuf[1] = basis_64[(ibuf[0] & 0x03)<<4|ibuf[1]>>4];
      obuf[2] = basis_64[(ibuf[1] & 0x0F)<<2|ibuf[2]>>6];
      obuf[3] = basis_64[ ibuf[2] & 0x3F ];

      if(n < 2)
        obuf[2] = '=';

      if(n < 3)
        obuf[3] = '=';

      if((rc=(*func)(obuf, 4, arg)))
        break;

      i += 4;

      // check that the encoded word does not get to large
      if(foldlen && i + 2 > foldlen - 1 + 4)
        break;
    }

    if(rc != 0)
      break;

    // Encoded-words must be separated by
    // a linear-white-space.
    if((rc = (*func)("?= ", len > 0 ? 3 : 2, arg)))
      break;
  }

  RETURN(rc);
  return rc;
}

///
/// rfc2047_encode_callback()
// The main callback function of the RFC2047 encoding. It takes a source
// string and a charset definition. In addition, it takes a qp_allow()
// function pointer which is a function that returns 1 if a character is
// to be accepted for the QP encoding or not. In addition, it takes
// a storage function pointer (func) which is called for every character
// that was encoded and should be written into a string.
static int rfc2047_encode_callback(const char *str, const char *charset,
                                   int (*qp_allow)(const char),
                                   int (*func)(const char *, size_t, void *),
                                   void *arg)
{
  int rc = 0;

  ENTER();

  if(str != NULL && str[0] != '\0')
  {
    // output quoted-printable-encoded
    while(*str != '\0')
    {
      size_t i;
      size_t j;
      size_t n;
      size_t c;

      for(i=0; str[i] != '\0'; i++)
      {
        if(!ISSPACE((int)(unsigned char)str[i]) && DOENCODE(str[i]))
          break;
      }

      if(str[i] == '\0')
      {
        rc = (i ? (*func)(str, i, arg) : 0);
        break;
      }

      // Find start of word
      while(i)
      {
        --i;

        if(ISSPACE((int)(unsigned char)str[i]))
        {
          ++i;
          break;
        }
      }

      if(i > 0)
      {
        if((rc = (*func)(str, i, arg)) != 0)
          break;

        str += i;
      }

      // Figure out when to stop MIME decoding. Consecutive
      // MIME-encoded words are MIME-encoded together.
      for(i=0;;)
      {
        for(; str[i] != '\0'; i++)
        {
          if(ISSPACE((int)(unsigned char)str[i]))
            break;
        }

        if(str[i] == '\0')
          break;

        for(c=i; str[c] != '\0' && ISSPACE((int)(unsigned char)str[c]); ++c)
          ; // nothing

        for(; str[c] != '\0'; c++)
        {
          if(ISSPACE((int)(unsigned char)str[c]) || DOENCODE(str[c]))
            break;
        }

        if(str[c] == '\0' || ISSPACE((int)(unsigned char)str[c]))
          break;

        i=c;
      }

      // Figure out whether base64 is a better choice.
      for(n=0, j=0; j < i; j++)
      {
        if(DOENCODEWORD(str[j]))
          ++n;
      }

      // check the ratio and decide which encoding to take
      if(n > i/10)
        rc = rfc2047_encode_base64(str, i, charset, func, arg, 70);
      else
        rc = rfc2047_encode_qp(str, i, charset, qp_allow, func, arg, 70);

      if(rc != 0)
        break;

      str += i;
    }
  }

  RETURN(rc);
  return rc;
}

///
/// rfc2047_count_char()
// a function that can be used as one of the callback functions
// for rfc2047_encode_callback(). Here it simply counts the number of
// chars.
static int rfc2047_count_char(const char *c UNUSED, size_t l, void *p)
{
  size_t *i=(size_t *)p;
  *i += l;
  return 0;
}

///
/// rfc2047_save_char()
// a function that can be used as one of the callback functions
// for rfc2047_encode_callback(). This function stores the supplied
// characters in the destination string.
static int rfc2047_save_char(const char *c, size_t l, void *p)
{
  char **s=(char **)p;
  memcpy(*s, c, l);
  *s += l;
  return 0;
}

///
/// rfc2047_encode_str()
// Special RFC2047 string encoding routine which directly encodes a
// supplied string according to the RFC2047 standard. Please note that this
// function encodes everything in a target string while automatically
// taking care of eventually required spaces due to the maximum line limits
// defined in the RFC822/RFC2047 standards. It tries to only encode those
// passages which really require any encoding due to non US-ASCII chars.
static char *rfc2047_encode_str(const char *str, const char *charset,
                                int (*qp_allow)(const char c))
{
  size_t i=1;
  char *s;

  ENTER();

  // let us first count how much chars we are going to produce
  rfc2047_encode_callback(str, charset, qp_allow, &rfc2047_count_char, &i);
  if((s = malloc(i)) != NULL)
  {
    char *p = s;
  
    // now do the real work
    rfc2047_encode_callback(str, charset, qp_allow, &rfc2047_save_char, &p);
    *p = '\0';
  }

  RETURN(s);
  return s;
}

///
/// rfc2047_qp_allow_any()
// This query function always returns 1 to signal the encode_callback
// function that it should accept all characters.
static int rfc2047_qp_allow_any(const char c UNUSED)
{
  return 1;
}

///
/// rfc2047_qp_allow_comment()
/*
static int rfc2047_qp_allow_comment(const char c)
{
  if(c == '(' || c == ')' || c == '"')
    return 0;
  else
    return 1;
}
*/
///
/// rfc2047_qp_allow_word()
/*
static int rfc2047_qp_allow_word(const char c)
{
  return strchr(basis_64, c) != NULL ||
         strchr("*-=_", c) != NULL;
}
*/

///
/// rfc2047_encode_file()
// generic function that encodes a source str according to RFC2047 rules.
// It will use the encode_str function and then afterwards find out where
// exactly the encoded string should be split into different lines of
// maximum 75 chars long as the RFC2047 demands it.
int rfc2047_encode_file(FILE *fh, const char *src, const size_t offset)
{
  char *dst;

  ENTER();

  // call encode_str() to encode the source string into a valid
  // RFC2047 encoded string with enough spaces so that we can
  // split it later into separate lines not longer than 75 chars.
  if((dst = rfc2047_encode_str(src, strippedCharsetName(G->localCharset), &rfc2047_qp_allow_any)))
  {
    size_t len = strlen(dst);

    D(DBF_MIME, "encoded rfc2047 string: '%s'", dst);

    // now we split the dst string into different lines
    if(len >= 75-offset)
    {
      char *s = dst;
      char *e = dst;
      char *last_space = NULL;
      size_t c = offset;

      // start our search
      while(len > 0)
      {
        if(*e == ' ')
          last_space = e;

        // check if we need a newline and
        // if so we go and write out the last
        // stuff including a newline.
        if(c >= 75 && last_space != NULL)
        {
          fwrite(s, last_space-s, 1, fh);

          if(len > 1)
            fwrite("\n ", 2, 1, fh);

          s = last_space+1;
          c = e-s;
          last_space = NULL;
        }

        c++;
        e++;
        len--;
      }

      if(c > 0)
        fwrite(s, e-s, 1, fh);
    }
    else
      fwrite(dst, len, 1, fh);

    // free the dst string
    free(dst);
  }

  RETURN(0);
  return 0;
}

///
/// rfc2047_decode()
// decodes a rfc2047 encoded string and eventually translates
// each character of the decoded string according to the provided
// translation table.
struct rfc2047_decode_info
{
  char *dst;
  unsigned int maxlen;
};

// this function uses the decode_int() function and attachs a callback
// function to it which will be called each time a token has been
// decoded so that an eventually existing translation table can be used
// to translate the charsets. returns the number of translated chars or
// -1 if an malloc error occurred and -2 if some unknown encoding was
// specified in the text, -3 if the base64 decoding failed.
int rfc2047_decode(char *dst, const char *src, unsigned int maxlen)
{
  int result;
  // pack all the necessary information in the decode_info
  // structure so that the decode function can process it.
  struct rfc2047_decode_info info;
  info.dst    = dst;
  info.maxlen = maxlen;

  // call the decode_int function to start decoding our
  // data.
  result = rfc2047_decode_int(src, &rfc2047_dec_callback, &info);
  info.dst[0] = '\0'; // make sure this string is null-terminated

  // on success return the decoded string len.
  if(result > 0)  return (int)(maxlen-info.maxlen);
  else            return result;
}

///
/// rfc2047_dec_callback()
// the callback function that is called by the decode_int() function each
// time a string was successfully decoded so that it can be converted.
static int rfc2047_dec_callback(const char *txt, unsigned int len, const char *chset,
                                UNUSED const char *lang, void *arg)
{
  struct rfc2047_decode_info *info = (struct rfc2047_decode_info *)arg;

  // in case the user wants us to detect the correct cyrillic codeset
  // we do it now
  if(C->DetectCyrillic &&
     (chset == NULL || stricmp(chset, "utf-8") != 0))
  {
    struct codeset *cs = CodesetsFindBest(CSA_Source,         txt,
                                          CSA_SourceLen,      len,
                                          CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                          TAG_DONE);

    if(cs != NULL)
      chset = cs->name;
  }

  // now we try to get the src codeset from codesets.library
  // and convert the string into our local charset if required
  if(chset != NULL && chset[0] != '\0')
  {
    // check if the src codeset of the string isn't the same
    // like our local one.
    if(stricmp(chset, strippedCharsetName(G->localCharset)) != 0)
    {
      struct codeset *srcCodeset;

      if((srcCodeset = CodesetsFind((char *)chset,
                                    CSA_CodesetList,       G->codesetsList,
                                    CSA_FallbackToDefault, FALSE,
                                    TAG_DONE)) != NULL)
      {
        ULONG dstLen = 0;

        // now we convert the text from the source codeset to our
        // local codeset
        STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                        CSA_DestCodeset,   G->localCharset,
                                        CSA_Source,        txt,
                                        CSA_SourceLen,     len,
                                        CSA_DestLenPtr,    &dstLen,
                                        TAG_DONE);

        // now that we have our converted string we can go and
        // copy it over our destination buffer
        if(str)
        {
          // before we go on we check whether we have enough space
          // to put the txt in our destination
          if(info->maxlen < dstLen)
          {
            W(DBF_MIME, "not enough space to put converted string with len %ld into info->dst!", dstLen);

            CodesetsFreeA(str, NULL);
            return -1;
          }

          // so we can savely decrease it.
          info->maxlen -= dstLen;

          // lets copy all data from our converted string to
          // our own destination buffer
          memmove(info->dst, str, dstLen);

          // free our codesets buffer.
          CodesetsFreeA(str, NULL);

          // increase the destination pointer so that the
          // next iteration can profit out of it.
          info->dst += dstLen;

          // return without an error
          return 0;
        }
        else
          W(DBF_MIME, "couldn't convert src str via CodesetsConvertStr()!");
      }
      else
        W(DBF_MIME, "couldn't find charset '%s' in codesets.library!", chset);
    }
  }

  if(info->maxlen < len)
  {
    W(DBF_MIME, "not enough space to put string with len %ld into info->dst!", len);
    return -1;
  }

  // so we can savely decrease it.
  info->maxlen -= len;

  // lets copy all data from our converted string to
  // our own destination buffer
  memmove(info->dst, txt, len);

  // increase the destination pointer so that the
  // next iteration can profit out of it.
  info->dst += len;

  return 0;
}
///
/// rfc2047_search_quote()
//
INLINE char *rfc2047_search_quote(const char **ptr)
{
  const char *p = *ptr;
  size_t l;
  char *s;

  while(**ptr && **ptr != '?')
    ++(*ptr);

  l = *ptr - p;
  if((s = malloc(l + 1)) != NULL)
  {
    memcpy(s, p, l);
    s[l] = 0;
  }

  return (s);
}

///
/// rfc2047_decode_int()
// This is the main rcf2047 decoding function. It receives rfc2047-encoded
// text, and a callback function.  The callback function is repeatedly
// called, each time receiving a piece of decoded text.  The decoded
// info includes the text fragment, text length, the input charset, the
// language (if existant) and void pointer to user specific data for
// the processing. If the callback function returns non-zero, rfc2047
// decoding terminates, returning the result code.  Otherwise,
// rfc2047_decode returns 0 after a successfull decoding (-1 if malloc
// failed).
static int rfc2047_decode_int(const char *text,
                              int (*func)(const char *, unsigned int, const char *, const char *, void *),
                              void *arg)
{
  int rc, result=0;
  int unknown_enc=0;
  int had_last_word=0;
  const char *p;
  char *chset, *lang;
  char *encoding;
  char *enctext;

  while(text && *text)
  {
    p=text;

    if(*text != '=' || *(text+1) != '?')
    {
      while(*text)
      {
        // even if RFC2047 exactly defines which chars are allowed right before
        // an "=?" tag, we do accept all chars right before an "=?" tag for compatibility
        // reasons (even Thunderbird accepts something like 'Titel/Abs=?ISO-8859-1?Q?tract_f=FCr_Pap?=er Contribution')
        if(*text == '=' && *(text+1) == '?')
          break;

        if(!isspace((int)(unsigned char)*text))
          had_last_word=0;

        ++text;
      }

      if(text > p && !had_last_word)
      {
        rc = (*func)(p, (unsigned int)(text-p), 0, 0, arg);
        if(rc) return (rc);
      }

      continue;
    }

    text += 2;
    if((chset = rfc2047_search_quote(&text)) == 0)
      return -1;

    if(*text) ++text;
    if((encoding = rfc2047_search_quote(&text)) == 0)
    {
      free(chset);
      return -1;
    }

    if(*text) ++text;
    if((enctext = rfc2047_search_quote(&text)) == 0)
    {
      free(encoding);
      free(chset);

      return -1;
    }

    if(*text == '?' && text[1] == '=')
      text += 2;

    // now we check the encoding string.
    // q/Q defines to quoted-printable decoding
    // b/B defines to base64 encoding
    // and to sort out possible errors we check for the finalizing NUL-byte
    if(encoding[0] != '\0' && encoding[1] == '\0') // have to be 1 char long
    {
      switch(tolower(encoding[0]))
      {
        // we found a quoted-printable encoded rfc2047 compliant string, so
        // lets decode it.
        case 'q':
        {
          char *q, *r;

          for(q=r=enctext; *q; )
          {
            int c;

            if(*q == '=' && q[1] && q[2])
            {
              unsigned char c1 = hexchar(q[1]);
              unsigned char c2 = hexchar(q[2]);

              if(c1 != 255 && c2 != 255)
              {
                *r++ = c1<<4 | c2;
              }
              else
              {
                *r++ = *q;
                *r++ = q[1];
                *r++ = q[2];
              }

              q += 3;

              continue;
            }

            c = *q++;
            if(c == '_') c = ' ';
            *r++ = c ;
          }
          *r=0;
        }
        break;

        // we found a base64 encoded rfc2047 compliant string, so
        // lets decode it.
        case 'b':
        {
          int res = base64decode(enctext, (unsigned char *)enctext, strlen(enctext));

          if(res > 0)
            enctext[res] = '\0';
          else
          {
            result = -3; // signal an base64 decoding error.
            unknown_enc = 1;
          }
        }
        break;

        default:
        {
          // if we end up here the probabilty that we have a rfc2047 compliant
          // string with an unknown encoding is somehow high.
          result = -2; // signal unknown encoding to caller
          unknown_enc = 1;
        }
        break;
      }
    }
    else unknown_enc = 1;

    // if no error occurred we are going to call the callback function
    if(unknown_enc == 1)
    {
      rc = (*func)(p, (unsigned int)(text-p), 0, 0, arg);
      unknown_enc = 0; // clear it immediatly
    }
    else
    {
      // rfc2231 updates rfc2047 encoded words...
      // The ABNF given in RFC 2047 for encoded-words is:
      //   encoded-word := "=?" charset "?" encoding "?" encoded-text "?="
      // This specification changes this ABNF to:
      //   encoded-word := "=?" charset ["*" language] "?" encoding "?" encoded-
      if((lang = strrchr(chset, '*')))
        *lang++ = '\0';

      rc = (*func)(enctext, strlen(enctext), chset, lang, arg);
    }

    // free everything and check if a returncode was given.
    free(enctext);
    free(chset);
    free(encoding);
    if(rc) return rc;

    // Ignore blanks between enc words
    had_last_word=1;
  }

  return result;
}

///

/*** RFC 1738 URL encoding/decoding routines ***/
/// urlencode()
// URL encoding function returning the length of the encoded string.
int urlencode(char *to, const char *from, unsigned int len)
{
  char *optr = to;

  ENTER();

  // only continue of we have reasonable input
  // values.
  if(from && *from && to)
  {
    unsigned char *iptr = (unsigned char *)from;
    unsigned char c = *iptr;

    while(c != '\0' && (optr-to) < (int)len)
    {
      if(isalnum(c))
        *optr++ = c;
      else
      {
        // the char is not an alphanumeric character
        // so we need to encode it like "%XX" where XX is
        // the hexadecimal interpretation of it
        *optr++ = '%';
        *optr++ = basis_hex[(c >> 4) & 0xF];
        *optr++ = basis_hex[c & 0xF];
      }

      c = *(++iptr);
    }

    // NUL terminate the output string
    *optr = '\0';
  }

  RETURN((int)(optr-to));
  return optr-to;
}

///

/*** RFC 2231 MIME parameter encoding/decoding routines ***/
/// rfc2231_encode_file()
// Special encoding routines based on RFC2231 which defines how
// content parameters in a header field should be encoded.
// This function takes the actual file pointer, the string that
// should be encoded and the actual parameter name for which we
// encode 'str'.
int rfc2231_encode_file(FILE *fh, const char *paramName, const char *str)
{
  int num = 0;
  int outchars = 0;
  char buf[SIZE_DEFAULT];
  char *p = (char *)str;
  char *optr = &buf[0];

  ENTER();

  // we loop until we have processed the whole
  // input string
  while(*p)
  {
    unsigned char c = (unsigned char)*p;

    if(optr-buf == 0)
    {
      // if this is the very first parameter
      // part, we go and add the charset as well but no language
      // definition as we don't support that yet.
      if(num == 0)
        snprintf(buf, SIZE_DEFAULT, "\n\t%s*%d*=%s''", paramName, num++, strippedCharsetName(G->localCharset));
      else
        snprintf(buf, SIZE_DEFAULT, "\n\t%s*%d*=", paramName, num++);

      optr = buf+strlen(buf);
    }
    else if(optr-buf > 78-3) // max 78 chars line length
    {
      if(c)
        *optr++ = ';';

      // output our buffer to the file pointer
      if(fwrite(buf, optr-buf, 1, fh) == 0)
        break;

      outchars += optr-buf;
      optr = &buf[0];

      continue;
    }

    // encode the current char if it isn't an alphanumeric
    // one
    if(isalnum(c))
      *optr++ = c;
    else
    {
      // the char is not an alphanumeric character
      // so we need to encode it like "%XX" where XX is
      // the hexadecimal interpretation of it
      *optr++ = '%';
      *optr++ = basis_hex[(c >> 4) & 0xF];
      *optr++ = basis_hex[c & 0xF];
    }

    // iterate forward
    p++;
  }

  // if there is still something in out output buffer
  // we output it now
  if(optr-buf > 0)
  {
    fwrite(buf, optr-buf, 1, fh);
    outchars += optr-buf;
  }

  RETURN(outchars);
  return outchars;
}

///
/// rfc2231_decode()
// the main RFC 2231 decoding routine
int rfc2231_decode(char *attr, char *value, char **result, struct codeset **srcCodeset)
{
  int ret = 0;

  ENTER();

  if(attr[0] == '0' || attr[0] == '\0')
  {
    char *p;

    // we found the 'name*0=' parameter, so we have to
    // see if we can retrieve the charset and language
    // spec from our value
    if((p = strchr(value, '\'')))
    {
      *p = '\0';

      if(p-value > 0)
      {
        // search for the codeset via codesets.library
        *srcCodeset = CodesetsFind(value,
                                   CSA_CodesetList,       G->codesetsList,
                                   CSA_FallbackToDefault, FALSE,
                                   TAG_DONE);
      }

      // and we skip the language part as we don't
      // require it yet.
      if((p = strchr(++p, '\'')))
      {
        // nowe we decode the encoded string
        ret = rfc2231_decode_int(value, p+1, *srcCodeset);
      }
      else
        W(DBF_MIME, "malformed rfc2231 content parameter found! '%s'", value);
    }
    else
      W(DBF_MIME, "malformed rfc2231 content parameter found! '%s'", value);

    *result = value;
  }
  else if(isdigit(attr[0]))
  {
    // we found another parameter with number > 0
    // so we have to append what we decode next
    ret = rfc2231_decode_int(value, value, *srcCodeset);

    // now we concatenate
    if(*result)
    {
      int newSize = strlen(*result)+strlen(value)+1;

      if((*result = realloc(*result, newSize)))
      {
        strlcat(*result, value, newSize);

        free(value);
      }
    }
  }
  else
    *result = value;

  RETURN(ret);
  return ret;
}

///
/// rfc2231_decode_int()
// function which does the actual RFC2231 based string decoding
// (not including parameter concatenating)
static int rfc2231_decode_int(char *dst, const char *src, struct codeset *srcCodeset)
{
  char *p = (char *)src;
  char *q = dst;

  ENTER();

  while(*p)
  {
    if(*p == '%' && p[1] != '\0' && p[2] != '\0')
    {
      unsigned char c1 = hexchar(p[1]);
      unsigned char c2 = hexchar(p[2]);

      // so we have enough space, lets decode it, but let us
      // check if the two chars are really hexadecimal chars
      if(c1 != 255 && c2 != 255)
      {
        *dst++ = c1<<4 | c2;
        p += 3;
      }
      else
        *dst++ = *p++;
    }
    else
      *dst++ = *p++;
  }

  *dst = '\0';

  if(srcCodeset && srcCodeset != G->localCharset && p-src > 0)
  {
    STRPTR str = CodesetsConvertStr(CSA_SourceCodeset, srcCodeset,
                                    CSA_DestCodeset,   G->localCharset,
                                    CSA_Source,        q,
                                    TAG_DONE);

    if(str)
    {
      strcpy(q, str);
      CodesetsFreeA(str, NULL);
    }
    else
      W(DBF_MIME, "error while trying to convert rfc2231 decoded string to local charset!");
  }

  RETURN((int)(p-src));
  return p-src;
}

///

/**** MIME Types/Viewers ****/
/// IntMimeTypeArray[]
const struct IntMimeType IntMimeTypeArray[] =
{
  //                  ContentType                       Extensions          Description
  /* MT_TX_PLAIN */ { "text/plain",                     "txt asc",          MSG_CTtextplain },
  /* MT_TX_HTML  */ { "text/html",                      "html htm shtml",   MSG_CTtexthtml },
  /* MT_TX_XML   */ { "text/xml",                       "xml",              MSG_CTtextxml },
  /* MT_TX_GUIDE */ { "text/x-aguide",                  "guide",            MSG_CTtextaguide },
  /* MT_TX_GZIP  */ { "application/gzip",               "gz",               MSG_CTapplicationgz },
  /* MT_AP_HQX   */ { "application/mac-binhex40",       "hqx",              MSG_CTapplicationhqx },
  /* MT_AP_XLS   */ { "application/msexcel",            "xls xla",          MSG_CTapplicationxls },
  /* MT_AP_PPT   */ { "application/mspowerpoint",       "ppt ppz pps pot",  MSG_CTapplicationppt },
  /* MT_AP_PPT   */ { "application/msword",             "doc dot",          MSG_CTapplicationdoc },
  /* MT_AP_OCTET */ { "application/octet-stream",       "bin exe",          MSG_CTapplicationoctetstream },
  /* MT_AP_PS    */ { "application/postscript",         "ps eps ai",        MSG_CTapplicationpostscript },
  /* MT_AP_PDF   */ { "application/pdf",                "pdf",              MSG_CTapplicationpdf },
  /* MT_AP_PGP   */ { "application/pgp",                "pgp",              MSG_CTapplicationpgp },
  /* MT_AP_RTF   */ { "application/rtf",                "rtf",              MSG_CTapplicationrtf },
  /* MT_AP_BZ2   */ { "application/x-bzip2",            "bz2",              MSG_CTapplicationbz2 },
  /* MT_AP_Z     */ { "application/x-compress",         "z",                MSG_CTapplicationz },
  /* MT_AP_LHA   */ { "application/x-lha",              "lha",              MSG_CTapplicationlha },
  /* MT_AP_LZX   */ { "application/x-lzx",              "lzx",              MSG_CTapplicationlzx },
  /* MT_AP_TAR   */ { "application/x-tar",              "tar gtar",         MSG_CTapplicationtar },
  /* MT_AP_TGZ   */ { "application/x-tar-gz",           "tgz tar.gz",       MSG_CTapplicationtgz },
  /* MT_AP_AEXE  */ { "application/x-amiga-executable", NULL,               MSG_CTapplicationamigaexe },
  /* MT_AP_SCRIPT*/ { "application/x-amigados-script",  NULL,               MSG_CTapplicationadosscript },
  /* MT_AP_REXX  */ { "application/x-rexx",             "rexx rx",          MSG_CTapplicationrexx },
  /* MT_AP_ZIP   */ { "application/zip",                "zip",              MSG_CTapplicationzip },
  /* MT_IM_BMP   */ { "image/bmp",                      "bmp",              MSG_CTimagebmp },
  /* MT_IM_JPG   */ { "image/jpeg",                     "jpg jpeg",         MSG_CTimagejpeg },
  /* MT_IM_GIF   */ { "image/gif",                      "gif",              MSG_CTimagegif },
  /* MT_IM_PNG   */ { "image/png",                      "png",              MSG_CTimagepng },
  /* MT_IM_TIFF  */ { "image/tiff",                     "tif tiff",         MSG_CTimagetiff },
  /* MT_IM_ILBM  */ { "image/x-ilbm",                   "iff ilbm",         MSG_CTimageilbm },
  /* MT_AU_AU    */ { "audio/basic",                    "au snd",           MSG_CTaudiobasic },
  /* MT_AU_MPEG  */ { "audio/mpeg",                     "mp3 mp2",          MSG_CTaudiompeg },
  /* MT_AU_AIFF  */ { "audio/x-aiff",                   "aiff aif aifc",    MSG_CTaudioaiff },
  /* MT_AU_MIDI  */ { "audio/x-midi",                   "midi mid",         MSG_CTaudiomidi },
  /* MT_AU_8SVX  */ { "audio/x-8svx",                   "svx",              MSG_CTaudio8svx },
  /* MT_AU_WAV   */ { "audio/x-wav",                    "wav",              MSG_CTaudiowav },
  /* MT_VI_MPG   */ { "video/mpeg",                     "mpg mpeg",         MSG_CTvideompeg },
  /* MT_VI_MOV   */ { "video/quicktime",                "qt mov",           MSG_CTvideoquicktime },
  /* MT_VI_ANIM  */ { "video/x-anim",                   "anim",             MSG_CTvideoanim },
  /* MT_VI_AVI   */ { "video/x-msvideo",                "avi",              MSG_CTvideomsvideo },
  /* MT_ME_EMAIL */ { "message/rfc822",                 "eml",              MSG_CTmessagerfc822 },
                    { NULL,                             NULL,               NULL }
};

///
/// CreateNewMimeType
//  Initializes a new MIME type structure
struct MimeTypeNode *CreateNewMimeType(void)
{
  struct MimeTypeNode *mt;

  ENTER();

  if((mt = calloc(1, sizeof(struct MimeTypeNode))))
    strlcpy(mt->ContentType, "?/?", sizeof(mt->ContentType));

  RETURN(mt);
  return mt;
}

///
/// FreeMimeTypeList
void FreeMimeTypeList(struct MinList *mimeTypeList)
{
  ENTER();

  if(IsListEmpty((struct List *)mimeTypeList) == FALSE)
  {
    struct MinNode *curNode;

    // we have to free the mimeTypeList
    while((curNode = (struct MinNode *)RemHead((struct List *)mimeTypeList)) != NULL)
    {
      struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;

      free(mt);
    }

    NewList((struct List *)mimeTypeList);
  }

  LEAVE();
}

///
/// CompareMimeTypeLists
// compare two MIME type lists
BOOL CompareMimeTypeLists(const struct MinList *mtl1, const struct MinList *mtl2)
{
  BOOL equal = TRUE;
  struct MinNode *mln1 = mtl1->mlh_Head;
  struct MinNode *mln2 = mtl2->mlh_Head;

  ENTER();

  // walk through both lists in parallel and compare the single nodes
  while(mln1->mln_Succ != NULL && mln2->mln_Succ != NULL)
  {
    struct MimeTypeNode *mtn1 = (struct MimeTypeNode *)mln1;
    struct MimeTypeNode *mtn2 = (struct MimeTypeNode *)mln2;

    // compare every single member of the structure
    if(strcmp(mtn1->ContentType, mtn2->ContentType) != 0 ||
       strcmp(mtn1->Extension,   mtn2->Extension) != 0 ||
       strcmp(mtn1->Description, mtn2->Description) != 0 ||
       strcmp(mtn1->Command,     mtn2->Command) != 0)
    {
      // something does not match
      equal = FALSE;
      break;
    }
    mln1 = mln1->mln_Succ;
    mln2 = mln2->mln_Succ;
  }

  // if there are any nodes left then the two lists cannot be equal
  if(mln1->mln_Succ != NULL || mln2->mln_Succ != NULL)
  {
    equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///

/**** MD5 message-digest routines ***/
/// md5 message-digest
/***************************************************************************

 Original file description of md5 calculation part

 This code implements the MD5 message-digest algorithm.
 The algorithm is due to Ron Rivest.  This code was
 written by Colin Plumb in 1993, no copyright is claimed.
 This code is in the public domain; do with it what you wish.

 Equivalent code is available from RSA Data Security, Inc.
 This code has been tested against that, and is equivalent,
 except that you don't need to include two pages of legalese
 with every copy.

 To compute the message digest of a chunk of bytes, declare an
 MD5Context structure, pass it to md5init, call md5update as
 needed on buffers full of bytes, and then call md5final, which
 will fill a supplied 16-byte array with the digest.

***************************************************************************/

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f, w, x, y, z, data, s) \
        ( w += f(x, y, z) + data,  w = w<<s | w>>(32-s),  w += x )

///
/// byteReverse()
INLINE void byteReverse(unsigned char *buf, unsigned int longs)
{
  unsigned long t;

  do
  {
    t = (unsigned long)((unsigned)buf[3]<<8 | buf[2]) << 16 | ((unsigned)buf[1]<<8 | buf[0]);
    *(unsigned long *)buf = t;
    buf += 4;
  }
  while(--longs);
}
///
/// md5transform()
/*
 * The core of the MD5 algorithm, this alters an existing MD5 hash to
 * reflect the addition of 16 longwords of new data.  md5update blocks
 * the data and converts bytes into longwords for this routine.
 */
INLINE void md5transform(unsigned long buf[4], unsigned long const in[16])
{
  register unsigned long a, b, c, d;

  a = buf[0];
  b = buf[1];
  c = buf[2];
  d = buf[3];

  MD5STEP(F1, a, b, c, d, in[ 0]+0xd76aa478,  7);
  MD5STEP(F1, d, a, b, c, in[ 1]+0xe8c7b756, 12);
  MD5STEP(F1, c, d, a, b, in[ 2]+0x242070db, 17);
  MD5STEP(F1, b, c, d, a, in[ 3]+0xc1bdceee, 22);
  MD5STEP(F1, a, b, c, d, in[ 4]+0xf57c0faf,  7);
  MD5STEP(F1, d, a, b, c, in[ 5]+0x4787c62a, 12);
  MD5STEP(F1, c, d, a, b, in[ 6]+0xa8304613, 17);
  MD5STEP(F1, b, c, d, a, in[ 7]+0xfd469501, 22);
  MD5STEP(F1, a, b, c, d, in[ 8]+0x698098d8,  7);
  MD5STEP(F1, d, a, b, c, in[ 9]+0x8b44f7af, 12);
  MD5STEP(F1, c, d, a, b, in[10]+0xffff5bb1, 17);
  MD5STEP(F1, b, c, d, a, in[11]+0x895cd7be, 22);
  MD5STEP(F1, a, b, c, d, in[12]+0x6b901122,  7);
  MD5STEP(F1, d, a, b, c, in[13]+0xfd987193, 12);
  MD5STEP(F1, c, d, a, b, in[14]+0xa679438e, 17);
  MD5STEP(F1, b, c, d, a, in[15]+0x49b40821, 22);

  MD5STEP(F2, a, b, c, d, in[ 1]+0xf61e2562,  5);
  MD5STEP(F2, d, a, b, c, in[ 6]+0xc040b340,  9);
  MD5STEP(F2, c, d, a, b, in[11]+0x265e5a51, 14);
  MD5STEP(F2, b, c, d, a, in[ 0]+0xe9b6c7aa, 20);
  MD5STEP(F2, a, b, c, d, in[ 5]+0xd62f105d,  5);
  MD5STEP(F2, d, a, b, c, in[10]+0x02441453,  9);
  MD5STEP(F2, c, d, a, b, in[15]+0xd8a1e681, 14);
  MD5STEP(F2, b, c, d, a, in[ 4]+0xe7d3fbc8, 20);
  MD5STEP(F2, a, b, c, d, in[ 9]+0x21e1cde6,  5);
  MD5STEP(F2, d, a, b, c, in[14]+0xc33707d6,  9);
  MD5STEP(F2, c, d, a, b, in[ 3]+0xf4d50d87, 14);
  MD5STEP(F2, b, c, d, a, in[ 8]+0x455a14ed, 20);
  MD5STEP(F2, a, b, c, d, in[13]+0xa9e3e905,  5);
  MD5STEP(F2, d, a, b, c, in[ 2]+0xfcefa3f8,  9);
  MD5STEP(F2, c, d, a, b, in[ 7]+0x676f02d9, 14);
  MD5STEP(F2, b, c, d, a, in[12]+0x8d2a4c8a, 20);

  MD5STEP(F3, a, b, c, d, in[ 5]+0xfffa3942,  4);
  MD5STEP(F3, d, a, b, c, in[ 8]+0x8771f681, 11);
  MD5STEP(F3, c, d, a, b, in[11]+0x6d9d6122, 16);
  MD5STEP(F3, b, c, d, a, in[14]+0xfde5380c, 23);
  MD5STEP(F3, a, b, c, d, in[ 1]+0xa4beea44,  4);
  MD5STEP(F3, d, a, b, c, in[ 4]+0x4bdecfa9, 11);
  MD5STEP(F3, c, d, a, b, in[ 7]+0xf6bb4b60, 16);
  MD5STEP(F3, b, c, d, a, in[10]+0xbebfbc70, 23);
  MD5STEP(F3, a, b, c, d, in[13]+0x289b7ec6,  4);
  MD5STEP(F3, d, a, b, c, in[ 0]+0xeaa127fa, 11);
  MD5STEP(F3, c, d, a, b, in[ 3]+0xd4ef3085, 16);
  MD5STEP(F3, b, c, d, a, in[ 6]+0x04881d05, 23);
  MD5STEP(F3, a, b, c, d, in[ 9]+0xd9d4d039,  4);
  MD5STEP(F3, d, a, b, c, in[12]+0xe6db99e5, 11);
  MD5STEP(F3, c, d, a, b, in[15]+0x1fa27cf8, 16);
  MD5STEP(F3, b, c, d, a, in[ 2]+0xc4ac5665, 23);

  MD5STEP(F4, a, b, c, d, in[ 0]+0xf4292244,  6);
  MD5STEP(F4, d, a, b, c, in[ 7]+0x432aff97, 10);
  MD5STEP(F4, c, d, a, b, in[14]+0xab9423a7, 15);
  MD5STEP(F4, b, c, d, a, in[ 5]+0xfc93a039, 21);
  MD5STEP(F4, a, b, c, d, in[12]+0x655b59c3,  6);
  MD5STEP(F4, d, a, b, c, in[ 3]+0x8f0ccc92, 10);
  MD5STEP(F4, c, d, a, b, in[10]+0xffeff47d, 15);
  MD5STEP(F4, b, c, d, a, in[ 1]+0x85845dd1, 21);
  MD5STEP(F4, a, b, c, d, in[ 8]+0x6fa87e4f,  6);
  MD5STEP(F4, d, a, b, c, in[15]+0xfe2ce6e0, 10);
  MD5STEP(F4, c, d, a, b, in[ 6]+0xa3014314, 15);
  MD5STEP(F4, b, c, d, a, in[13]+0x4e0811a1, 21);
  MD5STEP(F4, a, b, c, d, in[ 4]+0xf7537e82,  6);
  MD5STEP(F4, d, a, b, c, in[11]+0xbd3af235, 10);
  MD5STEP(F4, c, d, a, b, in[ 2]+0x2ad7d2bb, 15);
  MD5STEP(F4, b, c, d, a, in[ 9]+0xeb86d391, 21);

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}
///
/// md5init()
/*
 * Start MD5 accumulation.  Set bit count to 0 and buffer to mysterious
 * initialization constants.
 */
void md5init(struct MD5Context *ctx)
{
  ctx->state[0] = 0x67452301;
  ctx->state[1] = 0xefcdab89;
  ctx->state[2] = 0x98badcfe;
  ctx->state[3] = 0x10325476;

  ctx->count[0] = 0;
  ctx->count[1] = 0;
}
///
/// md5update()
/*
 * Update context to reflect the concatenation of another buffer full
 * of bytes.
 */
void md5update(struct MD5Context *ctx, unsigned char const *buf, unsigned int len)
{
  unsigned int t;

  /* Update bitcount */
  t = ctx->count[0];
  if((ctx->count[0] = t + ((unsigned long)len << 3)) < t)
     ctx->count[1]++; /* Carry from low to high */
  ctx->count[1] += len >> 29;

  t = (t >> 3) & 0x3f;    /* Bytes already in shsInfo->data */

  /* Handle any leading odd-sized chunks */
  if(t)
  {
    unsigned char *p = (unsigned char *)ctx->buffer + t;

    t = 64-t;
    if(len < t)
    {
       memcpy(p, buf, len);
       return;
    }
    memcpy(p, buf, t);
    byteReverse(ctx->buffer, 16);
    md5transform(ctx->state, (unsigned long *)ctx->buffer);
    buf += t;
    len -= t;
  }

  /* Process data in 64-byte chunks */
  while(len >= 64)
  {
    memcpy(ctx->buffer, buf, 64);
    byteReverse(ctx->buffer, 16);
    md5transform(ctx->state, (unsigned long *)ctx->buffer);
    buf += 64;
    len -= 64;
  }

  /* Handle any remaining bytes of data. */
  memcpy(ctx->buffer, buf, len);
}
///
/// md5final()
/*
 * Final wrapup - pad to 64-byte boundary with the bit pattern
 * 1 0* (64-bit count of bits processed, MSB-first)
 */
void md5final(unsigned char digest[16], struct MD5Context *ctx)
{
  unsigned count;
  unsigned char *p;

  /* Compute number of bytes mod 64 */
  count = (ctx->count[0] >> 3) & 0x3F;

  /* Set the first char of padding to 0x80.  This is safe since there is
     always at least one byte free */
  p = ctx->buffer + count;
  *p++ = 0x80;

  /* Bytes of padding needed to make 64 bytes */
  count = 64 - 1 - count;

  /* Pad out to 56 mod 64 */
  if(count < 8)
  {
     /* Two lots of padding:  Pad the first block to 64 bytes */
     memset(p, 0, count);
     byteReverse(ctx->buffer, 16);
     md5transform(ctx->state, (unsigned long *)ctx->buffer);

     /* Now fill the next block with 56 bytes */
     memset(ctx->buffer, 0, 56);
  }
  else
  {
     /* Pad block to 56 bytes */
     memset(p, 0, count-8);
  }
  byteReverse(ctx->buffer, 14);

  /* Append length in bits and transform */
  ((unsigned long *)ctx->buffer)[ 14 ] = ctx->count[0];
  ((unsigned long *)ctx->buffer)[ 15 ] = ctx->count[1];

  md5transform(ctx->state, (unsigned long *)ctx->buffer);
  byteReverse((unsigned char *)ctx->state, 4);
  memcpy(digest, ctx->state, 16);
  memset(ctx, 0, sizeof(ctx));    /* In case it's sensitive */
}
///
/// md5hmac()
// text     pointer to data stream
// text_len length of data stream
// key      pointer to authentication key
// key_len  length of authentication key
// digest   caller digest to be filled in
void md5hmac(unsigned char * text, int text_len, unsigned char *key, int key_len, unsigned char digest[16])
{
  struct MD5Context context;
  unsigned char k_ipad[65];    /* inner padding - key XORd with ipad */
  unsigned char k_opad[65];    /* outer padding - key XORd with opad */
  unsigned char tk[16];
  int i;

  /* if key is longer than 64 bytes reset it to key=MD5(key) */
  if(key_len > 64)
  {
    struct MD5Context tctx;

    md5init(&tctx);
    md5update(&tctx, key, key_len);
    md5final(tk, &tctx);

    key = tk;
    key_len = 16;
  }

  /*
   * the HMAC_MD5 transform looks like:
   *
   * MD5(K XOR opad, MD5(K XOR ipad, text))
   *
   * where K is an n byte key
   * ipad is the byte 0x36 repeated 64 times
   * opad is the byte 0x5c repeated 64 times
   * and text is the data being protected
   */

  /* start out by storing key in pads */
  memset(k_ipad, 0, sizeof(k_ipad));
  memset(k_opad, 0, sizeof(k_opad));
  memcpy(k_ipad, key, key_len);
  memcpy(k_opad, key, key_len);

  /* XOR key with ipad and opad values */
  for (i=0; i<64; i++)
  {
     k_ipad[i] ^= 0x36;
     k_opad[i] ^= 0x5c;
  }
  /* perform inner MD5 */
  md5init(&context);                   /* init context for 1st pass */
  md5update(&context, k_ipad, 64);     /* start with inner pad */
  md5update(&context, text, text_len); /* then text of datagram */
  md5final(digest, &context);          /* finish up 1st pass */

  /* perform outer MD5 */
  md5init(&context);                   /* init context for 2nd pass */
  md5update(&context, k_opad, 64);     /* start with outer pad */
  md5update(&context, digest, 16);     /* then results of 1st hash */
  md5final(digest, &context);          /* finish up 2nd pass */
}
///


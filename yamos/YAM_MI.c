/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2004 by YAM Open Source Team

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

#include "YAM_debug.h"
#include "YAM_error.h"
#include "YAM_locale.h"
#include "YAM_mime.h"
#include "YAM_utilities.h"

/* local */
static void uueget(char*, FILE*, int);
static BOOL gettxtline(char*, int, char**);
static BOOL getline(char*, int, FILE*);

static int rfc2047_decode_int(const char *text,
                              int (*func)(const char *, unsigned int, const char *, const char *, void *),
                              void *arg);
static int rfc2047_dec_callback(const char *txt, unsigned int len, const char *chset,
                                const char *lang, void *arg);
INLINE char *rfc2047_search_quote(const char **ptr);


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

#define UUENC_IWIDTH 45                     // number of unencoded input chars
#define UUENC_OWIDTH (UUENC_IWIDTH*8/6)     // number of final output chars after encoding
#define UUENC_IBUF   (UUENC_IWIDTH*100)     // bytes to use as a uucode input encoding buffer
#define UUENC_OBUF   ((UUENC_OWIDTH+3)*80)  // bytes to use as a uucode output encoding buffer
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
// decoded string or 0 on an occurred error or a minus integer as
// an indicator of a short count in the encoded string. The source
// string doesn`t have to be NUL-terminated and only 'len' characters
// are going to be decoded. The decoding also stops as soon as the
// ending padding '==' or '=' characters are found.
int base64decode(char *to, const unsigned char *from, unsigned int len)
{
  unsigned char *fromp = (unsigned char *)from;
  unsigned char x, y;
  char *top = to;

  while(len >= 4)
  {
    len--;
    x = *fromp++;
    if(x > 127 || (x = index_64[x]) == 255)
    {
      return 0;
    }

    if((y = *fromp++) == '\0' ||
       y > 127 || (y = index_64[y]) == 255)
    {
      return 0;
    }

    len--;
    *top++ = (x << 2) | (y >> 4);

    if(len > 0)
    {
      len--;
      if((x = *fromp++) == '=')
      {
        if((len > 0 && *fromp++ != '='))
        {
          return 0;
        }

        len--;

        // we received the padding string
        // lets break out here
        break;
      }
      else
      {
        if(x > 127 || (x = index_64[x]) == 255)
        {
          return 0;
        }

        *top++ = (y << 4) | (x >> 2);
        if(len > 0)
        {
          len--;
          if ((y = *fromp++) == '=')
          {
            // we received the padding string
            // lets break out here
            break;
          }
          else
          {
            if (y > 127 || (y = index_64[y]) == 255)
            {
              return 0;
            }

            *top++ = (x << 6) | y;
          }
        }
      }
    }
  }

  *top = 0;
  if(len > 0)
  {
    // return -len to signal a short count
    return -(int)len;
  }

  return top - to;
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

  DB(kprintf("base64encode_file(): %ld\n", convLF);)

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
        DB(kprintf("EOF file at %ld\n", ftell(in));)

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        DB(kprintf("error on reading data!\n");)

        // an error occurred, lets return -1
        return -1;
      }
    }

    // now we check wheter the user want to convert each LF into a CRLF
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
    encoded = base64encode(outbuffer, inbuffer, read);
    sumencoded += encoded;

    // if the base64encoding routine returns <= 0 then there is obviously
    // something wrong
    if(encoded <= 0)
    {
      DB(kprintf("error on encoding data!\n");)
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
        else todo = towrite;
      }
      else todo = missing_chars;

      // now we do a binary write of the data
      if(fwrite(optr, 1, todo, out) != todo)
      {
        DB(kprintf("error on writing data!\n");)
        // an error must have occurred.
        return -1;
      }

      // lets modify our counters
      towrite -= todo;
      optr += todo;

      // then we have to check wheter we have written
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
      else if(fputc('\n', out) == EOF)
      {
        DB(kprintf("error on writing newline\n");)
        return -1;
      }
      else missing_chars = 0;
    }
  }

  DB(kprintf("finished base64encode_file(): %ld\n", sumencoded);)
  return sumencoded;
}

///
/// base64decode_file()
//  Decodes a file in base64 format. Takes care of an eventually specified translation
//  table as well as a CRLF->LF translation for printable text. It reads in the base64
//  strings line by line from the in file stream, decodes it and writes out the
//  decoded data with fwrite() to the out stream. It returns the total bytes of
//  written (decoded) data.
long base64decode_file(FILE *in, FILE *out,
                       struct TranslationTable *tt, BOOL convCRLF)
{
  char inbuffer[B64DEC_BUF+1];
  char outbuffer[B64DEC_BUF/4*3];
  char ungetbuf[3];
  long decodedChars = 0;
  size_t next_unget = 0;
  BOOL eof_reached = FALSE;

  DB(kprintf("base64decode_file(): %ld %ld\n", convCRLF, tt);)

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
        DB(kprintf("EOF file at %ld\n", ftell(in));)

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0 && next_unget == 0)
          break;
      }
      else
      {
        DB(kprintf("error on reading data!\n");)

        // an error occurred, lets return -1
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
        // on an EOF we shouldn`t have any unget chars
        return -1;
      }
    }

    // now that we have a whitespace free somewhat base64 encoded
    // string, we can call the base64decode() function to finally
    // decode the string
    if(read <= 0 ||
       (outLength = base64decode(outbuffer, inbuffer, read)) <= 0)
    {
      DB(kprintf("error on decoding: %ld %ld\n", read, outLength);)

      // it should not happen that we face a shortCount
      // or error
      return -1;
    }
    #if defined(DEBUG)
    else if(outLength > B64DEC_BUF/4*3)
    {
      kprintf("Error: outbuffer has been overwritten by base64decode()!!!\n");
      return -1;
    }
    #endif

    // if we got a translation table or need to convert a CRLF we have to parse through
    // the decoded string again and convert some chars.
    if(tt || convCRLF)
    {
      long r;
      char *rc = outbuffer;
      char *wc = outbuffer;

      for(r=0; r < outLength; r++, rc++)
      {
        // check if this is a CRLF
        if(convCRLF && *rc == '\r' &&
           outLength-r > 1 && rc[1] == '\n')
        {
          // if so, skip the \r
          continue;
        }
        else if(tt)
        {
          // if not, convert the char
          *wc = tt->Table[(UBYTE)*rc];

          // increase the write counter
          wc++;
        }
        else
        {
          // if no translation table is given lets copy
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
    if(fwrite(outbuffer, sizeof(char), (size_t)outLength, out) != (size_t)outLength)
    {
      DB(kprintf("error on writing data!\n");)

      // an error occurred while writing...
      return -1;
    }

    // increase the decodedChars counter
    decodedChars += outLength;
  }

  DB(kprintf("finished base64decode_file(): %ld\n", decodedChars);)
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

  DB(kprintf("qpencode_file()\n");)

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
        DB(kprintf("EOF file at %ld\n", ftell(in));)

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        DB(kprintf("error on reading data!\n");)

        // an error occurred, lets return -1
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
              (last = -1 && c == 'F' && strncmp(iptr, "rom ", 4) == 0))
      {

        // before we can encode the data we have to check
        // wheter there is enough space left on the line
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
        // have to check wheter this line is going to be longer
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
          DB(kprintf("error on writing data!\n");)
          // an error must have occurred.
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
      DB(kprintf("error on writing data!\n");)
      // an error must have occurred.
      return -1;
    }
  }

  DB(kprintf("finished qpencode_file(): %ld\n", encoded_chars);)
  return encoded_chars;
}

///
/// qpdecode_file()
// Decodes a whole file using the quoted-printable format defined in
// RFC 2045 (page 19)
long qpdecode_file(FILE *in, FILE *out, struct TranslationTable *tt)
{
  unsigned char inbuffer[QPDEC_BUF+1]; // lets use a 4096 byte large input buffer
  unsigned char outbuffer[QPDEC_BUF+1];// to speed things up we use the same amount
                                       // of memory for an output buffer as the
                                       // decoded string can't be larger than
                                       // the encoded one.

  unsigned char *iptr;
  unsigned char *optr = outbuffer;
  unsigned char c;
  size_t read;
  size_t next_unget = 0;
  long decoded = 0;
  int result = 0;
  BOOL eof_reached = FALSE;

  DB(kprintf("qpdecode_file()\n");)

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
        DB(kprintf("EOF file at %ld\n", ftell(in));)

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0 && next_unget == 0)
          break;
      }
      else
      {
        DB(kprintf("error on reading data!\n");)

        // an error occurred, lets return -1
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
            *optr++ = tt ? tt->Table[(UBYTE)(c1<<4 | c2)] : c1<<4 | c2;

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
        *optr++ = tt ? tt->Table[(UBYTE)c] : c;
      }

      // let us now check if our outbuffer is filled up so that we can write
      // out the data to our out stream.
      if(optr-outbuffer >= QPENC_BUF)
      {
        size_t todo = optr-outbuffer;

        // now we do a binary write of the data
        if(fwrite(outbuffer, 1, todo, out) != todo)
        {
          DB(kprintf("error on writing data!\n");)
          // an error must have occurred.
          return -1;
        }

        // now reset the outbuffer and stuff
        optr = outbuffer;
      }
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
      DB(kprintf("error on writing data!\n");)
      // an error must have occurred.
      return -1;
    }
  }

  // if we end up here and read > 0 then the decoding wasn't finished
  // and we have to return an error
  if(read > 0)
    return -2; // -2 means "unfinished decoding"

  // on success lets return the number of decoded
  // chars
  DB(kprintf("finished qpdecode_file(): %ld\n", decoded);)
  return result == 0 ? decoded : result;
}

///

/*** URL decoding routines ***/
/// fromform
//  Converts an url-encoded file into plain text
void fromform(FILE *infile, FILE *outfile, struct TranslationTable *tt)
{
   unsigned int c;
   while ((c = fgetc(infile)) != (unsigned int)-1)
   {
      switch (c)
      {
         case '&': fputc('\n', outfile); break;
         case '%': c = (index_hex[fgetc(infile)]<<4)+index_hex[fgetc(infile)];
                   switch (c)
                   {
                      case '\n': fputs("\n ", outfile); break;
                      case '\r': break;
                      default  : fputc(tt ? (unsigned int)tt->Table[(UBYTE)c] : c, outfile); break;
                   }
                   break;
         case '+': fputc(' ', outfile); break;
         case '=': fputs(" = ", outfile); break;
         default : fputc(tt ? (unsigned int)tt->Table[(UBYTE)c] : c, outfile); break;
      }
   }
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

  DB(kprintf("uuencode_file()\n");)

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
        DB(kprintf("EOF file at %ld\n", ftell(in));)

        eof_reached = TRUE; // we found an EOF

        // if the last read was zero we can exit immediatly
        if(read == 0)
          break;
      }
      else
      {
        DB(kprintf("error on reading data!\n");)

        // an error occurred, lets return -1
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
        *optr++ = UUENCODE_CHAR((c1+c2)%UUMAX_CHAR);
        *optr++ = '\n';
      }

      // let us now check if our outbuffer is filled up so that we can write
      // out the data to our out stream.
      if(optr-outbuffer >= UUENC_OBUF ||
         eof_reached == TRUE)
      {
        size_t todo = optr-outbuffer;

        // now we do a binary write of the data
        if(fwrite(outbuffer, 1, todo, out) != todo)
        {
          DB(kprintf("error on writing data!\n");)
          // an error must have occurred.
          return -1;
        }

        // now reset the outbuffer and stuff
        optr = outbuffer;
      }
    }
  }

  DB(kprintf("finished uuencode_file(): %ld\n", encoded_chars);)
  return encoded_chars;
}

///
/// uueget
//  Decodes four UU encoded bytes
static void uueget(char *ptr, FILE *outfp, int n)
{
   int c1, c2, c3;
   unsigned char p0, p1, p2, p3;

   p0 = (ptr[0] - ' ') & 0x3F;
   p1 = (ptr[1] - ' ') & 0x3F;
   p2 = (ptr[2] - ' ') & 0x3F;
   p3 = (ptr[3] - ' ') & 0x3F;
   c1 = p0 << 2 | p1 >> 4;
   c2 = p1 << 4 | p2 >> 2;
   c3 = p2 << 6 | p3;
   if (n >= 1) fputc(c1, outfp);
   if (n >= 2) fputc(c2, outfp);
   if (n >= 3) fputc(c3, outfp);
}

///
/// gettxtline
//  Reads next line of UU encoded string
static BOOL gettxtline(char *buf, int size, char **rptr)
{
   int c;
   char *ptr = buf;

   for (c = 0; c < size; ++c)buf[c] = ' ';
   do
   {
      c = (int)**rptr; (*rptr)++;
      if (!c) { *ptr = '\0'; return (BOOL)(ptr == buf); }
      else if (c == '\n' || c == '\r') { *ptr = '\0'; return FALSE; }
      // Emm: I guess the following line was meant to process quoted
      // mails, but it causes file corruption when the '>' is really
      // part of the uuencoding (usually, this happens for the last
      // line).
      //else if (ptr == buf && c == '>') continue;
      else if (size > 0) { *ptr++ = c; size--; }
   } while (TRUE);
   return FALSE;
}

///
/// fromuuetxt
//  Decodes a string in UUE format
void fromuuetxt(char **txt, FILE *outfp)
{
   char buf[SIZE_LINE];

   while (TRUE)
   {
      if (gettxtline(buf, sizeof(buf), txt))
      {
         ER_NewError(GetStr(MSG_ER_UnexpEOFUU), NULL, NULL);
         return;
      }
      if (!strncmp(buf, "begin", 5)) break;
   }
   while (TRUE)
   {
      if (gettxtline(buf, sizeof(buf), txt))
      {
         ER_NewError(GetStr(MSG_ER_UnexpEOFUU), NULL, NULL);
         return;
      }
      else if (!strncmp(buf, "end", 5)) break;
      else if (*buf == '\0') continue;
      else
      {
         int length = (*buf - ' ');
         if (*buf == '`') length = 0;
         if (length < 0 || length > 63)
            ER_NewError(GetStr(MSG_ER_InvalidLength), (char *)length, NULL);
         else
         {
            char *ptr = buf + 1;
            while (length > 0) { uueget(ptr, outfp, length); length -= 3; ptr += 4; }
         }
      }
   }
}

///
/// getline
//  Reads next line from a UU encoded file
static BOOL getline(char *buf, int size, FILE *fp)
{
   int c;
   char *ptr = buf;

   for (c = 0; c < size; ++c)buf[c] = ' ';
   do
   {
      if ((c = fgetc(fp)) == -1) {*ptr = '\0'; return (BOOL)(ptr == buf); }
      else if (c == '\n' || c == '\r') { *ptr = '\0'; return FALSE; }
      // Emm: I guess the following line was meant to process quoted
      // mails, but it causes file corruption when the '>' is really
      // part of the uuencoding (usually, this happens for the last
      // line).
      //else if (ptr == buf && c == '>') continue;
      else if (size > 0) { *ptr++ = c; size--; }
   } while (TRUE);
   return FALSE;
}

///
/// fromuue
//  Decodes a file in UUE format
void fromuue(FILE *infp, FILE *outfp)
{
   char buf[SIZE_LINE];

   while (TRUE)
   {
      if (getline(buf, sizeof(buf), infp))
      {
         ER_NewError(GetStr(MSG_ER_UnexpEOFUU), NULL, NULL);
         return;
      }
      if (!strncmp(buf, "begin", 5)) break;
   }
   while (TRUE)
   {
      if (getline(buf, sizeof(buf), infp))
      {
         ER_NewError(GetStr(MSG_ER_UnexpEOFUU), NULL, NULL);
         return;
      }
      else if (!strncmp(buf, "end", 5)) break;
      else if (*buf == '\0') continue;
      else
      {
         int length = (*buf - ' ');
         if (*buf == '`') length = 0;
         if (length < 0 || length > 63)
            ER_NewError(GetStr(MSG_ER_InvalidLength), (char *)length, NULL);
         else
         {
            char *ptr = buf + 1;
            while (length > 0) { uueget(ptr, outfp, length); length -= 3; ptr += 4; }
         }
      }
   }
}

///

/*** RFC 2047 MIME encoding/decoding routines ***/
/// rfc2047_encode_file()
// Special encoding routines based on RFC2047 which directly
// encodes header and body text into the supllied file stream. Please note
// that this function tries to be as RFC compliant as possible, which means
// that it does not simply generate a large huge encoded-word with text
// in it. It tries to only encode those passages which really require
// this encoding because of special characters in it.
int rfc2047_encode_file(FILE *fh, const char *str,
                        const struct TranslationTable *tt)
{
  char *c = (char *)str;
  char *c_wstart = NULL;      // pointer to the start of the actual word
  char encode_buf[SIZE_LINE]; // some general encoding buffer.
  char *ebp = encode_buf;
  char *eb_wstart = NULL;
  char *eb_wstart_prev = NULL;
  char *eb_wend = NULL;
  BOOL encode_mode = FALSE;

  // in the following we parse the string charwise and separate each
  // single word, analyze it to be RFC 2047 compliant and if any non US-ASCII
  // chars are found we convert them to quoted printables and concatenate them
  // in one long string, taking care of maximum line lengths as defined
  // in the RFC.
  do
  {
    // to start the search we keep a pointer to the start
    // of the actual word.
    if(c_wstart == NULL)
    {
			// and if the current char is not a linear white space we found
			// the start
      if(!is_lwsp(*c))
      {
        c_wstart = c;     // save the current c as the word start in the source string
        eb_wstart = ebp;  // save the current ebp as the word start in the encode_buffer
      }
      else if(encode_mode == FALSE)
      {
        // ok, we found a whitespace and we are not in encoding mode
        // so we can directly copy this whitespace to our
        // encode buffer.
        *ebp = *c;
        ebp++;

        if(*c == '\0')
          break;

        c++;
        continue;
      }
    }

    // now we have to check wheter we are in the encode mode or not
    // This means, that if we previously found characters which require
    // endcoding, the followed character also should be encoded if
    // it is no whitespace and not quoted printable safe.
    if(encode_mode == TRUE)
    {
      if(is_lwsp(*c) || *c == '\0')
      {
        // a whitespace/nullbyte stops the encoding until we have
        // verified that the next word also contains non US-ASCII
        // chars so that we can concatenate those strings later on
        // again to one large on.
        eb_wend = ebp;
        *ebp = '?';
        ebp++;
        *ebp = '=';
        ebp++;
        *ebp = *c;
        ebp++;

        c_wstart = NULL;
        encode_mode = FALSE;

        // then we check wheter the current line is
        // larger than 75 chars as this is the limit for a line
        // containing RFC 2047 encoded strings
        while((ebp-encode_buf) > ENCWORD_LEN)
        {
          // check if there are any words before the current one
          if(eb_wstart != encode_buf)
          {
            // ok, then write until eb_wstart-1 and move the rest
            fwrite(encode_buf, eb_wstart-encode_buf-1, 1, fh);

            // also put out a CRLF SPACE to signal a new line
            // starts
            fwrite("\r\n ", 3*sizeof(char), 1, fh);

            // then move the other stuff to the start
            memmove(encode_buf, eb_wstart, ebp-eb_wstart);
            eb_wend = encode_buf+(eb_wend-eb_wstart);
            eb_wstart = encode_buf;
            ebp = eb_wend;
          }
          else
          {
            // so it seems to now got a huge encoded-word that we
            // require to split up into several small (<75 chars) ones
            // and we do this in a loop, of course.
            while((ebp-encode_buf) > ENCWORD_LEN)
            {
              char *split_pos;

              // ok, now it gets a bit more tricky, as we
              // have to split the encoded word, which seems to
              // be larger than 75 chars into smaller pieces
              split_pos = encode_buf+ENCWORD_LEN-2; // we need 2 for the ending ?=
              if(*split_pos != '=')
              {
                if(*(split_pos-1) == '=')
                {
                  split_pos--;
                }
                else if(*(split_pos-2) == '=')
                {
                  split_pos -= 2;
                }
              }

              // now we should have the position where we can safely split
              // the encoded word, so lets do it
              fwrite(encode_buf, split_pos-encode_buf, 1, fh);

              // now add "?=\r\n " so that the next line is prepared
              fwrite("?=\r\n ", 5*sizeof(char), 1, fh);

              if(ebp-split_pos > 3)
              {
                int move_start;

                // as we splitted an encoded-word we have to start the next
                // line with a proper encoded-word start again.
                if(tt && tt->SourceCharset)
                  move_start = sprintf(encode_buf, "=?%s?Q?", tt->SourceCharset);
                else
                  move_start = sprintf(encode_buf, "=?ISO-8859-1?Q?");

                // then move the other stuff to the start again
                memmove(encode_buf+move_start, split_pos, ebp-split_pos);

                eb_wstart = encode_buf+move_start;
                eb_wend = eb_wstart+(eb_wend-split_pos);
                ebp = eb_wstart+(ebp-split_pos);
              }
              else
              {
                eb_wstart = encode_buf;
                eb_wend = encode_buf;
                ebp = encode_buf;
              }
            }
          }
        }

        eb_wstart_prev = eb_wstart;
      }
      else
      {
        // so this is something other than a whitespace and if
        // it is a non US-ASCII char or not safe in a quoted
        // printable string we encode it accordingly to the
        // quoted-printable rules.
        if(!is_qpsafe(*c) || !is_esafe(*c))
        {
          *ebp = '=';
          ebp++;
          *ebp = basis_hex[(*c >> 4) & 0xF];
          ebp++;
          *ebp = basis_hex[*c & 0xF];
          ebp++;
        }
        else
        {
          // so this char seems to be safe to directly be put
          // into our final encode buffer
          *ebp = *c;
          ebp++;
        }
      }
    }
    else
    {
      // then we check wheter the current char is a non US-ASCII (7bit)
      // char which would require us to encode the full word as a RFC 2047
      // compliant `encoded-word`. 
      if(*c != '\0' &&
         (!isascii(*c) || iscntrl(*c) ||
          (*c == '=' && *(c+1) == '?' && isascii(*(c+2)) &&
           (c == str || is_lwsp(*(c-1))))))
      {
        // ok, this is a non US-ASCII char and should be encoded
        // accordingly. so lets check wheter the previous word was
        // also encoded or not and if so we concatenate them
        if(eb_wend)
        {
          ebp = eb_wend;
          eb_wstart = eb_wstart_prev;

          // decrease c_wstart by one because we want to encode
          // the separating char aswell
          c_wstart--;
        }
        else
        {
          // no it seems this is a plain start so lets add the full
          // encoding aswell.
          ebp = eb_wstart;

          // before we place encoded data in our encode buffer we have to
          // place the "=?charset?Q?" string at the beginning because here
          // the encoding starts
          if(tt && tt->SourceCharset)
            ebp += sprintf(ebp, "=?%s?Q?", tt->SourceCharset);
          else
            ebp += sprintf(ebp, "=?ISO-8859-1?Q?");
        }

        // so this is where the actual encoding is performed now.
        // we start at the last recognized word start and go on until
        // the current character, then the upper loop does the rest of the job
        // for us.
        while(c_wstart <= c)
        {
          if(!is_qpsafe(*c_wstart) || !is_esafe(*c_wstart))
          {
            *ebp = '=';
            ebp++;
            *ebp = basis_hex[(*c_wstart >> 4) & 0xF];
            ebp++;
            *ebp = basis_hex[*c_wstart & 0xF];
          }
          else if(*c_wstart == ' ')
          {
            *ebp = '_'; // RFC 2047 allows to replace a SPACE by a '_'
          }
          else
          {
            *ebp = *c_wstart;
          }

          ebp++;
          c_wstart++;
        }

        // now flag that we are in global encoding mode now
        // so that the upper loops encodes everything properly.
        encode_mode = TRUE;
      }
      else
      {
        // if we end up here the it is fine to directly place
        // the current char in the encoding buffer.
        *ebp = *c;
        ebp++;

        // if this had been a whitespace we found it without encoding
        // enabled, so we probably found a "clean" word. So we can also
        // clean the last word markers
        if(is_lwsp(*c) || *c == '\0')
        {
          c_wstart = NULL;
          eb_wend = NULL;

          // only if there is an encoded-word on the current line
          // we check wheter the line is too long and split
          // it accordingly.
          if((ebp-encode_buf) > ENCWORD_LEN)
          {
            // check if there are any words before the current one
            // and if not we got a huge plain unencoded word on the line
            // so we don't have to worry about any 75 char limit like
            // defined in rfc 2047
            if(eb_wstart != encode_buf)
            {
              // ok, then write until eb_wstart-1 and move the rest
              fwrite(encode_buf, eb_wstart-encode_buf-1, 1, fh);

              // also put out a CRLF SPACE to signal a new line
              // starts
              fwrite("\r\n ", 3*sizeof(char), 1, fh);

              // then move the other stuff to the start
              memmove(encode_buf, eb_wstart, ebp-eb_wstart);
              eb_wend = encode_buf+(ebp-eb_wstart);
              eb_wstart = encode_buf;
              ebp = eb_wend;
            }
          }
        }
      }
    }

    if(*c == '\0')
      break;

    c++;
  }
  while(1);

  // write it out to the file stream
  fwrite(encode_buf, strlen(encode_buf), 1, fh);

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
  struct TranslationTable *tt;
};

// this function uses the decode_int() function and attachs a callback
// function to it which will be called each time a token has been
// decoded so that an eventually existing translation table can be used
// to translate the charsets. returns the number of translated chars or
// -1 if an malloc error occurred and -2 if some unknown encoding was
// specified in the text, -3 if the base64 decoding failed.
int rfc2047_decode(char *dst, const char *src, unsigned int maxlen,
                   struct TranslationTable *tt)
{
  int result;
  // pack all the necessary information in the decode_info
  // structure so that the decode function can process it.
  struct rfc2047_decode_info info;
  info.dst    = dst;
  info.maxlen = maxlen;
  info.tt     = tt;

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
  BOOL translate = FALSE;

  // before we go on we check wheter we have enough space
  // to put the txt in our destination
  if(info->maxlen<len)
    return -1;

  // so we can savely decrease it.
  info->maxlen -= len;

  // check wheter we have a valid translation table or not
  if(info->tt && info->tt->SourceCharset)
  {
    if(chset)
    {
      if(MatchNoCase((char *)chset, info->tt->SourceCharset))
        translate = TRUE;
    }
    else if(strcmp(info->tt->SourceCharset, "#?") == 0)
    {
      translate = TRUE;
    }
  }

  // if we recognized a valid charset translation, lets start
  if(translate)
  {
    unsigned int i;

    // it seems we have a valid translation table, so lets
    // parse through our text and put the translated char in
    // our destination.
    for(i=0; i < len; i++)
    {
      info->dst[i] = info->tt->Table[(UBYTE)(txt[i])];
    }
  }
  else
  {
    // no translation table is available, so lets just
    // copy the decoded stuff in our destination pointer
    memcpy(info->dst, txt, len);
  }

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
  const char *start = text;

  while(text && *text)
  {
    p=text;

    if(*text != '=' || *(text+1) != '?')
    {
      while(*text)
      {
        if(*text == '=' && *(text+1) == '?' &&
           (text == start || is_lwsp(*(text-1)) || *(text-1) == '('))
        {
          break;
        }

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
          int res = base64decode(enctext, enctext, strlen(enctext));
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
      // RFC 2231 language
      lang = strrchr(chset, '*');
      if(lang) *lang++ = 0;

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

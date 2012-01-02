/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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
#include <stdio.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/codesets.h>

#include "YAM.h"
#include "YAM_config.h"

#include "mime/base64.h"

#include "Debug.h"

// Global variables

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

// some defines that can be usefull
#define B64_LINELEN 72    // number of chars before the b64encode_file() issues a CRLF
#define B64DEC_BUF  4096  // bytes to use as a base64 file decoding buffer
#define B64ENC_BUF  4095  // bytes to use as a base64 file encoding buffer (must be a multiple of 3)

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

      STRPTR str = CodesetsConvertStr(CSA_SourceCodeset,   srcCodeset,
                                      CSA_DestCodeset,     G->readCharset,
                                      CSA_Source,          outbuffer,
                                      CSA_SourceLen,       outLength,
                                      CSA_DestLenPtr,      &strLen,
                                      CSA_MapForeignChars, C->MapForeignChars,
                                      TAG_DONE);

      if(str != NULL && strLen > 0)
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


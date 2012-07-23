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

#include "mime/qprintable.h"

#include "Debug.h"

// Global variables

// some defines that can be usefull
#define QP_LINELEN  76    // number of chars before qpencode_file() issues a CRLF
#define QPENC_BUF   4096  // bytes to use as a quoted-printable file encoding buffer
#define QPDEC_BUF   4096  // bytes to use as a quoted-printable file decoding buffer

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

    while(read != 0)
    {
      c = *iptr++;
      read--;

      if(c == '=')
      {
        // check if the next char is a newline so that
        // we can skip the current =
        if(read != 0 && *iptr == '\n')
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
        W(DBF_MIME, "nonallowed character '%lc' (%02lx) found", c, c);
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
        if(C->DetectCyrillic == TRUE)
        {
          if(srcCodeset == NULL || (srcCodeset->name != NULL && stricmp(srcCodeset->name, "utf-8") != 0))
          {
            struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                                  CSA_SourceLen,      todo,
                                                  CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                                  TAG_DONE);

            if(cs != NULL && cs != srcCodeset)
              srcCodeset = cs;
          }
        }

        // if the caller supplied a source codeset, we have to
        // make sure we convert our outbuffer before writing it out
        // to the file into our local charset
        if(srcCodeset != NULL)
        {
          ULONG strLen = 0;

          STRPTR str = CodesetsConvertStr(CSA_SourceCodeset,   srcCodeset,
                                          CSA_DestCodeset,     G->readCharset,
                                          CSA_Source,          dptr,
                                          CSA_SourceLen,       todo,
                                          CSA_DestLenPtr,      &strLen,
                                          CSA_MapForeignChars, C->MapForeignChars,
                                          TAG_DONE);

          if(str != NULL && strLen > 0)
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
    if(C->DetectCyrillic == TRUE &&
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
    if(srcCodeset != NULL)
    {
      ULONG strLen = 0;

      STRPTR str = CodesetsConvertStr(CSA_SourceCodeset,   srcCodeset,
                                      CSA_DestCodeset,     G->readCharset,
                                      CSA_Source,          dptr,
                                      CSA_SourceLen,       todo,
                                      CSA_DestLenPtr,      &strLen,
                                      CSA_MapForeignChars, C->MapForeignChars,
                                      TAG_DONE);

      if(str != NULL && strLen > 0)
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

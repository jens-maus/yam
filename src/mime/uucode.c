/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2015 YAM Open Source Team

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/codesets.h>

#include "YAM.h"

#include "mime/qprintable.h"

#include "Config.h"

#include "Debug.h"

// some defines that can be usefull
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
long uudecode_file(FILE *in, FILE *out, struct codeset *srcCodeset, BOOL isText)
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

  D(DBF_MIME, "codeset '%s'", srcCodeset != NULL ? srcCodeset->name : "none");

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
          if(C->DetectCyrillic == TRUE && isText == TRUE)
          {
            if(srcCodeset == NULL || (srcCodeset->name != NULL && stricmp(srcCodeset->name, "utf-8") != 0))
            {
              struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                                    CSA_SourceLen,      todo,
                                                    CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                                    TAG_DONE);

              if(cs != NULL && cs != srcCodeset)
              {
                D(DBF_MIME, "using codeset '%s' instead of '%s'", srcCodeset != NULL ? srcCodeset->name : "none", cs->name);
                srcCodeset = cs;
              }
            }
          }

          // if the caller supplied a source codeset, we have to
          // make sure we convert our outbuffer before writing it out
          // to the file in UTF8
          if(isText == TRUE && srcCodeset != NULL && stricmp(srcCodeset->name, "utf-8") != 0)
          {
            ULONG strLen = 0;

            UTF8 *str = CodesetsUTF8Create(CSA_Source,          dptr,
                                           CSA_SourceLen,       todo,
                                           CSA_SourceCodeset,   srcCodeset,
                                           CSA_DestLenPtr,      &strLen,
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
              W(DBF_MIME, "error while trying to convert uudecoded string to UTF8");
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
    if(C->DetectCyrillic == TRUE && isText == TRUE)
    {
      if(srcCodeset == NULL || (srcCodeset->name != NULL && stricmp(srcCodeset->name, "utf-8") != 0))
      {
        struct codeset *cs = CodesetsFindBest(CSA_Source,         dptr,
                                              CSA_SourceLen,      todo,
                                              CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                              TAG_DONE);

        if(cs != NULL && cs != srcCodeset)
        {
          D(DBF_MIME, "using codeset '%s' instead of '%s'", srcCodeset != NULL ? srcCodeset->name : "none", cs->name);
          srcCodeset = cs;
        }
      }
    }

    // if the caller supplied a source codeset, we have to
    // make sure we convert our outbuffer before writing it out
    // to the file in UTF8, but we must not touch binary/non-text data
    if(isText == TRUE && srcCodeset != NULL && stricmp(srcCodeset->name, "utf-8") != 0)
    {
      ULONG strLen = 0;

      UTF8 *str = CodesetsUTF8Create(CSA_Source,          dptr,
                                     CSA_SourceLen,       todo,
                                     CSA_SourceCodeset,   srcCodeset,
                                     CSA_DestLenPtr,      &strLen,
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

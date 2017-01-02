/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2017 YAM Open Source Team

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
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/codesets.h>

#include "SDI_compiler.h"

#include "YAM.h"

#include "mime/base64.h"
#include "mime/qprintable.h"

#include "Config.h"

#include "Debug.h"

// RFC 2047 routines
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

#define ENCWORD_LEN    75                  // max. encoded-word length (rfc2047 2.0)

#define CHARS_LWSP     " \t\n\r"           // linear whitespace chars
#define CHARS_TSPECIAL "()<>@,;:\\\"/[]?="
#define CHARS_SPECIAL  "()<>@,;:\\\".[]"
#define CHARS_CSPECIAL "()\\\r"            // not in comments
#define CHARS_DSPECIAL "[]\\\r \t"         // not in domains
#define CHARS_ESPECIAL "()<>@,;:\"/[]?.=_" // encoded word specials (rfc2047 5.1)
#define CHARS_PSPECIAL "!*+-/=_"           // encoded phrase specials (rfc2047 5.3)

// local functions
static int rfc2047_decode_int(const char *text,
                              int (*func)(const char *, unsigned int, const char *, const char *, void *),
                              void *arg);
static int rfc2047_decode_callback(const char *txt, unsigned int len, const char *chset,
                                   const char *lang, void *arg);
INLINE char *rfc2047_search_quote(const char **ptr);

/*** RFC 2047 MIME encoding/decoding routines ***/
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

  if(IsStrEmpty(str) == FALSE)
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
  if((dst = rfc2047_encode_str(src, strippedCharsetName(G->writeCodeset), &rfc2047_qp_allow_any)))
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
  result = rfc2047_decode_int(src, &rfc2047_decode_callback, &info);
  info.dst[0] = '\0'; // make sure this string is null-terminated

  // on success return the decoded string len.
  if(result > 0)  return (int)(maxlen-info.maxlen);
  else            return result;
}

///
/// rfc2047_decode_callback()
// the callback function that is called by the decode_int() function each
// time a string was successfully decoded so that it can be converted.
static int rfc2047_decode_callback(const char *txt, unsigned int len, const char *chset,
                                   UNUSED const char *lang, void *arg)
{
  struct rfc2047_decode_info *info = (struct rfc2047_decode_info *)arg;

  // in case the user wants us to detect the correct cyrillic codeset
  // we do it now
  if(C->DetectCyrillic == TRUE)
  {
    if(chset == NULL || stricmp(chset, "utf-8") != 0)
    {
      struct codeset *cs = CodesetsFindBest(CSA_Source,         txt,
                                            CSA_SourceLen,      len,
                                            CSA_CodesetFamily,  CSV_CodesetFamily_Cyrillic,
                                            TAG_DONE);

      if(cs != NULL)
        chset = cs->name;
    }
  }

  // now we try to get the src codeset from codesets.library
  // and convert the string into our local charset if required
  if(IsStrEmpty(chset) == FALSE)
  {
    // check if the src codeset of the string isn't the same
    // like our local one.
    if(stricmp(chset, strippedCharsetName(G->localCodeset)) != 0)
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
        STRPTR str = CodesetsConvertStr(CSA_SourceCodeset,   srcCodeset,
                                        CSA_DestCodeset,     G->localCodeset,
                                        CSA_Source,          txt,
                                        CSA_SourceLen,       len,
                                        CSA_DestLenPtr,      &dstLen,
                                        CSA_MapForeignChars, C->MapForeignChars,
                                        TAG_DONE);

        // now that we have our converted string we can go and
        // copy it over our destination buffer
        if(str != NULL && dstLen > 0)
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
          W(DBF_MIME, "couldn't convert src str via CodesetsConvertStr(), codesets '%s' -> '%s'!", SafeStr(srcCodeset->name), SafeStr(G->localCodeset->name));
      }
      #if defined(DEBUG)
      else if(stricmp(chset, "us-ascii") != 0)
        W(DBF_MIME, "couldn't find charset '%s' in codesets.library!", chset);
      #endif
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

  l = *ptr - p + 1;
  if((s = malloc(l)) != NULL)
  {
    strlcpy(s, p, l);
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

  while(IsStrEmpty(text) == FALSE)
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
        if(rc != 0)
          return rc;
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
      switch(encoding[0])
      {
        // we found a quoted-printable encoded rfc2047 compliant string, so
        // lets decode it.
        case 'q':
        case 'Q':
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
            if(c == '_')
              c = ' ';
            *r++ = c ;
          }
          *r=0;
        }
        break;

        // we found a base64 encoded rfc2047 compliant string, so
        // lets decode it.
        case 'b':
        case 'B':
        {
          char *etext = Trim(enctext);
          char *dectext = NULL;
          int res;

          if((res = base64decode(&dectext, etext, strlen(etext))) > 0)
          {
            free(enctext);
            enctext = dectext;
          }
          else
          {
            W(DBF_MIME, "base64 decoding returned: %d '%s'", res, enctext);
            result = -3; // signal a base64 decoding error to caller
            unknown_enc = 1;
          }
        }
        break;

        default:
        {
          // if we end up here the probabilty that we have a rfc2047 compliant
          // string with an unknown encoding is somehow high.
          result = -2; // signal an unknown encoding to caller
          unknown_enc = 1;
        }
        break;
      }
    }
    else
      unknown_enc = 1;

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
      //   encoded-word := "=?" charset ["*" language] "?" encoding "?" encoded-text "?="
      if((lang = strrchr(chset, '*')))
        *lang++ = '\0';

      rc = (*func)(enctext, strlen(enctext), chset, lang, arg);
    }

    // free everything and check if a returncode was given.
    free(enctext);
    free(chset);
    free(encoding);
    if(rc != 0)
      return rc;

    // Ignore blanks between enc words
    had_last_word=1;
  }

  return result;
}

///

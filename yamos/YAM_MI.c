/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2001 by YAM Open Source Team

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

#include <string.h>

#include "YAM.h"
#include "YAM_error.h"
#include "YAM_locale.h"
#include "YAM_utilities.h"

/* local */
static int nextcharin(FILE*, BOOL);
static void output64chunk(int, int, int, int, FILE*);
static void almostputc(int, FILE*, struct TranslationTable*, BOOL);
static void uueget(char*, FILE*, int);
static BOOL gettxtline(char*, int, char**);
static BOOL getline(char*, int, FILE*);
static int outdec(char*, FILE*);


/***************************************************************************
 MIME I/O routines
***************************************************************************/

/// Global variables
static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static char basis_hex[] = "0123456789ABCDEF";

static char index_64[128] = {
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
   52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1,-1,-1,-1,
   -1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
   15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
   -1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
   41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};
static char index_hex[128] = {
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
   -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
   -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

#define hexchar(c)  (((c) > 127) ? -1 : index_hex[(c)])
#define char64(c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])
#define SUMSIZE 64
#define ENC(c) ((c) ? ((c) & 077) + ' ': '`')

static BOOL InNewline = FALSE;
static BOOL CRpending = FALSE;
BOOL *NeedsPortableNewlines;
///

/// nextcharin
//  Reads next byte from a text files, handles CRLF line breaks
static int nextcharin(FILE *infile, BOOL PortableNewlines)
{
   int c;

   if (!PortableNewlines) return fgetc(infile);
   if (InNewline) { InNewline = FALSE; return 10; }    /***BUG***/
   c = fgetc(infile);
   if (c == '\n') { InNewline = TRUE; return 13; }
   return c;
}

///
/// encode64
//  Encodes string in base64 format
void encode64(char *s, char *d, int len)
{
   int i;

   for(i=0;i<len;i+=3)
   {
      int c1, c2, c3, c4, count=len-i;

      c1 = *s >> 2;
      c2 = ((*s << 4) & 060) | ((s[1] >> 4) & 017);
      c3 = ((s[1] << 2) & 074) | ((s[2] >> 6) & 03);
      c4 = s[2] & 077;
      *d++=basis_64[c1];
      *d++=basis_64[c2];
      if (count == 1) {
        *d++='=';
        *d++='=';
      }
      else {
        *d++=basis_64[c3];
        if (count == 2)
          *d++='=';
        else
          *d++=basis_64[c4];
      }
      s+=3;
   }
   *d=0;
}

///
/// decode64
//  Decodes string in base64 format
#define BASE64(c) (index_64[(unsigned char)(c) & 0x7F])
char *decode64 (char *dest, char *src, char *srcmax)
{
   while (src + 3 < srcmax)
     {
        *dest++ = (BASE64(src[0]) << 2) | (BASE64(src[1]) >> 4);
        
        if (src[2] == '=') break;
        *dest++ = ((BASE64(src[1]) & 0xf) << 4) | (BASE64(src[2]) >> 2);
        
        if (src[3] == '=') break;
        *dest++ = ((BASE64(src[2]) & 0x3) << 6) | BASE64(src[3]);
        src += 4;
     }
   *dest=0;
   return dest;
}
///
/// output64chunk
//  Writes three bytes in base64 format
static void output64chunk(int c1, int c2, int c3, int pads, FILE *outfile)
{
   fputc(basis_64[c1>>2], outfile);
   fputc(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)], outfile);
   switch(pads)
   {
      case 0 :
         fputc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
         fputc(basis_64[c3 & 0x3F], outfile);
         break;
      case 2 :
         fputs("==", outfile);
         break;
      default :
         fputc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
         fputc('=', outfile);
   }
/*
   if (pads == 2) 
   {
      fputs("==", outfile);
   } 
   else if (pads) 
   {
      fputc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
      fputc('=', outfile);
   }
   else 
   {
      fputc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
      fputc(basis_64[c3 & 0x3F], outfile);
   }
*/
}

///
/// to64
//  Encodes a file using base64 format
void to64(FILE *infile, FILE *outfile, BOOL PortableNewlines)
{
   int c1, c2, c3, ct=0;
   InNewline = 0;
   while ((c1 = nextcharin(infile, PortableNewlines)) != -1)
   {
      c2 = nextcharin(infile, PortableNewlines);
      if (c2 == -1)
         output64chunk(c1, 0, 0, 2, outfile);
      else
      {
         c3 = nextcharin(infile, PortableNewlines);
         if (c3 == -1) output64chunk(c1, c2, 0, 1, outfile);
         else          output64chunk(c1, c2, c3, 0, outfile);
      }
      ct += 4;
      if (ct > 71) { fputc('\n', outfile); ct = 0; }
   }
   if (ct) fputc('\n', outfile);
}

///
/// almostputc
//  Writes a bytes, handles line breaks and translation tables
static void almostputc(int c, FILE *outfile, struct TranslationTable *tt, BOOL PortableNewlines)
{
   if (tt) c = (int)tt->Table[(UBYTE)c];
   if (CRpending) 
   {
      if (c == 10) { fputc('\n', outfile); CRpending = FALSE; }
      else
      {
         fputc(13, outfile);
         if (c != 13) { fputc(c, outfile); CRpending = FALSE; }
      }
   } 
   else 
      if (PortableNewlines && c == 13) CRpending = TRUE; else fputc(c, outfile);
}

///
/// from64txt
//  Decodes a string in base64 format
void from64txt(char *src, char *dst, struct TranslationTable *tt)
{
   int c1, c2, c3, c4;
   UBYTE c;

   while (c1 = (int)*src++)
   {
      if (ISpace((char)c1)) continue;
      do { c2 = (int)*src++; } while (c2 && ISpace((char)c2));
      do { c3 = (int)*src++; } while (c3 && ISpace((char)c3));
      do { c4 = (int)*src++; } while (c4 && ISpace((char)c4));
      if (!c2 || !c3 || !c4) return;
      c1 = char64(c1); c2 = char64(c2); c = (UBYTE)((c1<<2) | ((c2&0x30)>>4));
      *dst++ = tt ? (char)tt->Table[c] : (char)c;
      if (c3 != '=') 
      {
         c3 = char64(c3); c = (UBYTE)(((c2&0XF) << 4) | ((c3&0x3C) >> 2));
         *dst++ = tt ? (char)tt->Table[c] : (char)c;
         if (c4 != '=') 
         {
            c4 = char64(c4); c = (UBYTE)(((c3&0x03) <<6) | c4);
            *dst++ = tt ? (char)tt->Table[c] : (char)c;
         }
      }
   }
}

///
/// from64
//  Decodes a file in base64 format
void from64(FILE *infile, FILE *outfile, struct TranslationTable *tt, BOOL PortableNewlines)
{
   int c1, c2, c3, c4;
   BOOL DataDone = FALSE;

   CRpending = FALSE;
   while ((c1 = fgetc(infile)) != -1) 
   {
      if (ISpace((char)c1)) continue;
      if (DataDone) continue;
      do { c2 = fgetc(infile); } while (c2 != -1 && ISpace((char)c2));
      do { c3 = fgetc(infile); } while (c3 != -1 && ISpace((char)c3));
      do { c4 = fgetc(infile); } while (c4 != -1 && ISpace((char)c4));
      if (c2 == -1 || c3 == -1 || c4 == -1) 
      {
         ER_NewError(GetStr(MSG_ER_UnexpEOFB64), NULL, NULL);
         return;
      }
      if (c1 == '=' || c2 == '=') { DataDone = TRUE; continue; }
      c1 = char64(c1);
      c2 = char64(c2);
      almostputc(((c1<<2) | ((c2&0x30)>>4)), outfile, tt, PortableNewlines);
      if (c3 == '=') 
         DataDone = TRUE;
      else 
      {
         c3 = char64(c3);
         almostputc((((c2&0XF) << 4) | ((c3&0x3C) >> 2)), outfile, tt, PortableNewlines);
         if (c4 == '=') 
            DataDone = 1;
         else  
         {
            c4 = char64(c4);
            almostputc((((c3&0x03) <<6) | c4), outfile, tt, PortableNewlines);
         }
      }
   }
   if (CRpending) fputc(13, outfile);
}

///
/// toqp
//  Encodes a file using quoted-printable format
void toqp(FILE *infile, FILE *outfile)
{
   int c, ct = 0, prevc = 255;

   while ((c = fgetc(infile)) != -1)
   {
      if ((c < 32 && (c != '\n' && c != '\t')) || (c == '=') || (c >= 127) || (ct == 0 && c == '.')) 
      {
         fputc('=', outfile);
         fputc(basis_hex[c>>4], outfile);
         fputc(basis_hex[c&0xF], outfile);
         ct += 3;
         prevc = 'A';
      }
      else if (c == '\n') 
      {
         if (prevc == ' ' || prevc == '\t') 
         {
            fputs("=\n", outfile);
         }
         fputc('\n', outfile);
         ct = 0;
         prevc = c;
      } 
      else 
      {
         if (c == 'F' && prevc == '\n') 
         {
            if ((c = fgetc(infile)) == 'r') 
               if ((c = fgetc(infile)) == 'o') 
                  if ((c = fgetc(infile)) == 'm') 
                     if ((c = fgetc(infile)) == ' ') { fputs("=46rom", outfile);  ct += 6; }
                     else { fputs("From", outfile); ct += 4; }
                  else { fputs("Fro", outfile); ct += 3; }
               else { fputs("Fr", outfile); ct += 2; }
            else { fputc('F', outfile); ++ct; }
            ungetc(c, infile);
            prevc = 'x'; 
         } 
         else
         {
            fputc(c, outfile);
            ++ct;
            prevc = c;
         }
      }
      if (ct > 72) 
      {
         fputs("=\n", outfile);
         ct = 0;
         prevc = '\n';
      }
   }
   if (ct) fputs("=\n", outfile);
}

///
/// fromform
//  Converts an url-encoded file into plain text
void fromform(FILE *infile, FILE *outfile, struct TranslationTable *tt)
{
   unsigned int c;
   while ((c = fgetc(infile)) != -1)
   {
      switch (c)
      {
         case '&': fputc('\n', outfile); break;
         case '%': c = (index_hex[fgetc(infile)]<<4)+index_hex[fgetc(infile)];
                   switch (c)
                   {
                      case '\n': fputs("\n ", outfile); break;
                      case '\r': break;
                      default  : fputc(tt ? (int)tt->Table[(UBYTE)c] : c, outfile); break;
                   }
                   break;
         case '+': fputc(' ', outfile); break;
         case '=': fputs(" = ", outfile); break;
         default : fputc(tt ? (int)tt->Table[(UBYTE)c] : c, outfile); break;
      }
   }
}

///
/// fromqptxt
//  Decodes a string in quoted-printable format
void fromqptxt(char *src, char *dst, struct TranslationTable *tt)
{
   unsigned int c1, c2;
   UBYTE c;

   while (c1 = *src++) 
      if (c1 == '=') 
      {
         c1 = *src++; c2 = *src++;
         c1 = hexchar(c1); c2 = hexchar(c2); c = (UBYTE)(c1<<4 | c2);
         *dst++ = tt ? (char)tt->Table[c] : (char)c;
      }
      else *dst++ = tt ? (char)tt->Table[(UBYTE)c1] : (char)c1;
}

///
/// fromqp
//  Decodes a file in quoted-printable format
void fromqp(FILE *infile, FILE *outfile, struct TranslationTable *tt)
{
   unsigned int c1, c2;
   BOOL neednewline = FALSE;

   while ((c1 = fgetc(infile)) != -1) 
   {
      if (neednewline) { fputc('\n', outfile); neednewline = FALSE; };
      if (c1 == '=')
      {
         c1 = fgetc(infile);
         if (c1 != '\n')
         {
            c2 = fgetc(infile);
            c1 = hexchar(c1);
            c2 = hexchar(c2);
            fputc(tt ? (int)tt->Table[(UBYTE)(c1<<4 | c2)] : c1<<4 | c2, outfile);
         }
      }
      else if (c1 == '\n') neednewline = TRUE;
      else fputc(tt ? (int)tt->Table[(UBYTE)c1] : c1, outfile);
   }
   if (neednewline) fputc('\n', outfile);
}

///
/// DoesNeedPortableNewlines
//  Checks if line breaks must be portable (CRLF)
BOOL DoesNeedPortableNewlines(char *ctype)
{
   if (!strnicmp(ctype, "text", 4)) return TRUE;
   if (!strnicmp(ctype, "message", 7)) return TRUE;
   if (!strnicmp(ctype, "multipart", 9)) return TRUE;
   return FALSE;
}
///

/*** UU encode/decode stuff ***/
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
      else if (c == '\n' || c == '\r') { *ptr = '\0'; return False; }
      // Emm: I guess the following line was meant to process quoted
      // mails, but it causes file corruption when the '>' is really
      // part of the uuencoding (usually, this happens for the last
      // line).
      //else if (ptr == buf && c == '>') continue;
      else if (size > 0) { *ptr++ = c; size--; }
   } while (TRUE);
   return False;
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
      else if (c == '\n' || c == '\r') { *ptr = '\0'; return False; }
      // Emm: I guess the following line was meant to process quoted
      // mails, but it causes file corruption when the '>' is really
      // part of the uuencoding (usually, this happens for the last
      // line).
      //else if (ptr == buf && c == '>') continue;
      else if (size > 0) { *ptr++ = c; size--; }
   } while (TRUE);
   return False;
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
/// outdec
//  Encodes three bytes using UUE format
static int outdec(char *p, FILE *out)
{
   int c1,c2,c3,c4;

   c1 = *p >> 2;
   c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
   c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
   c4 = p[2] & 077;
   fputc(ENC(c1), out);
   fputc(ENC(c2), out);
   fputc(ENC(c3), out);
   fputc(ENC(c4), out);
   return (p[0]+p[1]+p[2]) % SUMSIZE;
}

///
/// touue
//  Encodes a file using UUE format
void touue(FILE *in, FILE *out)
{
   char buf[80];
   int i,n,checksum;

   for (;;) 
   {
      n = fread(buf, 1, 45, in);
      fputc(ENC(n), out);
      checksum = 0;
      for (i = 0; i < n; i += 3) checksum = (checksum+outdec(&buf[i], out))%SUMSIZE;
      fputc(ENC(checksum), out);
      fputc('\n', out);
      if (n <= 0) break;
   }
}
///

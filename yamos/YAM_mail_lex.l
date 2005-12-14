/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2005 by YAM Open Source Team

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

/*
 * Remember to call flex with the -i in order to create a case-insensitive
 * scanner:
 * flex -i lexer.l
 */

%option noyywrap
%option never-interactive

%{
    #include <string.h>
    #include <proto/exec.h>
    #include "YAM_mail_lex.h"
    #include "YAM_utilities.h"

    #define YY_NEVER_INTERACTIVE 1
    #define YY_NO_INPUT 1
%}

/*
 * The URL matching rules for this lexer try to be compatible to the official
 * RFC 1738 (http://www.faqs.org/rfcs/rfc1738.html) which defines how common
 * URLs look like and defines the correct BNF form for matching these URLs
 *
 * However, minor adaptions were made to make the URL matching more compatible to
 * the way we require it in our text parser in YAM
 *
 */

/* common rules */
LOGIN       ({USER}?(":"{PASSWORD})?"@")?{HOSTPORT}
HOSTPORT    {HOST}(":"{PORT})?
HOST        ({HOSTNAME}|{HOSTNUMBER})
HOSTNAME    ({DOMAINLABEL}".")+{TOPLABEL}
DOMAINLABEL ([[:alnum:]]|[[:alnum:]]([[:alnum:]]|"-")*[[:alnum:]])
TOPLABEL    ([[:alpha:]]|[[:alpha:]]([[:alnum:]]|"-")*[[:alnum:]])
HOSTNUMBER  [[:digit:]]{1,3}"."[[:digit:]]{1,3}"."[[:digit:]]{1,3}"."[[:digit:]]{1,3}
PORT        [[:digit:]]+
USER        ({UCHAR}|";"|"?"|"&"|"=")*
PASSWORD    ({UCHAR}|";"|"?"|"&"|"=")*
URLPATH     {XCHAR}*

/* FTP (see also RFC959) */
FTPURL      "ftp://"{LOGIN}("/"{FPATH}(";type="{FTPTYPE})?)?{NOTATEOL}
FPATH       {FSEGMENT}("/"{FSEGMENT})*
FSEGMENT    ({UCHAR}|"?"|":"|"@"|"&"|"=")*
FTPTYPE     [AIDaid]

/* FILE */
FILEURL     "file://"({HOST}|"localhost")?"/"{FPATH}{NOTATEOL}

/* HTTP/HTTPS/PLAINURL */
HTTPURL     "http://"{HOSTPORT}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
HTTPSURL    "https://"{HOSTPORT}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
PLAINURL    ("www"|"ftp")"."{HOSTPORT}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
HPATH       "~"?{HSEGMENT}("/"{HSEGMENT})*
HSEGMENT    ({UCHAR}|";"|":"|"@"|"&"|"="|"#")*
SEARCH      ({UCHAR}|";"|":"|"@"|"&"|"="|"#"|"/")*

/* GOPHER (see also RFC1436) */
GOPHERURL   "gopher://"{HOSTPORT}("/"({GTYPE}({SELECTOR}("%09"{SEARCH}("%09"{GOPHSTR})?)?)?)?)?{NOTATEOL}
GTYPE       {XCHAR}
SELECTOR    {XCHAR}*
GOPHSTR     {XCHAR}*

/* MAILTO (see also RFC822 */
MAILTOURL   "mailto:"{ENC822ADDR}("?"{SEARCH})?{NOTATEOL}
ENC822ADDR  ([[:alnum:]]|{SAFE})+"@"{HOST}

/* NEWS (see also RFC1036 */
NEWSURL     "news:"{GROUPART}{NOTATEOL}
GROUPART    ("*"|{GROUP}|{ARTICLE})
GROUP       [[:alpha:]]([[:alnum:]]|"-"|"."|"+"|"_")*
ARTICLE     ({UCHAR}|";"|"/"|"?"|":"|"&"|"=")+"@"{HOST}

/* NNTP (see also RFC977 */
NNTPURL     "nntp://"{HOSTPORT}"/"{GROUP}("/"[[:digit:]]+)?{NOTATEOL}

/* TELNET */
TELNETURL   "telnet://"{LOGIN}("/")?

/* Miscellaneous definitions */
SAFE        ("$"|"-"|"_"|"."|"+")
EXTRA       ("!"|"*"|"'"|"("|")"|",")
RESERVED    (";"|"/"|"?"|":"|"@"|"&"|"=")
ESCAPE      "%"[[:xdigit:]]{2}
UNRESERVED  ([[:alnum:]]|{SAFE}|{EXTRA})
UCHAR       ({UNRESERVED}|{ESCAPE})
XCHAR       ({UNRESERVED}|{RESERVED}|{ESCAPE})
NOTATEOL    ([^[:punct:][:space:]]|{ESCAPE}|"/"|"=")

%%
^"<sb>"                               {return tSB ; }
^"<tsb>"                              {return tTSB; }
^([[:blank:]]*[[:alpha:]]{0,3}">")+   {return tCITE;}

" "  {return tSPACE      ;}
"  " {return tDOUBLESPACE;}
"\t" {return tTAB        ;}
"("  {return tBEGINPAREN ;}
")"  {return tENDPAREN   ;}
"<"  {return tLESS       ;}
">"  {return tGREATER    ;}
"&"  {return tAMPERSAND  ;}
"\n" {return tNEWLINE    ;}
"*"  {return tSTAR       ;}
"/"  {return tSLASH      ;}
"_"  {return tUNDERSCORE ;}
"#"  {return tHASH       ;}

[[:blank:]]"*"[^[:blank:]\n*]([^\n*]*[^[:blank:]\n*])?"*"[[:blank:]\n,.!?:]       |
^"*"[^[:blank:]\n*]([^\n*]*[^[:blank:]\n*])?"*"[[:blank:]\n,.!?:]                 {return tBOLD      ;}

[[:blank:]]"/"[^[:blank:]\n/]([^\n/]*[^[:blank:]\n/])?"/"[[:blank:]\n,.!?:]       |
^"/"[^[:blank:]\n/]([^\n/]*[^[:blank:]\n/])?"/"[[:blank:]\n,.!?:]                 {return tITALIC    ;}

[[:blank:]]"_"[^[:blank:]\n_]([^\n_]*[^[:blank:]\n_])?"_"[[:blank:]\n,.!?:]       |
^"_"[^[:blank:]\n_]([^\n_]*[^[:blank:]\n_])?"_"[[:blank:]\n,.!?:]                 {return tUNDERLINE ;}

[[:blank:]]"#"[^[:blank:]\n#?]([^\n#]*[^[:blank:]\n#])?"#"[[:blank:]\n,.!?:]      |
^"#"[^[:blank:]\n#?]([^\n#]*[^[:blank:]\n#])?"#"[[:blank:]\n,.!?:]                {return tCOLORED   ;}

{HTTPURL}                                      {return tHTTP      ;}
{HTTPSURL}                                     {return tHTTPS     ;}
{FTPURL}                                       {return tFTP       ;}
{FILEURL}                                      {return tFILE      ;}
{GOPHERURL}                                    {return tGOPHER    ;}
{MAILTOURL}                                    {return tMAILTO    ;}
{NEWSURL}                                      |
{NNTPURL}                                      {return tNEWS      ;}
{TELNETURL}                                    {return tTELNET    ;}
{ENC822ADDR}                                   {return tEMAIL     ;}
{PLAINURL}                                     {return tURL       ;}
.                                              {return tNORMALTEXT;}

%%
/// ParseEmailText()
//  Function to parse the supplied text and replace text passages that contain
//  tokens that could be sourrounded by textstyles.
char *ParseEmailText(char *mailTxt)
{
  char *cmsg = NULL;
  char tmp[] = " ";
  int len, wptr=0;
  YY_BUFFER_STATE buffer;
  enum tokenType type;

  if(!mailTxt) return NULL;

  // lets prepare the mailTxt for the lexer
  if((buffer = yy_scan_string(mailTxt)))
  {
    if((cmsg = calloc(len=(strlen(mailTxt)*3)/2+1, sizeof(char))))
    {
      BOOL centerActivated = FALSE;
      char *colorTag = NULL;

      while((type = yylex()))
      {
        switch(type)
        {
          // if this token is a URL we have to mark it with a color
          case tEMAIL:
          case tHTTP:
          case tHTTPS:
          case tFTP:
          case tFILE:
          case tGOPHER:
          case tTELNET:
          case tMAILTO:
          case tNEWS:
          case tURL:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033p[12]");
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, colorTag ? colorTag : "\033p[0]");
          }
          break;

          case tTSB:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033c\033[s:18]");
            centerActivated = TRUE;
          }
          break;

          case tSB:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033c\033[s:2]");
            centerActivated = TRUE;
          }
          break;

          case tBOLD:
          {
            // first we check if we have to write something back
            if(yytext[0] != '*')
            {
              tmp[0] = yytext[0];
              cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
              yytext++;
              yyleng--;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033b");
            yytext[yyleng-2] = '\0';
            cmsg = AppendToBuffer(cmsg, &wptr, &len, &yytext[1]);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033n");
            unput(yytext[yyleng-1]);
          }
          break;

          case tITALIC:
          {
            // first we check if we have to write something back
            if(yytext[0] != '/')
            {
              tmp[0] = yytext[0];
              cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
              yytext++;
              yyleng--;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033i");
            yytext[yyleng-2] = '\0';
            cmsg = AppendToBuffer(cmsg, &wptr, &len, &yytext[1]);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033n");
            unput(yytext[yyleng-1]);
          }
          break;

          case tUNDERLINE:
          {
            // first we check if we have to write something back
            if(yytext[0] != '_')
            {
              tmp[0] = yytext[0];
              cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
              yytext++;
              yyleng--;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033u");
            yytext[yyleng-2] = '\0';
            cmsg = AppendToBuffer(cmsg, &wptr, &len, &yytext[1]);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033n");
            unput(yytext[yyleng-1]);
          }
          break;

          case tCOLORED:
          {
            // first we check if we have to write something back
            if(yytext[0] != '#')
            {
              tmp[0] = yytext[0];
              cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
              yytext++;
              yyleng--;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033p[7]");
            yytext[yyleng-2] = '\0';
            cmsg = AppendToBuffer(cmsg, &wptr, &len, &yytext[1]);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, colorTag ? colorTag : "\033p[0]");
            unput(yytext[yyleng-1]);
          }
          break;

          case tCITE:
          {
            char *ptr;
            int quotelevel = 0;

            while((ptr = strchr(yytext, '>')))
            {
              quotelevel++;

              // prepare the ColorTag
              if(quotelevel%4 == 1)       colorTag = "\033p[8]";
              else if(quotelevel%4 == 2)  colorTag = "\033p[9]";
              else if(quotelevel%4 == 3)  colorTag = "\033p[10]";
              else if(quotelevel%4 == 0)  colorTag = "\033p[11]";

              cmsg = AppendToBuffer(cmsg, &wptr, &len, colorTag);
              *ptr = '\0'; // temporary set the > to a null byte so that we can append to our buffer
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
              cmsg = AppendToBuffer(cmsg, &wptr, &len, ">");
              yytext = ++ptr;
            }
          }
          break;

          default:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

            if(type == tNEWLINE)
            {
              if(centerActivated)
              {
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033l");
                centerActivated = FALSE;
              }
              colorTag = NULL;
            }
          }
          break;
        }
      }
    }
    yy_delete_buffer(buffer);
  }

  return cmsg;
}

///
/// ExtractURL()
enum tokenType ExtractURL(char *text, char *resultBuffer)
{
  YY_BUFFER_STATE buffer;
  enum tokenType type;
  enum tokenType result = 0;

  if(!text || !resultBuffer) return 0;

  // clear the resultBuffer
  resultBuffer[0] = '\0';

  // lets prepare the text for the lexer
  if((buffer = yy_scan_string(text)))
  {
    while(!result && (type = yylex()))
    {
      switch(type)
      {
        // all recognizeable tokens can be returned.
        case tEMAIL:
        case tHTTP:
        case tHTTPS:
        case tFTP:
        case tFILE:
        case tGOPHER:
        case tTELNET:
        case tMAILTO:
        case tNEWS:
        case tURL:
        {
          // copy yytext to the resultBuffer
          strcpy(resultBuffer, yytext);
          result = type;
        }
        break;

        default:
          // nothing
        break;
      }
    }

    yy_delete_buffer(buffer);
  }

  return result;
}

///


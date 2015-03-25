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

/*
 * Remember to call flex with the -i in order to create a case-insensitive
 * scanner:
 * flex -i lexer.l
 */

%option noyywrap
%option never-interactive
%option case-insensitive
%option prefix="pm_"
%option nodefault
%option warn

%{
    #include <string.h>

    #include "YAM_read.h"

    #include "Config.h"
    #include "DynamicString.h"
    #include "ParseEmail.h"

    #include "Debug.h"

    #define YY_NO_INPUT 1
    #define YY_NO_UNISTD_H 1

    // check for the FLEX version and that the
    // developer uses the minimum version required
    #if YY_FLEX_MAJOR_VERSION < 2 || \
        (YY_FLEX_MAJOR_VERSION == 2 && (YY_FLEX_MINOR_VERSION < 5 || \
                                        (YY_FLEX_MINOR_VERSION == 5 && YY_FLEX_SUBMINOR_VERSION < 27)))
    #error "minimum flex version 2.5.27 is required to build"
    #endif

    // now we check the recommended flex version and
    // output a warning instead.
    #if YY_FLEX_MAJOR_VERSION < 2 || \
        (YY_FLEX_MAJOR_VERSION == 2 && (YY_FLEX_MINOR_VERSION < 5 || \
                                        (YY_FLEX_MINOR_VERSION == 5 && YY_FLEX_SUBMINOR_VERSION < 35)))
    #warning "YAM is compiled with a flex version < 2.5.35. This might cause unexpected problems or performance issues."
    #endif
%}

/*
 * The URL matching rules for this lexer try to be compatible to the official
 * RFC 3986 (http://www.faqs.org/rfcs/rfc3986.html) which defines how common
 * URL/URI look like and defines the correct BNF form for matching these URLs
 *
 * However, minor adaptions were made to make the URL matching more compatible to
 * the way we require it in our text parser in YAM
 *
 */

/* common rules */
AUTHORITY   ({USER}?(":"{PASSWORD})?"@")?{HOSTPORT}
HOSTPORT    {HOST}(":"{PORT})?
HOST        ({HOSTNAME}|{HOSTNUMBER})
HOSTNAME    ([[:alnum:]]|"-"|"."|"_"|"-")+
HOSTNUMBER  [[:digit:]]{1,3}"."[[:digit:]]{1,3}"."[[:digit:]]{1,3}"."[[:digit:]]{1,3}
PORT        [[:digit:]]+
USER        ({UCHAR}|";"|"?"|"&"|"=")*
PASSWORD    ({UCHAR}|";"|"?"|"&"|"=")*
URLPATH     {XCHAR}*

/* FTP (see also RFC959) */
FTPURL      "ftp://"{AUTHORITY}("/"{FPATH}(";type="{FTPTYPE})?)?{NOTATEOL}
FPATH       {FSEGMENT}("/"{FSEGMENT})*
FSEGMENT    ({UCHAR}|"?"|":"|"@"|"&"|"=")*
FTPTYPE     [AIDaid]

/* FILE */
FILEURL     "file://"{AUTHORITY}?"/"{FPATH}{NOTATEOL}

/* HTTP/HTTPS/PLAINURL */
HTTPURL     "http://"{AUTHORITY}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
HTTPSURL    "https://"{AUTHORITY}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
PLAINURL    ("www"|"ftp")"."{HOSTNAME}"."[[:alpha:]]{1,4}("/"{HPATH}("?"{SEARCH})?)?{NOTATEOL}
HPATH       "~"?{HSEGMENT}("/""~"?{HSEGMENT})*
HSEGMENT    ({UCHAR}|[^ \t\r\n]|";"|":"|"@"|"&"|"="|"#")*
SEARCH      ({UCHAR}|[^ \t\r\n]|";"|":"|"@"|"&"|"="|"#"|"/")*

/* GOPHER (see also RFC1436) */
GOPHERURL   "gopher://"{AUTHORITY}("/"({GTYPE}({SELECTOR}("%09"{SEARCH}("%09"{GOPHSTR})?)?)?)?)?{NOTATEOL}
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
NNTPURL     "nntp://"{AUTHORITY}"/"{GROUP}("/"[[:digit:]]+)?{NOTATEOL}

/* TELNET */
TELNETURL   "telnet://"{AUTHORITY}"/"?

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
^"-- \n"                              {return tSIGNATURE;}
^"\033c\033[s:18]"                    {return tNEXTPART;}
^[-_]{30,}"\n"{2}                     {return tENDSIGNATURE;}

[[:blank:]]"*"[^[:blank:]\n*]([^\n*_/#]*[^[:blank:]\n*_/#])?"*"[[:blank:]\n,.!?:]   |
^"*"[^[:blank:]\n*]([^\n*_/#]*[^[:blank:]\n*_/#])?"*"[[:blank:]\n,.!?:]             {return tBOLD      ;}

[[:blank:]]"/"[^[:blank:]\n/]([^\n/_*#]*[^[:blank:]\n/_*#])?"/"[[:blank:]\n,.!?:]   |
^"/"[^[:blank:]\n/]([^\n/_*#]*[^[:blank:]\n/_*#])?"/"[[:blank:]\n,.!?:]             {return tITALIC    ;}

[[:blank:]]"_"[^[:blank:]\n_]([^\n_*#/]*[^[:blank:]\n_*#/])?"_"[[:blank:]\n,.!?:]   |
^"_"[^[:blank:]\n_]([^\n_*#/]*[^[:blank:]\n_*#/])?"_"[[:blank:]\n,.!?:]             {return tUNDERLINE ;}

[[:blank:]]"#"[^[:blank:]\n#?]([^\n#*_/]*[^[:blank:]\n#*_/])?"#"[[:blank:]\n,.!?:]  |
^"#"[^[:blank:]\n#?]([^\n#*_/]*[^[:blank:]\n#*_/])?"#"[[:blank:]\n,.!?:]            {return tCOLORED   ;}

" "            {return tSPACE      ;}
"\t"           {return tTAB        ;}
"("            {return tBEGINPAREN ;}
")"            {return tENDPAREN   ;}
"<"            {return tLESS       ;}
">"            {return tGREATER    ;}
"&"            {return tAMPERSAND  ;}
"\n"           {return tNEWLINE    ;}
"*"            {return tSTAR       ;}
"/"            {return tSLASH      ;}
"_"            {return tUNDERSCORE ;}
"#"            {return tHASH       ;}

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
char *ParseEmailText(const char *mailTxt, const BOOL handleSigDash,
                     const BOOL useStyles, const BOOL useColors)
{
  char *cmsg = NULL;
  YY_BUFFER_STATE buffer;
  enum tokenType type;

  ENTER();

  if(mailTxt == NULL)
  {
    RETURN(NULL);
    return NULL;
  }

  // lets prepare the mailTxt for the lexer
  if((buffer = yy_scan_string(mailTxt)))
  {
    if((cmsg = dstralloc((strlen(mailTxt)*3)/2+1)) != NULL)
    {
      BOOL centerActivated = FALSE;
      BOOL signatureFound = FALSE;
      const char *defaultPen = "\033p[0]";

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
            if(useColors)
            {
              dstrcat(&cmsg, "\033p[12]\033u");
              dstrcat(&cmsg, yytext);
              dstrcat(&cmsg, "\033n");
              dstrcat(&cmsg, defaultPen);
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tTSB:
          {
            // we only allow that obsolete <tsb> stuff to work within signatures
            if(signatureFound)
            {
              dstrcat(&cmsg, "\033c\033[s:18]");
              centerActivated = TRUE;
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tSB:
          {
            // we only allow that obsolete <sb> stuff to work within signatures
            if(signatureFound)
            {
              dstrcat(&cmsg, "\033c\033[s:2]");
              centerActivated = TRUE;
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tBOLD:
          {
            if(useStyles)
            {
              char *p;

              if((p = strchr(yytext, '*')))
              {
                char *q;

                *p++ = '\0';
                dstrcat(&cmsg, yytext);

                q = p;
                if((q = strchr(p, '*')))
                {
                  *q++ = '\0';

                  dstrcat(&cmsg, "\033b*");
                  dstrcat(&cmsg, p);
                  dstrcat(&cmsg, "*\033n");

                  if(*q != '\0')
                    yyunput(*q, yytext);
                }
                else
                  W(DBF_MAIL, "couldn't find ending '*'");
              }
              else
                W(DBF_MAIL, "couldn't find starting '*'");
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tITALIC:
          {
            if(useStyles)
            {
              char *p;

              if((p = strchr(yytext, '/')))
              {
                char *q;

                *p++ = '\0';
                dstrcat(&cmsg, yytext);

                q = p;
                if((q = strchr(p, '/')))
                {
                  *q++ = '\0';

                  dstrcat(&cmsg, "\033i/");
                  dstrcat(&cmsg, p);
                  dstrcat(&cmsg, "/\033n");

                  if(*q != '\0')
                    yyunput(*q, yytext);
                }
                else
                  W(DBF_MAIL, "couldn't find ending '/'");
              }
              else
                W(DBF_MAIL, "couldn't find starting '/'");
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tUNDERLINE:
          {
            if(useStyles)
            {
              char *p;

              if((p = strchr(yytext, '_')))
              {
                char *q;

                *p++ = '\0';
                dstrcat(&cmsg, yytext);

                q = p;
                if((q = strchr(p, '_')))
                {
                  *q++ = '\0';

                  dstrcat(&cmsg, "\033u_");
                  dstrcat(&cmsg, p);
                  dstrcat(&cmsg, "_\033n");

                  if(*q != '\0')
                    yyunput(*q, yytext);
                }
                else
                  W(DBF_MAIL, "couldn't find ending '_'");
              }
              else
                W(DBF_MAIL, "couldn't find starting '_'");
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tCOLORED:
          {
            if(useColors)
            {
              char *p;

              if((p = strchr(yytext, '#')))
              {
                char *q;

                *p++ = '\0';
                dstrcat(&cmsg, yytext);

                q = p;
                if((q = strchr(p, '#')))
                {
                  *q++ = '\0';

                  dstrcat(&cmsg, "\033p[7]#");
                  dstrcat(&cmsg, p);
                  dstrcat(&cmsg, "#");
                  dstrcat(&cmsg, defaultPen);

                  if(*q != '\0')
                    yyunput(*q, yytext);
                }
                else
                  W(DBF_MAIL, "couldn't find ending '#'");
              }
              else
                W(DBF_MAIL, "couldn't find starting '#'");
            }
            else
              dstrcat(&cmsg, yytext);
          }
          break;

          case tCITE:
          {
            // we only allow analyzing quoting chars '>'
            // in case we are not in the signature part of
            // the mail
            char *ptr;
            int quotelevel = 0;

            while((ptr = strchr(yytext, '>')))
            {
              quotelevel++;

              if(useColors)
              {
                // prepare the ColorTag
                if(quotelevel%4 == 1)
                  defaultPen = "\033p[8]";
                else if(quotelevel%4 == 2)
                  defaultPen = "\033p[9]";
                else if(quotelevel%4 == 3)
                  defaultPen = "\033p[10]";
                else if(quotelevel%4 == 0)
                  defaultPen = "\033p[11]";

                dstrcat(&cmsg, defaultPen);
              }

              *ptr = '\0'; // temporary set the > to a null byte so that we can append to our buffer
              dstrcat(&cmsg, yytext);
              dstrcat(&cmsg, ">");
              yytext = ++ptr;
            }
          }
          break;

          case tSIGNATURE:
          {
            enum SigSepType sst = C->SigSepLine;

            // as we are in the signature area of the
            // mail now, we can set a new defaultPen (the ghosted one)
            if(useColors)
              defaultPen = "\033p[13]";

            // in case the user doesn't want to
            // tinker around with the signature dash we
            // just add it right away.
            if(handleSigDash == FALSE || signatureFound == TRUE)
              sst = SST_DASH;

            switch(sst)
            {
              case SST_DASH:
              {
                if(useColors)
                  dstrcat(&cmsg, defaultPen);

                dstrcat(&cmsg, yytext);
              }
              break;

              case SST_BAR:
                dstrcat(&cmsg, "\033[s:2]\n");
              break;

              case SST_BLANK:
                dstrcat(&cmsg, "\n");
              break;

              case SST_SKIP:
                // nothing
              break;
            }

            if(useColors)
              dstrcat(&cmsg, defaultPen);

            // mark the signature as 'found' so that we
            // can show text somewhat in another color and
            // also skip further signature things
            signatureFound = TRUE;
          }
          break;

          case tENDSIGNATURE:
          {
            // if we are in signature mode we have to reset that state
            // in case we find a "\nFrom: " line as this might point out
            // that this mail may be a message digest from a mailing list
            // and as such we don't want to have it highlighted as a
            // signature.
            if(signatureFound)
            {
              defaultPen = "\033p[0]";

              if(useColors)
                dstrcat(&cmsg, defaultPen);

              signatureFound = FALSE;
            }

            dstrcat(&cmsg, yytext);
          }
          break;

          case tNEXTPART:
          {
            dstrcat(&cmsg, yytext);

            // we reset the signature state
            // as we are running into text of a new part
            signatureFound = FALSE;
          }
          break;

          case tNEWLINE:
          {
            dstrcat(&cmsg, yytext);

            if(centerActivated)
            {
              dstrcat(&cmsg, "\033l");
              centerActivated = FALSE;
            }

            if(signatureFound == FALSE)
              defaultPen = "\033p[0]";
            else
            {
              defaultPen = "\033p[13]";

              if(useColors)
                dstrcat(&cmsg, defaultPen);
            }
          }
          break;

          default:
            dstrcat(&cmsg, yytext);
          break;
        }
      }

      // we have to end the text with a color clearing ESC sequence
      // in case the signature was active at last
      if(signatureFound && useColors)
        dstrcat(&cmsg, "\033p[0]");
    }

    yy_delete_buffer(buffer);

    // call the destroy function ourself to avoid memory leaks, because flex doesn't do it
    pm_lex_destroy();
  }

  RETURN(cmsg);
  return cmsg;
}

///
/// ExtractURL()
enum tokenType ExtractURL(const char *text, char **resultBuffer)
{
  YY_BUFFER_STATE buffer;
  enum tokenType type;
  enum tokenType result = 0;

  ENTER();

  if(text != NULL && resultBuffer != NULL)
  {
    // clear the resultBuffer
    *resultBuffer = NULL;

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
            *resultBuffer = strdup(yytext);
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

    // call the destroy function ourself to avoid memory leaks, because flex doesn't do it
    pm_lex_destroy();
  }

  RETURN(result);
  return result;
}

///

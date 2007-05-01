/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2007 by YAM Open Source Team

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

    #include "YAM_mail_lex.h"
    #include "YAM_utilities.h"
    #include "YAM_config.h"
    #include "YAM_read.h"

    #include "Debug.h"

    #define YY_NO_INPUT 1
    #define YY_NO_UNISTD_H 1
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
^"-- \n"                              {return tSIGNATURE;}
^"\033c\033[s:18]"                    {return tNEXTPART;}
^"\n""-"{30,}"\n"                     {return tENDSIGNATURE;}

[[:blank:]]"*"[^[:blank:]\n*]([^\n*]*[^[:blank:]\n*])?"*"[[:blank:]\n,.!?:]       |
^"*"[^[:blank:]\n*]([^\n*]*[^[:blank:]\n*])?"*"[[:blank:]\n,.!?:]                 {return tBOLD      ;}

[[:blank:]]"/"[^[:blank:]\n/]([^\n/]*[^[:blank:]\n/])?"/"[[:blank:]\n,.!?:]       |
^"/"[^[:blank:]\n/]([^\n/]*[^[:blank:]\n/])?"/"[[:blank:]\n,.!?:]                 {return tITALIC    ;}

[[:blank:]]"_"[^[:blank:]\n_]([^\n_]*[^[:blank:]\n_])?"_"[[:blank:]\n,.!?:]       |
^"_"[^[:blank:]\n_]([^\n_]*[^[:blank:]\n_])?"_"[[:blank:]\n,.!?:]                 {return tUNDERLINE ;}

[[:blank:]]"#"[^[:blank:]\n#?]([^\n#]*[^[:blank:]\n#])?"#"[[:blank:]\n,.!?:]      |
^"#"[^[:blank:]\n#?]([^\n#]*[^[:blank:]\n#])?"#"[[:blank:]\n,.!?:]                {return tCOLORED   ;}

[:space:]    {return tSPACE      ;}
[:space:]{2} {return tDOUBLESPACE;}
"\t"         {return tTAB        ;}
"("          {return tBEGINPAREN ;}
")"          {return tENDPAREN   ;}
"<"          {return tLESS       ;}
">"          {return tGREATER    ;}
"&"          {return tAMPERSAND  ;}
"\n"         {return tNEWLINE    ;}
"*"          {return tSTAR       ;}
"/"          {return tSLASH      ;}
"_"          {return tUNDERSCORE ;}
"#"          {return tHASH       ;}

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
char *ParseEmailText(const char *mailTxt, BOOL handleSigDash)
{
  char *cmsg = NULL;
  int len, wptr=0;
  YY_BUFFER_STATE buffer;
  enum tokenType type;

  ENTER();

  if(!mailTxt)
  {
    RETURN(NULL);
    return NULL;
  }

  // lets prepare the mailTxt for the lexer
  if((buffer = yy_scan_string(mailTxt)))
  {
    if((cmsg = calloc(len=(strlen(mailTxt)*3)/2+1, sizeof(char))))
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
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033p[12]");
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);
          }
          break;

          case tTSB:
          {
            // we only allow that obsolete <tsb> stuff to work within signatures
            if(signatureFound)
            {
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033c\033[s:18]");
              centerActivated = TRUE;
            }
            else
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          }
          break;

          case tSB:
          {
            // we only allow that obsolete <sb> stuff to work within signatures
            if(signatureFound)
            {
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033c\033[s:2]");
              centerActivated = TRUE;
            }
            else
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          }
          break;

          case tBOLD:
          {
            char *p;
            char *q;

            if((p = strchr(yytext, '*')))
            {
              *p++ = '\0';
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

              q = p;
              if((q = strchr(p, '*')))
              {
                *q++ = '\0';

                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033b*");
                cmsg = AppendToBuffer(cmsg, &wptr, &len, p);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "*\033n");

                if(*q != '\0')
                  yyunput(*q, yytext);
              }
              else
                W(DBF_MAIL, "couldn't find ending '*'");
            }
            else
              W(DBF_MAIL, "couldn't find starting '*'");
          }
          break;

          case tITALIC:
          {
            char *p;
            char *q;

            if((p = strchr(yytext, '/')))
            {
              *p++ = '\0';
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

              q = p;
              if((q = strchr(p, '/')))
              {
                *q++ = '\0';

                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033i/");
                cmsg = AppendToBuffer(cmsg, &wptr, &len, p);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "/\033n");

                if(*q != '\0')
                  yyunput(*q, yytext);
              }
              else
                W(DBF_MAIL, "couldn't find ending '/'");
            }
            else
              W(DBF_MAIL, "couldn't find starting '/'");
          }
          break;

          case tUNDERLINE:
          {
            char *p;
            char *q;

            if((p = strchr(yytext, '_')))
            {
              *p++ = '\0';
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

              q = p;
              if((q = strchr(p, '_')))
              {
                *q++ = '\0';

                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033u_");
                cmsg = AppendToBuffer(cmsg, &wptr, &len, p);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "_\033n");

                if(*q != '\0')
                  yyunput(*q, yytext);
              }
              else
                W(DBF_MAIL, "couldn't find ending '_'");
            }
            else
              W(DBF_MAIL, "couldn't find starting '_'");
          }
          break;

          case tCOLORED:
          {
            char *p;
            char *q;

            if((p = strchr(yytext, '#')))
            {
              *p++ = '\0';
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

              q = p;
              if((q = strchr(p, '#')))
              {
                *q++ = '\0';

                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033p[7]#");
                cmsg = AppendToBuffer(cmsg, &wptr, &len, p);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "#");
                cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);

                if(*q != '\0')
                  yyunput(*q, yytext);
              }
              else
                W(DBF_MAIL, "couldn't find ending '*'");
            }
            else
              W(DBF_MAIL, "couldn't find starting '*'");
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

              // prepare the ColorTag
              if(quotelevel%4 == 1)
                defaultPen = "\033p[8]";
              else if(quotelevel%4 == 2)
                defaultPen = "\033p[9]";
              else if(quotelevel%4 == 3)
                defaultPen = "\033p[10]";
              else if(quotelevel%4 == 0)
                defaultPen = "\033p[11]";

              cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);
              *ptr = '\0'; // temporary set the > to a null byte so that we can append to our buffer
              cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
              cmsg = AppendToBuffer(cmsg, &wptr, &len, ">");
              yytext = ++ptr;
            }
          }
          break;

          case tSIGNATURE:
          {
            enum SigSepType sst = C->SigSepLine;

            // as we are in the signature area of the
            // mail now, we can set a new defaultPen (the ghosted one)
            defaultPen = "\033p[13]";

            // in case the user doesn't want to
            // tinker around with the signature dash we
            // just add it right away.
            if(handleSigDash == FALSE || signatureFound == TRUE)
              sst = SST_DASH;

            switch(sst)
            {
              case SST_DASH:
                cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);
                cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
              break;

              case SST_BAR:
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033[s:2]\n");
              break;

              case SST_BLANK:
                cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
              break;

              case SST_SKIP:
                // nothing
              break;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);

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
              cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);

              signatureFound = FALSE;
            }

            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          }
          break;

          case tNEXTPART:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

            // we reset the signature state
            // as we are running into text of a new part
            signatureFound = FALSE;
          }
          break;

          case tNEWLINE:
          {
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);

            if(centerActivated)
            {
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033l");
              centerActivated = FALSE;
            }

            if(signatureFound == FALSE)
              defaultPen = "\033p[0]";
            else
            {
              defaultPen = "\033p[13]";
              cmsg = AppendToBuffer(cmsg, &wptr, &len, defaultPen);
            }
          }
          break;

          default:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          break;
        }
      }

      // we have to end the text with a color clearing ESC sequence
      // in case the signature was active at last
      if(signatureFound)
        cmsg = AppendToBuffer(cmsg, &wptr, &len, "\033p[0]");
    }
    yy_delete_buffer(buffer);
  }

  RETURN(cmsg);
  return cmsg;
}

///
/// ExtractURL()
enum tokenType ExtractURL(const char *text, char *resultBuffer)
{
  YY_BUFFER_STATE buffer;
  enum tokenType type;
  enum tokenType result = 0;

  ENTER();

  if(text && resultBuffer)
  {
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
  }

  RETURN(result);
  return result;
}

///

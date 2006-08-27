/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2006 by YAM Open Source Team

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
%option nounput
%option never-interactive
%option case-insensitive
%option prefix="h2m_"
%option nodefault
%option warn

%{
    #include <string.h>
    #include <proto/exec.h>

    #include "HTML2Mail.h"
    #include "YAM_utilities.h"

    #include "Debug.h"

    #define YY_NO_INPUT 1
%}

/*
 * A simple but smart HTML2Text converter using the nice and smooth 'flex'
 * lexical analyzer tool instead of doing all the parsing manually.
 *
 */

/* common 'HTML' tag rules */

PARAGRAPH     "<""/"?"p"[^>]*">"
BR            "<br"[^>]*">"
HR            "<hr"[^>]*">"
UNKNOWN       "<""/"?[^>]*">"
UNKNOWN_CHAR  "&""#"?[[:alnum:]]+";"

%%
{PARAGRAPH}     { return ht_PARAGRAPH;    }
{BR}            { return ht_BR;           }
{HR}            { return ht_HR;           }
{UNKNOWN}       { return ht_UNKNOWN;      }

"&nbsp;"        { return ht_NBSP;         }
"&amp;"         { return ht_AMP;          }
"&gt;"          { return ht_GT;           }
"&lt;"          { return ht_LT;           }
"&quot;"        { return ht_QUOT;         }
"&auml;"        { return ht_AUML;         }
"&ouml;"        { return ht_OUML;         }
"&uuml;"        { return ht_UUML;         }
"&szlig;"       { return ht_SZLIG;        }
"&sup2;"        { return ht_SUP2;         }
"&sup3;"        { return ht_SUP3;         }
"&reg;"         { return ht_REG;          }
"&copy;"        { return ht_COPY;         }
{UNKNOWN_CHAR}  { return ht_UNKNOWN_CHAR; }

\n              { return ht_UNKNOWN;      }
.               { return ht_NORMALTEXT;   }

%%
/// html2mail()
// Function to parse through a HTML document and convert it to a
// "standard" RFC822 conform mail text message excluding any header
// information.
char *html2mail(char *htmlTxt)
{
  char *cmsg = NULL;
  YY_BUFFER_STATE buffer;

  ENTER();

  if(!htmlTxt)
  {
    RETURN(NULL);
    return NULL;
  }

  // lets prepare the htmlTxt for the lexer
  if((buffer = yy_scan_string(htmlTxt)))
  {
    int len, wptr=0;

    if((cmsg = calloc(len=(strlen(htmlTxt)*3)/2+1, sizeof(char))))
    {
      enum htmlTagType type;

      while((type = yylex()))
      {
        switch(type)
        {
          case ht_PARAGRAPH:
          case ht_BR:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
          break;

          case ht_HR:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n<sb>\n");
          break;

          case ht_UNKNOWN:
            // nothing
          break;

          case ht_NBSP:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, " ");
          break;

          case ht_AMP:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "&");
          break;

          case ht_GT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, ">");
          break;

          case ht_LT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "<");
          break;

          case ht_QUOT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\"");
          break;

          case ht_AUML:
          {
            if(yytext[1] == 'A')
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "Ä");
            else
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "ä");
          }
          break;

          case ht_OUML:
          {
            if(yytext[1] == 'O')
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "Ö");
            else
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "ö");
          }
          break;

          case ht_UUML:
          {
            if(yytext[1] == 'U')
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "Ü");
            else
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "ü");
          }
          break;

          case ht_SZLIG:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "ß");
          break;

          case ht_SUP2:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "²");
          break;

          case ht_SUP3:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "³");
          break;

          case ht_REG:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "®");
          break;

          case ht_COPY:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "©");
          break;

          case ht_UNKNOWN_CHAR:
            W(DBF_STARTUP, "unknown HTML char: '%s'", yytext);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "?");
          break;

          case ht_NORMALTEXT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          break;
        }
      }
    }

    yy_delete_buffer(buffer);
  }

  RETURN(cmsg);
  return cmsg;
}

///


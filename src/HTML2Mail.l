/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2011 YAM Open Source Team

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
    #include <ctype.h>

    #include <proto/exec.h>

    #include "HTML2Mail.h"
    #include "YAM_utilities.h"

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
 * A simple but smart HTML2Text converter using the nice and smooth 'flex'
 * lexical analyzer tool instead of doing all the parsing manually.
 */

/* start conditions */
%s tag_s tag_e comment style href pre

/* common 'HTML' tag rules */
NOT_GT        [^>]*
TAGEND        ("/"?">"|" "{NOT_GT}"/"?">")
TAGSTART      (">"|" "{NOT_GT}">")

BOLD          "<""/"?("b"|"strong"){NOT_GT}">"
ITALIC        "<""/"?("i"|"em"){NOT_GT}">"
UNDERLINE     "<""/"?"u"{NOT_GT}">"

ASCII_CHAR    "&#"[[:digit:]]{1,3}";"
UNKNOWN       "<"{NOT_GT}">"
UNKNOWN_CHAR  "&""#"?[[:alnum:]]+";"

%%

"<"                         BEGIN(tag_s);
<tag_s>"!--"                { BEGIN(comment); D(DBF_HTML, "<!--:     '%s'", yytext); }
<tag_s>"style"{TAGSTART}    { BEGIN(style);   D(DBF_HTML, "<style>:  '%s'", yytext); }
<tag_s>"pre"{TAGSTART}      { BEGIN(pre); }
<tag_s>"br"{TAGEND}         { BEGIN(INITIAL); return ht_BR;         }
<tag_s>"div"{TAGEND}        { BEGIN(INITIAL); return ht_DIV;        }
<tag_s>"hr"{TAGEND}         { BEGIN(INITIAL); return ht_HR;         }
<tag_s>"/"?"p"{TAGEND}      { BEGIN(INITIAL); return ht_PARAGRAPH;  }
<tag_s>"/h"[[:digit:]]">"   { BEGIN(INITIAL); return ht_HEAD;       }
<tag_s>"a "{NOT_GT}"href="  { BEGIN(href);    D(DBF_HTML, "<a href=: '%s'", yytext); }
<tag_s>"/a>"                { BEGIN(INITIAL); return ht_HREF_END;   }
<tag_s>"/dd>"               { BEGIN(INITIAL); return ht_DD;         }
<tag_s>"/dl>"               { BEGIN(INITIAL); return ht_DL;         }
<tag_s>"/dt>"               { BEGIN(INITIAL); return ht_DT;         }
<tag_s>"/ul>"               { BEGIN(INITIAL); return ht_UL;         }
<tag_s>"/li>"               { BEGIN(INITIAL); return ht_LI;         }
<tag_s>">"                  BEGIN(INITIAL);
<tag_s,tag_e>">"            BEGIN(INITIAL);
<tag_s>.|\n                 BEGIN(tag_e);
<tag_e>.|\n                 /* eat up everything else */

<comment>"-->"              { BEGIN(INITIAL); D(DBF_HTML, "--> :     '%s'", yytext); }
<comment>.|\n               /* eat up everything else */

<style>"</style>"           { BEGIN(INITIAL); D(DBF_HTML, "</style>: '%s'", yytext); }
<style>.|\n                 /* eat up everything else */

<href>"\""                  /* eat up quotes */
<href>[^ >"]+               { BEGIN(tag_e); return ht_HREF; }
<href>.|\n                  BEGIN(tag_e);

"&sp;"          { return ht_SP;           }
"&excl;"        { return ht_EXCL;         }
"&quot;"        { return ht_QUOT;         }
"&num;"         { return ht_NUM;          }
"&dollar;"      { return ht_DOLLAR;       }
"&percnt;"      { return ht_PERCNT;       }
"&amp;"         { return ht_AMP;          }
"&apos;"        { return ht_APOS;         }
"&lpar;"        { return ht_LPAR;         }
"&rpar;"        { return ht_RPAR;         }
"&ast;"         { return ht_AST;          }
"&plus;"        { return ht_PLUS;         }
"&comma;"       { return ht_COMMA;        }
"&minus;"       |
"&ndash;"       |
"&mdash;"       |
"&hyphen;"      { return ht_HYPHEN;       }
"&period;"      { return ht_PERIOD;       }
"&sol;"         { return ht_SOL;          }
"&colon;"       { return ht_COLON;        }
"&semi;"        { return ht_SEMI;         }
"&lt;"          { return ht_LT;           }
"&equals;"      { return ht_EQUALS;       }
"&gt;"          { return ht_GT;           }
"&quest;"       { return ht_QUEST;        }
"&commat;"      { return ht_COMMAT;       }
"&lsgb;"        { return ht_LSGB;         }
"&bsol;"        { return ht_BSOL;         }
"&rsgb;"        { return ht_RSGB;         }
"&circ;"        { return ht_CIRC;         }
"&horbar;"      |
"&lowbar;"      { return ht_LOWBAR;       }
"&grave;"       { return ht_GRAVE;        }
"&lcub;"        { return ht_LCUB;         }
"&verbar;"      { return ht_VERBAR;       }
"&rcub;"        { return ht_RCUB;         }
"&tilde;"       { return ht_TILDE;        }

"&nbsp;"        { return ht_NBSP;         }
"&iexcl;"       { return ht_IEXCL;        }
"&cent;"        { return ht_CENT;         }
"&pound;"       { return ht_POUND;        }
"&euro;"        |
"&curren;"      { return ht_CURREN;       }
"&yen;"         { return ht_YEN;          }
"&brvbar;"      |
"&brkbar;"      { return ht_BRKBAR;       }
"&sect;"        { return ht_SECT;         }
"&uml;"         |
"&die;"         { return ht_UML;          }
"&copy;"        { return ht_COPY;         }
"&ordf;"        { return ht_ORDF;         }
"&laquo;"       { return ht_LAQUO;        }
"&not;"         { return ht_NOT;          }
"&shy;"         { return ht_SHY;          }
"&reg;"         { return ht_REG;          }
"&macr;"        { return ht_MACR;         }
"&deg;"         { return ht_DEG;          }
"&plusmn;"      { return ht_PLUSMN;       }
"&sup2;"        { return ht_SUP2;         }
"&sup3;"        { return ht_SUP3;         }
"&acute;"       { return ht_ACUTE;        }
"&micro;"       { return ht_MICRO;        }
"&para;"        { return ht_PARA;         }
"&middot;"      { return ht_MIDDOT;       }
"&cedil;"       { return ht_CEDIL;        }
"&sup1;"        { return ht_SUP1;         }
"&ordm;"        { return ht_ORDM;         }
"&raquo;"       { return ht_RAQUO;        }
"&frac14;"      { return ht_FRAC14;       }
"&half;"        |
"&frac12;"      { return ht_FRAC12;       }
"&frac34;"      { return ht_FRAC34;       }
"&iquest;"      { return ht_IQUEST;       }

"&agrave;"      { return ht_AGRAVE;       }
"&aacute;"      { return ht_AACUTE;       }
"&acirc;"       { return ht_ACIRC;        }
"&atilde;"      { return ht_ATILDE;       }
"&auml;"        { return ht_AUML;         }
"&aring;"       { return ht_ARING;        }
"&aeling;"      { return ht_AELING;       }
"&ccedil;"      { return ht_CCEDIL;       }
"&egrave;"      { return ht_EGRAVE;       }
"&eacute;"      { return ht_EACUTE;       }
"&ecirc;"       { return ht_ECIRC;        }
"&euml;"        { return ht_EUML;         }
"&igrave;"      { return ht_IGRAVE;       }
"&iacute;"      { return ht_IACUTE;       }
"&icirc;"       { return ht_ICIRC;        }
"&iuml;"        { return ht_IUML;         }
"&eth;"         { return ht_ETH;          }
"&ntilde;"      { return ht_NTILDE;       }
"&ograve;"      { return ht_OGRAVE;       }
"&oacute;"      { return ht_OACUTE;       }
"&ocirc;"       { return ht_OCIRC;        }
"&otilde;"      { return ht_OTILDE;       }
"&ouml;"        { return ht_OUML;         }
"&times;"       { return ht_TIMES;        }
"&oslash;"      { return ht_OSLASH;       }
"&ugrave;"      { return ht_UGRAVE;       }
"&uacute;"      { return ht_UACUTE;       }
"&ucirc;"       { return ht_UCIRC;        }
"&uuml;"        { return ht_UUML;         }
"&yacute;"      { return ht_YACUTE;       }
"&thorn;"       { return ht_THORN;        }

"&szlig;"       { return ht_SZLIG;        }
"&divide;"      { return ht_DIVIDE;       }
"&yuml;"        { return ht_YUML;         }

"&#8482;"       |
"&trade;"       { return ht_TRADE;        }

<pre>"</pre>"   { BEGIN(INITIAL); return ht_PRE; }
<pre>.|\n       { return ht_NORMALTEXT; }

{ASCII_CHAR}    { return ht_ASCII_CHAR;   }
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
      char *lastHref = NULL;

      while((type = yylex()))
      {
        switch(type)
        {
          case ht_PARAGRAPH:
          case ht_BR:
          case ht_DIV:
          case ht_DD:
          case ht_DL:
          case ht_UL:
          case ht_LI:
          case ht_HEAD:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
          break;

          case ht_DT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n  ");
          break;

          case ht_PRE:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n\n");
          break;

          case ht_HR:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n---------------------------------------------------------------------------\n");
          break;

          case ht_BOLD:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "*");
          break;

          case ht_ITALIC:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "/");
          break;

          case ht_UNDERLINE:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "_");
          break;

          case ht_HREF:
          {
            free(lastHref);
            lastHref = strdup(yytext);
          }
          break;

          case ht_HREF_END:
          {
            if(lastHref != NULL)
            {
              cmsg = AppendToBuffer(cmsg, &wptr, &len, " <");
              cmsg = AppendToBuffer(cmsg, &wptr, &len, lastHref);
              cmsg = AppendToBuffer(cmsg, &wptr, &len, "> ");

              free(lastHref);
              lastHref = NULL;
            }
          }
          break;

          case ht_STYLE:
          case ht_COMMENT:
          case ht_UNKNOWN:
            // nothing
          break;

          case ht_SP:
          case ht_NBSP:
          case ht_EXCL:
          case ht_QUOT:
          case ht_NUM:
          case ht_DOLLAR:
          case ht_PERCNT:
          case ht_AMP:
          case ht_APOS:
          case ht_LPAR:
          case ht_RPAR:
          case ht_AST:
          case ht_PLUS:
          case ht_COMMA:
          case ht_HYPHEN:
          case ht_PERIOD:
          case ht_SOL:
          case ht_COLON:
          case ht_SEMI:
          case ht_LT:
          case ht_EQUALS:
          case ht_GT:
          case ht_QUEST:
          case ht_COMMAT:
          case ht_LSGB:
          case ht_BSOL:
          case ht_RSGB:
          case ht_CIRC:
          case ht_LOWBAR:
          case ht_GRAVE:
          case ht_LCUB:
          case ht_VERBAR:
          case ht_RCUB:
          case ht_TILDE:
          case ht_IEXCL:
          case ht_CENT:
          case ht_POUND:
          case ht_CURREN:
          case ht_YEN:
          case ht_BRKBAR:
          case ht_SECT:
          case ht_UML:
          case ht_COPY:
          case ht_ORDF:
          case ht_LAQUO:
          case ht_NOT:
          case ht_SHY:
          case ht_REG:
          case ht_MACR:
          case ht_DEG:
          case ht_PLUSMN:
          case ht_SUP2:
          case ht_SUP3:
          case ht_ACUTE:
          case ht_MICRO:
          case ht_PARA:
          case ht_MIDDOT:
          case ht_CEDIL:
          case ht_SUP1:
          case ht_ORDM:
          case ht_RAQUO:
          case ht_FRAC14:
          case ht_FRAC12:
          case ht_FRAC34:
          case ht_IQUEST:
          case ht_TIMES:
          case ht_DIVIDE:
          case ht_SZLIG:
          case ht_YUML:
          {
            char tmp[] = " \0";

            tmp[0] = type;

            cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
          }
          break;

          case ht_AGRAVE:
          case ht_AACUTE:
          case ht_ACIRC:
          case ht_ATILDE:
          case ht_AUML:
          case ht_ARING:
          case ht_AELING:
          case ht_CCEDIL:
          case ht_EGRAVE:
          case ht_EACUTE:
          case ht_ECIRC:
          case ht_EUML:
          case ht_IGRAVE:
          case ht_IACUTE:
          case ht_ICIRC:
          case ht_IUML:
          case ht_ETH:
          case ht_NTILDE:
          case ht_OGRAVE:
          case ht_OACUTE:
          case ht_OCIRC:
          case ht_OTILDE:
          case ht_OUML:
          case ht_OSLASH:
          case ht_UGRAVE:
          case ht_UACUTE:
          case ht_UCIRC:
          case ht_UUML:
          case ht_YACUTE:
          case ht_THORN:
          {
            char tmp[] = " \0";

            // check if the first char is lowercase
            // and if so we have to add 32 to our current
            // character value.
            if(tolower(yytext[1]) == yytext[1])
              tmp[0] = type + 32;
            else
              tmp[0] = type;

            cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
          }
          break;

          case ht_TRADE:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "(tm)");
          break;

          case ht_ASCII_CHAR:
          {
            unsigned int c = atoi(&yytext[2]);

            if(c >= 32 && c <= 255)
            {
              char tmp[] = " \0";

              tmp[0] = c;
              cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
            }
            else
              D(DBF_HTML, "found HTML ASCII char out of bounds: '%s'", yytext);
          }
          break;

          case ht_UNKNOWN_CHAR:
            D(DBF_HTML, "unknown HTML char: '%s'", yytext);
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "?");
          break;

          case ht_NORMALTEXT:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, yytext);
          break;
        }
      }

      free(lastHref);
    }

    yy_delete_buffer(buffer);

    // call the destroy function ourself to avoid memory leaks, because flex doesn't do it
    h2m_lex_destroy();
  }

  RETURN(cmsg);
  return cmsg;
}

///


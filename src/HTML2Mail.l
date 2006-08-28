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
    #include <ctype.h>

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

STYLE         "<style"[^>]*">"(.|\n)*"</style>"
PARAGRAPH     "<""/"?"p"[^>]*">"
BR            "<br"[^>]*">"
DIV           "<div"[^>]*">"
HR            "<hr"[^>]*">"
BOLD          "<""/"?("b"|"strong")[^>]*">"
ITALIC        "<""/"?("i"|"em")[^>]*">"
UNDERLINE     "<""/"?"u"[^>]*">"
UNKNOWN       "<""/"?[^>]*">"
UNKNOWN_CHAR  "&""#"?[[:alnum:]]+";"

%%

{STYLE}         { return ht_STYLE;        }
{PARAGRAPH}     { return ht_PARAGRAPH;    }
{BR}            { return ht_BR;           }
{DIV}           { return ht_DIV;          }
{HR}            { return ht_HR;           }
{UNKNOWN}       { return ht_UNKNOWN;      }

"&#032;"        |
"&sp;"          { return ht_SP;           }

"&#033;"        |
"&excl;"        { return ht_EXCL;         }

"&#034;"        |
"&quot;"        { return ht_QUOT;         }

"&#035;"        |
"&num;"         { return ht_NUM;          }

"&#036;"        |
"&dollar;"      { return ht_DOLLAR;       }

"&#037;"        |
"&percnt;"      { return ht_PERCNT;       }

"&#038;"        |
"&amp;"         { return ht_AMP;          }

"&#039;"        |
"&apos;"        { return ht_APOS;         }

"&#040;"        |
"&lpar;"        { return ht_LPAR;         }

"&#041;"        |
"&rpar;"        { return ht_RPAR;         }

"&#042;"        |
"&ast;"         { return ht_AST;          }

"&#043;"        |
"&plus;"        { return ht_PLUS;         }

"&#044;"        |
"&comma;"       { return ht_COMMA;        }

"&#045;"        |
"&minus;"       |
"&ndash;"       |
"&mdash;"       |
"&hyphen;"      { return ht_HYPHEN;       }

"&#046;"        |
"&period;"      { return ht_PERIOD;       }

"&#047;"        |
"&sol;"         { return ht_SOL;          }

"&#058;"        |
"&colon;"       { return ht_COLON;        }

"&#059;"        |
"&semi;"        { return ht_SEMI;         }

"&#060;"        |
"&lt;"          { return ht_LT;           }

"&#061;"        |
"&equals;"      { return ht_EQUALS;       }

"&#062;"        |
"&gt;"          { return ht_GT;           }

"&#063;"        |
"&quest;"       { return ht_QUEST;        }

"&#064;"        |
"&commat;"      { return ht_COMMAT;       }

"&#091;"        |
"&lsgb;"        { return ht_LSGB;         }

"&#092;"        |
"&bsol;"        { return ht_BSOL;         }

"&#093;"        |
"&rsgb;"        { return ht_RSGB;         }

"&#094;"        |
"&circ;"        { return ht_CIRC;         }

"&#095;"        |
"&horbar;"      |
"&lowbar;"      { return ht_LOWBAR;       }

"&#096;"        |
"&grave;"       { return ht_GRAVE;        }

"&#123;"        |
"&lcub;"        { return ht_LCUB;         }

"&#124;"        |
"&verbar;"      { return ht_VERBAR;       }

"&#125;"        |
"&rcub;"        { return ht_RCUB;         }

"&#126;"        |
"&tilde;"       { return ht_TILDE;        }


"&#160;"        |
"&nbsp;"        { return ht_NBSP;         }

"&#161;"        |
"&iexcl;"       { return ht_IEXCL;        }

"&#162;"        |
"&cent;"        { return ht_CENT;         }

"&#163;"        |
"&pound;"       { return ht_POUND;        }

"&#164;"        |
"&euro;"        |
"&curren;"      { return ht_CURREN;       }

"&#165;"        |
"&yen;"         { return ht_YEN;          }

"&#166;"        |
"&brvbar;"      |
"&brkbar;"      { return ht_BRKBAR;       }

"&#167;"        |
"&sect;"        { return ht_SECT;         }

"&#168;"        |
"&uml;"         |
"&die;"         { return ht_UML;          }

"&#169;"        |
"&copy;"        { return ht_COPY;         }

"&#170;"        |
"&ordf;"        { return ht_ORDF;         }

"&#171;"        |
"&laquo;"       { return ht_LAQUO;        }

"&#172;"        |
"&not;"         { return ht_NOT;          }

"&#173;"        |
"&shy;"         { return ht_SHY;          }

"&#174;"        |
"&reg;"         { return ht_REG;          }

"&#175;"        |
"&macr;"        { return ht_MACR;         }

"&#176;"        |
"&deg;"         { return ht_DEG;          }

"&#177;"        |
"&plusmn;"      { return ht_PLUSMN;       }

"&#178;"        |
"&sup2;"        { return ht_SUP2;         }

"&#179;"        |
"&sup3;"        { return ht_SUP3;         }

"&#180;"        |
"&acute;"       { return ht_ACUTE;        }

"&#181;"        |
"&micro;"       { return ht_MICRO;        }

"&#182;"        |
"&para;"        { return ht_PARA;         }

"&#183;"        |
"&middot;"      { return ht_MIDDOT;       }

"&#184;"        |
"&cedil;"       { return ht_CEDIL;        }

"&#185;"        |
"&sup1;"        { return ht_SUP1;         }

"&#186;"        |
"&ordm;"        { return ht_ORDM;         }

"&#187;"        |
"&raquo;"       { return ht_RAQUO;        }

"&#188;"        |
"&frac14;"      { return ht_FRAC14;       }

"&#189;"        |
"&half;"        |
"&frac12;"      { return ht_FRAC12;       }

"&#190;"        |
"&frac34;"      { return ht_FRAC34;       }

"&#191;"        |
"&iquest;"      { return ht_IQUEST;       }


"&#192;"        |
"&#224;"        |
"&agrave;"      { return ht_AGRAVE;       }

"&#193;"        |
"&#225;"        |
"&aacute;"      { return ht_AACUTE;       }

"&#194;"        |
"&#226;"        |
"&acirc;"       { return ht_ACIRC;        }

"&#195;"        |
"&#227;"        |
"&atilde;"      { return ht_ATILDE;       }

"&#196;"        |
"&#228;"        |
"&auml;"        { return ht_AUML;         }

"&#197;"        |
"&#229;"        |
"&aring;"       { return ht_ARING;        }

"&#198;"        |
"&#230;"        |
"&aeling;"      { return ht_AELING;       }

"&#199;"        |
"&#231;"        |
"&ccedil;"      { return ht_CCEDIL;       }

"&#200;"        |
"&#232;"        |
"&egrave;"      { return ht_EGRAVE;       }

"&#201;"        |
"&#233;"        |
"&eacute;"      { return ht_EACUTE;       }

"&#202;"        |
"&#234;"        |
"&ecirc;"       { return ht_ECIRC;        }

"&#203;"        |
"&#235;"        |
"&euml;"        { return ht_EUML;         }

"&#204;"        |
"&#236;"        |
"&igrave;"      { return ht_IGRAVE;       }

"&#205;"        |
"&#237;"        |
"&iacute;"      { return ht_IACUTE;       }

"&#206;"        |
"&#238;"        |
"&icirc;"       { return ht_ICIRC;        }

"&#207;"        |
"&#239;"        |
"&iuml;"        { return ht_IUML;         }

"&#208;"        |
"&#240;"        |
"&eth;"         { return ht_ETH;          }

"&#209;"        |
"&#241;"        |
"&ntilde;"      { return ht_NTILDE;       }

"&#210;"        |
"&#242;"        |
"&ograve;"      { return ht_OGRAVE;       }

"&#211;"        |
"&#243;"        |
"&oacute;"      { return ht_OACUTE;       }

"&#212;"        |
"&#244;"        |
"&ocirc;"       { return ht_OCIRC;        }

"&#213;"        |
"&#245;"        |
"&otilde;"      { return ht_OTILDE;       }

"&#214;"        |
"&#246;"        |
"&ouml;"        { return ht_OUML;         }

"&#215;"        |
"&times;"       { return ht_TIMES;        }

"&#216;"        |
"&#248;"        |
"&oslash;"      { return ht_OSLASH;       }

"&#217;"        |
"&#249;"        |
"&ugrave;"      { return ht_UGRAVE;       }

"&#218;"        |
"&#250;"        |
"&uacute;"      { return ht_UACUTE;       }

"&#219;"        |
"&#251;"        |
"&ucirc;"       { return ht_UCIRC;        }

"&#220;"        |
"&#252;"        |
"&uuml;"        { return ht_UUML;         }

"&#221;"        |
"&#253;"        |
"&yacute;"      { return ht_YACUTE;       }

"&#222;"        |
"&#254;"        |
"&thorn;"       { return ht_THORN;        }

"&#223;"        |
"&szlig;"       { return ht_SZLIG;        }

"&#247;"        |
"&divide;"      { return ht_DIVIDE;       }

"&#255;"        |
"&yuml;"        { return ht_YUML;         }

"&#8482;"       |
"&trade;"       { return ht_TRADE;        }

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
          case ht_DIV:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n");
          break;

          case ht_HR:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "\n<sb>\n");
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

          case ht_STYLE:
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
            unsigned char num;

            if(yytext[1] == '#')
            {
              num = atoi(&yytext[2]);
              if(num != type)
                num = type+32;
            }
            else if(tolower(yytext[1]) == yytext[1])
            {
              // the first char is a lowercase char
              num = type+32;
            }
            else
              num = type;

            tmp[0] = num;
            cmsg = AppendToBuffer(cmsg, &wptr, &len, tmp);
          }
          break;

          case ht_TRADE:
            cmsg = AppendToBuffer(cmsg, &wptr, &len, "(tm)");
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


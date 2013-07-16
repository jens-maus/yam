/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2013 YAM Open Source Team

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
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__AROS__)
#include <sys/types.h>
#endif

#include <netinet/in.h>

#include <clib/alib_protos.h>
#include <clib/macros.h>
#include <datatypes/pictureclass.h>
#include <datatypes/soundclass.h>
#include <devices/printer.h>
#include <dos/doshunks.h>
#include <dos/dostags.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <libraries/asl.h>
#include <libraries/gadtools.h>
#include <libraries/openurl.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/NListtree_mcc.h>
#include <mui/NListview_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <workbench/startup.h>
#include <proto/codesets.h>
#include <proto/datatypes.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/iffparse.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/keymap.h>
#include <proto/layers.h>
#include <proto/locale.h>
#include <proto/muimaster.h>
#include <proto/openurl.h>
#include <proto/timer.h>
#include <proto/utility.h>
#include <proto/wb.h>
#include <proto/xpkmaster.h>

#if defined(__amigaos4__)
#include <proto/application.h>
#endif

#include "extrasrc.h"

#include "SDI_hook.h"
#include "SDI_stdarg.h"
#include "timeval.h"

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_error.h"
#include "YAM_find.h"
#include "YAM_folderconfig.h"
#include "YAM_global.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_read.h"
#include "YAM_utilities.h"

#include "mui/ClassesExtra.h"
#include "mui/Base64Dataspace.h"
#include "mui/InfoBar.h"
#include "mui/MainFolderListtree.h"
#include "mui/MainMailList.h"
#include "mui/MainMailListGroup.h"
#include "mui/ReadMailGroup.h"
#include "mui/YAMApplication.h"

#include "AppIcon.h"
#include "Busy.h"
#include "FileInfo.h"
#include "FolderList.h"
#include "Locale.h"
#include "MailList.h"
#include "MailServers.h"
#include "MethodStack.h"
#include "MimeTypes.h"
#include "MUIObjects.h"
#include "ParseEmail.h"
#include "Requesters.h"
#include "Threads.h"

#include "Debug.h"

#define CRYPTBYTE 164

#if !defined(SDTA_Cycles)
#define SDTA_Cycles           (SDTA_Dummy + 6)
#endif

struct UniversalClassData
{
  struct UniversalGUIData { Object *WI; } GUI;
};

int BusyLevel = 0;

/***************************************************************************
 Utilities
***************************************************************************/

struct ZombieFile
{
  struct MinNode node;
  char *fileName;
};

#if !defined(__amigaos4__) || (INCLUDE_VERSION < 50)
struct PathNode
{
  BPTR pn_Next;
  BPTR pn_Lock;
};
#endif

struct LaunchCommandData
{
  const char *cmd;
  enum OutputDefType outdef;
};

/// FreePathList
// free a previously copied list of PathNodes
#if !defined(__amigaos4__)
static void FreePathList(struct PathNode *pn)
{
  ENTER();

  while(pn != NULL)
  {
    struct PathNode *next = BADDR(pn->pn_Next);

    UnLock(pn->pn_Lock);
    FreeVec(pn);

    pn = next;
  }

  LEAVE();
}
#endif

///
/// CopyPathList
// create a copied list of PathNodes
// based on what OS4 does to copy a list of PathNodes
#if !defined(__amigaos4__)
static struct PathNode *CopyPathList(const struct PathNode *src)
{
  struct PathNode *this;
  struct PathNode *first = NULL;
  struct PathNode *prev = NULL;
  APTR oldwin;

  ENTER();

  // prevent any error requesters from popping up during the copy process,
  // the DupLock() call might cause this
  oldwin = SetProcWindow((APTR)-1);

  for(; src; src = BADDR(src->pn_Next))
  {
    if((this = AllocVec(sizeof(*this), MEMF_SHARED)) != NULL)
    {
      // duplicate the path node, a failed DupLock() causes *NO* harm
      this->pn_Lock = DupLock(src->pn_Lock);
      this->pn_Next = 0;

      if(first == NULL)
        first = this;

      // link the new node into the list
      if(prev != NULL)
        prev->pn_Next = MKBADDR(this);

      prev = this;
    }
    else
    {
      FreePathList(first);
      first = NULL;
      break;
    }
  }

  // restore the old error requester behaviour
  SetProcWindow(oldwin);

  RETURN(first);
  return first;
}
#endif

///
/// ObtainSearchPath
// This returns a duplicated search path (preferable the workbench
// search path) usable for NP_Path of SystemTagList().
static BPTR ObtainSearchPath(void)
{
  BPTR path = ZERO;

  ENTER();

  if(WorkbenchBase != NULL && LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 44, 0) == TRUE)
  {
    if(WorkbenchControl(NULL, WBCTRLA_DuplicateSearchPath, &path, TAG_DONE) == FALSE)
    {
      // eliminate any possibly modified path pointer again if the call failed
      path = ZERO;
    }
  }

  #if !defined(__amigaos4__)
  // if we couldn't obtain a duplicate copy of the workbench search
  // path here it is very likely that we are running on a system with
  // workbench.library < 44 or on MorphOS with an old workbench.lib.
  if(path == ZERO)
  {
    struct Task *me = FindTask(NULL);
    struct CommandLineInterface *cli = NULL;

    // make sure we are a process, otherwise we are not allowed to call Cli()
    if(me->tc_Node.ln_Type == NT_PROCESS)
    {
      cli = Cli();
    }

    // check if we obtained a CLI structure yet
    if(cli == NULL)
    {
      // no own CLI structure, most probably we were started from Workbench
      struct Process *wb;

      // find the Workbench process
      Forbid();
      wb = (struct Process *)FindTask("Workbench");
      Permit();
      if(wb != NULL)
      {
        // use the Workbench's CLI structure
        cli = BADDR(wb->pr_CLI);
      }
    }

    if(cli != NULL)
      path = MKBADDR(CopyPathList(BADDR(cli->cli_CommandDir)));
  }
  #endif

  RETURN(path);
  return path;
}

///
/// ReleaseSearchPath
// Free the memory returned by ObtainSearchPath
static void ReleaseSearchPath(BPTR path)
{
  ENTER();

  if(path != ZERO)
  {
    if(WorkbenchBase != NULL && LIB_VERSION_IS_AT_LEAST(WorkbenchBase, 44, 0) == TRUE)
    {
      if(WorkbenchControl(NULL, WBCTRLA_FreeSearchPath, path, TAG_DONE) != FALSE)
        path = ZERO;
    }

    #if !defined(__amigaos4__)
    if(path != ZERO)
      FreePathList(BADDR(path));
    #endif
  }

  LEAVE();
}
///

/*** String related ***/
/// MatchNoCase
//  Case insensitive pattern matching
BOOL MatchNoCase(const char *string, const char *match)
{
  BOOL result = FALSE;
  LONG patternlen = strlen(match)*2+2; // ParsePattern() needs at least 2*source+2 bytes buffer
  char *pattern;

  ENTER();

  if((pattern = malloc((size_t)patternlen)) != NULL)
  {
    if(ParsePatternNoCase((STRPTR)match, pattern, patternlen) != -1)
      result = MatchPatternNoCase((STRPTR)pattern, (STRPTR)string);

    free(pattern);
  }

  RETURN(result);
  return result;
}
///
/// StripUnderscore
//  Removes underscore from button labels
char *StripUnderscore(const char *label)
{
  static char newlabel[SIZE_DEFAULT];
  char *p;

  ENTER();

  for(p=newlabel; *label; label++)
  {
    if(*label != '_')
      *p++ = *label;
  }
  *p = '\0';

  RETURN(newlabel);
  return newlabel;
}

///
/// TrimStart
//  Strips leading spaces
char *TrimStart(const char *s)
{
  ENTER();

  while(*s != '\0' && isspace(*s))
    s++;

  RETURN(s);
  return (char *)s;
}

///
/// TrimEnd
//  Removes trailing spaces
char *TrimEnd(char *s)
{
  char *e = s+strlen(s)-1;

  ENTER();

  while(e >= s && isspace(*e))
    *e-- = '\0';

  RETURN(s);
  return s;
}

///
/// Trim
// Removes leading and trailing spaces
char *Trim(char *s)
{
  ENTER();

  if(s != NULL)
  {
    s = TrimStart(s);
    s = TrimEnd(s);
  }

  RETURN(s);
  return s;
}

///
/// MyStrChr
//  Searches for a character in string, ignoring text in quotes
char *MyStrChr(const char *s, const char c)
{
  char *result = NULL;
  BOOL nested = FALSE;

  ENTER();

  while(*s != '\0')
  {
    if(*s == '"')
      nested = !nested;
    else if(*s == c && !nested)
    {
      result = (char *)s;
      break;
    }

    s++;
  }

  RETURN(result);
  return result;
}

///
/// Decrypt
//  Decrypts passwords
char *Decrypt(const char *source)
{
  static char buffer[SIZE_PASSWORD+2];
  char *write = &buffer[SIZE_PASSWORD];

  ENTER();

  *write-- = '\0';
  while(*source != '\0')
  {
    *write-- = ((char)atoi(source)) ^ CRYPTBYTE;
    source += 4;
  }
  write++;

  RETURN(write);
  return write;
}

///
/// Encrypt
//  Encrypts passwords
char *Encrypt(const char *source)
{
  static char buffer[4*SIZE_PASSWORD+2];
  char *read = (char *)(source+strlen(source)-1);

  ENTER();

  *buffer = '\0';
  while(read >= source)
  {
    unsigned char c = (*read--) ^ CRYPTBYTE;
    int p = strlen(buffer);

    snprintf(&buffer[p], sizeof(buffer)-p, "%03d ", c);
  }

  RETURN(buffer);
  return buffer;
}

///
/// UnquoteString
//  Removes quotes from a string, skipping "escaped" quotes
char *UnquoteString(const char *s, BOOL new)
{
  char *ans;
  char *o = (char *)s;

  ENTER();

  // check if the string contains any quote chars
  // at all
  if(strchr(s, '"') == NULL)
  {
    if(new == TRUE)
      o = strdup(s);

    RETURN(o);
    return o;
  }

  // now start unquoting the string
  if((ans = malloc(strlen(s)+1)))
  {
    char *t = ans;

    while(*s)
    {
      if(*s == '\\')
        *t++ = *++s;
      else if(*s == '"')
        ; // nothing
      else
        *t++ = *s;

      ++s;
    }

    *t = '\0';

    // in case the user wants to have the copy lets do it
    if(new == TRUE)
    {
      RETURN(ans);
      return ans;
    }

    // otherwise overwrite the original string array
    strcpy(o, ans);

    free(ans);
  }

  RETURN(o);
  return o;
}

///
/// ReplaceInvalidChars
// replaces invalid filename characters by "_"
void ReplaceInvalidChars(char *name)
{
  unsigned char *ptr = (unsigned char *)name;
  unsigned char c;

  ENTER();

  while((c = *ptr) != '\0')
  {
    // see if we have to replace certain unallowed characters
    // by a '_'
    if((c <= 0x20) || (c > 0x80 && c < 0xa0) || strchr(":/#?*()[]<>|%'\"", c) != NULL)
      *ptr = '_';

    ptr++;
  }

  LEAVE();
}

///

/*** File related ***/
/// GetLine
// gets a NUL terminated line of a file handle and strips any
// trailing CR or LF
ssize_t GetLine(char **buffer, size_t *size, FILE *fh)
{
  ssize_t len;

  ENTER();

  if((len = getline(buffer, size, fh)) > 0)
  {
    char *buf = *buffer;

    // strip possible CR or LF characters at the end of the line
    if(buf[len-1] == '\n')
    {
      // search for possible CR or LF characters and adjust the length information accordingly
      if(len > 1 && buf[len-2] == '\r')
        len -= 2;
      else
        len -= 1;

      buf[len] = '\0';
    }
  }
  #if defined(DEBUG)
  else
  {
    if(feof(fh) == 0 || ferror(fh) != 0)
    {
      // something bad happened, so we return NULL to signal abortion
      W(DBF_MAIL, "getline() in GetLine() returned NULL and feof()=%ld || ferror()=%ld", feof(fh), ferror(fh));
    }
  }
  #endif

  RETURN(len);
  return len;
}

///
/// RenameFile
//  Renames a file and restores the protection bits
BOOL RenameFile(const char *oldname, const char *newname)
{
  BOOL result = FALSE;

  ENTER();

  if(Rename(oldname, newname))
  {
    // the rename succeeded, now change the file permissions

    #if defined(__amigaos4__)
    struct ExamineData *ed;

    if((ed = ExamineObjectTags(EX_StringName, newname, TAG_DONE)) != NULL)
    {
      ULONG prots = ed->Protection;

      FreeDosObject(DOS_EXAMINEDATA, ed);
      if(SetProtection(newname, prots & ~EXDF_ARCHIVE))
        result = TRUE;
    }
    #else
    struct FileInfoBlock *fib;

    if((fib = AllocDosObject(DOS_FIB,NULL)) != NULL)
    {
      BPTR lock;

      if((lock = Lock(newname, ACCESS_READ)))
      {
        if(Examine(lock, fib))
        {
          UnLock(lock);
          if(SetProtection(newname, fib->fib_Protection & ~FIBF_ARCHIVE))
            result = TRUE;
        }
        else
          UnLock(lock);
      }
      FreeDosObject(DOS_FIB, fib);
    }
    #endif
  }

  RETURN(result);
  return result;
}
///
/// CopyFile
//  Copies a file
BOOL CopyFile(const char *dest, FILE *destfh, const char *sour, FILE *sourfh)
{
  BOOL success = FALSE;
  char *buf;

  ENTER();

  // allocate a dynamic buffer instead of placing it on the stack
  if((buf = malloc(SIZE_FILEBUF)) != NULL)
  {
    if(sour != NULL && (sourfh = fopen(sour, "r")) != NULL)
      setvbuf(sourfh, NULL, _IOFBF, SIZE_FILEBUF);

    if(sourfh != NULL && dest != NULL && (destfh = fopen(dest, "w")) != NULL)
      setvbuf(destfh, NULL, _IOFBF, SIZE_FILEBUF);

    if(sourfh != NULL && destfh != NULL)
    {
      int len;

      while((len = fread(buf, 1, SIZE_FILEBUF, sourfh)) > 0)
      {
        if(fwrite(buf, 1, len, destfh) != (size_t)len)
          break;
      }

      // if we arrived here because this was the eof of the sourcefile
      // and non of the two filehandles are in error state we can set
      // success to TRUE.
      if(feof(sourfh) && !ferror(sourfh) && !ferror(destfh))
        success = TRUE;
    }

    if(dest != NULL && destfh != NULL)
      fclose(destfh);

    if(sour != NULL && sourfh != NULL)
      fclose(sourfh);

    free(buf);
  }

  RETURN(success);
  return success;
}

///
/// MoveFile
//  Moves a file (also from one partition to another)
BOOL MoveFile(const char *oldfile, const char *newfile)
{
  BOOL success = TRUE;

  ENTER();

  // we first try to rename the file with a standard Rename()
  // and if it doesn't work we do a raw copy
  if(RenameFile(oldfile, newfile) == FALSE)
  {
    // a normal rename didn't work, so lets copy the file
    if(CopyFile(newfile, 0, oldfile, 0) == FALSE ||
       DeleteFile(oldfile) == 0)
    {
      // also a copy didn't work, so lets return an error
      success = FALSE;
    }
  }

  RETURN(success);
  return success;
}

///
/// AppendFile
//  Append a file to another one
BOOL AppendFile(const char *dst, const char *src)
{
  BOOL success = FALSE;
  char *buf;

  ENTER();

  // allocate a dynamic buffer instead of placing it on the stack
  if((buf = malloc(SIZE_FILEBUF)) != NULL)
  {
    FILE *dstFH;
    FILE *srcFH;

    dstFH = fopen(dst, "a");
    srcFH = fopen(src, "r");

    if(dstFH != NULL && srcFH != NULL)
    {
      int len;

      setvbuf(dstFH, NULL, _IOFBF, SIZE_FILEBUF);
      setvbuf(srcFH, NULL, _IOFBF, SIZE_FILEBUF);

      while((len = fread(buf, 1, SIZE_FILEBUF, srcFH)) > 0)
      {
        if(fwrite(buf, 1, len, dstFH) != (size_t)len)
          break;
      }

      // if we arrived here because this was the eof of the sourcefile
      // and non of the two filehandles are in error state we can set
      // success to TRUE.
      if(feof(srcFH) && !ferror(srcFH) && !ferror(dstFH))
        success = TRUE;
    }

    if(dstFH != NULL)
      fclose(dstFH);

    if(srcFH != NULL)
      fclose(srcFH);

    free(buf);
  }

  RETURN(success);
  return success;
}

///
/// ConvertCRLF
//  Converts line breaks from LF to CRLF or vice versa
BOOL ConvertCRLF(char *in, char *out, BOOL to)
{
  BOOL success = FALSE;
  FILE *infh;

  ENTER();

  if((infh = fopen(in, "r")))
  {
    FILE *outfh;

    setvbuf(infh, NULL, _IOFBF, SIZE_FILEBUF);

    if((outfh = fopen(out, "w")))
    {
      char *buf = NULL;
      size_t size = 0;

      setvbuf(outfh, NULL, _IOFBF, SIZE_FILEBUF);

      while(GetLine(&buf, &size, infh) >= 0)
        fprintf(outfh, "%s%s\n", buf, to?"\r":"");

      success = TRUE;
      fclose(outfh);

      free(buf);
    }

    fclose(infh);
  }

  RETURN(success);
  return success;
}
///
/// Word_Length
//  returns the string length of the next word
static int Word_Length(const char *buf)
{
  unsigned char c;
  int len = 0;

  while((c = *buf))
  {
    if(isspace(c))
    {
      if(c == '\n' || c == '\r')
        return 0;

      len++;
    }
    else break;

    buf++;
  }


  while((c = *buf))
  {
    if(isspace(c) || c == '\0')
      break;

    len++;
    buf++;
  }

  return len;
}
///
/// Quoting_Chars
//  Determines and copies all quoting characters ">" to the buffer "buf"
//  out of the supplied text. It also returns the number of skipable
//  characters since the start of line like "JL>"
static int Quoting_Chars(char *buf, const int len, const char *text, int *post_spaces)
{
  unsigned char c;
  BOOL quote_found = FALSE;
  int i=0;
  int last_bracket = 0;
  int skip_chars = 0;
  int pre_spaces = 0;

  ENTER();

  *post_spaces = 0;

  while((c = *text++) && i < len-1)
  {
    if(c == '>')
    {
      last_bracket = i+1;

      quote_found = TRUE;
    }
    else
    {
      // if the current char is a newline or not between A-Z or a-z then we
      // can break out immediately as these chars are not allowed
      if(c == '\n' || (c != ' ' && (c < 'A' || c > 'z' || (c > 'Z' && c < 'a'))))
        break;

      if(c == ' ')
      {
        if(quote_found == TRUE)
        {
          // if we end up here we can count the number of spaces
          // after the quoting characters
          (*post_spaces)++;
        }
        else if(skip_chars == 0)
        {
          pre_spaces++;
        }
        else
          break;
      }
      else if(quote_found == TRUE || skip_chars > 2)
      {
        break;
      }
      else
        skip_chars++;
    }

    buf[i++] = c;
  }

  buf[last_bracket] = '\0';

  // if we got some spaces before anything else,
  // we put the amount of found pre_spaces in the post_spaces variable
  // instead
  if(pre_spaces > 0)
    (*post_spaces) = pre_spaces;

  // return the number of skipped chars before
  // any quote char was found.
  RETURN(last_bracket ? skip_chars+pre_spaces : 0);
  return last_bracket ? skip_chars+pre_spaces : 0;
}

///
/// QuoteText
//  Main mail text quotation function. It takes the source string "src" and
//  analyzes it concerning existing quoting characters. Depending on this
//  information it adds new quoting marks "prefix" to the start of each line
//  taking care of a correct word wrap if the line gets longs than "line_max".
//  All output is directly written to the already opened filehandle "out".
void QuoteText(FILE *out, const char *src, const int len, const int line_max)
{
  ENTER();

  // make sure the output file handle is valid
  if(out != NULL)
  {
    char temp_buf[128];
    int temp_len;
    BOOL newline = TRUE;
    BOOL wrapped = FALSE; // needed to implement automatic wordwrap while quoting
    BOOL lastwasspace = FALSE;
    int skip_on_next_newline = 0;
    int line_len = 0;
    int skip_chars;
    int post_spaces = 0;
    int srclen = len;

    // find out how many quoting chars the next line has
    skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
    temp_len = strlen(temp_buf) - skip_chars;
    src += skip_chars;
    srclen -= skip_chars;

    while(srclen > 0)
    {
      char c = *src;

      // break out if we received a NUL byte, because this
      // should really never happen
      if(c == '\0')
        break;

      // skip any LF
      if(c == '\r')
      {
        src++;
        srclen--;
        continue;
      }

      // on a CR (newline)
      if(c == '\n')
      {
        src++;
        srclen--;

        // find out how many quoting chars the next line has
        skip_chars = Quoting_Chars(temp_buf, sizeof(temp_buf), src, &post_spaces);
        src += (skip_chars + skip_on_next_newline);
        srclen -= (skip_chars + skip_on_next_newline);
        skip_on_next_newline = 0;

        if(temp_len == ((int)strlen(temp_buf)-skip_chars) && wrapped)
        {
          // the text has been wrapped previously and the quoting chars
          // are the same like the previous line, so the following text
          // probably belongs to the same paragraph

          srclen -= temp_len; // skip the quoting chars
          src += temp_len;
          wrapped = FALSE;

          // check whether the next char will be a newline or not, because
          // a newline indicates a new empty line, so there is no need to
          // cat something together at all
          if(*src != '\n')
          {
            // add a space to if this was the first quoting
            if(lastwasspace == FALSE && (temp_len == 0 || *src != ' '))
            {
              fputc(' ', out);
              line_len++;
              lastwasspace = TRUE;
            }

            continue;
          }
        }

        temp_len = strlen(temp_buf)-skip_chars;
        wrapped = FALSE;

        // check whether this line would be zero or not and if so we
        // have to care about if the user wants to also quote empty lines
        if(line_len == 0 && C->QuoteEmptyLines)
          fputs(C->QuoteChar, out);

        // then put a newline in our file
        fputc('\n', out);
        newline = TRUE;
        lastwasspace = FALSE;

        line_len = 0;

        continue;
      }

      if(newline)
      {
        if(c == '>')
        {
          fputs(C->QuoteChar, out);
          line_len += strlen(C->QuoteChar);
        }
        else
        {
          fputs(C->QuoteChar, out);
          fputc(' ', out);
          line_len += strlen(C->QuoteChar)+1;
        }

        newline = FALSE;
      }

      // we check whether this char was a whitespace
      // or not and if so we set the lastwasspace flag and we also check if
      // we are near the end of the line so that we have to initiate a word wrap
      if((lastwasspace = isspace(c)) && line_len + Word_Length(src) >= line_max)
      {
        char *indent;

        src++;
        srclen--;

        // output a newline to start a new line
        fputc('\n', out);

        // reset line_len
        line_len = 0;

        fputs(C->QuoteChar, out);
        line_len += strlen(C->QuoteChar);

        if(strlen(temp_buf))
        {
          fputs(temp_buf+skip_chars, out);
          line_len += strlen(temp_buf)-skip_chars;
          lastwasspace = FALSE;
        }
        else
        {
          fputc(' ', out);
          line_len++;
          lastwasspace = TRUE;
        }

        // lets check the indention of the next line
        if((indent = strchr(src, '\n')) && ++indent != '\0')
        {
          int pre_spaces;

          Quoting_Chars(temp_buf, sizeof(temp_buf), indent, &pre_spaces);

          skip_on_next_newline = pre_spaces;

          if(pre_spaces == 0)
            pre_spaces += post_spaces;

          while(pre_spaces--)
          {
            fputc(' ', out);
            line_len++;
            lastwasspace = TRUE;
          }
        }

        wrapped = TRUE; // indicates that a word has been wrapped manually
        continue;
      }

      fputc(c, out);
      line_len++;

      src++;
      srclen--;
    }

    // check whether we finished the quoting with
    // a newline or otherwise the followed signature won't fit correctly
    if(newline == FALSE)
      fputc('\n', out);
  }

  LEAVE();
}
///
/// SimpleWordWrap
//  Reformats a file to a new line length
void SimpleWordWrap(const char *filename, int wrapsize)
{
  BPTR fh;

  ENTER();

  if((fh = Open((STRPTR)filename, MODE_OLDFILE)))
  {
    char ch;
    int p = 0;
    int lsp = -1;
    int sol = 0;

    while(Read(fh, &ch, 1) == 1)
    {
      if(p - sol > wrapsize && lsp >= 0)
      {
        ch = '\n';
        ChangeFilePosition(fh, (LONG)lsp - p - 1, OFFSET_CURRENT);
        p = lsp;
        Write(fh, &ch, 1);
      }

      if(isspace(ch))
        lsp = p;
      if(ch == '\n')
      {
        sol = p + 1;
        lsp = -1;
      }
      p++;
    }
    Close(fh);
  }

  LEAVE();
}
///
/// ReqFile
//  Puts up a file requester
struct FileReqCache *ReqFile(enum ReqFileType num, Object *win,
                             const char *title, int mode,
                             const char *drawer, const char *file)
{
  // the following arrays depend on the ReqFileType enumeration
  static const char *const acceptPattern[ASL_MAX] =
  {
    "#?.addressbook#?",                    // ASL_ABOOK
    "#?.config#?",                         // ASL_CONFIG
    NULL,                                  // ASL_DETACH
    "~(#?.info)",                          // ASL_ATTACH
    "#?.(yam|rexx|rx)",                    // ASL_REXX
    "#?.(gif|jpg|jpeg|png|iff|ilbm)",      // ASL_PHOTO
    "#?.((mbx|mbox|eml|dbx|msg)|#?,#?)",   // ASL_IMPORT
    "#?.(mbx|mbox|eml)",                   // ASL_EXPORT
    NULL,                                  // ASL_FOLDER
    "#?.(ldif|ldi)",                       // ASL_ABOOK_LIF
    "#?.csv",                              // ASL_ABOOK_CSV
    "#?.(tab|txt)",                        // ASL_ABOOK_TAB
    "#?.xml",                              // ASL_ABOOK_XML
    "#?",                                  // ASL_GENERIC
    "#?",                                  // ASL_UPDATE
    "#?.sfd",                              // ASL_FILTER
  };

  struct FileReqCache *result = NULL;

  ENTER();

  if(num < ASL_MAX)
  {
    struct FileRequester *fileReq;

    // allocate the required data for our file requester
    if((fileReq = MUI_AllocAslRequest(ASL_FileRequest, NULL)) != NULL)
    {
      const char *pattern = acceptPattern[num];
      struct FileReqCache *frc = G->FileReqCache[num];
      BOOL reqResult;
      BOOL usefrc = frc->used;

      // do the actual file request now
      reqResult = MUI_AslRequestTags(fileReq,
                                     ASLFR_Window,         xget(win, MUIA_Window_Window),
                                     ASLFR_TitleText,      title,
                                     ASLFR_PositiveText,   hasSaveModeFlag(mode) ? tr(MSG_UT_Save) : tr(MSG_UT_Load),
                                     ASLFR_DoSaveMode,     hasSaveModeFlag(mode),
                                     ASLFR_DoMultiSelect,  hasMultiSelectFlag(mode),
                                     ASLFR_DrawersOnly,    hasDrawersOnlyFlag(mode),
                                     ASLFR_RejectIcons,    FALSE,
                                     ASLFR_DoPatterns,     pattern != NULL,
                                     ASLFR_InitialFile,    file,
                                     ASLFR_InitialDrawer,  usefrc ? frc->drawer : drawer,
                                     ASLFR_InitialPattern, pattern ? pattern : "#?",
                                     usefrc ? ASLFR_InitialLeftEdge : TAG_IGNORE, frc->left_edge,
                                     usefrc ? ASLFR_InitialTopEdge  : TAG_IGNORE, frc->top_edge,
                                     usefrc ? ASLFR_InitialWidth    : TAG_IGNORE, frc->width,
                                     usefrc ? ASLFR_InitialHeight   : TAG_IGNORE, frc->height,
                                     TAG_DONE);

      // copy the data out of our fileRequester into our
      // own cached structure we return to the user
      if(reqResult != 0)
      {
        // free previous resources
        FreeFileReqCache(frc);

        // copy all necessary data from the ASL filerequester structure
        // to our cache
        frc->file     = strdup(fileReq->fr_File);
        frc->drawer   = strdup(fileReq->fr_Drawer);
        frc->pattern  = strdup(fileReq->fr_Pattern);
        frc->numArgs  = fileReq->fr_NumArgs;
        frc->left_edge= fileReq->fr_LeftEdge;
        frc->top_edge = fileReq->fr_TopEdge;
        frc->width    = fileReq->fr_Width;
        frc->height   = fileReq->fr_Height;
        frc->used     = TRUE;

        // now we copy the optional arglist
        if(fileReq->fr_NumArgs > 0)
        {
          if((frc->argList = calloc(sizeof(char*), fileReq->fr_NumArgs)) != NULL)
          {
            int i;

            for(i=0; i < fileReq->fr_NumArgs; i++)
              frc->argList[i] = strdup(fileReq->fr_ArgList[i].wa_Name);
          }
        }
        else
          frc->argList = NULL;

        // everything worked out fine, so lets return
        // our globally cached filereq structure.
        result = frc;
      }
      else if(IoErr() != 0)
      {
        // and IoErr() != 0 signals that something
        // serious happend and that we have to inform the
        // user
        ER_NewError(tr(MSG_ER_CANTOPENASL));

        // beep the display as well
        DisplayBeep(NULL);
      }


      // free the ASL request structure again.
      MUI_FreeAslRequest(fileReq);
    }
    else
      ER_NewError(tr(MSG_ErrorAslStruct));
  }

  RETURN(result);
  return result;
}
///
/// FreeFileReqCache
// free all structures inside a filerequest cache structure
void FreeFileReqCache(struct FileReqCache *frc)
{
  ENTER();

  if(frc != NULL)
  {
    free(frc->file);
    free(frc->drawer);
    free(frc->pattern);

    if(frc->numArgs > 0)
    {
      int j;

      for(j=0; j < frc->numArgs; j++)
        free(frc->argList[j]);

      free(frc->argList);
    }
  }

  LEAVE();
}
///
/// AddZombieFile
//  add an orphaned file to the zombie file list
void AddZombieFile(const char *fileName)
{
  ENTER();

  // make sure the file exists, otherwise we don't need to do anything
  if(FileExists(fileName) == TRUE)
  {
    struct ZombieFile *zombie;

    if((zombie = (struct ZombieFile *)AllocSysObjectTags(ASOT_NODE,
      ASONODE_Size, sizeof(*zombie),
      ASONODE_Min, TRUE,
      TAG_DONE)) != NULL)
    {
      if((zombie->fileName = strdup(fileName)) != NULL)
      {
        AddTail((struct List *)&G->zombieFileList, (struct Node *)&zombie->node);

        D(DBF_UTIL, "added file '%s' to the zombie list", fileName);

        // trigger the retry mechanism in 5 minutes
        RestartTimer(TIMER_DELETEZOMBIEFILES, 5 * 60, 0);
      }
      else
        FreeSysObject(ASOT_NODE, zombie);
    }
  }

  LEAVE();
}
///
/// DeleteZombieFiles
//  try to delete all files in the list of zombie files
BOOL DeleteZombieFiles(BOOL force)
{
  BOOL listCleared = TRUE;
  struct Node *node;

  ENTER();

  // always get the first node of the list, it will be Remove()'d later
  // if the corresponding file could be closed.
  while((node = GetHead((struct List *)&G->zombieFileList)) != NULL)
  {
    struct ZombieFile *zombie = (struct ZombieFile *)node;

    D(DBF_UTIL, "trying to delete zombie file '%s'", zombie->fileName);

    // try again to delete the file, if it still exists
    if(force == FALSE && FileExists(zombie->fileName) == TRUE && DeleteFile(zombie->fileName) == 0)
    {
      // deleting failed again, but we are allowed to retry
      listCleared = FALSE;

      W(DBF_UTIL, "zombie file '%s' cannot be deleted, leaving in list", zombie->fileName);

      // break out because we couldn't close a file
      break;
    }
    else
    {
      // remove and free this node
      Remove(node);
      free(zombie->fileName);
      FreeSysObject(ASOT_NODE, zombie);
    }
  }

  RETURN(listCleared);
  return listCleared;
}
///
/// OpenTempFile
//  Creates or opens a temporary file
struct TempFile *OpenTempFile(const char *mode)
{
  struct TempFile *tf;

  ENTER();

  if((tf = calloc(1, sizeof(*tf))) != NULL)
  {
    // the tempfile MUST be SIZE_MFILE long because we
    // also use this tempfile routine for showing temporary mails which
    // conform to SIZE_MFILE
    char buf[SIZE_MFILE];

    // now format our temporary filename according to our Application data
    // this format tries to make the temporary filename kinda unique.
    snprintf(buf, sizeof(buf), "YAMt%08x.tmp", (unsigned int)GetUniqueID());

    // now add the temporary path to the filename
    AddPath(tf->Filename, C->TempDir, buf, sizeof(tf->Filename));

    if(mode != NULL)
    {
      if((tf->FP = fopen(tf->Filename, mode)) == NULL)
      {
        E(DBF_UTIL, "couldn't create temporary file: '%s'", tf->Filename);

        // on error we free everything
        free(tf);
        tf = NULL;
      }
      else
        setvbuf(tf->FP, NULL, _IOFBF, SIZE_FILEBUF);
    }
  }

  RETURN(tf);
  return tf;
}
///
/// CloseTempFile
//  Closes a temporary file
void CloseTempFile(struct TempFile *tf)
{
  ENTER();

  if(tf != NULL)
  {
    if(tf->FP != NULL)
      fclose(tf->FP);

    D(DBF_UTIL, "DeleteTempFile: '%s'", tf->Filename);
    if(DeleteFile(tf->Filename) == 0)
      AddZombieFile(tf->Filename);

    free(tf);
  }

  LEAVE();
}
///
/// DumpClipboard
//  Exports contents of clipboard unit 0 to a file
#define ID_FTXT   MAKE_ID('F','T','X','T')
#define ID_CHRS   MAKE_ID('C','H','R','S')
BOOL DumpClipboard(FILE *out)
{
  BOOL success = FALSE;
  struct IFFHandle *iff;

  ENTER();

  if((iff = AllocIFF()) != NULL)
  {
    if((iff->iff_Stream = (ULONG)OpenClipboard(PRIMARY_CLIP)) != 0)
    {
      InitIFFasClip(iff);
      if(OpenIFF(iff, IFFF_READ) == 0)
      {
        if(StopChunk(iff, ID_FTXT, ID_CHRS) == 0)
        {
          while(TRUE)
          {
            struct ContextNode *cn;
            long error;
            long rlen;
            UBYTE readbuf[SIZE_DEFAULT];

            error = ParseIFF(iff, IFFPARSE_SCAN);
            if(error == IFFERR_EOC)
              continue;
            else if(error != 0)
              break;

            if((cn = CurrentChunk(iff)) != NULL)
            {
              if(cn->cn_Type == ID_FTXT && cn->cn_ID == ID_CHRS)
              {
                success = TRUE;
                while((rlen = ReadChunkBytes(iff, readbuf, SIZE_DEFAULT)) > 0)
                  fwrite(readbuf, 1, (size_t)rlen, out);
              }
            }
          }
        }
        CloseIFF(iff);
      }
      CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
    }
    FreeIFF(iff);
  }

  RETURN(success);
  return success;
}
///
/// IsFolderDir
//  Checks if a directory is used as a mail folder
static BOOL IsFolderDir(const char *dir)
{
  BOOL result = FALSE;
  char *filename;
  int i;

  ENTER();

  filename = (char *)FilePart(dir);

  for(i = 0; i < FT_NUM; i++)
  {
    if(FolderName[i] != NULL && stricmp(filename, FolderName[i]) == 0)
    {
      result = TRUE;
      break;
    }
  }

  if(result == FALSE)
  {
    char fname[SIZE_PATHFILE];

    result = (FileExists(AddPath(fname, dir, ".fconfig", sizeof(fname))) ||
              FileExists(AddPath(fname, dir, ".index", sizeof(fname))));
  }

  RETURN(result);
  return result;
}
///
/// AllFolderLoaded
//  Checks if all folder index are correctly loaded
BOOL AllFolderLoaded(void)
{
  BOOL allLoaded = TRUE;

  ENTER();

  LockFolderListShared(G->folders);

  if(IsFolderListEmpty(G->folders) == FALSE)
  {
    struct FolderNode *fnode;

    ForEachFolderNode(G->folders, fnode)
    {
      if(fnode->folder->LoadedMode != LM_VALID && !isGroupFolder(fnode->folder))
      {
        allLoaded = FALSE;
        break;
      }
    }
  }
  else
    allLoaded = FALSE;

  UnlockFolderList(G->folders);

  RETURN(allLoaded);
  return allLoaded;
}
///
/// DeleteMailDir (rec)
//  Recursively deletes a mail directory
BOOL DeleteMailDir(const char *dir, BOOL isroot)
{
  BOOL result = TRUE;
  APTR context;

  ENTER();

  if((context = ObtainDirContextTags(EX_StringName, (ULONG)dir,
                                     EX_DataFields, EXF_TYPE|EXF_NAME,
                                     EX_DoCurrentDir, TRUE,
                                     TAG_DONE)) != NULL)
  {
    struct ExamineData *ed;

    while((ed = ExamineDir(context)) != NULL && result == TRUE)
    {
      BOOL isdir = EXD_IS_DIRECTORY(ed);
      char *filename = (char *)ed->Name;
      char fname[SIZE_PATHFILE];

      AddPath(fname, dir, filename, sizeof(fname));

      if(isroot == TRUE)
      {
        if(isdir == TRUE)
        {
          if(IsFolderDir(fname) == TRUE)
            result = DeleteMailDir(fname, FALSE);
        }
        else
        {
          // check for our own special files starting with a "."
          // note that we might have several .uidl#? files
          if(stricmp(filename, ".config")            == 0 ||
             stricmp(filename, ".glossary")          == 0 ||
             stricmp(filename, ".addressbook")       == 0 ||
             stricmp(filename, ".emailcache")        == 0 ||
             stricmp(filename, ".folders")           == 0 ||
             stricmp(filename, ".spamdata")          == 0 ||
             strnicmp(filename, ".signature", 10)    == 0 ||
             strnicmp(filename, ".altsignature", 13) == 0 ||
             strnicmp(filename, ".uidl", 6)          == 0)
          {
            if(DeleteFile(fname) == 0)
            {
              W(DBF_FOLDER, "failed to delete file '%s' (error %ld)", fname, IoErr());
              result = FALSE;
            }
          }
        }
      }
      else if(isdir == FALSE)
      {
        if(isValidMailFile(filename) == TRUE  ||
           stricmp(filename, ".fconfig") == 0 ||
           stricmp(filename, ".fimage") == 0  ||
           stricmp(filename, ".index") == 0)
        {
          if(DeleteFile(fname) == 0)
          {
            W(DBF_FOLDER, "failed to delete file '%s' (error %ld)", fname, IoErr());
            result = FALSE;
          }
        }
      }
    }

    // check for an error by ExamineDir() only if nothing else failed
    if(result == TRUE)
    {
      LONG error;

      error = IoErr();
      if(error != 0 && error != ERROR_NO_MORE_ENTRIES)
        E(DBF_FOLDER, "ExamineDir() failed, error %ld", error);
    }

    ReleaseDirContext(context);

    if(result == TRUE && DeleteFile(dir) == 0)
      result = FALSE;
  }
  else
    result = FALSE;

  RETURN(result);
  return result;
}
///
/// FileToBuffer
//  Reads a complete file into memory
char *FileToBuffer(const char *file)
{
  char *text = NULL;
  LONG size;

  ENTER();

  if(ObtainFileInfo(file, FI_SIZE, &size) == TRUE &&
     size > 0 && (text = malloc((size+1)*sizeof(char))) != NULL)
  {
    FILE *fh;

    text[size] = '\0'; // NUL-terminate the string

    if((fh = fopen(file, "r")) != NULL)
    {
      if(fread(text, sizeof(char), size, fh) != (size_t)size)
      {
        free(text);
        text = NULL;
      }

      fclose(fh);
    }
    else
    {
      free(text);
      text = NULL;
    }
  }

  RETURN(text);
  return text;
}
///
/// FileCount
// returns the total number of files matching a pattern that are in a directory
// or -1 if an error occurred.
LONG FileCount(const char *directory, const char *pattern)
{
  APTR context;
  char *parsedPattern;
  LONG parsedPatternSize;
  LONG result = 0;

  ENTER();

  if(pattern == NULL)
    pattern = "#?";

  parsedPatternSize = strlen(pattern) * 2 + 2;
  if((parsedPattern = malloc(parsedPatternSize)) != NULL)
  {
    ParsePatternNoCase(pattern, parsedPattern, parsedPatternSize);

    #if defined(__amigaos4__)
    // dos.library before 52.17 has a small bug and needs a hook for the matching process
    if((context = ObtainDirContextTags(EX_StringName, (ULONG)directory,
                                       EX_DataFields, EXF_TYPE|EXF_NAME,
                                       EX_MatchString, (ULONG)parsedPattern,
                                       EX_MatchFunc, LIB_VERSION_IS_AT_LEAST(DOSBase, 52, 17) ? NULL : &ExamineDirMatchHook,
                                       TAG_DONE)) != NULL)
    #else
    if((context = ObtainDirContextTags(EX_StringName, (ULONG)directory,
                                       EX_DataFields, EXF_TYPE|EXF_NAME,
                                       EX_MatchString, (ULONG)parsedPattern,
                                       TAG_DONE)) != NULL)
    #endif
    {
      struct ExamineData *ed;
      LONG error;

      while((ed = ExamineDir(context)) != NULL)
      {
        // count the number of files
        if(EXD_IS_FILE(ed))
          result++;
      }

      error = IoErr();
      if(error != 0 && error != ERROR_NO_MORE_ENTRIES)
      {
        E(DBF_ALWAYS, "FileCount() failed, error %ld", error);
        result = -1;
      }

      ReleaseDirContext(context);
    }

    free(parsedPattern);
  }
  else
    result = -1;

  RETURN(result);
  return result;
}
///
/// AddPath
// Function that is a wrapper to AddPart so that we can add the
// specified path 'add' to an existing/non-existant 'src' which
// is then stored in dst of max size 'size'.
char *AddPath(char *dst, const char *src, const char *add, const size_t size)
{
  ENTER();

  strlcpy(dst, src, size);
  if(AddPart(dst, add, size) == FALSE)
  {
    E(DBF_ALWAYS, "AddPath()/AddPart() buffer overflow detected!");
    dst = NULL;
  }

  RETURN(dst);
  return dst;
}
///

/*** Mail related ***/
/// CreateFilename
//  Prepends mail directory to a file name
char *CreateFilename(const char * const file, char *fullPath, const size_t fullPathSize)
{
  ENTER();

  AddPath(fullPath, G->MA_MailDir, file, fullPathSize);

  RETURN(fullPath);
  return fullPath;
}
///
/// CreateDirectory
//  Makes a directory
BOOL CreateDirectory(const char *dir)
{
  BOOL success = FALSE;

  ENTER();

  // check if dir isn't empty
  if(dir[0] != '\0')
  {
    enum FType ft;

    if(ObtainFileInfo(dir, FI_TYPE, &ft) == TRUE)
    {
      if(ft == FIT_DRAWER)
        success = TRUE;
      else if(ft == FIT_NONEXIST)
      {
        char buf[SIZE_PATHFILE];
        BPTR fl;
        size_t len = strlen(dir)-1;

        // check for trailing slashes
        if(dir[len] == '/')
        {
          // we make a copy of dir first because
          // we are not allowed to modify it
          strlcpy(buf, dir, sizeof(buf));

          // remove all trailing slashes
          while(len > 0 && buf[len] == '/')
            buf[len--] = '\0';

          // set dir to our buffer
          dir = buf;
        }

        // use utility/CreateDir() to create the
        // directory
        if((fl = CreateDir(dir)))
        {
          UnLock(fl);
          success = TRUE;
        }
      }
    }

    if(G->MA != NULL && success == FALSE)
      ER_NewError(tr(MSG_ER_CantCreateDir), dir);
  }

  RETURN(success);
  return success;
}
///
/// GetMailFile
//  Returns path of a message file
void GetMailFile(char *string, const size_t stringSize, const struct Folder *folder, const struct Mail *mail)
{
  ENTER();

  if(folder == NULL && mail != NULL)
    folder = mail->Folder;

  if(folder == NULL || folder == (struct Folder *)-1)
    AddPath(string, C->TempDir, mail->MailFile, stringSize);
  else
    AddPath(string, folder->Fullpath, mail->MailFile, stringSize);

  LEAVE();
}
///
/// BuildAddress
// Creates "Real Name <E-mail>" string from a given address and name
// according to the rules defined in RFC2822, which in fact takes care
// of quoatation of the real name as well as escaping some special characters
char *BuildAddress(char *buffer, size_t buflen, const char *address, const char *name)
{
  ENTER();

  // check that buffer is != NULL
  if(buffer != NULL)
  {
    D(DBF_MAIL, "build full address from address '%s' and name '%s'", SafeStr(address), SafeStr(name));

    // check if a real name is given at all
    // or not
    if(name != NULL && name[0] != '\0')
    {
      // search for some chars which, when present,
      // require us to put the real name into quotations
      // see RFC2822 (section 3.2.1) - However, we don't
      // include "." here because of the comments in section 4.1
      // of RFC2822, which states that "." is a valid char in a
      // "phrase" token and don't need to be escaped.
      if(strpbrk(name, "()<>[]:;@\\,\"") != NULL) // check for 'specials' excluding '.'
      {
        char quotedstr[SIZE_REALNAME];
        const char *s = name;
        char *d;

        // we now have to search for a '"' quotation char or for
        // an escape '\' char which we need to escape via '\', if
        // it exists in the specified real name
        if((d = strpbrk(s, "\"\\")) != NULL)
        {
          quotedstr[0] = '\0';

          // now iterate through s and escape any char
          // we require to escape
          do
          {
            size_t qlen;

            // copy everything until the first escapable char
            if(d-s > 0)
            {
              qlen = strlen(quotedstr)+(d-s)+1;
              strlcat(quotedstr, s, (qlen <= sizeof(quotedstr)) ? qlen : sizeof(quotedstr));
            }

            // add the escape char + the char we want to escape
            qlen = strlen(quotedstr);
            if(qlen+2 < sizeof(quotedstr))
            {
              quotedstr[qlen]   = '\\';
              quotedstr[qlen+1] = *d;
              quotedstr[qlen+2] = '\0';
            }

            // prepare the next iteration
            s = d+1;
          }
          while((d = strpbrk(s, "\"\\")) != NULL);

          // check if there is anything left
          // to attach to quotedstr
          if(s < (name+strlen(name)))
            strlcat(quotedstr, s, sizeof(quotedstr));
        }
        else
        {
          // otherwise simply output the real name
          strlcpy(quotedstr, name, sizeof(quotedstr));
        }

        // add the addr-spec string
        snprintf(buffer, buflen, "\"%s\" <%s>", quotedstr, address);
      }
      else
        snprintf(buffer, buflen, "%s <%s>", name, address);
    }
    else
      strlcpy(buffer, address, buflen);

    D(DBF_MAIL, "built full address '%s'", buffer);
  }
  else
    E(DBF_UTIL, "BuildAddress buffer==NULL error!");

  RETURN(buffer);
  return buffer;
}

///
/// ExtractAddress
//  Extracts e-mail address and real name according to RFC2822 (section 3.4)
void ExtractAddress(const char *line, struct Person *pe)
{
  char *save;

  ENTER();

  pe->Address[0] = '\0';
  pe->RealName[0] = '\0';

  SHOWSTRING(DBF_MIME, line);

  // create a temp copy of our source
  // string so that we don't have to alter it.
  if((save = strdup(line)) != NULL)
  {
    char *p = save;
    char *start;
    char *end;
    char *address = NULL;
    char *realname = NULL;

    // skip leading whitespaces
    p = TrimStart(p);

    // we first try to extract the email address part of the line in case
    // the email is in < > brackets (see RFC2822)
    //
    // something like: "Realname <mail@address.net>"
    if((start = MyStrChr(p, '<')) != NULL && (end = MyStrChr(start, '>')) != NULL)
    {
      *start = '\0';
      *end = '\0';

      // now we have successfully extract the
      // email address between start and end
      address = ++start;

      // per definition of RFC 2822, the realname (display name)
      // should be in front of the email, but we will extract it later on
      realname = p;
    }

    // if we haven't found the email yet
    // we might have search for something like "mail@address.net (Realname)"
    if(address == NULL)
    {
      // extract the mail address first
      for(start=end=p; *end != '\0' && !isspace(*end) && *end != ',' && *end != '('; end++);

      // now we should have the email address
      if(end > start)
      {
        char *s = NULL;

        if(*end != '\0')
        {
          *end = '\0';
          s = end+1;
        }

        // we have the mail address
        address = start;

        // we should have the email address now so we go and extract
        // the realname encapsulated in ( )
        if(s != NULL && (s = strchr(s, '(')) != NULL)
        {
          start = ++s;

          // now we search for the last closing )
          end = strrchr(start, ')');
          if(end != NULL)
            *end = '\0';
          else
            end = start+strlen(start);

          realname = start;
        }
      }
    }

    // we successfully found an email adress, so we go
    // and copy it into our person's structure.
    if(address != NULL)
      strlcpy(pe->Address, Trim(address), sizeof(pe->Address));

    // in case we found a descriptive realname we go and
    // parse it for quoted and escaped passages.
    if(realname != NULL)
    {
      unsigned int i;
      BOOL quoted = FALSE;

      // as a realname may be quoted '"' and also may contain escaped sequences
      // like '\"', we extract the realname more carefully here.
      p = Trim(realname);

      // check if the realname is quoted or not
      if(*p == '"')
      {
        quoted = TRUE;
        p++;
      }

      for(i=0; *p != '\0' && i < sizeof(pe->RealName); i++, p++)
      {
        if(quoted)
        {
          if(*p == '\\')
            p++;
          else if(*p == '"' && strlen(p) == 1)
            break;
        }

        if(*p != '\0')
          pe->RealName[i] = *p;
        else
          break;
      }

      // make sure we properly NUL-terminate
      // the string
      if(i < sizeof(pe->RealName))
        pe->RealName[i] = '\0';
      else
        pe->RealName[sizeof(pe->RealName)-1] = '\0';
    }

    D(DBF_MIME, "addr: '%s'", pe->Address);
    D(DBF_MIME, "real: '%s'", pe->RealName);

    free(save);
  }

  LEAVE();
}
///
/// DescribeCT
//  Returns description of a content type
const char *DescribeCT(const char *ct)
{
  const char *ret = ct;

  ENTER();

  if(ct == NULL)
    ret = tr(MSG_CTunknown);
  else
  {
    struct Node *curNode;

    // first we search through the users' own MIME type list
    IterateList(&C->mimeTypeList, curNode)
    {
      struct MimeTypeNode *mt = (struct MimeTypeNode *)curNode;
      char *type;

      // find the type right after the '/' delimiter
      if((type = strchr(mt->ContentType, '/')) != NULL)
        type++;
      else
        type = (char *)"";

      // don't allow the catch-all and empty types
      if(type[0] != '*' && type[0] != '?' && type[0] != '#' && type[0] != '\0')
      {
        if(stricmp(ct, mt->ContentType) == 0 && mt->Description[0] != '\0')
        {
          ret = mt->Description;
          break;
        }
      }

    }

    // if we still haven't identified the description
    // we go and search through the internal list
    if(ret == ct)
    {
      unsigned int i;

      for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
      {
        if(stricmp(ct, IntMimeTypeArray[i].ContentType) == 0)
        {
          ret = tr(IntMimeTypeArray[i].Description);
          break;
        }
      }
    }
  }

  RETURN(ret);
  return ret;
}
///
/// GetDateStamp
//  Get number of seconds since 1/1-1978
time_t GetDateStamp(void)
{
  struct DateStamp ds;
  time_t seconds;

  ENTER();

  // get the actual time
  DateStamp(&ds);
  seconds = ds.ds_Days * 24 * 60 * 60 +
            ds.ds_Minute * 60 +
            ds.ds_Tick / TICKS_PER_SECOND;

  RETURN(seconds);
  return seconds;
}
///
///
enum DST GetDSTinfo(int year, int month, int day)
{
  struct tm a = {};
  struct tm b;
  a.tm_isdst = -1;
  a.tm_mday = day;
  a.tm_mon  = month - 1;
  a.tm_year = year - 1900;
  b = a;
  a.tm_hour = 23;
  a.tm_min = 59;
  mktime(&a);
  mktime(&b);

  if      (a.tm_isdst  && b.tm_isdst)  return DST_ON;
  else if (!a.tm_isdst && !b.tm_isdst) return DST_OFF;
  else if (a.tm_isdst  && !b.tm_isdst) return DST_OFFTOON;
  else                                 return DST_ONTOOFF;
}
///
/// DateStampUTC
//  gets the current system time in UTC
void DateStampUTC(struct DateStamp *ds)
{
  ENTER();

  DateStamp(ds);
  DateStampTZConvert(ds, TZC_UTC);

  LEAVE();
}
///
/// GetSysTimeUTC
//  gets the actual system time in UTC
void GetSysTimeUTC(struct TimeVal *tv)
{
  ENTER();

  GetSysTime(TIMEVAL(tv));
  TimeValTZConvert(tv, TZC_UTC);

  LEAVE();
}
///
/// TimeValTZConvert
//  converts a supplied timeval depending on the TZConvert flag to be converted
//  to/from UTC
void TimeValTZConvert(struct TimeVal *tv, enum TZConvert tzc)
{
  ENTER();

  if(tzc == TZC_LOCAL)
    tv->Seconds += (C->TimeZone + C->DaylightSaving * 60) * 60;
  else if(tzc == TZC_UTC)
    tv->Seconds -= (C->TimeZone + C->DaylightSaving * 60) * 60;

  LEAVE();
}
///
/// DateStampTZConvert
//  converts a supplied DateStamp depending on the TZConvert flag to be converted
//  to/from UTC
void DateStampTZConvert(struct DateStamp *ds, enum TZConvert tzc)
{
  ENTER();

  // convert the DateStamp from local -> UTC or visa-versa
  if(tzc == TZC_LOCAL)
    ds->ds_Minute += (C->TimeZone + C->DaylightSaving * 60);
  else if(tzc == TZC_UTC)
    ds->ds_Minute -= (C->TimeZone + C->DaylightSaving * 60);

  // we need to check the datestamp variable that it is still in it's borders
  // after the UTC correction
  while(ds->ds_Minute < 0)
  {
    ds->ds_Minute += 1440;
    ds->ds_Days--;
  }
  while(ds->ds_Minute >= 1440)
  {
    ds->ds_Minute -= 1440;
    ds->ds_Days++;
  }

  LEAVE();
}
///
/// TimeVal2DateStamp
//  converts a struct TimeVal to a struct DateStamp
void TimeVal2DateStamp(const struct TimeVal *tv, struct DateStamp *ds, enum TZConvert tzc)
{
  LONG seconds;

  ENTER();

  seconds = tv->Seconds + (tv->Microseconds / 1000000);

  ds->ds_Days   = seconds / 86400;       // calculate the days since 1.1.1978
  ds->ds_Minute = (seconds % 86400) / 60;
  ds->ds_Tick   = (tv->Seconds % 60) * TICKS_PER_SECOND + (tv->Microseconds / 20000);

  // if we want to convert from/to UTC we need to do this now
  if(tzc != TZC_NONE)
    DateStampTZConvert(ds, tzc);

  LEAVE();
}
///
/// DateStamp2TimeVal
//  converts a struct DateStamp to a struct TimeVal
void DateStamp2TimeVal(const struct DateStamp *ds, struct TimeVal *tv, enum TZConvert tzc)
{
  ENTER();

  // check if the ptrs are set or not.
  if(ds != NULL && tv != NULL)
  {
    // creates wrong timevals from DateStamps with year >= 2114 ...
    tv->Seconds = (ds->ds_Days * 24 * 60 + ds->ds_Minute) * 60 + ds->ds_Tick / TICKS_PER_SECOND;
    tv->Microseconds = (ds->ds_Tick % TICKS_PER_SECOND) * 1000000 / TICKS_PER_SECOND;

    // if we want to convert from/to UTC we need to do this now
    if(tzc != TZC_NONE)
      TimeValTZConvert(tv, tzc);
  }

  LEAVE();
}
///
/// TimeVal2String
//  Converts a timeval structure to a string with using DateStamp2String after a convert
BOOL TimeVal2String(char *dst, int dstlen, const struct TimeVal *tv, enum DateStampType mode, enum TZConvert tzc)
{
  BOOL result;
  struct DateStamp ds;

  // convert the timeval into a datestamp
  ENTER();

  TimeVal2DateStamp(tv, &ds, TZC_NONE);

  // then call the DateStamp2String() function to get the real string
  result = DateStamp2String(dst, dstlen, &ds, mode, tzc);

  RETURN(result);
  return result;
}
///
/// DateStamp2String
//  Converts a datestamp to a string. The caller have to make sure that the destination has
//  at least 64 characters space.
BOOL DateStamp2String(char *dst, int dstlen, struct DateStamp *date, enum DateStampType mode, enum TZConvert tzc)
{
  char datestr[64], timestr[64], daystr[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.
  struct DateTime dt;
  struct DateStamp dsnow;
  BOOL success = FALSE;

  ENTER();

  // if this argument is not set we get the actual time
  if(date == NULL)
    date = DateStamp(&dsnow);

  if(mode == DSS_TIME || mode == DSS_SHORTTIME)
    date->ds_Days = 0;

  // now we fill the DateTime structure with the data for our request.
  dt.dat_Stamp   = *date;
  dt.dat_Format  = (mode == DSS_USDATETIME || mode == DSS_UNIXDATE) ? FORMAT_USA : FORMAT_DEF;
  dt.dat_Flags   = (mode == DSS_RELDATETIME || mode == DSS_RELDATEBEAT) ? DTF_SUBST : 0;
  dt.dat_StrDate = datestr;
  dt.dat_StrTime = timestr;
  dt.dat_StrDay  = daystr;

  // now we check whether we have to convert the datestamp to a specific TZ or not
  if(tzc != TZC_NONE)
    DateStampTZConvert(&dt.dat_Stamp, tzc);

  // lets terminate the strings as OS 3.1 is strange
  datestr[31] = '\0';
  timestr[31] = '\0';
  daystr[31]  = '\0';

  // lets convert the DateStamp now to a string
  if(DateToStr(&dt) != DOSFALSE)
  {
    switch(mode)
    {
      case DSS_UNIXDATE:
      {
        int y = atoi(&datestr[6]);

        // this is a Y2K patch
        // if less than 8035 days have passed since 1.1.1978 then we are in the 20th century
        if(date->ds_Days < 8035)
          y += 1900;
        else
          y += 2000;

        snprintf(dst, dstlen, "%s %s %02d %s %d\n", wdays[dt.dat_Stamp.ds_Days%7], months[atoi(datestr)-1], atoi(&datestr[3]), timestr, y);
      }
      break;

      case DSS_DATETIME:
      case DSS_USDATETIME:
      case DSS_RELDATETIME:
      {
        snprintf(dst, dstlen, "%s %s", datestr, timestr);
      }
      break;

      case DSS_WEEKDAY:
      {
        strlcpy(dst, daystr, dstlen);
      }
      break;

      case DSS_DATE:
      {
        strlcpy(dst, datestr, dstlen);
      }
      break;

      case DSS_TIME:
      {
        strlcpy(dst, timestr, dstlen);
      }
      break;

      case DSS_SHORTTIME:
      {
        // find the last ':' and strip the string there so
        // that it does not include any seconds
        char *first = strchr(timestr, ':');
        char *last = strrchr(timestr, ':');

        if(first != NULL && last != NULL && last != first)
          *last = '\0';

        strlcpy(dst, timestr, dstlen);
      }
      break;

      case DSS_BEAT:
      case DSS_DATEBEAT:
      case DSS_RELDATEBEAT:
      {
        // calculate the beat time
        unsigned int beat = (((date->ds_Minute-C->TimeZone+(C->DaylightSaving?0:60)+1440)%1440)*1000)/1440;

        if(mode == DSS_DATEBEAT || mode == DSS_RELDATEBEAT)
          snprintf(dst, dstlen, "%s @%03d", datestr, beat);
        else
          snprintf(dst, dstlen, "@%03d", beat);
      }
      break;
    }

    // in any case we succeeded
    success = TRUE;

    D(DBF_UTIL, "converted DateStamp %ld/%ld/%ld to string '%s'", date->ds_Days, date->ds_Minute, date->ds_Tick, dst);
  }
  else
  {
    W(DBF_UTIL, "couldn't convert DateStamp %ld/%ld/%ld to string", date->ds_Days, date->ds_Minute, date->ds_Tick);
    // clear the destination string on failure
    dst[0] = '\0';
  }

  RETURN(success);
  return success;
}
///
/// DateStamp2RFCString
BOOL DateStamp2RFCString(char *dst, const int dstlen, const struct DateStamp *date, const int timeZone, const BOOL convert)
{
  struct DateStamp datestamp;
  struct ClockData cd;
  time_t seconds;
  int convertedTimeZone = (timeZone/60)*100 + (timeZone%60);

  ENTER();

  // if date == NULL we get the current time/date
  if(date == NULL)
    DateStamp(&datestamp);
  else
    memcpy(&datestamp, date, sizeof(struct DateStamp));

  // if the user wants to convert the datestamp we have to make sure we
  // substract/add the timeZone
  if(convert && timeZone != 0)
  {
    datestamp.ds_Minute += timeZone;

    // we need to check the datestamp variable that it is still in it's borders
    // after adjustment
    while(datestamp.ds_Minute < 0)     { datestamp.ds_Minute += 1440; datestamp.ds_Days--; }
    while(datestamp.ds_Minute >= 1440) { datestamp.ds_Minute -= 1440; datestamp.ds_Days++; }
  }

  // lets form the seconds now for the Amiga2Date function
  seconds = (datestamp.ds_Days*24*60*60 + datestamp.ds_Minute*60 + datestamp.ds_Tick/TICKS_PER_SECOND);

  // use utility's Amiga2Date for calculating the correct date/time
  Amiga2Date(seconds, &cd);

  // use snprintf to format the RFC2822 conforming datetime string.
  snprintf(dst, dstlen, "%s, %02d %s %d %02d:%02d:%02d %+05d", wdays[cd.wday],
                                                               cd.mday,
                                                               months[cd.month-1],
                                                               cd.year,
                                                               cd.hour,
                                                               cd.min,
                                                               cd.sec,
                                                               convertedTimeZone);

  RETURN(TRUE);
  return TRUE;
}
///
/// DateStamp2Long
// Converts a datestamp to a pseudo numeric value
long DateStamp2Long(struct DateStamp *date)
{
  char *s;
  char datestr[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.
  struct DateStamp dsnow;
  struct DateTime dt;
  int y;
  long res = 0;

  ENTER();

  if(date == NULL)
    date = DateStamp(&dsnow);

  memset(&dt, 0, sizeof(struct DateTime));
  dt.dat_Stamp   = *date;
  dt.dat_Format  = FORMAT_USA;
  dt.dat_StrDate = datestr;

  if(DateToStr(&dt) != DOSFALSE)
  {
    s = Trim(datestr);

    // get the year
    y = atoi(&s[6]);

    // this is a Y2K patch
    // if less then 8035 days has passed since 1.1.1978 then we are in the 20th century
    if(date->ds_Days < 8035) y += 1900;
    else y += 2000;

    res = (100*atoi(&s[3])+atoi(s))*10000+y;
  }
  else
  {
    W(DBF_UTIL, "couldn't convert DateStamp %ld/%ld/%ld to LONG", date->ds_Days, date->ds_Minute, date->ds_Tick);
  }

  RETURN(res);
  return res;
}
///
/// String2DateStamp
//  Tries to converts a string into a datestamp via StrToDate()
BOOL String2DateStamp(struct DateStamp *dst, const char *string, enum DateStampType mode, enum TZConvert tzc)
{
  char datestr[64], timestr[64]; // we don't use LEN_DATSTRING as OS3.1 anyway ignores it.
  char *datestrPtr = datestr;
  char *timestrPtr = timestr;
  BOOL result = FALSE;

  ENTER();

  // depending on the DateStampType we have to try to split the string differently
  // into the separate datestr/timestr combo
  switch(mode)
  {
    case DSS_UNIXDATE:
    {
      char *p;

      // we walk from the front to the back and skip the week
      // day name
      if((p = strchr(string, ' ')) != NULL)
      {
        int month;

        // extract the month
        for(month=0; month < 12; month++)
        {
          if(strnicmp(string, months[month], 3) == 0)
            break;
        }

        if(month >= 12)
          break;

        // extract the day
        if((p = strchr(p, ' ')) != NULL)
        {
          int day = atoi(p+1);

          if(day < 1 || day > 31)
            break;

          // extract the timestring
          if((p = strchr(p, ' ')) != NULL)
          {
            strlcpy(timestr, p+1, MIN((ULONG)8, sizeof(timestr)));

            // extract the year
            if((p = strchr(p, ' ')) != NULL)
            {
              int year = atoi(p+1);

              if(year < 1970 || year > 2070)
                break;

              // now we can compose our datestr
              snprintf(datestr, sizeof(datestr), "%02d-%02d-%02d", month+1, day, year%100);

              result = TRUE;
            }
          }
        }
      }
    }
    break;

    case DSS_DATETIME:
    case DSS_USDATETIME:
    case DSS_RELDATETIME:
    {
      char *p;

      // copy the datestring
      if((p = strchr(string, ' ')) != NULL)
      {
        strlcpy(datestr, string, MIN(sizeof(datestr), (unsigned int)(p - string + 1)));
        strlcpy(timestr, p + 1, sizeof(timestr));

        result = TRUE;
      }
    }
    break;

    case DSS_WEEKDAY:
    case DSS_DATE:
    {
      strlcpy(datestr, string, sizeof(datestr));
      // ignore the time part
      timestrPtr = NULL;
      result = TRUE;
    }
    break;

    case DSS_TIME:
    case DSS_SHORTTIME:
    {
      strlcpy(timestr, string, sizeof(timestr));
      // ignore the date part
      datestrPtr = NULL;
      result = TRUE;
    }
    break;

    case DSS_BEAT:
    case DSS_DATEBEAT:
    case DSS_RELDATEBEAT:
      // not supported yet.
    break;
  }

  // we continue only if everything until now is fine.
  if(result == TRUE)
  {
    struct DateTime dt;

    // now we fill the DateTime structure with the data for our request.
    dt.dat_Format  = (mode == DSS_USDATETIME || mode == DSS_UNIXDATE) ? FORMAT_USA : FORMAT_DEF;
    dt.dat_Flags   = 0; // perhaps later we can add Weekday substitution
    dt.dat_StrDate = datestrPtr;
    dt.dat_StrTime = timestrPtr;
    dt.dat_StrDay  = NULL;

    // convert the string to a dateStamp
    if(StrToDate(&dt) != DOSFALSE)
    {
      // now we check whether we have to convert the datestamp to a specific TZ or not
      if(tzc != TZC_NONE)
        DateStampTZConvert(&dt.dat_Stamp, tzc);

      // strip the days if we are interested in the time only
      if(mode == DSS_TIME || mode == DSS_SHORTTIME)
        dt.dat_Stamp.ds_Days = 0;

      // now we do copy the datestamp stuff over the one from our mail
      memcpy(dst, &dt.dat_Stamp, sizeof(*dst));

      D(DBF_UTIL, "converted string '%s' to DateStamp %ld/%ld/%ld", string, dst->ds_Days, dst->ds_Minute, dst->ds_Tick, dst);
    }
    else
      result = FALSE;
  }

  if(result == FALSE)
  {
    W(DBF_UTIL, "couldn't convert string '%s' to struct DateStamp", string);
    // assume 01-Jan-78 00:00:00 on failure
    dst->ds_Days = 0;
    dst->ds_Minute = 0;
    dst->ds_Tick = 0;
  }

  RETURN(result);
  return result;
}

///
/// String2TimeVal
// converts a string to a struct TimeVal, if possible.
BOOL String2TimeVal(struct TimeVal *dst, const char *string, enum DateStampType mode, enum TZConvert tzc)
{
  struct DateStamp ds;
  BOOL result;

  ENTER();

  // we use the String2DateStamp function for conversion
  if((result = String2DateStamp(&ds, string, mode, tzc)) == TRUE)
  {
    // now we just have to convert the DateStamp to a struct TimeVal
    DateStamp2TimeVal(&ds, dst, TZC_NONE);
  }
  else
  {
    W(DBF_UTIL, "couln't convert string '%s' to struct TimeVal", string);
    // assume zero time on failure
    dst->Seconds = 0;
    dst->Microseconds = 0;
  }

  RETURN(result);
  return result;
}

///
/// TZtoMinutes
//  Converts time zone into a numeric offset also using timezone abbreviations
//  Refer to http://www.cise.ufl.edu/~sbeck/DateManip.html#TIMEZONES
int TZtoMinutes(const char *tzone)
{
  /*
    The following timezone names are currently understood (and can be used in parsing dates).
    These are zones defined in RFC 822.
      Universal:  GMT, UT
      US zones :  EST, EDT, CST, CDT, MST, MDT, PST, PDT
      Military :  A to Z (except J)
      Other    :  +HHMM or -HHMM
      ISO 8601 :  +HH:MM, +HH, -HH:MM, -HH

      In addition, the following timezone abbreviations are also accepted. In a few
      cases, the same abbreviation is used for two different timezones (for example,
      NST stands for Newfoundland Standard -0330 and North Sumatra +0630). In these
      cases, only 1 of the two is available. The one preceded by a '#' sign is NOT
      available but is documented here for completeness.
   */

   static const struct
   {
     const char *TZname;
     int   TZcorr;
   } time_zone_table[] =
   {
    { "IDLW",   -1200 }, // International Date Line West
    { "NT",     -1100 }, // Nome
    { "HST",    -1000 }, // Hawaii Standard
    { "CAT",    -1000 }, // Central Alaska
    { "AHST",   -1000 }, // Alaska-Hawaii Standard
    { "AKST",    -900 }, // Alaska Standard
    { "YST",     -900 }, // Yukon Standard
    { "HDT",     -900 }, // Hawaii Daylight
    { "AKDT",    -800 }, // Alaska Daylight
    { "YDT",     -800 }, // Yukon Daylight
    { "PST",     -800 }, // Pacific Standard
    { "PDT",     -700 }, // Pacific Daylight
    { "MST",     -700 }, // Mountain Standard
    { "MDT",     -600 }, // Mountain Daylight
    { "CST",     -600 }, // Central Standard
    { "CDT",     -500 }, // Central Daylight
    { "EST",     -500 }, // Eastern Standard
    { "ACT",     -500 }, // Brazil, Acre
    { "SAT",     -400 }, // Chile
    { "BOT",     -400 }, // Bolivia
    { "EDT",     -400 }, // Eastern Daylight
    { "AST",     -400 }, // Atlantic Standard
    { "AMT",     -400 }, // Brazil, Amazon
    { "ACST",    -400 }, // Brazil, Acre Daylight
//# { "NST",     -330 }, // Newfoundland Standard       nst=North Sumatra    +0630
    { "NFT",     -330 }, // Newfoundland
//# { "GST",     -300 }, // Greenland Standard          gst=Guam Standard    +1000
//# { "BST",     -300 }, // Brazil Standard             bst=British Summer   +0100
    { "BRST",    -300 }, // Brazil Standard
    { "BRT",     -300 }, // Brazil Standard
    { "AMST",    -300 }, // Brazil, Amazon Daylight
    { "ADT",     -300 }, // Atlantic Daylight
    { "ART",     -300 }, // Argentina
    { "NDT",     -230 }, // Newfoundland Daylight
    { "AT",      -200 }, // Azores
    { "BRST",    -200 }, // Brazil Daylight (official time)
    { "FNT",     -200 }, // Brazil, Fernando de Noronha
    { "WAT",     -100 }, // West Africa
    { "FNST",    -100 }, // Brazil, Fernando de Noronha Daylight
    { "GMT",     +000 }, // Greenwich Mean
    { "UT",      +000 }, // Universal (Coordinated)
    { "UTC",     +000 }, // Universal (Coordinated)
    { "WET",     +000 }, // Western European
    { "WEST",    +000 }, // Western European Daylight
    { "CET",     +100 }, // Central European
    { "FWT",     +100 }, // French Winter
    { "MET",     +100 }, // Middle European
    { "MEZ",     +100 }, // Middle European
    { "MEWT",    +100 }, // Middle European Winter
    { "SWT",     +100 }, // Swedish Winter
    { "BST",     +100 }, // British Summer              bst=Brazil standard  -0300
    { "GB",      +100 }, // GMT with daylight savings
    { "CEST",    +200 }, // Central European Summer
    { "EET",     +200 }, // Eastern Europe, USSR Zone 1
    { "FST",     +200 }, // French Summer
    { "MEST",    +200 }, // Middle European Summer
    { "MESZ",    +200 }, // Middle European Summer
    { "METDST",  +200 }, // An alias for MEST used by HP-UX
    { "SAST",    +200 }, // South African Standard
    { "SST",     +200 }, // Swedish Summer              sst=South Sumatra    +0700
    { "EEST",    +300 }, // Eastern Europe Summer
    { "BT",      +300 }, // Baghdad, USSR Zone 2
    { "MSK",     +300 }, // Moscow
    { "EAT",     +300 }, // East Africa
    { "IT",      +330 }, // Iran
    { "ZP4",     +400 }, // USSR Zone 3
    { "MSD",     +300 }, // Moscow Daylight
    { "ZP5",     +500 }, // USSR Zone 4
    { "IST",     +530 }, // Indian Standard
    { "ZP6",     +600 }, // USSR Zone 5
    { "NOVST",   +600 }, // Novosibirsk time zone, Russia
    { "NST",     +630 }, // North Sumatra               nst=Newfoundland Std -0330
//# { "SST",     +700 }, // South Sumatra, USSR Zone 6  sst=Swedish Summer   +0200
    { "JAVT",    +700 }, // Java
    { "CCT",     +800 }, // China Coast, USSR Zone 7
    { "AWST",    +800 }, // Australian Western Standard
    { "WST",     +800 }, // West Australian Standard
    { "PHT",     +800 }, // Asia Manila
    { "JST",     +900 }, // Japan Standard, USSR Zone 8
    { "ROK",     +900 }, // Republic of Korea
    { "ACST",    +930 }, // Australian Central Standard
    { "CAST",    +930 }, // Central Australian Standard
    { "AEST",   +1000 }, // Australian Eastern Standard
    { "EAST",   +1000 }, // Eastern Australian Standard
    { "GST",    +1000 }, // Guam Standard, USSR Zone 9  gst=Greenland Std    -0300
    { "ACDT",   +1030 }, // Australian Central Daylight
    { "CADT",   +1030 }, // Central Australian Daylight
    { "AEDT",   +1100 }, // Australian Eastern Daylight
    { "EADT",   +1100 }, // Eastern Australian Daylight
    { "IDLE",   +1200 }, // International Date Line East
    { "NZST",   +1200 }, // New Zealand Standard
    { "NZT",    +1200 }, // New Zealand
    { "NZDT",   +1300 }, // New Zealand Daylight
    { NULL,         0 }  // Others can be added in the future upon request.
   };

   // Military time zone table
   static const struct
   {
      char tzcode;
      int  tzoffset;
   } military_table[] =
   {
    { 'A',  -100 },
    { 'B',  -200 },
    { 'C',  -300 },
    { 'D',  -400 },
    { 'E',  -500 },
    { 'F',  -600 },
    { 'G',  -700 },
    { 'H',  -800 },
    { 'I',  -900 },
    { 'K', -1000 },
    { 'L', -1100 },
    { 'M', -1200 },
    { 'N',  +100 },
    { 'O',  +200 },
    { 'P',  +300 },
    { 'Q',  +400 },
    { 'R',  +500 },
    { 'S',  +600 },
    { 'T',  +700 },
    { 'U',  +800 },
    { 'V',  +900 },
    { 'W', +1000 },
    { 'X', +1100 },
    { 'Y', +1200 },
    { 'Z', +0000 },
    { 0,       0 }
   };

   int tzcorr = -1;

   /*
    * first we check if the timezone string conforms to one of the
    * following standards (RFC 822)
    *
    * 1.Other    :  +HHMM or -HHMM
    * 2.ISO 8601 :  +HH:MM, +HH, -HH:MM, -HH
    * 3.Military :  A to Z (except J)
    *
    * only if none of the 3 above formats match, we take our hughe TZtable
    * and search for the timezone abbreviation
    */

   // check if the timezone definition starts with a + or -
   if(tzone[0] == '+' || tzone[0] == '-')
   {
      tzcorr = atoi(&tzone[1]);

      // check if tzcorr is correct of if it is perhaps a ISO 8601 format
      if(tzcorr != 0 && tzcorr/100 == 0)
      {
        char *c;

        // multiply it by 100 so that we have now a correct format
        tzcorr *= 100;

        // then check if we have a : to seperate HH:MM and add the minutes
        // to tzcorr
        if((c = strchr(tzone, ':')))
          tzcorr += atoi(c);
      }

      // now we have to distingush between + and -
      if(tzone[0] == '-')
        tzcorr = -tzcorr;
   }
   else if(isalpha(tzone[0]))
   {
      int i;

      // if we end up here then the timezone string is
      // probably a abbreviation and we first check if it is a military abbr
      if(isalpha(tzone[1]) == 0) // military need to be 1 char long
      {
        for(i=0; military_table[i].tzcode; i++)
        {
          if(toupper(tzone[0]) == military_table[i].tzcode)
          {
            tzcorr = military_table[i].tzoffset;
            break;
          }
        }
      }
      else
      {
        for(i=0; time_zone_table[i].TZname; i++) // and as a last chance we scan our abbrev table
        {
          if(strnicmp(time_zone_table[i].TZname, tzone, strlen(time_zone_table[i].TZname)) == 0)
          {
            tzcorr = time_zone_table[i].TZcorr;
            D(DBF_UTIL, "TZtoMinutes: found abbreviation '%s' (%ld)", time_zone_table[i].TZname, tzcorr);
            break;
          }
        }

        if(tzcorr == -1)
          W(DBF_UTIL, "TZtoMinutes: abbreviation '%s' NOT found!", tzone);
      }
   }

   if(tzcorr == -1)
     W(DBF_UTIL, "couldn't parse timezone from '%s'", tzone);

   return tzcorr == -1 ? 0 : (tzcorr/100)*60 + (tzcorr%100);
}
///
/// FormatSize
//  Displays large numbers using group separators
void FormatSize(LONG size, char *buf, int buflen, enum SizeFormat forcedPrecision)
{
  const char *dp;
  double dsize;

  ENTER();

  dp = G->Locale ? (const char *)G->Locale->loc_DecimalPoint : ".";
  dsize = (double)size;

  // see if the user wants to force a precision output or if he simply
  // wants to output based on C->SizeFormat (forcedPrecision = SF_AUTO)
  if(forcedPrecision == SF_AUTO)
    forcedPrecision = C->SizeFormat;

  // we check what SizeFormat the user has choosen
  switch(forcedPrecision)
  {
    // the precision modes use sizes as base of 2
    enum { KB = 1024, MB = 1024 * 1024, GB = 1024 * 1024 * 1024 };

    /*
    ** ONE Precision mode
    ** This will result in the following output:
    ** 1.2 GB - 12.3 MB - 123.4 KB - 1234 B
    */
    case SF_1PREC:
    {
      char *p;

      if(size < KB)
        snprintf(buf, buflen, "%d B", (unsigned int)size);
      else if(size < MB)
        snprintf(buf, buflen, "%.1f KB", dsize/KB);
      else if(size < GB)
        snprintf(buf, buflen, "%.1f MB", dsize/MB);
      else
        snprintf(buf, buflen, "%.1f GB", dsize/GB);

      if((p = strchr(buf, '.')) != NULL)
        *p = *dp;
    }
    break;

    /*
    ** TWO Precision mode
    ** This will result in the following output:
    ** 1.23 GB - 12.34 MB - 123.45 KB - 1234 B
    */
    case SF_2PREC:
    {
      char *p;

      if(size < KB)
        snprintf(buf, buflen, "%d B", (unsigned int)size);
      else if(size < MB)
        snprintf(buf, buflen, "%.2f KB", dsize/KB);
      else if(size < GB)
        snprintf(buf, buflen, "%.2f MB", dsize/MB);
      else
        snprintf(buf, buflen, "%.2f GB", dsize/GB);

      if((p = strchr(buf, '.')) != NULL)
        *p = *dp;
    }
    break;

    /*
    ** THREE Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.345 MB - 123.456 KB - 1234 B
    */
    case SF_3PREC:
    {
      char *p;

      if(size < KB)
        snprintf(buf, buflen, "%d B", (unsigned int)size);
      else if(size < MB)
        snprintf(buf, buflen, "%.3f KB", dsize/KB);
      else if(size < GB)
        snprintf(buf, buflen, "%.3f MB", dsize/MB);
      else
        snprintf(buf, buflen, "%.3f GB", dsize/GB);

      if((p = strchr(buf, '.')) != NULL)
        *p = *dp;
    }
    break;

    /*
    ** MIXED Precision mode
    ** This will result in the following output:
    ** 1.234 GB - 12.34 MB - 123.4 KB - 1234 B
    */
    case SF_MIXED:
    {
      char *p;

      if(size < KB)
        snprintf(buf, buflen, "%d B", (unsigned int)size);
      else if(size < MB)
        snprintf(buf, buflen, "%.1f KB", dsize/KB);
      else if(size < GB)
        snprintf(buf, buflen, "%.2f MB", dsize/MB);
      else
        snprintf(buf, buflen, "%.3f GB", dsize/GB);

      if((p = strchr(buf, '.')) != NULL)
        *p = *dp;
    }
    break;

    /*
    ** STANDARD mode
    ** This will result in the following output:
    ** 1,234,567 (bytes)
    */
    case SF_AUTO:
    default:
    {
      const char *gs = G->Locale ? (const char *)G->Locale->loc_GroupSeparator : ",";

      // as we just split the size to another value, we redefine the KB/MB/GB values to base 10 variables
      enum { KiB = 1000, MiB = 1000 * 1000, GiB = 1000 * 1000 * 1000 };

      if(size < KiB)
      {
        snprintf(buf, buflen, "%d B", (unsigned int)size);
      }
      else if(size < MiB)
      {
        ldiv_t k;

        k = ldiv(size, KiB);
        snprintf(buf, buflen, "%d%s%03d B", (unsigned int)k.quot, gs, (unsigned int)k.rem);
      }
      else if(size < GiB)
      {
        ldiv_t m, k;

        m = ldiv(size, MiB);
        k = ldiv(m.rem, KiB);
        snprintf(buf, buflen, "%d%s%03d%s%03d B", (unsigned int)m.quot, gs, (unsigned int)k.quot, gs, (unsigned int)k.rem);
      }
      else
      {
        ldiv_t g, m, k;

        g = ldiv(size, GiB);
        m = ldiv(g.rem, MiB);
        k = ldiv(m.rem, KiB);
        snprintf(buf, buflen, "%d%s%03d%s%03d%s%03d B", (unsigned int)g.quot, gs, (unsigned int)m.quot, gs, (unsigned int)k.quot, gs, (unsigned int)k.rem);
      }
    }
    break;
  }

  LEAVE();
}
///
/// MailExists
//  Checks if a message still exists
BOOL MailExists(const struct Mail *mailptr, struct Folder *folder)
{
  BOOL exists;

  ENTER();

  if(isVirtualMail(mailptr))
  {
    exists = TRUE;
  }
  else
  {
    if(folder == NULL)
      folder = mailptr->Folder;

    LockMailListShared(folder->messages);

    exists = (FindMailByAddress(folder->messages, mailptr) != NULL);

    UnlockMailList(folder->messages);
  }

  RETURN(exists);
  return exists;
}
///
/// DisplayMailList
//  Lists folder contents in the message listview
void DisplayMailList(struct Folder *fo, Object *lv)
{
  struct Mail **array;
  int lastActive;
  struct BusyNode *busy;

  ENTER();

  lastActive = fo->LastActive;

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BusyDisplayingList), "");

  // we convert the mail list of the folder
  // to a temporary array because that allows us
  // to quickly populate the NList object.
  if((array = MailListToMailArray(fo->messages)) != NULL)
  {
    // We do not encapsulate this Clear&Insert with a NList_Quiet because
    // this will speed up the Insert with about 3-4 seconds for ~6000 items
    DoMethod(lv, MUIM_NList_Clear);
    DoMethod(lv, MUIM_NList_Insert, array, fo->Total, MUIV_NList_Insert_Sorted,
                 C->AutoColumnResize ? MUIF_NONE : MUIV_NList_Insert_Flag_Raw);

    free(array);
  }

  BusyEnd(busy);

  // Now we have to recover the LastActive or otherwise it will be -1 later
  fo->LastActive = lastActive;

  LEAVE();
}
///
/// AddMailToFolder
//  Adds a message to a folder
struct Mail *AddMailToFolder(const struct Mail *mail, struct Folder *folder)
{
  struct Mail *new;

  ENTER();

  if((new = CloneMail(mail)) != NULL)
  {
    // add the cloned message to the folder
    LockMailList(folder->messages);
    AddMailToFolderSimple(new, folder);
    UnlockMailList(folder->messages);

    // expire the folder's index as we just added a new message
    MA_ExpireIndex(folder);
  }

  RETURN(new);
  return new;
}

///
/// AddMailToFolderSimple
//  Adds a message to a folder with already locked mail list
void AddMailToFolderSimple(struct Mail *mail, struct Folder *folder)
{
  ENTER();

  mail->Folder = folder;

  // let's add the new message to the folder's message list
  AddNewMailNode(folder->messages, mail);

  // let's summarize the stats
  folder->Total++;
  folder->Size += mail->Size;

  if(hasStatusNew(mail))
    folder->New++;

  if(!hasStatusRead(mail))
    folder->Unread++;

  LEAVE();
}

///
/// RemoveMailFromList
//  Removes a message from a folder
void RemoveMailFromList(struct Mail *mail, const BOOL closeWindows, const BOOL checkConnections)
{
  struct Folder *folder = mail->Folder;
  struct MailNode *mnode;
  struct Node *curNode;

  ENTER();

  // now we remove the mail from main mail
  // listviews in case the folder of it is the
  // currently active one.
  if(folder == GetCurrentFolder())
    DoMethod(G->MA->GUI.PG_MAILLIST, MUIM_MainMailListGroup_RemoveMail, mail);

  // remove the mail from the search window's mail list as well, if the
  // search window exists at all
  if(G->FI != NULL)
    DoMethod(G->FI->GUI.LV_MAILS, MUIM_MainMailList_RemoveMail, mail);

  // lets decrease the folder statistics first
  folder->Total--;
  folder->Size -= mail->Size;

  if(hasStatusNew(mail))
    folder->New--;

  if(!hasStatusRead(mail))
    folder->Unread--;

  LockMailList(folder->messages);

  if((mnode = FindMailByAddress(folder->messages, mail)) != NULL)
  {
    // remove the mail from the folder's mail list
    D(DBF_UTIL, "removing mail with subject '%s' from folder '%s'", mail->Subject, folder->Name);
    RemoveMailNode(folder->messages, mnode);
    DeleteMailNode(mnode);
  }

  UnlockMailList(folder->messages);

  if(checkConnections == TRUE)
  {
    int activeConnections;

    // now check if the mail to be removed has just been downloaded, but not yet filtered
    ObtainSemaphoreShared(G->connectionSemaphore);
    activeConnections = G->activeConnections;
    ReleaseSemaphore(G->connectionSemaphore);

    // we need to check only if there are any active connections
    if(activeConnections > 0)
    {
      struct MailServerNode *msn;
      int i = 0;
      BOOL mailFound = FALSE;

      while(mailFound == FALSE && (msn = GetMailServer(&C->pop3ServerList, i)) != NULL)
      {
        if(hasServerInUse(msn) == TRUE)
        {
          LockMailList(msn->downloadedMails);

          if((mnode = FindMailByAddress(msn->downloadedMails, mail)) != NULL)
          {
            // remove the mail from the list of just downloaded mails,
            // so it will not be filtered anymore when the download
            // process finishes
            D(DBF_UTIL, "removing mail with subject '%s' from download list", mail->Subject);
            RemoveMailNode(msn->downloadedMails, mnode);
            DeleteMailNode(mnode);

            // we found the mail, but it cannot be part of more than one list thus we
            // can exit this loop
            mailFound = TRUE;
          }

          UnlockMailList(msn->downloadedMails);
        }

        i++;
      }
    }
  }

  // then we have to mark the folder index as expired so
  // that it will be saved next time.
  MA_ExpireIndex(folder);

  // Now we check if there is any read window with that very same
  // mail currently open and if so we have to close it.
  curNode = GetHead((struct List *)&G->readMailDataList);
  while(curNode != NULL)
  {
    struct ReadMailData *rmData = (struct ReadMailData *)curNode;
    struct Node *nextNode = GetSucc(curNode);

    if(rmData->mail == mail)
    {
      if(closeWindows == TRUE && rmData->readWindow != NULL)
      {
        // Just ask the window to close itself, this will effectively clear the pointer.
        // We cannot set the attribute directly, because a DoMethod() call is synchronous
        // and then the read window would modify the list we are currently walking through
        // by calling CleanupReadMailData(). Hence we just let the application do the dirty
        // work as soon as it has the possibility to do that, but not before this loop is
        // finished. This works, because the ReadWindow class catches any modification to
        // MUIA_Window_CloseRequest itself. A simple set(win, MUIA_Window_Open, FALSE) would
        // visibly close the window, but it would not invoke the associated hook which gets
        // invoked when you close the window by clicking on the close gadget.
        DoMethod(G->App, MUIM_Application_PushMethod, rmData->readWindow, 3, MUIM_Set, MUIA_Window_CloseRequest, TRUE);
      }
      else
      {
        // Just clear pointer to this mail if we don't want to close the window or if
        // there is no window to close at all.
        rmData->mail = NULL;
      }
    }

    curNode = nextNode;
  }

  // and last, but not least, we have to free the mail
  FreeMail(mail);

  LEAVE();
}

///
/// ReplaceMailInFolder
// replace a mail in a folder by a new one
struct Mail *ReplaceMailInFolder(const char *mailFile, const struct Mail *mail, struct Folder *folder, struct Mail **replacedMail)
{
  struct Mail *addedMail;

  ENTER();

  *replacedMail = NULL;

  if((addedMail = CloneMail(mail)) != NULL)
  {
    struct MailNode *mnode;

    LockMailList(folder->messages);

    if((mnode = FindMailByFilename(folder->messages, mailFile)) != NULL)
    {
  	  // remove the old mail's stats from the folder stats
  	  folder->Size -= mnode->mail->Size;

      if(hasStatusNew(mnode->mail))
        folder->New--;

      if(!hasStatusRead(mnode->mail))
        folder->Unread--;

      addedMail->Folder = folder;

      // addd the new mail's stats to the folder stats
      folder->Size += addedMail->Size;

      if(hasStatusNew(addedMail))
        folder->New++;

      if(!hasStatusRead(addedMail))
        folder->Unread++;

      // remember the replaced mail
      *replacedMail = mnode->mail;

      // replace the mail in the list
      mnode->mail = addedMail;
    }
    else
    {
      AddMailToFolderSimple(addedMail, folder);
    }

    UnlockMailList(folder->messages);

    // expire the folder's index as we just added a new message
    MA_ExpireIndex(folder);
  }

  RETURN(addedMail);
  return addedMail;
}

///
/// ClearFolderMails
//  Removes all messages from a folder
void ClearFolderMails(struct Folder *folder, BOOL resetstats)
{
  struct Node *node;

  ENTER();

  ASSERT(folder != NULL);
  ASSERT(folder->messages != NULL);
  ASSERT(folder->messages->lockSemaphore != NULL);
  D(DBF_FOLDER, "clearing mail list of folder '%s'", folder->Name);

  // First we check if there is any read window open with a mail
  // belonging to the folder we are about to clear.
  // Instead of checking each of the folder's mail to be contained
  // in thelist of active readMailData we just check if one the
  // active readMailData is pointing back to the folder. This is
  // much more efficient as one has usually only very few read
  // windows opened in parallel.
  node = GetHead((struct List *)&G->readMailDataList);
  while(node != NULL)
  {
    struct ReadMailData *rmData = (struct ReadMailData *)node;
    struct Node *nextNode = GetSucc(node);

    if(rmData->mail != NULL && rmData->mail->Folder == folder)
      CleanupReadMailData(rmData, TRUE);

    node = nextNode;
  }

  LockMailList(folder->messages);
  ClearMailList(folder->messages, TRUE);
  UnlockMailList(folder->messages);

  D(DBF_FOLDER, "cleared mail list of folder '%s'", folder->Name);

  if(resetstats == TRUE)
  {
    folder->Total = 0;
    folder->New = 0;
    folder->Unread = 0;
    folder->Size = 0;
  }

  LEAVE();
}
///
/// GetPackMethod
//  Returns packer type and efficiency
static BOOL GetPackMethod(enum FolderMode fMode, char **method, int *eff)
{
  BOOL result = TRUE;

  ENTER();

  switch(fMode)
  {
    case FM_XPKCOMP:
    {
      *method = C->XPKPack;
      *eff = C->XPKPackEff;
    }
    break;

    case FM_XPKCRYPT:
    {
      *method = C->XPKPackEncrypt;
      *eff = C->XPKPackEncryptEff;
    }
    break;

    default:
    {
      *method = NULL;
      *eff = 0;
      result = FALSE;
    }
    break;
  }

  RETURN(result);
  return result;
}
///
/// CompressMailFile
//  Shrinks a message file
static BOOL CompressMailFile(const char *src, const char *dst, const char *passwd, const char *method, int eff)
{
  long error = -1;

  ENTER();

  D(DBF_XPK, "CompressMailFile: %08lx - [%s] -> [%s] - [%s] - [%s] - %ld", XpkBase, src, dst, passwd, method, eff);

  if(XpkBase != NULL)
  {
    error = XpkPackTags(XPK_InName,      src,
                        XPK_OutName,     dst,
                        XPK_Password,    passwd,
                        XPK_PackMethod,  method,
                        XPK_PackMode,    eff,
                        TAG_DONE);

    #if defined(DEBUG)
    if(error != XPKERR_OK)
    {
      char buf[1024];

      XpkFault(error, NULL, buf, sizeof(buf));

      E(DBF_XPK, "XpkPackTags() returned an error %ld: '%s'", error, buf);
    }
    #endif
  }

  RETURN((BOOL)(error == XPKERR_OK));
  return (BOOL)(error == XPKERR_OK);
}
///
/// UncompressMailFile
//  Expands a compressed message file
static BOOL UncompressMailFile(const char *src, const char *dst, const char *passwd)
{
  long error = -1;

  ENTER();

  D(DBF_XPK, "UncompressMailFile: %08lx - [%s] -> [%s] - [%s]", XpkBase, src, dst, passwd);

  if(XpkBase != NULL)
  {
    error = XpkUnpackTags(XPK_InName,    src,
                          XPK_OutName,   dst,
                          XPK_Password,  passwd,
                          TAG_DONE);

    #if defined(DEBUG)
    if(error != XPKERR_OK)
    {
      char buf[1024];

      XpkFault(error, NULL, buf, sizeof(buf));

      E(DBF_XPK, "XpkUnPackTags() returned an error %ld: '%s'", error, buf);
    }
    #endif
  }

  RETURN((BOOL)(error == XPKERR_OK));
  return (BOOL)(error == XPKERR_OK);
}
///
/// TransferMailFile
//  Copies or moves a message file, handles compression
int TransferMailFile(BOOL copyit, struct Mail *mail, struct Folder *dstfolder)
{
  struct Folder *srcfolder = mail->Folder;
  enum FolderMode srcMode = srcfolder->Mode;
  enum FolderMode dstMode = dstfolder->Mode;
  int success = -1;

  ENTER();

  D(DBF_UTIL, "TransferMailFile: %s '%s' to '%s' %ld->%ld", copyit ? "copy" : "move", mail->MailFile, dstfolder->Fullpath, srcMode, dstMode);

  if(MA_GetIndex(srcfolder) == TRUE && MA_GetIndex(dstfolder) == TRUE)
  {
    char *pmeth;
    int peff = 0;
    char srcbuf[SIZE_PATHFILE];
    char dstbuf[SIZE_PATHFILE];
    char dstFileName[SIZE_MFILE];
    char *srcpw = srcfolder->Password;
    char *dstpw = dstfolder->Password;
    BOOL counterExceeded = FALSE;

    // get some information we require
    GetPackMethod(dstMode, &pmeth, &peff);
    GetMailFile(srcbuf, sizeof(srcbuf), NULL, mail);

    // check if we can just take the exactly same filename in the destination
    // folder or if we require to increase the mailfile counter to make it
    // unique
    strlcpy(dstFileName, mail->MailFile, sizeof(dstFileName));

    AddPath(dstbuf, dstfolder->Fullpath, dstFileName, sizeof(dstbuf));
    if(FileExists(dstbuf) == TRUE)
    {
      int mCounter = atoi(&dstFileName[13]);

      do
      {
        if(mCounter < 1 || mCounter >= 999)
          // no more numbers left
          // now we have to leave this function
          counterExceeded = TRUE;
        else
        {
          mCounter++;

          snprintf(&dstFileName[13], sizeof(dstFileName)-13, "%03d", mCounter);
          dstFileName[16] = ','; // restore it

          AddPath(dstbuf, dstfolder->Fullpath, dstFileName, sizeof(dstbuf));
        }
      }
      while(counterExceeded == FALSE && FileExists(dstbuf) == TRUE);

      if(counterExceeded == FALSE)
      {
        // if we end up here we finally found a new mailfilename which we can use, so
        // lets copy it to our MailFile variable
        D(DBF_UTIL, "renaming mail file from '%s' to '%s'", mail->MailFile, dstFileName);
        strlcpy(mail->MailFile, dstFileName, sizeof(mail->MailFile));
      }
    }

    if(counterExceeded == FALSE)
    {
      // now that we have the source and destination filename
      // we can go and do the file operation depending on some data we
      // acquired earlier
      if((srcMode == dstMode && srcMode <= FM_SIMPLE) ||
         (srcMode <= FM_SIMPLE && dstMode <= FM_SIMPLE))
      {
        if(copyit == TRUE)
          success = CopyFile(dstbuf, 0, srcbuf, 0) ? 1 : -1;
        else
          success = MoveFile(srcbuf, dstbuf) ? 1 : -1;
      }
      else if(isXPKFolder(srcfolder))
      {
        if(isXPKFolder(dstfolder) == FALSE)
        {
          // if we end up here the source folder is a compressed folder but the
          // destination one not. so lets uncompress it
          success = UncompressMailFile(srcbuf, dstbuf, srcpw) ? 1 : -2;
          if(success > 0 && copyit == FALSE)
            success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
        }
        else
        {
          // here the source folder is a compressed+crypted folder and the
          // destination one also, so we have to uncompress the file to a
          // temporarly file and compress it immediatly with the destination
          // password again.
          struct TempFile *tf;

          if((tf = OpenTempFile(NULL)) != NULL)
          {
            success = UncompressMailFile(srcbuf, tf->Filename, srcpw) ? 1 : -2;
            if(success > 0)
            {
              // compress it immediatly again
              success = CompressMailFile(tf->Filename, dstbuf, dstpw, pmeth, peff) ? 1 : -2;
              if(success > 0 && copyit == FALSE)
                success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
            }

            CloseTempFile(tf);
          }
        }
      }
      else
      {
        if(isXPKFolder(dstfolder))
        {
          // here the source folder is not compressed, but the destination one
          // so we compress the file in the destionation folder now
          success = CompressMailFile(srcbuf, dstbuf, dstpw, pmeth, peff) ? 1 : -2;
          if(success > 0 && copyit == FALSE)
            success = (DeleteFile(srcbuf) != 0) ? 1 : -1;
        }
        else
          // if we end up here then there is something seriously wrong
          success = -3;
      }
    }
  }

  RETURN(success);
  return success;
}
///
/// RepackMailFile
//  (Re/Un)Compresses a message file
//  Note: If dstMode is -1 and passwd is NULL, then this function packs
//        the current mail. It will assume it is plaintext and needs to be packed now
BOOL RepackMailFile(struct Mail *mail, enum FolderMode dstMode, const char *passwd)
{
  char *pmeth = NULL;
  char srcbuf[SIZE_PATHFILE];
  char dstbuf[SIZE_PATHFILE];
  struct Folder *folder;
  int peff = 0;
  enum FolderMode srcMode;
  BOOL success = FALSE;

  ENTER();

  folder = mail->Folder;
  srcMode = folder->Mode;

  // if this function was called with dstxpk=-1 and passwd=NULL then
  // we assume we need to pack the file from plain text to the currently
  // selected pack method of the folder
  if((LONG)dstMode == -1 && passwd == NULL)
  {
    srcMode = FM_NORMAL;
    dstMode = folder->Mode;
    passwd  = folder->Password;
  }

  MA_GetIndex(folder);
  GetMailFile(srcbuf, sizeof(srcbuf), NULL, mail);
  GetPackMethod(dstMode, &pmeth, &peff);
  snprintf(dstbuf, sizeof(dstbuf), "%s.tmp", srcbuf);

  SHOWSTRING(DBF_UTIL, srcbuf);

  if((srcMode == dstMode && srcMode <= FM_SIMPLE) ||
     (srcMode <= FM_SIMPLE && dstMode <= FM_SIMPLE))
  {
    // the FolderModes are the same so lets do nothing
    success = TRUE;

    D(DBF_UTIL, "repack not required.");
  }
  else if(srcMode > FM_SIMPLE)
  {
    if(dstMode <= FM_SIMPLE)
    {
      D(DBF_UTIL, "uncompressing");

      // if we end up here the source folder is a compressed folder so we
      // have to just uncompress the file
      if(UncompressMailFile(srcbuf, dstbuf, folder->Password) &&
         DeleteFile(srcbuf) != 0)
      {
        if(RenameFile(dstbuf, srcbuf) != 0)
          success = TRUE;
      }
    }
    else
    {
      // if we end up here, the source folder is a compressed+crypted one and
      // the destination mode also
      D(DBF_UTIL, "uncompressing/recompress");

      if(UncompressMailFile(srcbuf, dstbuf, folder->Password) &&
         CompressMailFile(dstbuf, srcbuf, passwd, pmeth, peff))
      {
        if(DeleteFile(dstbuf) != 0)
          success = TRUE;
      }
    }
  }
  else
  {
    if(dstMode > FM_SIMPLE)
    {
      D(DBF_UTIL, "compressing");

      // here the source folder is not compressed, but the destination mode
      // signals to compress it
      if(CompressMailFile(srcbuf, dstbuf, passwd, pmeth, peff) &&
         DeleteFile(srcbuf) != 0)
      {
        success = RenameFile(dstbuf, srcbuf);
      }
    }
  }

  MA_UpdateMailFile(mail);

  RETURN(success);
  return success;
}
///
/// DoPack
//  Compresses a file
BOOL DoPack(const char *file, const char *newfile, const struct Folder *folder)
{
  char *pmeth = NULL;
  int peff = 0;
  BOOL result = FALSE;

  ENTER();

  if(GetPackMethod(folder->Mode, &pmeth, &peff) == TRUE)
  {
    if(CompressMailFile(file, newfile, folder->Password, pmeth, peff) == TRUE)
    {
      if(DeleteFile(file) != 0)
      {
        result = TRUE;
      }
    }
  }

  RETURN(result);
  return result;
}
///
/// StartUnpack
//  Unpacks a file to a temporary file
char *StartUnpack(const char *file, char *newfile, const struct Folder *folder)
{
  FILE *fh;
  char *result = NULL;

  ENTER();

  if((fh = fopen(file, "r")) != NULL)
  {
    BOOL xpk = FALSE;

    // check if the source file is really XPK compressed or not.
    if(fgetc(fh) == 'X' && fgetc(fh) == 'P' && fgetc(fh) == 'K')
      xpk = TRUE;

    fclose(fh);
    fh = NULL;

    // now we compose a temporary filename and start
    // uncompressing the source file into it.
    if(xpk == TRUE)
    {
      char nfile[SIZE_FILE];

      snprintf(nfile, sizeof(nfile), "YAMu%08x.unp", (unsigned int)GetUniqueID());
      AddPath(newfile, C->TempDir, nfile, SIZE_PATHFILE);

      // check that the destination filename
      // doesn't already exist
      if(FileExists(newfile) == FALSE && UncompressMailFile(file, newfile, folder ? folder->Password : ""))
        result = newfile;
    }
    else
    {
      strcpy(newfile, file);
      result = newfile;
    }
  }

  RETURN(result);
  return result;
}
///
/// FinishUnpack
//  Deletes temporary unpacked file
void FinishUnpack(const char *file)
{
  char ext[SIZE_FILE];

  ENTER();

  // we just delete if this is really related to a unpack file
  stcgfe(ext, file);
  if(strcmp(ext, "unp") == 0)
  {
    struct Node *curNode;

    IterateList(&G->readMailDataList, curNode)
    {
      struct ReadMailData *rmData = (struct ReadMailData *)curNode;

      // check if the file is still in use and if so we quit immediately
      // leaving the file untouched.
      if(stricmp(file, rmData->readFile) == 0)
      {
        LEAVE();
        return;
      }
    }

    if(DeleteFile(file) == 0)
      AddZombieFile(file);
  }

  LEAVE();
}
///

/*** Hooks ***/
/// ExamineDirMatchHook
// dos.library 52.12 from the July update doesn't use the supplied match string
// correctly and simply returns all directory entries instead of just the matching
// ones. So we have to do the dirty work ourself. This bug has been fixed
// since dos.library 52.17.
#if defined(__amigaos4__)
HOOKPROTONH(ExamineDirMatchFunc, LONG, CONST_STRPTR matchString, const struct ExamineData *ed)
{
  LONG matches = TRUE;

  ENTER();

  if(matchString != NULL)
    matches = MatchPatternNoCase(matchString, ed->Name);

  RETURN(matches);
  return matches;
}
MakeHook(ExamineDirMatchHook, ExamineDirMatchFunc);
#endif
///

/*** MUI related ***/
/// SafeOpenWindow
//  Tries to open a window
BOOL SafeOpenWindow(Object *obj)
{
  BOOL success = FALSE;

  ENTER();

  if(obj != NULL)
  {
    // make sure we open the window object
    set(obj, MUIA_Window_Open, TRUE);

    D(DBF_GUI, "window with title '%s' is %s", (char *)xget(obj, MUIA_Window_Title), xget(obj, MUIA_Window_Open) == TRUE ? "open" : "not open");
    D(DBF_GUI, "YAM is %s", xget(G->App, MUIA_Application_Iconified) == TRUE ? "iconified" : "not iconified");

    // now we check whether the window was successfully
    // opened or the application is in iconify state
    if(xget(obj, MUIA_Window_Open) == TRUE ||
       xget(G->App, MUIA_Application_Iconified) == TRUE)
    {
      success = TRUE;
    }
  }

  if(success == FALSE && obj != NULL)
  {
    // otherwise we perform a DisplayBeep()
    E(DBF_ALWAYS, "failed to open window with title '%s'", (char *)xget(obj, MUIA_Window_Title));
    DisplayBeep(NULL);
  }

  RETURN(success);
  return success;
}
///
/// DisposeModule
// Free resources of a MUI window
void DisposeModule(void *modptr)
{
  struct UniversalClassData **module = (struct UniversalClassData **)modptr;

  ENTER();

  if(*module != NULL)
  {
    Object *window = (*module)->GUI.WI;

    D(DBF_GUI, "removing window from app: %08lx", window);

    // close the window
    set(window, MUIA_Window_Open, FALSE);

    // remove the window from our app
    DoMethod(G->App, OM_REMMEMBER, window);

    // dispose the window object
    MUI_DisposeObject(window);

    free(*module);
    *module = NULL;
  }

  LEAVE();
}
HOOKPROTONHNO(DisposeModuleFunc, void, void **arg)
{
  DisposeModule(arg[0]);
}
MakeHook(DisposeModuleHook,DisposeModuleFunc);
///
/// LoadLayout
//  Loads column widths from ENV:MUI/YAM.cfg
void LoadLayout(void)
{
  const char *ls;
  char *endptr;

  ENTER();

  // set some sensible default values first
  G->Weights[0] = 30;
  G->Weights[1] = 100;
  G->Weights[6] = 50;
  G->Weights[4] = 30;
  G->Weights[5] = 100;
  G->Weights[7] = 100;
  G->Weights[8] = 5;
  G->Weights[9] = 100;
  G->Weights[10] = 5;
  G->Weights[11] = 100;
  strlcpy(G->preselectionListLayout, EMPTY_B64DSPACE_STRING, sizeof(G->preselectionListLayout));

  // Load the application configuration from the ENV: directory.
  DoMethod(G->App, MUIM_Application_Load, MUIV_Application_Load_ENV);

  // we encode the different weight factors which are embeeded in a dummy string
  // gadgets:
  //
  // 0:  Horizontal weight of left foldertree in main window.
  // 1:  Horizontal weight of right maillistview in main window.
  // 2:  unused
  // 3:  unused
  // 4:  Horizontal weight of listview group in the glossary window
  // 5:  Horizontal weight of text group in the glossary window
  // 6:  Vertical weight of top right maillistview group in main window.
  // 7:  Vertical weight of bottom right embedded read pane object in the main window.
  // 8:  Vertical weight of top object (headerlist) of the embedded read pane
  // 9:  Vertical weight of bottom object (texteditor) of the embedded read pane
  // 10: Vertical weight of top object (headerlist) in a read window
  // 11: Vertical weight of bottom object (texteditor) in a read window

  if((ls = (STRPTR)xget(G->MA->GUI.ST_LAYOUT, MUIA_String_Contents)) == NULL ||
      ls[0] == '\0')
  {
    ls = "\n";

    D(DBF_UTIL, "using default layout string '%s'", ls);
  }
  else
    D(DBF_UTIL, "loaded layout string '%s'", ls);

  D(DBF_UTIL, "first character '%lc' is %s", ls[0], isdigit(ls[0]) ? "a digit" : "no digit");

  // old style layout strings start with a number
  // do NOT check for TRUE, as isdigit() is documented to return a non-zero value only
  if(isdigit(ls[0]))
  {
    LONG v;

    D(DBF_UTIL, "parsing old style layout string");

    // lets get the numbers for each weight factor out of the contents
    // of the fake string gadget
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[0] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[1] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[2] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[3] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[4] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[5] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[6] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[7] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[8] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[9] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[10] = v;

    ls = endptr;
    v = strtol(ls, &endptr, 10);
    if(endptr != NULL && endptr != ls)
      G->Weights[11] = v;

    if(endptr != NULL)
      strlcpy(G->preselectionListLayout, Trim(endptr), sizeof(G->preselectionListLayout));
  }
  else
  {
    struct RDArgs *rdsource;

    D(DBF_UTIL, "parsing ReadArgs() style layout string");

    // allocate an additional RDArgs structure as we are going to let ReadArgs()
    // work on our own buffer instead of the command line
    if((rdsource = AllocDosObject(DOS_RDARGS, NULL)) != NULL)
    {
      // Use this template for parsing the string.
      // Note the DUMMY option at the end to catch any value which
      // could not be assigned to any variable.
      #define LAYOUT_TEMPLATE  "MAINFOLDERTREEHORIZ/K/N," \
                               "MAINMAILLISTHORIZ/K/N," \
                               "MAINMAILLISTVERT/K/N," \
                               "GLOSSARYLISTHORIZ/K/N," \
                               "GLOSSARYTEXTHORIZ/K/N," \
                               "READPANEVERT/K/N," \
                               "READPANEHEADERVERT/K/N," \
                               "READPANETEXTVERT/K/N," \
                               "READWINHEADERVERT/K/N," \
                               "READWINTEXTVERT/K/N," \
                               "PRESELECTIONLIST/K," \
                               "DUMMY/M"

      union LayoutArgs
      {
        LONG array[12];
        struct
        {
          LONG *mainFolderTreeHoriz;
          LONG *mainMailListHoriz;
          LONG *mainMailListVert;
          LONG *glossaryListHoriz;
          LONG *glossaryTextHoriz;
          LONG *readPaneVert;
          LONG *readPaneHeaderVert;
          LONG *readPaneTextVert;
          LONG *readWinHeaderVert;
          LONG *readWinTextVert;
          STRPTR preselectionList;
          STRPTR *dummy;
        } vars;
      } args;
      struct RDArgs *rda;

      // fill in the string to be parsed
      rdsource->RDA_Source.CS_Buffer = (STRPTR)ls;
      rdsource->RDA_Source.CS_Length = strlen(ls);
      rdsource->RDA_Source.CS_CurChr = 0;

      memset(&args, 0, sizeof(args));

      // now let DOS parse the layout string
      if((rda = ReadArgs(LAYOUT_TEMPLATE, args.array, rdsource)) != NULL)
      {
        D(DBF_UTIL, "ReadArgs() of '%s' succeeded", ls);

        if(args.vars.mainFolderTreeHoriz != NULL)
          G->Weights[0] = args.vars.mainFolderTreeHoriz[0];

        if(args.vars.mainMailListHoriz != NULL)
          G->Weights[1] = args.vars.mainMailListHoriz[0];

        if(args.vars.mainMailListVert != NULL)
          G->Weights[6] = args.vars.mainMailListVert[0];

        if(args.vars.glossaryListHoriz != NULL)
          G->Weights[4] = args.vars.glossaryListHoriz[0];

        if(args.vars.glossaryTextHoriz != NULL)
          G->Weights[5] = args.vars.glossaryTextHoriz[0];

        if(args.vars.readPaneVert != NULL)
          G->Weights[7] = args.vars.readPaneVert[0];

        if(args.vars.readPaneHeaderVert != NULL)
          G->Weights[8] = args.vars.readPaneHeaderVert[0];

        if(args.vars.readPaneTextVert != NULL)
          G->Weights[9] = args.vars.readPaneTextVert[0];

        if(args.vars.readWinHeaderVert != NULL)
          G->Weights[10] = args.vars.readWinHeaderVert[0];

        if(args.vars.readWinTextVert != NULL)
          G->Weights[11] = args.vars.readWinTextVert[0];

        if(args.vars.preselectionList != NULL)
          strlcpy(G->preselectionListLayout, args.vars.preselectionList, sizeof(G->preselectionListLayout));

        if(args.vars.dummy != NULL)
          D(DBF_UTIL, "ignored layout parameters '%s'", args.vars.dummy);

        FreeArgs(rda);
      }
      else
        E(DBF_UTIL, "ReadArgs() of '%s' failed, error %ld", ls, IoErr());

      FreeDosObject(DOS_RDARGS, rdsource);
    }
  }

  // lets set the weight factors to the corresponding GUI elements now
  // if they exist
  set(G->MA->GUI.LV_FOLDERS,  MUIA_HorizWeight, G->Weights[0]);
  set(G->MA->GUI.GR_MAILVIEW, MUIA_HorizWeight, G->Weights[1]);

  // if the embedded read pane is active we set its weight values
  if(C->EmbeddedReadPane == TRUE)
  {
    set(G->MA->GUI.GR_MAILLIST, MUIA_VertWeight, G->Weights[6]);
    xset(G->MA->GUI.MN_EMBEDDEDREADPANE, MUIA_VertWeight,                 G->Weights[7],
                                         MUIA_ReadMailGroup_HGVertWeight, G->Weights[8],
                                         MUIA_ReadMailGroup_TGVertWeight, G->Weights[9]);
  }

  LEAVE();
}
///
/// SaveLayout
//  Saves column widths to ENV(ARC):MUI/YAM.cfg
void SaveLayout(BOOL permanent)
{
  char *buf;

  ENTER();

  // create a ReadArgs() compatible string containing all the weight values
  // this one must be terminated by LF
  if(asprintf(&buf, "MAINFOLDERTREEHORIZ=%d " \
                    "MAINMAILLISTHORIZ=%d " \
                    "MAINMAILLISTVERT=%d " \
                    "GLOSSARYLISTHORIZ=%d " \
                    "GLOSSARYTEXTHORIZ=%d " \
                    "READPANEVERT=%d " \
                    "READPANEHEADERVERT=%d " \
                    "READPANETEXTVERT=%d " \
                    "READWINHEADERVERT=%d " \
                    "READWINTEXTVERT=%d " \
                    "PRESELECTIONLIST=\"%s\" " \
                    "\n",
    (int)G->Weights[0],
    (int)G->Weights[1],
    (int)G->Weights[6],
    (int)G->Weights[4],
    (int)G->Weights[5],
    (int)G->Weights[7],
    (int)G->Weights[8],
    (int)G->Weights[9],
    (int)G->Weights[10],
    (int)G->Weights[11],
    G->preselectionListLayout) != -1)
  {
    setstring(G->MA->GUI.ST_LAYOUT, buf);

    DoMethod(G->App, MUIM_Application_Save, MUIV_Application_Save_ENV);

    // if we want to save to ENVARC:
    if(permanent == TRUE)
    {
      APTR oldWindowPtr;

      // this is for the people out there having their SYS: partition locked and whining about
      // YAM popping up a error requester upon the exit - so it's their fault now if
      // the MUI objects aren't saved correctly.
      oldWindowPtr = SetProcWindow((APTR)-1);

      DoMethod(G->App, MUIM_Application_Save, MUIV_Application_Save_ENVARC);

      D(DBF_UTIL, "permanently saved layout string '%s'", SafeStr(buf));

      // restore the old windowPtr
      SetProcWindow(oldWindowPtr);
    }
    else
      D(DBF_UTIL, "saved layout weight string '%s'", SafeStr(buf));

    free(buf);
  }

  LEAVE();
}

///
/// ConvertKey
//  Converts input event to key code
unsigned char ConvertKey(const struct IntuiMessage *imsg)
{
  struct InputEvent ie;
  unsigned char code = 0;

  ENTER();

  ie.ie_NextEvent    = NULL;
  ie.ie_Class        = IECLASS_RAWKEY;
  ie.ie_SubClass     = 0;
  ie.ie_Code         = imsg->Code;
  ie.ie_Qualifier    = imsg->Qualifier;
  ie.ie_EventAddress = (APTR *) *((ULONG *)imsg->IAddress);

  if(MapRawKey(&ie, (STRPTR)&code, 1, NULL) != 1)
    W(DBF_GUI, "MapRawKey returned != 1 (%08lx)", code);

  RETURN(code);
  return code;
}
///

/*** GFX related ***/
#if !defined(__amigaos4__)
/// struct LayerHookMsg
struct LayerHookMsg
{
  struct Layer *layer;
  struct Rectangle bounds;
  LONG offsetx;
  LONG offsety;
};

///
/// struct BltHook
struct BltHook
{
  struct Hook hook;
  struct BitMap maskBitMap;
  struct BitMap *srcBitMap;
  LONG srcx,srcy;
  LONG destx,desty;
};

///
/// MyBltMaskBitMap
static void MyBltMaskBitMap(const struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct BitMap *destBitMap, LONG xDest, LONG yDest, LONG xSize, LONG ySize, struct BitMap *maskBitMap)
{
  ENTER();

  BltBitMap((struct BitMap *)srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);
  BltBitMap(maskBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0xe2,~0,NULL);
  BltBitMap((struct BitMap *)srcBitMap,xSrc,ySrc,destBitMap, xDest, yDest, xSize, ySize, 0x99,~0,NULL);

  LEAVE();
}

///
/// BltMaskHook
HOOKPROTO(BltMaskFunc, void, struct RastPort *rp, struct LayerHookMsg *msg)
{
  struct BltHook *h = (struct BltHook*)hook;

  LONG width = msg->bounds.MaxX - msg->bounds.MinX+1;
  LONG height = msg->bounds.MaxY - msg->bounds.MinY+1;
  LONG offsetx = h->srcx + msg->offsetx - h->destx;
  LONG offsety = h->srcy + msg->offsety - h->desty;

  MyBltMaskBitMap(h->srcBitMap, offsetx, offsety, rp->BitMap, msg->bounds.MinX, msg->bounds.MinY, width, height, &h->maskBitMap);
}
MakeStaticHook(BltMaskHook, BltMaskFunc);

///
/// MyBltMaskBitMapRastPort
void MyBltMaskBitMapRastPort(struct BitMap *srcBitMap, LONG xSrc, LONG ySrc, struct RastPort *destRP, LONG xDest, LONG yDest, LONG xSize, LONG ySize, ULONG minterm, APTR bltMask)
{
  ENTER();

  if(GetBitMapAttr(srcBitMap, BMA_FLAGS) & BMF_INTERLEAVED)
  {
    LONG src_depth = GetBitMapAttr(srcBitMap, BMA_DEPTH);
    struct Rectangle rect;
    struct BltHook hook;

    // Define the destination rectangle in the rastport
    rect.MinX = xDest;
    rect.MinY = yDest;
    rect.MaxX = xDest + xSize - 1;
    rect.MaxY = yDest + ySize - 1;

    // Initialize the hook
    InitHook(&hook.hook, BltMaskHook, NULL);
    hook.srcBitMap = srcBitMap;
    hook.srcx = xSrc;
    hook.srcy = ySrc;
    hook.destx = xDest;
    hook.desty = yDest;

    // Initialize a bitmap where all plane pointers points to the mask
    InitBitMap(&hook.maskBitMap, src_depth, GetBitMapAttr(srcBitMap, BMA_WIDTH), GetBitMapAttr(srcBitMap, BMA_HEIGHT));
    while(src_depth)
    {
      hook.maskBitMap.Planes[--src_depth] = bltMask;
    }

    // Blit onto the Rastport */
    DoHookClipRects(&hook.hook, destRP, &rect);
  }
  else
    BltMaskBitMapRastPort(srcBitMap, xSrc, ySrc, destRP, xDest, yDest, xSize, ySize, minterm, bltMask);

  LEAVE();
}

///
#endif

/*** Miscellaneous stuff ***/
/// PGPGetPassPhrase
//  Asks user for the PGP passphrase
void PGPGetPassPhrase(void)
{
  char pgppass[SIZE_DEFAULT];

  ENTER();

  // check if a PGPPASS variable exists already
  if(GetVar("PGPPASS", pgppass, sizeof(pgppass), GVF_GLOBAL_ONLY) < 0)
  {
    // check if we really require to request a passphrase from
    // the user
    if(G->PGPPassPhrase[0] != '\0' &&
       C->PGPPassInterval > 0 && G->LastPGPUsage > 0 &&
       time(NULL)-G->LastPGPUsage <= (time_t)(C->PGPPassInterval*60))
    {
      // nothing
    }
    else
    {
      pgppass[0] = '\0';

      if(PassphraseRequest(pgppass, SIZE_DEFAULT, G->MA->GUI.WI) > 0)
        G->LastPGPUsage = time(NULL);
      else
        G->LastPGPUsage = 0;

      strlcpy(G->PGPPassPhrase, pgppass, sizeof(G->PGPPassPhrase));
    }

    // make sure we delete the passphrase variable immediately after
    // having processed the PGP command
    G->PGPPassVolatile = TRUE;

    // set a global PGPPASS variable, but do not write it
    // to ENVARC:
    SetVar("PGPPASS", G->PGPPassPhrase, -1, GVF_GLOBAL_ONLY);
  }
  else
  {
    W(DBF_MAIL, "ENV:PGPPASS already exists!");

    // don't delete env-variable on PGPClearPassPhrase()
    G->PGPPassVolatile = FALSE;

    // copy the content of the env variable to our
    // global passphrase variable
    strlcpy(G->PGPPassPhrase, pgppass, sizeof(G->PGPPassPhrase));
    G->LastPGPUsage = 0;
  }

  LEAVE();
}
///
/// PGPClearPassPhrase
//  Clears the ENV variable containing the PGP passphrase
void PGPClearPassPhrase(BOOL force)
{
  ENTER();

  if(G->PGPPassVolatile)
    DeleteVar("PGPPASS", GVF_GLOBAL_ONLY);

  if(force)
    G->PGPPassPhrase[0] = '\0';

  LEAVE();
}
///
/// PGPCommand
//  Launches a PGP command
LONG PGPCommand(const char *progname, const char *options, const int flags)
{
  LONG error;
  char command[SIZE_LARGE];
  struct BusyNode *busy;

  ENTER();

  D(DBF_UTIL, "[%s] [%s] - flags: %ld", progname, options, flags);

  AddPath(command, C->PGPCmdPath, progname, sizeof(command));
  strlcat(command, " >" PGPLOGFILE " ", sizeof(command));
  strlcat(command, options, sizeof(command));

  busy = BusyBegin(BUSY_TEXT);
  BusyText(busy, tr(MSG_BusyPGPrunning), "");
  error = LaunchCommand(command, LAUNCHF_IGNORE_RC, OUT_REDIRECT);
  BusyEnd(busy);

  if(error > 0 && !hasNoErrorsFlag(flags))
    ER_NewError(tr(MSG_ER_PGPreturnsError), command, PGPLOGFILE);
  else if(error < 0)
    ER_NewError(tr(MSG_ER_PGPnotfound), C->PGPCmdPath);
  else if(error == 0 && !hasKeepLogFlag(flags))
  {
    if(DeleteFile(PGPLOGFILE) == 0)
      AddZombieFile(PGPLOGFILE);
  }

  RETURN(error);
  return error;
}

///
/// DisplayStatistics
//  Calculates folder statistics and update mailbox status icon
void DisplayStatistics(struct Folder *fo, BOOL updateAppIcon)
{
  ENTER();

  // If the parsed argument is NULL we want to show the statistics from the actual folder
  if(fo == NULL)
    fo = GetCurrentFolder();
  else if(fo == (struct Folder *)-1)
    fo = FO_GetFolderByType(FT_INCOMING, NULL);

  if(fo != NULL)
  {
    D(DBF_GUI, "updating statistics for folder '%s', appicon %ld", fo->Name, updateAppIcon);

    // update the stats for this folder
    FO_UpdateStatistics(fo);
    // Recalc the number of messages of the folder group
    FO_UpdateTreeStatistics(fo, TRUE);

    if(fo == GetCurrentFolder())
    {
      DoMethod(G->MA->GUI.IB_INFOBAR, MUIM_InfoBar_SetFolder, fo);
    }

    if(G->AppIconQuiet == FALSE && updateAppIcon == TRUE)
      UpdateAppIcon();
  }

  LEAVE();
}

///
/// FolderTreeUpdate
// updates the stats of all folders and redraws the folder tree
void FolderTreeUpdate(void)
{
  struct FolderNode *fnode;

  ENTER();

  LockFolderList(G->folders);

  // first clear the stats of all folder groups
  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    if(isGroupFolder(folder) == TRUE)
    {
      folder->Unread  = 0;
      folder->New     = 0;
      folder->Total   = 0;
      folder->Sent    = 0;
      folder->Deleted = 0;
      folder->Size    = 0;
    }
  }

  // new update the stats of all folders
  ForEachFolderNode(G->folders, fnode)
  {
    struct Folder *folder = fnode->folder;

    if(isGroupFolder(folder) == FALSE)
    {
      struct FolderNode *parentFNode;

      FO_UpdateStatistics(folder);

      // add the folder stats to the parent group's stats if it exists
      parentFNode = folder->parent;
      while(parentFNode != NULL)
      {
        struct Folder *parentFolder = parentFNode->folder;

        parentFolder->Unread  += folder->Unread;
        parentFolder->New     += folder->New;
        parentFolder->Total   += folder->Total;
        parentFolder->Sent    += folder->Sent;
        parentFolder->Deleted += folder->Deleted;
        parentFolder->Size    += folder->Size;

        parentFNode = parentFolder->parent;
      }
    }
  }

  UnlockFolderList(G->folders);

  // redraw the folder tree and the AppIcon
  DoMethod(G->MA->GUI.NL_FOLDERS, MUIM_NListtree_Redraw, MUIV_NListtree_Redraw_All, MUIF_NONE);
  if(G->AppIconQuiet == FALSE)
    UpdateAppIcon();

  LEAVE();
}

///
/// CheckPrinter
//  Checks if printer is ready to print something
BOOL CheckPrinter(const Object *win)
{
  BOOL result = FALSE;

  ENTER();

  // check if the user wants us to check the printer state
  // at all.
  if(C->PrinterCheck == TRUE)
  {
    struct MsgPort *mp;

    // create the message port
    if((mp = AllocSysObjectTags(ASOT_PORT, TAG_DONE)) != NULL)
    {
      struct IOStdReq *pio;

      // create the IO request for checking the printer status
      if((pio = AllocSysObjectTags(ASOT_IOREQUEST,
                                   ASOIOR_Size,      sizeof(struct IOStdReq),
                                   ASOIOR_ReplyPort, (ULONG)mp,
                                   TAG_DONE)) != NULL)
      {
        // from here on we assume the printer is online
        // but we do deeper checks.
        result = TRUE;

        // open printer.device unit 0
        if(OpenDevice("printer.device", 0, (struct IORequest *)pio, 0) == 0)
        {
          // we allow to retry the checking so
          // we iterate into a do/while loop
          do
          {
            UWORD ioResult = 0;

            // fill the IO request for querying the
            // device/line status of printer.device
            pio->io_Message.mn_ReplyPort = mp;
            pio->io_Command = PRD_QUERY;
            pio->io_Data = &ioResult;
            pio->io_Actual = 0;

            // initiate the IO request
            if(DoIO((struct IORequest *)pio) == 0)
            {
              // printer seems to be a parallel printer
              if(pio->io_Actual == 1)
              {
                D(DBF_PRINT, "received io request status: %08lx", ioResult);

                // check for any possible error state
                if(isFlagSet(ioResult>>8, (1<<0))) // printer busy (offline)
                {
                  ULONG res;

                  W(DBF_PRINT, "printer found to be in 'busy or offline' status");

                  // issue a requester telling the user about the faulty
                  // printer state
                  res = MUI_Request(G->App, win, MUIF_NONE, tr(MSG_ErrorReq),
                                                            tr(MSG_ER_PRINTER_OFFLINE_GADS),
                                                            tr(MSG_ER_PRINTER_OFFLINE));

                  if(res == 0) // Cancel/ESC
                  {
                    result = FALSE;
                    break;
                  }
                  else if(res == 1) // Retry
                    continue;
                  else // Ignore
                    break;
                }
                else if(isFlagSet(ioResult>>8, (1<<1))) // paper out
                {
                  ULONG res;

                  W(DBF_PRINT, "printer found to be in 'paper out' status");

                  // issue a requester telling the user about the faulty
                  // printer state
                  res = MUI_Request(G->App, win, MUIF_NONE, tr(MSG_ErrorReq),
                                                            tr(MSG_ER_PRINTER_NOPAPER_GADS),
                                                            tr(MSG_ER_PRINTER_NOPAPER));

                  if(res == 0) // Cancel/ESC
                  {
                    result = FALSE;
                    break;
                  }
                  else if(res == 1) // Retry
                    continue;
                  else // Ignore
                    break;
                }
                else
                {
                  D(DBF_PRINT, "printer was found to be ready");
                  break;
                }
              }
              else
              {
                // the rest signals an unsupported printer device
                // for status checking, so we assume the printer to
                // be online
                W(DBF_PRINT, "unsupported printer device ID '%ld'. Assuming online.", pio->io_Actual);
                break;
              }
            }
            else
            {
              W(DBF_PRINT, "DoIO() on printer status request failed!");
              break;
            }
          }
          while(TRUE);

          CloseDevice((struct IORequest *)pio);
        }
        else
          W(DBF_PRINT, "couldn't open printer.device unit 0");

        FreeSysObject(ASOT_IOREQUEST, pio);
      }
      else
        W(DBF_PRINT, "wasn't able to create io request for printer state checking");

      FreeSysObject(ASOT_PORT, mp);
    }
    else
      W(DBF_PRINT, "wasn't able to create msg port for printer state checking");
  }
  else
  {
    W(DBF_PRINT, "PrinterCheck disabled, assuming printer online");
    result = TRUE;
  }

  RETURN(result);
  return result;
}
///
/// PlaySound
//  Plays a sound file using datatypes
BOOL PlaySound(const char *filename)
{
  BOOL result = FALSE;

  ENTER();

  if(DataTypesBase != NULL)
  {
    // if we previously created a sound object
    // lets dispose it first.
    if(G->SoundDTObj != NULL)
    {
      DoMethod(G->SoundDTObj, DTM_TRIGGER, NULL, STM_STOP, NULL);
      DisposeDTObject(G->SoundDTObj);
    }

    // create the new datatype object
    if((G->SoundDTObj = NewDTObject((char *)filename, DTA_SourceType, DTST_FILE,
                                                      DTA_GroupID,    GID_SOUND,
                                                      SDTA_Cycles,    1,
                                                      TAG_DONE)) != NULL)
    {
      ULONG error;

      // play the sound
      #if defined(__amigaos4__) || defined(__MORPHOS__)
      // AmigaOS4's sound.dt returns 1 in case everything was ok, 0 otherwise
      error = DoMethod(G->SoundDTObj, DTM_TRIGGER, NULL, STM_PLAY, NULL);
      #else
      // AmigaOS3's return value differs between different versions of sound.dt
      // Some always return 0, some return a value from the subclass.
      // AROS' sound.dt definitely always returns 0. I assume the same for MorphOS.
      // Thus we always signal success here in case creating the object succeeded.
      DoMethod(G->SoundDTObj, DTM_TRIGGER, NULL, STM_PLAY, NULL);
      error = 1;
      #endif

      if(error == 1)
        result = TRUE;

      D(DBF_UTIL, "started playback of '%s' returned %ld/%ld", filename, error, result);
    }
    else
      W(DBF_UTIL, "failed to create sound DT object from '%s'", filename);
  }
  else
    W(DBF_UTIL, "datatypes.library missing, no sound playback!");

  // let the application show an error in case anything went wrong
  if(result == FALSE)
    ER_NewError(tr(MSG_ERROR_PLAYSOUND), filename);

  RETURN(result);
  return result;
}

///
/// MatchExtension
//  Matches a file extension against a list of extension
static BOOL MatchExtension(const char *fileext, const char *extlist)
{
  BOOL result = FALSE;

  ENTER();

  if(extlist)
  {
    const char *s = extlist;
    size_t extlen = strlen(fileext);

    // now we search for our delimiters step by step
    while(*s)
    {
      const char *e;

      if((e = strpbrk(s, " |;,")) == NULL)
        e = s+strlen(s);

      D(DBF_MIME, "try matching file extension '%s' with '%s' %ld", fileext, s, e-s);

      // now check if the extension matches
      if((size_t)(e-s) == extlen &&
         strnicmp(s, fileext, extlen) == 0)
      {
        D(DBF_MIME, "matched file extension '%s' with type '%s'", fileext, s);

        result = TRUE;
        break;
      }

      // set the next start to our last search
      if(*e)
        s = ++e;
      else
        break;
    }
  }

  RETURN(result);
  return result;
}

///
/// IdentifyFile
// Tries to identify a file and returns its content-type if applicable
// otherwise NULL
const char *IdentifyFile(const char *fname)
{
  char ext[SIZE_FILE];
  const char *ctype = NULL;

  ENTER();

  // Here we try to identify the file content-type in multiple steps:
  //
  // 1: try to walk through the users' mime type list and check if
  //    a specified extension in the list matches the one of our file.
  //
  // 2: check against our hardcoded internal list of known extensions
  //    and try to do some semi-detailed analysis of the file header
  //
  // 3: use datatypes.library to find out the file class and construct
  //    an artifical content-type partly matching the file.

  // extract the extension of the file name first
  stcgfe(ext, fname);
  SHOWSTRING(DBF_MIME, ext);

  // now we try to identify the file by the extension first
  if(ext[0] != '\0')
  {
    struct Node *curNode;

    D(DBF_MIME, "identifying file by extension (mimeTypeList)");
    // identify by the user specified mime types
    IterateList(&C->mimeTypeList, curNode)
    {
      struct MimeTypeNode *curType = (struct MimeTypeNode *)curNode;

      if(curType->Extension[0] != '\0' &&
         MatchExtension(ext, curType->Extension))
      {
        ctype = curType->ContentType;
        break;
      }
    }

    if(ctype == NULL)
    {
      unsigned int i;

      D(DBF_MIME, "identifying file by extension (hardcoded list)");

      // before we are going to try to identify the file by reading some bytes out of
      // it, we try to identify it only by the extension.
      for(i=0; IntMimeTypeArray[i].ContentType != NULL; i++)
      {
        if(IntMimeTypeArray[i].Extension != NULL &&
           MatchExtension(ext, IntMimeTypeArray[i].Extension))
        {
          ctype = IntMimeTypeArray[i].ContentType;
          break;
        }
      }
    }
  }

  // go on if we haven't got a content-type yet and try to identify
  // it with our own, hardcoded means.
  if(ctype == NULL)
  {
    FILE *fh;

    D(DBF_MIME, "identifying file by binary comparing the first bytes of '%s'", fname);

    // now that we still haven't been able to identify the file, we go
    // and read in some bytes from the file and try to identify it by analyzing
    // the binary data.
    if((fh = fopen(fname, "r")))
    {
      char buffer[SIZE_LARGE];
      int rlen;
      long *lbuf = (long *)buffer;

      // we read in SIZE_LARGE into our temporary buffer without
      // checking if it worked out.
      rlen = fread(buffer, 1, SIZE_LARGE-1, fh);
      buffer[rlen] = '\0'; // NUL terminate the buffer.

      // close the file immediately.
      fclose(fh);
      fh = NULL;

      if(!strnicmp(buffer, "@database", 9))                                      ctype = IntMimeTypeArray[MT_TX_GUIDE].ContentType;
      else if(!strncmp(buffer, "%PDF-", 5))                                      ctype = IntMimeTypeArray[MT_AP_PDF].ContentType;
      else if(!strncmp(&buffer[2], "-lh5-", 5))                                  ctype = IntMimeTypeArray[MT_AP_LHA].ContentType;
      else if(!strncmp(buffer, "LZX", 3))                                        ctype = IntMimeTypeArray[MT_AP_LZX].ContentType;
      else if(*lbuf >= HUNK_UNIT && *lbuf <= HUNK_INDEX)                         ctype = IntMimeTypeArray[MT_AP_AEXE].ContentType;
      else if(!strncmp(&buffer[6], "JFIF", 4))                                   ctype = IntMimeTypeArray[MT_IM_JPG].ContentType;
      else if(!strncmp(buffer, "GIF8", 4))                                       ctype = IntMimeTypeArray[MT_IM_GIF].ContentType;
      else if(!strncmp(&buffer[1], "PNG", 3))                                    ctype = IntMimeTypeArray[MT_IM_PNG].ContentType;
      else if(!strncmp(&buffer[8], "ILBM", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_IM_ILBM].ContentType;
      else if(!strncmp(&buffer[8], "8SVX", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_AU_8SVX].ContentType;
      else if(!strncmp(&buffer[8], "ANIM", 4) && !strncmp(buffer, "FORM", 4))    ctype = IntMimeTypeArray[MT_VI_ANIM].ContentType;
      else if(strcasestr(buffer, "\nFrom:"))                                     ctype = IntMimeTypeArray[MT_ME_EMAIL].ContentType;
      else
      {
        // now we do a statistical analysis to see if the file
        // is a binary file or not. Because then we use datatypes.library
        // for generating an artificial MIME type.
        int notascii = 0;
        int i;

        for(i=0; i < rlen; i++)
        {
          unsigned char c = buffer[i];

          // see if the current buffer position is
          // considered an ASCII/SPACE char.
          if((c < 32 || c > 127) && !isspace(c))
            notascii++;
        }

        // if the amount of not ASCII chars is lower than rlen/10 we
        // consider it a text file and don't do a deeper analysis.
        if(notascii < rlen/10)
        {
          ULONG prot;

          ObtainFileInfo(fname, FI_PROTECTION, &prot);
          ctype = IntMimeTypeArray[(prot & FIBF_SCRIPT) ? MT_AP_SCRIPT : MT_TX_PLAIN].ContentType;
        }
        else
        {
          D(DBF_MIME, "identifying file through datatypes.library");

          // per default we end up with an "application/octet-stream" content-type
          ctype = IntMimeTypeArray[MT_AP_OCTET].ContentType;

          if(DataTypesBase != NULL)
          {
            BPTR lock;

            if((lock = Lock(fname, ACCESS_READ)))
            {
              struct DataType *dtn;

              if((dtn = ObtainDataTypeA(DTST_FILE, (APTR)lock, NULL)) != NULL)
              {
                const char *type = NULL;
                struct DataTypeHeader *dth = dtn->dtn_Header;

                switch(dth->dth_GroupID)
                {
                  case GID_SYSTEM:     break;
                  case GID_DOCUMENT:   type = "application"; break;
                  case GID_TEXT:       type = "text"; break;
                  case GID_MUSIC:
                  case GID_SOUND:
                  case GID_INSTRUMENT: type = "audio"; break;
                  case GID_PICTURE:    type = "image"; break;
                  case GID_MOVIE:
                  case GID_ANIMATION:  type = "video"; break;
                }

                if(type)
                {
                  static char contentType[SIZE_CTYPE];

                  snprintf(contentType, sizeof(contentType), "%s/x-%s", type, dth->dth_BaseName);
                  ctype = contentType;
                }

                ReleaseDataType(dtn);
              }

              UnLock (lock);
            }
          }
        }
      }
    }
  }

  RETURN(ctype);
  return ctype;
}
///
/// GetRealPath
//  Function that gets the real path out of a supplied path. It will correctly resolve pathes like PROGDIR: aso.
char *GetRealPath(const char *path)
{
  char *realPath;
  BPTR lock;
  BOOL success = FALSE;
  static char buf[SIZE_PATHFILE];

  ENTER();

  // lets try to get a Lock on the supplied path
  if((lock = Lock(path, SHARED_LOCK)))
  {
    // so, if it seems to exists, we get the "real" name out of
    // the lock again.
    if(NameFromLock(lock, buf, sizeof(buf)) != DOSFALSE)
      success = TRUE;

    // And then we unlock the file/dir immediatly again.
    UnLock(lock);
  }

  // only on success we return the realpath.
  realPath = success ? buf : (char *)path;

  RETURN(realPath);
  return realPath;
}

///
/// SyncLaunchCommand
// synchronously launch a DOS command
static LONG SyncLaunchCommand(const char *cmd, ULONG flags, enum OutputDefType outdef)
{
  LONG result = RETURN_FAIL;
  BPTR path;
  BPTR in;
  BPTR out;
  #if defined(__amigaos4__)
  BPTR err;
  BOOL closeErr;
  #endif

  ENTER();
  SHOWSTRING(DBF_UTIL, cmd);

  switch(outdef)
  {
    default:
    case OUT_STDOUT:
    {
      in = Input();
      out = Output();
      #if defined(__amigaos4__)
      err = ErrorOutput();
      closeErr = FALSE;
      #endif
    }
    break;

    case OUT_NIL:
    {
      in = Open("NIL:", MODE_OLDFILE);
      out = Open("NIL:", MODE_NEWFILE);
      #if defined(__amigaos4__)
      err = Open("NIL:", MODE_NEWFILE);
      closeErr = TRUE;
      #endif
    }
    break;

    case OUT_CONSOLE:
    {
      #if defined(__amigaos4__)
      // use a window with scrollback buffer on AmigaOS4.1+
      in = Open("CON:20/20/600/100/YAM thread/AUTO/CLOSE/WAIT/INACTIVE/HISTORY", MODE_NEWFILE);
      #else
      in = Open("CON:20/20/600/100/YAM thread/AUTO/CLOSE/WAIT/INACTIVE", MODE_NEWFILE);
      #endif
      out = ZERO;
      #if defined(__amigaos4__)
      err = ErrorOutput();
      closeErr = FALSE;
      #endif
    }
    break;

    case OUT_REDIRECT:
    {
      in = Open("NIL:", MODE_OLDFILE);
      out = ZERO;
      #if defined(__amigaos4__)
      err = Open("NIL:", MODE_NEWFILE);
      closeErr = TRUE;
      #endif
    }
    break;
  }

  // path may return 0, but that's fine.
  // and we also don't free it manually as this
  // is done by SystemTags/CreateNewProc itself.
  path = ObtainSearchPath();

  result = SystemTags(cmd,
    SYS_Input,    in,
    SYS_Output,   out,
    #if defined(__amigaos4__)
    SYS_Error,    err,
    NP_Child,     TRUE,
    #endif
    SYS_Asynch,   FALSE,
    NP_Name,      "YAM launch command thread",
    NP_Path,      path,
    NP_StackSize, C->StackSize,
    NP_WindowPtr, -1,           // show no requesters at all
    TAG_DONE);

  // enforce success if this is requested
  if(result > RETURN_OK && isFlagSet(flags, LAUNCHF_IGNORE_RC))
  {
    W(DBF_UTIL, "ignoring return code %ld", result);
    result = RETURN_OK;
  }

  if(result != RETURN_OK)
  {
    LONG error = IoErr();
    char fault[SIZE_LARGE];

    // an error occurred as SystemTags should always
    // return zero on success, no matter what.
    E(DBF_UTIL, "execution of '%s' failed, rc=%ld", cmd, result);

    // setup the error message and put it on the application's method stack to
    // let the main thread display it.
    Fault(error, NULL, fault, sizeof(fault));
    ER_NewError(tr(MSG_EXECUTE_COMMAND_FAILED), cmd, error, fault);

    // manually free our search path as SystemTags() shouldn't have freed
    // it itself, but only if the result is equal to -1. All other values
    // stem from the launched command itself and SystemTags() already freed
    // everything.
    if(result == -1)
      ReleaseSearchPath(path);
  }

  // close the channels only if we did open them before
  // we must *NEVER* close the Input()/Output() channels.
  if(outdef != OUT_STDOUT)
  {
    if(out != ZERO)
      Close(out);
    if(in != ZERO)
      Close(in);
    #if defined(__amigaos4__)
    // DOS will not close the error stream for us if we opened it ourself
    if(err != ZERO && closeErr == TRUE)
      Close(err);
    #endif
  }

  RETURN(result);
  return result;
}

///
/// LaunchCommand
//  Executes a DOS command in a separate thread
LONG LaunchCommand(const char *cmd, ULONG flags, enum OutputDefType outdef)
{
  LONG result = RETURN_FAIL;

  ENTER();

  if(isFlagSet(flags, LAUNCHF_ASYNC))
  {
    // make sure we don't recurse
    clearFlag(flags, LAUNCHF_ASYNC);

    // the sub thread's standard I/O channel are different from the main
    // thread's, so we let the subthread open a new console window instead.
    if(outdef == OUT_STDOUT)
      outdef = OUT_CONSOLE;

    // let the thread framework do the dirty work
    result = (DoAction(NULL, TA_LaunchCommand, TT_LaunchCommand_Command, cmd,
                                               TT_LaunchCommand_Flags, flags,
                                               TT_LaunchCommand_Output, outdef,
                                               TAG_DONE) != NULL);
  }
  else
    result = SyncLaunchCommand(cmd, flags, outdef);

  RETURN(result);
  return result;
}

///
/// GotoURLPossible
//  Check whether there is some kind of openurl.library or OS4.1's URL: device available
BOOL GotoURLPossible(void)
{
  BOOL gotoURLPossible = FALSE;
  #if defined(__amigaos4__)
  APTR oldWinPtr;
  struct DevProc *urlDevProc;
  #endif

  ENTER();

  #if defined(__amigaos4__)
  // disable requesters
  oldWinPtr = SetProcWindow((APTR)-1);

  // check whether URL: is mounted, this is OS4.1 only
  if((urlDevProc = GetDeviceProcFlags("URL:", NULL, LDF_DEVICES)) != NULL)
  {
    // yes it is, free the structure again
    FreeDeviceProc(urlDevProc);
    gotoURLPossible = TRUE;
    D(DBF_UTIL, "found URL: device");
  }

  // enable requesters again
  SetProcWindow(oldWinPtr);
  #endif

  // check whether openurl.library is available
  if(gotoURLPossible == FALSE && OpenURLBase != NULL)
  {
    gotoURLPossible = TRUE;
    D(DBF_UTIL, "found openurl.library");
  }

  RETURN(gotoURLPossible);
  return gotoURLPossible;
}

///
/// GotoURL
//  Loads an URL using an ARexx script or openurl.library
BOOL GotoURL(const char *url, const BOOL newWindow)
{
  BOOL wentToURL = FALSE;

  ENTER();

  if(url != NULL)
  {
    // The ARexx macro to open a URL is only possible after the startup phase
    // and if a script has been configured for this purpose.
    if(G != NULL && G->InStartupPhase == FALSE && C->RX[MACRO_URL].Script[0] != '\0')
    {
      char newurl[SIZE_LARGE];

      snprintf(newurl, sizeof(newurl), "\"%s\"", url);
      D(DBF_UTIL, "trying script '%s' to open URL '%s'", C->RX[MACRO_URL].Script, url);
      wentToURL = MA_StartMacro(MACRO_URL, newurl);
    }
    else
    {
      #if defined(__amigaos4__)
      // try URL: device at first
      if(wentToURL == FALSE)
      {
        char newurl[SIZE_LARGE];
        APTR oldWinPtr;
        BPTR urlFH;

        snprintf(newurl, sizeof(newurl), "URL:%s", url);

        // disable requesters
        oldWinPtr = SetProcWindow((APTR)-1);

        D(DBF_UTIL, "trying URL: device to open URL '%s'", url);
        if((urlFH = Open(newurl, MODE_OLDFILE)) != ZERO)
        {
          Close(urlFH);
          wentToURL = TRUE;
        }

        // enable requesters again
        SetProcWindow(oldWinPtr);
      }
      #endif

      if(wentToURL == FALSE && OpenURLBase != NULL)
      {
        // open the URL in a defined web browser and
        // let the user decide himself if he wants to see
        // it popping up in a new window or not (via OpenURL
        // prefs)
        D(DBF_UTIL, "trying openurl.library to open URL '%s'", url);
        wentToURL = URL_Open((STRPTR)url, URL_NewWindow, newWindow,
                                          TAG_DONE);
      }
    }
  }

  RETURN(wentToURL);
  return wentToURL;
}

///
/// SWSSearch
// Smith&Waterman 1981 extended string similarity search algorithm
// X, Y are the two strings that will be compared for similarity
// It will return a pattern which will reflect the similarity of str1 and str2
// in a Amiga suitable format. This is case-insensitive !
char *SWSSearch(const char *str1, const char *str2)
{
  char *similar;
  static char *Z = NULL;    // the destination string (result)
  int **L        = NULL;    // L matrix
  int **Ind      = NULL;    // Indicator matrix
  char *X;                  // 1.string X
  char *Y        = NULL;    // 2.string Y
  int lx;                   // length of X
  int ly;                   // length of Y
  int lz;                   // length of Z (maximum)
  int i, j;
  BOOL gap = FALSE;
  BOOL success = FALSE;

  // special enum for the Indicator
  enum  IndType { DELX=1, DELY, DONE, TAKEBOTH };

  ENTER();

  // by calling this function with (NULL, NULL) someone wants
  // to signal us to free the destination string
  if(str1 == NULL || str2 == NULL)
  {
    free(Z);
    Z = NULL;

    RETURN(NULL);
    return NULL;
  }

  // calculate the length of our buffers we need
  lx = strlen(str1)+1;
  ly = strlen(str2)+1;
  lz = MAX(lx, ly)*3+3;

  // first allocate all resources
  if(!(X   = calloc(lx+1, sizeof(char)))) goto abort;
  if(!(Y   = calloc(ly+1, sizeof(char)))) goto abort;

  // now we have to alloc our help matrixes
  if(!(L   = calloc(lx,   sizeof(int))))  goto abort;
  if(!(Ind = calloc(lx,   sizeof(int))))  goto abort;
  for(i = 0; i < lx; i++)
  {
    if(!(L[i]   = calloc(ly, sizeof(int)))) goto abort;
    if(!(Ind[i] = calloc(ly, sizeof(int)))) goto abort;
  }

  // and allocate the result string separately
  free(Z);
  if(!(Z = calloc(lz, sizeof(char)))) goto abort;

  // we copy str1&str2 into X and Y but have to copy a placeholder in front of them
  memcpy(&X[1], str1, lx);
  memcpy(&Y[1], str2, ly);

  for(i = 1; i < lx; i++)
    Ind[i][0] = DELX;

  for(j = 1; j < ly; j++)
    Ind[0][j] = DELY;

  Ind[0][0] = DONE;

  // Now we calculate the L matrix
  // this is the first step of the SW algorithm
  for(i = 1; i < lx; i++)
  {
    for(j = 1; j < ly; j++)
    {
      if(toupper(X[i]) == toupper(Y[j]))  // case insensitive version
      {
        L[i][j] = L[i-1][j-1] + 1;
        Ind[i][j] = TAKEBOTH;
      }
      else
      {
        if(L[i-1][j] > L[i][j-1])
        {
          L[i][j] = L[i-1][j];
          Ind[i][j] = DELX;
        }
        else
        {
          L[i][j] = L[i][j-1];
          Ind[i][j] = DELY;
        }
      }
    }
  }

#ifdef DEBUG
  // for debugging only
  // This will print out the L & Ind matrix to identify problems
/*
  printf(" ");
  for(j=0; j < ly; j++)
  {
    printf(" %c", Y[j]);
  }
  printf("\n");

  for(i=0; i < lx; i++)
  {
    printf("%c ", X[i]);

    for(j=0; j < ly; j++)
    {
      printf("%d", L[i][j]);
      if(Ind[i][j] == TAKEBOTH)  printf("'");
      else if(Ind[i][j] == DELX) printf("^");
      else if(Ind[i][j] == DELY) printf("<");
      else printf("*");
    }
    printf("\n");
  }
*/
#endif

  // the second step of the SW algorithm where we
  // process the Ind matrix which represents which
  // char we take and which we delete

  Z[--lz] = '\0';
  i = lx-1;
  j = ly-1;

  while(i >= 0 && j >= 0 && Ind[i][j] != DONE)
  {
    if(Ind[i][j] == TAKEBOTH)
    {
      Z[--lz] = X[i];

      i--;
      j--;
      gap = FALSE;
    }
    else if(Ind[i][j] == DELX)
    {
      if(!gap)
      {
        if(j > 0)
        {
          Z[--lz] = '?';
          Z[--lz] = '#';
        }
        gap = TRUE;
      }
      i--;
    }
    else if(Ind[i][j] == DELY)
    {
      if(!gap)
      {
        if(i > 0)
        {
          Z[--lz] = '?';
          Z[--lz] = '#';
        }
        gap = TRUE;
      }
      j--;
    }
  }

  success = TRUE;

abort:

  // now we free our temporary buffers now
  free(X);
  free(Y);

  // lets free our help matrixes
  if(L != NULL)
  {
    for(i = 0; i < lx; i++)
    {
      free(L[i]);
    }
    free(L);
  }
  if(Ind != NULL)
  {
    for(i = 0; i < lx; i++)
    {
      free(Ind[i]);
    }
    free(Ind);
  }

  similar = success ? &(Z[lz]) : NULL;

  RETURN(similar);
  return similar;
}
///
/// CRC32
//  Function that calculates a 32bit crc checksum for a provided buffer.
//  See http://www.4d.com/ACIDOC/CMU/CMU79909.HTM for more information about
//  the CRC32 algorithm.
//  This implementation allows the usage of more than one persistant calls of
//  the crc32 function. This allows to calculate a valid crc32 checksum over
//  an unlimited amount of buffers.
ULONG CRC32(const void *buffer, unsigned int count, ULONG crc)
{
  /* table generated with the following code:
   *
   * #define CRC32_POLYNOMIAL 0xEDB88320L
   *
   * int i, j;
   *
   * for (i = 0; i <= 255; i++) {
   *   unsigned long crc = i;
   *   for (j = 8; j > 0; j--) {
   *     if (crc & 1)
   *       crc = (crc >> 1) ^ CRC32_POLYNOMIAL;
   *     else
   *       crc >>= 1;
   *   }
   *   CRCTable[i] = crc;
   * }
   */
  static const unsigned long CRCTable[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
  };
  const unsigned char *p = (const unsigned char *)buffer;

  ENTER();

  // we calculate the crc32 now.
  while(count-- != 0)
  {
    ULONG temp1 = (crc >> 8) & 0x00FFFFFFL;
    ULONG temp2 = CRCTable[((int)crc ^ *p++) & 0xFF];

    crc = temp1 ^ temp2;
  }

  RETURN(crc);
  return crc;
}

///
/// strippedCharsetName()
// return the charset code stripped and without any white spaces
char *strippedCharsetName(const struct codeset* codeset)
{
  char *strStart = TrimStart(codeset->name);
  char *strEnd = strchr(strStart, ' ');

  if(strEnd > strStart || strStart > codeset->name)
  {
    static char strippedName[SIZE_CTYPE+1];

    if(strEnd > strStart && (size_t)(strEnd-strStart) < sizeof(strippedName))
      strlcpy(strippedName, strStart, strEnd-strStart+1);
    else
      strlcpy(strippedName, strStart, sizeof(strippedName));

    return strippedName;
  }
  else
    return codeset->name;
}

///
/// GetPubScreenName
// return the name of a public screen, if the screen is public
void GetPubScreenName(const struct Screen *screen, char *pubName, ULONG pubNameSize)
{
  ENTER();

  // we use "Workbench" as the default public screen name
  strlcpy(pubName, "Workbench", pubNameSize);

  if(screen != NULL)
  {
    // try to get the public screen name
    #if defined(__amigaos4__)
    // this very handy function is OS4 only
    if(GetScreenAttr((struct Screen *)screen, SA_PubName, pubName, pubNameSize) == 0)
    {
      // GetScreenAttr() failed, copy the default name again, in case it was changed
      strlcpy(pubName, "Workbench", pubNameSize);
    }
    #else
    struct List *pubScreenList;

    // on all other systems we have to obtain the public screen name in the hard way
    // first get the list of all public screens
    if((pubScreenList = LockPubScreenList()) != NULL)
    {
      struct Node *curNode;

      // then iterate through this list
      IterateList(pubScreenList, curNode)
      {
        struct PubScreenNode *psn = (struct PubScreenNode *)curNode;

        // check if we found the given screen
        if(psn->psn_Screen == screen)
        {
          // copy the name and get out of the loop
          strlcpy(pubName, psn->psn_Node.ln_Name, pubNameSize);
          break;
        }
      }

      // unlock the list again
      UnlockPubScreenList();
    }
    #endif
  }

  LEAVE();
}

///
/// TimeHasElapsed
// check wheter the given number of microsecond has passed since the last
// check specified by <last>. If yes, then <last> will be set to the current
// time
BOOL TimeHasElapsed(struct TimeVal *last, ULONG micros)
{
  struct TimeVal now;
  struct TimeVal delta;
  BOOL elapsed = FALSE;

  ENTER();

  // get the current time
  GetSysTime(TIMEVAL(&now));
  delta = now;

  // substract the known last time
  SubTime(TIMEVAL(&delta), TIMEVAL(last));

  // check whether either one second or the number of microseconds has passed
  if(delta.Seconds > 0 || delta.Microseconds > micros)
  {
    // remember the current time and signal success
    *last = now;
    elapsed = TRUE;
  }

  RETURN(elapsed);
  return elapsed;
}

///

/*** REXX interface support ***/
/// AllocReqText
//  Prepare multi-line text for requesters, converts \n to line breaks
char *AllocReqText(const char *s)
{
  char *reqtext;

  ENTER();

  if((reqtext = calloc(strlen(s) + 1, 1)) != NULL)
  {
    char *d = reqtext;

    while(*s != '\0')
    {
      if(s[0] == '\\' && s[1] == 'n')
      {
        *d++ = '\n';
        s++;
        s++;
      }
      else
        *d++ = *s++;
    }
  }

  RETURN(reqtext);
  return reqtext;
}

///
/// ToLowerCase
//  Change a complete string to lower case
void ToLowerCase(char *str)
{
  char c;

  ENTER();

  while ((c = *str) != '\0')
    *str++ = tolower(c);

  LEAVE();
}

///
/// WriteUInt32
//  write a 32bit variable to a stream, big endian style
int WriteUInt32(FILE *stream, ULONG value)
{
  int n;

  ENTER();

  // convert the value to big endian style
  value = htonl(value);

  n = fwrite(&value, sizeof(value), 1, stream);

  RETURN(n);
  return n;
}

///
/// ReadUInt32
//  read a 32bit variable from a stream, big endian style
int ReadUInt32(FILE *stream, ULONG *value)
{
  int n;

  ENTER();

  if((n = fread(value, sizeof(*value), 1, stream)) == 1)
  {
    // convert the value to big endian style
    *value = ntohl(*value);
  }

  RETURN(n);
  return n;
}

///
/// DuplicateNode
// create a copy of an existing node using AllocSysObject()
void *DuplicateNode(const void *node, const size_t size)
{
  void *dup;

  ENTER();

  if((dup = AllocSysObjectTags(ASOT_NODE,
    ASONODE_Size, size,
    ASONODE_Min, TRUE,
    TAG_DONE)) != NULL)
  {
    memcpy(dup, node, size);
  }

  RETURN(dup);
  return dup;
}

///
/// CountNodes
//  returns the number of nodes currently in a struct List
int CountNodes(const struct MinList *list)
{
  struct Node *curNode;
  int result = 0;

  ENTER();

  IterateList(list, curNode)
    result++;

  RETURN(result);
  return result;
}

///
/// CompareLists
// compare two lists using a comparison function
BOOL CompareLists(const struct List *lh1, const struct List *lh2, BOOL (* compare)(const struct Node *, const struct Node *))
{
  BOOL equal = TRUE;
  struct Node *ln1;
  struct Node *ln2;

  ENTER();

  ln1 = GetHead((struct List *)lh1);
  ln2 = GetHead((struct List *)lh2);

  // walk through both lists in parallel and compare the single nodes
  while(ln1 != NULL && ln2 != NULL)
  {
    // compare the two nodes
    if(compare(ln1, ln2) == FALSE)
    {
      // something does not match
      equal = FALSE;
      break;
    }

    // advance to the next nodes in each list
    ln1 = GetSucc(ln1);
    ln2 = GetSucc(ln2);
  }

  if(equal == TRUE)
  {
    // if both lists are equal so far, but there are any nodes left in either list
    // then the two lists cannot be equal
    if(ln1 != NULL || ln2 != NULL)
      equal = FALSE;
  }

  RETURN(equal);
  return equal;
}

///
/// SortNListToExecList
// successively obtains the elements of an NList object and reorders them
// with in an Exec list accordingly
void SortNListToExecList(Object *nList, struct MinList *execList)
{
  int i;

  ENTER();

  // as the user may have changed the order of the entries in the NList object
  // we have to make sure the order in the NList fits to the Exec list's order
  i = 0;
  do
  {
    struct Node *node = NULL;

    DoMethod(nList, MUIM_NList_GetEntry, i, &node);
    if(node == NULL)
      break;

    // for resorting the exec list we just have to remove that particular entry
    // and add it to the tail - all other operations like adding/removing should
    // have been done by others already - so this is just resorting
    Remove(node);
    AddTail((struct List *)execList, node);

    i++;
  }
  while(TRUE);

  LEAVE();
}

///
/// GetHostName
// retrieve the hostname of the system YAM is currently running on for things
// like SMTP authentification and so on
int GetHostName(char *name, size_t namelen)
{
  int result = -1;
  struct Connection *conn;

  ENTER();

  // we create a temporary connection structure
  // which is required to call GetFQDN()
  if((conn = CreateConnection(TRUE)) != NULL)
  {
    // call GetFQDN() to get a full qualified domain name (either valid hostname or
    // [x.x.x.x] encoded string with IP.
    result = GetFQDN(conn, name, namelen);

    DeleteConnection(conn);
  }

  RETURN(result);
  return result;
}

///
/// FreeStrArray
// free() a NULL terminated array of strings
void FreeStrArray(char **array)
{
  ENTER();

  if(array != NULL)
  {
    int i=0;
    char *p;

    do
    {
      if((p = array[i]) == NULL)
        break;

      free(p);
      i++;
    }
    while(1);

    free(array);
  }

  LEAVE();
}

///

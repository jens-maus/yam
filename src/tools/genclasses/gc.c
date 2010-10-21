/***************************************************************************

 GenClasses - MUI class dispatcher generator
 Copyright (C) 2001 by Andrew Bell <mechanismx@lineone.net>

 Contributed to the YAM Open Source Team as a special version
 Copyright (C) 2001-2010 by YAM Open Source Team

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

 $Id$

 TODO
    - Handle object creation order.
    - Improve tag value generation.
    - Generation of desciptive class tree in header comment.
    - DECLARE param parser should allow normal C style comments to be used
      too.

***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <string.h>
#include "gc.h"
#include "lists.h"
#include "crc32.h"

#if defined(__SASC)
/* SAS/C does not know snprintf(), hence we redirect it back to sprintf() */
size_t snprintf(char *s, size_t len, const char *f, ...)
{
  va_list args;
  size_t result;

  va_start(args, f);
  result = vsprintf(s, f, args);
  va_end(args);

  return result;
}
#endif

/*
 * When compiling for YAM under VBCC use:
 *
 * GenClasses <classespath> -bYAM -gpl -i"/YAM.h" -mkfilevmakefile,vc,-o,-+,-c*
 *
 *
 * History
 * -------
 * 0.34 - multiple dependencies from a private class were not handled correctly
 *        and rejected as a dependency loop. Additionally the parent's class name
 *        will now be included in the error message.
 * 0.33 - the .crc file is now written on every run, even if the checksum did not
 *        change. This solves some dependency issues in YAM's Makefile.
 * 0.32 - CRC checksums are now used to check whether any class definition has
 *        changed. Without any public change the sources will not be regenerated.
 * 0.31 - classes without any instance data don't need to pecify a Data structure
 *        with an unused dummy variable anymore.
 * 0.30 - the classes and their code are sorted alphabetically now.
 * 0.29 - added support for private classes being subclassed from other private
 *        classes.
 *
 * 0.28 - changed to use the new SDISPATCHER macros instead which are generating
 *        static dispatcher functions instead of global ones.
 *
 * 0.27 - added warning in case a BOOL parameter type is used in a DECLARE()
 *        statement as this type is known to case trouble.
 *
 * 0.26 - fixed a bug in the list_findname() function where after the last fix
 *        the function didn't correctly find a named node. In addition, a new "-q"
 *        option now prevents all standard output as only fprintf(stderr,...)
 *        will be shown upon execution
 *
 * 0.25 - fixed a memory leak due to a wrong linked list implementation. There
 *        was always one node left in a list upon calling list_remhead() until
 *        it returned NULL. Also added a sort functions for linked lists.
 *
 * 0.24 - compilable by SAS/C again. The missing snprintf() is redirected to
 *        sprintf().
 *
 * 0.23 - use new DISPATCHER() macro instead of DISPATCHERPROTO()
 *
 * 0.22 - removed unncesessary STDARGS uses
 *
 * 0.21 - raised the warning levels to -pedantic and fixed some compilation
 *        warnings triggers by some oddities in our code.
 *
 * 0.20 - modified generated code to be somewhat more compatible to compiling
 *        with -Wwrite-strings
 *
 * 0.19 - replaced all sprintf() uses by proper snprintf() ones.
 *
 * 0.18 - minor cosmetic changes to the generated source layout
 *
 * 0.17 - removed all 'long' datatype uses as this generates tag values that
 *        are larger than 32bit and therefore make us run into problems.
 *        Also modified the whole tagvalue generation code to just generate
 *        tag values that are between TAG_USER (0x80000000) and the MUI's
 *        one (0x80010000) because otherwise we might run into problems.
 *
 * 0.16 - exported text is now placed before the generated MUIP_ structures
 *        because it may contain definition which are required there.
 *
 * 0.15 - changed the variable argument definition to use the new macros from
 *        SDI_stdarg.h
 *
 * 0.14 - removed the special OS4 ASMSTUB handling from the generated code again
 *        as the 51.4 kernel of AmigaOS4 seem to handle m68k<>PPC cross calls
 *        correctly now.
 *
 * 0.13 - added UNUSED define specification to function prototype macros of
 *        each class. This prevents dozen of warnings because of not used
 *        parameters.
 *
 * 0.12 - added special AmigaOS4 only dependent muiDispatcherEntry wrapper
 *        dispatcher call. This should be only temporarly here and will be
 *        removed as soon as AmigaOS4 allows full PPC->68k cross hook calls.
 *
 * 0.11 - added AmigaOS4 support to the vararg _NewObject() function.
 *
 * 0.10 - fixed CleanupClasses to deal with NULL classes
 *
 * 0.9 - fixed crash in processclasssrc()
 *
 * 0.8 - added MORPHOS support for the vararg _NewObject() function.
 *       added STDARGS to _NewObject() function.
 *
 * 0.7 - added cast to CreateCustomClass call to get rid of #120 Warnings
 *
 * 0.6 - generated headernames now end with _cl.h to avoid name clashes
 *
 * 0.5 - fixed mass of enforcer hits
 *     - fixed serious bug in list handling
 *
 * 0.4 - fixed bug in extra header inclusion and makefile argument
 *     - the collision array of the hash function will be alloced dynamically
 *       so that we still can compile genclasses with near data & code.
 *     - updated TAG_ID to use 0x00430000 and a BUCKET_CNT of 98304
 *     - modified GPL header to reflect standard YAM source header
 *
 * 0.3 - List counter wasn't being set to 0 on init
 *       resulting in some large trashy values. :-)
 *     - Some prototypes with MUIP_xxxx weren't saved
 *       correctly.
 *     - makefile is marked as do not edit.
 *     - Doesn't use join #?.o in makefile anymore.
 *     - New keyword called CLASSDATA.
 *     - xxxxGetSize() function is now saved in header.
 *     - Uses strstr() to extract SuperClass and Description.
 *       This allows for the keywords to be embedded in comments.
 *     - Misc code changes.
 *
 * 0.2 - Added error reports to all fopen() calls.
 *     - Added myaddpart(), mypathpart() and
 *       myfilepart(). All path related problems should
 *       be gone now.
 *     - Now checks for .c extensions at the end of the
 *       filename instead of using strstr().
 *     - No longer casts the msg argument.
 *     - Source files without a Superclass: keyword are
 *       skipped.
 *     - Unterminated EXPORT block warning now gives
 *       correct line number in warning.
 *     - Classes.h now generates the prototypes for the
 *       OVERLOADS and DECLARES.
 *     - makefile now produces all.o target.
 *     - Implemented free_classdef().
 *     - Implemented linked-list routines.
 *     - Removed some globals.
 *     - Added generation of class headers.
 *     - New tag value generation.
 *     - Using long instead of int now.
 *     - exlineno was defined as char, oops.
 *
 * 0.1 - Initial version, given to Allan and Jens.
 *
 */

static const char * const verstr = "0.34";

/* Every shitty hack wouldn't be complete without some shitty globals... */

char  arg_classdir          [256];
char  arg_basename          [256];
char  arg_includes          [256];
char  arg_mkfile            [256];
char  arg_mkfile_dest       [256];
char *arg_mkfile_cc       = NULL;
char *arg_mkfile_outarg   = NULL;
char *arg_mkfile_ccopts   = NULL;
int   arg_gpl             = 0;     /* booleans */
int   arg_storm           = 0;
int   arg_v               = 0;
int   arg_q               = 0;

/*******************************************************************************
 *
 * Emulation of opendir()/closedir()/readdir() on AmigaOS
 *
 * Only include this if your compiler doesn't support the <dirent.h> routines.
 *
 *******************************************************************************
 *
 *
 */

#ifdef EMULATE_DIRENT

#include <proto/dos.h>
#include <exec/types.h>
#include <dos/dosextens.h>

typedef struct dirent
{
  char *d_name;
  BPTR lock;
  struct FileInfoBlock *fib;
} DIR;

void closedir( DIR *de )
{
  if (!de) return;
  if (de->lock) UnLock(de->lock);
  if (de->fib) FreeDosObject(DOS_FIB, de->fib);
  free(de);
}

DIR *opendir( char *name )
{
  DIR *de;
  if (!(de = calloc(1, sizeof(struct dirent)))) return NULL;
  if ((de->fib = AllocDosObject(DOS_FIB, NULL)) &&
    (de->lock = Lock(name, SHARED_LOCK)) &&
    (Examine(de->lock, de->fib)) && (de->fib->fib_DirEntryType > 0))
  {
    de->d_name = de->fib->fib_FileName;
  }
  else
  {
    closedir(de); de = NULL;
  }
  return de;
}

DIR *readdir( DIR *de )
{
  if (!de) return NULL;
  return ExNext(de->lock, de->fib) ? de : NULL;
}

#else
#include <dirent.h>
#ifndef TAG_USER
#define TAG_USER ((unsigned int)(1UL<<31))
#endif
#endif /* EMULATE_DIRENT */


/*******************************************************************************
 *
 * Assorted routines
 *
 *******************************************************************************
 *
 *
 */

char *skipwhitespaces( char *p ) /* Note: isspace() sucks... */
{
  unsigned char c;

  while ((c = *p))
  {
    if (c == '\t' || c == '\r' || c == '\n' || c == ' ' || c == 0xa0)
    {
      ++p;
    }
    else break;
  }

  return p;
}

char *stralloc( char *str )
{
  /* Copy str to buffer and strip leading and trailing white spaces, LFs, CRs, etc. */
  char *strvec, *estr;
  unsigned c;
  if (!str) str = "";
  if (!(strvec = calloc(1, strlen(str) + 1))) return NULL;
  if (*str == 0) return strvec; /* Empty string? */
  while ((c = *str)) if (c == '\t' || c == '\r' || c == '\n' || c == ' ' || c == 0xa0)
    ++str; else break;
  if (*str == 0) return strvec; /* Empty string still? */
  estr = str + (strlen(str) - 1);
  while (estr > str)
  {
    c = *estr;
    if (c == '\t' || c == '\r' || c == '\n' || c == ' ' || c == 0xa0) --estr; else break;
  }
  memcpy(strvec, str, (size_t)(estr - str) + 1);
  return strvec;
}

int myaddpart( char *path, char *name, size_t len )
{
  /* A version of dos.library/AddPart() that should
     work on both UNIX and Amiga systems. */
  char c;
  c = path[strlen(path) - 1];
  if (c != ':' && c != '/') strncat(path, "/", len);
  strncat(path, name, len);
  return 1;
}

char *mypathpart( char *path )
{
  char *p;
  if (!path) return NULL;
  if ((p = strrchr(path, '/'))) return p;
  if ((p = strrchr(path, ':'))) return ++p;
  return path;
}

char *myfilepart( char *path )
{
  char *p;
  if (!path) return NULL;
  if ((p = strrchr(path, '/'))) return ++p;
  if ((p = strrchr(path, ':'))) return ++p;
  return path;
}

/*******************************************************************************
 *
 * Tag value generation
 *
 *******************************************************************************
 *
 *
 */

int *collision_cnts;

static unsigned int gethash( char *str )
{
    static unsigned int primes[10] = { 3, 5, 7, 11, 13, 17, 19, 23, 27, 31 };
    unsigned int i = 0;
    unsigned int hash = strlen(str);
    while (*str)
    {
        hash *= 13;
        hash += (*str++) * primes[(++i) % 10];
    }

    return hash % BUCKET_CNT;
}

static unsigned int gettagvalue(char *tag, int checkcol)
{
  unsigned int hash = gethash(tag);
  unsigned int val = TAG_USER + (hash << 8) + collision_cnts[hash];

  /* now we check that val is definitly between TAG_USER and the MUI tag base
     because anything above/below that might cause problems. */
  if(val < TAG_USER || val >= 0x80010000)
    fprintf(stderr, "WARNING: generate tag value '%x' of class '%s' collides with BOOPSI/MUI tag values!\n", val, tag);

  /* if we should check for a collision we do so */
  if(checkcol)
    ++collision_cnts[hash];

  if(arg_v)
    fprintf(stdout, "Assigning tag %-35s with value %x\n", tag, val);

  return val;
}

/*******************************************************************************
 *
 * Parsing routines
 *
 *******************************************************************************
 *
 *
 */

void free_overload( struct overloaddef *od )
{
  if (!od) return;
  free(od->name);
}

void free_declare( struct declaredef *dd )
{
  if (!dd) return;
  free(dd->name);
  free(dd->params);
}

void free_exportblk( struct exportdef *ed )
{
  if (!ed) return;
  free(ed->exporttext);
}

void free_attr( struct attrdef *ad )
{
  if (!ad) return;
  free(ad->name);
}

void free_classdef( struct classdef *cd )
{
  struct node *n;
  if (!cd) return;
  free(cd->classdata);

  while((n = list_remhead(&cd->overloadlist)))
  {
    free_overload(n->data);
    free(n);
  }

  while((n = list_remhead(&cd->declarelist)))
  {
    free_declare(n->data);
    free(n);
  }

  while((n = list_remhead(&cd->attrlist)))
  {
    free_attr(n->data);
    free(n);
  }

  while((n = list_remhead(&cd->exportlist)))
  {
    free_exportblk(n->data);
    free(n);
  }

  free(cd);
}


void add_overload( struct classdef *cd, char *name )
{
  struct overloaddef *od;
  if (!(od = (struct overloaddef *) calloc(1, sizeof(struct overloaddef)))) return;
  if (!(od->name = stralloc(name))) return;
  list_saveitem(&cd->overloadlist, od->name, od);
}

void add_declare( struct classdef *cd, char *name, char *params )
{
  struct declaredef *dd;
  if (!(dd = (struct declaredef *) calloc(1, sizeof(struct declaredef)))) return;
  dd->name = stralloc(name);
  dd->params = stralloc(params);
  if (!dd->name || !dd->params)
  {
    free_declare(dd); return;
  }
  list_saveitem(&cd->declarelist, dd->name, dd);
}

void add_exportblk( struct classdef *cd, char *textblk )
{
  struct exportdef *ed;
  if (!(ed = (struct exportdef *) calloc(1, sizeof(struct exportdef)))) return;
  if (!(ed->exporttext = stralloc(textblk))) return;
  list_saveitem(&cd->exportlist, ed->exporttext, ed);
}

void add_attr( struct classdef *cd, char *name )
{
  struct attrdef *ad;
  if (!(ad = (struct attrdef *) calloc(1, sizeof(struct attrdef)))) return;
  if (!(ad->name = stralloc(name))) return;
  if (list_findname(&cd->attrlist, name))
  {
    if(arg_v)
      fprintf(stdout, "ATTR '%s' already collected, skipped.\n", name);

    free_attr(ad);
    return;
  }
  list_saveitem(&cd->attrlist, ad->name, ad);
}

struct classdef *processclasssrc( char *path )
{
  FILE *fp;
  int lineno = 0;
  char line[256], *p, *ob, *cb, *sub;
  struct classdef *cd;
  int spos, epos = 0, exlineno = lineno;
  char *blk = NULL;

  if (!(cd = calloc(1, sizeof(struct classdef)))) return NULL;
  if (!(fp = fopen(path, "r")))
  {
    fprintf(stderr, "ERROR: Unable to open %s\n", path);
    free(cd);

    return NULL;
  }
  list_init(&cd->overloadlist);
  list_init(&cd->declarelist);
  list_init(&cd->attrlist);
  list_init(&cd->exportlist);

  if ((cd->name = stralloc(myfilepart(path))))
  {
    if ((p = strrchr(cd->name, '.'))) *p = 0;
  }

  while (fgets(line, 255, fp))
  {
    lineno++;
    p = skipwhitespaces(line);
    if (!*p) continue;

    /*printf("line number = %ld [%s]\n", lineno, p);*/
    ob = cb = NULL;
    /******** Scan line for keywords and extract the associated information... ********/
    if (!cd->superclass && (sub = strstr(p, KEYWD_SUPERCLASS)))
    {
      if(arg_v)
        fprintf(stdout, KEYWD_SUPERCLASS " keyword found at line %d in file %s\n", lineno, path);

      sub += sizeof(KEYWD_SUPERCLASS) - 1;
      cd->superclass = stralloc(skipwhitespaces(sub));
      /* assume public superclasses first */
      cd->index = -1;
    }
    else if (!cd->desc && (sub = strstr(p, KEYWD_DESC)))
    {
      if(arg_v)
        fprintf(stdout, KEYWD_DESC " keyword found at line %d\n", lineno);

      sub += sizeof(KEYWD_DESC) - 1;
      cd->desc = stralloc(skipwhitespaces(sub));
    }
    else if (!cd->classdata && strstr(line, KEYWD_CLASSDATA))
    {
      if(arg_v)
        fprintf(stdout, KEYWD_CLASSDATA " keyword found at line %d\n", lineno);

      spos = ftell(fp);
      while(fgets(p = line, 255, fp))
      {
        lineno++; epos = ftell(fp);
        if (!(p = strstr(line, "*/"))) continue;
        epos += (p - line) - 3; /* + offset into line... */
        fseek(fp, spos, SEEK_SET);
        if (!(blk = calloc(1, (size_t)(epos - spos + 1))))
        {
          fprintf(stderr, "WARNING: Cannot read " KEYWD_CLASSDATA " block at line %d, out of memory!\n", exlineno); break;
        }
        if (fread(blk, (size_t)(epos - spos), 1, fp) != 1)
        {
          fprintf(stderr, "WARNING: Cannot read " KEYWD_CLASSDATA " block at line %d, fread() fails!\n", exlineno); break;
        }
        if ((ob = strchr(blk, '{')))
        {
          if (!(cb = strchr(ob + 1, '}'))) cb = blk + strlen(blk);
          *cb = 0;
          if ((cd->classdata = calloc(1, strlen(++ob) + 1)))
            strcpy(cd->classdata, ob);
        }
        else if ((cd->classdata = calloc(1, strlen(blk) + 1)))
          strcpy(cd->classdata, blk);

        free(blk);
        break;
      }
    }
    else if (strncmp(KEYWD_OVERLOAD, p, sizeof(KEYWD_OVERLOAD) - 1) == 0)
    {
      if(arg_v)
        fprintf(stdout, KEYWD_OVERLOAD " keyword found at line %d in file %s\n", lineno, path);

      p += sizeof(KEYWD_OVERLOAD) - 1;
      if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
      if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
      *cb = 0; add_overload(cd, ++ob);
    }
    else if (strncmp(KEYWD_DECLARE, p, sizeof(KEYWD_DECLARE) - 1) == 0)
    {
      if(arg_v)
        fprintf(stdout, KEYWD_DECLARE " keyword found at line %d in file %s\n", lineno, path);

      p += sizeof(KEYWD_DECLARE) - 1;
      if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
      if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
      if ((p = strstr(cb + 1, "//"))) p += 2;
      *cb = 0; add_declare(cd, ++ob, p);

      /* we check for the existance of a "BOOL" as a parameter
         and warn the user accordingly because BOOL is known to be unsafe */
      if(p != NULL && strstr(p, "BOOL ") != NULL)
        fprintf(stderr, "WARNING: " KEYWD_DECLARE "() - BOOL parameter type is unsafe at line %d in file %s\n", lineno, path);
    }
    else if ((sub = strstr(p, KEYWD_ATTR)) != NULL)
    {
      if(arg_v)
        fprintf(stdout, KEYWD_ATTR " keyword found at line %d in file %s\n", lineno, path);

      p = sub + sizeof(KEYWD_ATTR) - 1;
      if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
      if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
      *cb = 0; add_attr(cd, ++ob);
    }
    else if (strncmp("/*", p, 2) == 0) /* TODO: Use strstr() here, like CLASSDATA */
    {
      p = skipwhitespaces(p + 2);
      if (strncmp(KEYWD_EXPORT, p, sizeof(KEYWD_EXPORT) - 1) == 0)
      {
        if(arg_v)
          fprintf(stdout, KEYWD_EXPORT " keyword found at line %d in file %s\n", lineno, path);

        p += sizeof(KEYWD_EXPORT) - 1;
        spos = ftell(fp);
        while(fgets(p = line, 255, fp))
        {
          lineno++; epos = ftell(fp);
          if (!(p = strstr(line, "*/"))) continue;
          epos += (p - line) - 3; /* + offset into line... */
          fseek(fp, spos, SEEK_SET);
          if (!(blk = calloc(1, (size_t)(epos - spos + 1))))
          {
            fprintf(stderr, "WARNING: Cannot read " KEYWD_EXPORT " block at line %d, out of memory!\n", exlineno); break;
          }
          if (fread(blk, (size_t)(epos - spos), 1, fp) != 1)
          {
            fprintf(stderr, "WARNING: Cannot read " KEYWD_EXPORT " block at line %d, fread fails!\n", exlineno); break;
          }
          add_exportblk(cd, blk);
          free(blk);
          break;
        }

        if(epos == 0)
          fprintf(stderr, "WARNING: Unterminated EXPORT block at line %d\n", lineno);
      }
    }
  }  /* while() */

  /* a superclass is always required */
  if(cd->superclass == NULL)
  {
    fprintf(stderr, "WARNING: Source file '%s' doesn't contain a " KEYWD_SUPERCLASS " keyword. Skipping.\n", path);
    free_classdef(cd);
    cd = NULL;
  }
  /* instance data per object are not always required and
   * specifying a dummy structure only consumes memory
   */
/*
  else if(cd->classdata == NULL)
  {
    fprintf(stderr, "WARNING: Source file '%s' doesn't contain a " KEYWD_CLASSDATA " keyword. Skipping.\n", path);
    free_classdef(cd);
    cd = NULL;
  }
*/
  fclose(fp);
  return cd;
}

void printclasslist( struct list *classlist )
{
  struct classdef *cd;
  struct overloaddef *od;
  struct declaredef *dd;
  struct attrdef *ad;
  struct node *n, *nn;

  fprintf(stdout, "The following keywords were extracted:\n");

  for(nn = NULL; (nn = list_getnext(classlist, nn, (void **)&cd)); )
  {
    fprintf(stdout, "CLASS: %s\n", cd->name);

    for(n = NULL; (n = list_getnext(&cd->overloadlist, n, (void **) &od)); )
      fprintf(stdout, "  OVERLOAD: %s\n", od->name);

    for(n = NULL; (n = list_getnext(&cd->declarelist, n, (void **) &dd)); )
      fprintf(stdout, "   DECLARE: %s\n", dd->name);

    for(n = NULL; (n = list_getnext(&cd->attrlist, n, (void **) &ad)); )
      fprintf(stdout, "      ATTR: %s\n", ad->name);
  }
}

int compareclasses(const struct node *n1, const struct node *n2)
{
  struct classdef *cd1 = n1->data;
  struct classdef *cd2 = n2->data;

  return strcasecmp(cd1->name, cd2->name);
}

int scanclasses( char *dirname, struct list *classlist )
{
  DIR *dir;
  struct dirent *de;
  char *n, dirbuf[256];
  int len, srccnt = 0;
  struct classdef *cd;
  strncpy(dirbuf, dirname, 255);

  if(arg_v)
    fprintf(stdout, "scanning classes dir '%s'\n", dirbuf);

  if (!(dir = opendir(dirname)))
  {
    fprintf(stderr, "ERROR: Unable to open directory %s\n", dirname);

    return 0;
  }

  while ((de = readdir(dir)))
  {
    n = de->d_name; len = strlen(n);
    if (len < 2) continue;
    if ((n[len - 2] != '.') || (tolower(n[len - 1]) != 'c'))
    {
      if(arg_v)
        fprintf(stdout, "Skipping: %s\n", n);

      continue;
    }
    if (!strcmp(SOURCE_NAME, n) || !strcmp(HEADER_NAME, n)) continue;
    ++srccnt;
    myaddpart(dirbuf, de->d_name, 255);

    if(arg_v)
      fprintf(stdout, "processing: %s\n", dirbuf);

    if ((cd = processclasssrc(dirbuf)))
      list_saveitem(classlist, cd->name, cd);
    *mypathpart(dirbuf) = 0;
  }
  closedir(dir);
  if (srccnt == 0)
  {
    fprintf(stderr, "ERROR: Was unable to find any sources in %s\n", dirname);
    return 0;
  }
  /* alphabetically sort the class list */
  list_sort(classlist, compareclasses);
  if (arg_v) printclasslist(classlist);
  return 1;
}

void buildclasstree( struct list *classlist )
{
  struct classdef *outercd;
  struct node *outern;

  /* iterate over all classes and check whether one class has one
   * of our private classes as superclass
   */
  for(outern = NULL; (outern = list_getnext(classlist, outern, (void **)&outercd)); )
  {
    struct classdef *innercd;
    struct node *innern;

    for(innern = NULL; (innern = list_getnext(classlist, innern, (void **)&innercd)); )
    {
      if(innercd != outercd)
      {
        /* assume that the superclass name begins with "MUIC_"
         * and only respect nodes with a yet public superclass (supernode == NULL)
         */
        if(outercd->supernode == NULL && strcmp(&outercd->superclass[5], innercd->name) == 0)
        {
          /* remember the superclass */
          outercd->supernode = innercd;
        }
      }
      else
      {
        if(strcmp(&outercd->superclass[5], outercd->name) == 0)
        {
          /* one *VERY* dumb developer declared the class to be subclassed
           * from itself. This of course is absolute nonsense.
           * We just remember the class to be its own superclass. Eventually
           * this will generate an #error statement in the generated source
           * because of a zero sized dependency loop.
           */
          outercd->supernode = outercd;
        }
      }
    }
  }
}

unsigned long read_crc( const char *crcfile )
{
  FILE *fp;
  unsigned long crc = INVALID_CRC;

  if((fp = fopen(crcfile, "r")) != NULL)
  {
    char line[32];

    /* read the CRC from the file
     */
    if(fgets(line, 31, fp))
    {
      crc = strtoul(line, NULL, 16);
    }

    fclose(fp);
  }

  return crc;
}

void write_crc( const char *crcfile, const unsigned long crc )
{
  FILE *fp;

  if((fp = fopen(crcfile, "w")) != NULL)
  {
    fprintf(fp, "0x%08lx", crc);
    fclose(fp);
  }
}

unsigned long gen_crc( struct list *classlist )
{
  unsigned long crc;
  struct classdef *nextcd;
  struct declaredef *nextdd;
  struct exportdef *nexted;
  struct attrdef *nextad;
  struct overloaddef *nextod;
  struct node *n, *nn;

  crc = 0;

  /* iterate over all classes and calculate the CRC checksum from
   * all stuff the classes expose to the public (methods, definitions,
   * class data, etc).
   */
  for(nn = NULL; (nn = list_getnext(classlist, nn, (void **)&nextcd)); )
  {
    if(nextcd->name != NULL)
      crc = crc32(nextcd->name, strlen(nextcd->name), crc);
    if(nextcd->superclass != NULL)
      crc = crc32(nextcd->superclass, strlen(nextcd->superclass), crc);
    if(nextcd->desc != NULL)
      crc = crc32(nextcd->desc, strlen(nextcd->desc), crc);
    if(nextcd->classdata != NULL)
      crc = crc32(nextcd->classdata, strlen(nextcd->classdata), crc);

    for(n = NULL; (n = list_getnext(&nextcd->overloadlist, n, (void **)&nextod));)
    {
      crc = crc32(nextod->name, strlen(nextod->name), crc);
    }
    for(n = NULL; (n = list_getnext(&nextcd->declarelist, n, (void **)&nextdd));)
    {
      crc = crc32(nextdd->name, strlen(nextdd->name), crc);
      crc = crc32(nextdd->params, strlen(nextdd->params), crc);
    }

    for(n = NULL; (n = list_getnext(&nextcd->attrlist, n, (void **)&nextad));)
    {
      crc = crc32(nextad->name, strlen(nextad->name), crc);
    }

    for(n = NULL; (n = list_getnext(&nextcd->exportlist, n, (void **)&nexted));)
    {
      crc = crc32(nexted->exporttext, strlen(nexted->exporttext), crc);
    }
  }

  return crc;
}

/*******************************************************************************
 *
 * Source code generation
 *
 *******************************************************************************
 *
 *
 */

void gen_gpl( FILE *fp )
{
  if (!fp || !arg_gpl) return;

  fprintf(fp,
  "/***************************************************************************\n"
  "\n"
  " YAM - Yet Another Mailer\n"
  " Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>\n"
  " Copyright (C) 2000-2010 by YAM Open Source Team\n"
  "\n"
  " This program is free software; you can redistribute it and/or modify\n"
  " it under the terms of the GNU General Public License as published by\n"
  " the Free Software Foundation; either version 2 of the License, or\n"
  " (at your option) any later version.\n"
  "\n");

  fprintf(fp,
  " This program is distributed in the hope that it will be useful,\n"
  " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  " GNU General Public License for more details.\n"
  "\n");

  fprintf(fp,
  " You should have received a copy of the GNU General Public License\n"
  " along with this program; if not, write to the Free Software\n"
  " Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
  "\n"
  " YAM Official Support Site :  http://www.yam.ch/\n"
  " YAM OpenSource project    :  http://sourceforge.net/projects/yamos/\n"
  "\n"
  " $Id$\n"
  "\n"
  "***************************************************************************/\n");
}

void gen_supportroutines( FILE *fp )
{
  char *bn = arg_basename;

  fprintf(fp, "%s%s%s", arg_storm ? "/// " : "", arg_storm ? bn : "", arg_storm ? "_NewObject()\n" : "");
  fprintf(fp, "Object * VARARGS68K %s_NewObject(CONST_STRPTR className, ...)\n", bn);
  fprintf(fp, "{\n");
  fprintf(fp, "  Object *obj = NULL;\n");
  fprintf(fp, "  unsigned int i;\n");
  fprintf(fp, "\n");
  fprintf(fp, "  ENTER();\n");
  fprintf(fp, "\n");
  fprintf(fp, "  for(i = 0; i < NUMBEROFCLASSES; i++)\n");
  fprintf(fp, "  {\n");
  fprintf(fp, "    if(strcmp(MCCInfo[i].Name, className) == 0)\n");
  fprintf(fp, "    {\n");
  fprintf(fp, "      VA_LIST args;\n");
  fprintf(fp, "\n");
  fprintf(fp, "      VA_START(args, className);\n");
  fprintf(fp, "      obj = NewObjectA(%sClasses[i]->mcc_Class, NULL, (struct TagItem *)VA_ARG(args, ULONG));\n", bn);
  fprintf(fp, "      VA_END(args);\n");
  fprintf(fp, "\n");
  fprintf(fp, "      break;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "\n");
  fprintf(fp, "  RETURN(obj);\n");
  fprintf(fp, "  return obj;\n");
  fprintf(fp, "}\n");
  fprintf(fp, "%s", arg_storm ? "\n///" : "");
  fprintf(fp, "\n");

  fprintf(fp, "%s%s%s", arg_storm ? "/// " : "", arg_storm ? bn : "", arg_storm ? "_SetupClasses()\n" : "");
  fprintf(fp, "BOOL %s_SetupClasses(void)\n", bn);
  fprintf(fp, "{\n");
  fprintf(fp, "  BOOL success = TRUE;\n");
  fprintf(fp, "  unsigned int i;\n");
  fprintf(fp, "\n");
  fprintf(fp, "  ENTER();\n");
  fprintf(fp, "\n");
  fprintf(fp, "  memset(%sClasses, 0, sizeof(%sClasses));\n", bn, bn);
  fprintf(fp, "  for (i = 0; i < NUMBEROFCLASSES; i++)\n");
  fprintf(fp, "  {\n");
  fprintf(fp, "    const char *superClassName;\n");
  fprintf(fp, "    struct MUI_CustomClass *superMCC;\n");
  fprintf(fp, "    ULONG classdataSize;\n");
  fprintf(fp, "\n");
  fprintf(fp, "    if(MCCInfo[i].SuperMCCIndex == -1)\n");
  fprintf(fp, "    {\n");
  fprintf(fp, "      superClassName = MCCInfo[i].SuperClass;\n");
  fprintf(fp, "      superMCC = NULL;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "    else\n");
  fprintf(fp, "    {\n");
  fprintf(fp, "      superClassName = NULL;\n");
  fprintf(fp, "      superMCC = %sClasses[MCCInfo[i].SuperMCCIndex];\n", bn);
  fprintf(fp, "      if(superMCC == NULL)\n");
  fprintf(fp, "      {\n");
  fprintf(fp, "        E(DBF_STARTUP, \"superclass '%%s' of class '%%s' not yet created!\", MCCInfo[i].SuperClass, MCCInfo[i].Name);\n");
  fprintf(fp, "        success = FALSE;\n");
  fprintf(fp, "        break;\n");
  fprintf(fp, "      }\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "\n");
  fprintf(fp, "    if(MCCInfo[i].GetSize != NULL)\n");
  fprintf(fp, "      classdataSize = MCCInfo[i].GetSize();\n");
  fprintf(fp, "    else\n");
  fprintf(fp, "      classdataSize = 0;\n");
  fprintf(fp, "\n");
  fprintf(fp, "    D(DBF_STARTUP, \"creating class '%%s' as subclass of '%%s'\", MCCInfo[i].Name, MCCInfo[i].SuperClass);\n");
  fprintf(fp, "    %sClasses[i] = MUI_CreateCustomClass(NULL, superClassName, superMCC, classdataSize, MCCInfo[i].Dispatcher);\n", bn);
  fprintf(fp, "    if(%sClasses[i] == NULL)\n", bn);
  fprintf(fp, "    {\n");
  fprintf(fp, "      E(DBF_STARTUP, \"failed to create class '%%s' as subclass of '%%s'\", MCCInfo[i].Name, MCCInfo[i].SuperClass);\n");
  fprintf(fp, "      success = FALSE;\n");
  fprintf(fp, "      break;\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "\n");
  fprintf(fp, "  if(success == FALSE)\n");
  fprintf(fp, "    %s_CleanupClasses();\n", bn);
  fprintf(fp, "\n");
  fprintf(fp, "  RETURN(success);\n");
  fprintf(fp, "  return success;\n");
  fprintf(fp, "}\n");
  fprintf(fp, "%s", arg_storm ? "\n///" : "");
  fprintf(fp, "\n");

  fprintf(fp, "%s%s%s", arg_storm ? "/// " : "", arg_storm ? bn : "", arg_storm ? "_CleanupClasses()\n" : "");
  fprintf(fp, "void %s_CleanupClasses(void)\n", bn);
  fprintf(fp, "{\n");
  fprintf(fp, "  int i;\n");
  fprintf(fp, "\n");
  fprintf(fp, "  ENTER();\n");
  fprintf(fp, "\n");
  fprintf(fp, "  for(i = NUMBEROFCLASSES-1; i >= 0; i--)\n");
  fprintf(fp, "  {\n");
  fprintf(fp, "    if(%sClasses[i] != NULL)\n", bn);
  fprintf(fp, "      MUI_DeleteCustomClass(%sClasses[i]);\n", bn);
  fprintf(fp, "    %sClasses[i] = NULL;\n", bn);
  fprintf(fp, "  }\n");
  fprintf(fp, "\n");
  fprintf(fp, "  LEAVE();\n");
  fprintf(fp, "}\n");
  if(arg_storm)
  {
    fprintf(fp, "\n");
    fprintf(fp, "///\n");
  }
  fprintf(fp, "\n");
}

int gen_source( char *destfile, struct list *classlist )
{
  FILE *fp;
  struct classdef *nextcd;
  struct overloaddef *nextod;
  struct declaredef *nextdd;
  struct node *n, *nn;
  int goOn;
  int listindex;

  if(arg_v)
    fprintf(stdout, "Creating source      : %s\n", destfile);

  if((fp = fopen(destfile, "w")) == NULL)
  {
    fprintf(stderr, "ERROR: Unable to open %s\n", destfile);
    return 0;
  }

  /***************************************/
  /*           Write header...           */
  /***************************************/

  gen_gpl(fp);
  fprintf(fp, "\n");
  fprintf(fp, "/* " EDIT_WARNING " */\n");
  fprintf(fp, "\n");
  fprintf(fp, "#define INCLUDE_KITCHEN_SINK 1\n");
  fprintf(fp, "#include \"Classes.h\"\n");
  fprintf(fp, "#include \"Debug.h\"\n");
  fprintf(fp, "\n");
  fprintf(fp, "struct MUI_CustomClass *%sClasses[NUMBEROFCLASSES];\n", arg_basename);
  fprintf(fp, "\n");

  /***************************************/
  /*        Write dispatchers...         */
  /***************************************/

  fprintf(fp, "/*** Custom Class Dispatchers ***/\n");
  for(nn = NULL; (nn = list_getnext(classlist, nn, (void **)&nextcd)); )
  {
    if(arg_storm)
      fprintf(fp, "/// %sDispatcher()\n", nextcd->name);

    fprintf(fp, "SDISPATCHER(%sDispatcher)\n", nextcd->name);
    fprintf(fp, "{\n  switch(msg->MethodID)\n  {\n");
    /* Write OVERLOADs */
    for(n = NULL; (n = list_getnext(&nextcd->overloadlist, n, (void **)&nextod)); )
    {
      fprintf(fp, "    case %-40s: return m_%s_%-30s(cl, obj, msg);\n", nextod->name, nextcd->name, nextod->name);
    }
    /* Write DECLAREs */
    for(n = NULL; (n = list_getnext(&nextcd->declarelist, n, (void **)&nextdd)); )
    {
      char name[128];

      snprintf(name, sizeof(name), "MUIM_%s_%s", nextcd->name, nextdd->name);
      fprintf(fp, "    case %-40s: return m_%s_%-30s(cl, obj, (APTR)msg);\n", name, nextcd->name, nextdd->name);
    }
    fprintf(fp, "  }\n");
    fprintf(fp, " \n");
    fprintf(fp, "  return DoSuperMethodA(cl, obj, msg);\n");
    fprintf(fp, "}\n");
    fprintf(fp, "\n");

    if(arg_storm)
      fprintf(fp, "///\n");
  }

  /*****************************************/
  /*        Write MCCInfo struct           */
  /*****************************************/

  fprintf(fp, "\n");
  fprintf(fp, "/*** Custom Class Support ***/\n");
  if(arg_storm)
    fprintf(fp, "/// struct MCCInfo\n");

  fprintf(fp, "const struct\n");
  fprintf(fp, "{\n");
  fprintf(fp, "  CONST_STRPTR Name;\n");
  fprintf(fp, "  CONST_STRPTR SuperClass;\n");
  fprintf(fp, "  LONG SuperMCCIndex;\n");
  fprintf(fp, "  ULONG (*GetSize)(void);\n");
  fprintf(fp, "  APTR Dispatcher;\n");
  fprintf(fp, "} MCCInfo[NUMBEROFCLASSES] =\n");
  fprintf(fp, "{\n");

  /* do a breadth search first style iteration over all
   * classes to resolve possible dependencies between
   * private classes
   */
  listindex = 0;
  do
  {
    goOn = 0;

    for(n = NULL; (n = list_getnext(classlist, n, (void **) &nextcd)); )
    {
      /* handle unfinished class nodes only for which either the class is a subclass
       * of a public class (supernode == NULL), or for which the superclass' list
       * index in known already
       */
      if(nextcd->finished == 0 && (nextcd->supernode == NULL || nextcd->supernode->index != -1))
      {
        if(nextcd->classdata != NULL)
          fprintf(fp, "  { MUIC_%s, %s, %d, %sGetSize, ENTRY(%sDispatcher) },\n", nextcd->name, nextcd->superclass, nextcd->supernode != NULL ? nextcd->supernode->index : -1, nextcd->name, nextcd->name);
        else
          fprintf(fp, "  { MUIC_%s, %s, %d, NULL, ENTRY(%sDispatcher) },\n", nextcd->name, nextcd->superclass, nextcd->supernode != NULL ? nextcd->supernode->index : -1, nextcd->name);

        /* remember index within the list of this class
         * this one will be used later for depending subclasses
         */
        nextcd->index = listindex;
        listindex++;

        if(nextcd->supernode != NULL)
        {
          goOn = 1;
        }

        /* mark this class as finished */
        nextcd->finished = 1;
      }
    }
  }
  while(goOn == 1);

  /* if we handled less classes than are in our list then we most probably
   * skipped them because of a dependency loop
   */
  if(listindex < classlist->cnt)
  {
    fprintf(fp, "#error %ld unfinished classes, possible dependency loops in these classes:", classlist->cnt-listindex);
    for(n = NULL; (n = list_getnext(classlist, n, (void **) &nextcd)); )
    {
      /* handle all still unfinished class nodes with private superclasses */
      if(nextcd->finished == 0 && nextcd->supernode != NULL)
        fprintf(fp, " %s (subclass of %s)", nextcd->name, nextcd->supernode->name);
    }
    fprintf(fp, "\n");
  }

  fprintf(fp, "};\n");
  fprintf(fp, "\n");

  if(arg_storm)
    fprintf(fp, "///\n");

  /*****************************************/
  /*        Append support routines        */
  /*****************************************/

  gen_supportroutines(fp);
  fclose(fp);
  return 1;
}

int gen_header( char *destfile, struct list *classlist )
{
  FILE *fp;
  char *bn = arg_basename, *cn, *p;
  struct classdef *nextcd;
  struct declaredef *nextdd;
  struct exportdef *nexted;
  struct attrdef *nextad;
  struct overloaddef *nextod;
  struct node *n, *nn;

  if(arg_v)
    fprintf(stdout, "Creating header      : %s\n", destfile);

  if((fp = fopen(destfile, "w")) == NULL)
  {
    fprintf(stderr, "ERROR: Unable to open %s\n", destfile);
    return 0;
  }

  /***************************************/
  /*           Write header...           */
  /***************************************/

  gen_gpl(fp);
  fprintf(fp, "\n");
  fprintf(fp, "#ifndef CLASSES_CLASSES_H\n");
  fprintf(fp, "#define CLASSES_CLASSES_H\n");
  fprintf(fp, "\n");
  fprintf(fp, "/* " EDIT_WARNING " */\n");
  fprintf(fp, "\n");

  /* TODO: write class tree in header here */

  /***************************************/
  /*          Write includes...          */
  /***************************************/

/*
  fprintf(fp,
    "#include <clib/alib_protos.h>\n"
    "#include <libraries/mui.h>\n"
    "#include <proto/intuition.h>\n"
    "#include <proto/muimaster.h>\n"
    "#include <proto/utility.h>\n");
*/
  if(arg_includes[0])
  {
    char *nx, *p = arg_includes;

    do
    {
      if((nx = strchr(p, ',')) != NULL)
        *nx++ = 0;
      fprintf(fp, "#include \"%s\"\n", p);
    }
    while ((p = nx) != NULL);
  }

  /***************************************/
  /*            Write misc...            */
  /***************************************/

  fprintf(fp, "\n");
  fprintf(fp, "#define inittags(msg)   (((struct opSet *)msg)->ops_AttrList)\n");
  fprintf(fp, "#define GETDATA         struct Data *data = (struct Data *)INST_DATA(cl,obj)\n");
  fprintf(fp, "#define NUMBEROFCLASSES %ld\n", classlist->cnt);
  fprintf(fp, "\n");
  fprintf(fp, "extern struct MUI_CustomClass *%sClasses[NUMBEROFCLASSES];\n", bn);
  fprintf(fp, "Object * VARARGS68K %s_NewObject(CONST_STRPTR className, ...);\n", bn);
  fprintf(fp, "BOOL %s_SetupClasses(void);\n", bn);
  fprintf(fp, "void %s_CleanupClasses(void);\n", bn);
  fprintf(fp, "\n");

  /***************************************/
  /*             Class loop              */
  /***************************************/

  for(nn = NULL; (nn = list_getnext(classlist, nn, (void **) &nextcd)); )
  {
    cn = nextcd->name;

    /***********************************************/
    /* Write MUIC_, xxxObject, etc. for this class */
    /***********************************************/

    fprintf(fp, "/******** Class: %s (0x%08x) ********/\n", cn, gettagvalue(cn, 0));
    fprintf(fp, "\n");
    fprintf(fp, "#define MUIC_%s \"%s_%s\"\n", cn, bn, cn);
    fprintf(fp, "#define %sObject %s_NewObject(MUIC_%s\n", cn, bn, cn);

    for(n = NULL; (n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd));)
    {
      char name[128];

      snprintf(name, sizeof(name), "MUIM_%s_%s", cn, nextdd->name);
      fprintf(fp, "#define %-45s 0x%08x\n", name, gettagvalue(cn, 1));
    }

    /***************************************/
    /* Write attributes for this class     */
    /***************************************/

    for(n = NULL; (n = list_getnext(&nextcd->attrlist, n, (void **) &nextad));)
    {
      char name[128];

      snprintf(name, sizeof(name), "MUIA_%s_%s", cn, nextad->name);
      fprintf(fp, "#define %-45s 0x%08x\n", name, gettagvalue(cn, 1));
    }
    fprintf(fp, "\n");

    /***************************************/
    /* Write exported text for this class  */
    /***************************************/

    if(nextcd->exportlist.cnt)
    {
      fprintf(fp, "/* Exported text */\n\n");
      for (n = NULL; (n = list_getnext(&nextcd->exportlist, n, (void **) &nexted));)
        fprintf(fp, "%s\n\n", nexted->exporttext);

    }

    /*****************************************/
    /* Write MUIP_ structures for this class */
    /*****************************************/

    for(n = NULL; (n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd));)
    {
      fprintf(fp, "struct MUIP_%s_%s\n", cn, nextdd->name);
      fprintf(fp, "{\n");
      fprintf(fp, "  ULONG methodID;\n");

      if(strlen(nextdd->params) > 0)
      {
        char *lp;

        for(p = lp = nextdd->params;;)
        {
          if((p = strpbrk(lp, ",;")) != NULL)
          {
            *p++ = '\0';

            fprintf(fp, "  %s;\n", lp);

            lp = p;
          }
          else
          {
            if(strlen(lp) > 0)
              fprintf(fp, "  %s;\n", lp);

            break;
          }
        }
      }
      fprintf(fp, "};\n");
      fprintf(fp, "\n");
    }
    fprintf(fp, "\n");

    /***************************************/
    /* Write protos for this class         */
    /***************************************/

    if(nextcd->classdata != NULL)
      fprintf(fp, "ULONG %sGetSize(void);\n", cn);

    /* Write OVERLOADs */
    for(n = NULL; (n = list_getnext(&nextcd->overloadlist, n, (void **)&nextod));)
    {
      fprintf(fp, "ULONG m_%s_%s(struct IClass *cl, Object *obj, Msg msg);\n", nextcd->name, nextod->name);
    }
    /* Write DECLAREs */
    for(n = NULL; (n = list_getnext(&nextcd->declarelist, n, (void **)&nextdd));)
    {
      fprintf(fp, "ULONG m_%s_%s(struct IClass *cl, Object *obj, struct MUIP_%s_%s *msg);\n", nextcd->name, nextdd->name, cn, nextdd->name);
    }
    fprintf(fp, "\n");

  }
  fprintf(fp, "\n#endif /* CLASSES_CLASSES_H */\n\n");
  fclose(fp);
  return 1;
}

int gen_classheaders( struct list *classlist )
{
  struct node *n;
  struct classdef *nextcd;
  FILE *fp;

  for(n = NULL; (n = list_getnext(classlist, n, (void **)&nextcd));)
  {
    char name[128], buf[128], *p;
    char *cn = nextcd->name;

    snprintf(name, sizeof(name), "%s_cl.h", cn);
    myaddpart(arg_classdir, name, 255);

    if(arg_v)
      fprintf(stdout, "Creating class header: %s\n", arg_classdir);

    if((fp = fopen(arg_classdir, "w")) == NULL)
    {
      fprintf(stderr, "WARNING: Unable to write %s\n", name);
      *mypathpart(arg_classdir) = 0;

      continue;
    }
    strncpy(buf, cn, 127);
    for (p = buf; *p; p++)
      *p = toupper(*p);

    /* write the gpl to this class header also */
    gen_gpl(fp);
    fprintf(fp, "\n");
    fprintf(fp, "/* " EDIT_WARNING " */\n");
    fprintf(fp, "\n");

    fprintf(fp, "#ifndef %s_H\n", buf);
    fprintf(fp, "#define %s_H 1\n", buf);
    fprintf(fp, "\n");
    fprintf(fp, "#ifndef CLASSES_CLASSES_H\n");
    fprintf(fp, "  #define INCLUDE_KITCHEN_SINK 1\n");
    fprintf(fp, "  #include \"Classes.h\"\n");
    fprintf(fp, "#endif /* CLASSES_CLASSES_H */\n");
    fprintf(fp, "\n");

    fprintf(fp, "#define DECLARE(method)  ULONG m_%s_## method(UNUSED struct IClass *cl, UNUSED Object *obj, UNUSED struct MUIP_%s_## method *msg )\n", cn, cn);
    fprintf(fp, "#define OVERLOAD(method) ULONG m_%s_## method(UNUSED struct IClass *cl, UNUSED Object *obj, UNUSED Msg msg )\n", cn);
    fprintf(fp, "#define METHOD(method)   MUIM_%s_## method\n", cn);
    fprintf(fp, "#define ATTR(attr)       MUIA_%s_## attr\n", cn);
    fprintf(fp, "\n");

    if(nextcd->classdata != NULL)
    {
      fprintf(fp, "/* Exported CLASSDATA */\n");
      fprintf(fp, "\n");
      fprintf(fp, "struct Data\n");
      fprintf(fp, "{\n");
      fprintf(fp, "%s\n", nextcd->classdata);
      fprintf(fp, "};\n");
      fprintf(fp, "\n");
      fprintf(fp, "ULONG %sGetSize( void ) { return sizeof(struct Data); }\n", cn);
    }

    fprintf(fp, "\n");
    fprintf(fp, "#endif /* %s_H */\n", buf);

    *mypathpart(arg_classdir) = 0;
    fclose(fp);
  }
  return 1;
}

/*******************************************************************************
 *
 * Makefile generation
 *
 *
 * .p.s. I'm not an expert on makefiles, so someone improve this. ;-)
 *
 *******************************************************************************
 *
 *
 */

int gen_makefile( char *destfile, struct list *classlist )
{
  struct classdef *nextcd;
  char *cn, *p;
  FILE *fp;
  struct node *n;

  for(p = arg_mkfile_ccopts;;)
  {
    if((p = strchr(p, ',')))
      *p++ = ' ';
    else
      break;
  }

  if(arg_v)
    fprintf(stdout, "Creating makefile    : %s\n", destfile);

  if((fp = fopen(destfile, "w")) == NULL)
  {
    fprintf(stderr, "ERROR: Unable to open %s\n", destfile);
    return 0;
  }
  fprintf(fp, "#\n"
        "# " EDIT_WARNING "\n"
        "#\n");
  fprintf(fp, "\nCC = %s\n"
        "CCOPTS = %s\n\n", arg_mkfile_cc, arg_mkfile_ccopts);
  fprintf(fp, "all.o : %s ", OBJECT_NAME);
  for (n = NULL; (n = list_getnext(classlist, n, (void **) &nextcd));)
    fprintf(fp, "%s.o ", nextcd->name);
  fprintf(fp, "\n\tjoin " OBJECT_NAME " ");
  for (n = NULL; (n = list_getnext(classlist, n, (void **) &nextcd));)
    fprintf(fp, "%s.o ", nextcd->name);
  fprintf(fp, "AS all.o\n\n");
  fprintf(fp, "%s : %s %s ", OBJECT_NAME, SOURCE_NAME, HEADER_NAME);
  fprintf(fp, "\n\t$(CC) %s %s %s $(CCOPTS)\n\n",
        SOURCE_NAME, arg_mkfile_outarg, OBJECT_NAME);
  for (n = NULL; (n = list_getnext(classlist, n, (void **) &nextcd));)
  {
    cn = nextcd->name;
    fprintf(fp, "%s.o : %s.c\n"
          "\t$(CC) %s.c %s %s.o $(CCOPTS)\n\n", cn, cn, cn, arg_mkfile_outarg, cn);
  }
  fclose(fp);
  return 1;
}

/*******************************************************************************
 *
 * Argument processing and entry point
 *
 *******************************************************************************
 *
 *
 */

int getstrarg( char *argid, char *argline, char *dest, size_t destlen )
{
  char *p;
  size_t arglen = strlen(argid);
  if (strncmp(argid, argline, arglen) != 0) return 0;
  p = &argline[arglen];
  strncpy(dest, p, destlen);
  return 1;
}

int getblnarg( char *argid, char *argline, int *blnlong )
{
  if (strncmp(argid, argline, strlen(argid)) != 0) return 0;
  *blnlong = 1;
  return 1;
}

int doargs( unsigned int argc, char *argv[] )
{
  unsigned int i;
  int success;

  arg_gpl = 0;
  for (i = 1; i < argc; i++)
  {
    if (getstrarg("-b",      argv[i], arg_basename, 255)) continue;
    if (getstrarg("-i",      argv[i], arg_includes, 255)) continue;
    if (getblnarg("-gpl",    argv[i], &arg_gpl         )) continue;
    if (getblnarg("-storm",  argv[i], &arg_storm       )) continue;
    if (getstrarg("-mkfile", argv[i], arg_mkfile,   255)) continue;
    if (getblnarg("-v",      argv[i], &arg_v           )) continue;
    if (getblnarg("-verbose",argv[i], &arg_v           )) continue;
    if (getblnarg("-q",      argv[i], &arg_q           )) continue;
  }

  if(arg_q == 0 || argc < 2)
    fprintf(stdout, "GenClasses v%s by Andrew Bell <mechanismx@lineone.net>\n\n", verstr);

  if(argc < 2)
  {
    fprintf(stderr,
      "Usage: GenClasses <classdir> -b<basename> <options>\n"
         "\n"
      "Options:\n"
      "\n"
      " -b<basename>                                 - basename (.i.e. YAM) used in sources (required)\n"
      " -gpl                                         - write GPL headers into sources\n");

    fprintf(stderr,
      " -storm                                       - include storm/GoldED fold markers\n"
      " -i<includes>                                 - includes for Classes.h (.i.e. -i\"YAM.h\",\"YAM_hook.h\",<stdlib.h>\n"
      " -v                                           - verbose output while generating files\n"
      " -q                                           - keep output quiet and only output errors\n"
      " -mkfile<makefile>,<cc>,<outarg>,<ccopts,...> - Create a makefile\n"
      "    (.i.e. -mkfileVMakefile,vc,-o,-O3,-+)\n");

    return 0;
  }

  success = 1;
  strncpy(arg_classdir, argv[1], 255);
  if (arg_classdir[0] == 0 || arg_classdir[0] == '-')
  {
    fprintf(stderr, "No class dir specified, using current directory.\n");
    strcpy(arg_classdir, "");
  }
  if (!arg_basename[0])
  {
    success = 0;
    fprintf(stderr, "ERROR: You MUST provide a basename using the -b argument\n");
  }

  if(arg_mkfile[0])
  {
    if((arg_mkfile_cc     = strchr(arg_mkfile,        ',')))  *arg_mkfile_cc++     = 0;

    if(arg_mkfile_cc)
    {
      if((arg_mkfile_outarg = strchr(arg_mkfile_cc,     ','))) *arg_mkfile_outarg++ = 0;
    }

    if(arg_mkfile_outarg)
    {
      if ((arg_mkfile_ccopts = strchr(arg_mkfile_outarg, ','))) *arg_mkfile_ccopts++ = 0;
    }

    if (!arg_mkfile_cc)     arg_mkfile_cc     = "cc";
    if (!arg_mkfile_outarg) arg_mkfile_outarg = "-o";
    if (!arg_mkfile_ccopts) arg_mkfile_ccopts = "";
    strncpy(arg_mkfile_dest,   arg_classdir, 255);
    myaddpart(arg_mkfile_dest, arg_mkfile,   255);
  }

  return success;
}

int main( int argc, char *argv[] )
{
  struct node *n;
  struct list classlist;

  list_init(&classlist);

  if(!doargs(argc, argv))
    return 0;

  /* get memory for the hash */
  if((collision_cnts = calloc(BUCKET_CNT, sizeof(int))) == NULL)
    return 0;

  if(scanclasses(arg_classdir, &classlist))
  {
    unsigned long old_crc;
    unsigned long new_crc;

    buildclasstree(&classlist);

    /* get the previous CRC of all classes */
    myaddpart(arg_classdir, CRC_NAME, 255);
    old_crc = read_crc(arg_classdir);
    *mypathpart(arg_classdir) = 0;

    /* calculate the CRC of all just scanned classes */
    new_crc = gen_crc(&classlist);

    /* generate a new source only if there was no CRC before
     * or if the old CRC does not match the new one
     */
    if(old_crc == INVALID_CRC || old_crc != new_crc)
    {
      myaddpart(arg_classdir, SOURCE_NAME, 255);
      gen_source(arg_classdir, &classlist);
      *mypathpart(arg_classdir) = 0;

      myaddpart(arg_classdir, HEADER_NAME, 255);
      gen_header(arg_classdir, &classlist);
      *mypathpart(arg_classdir) = 0;

      gen_classheaders(&classlist);
    }

    /* save the new CRC for the next run */
    myaddpart(arg_classdir, CRC_NAME, 255);
    write_crc(arg_classdir, new_crc);
    *mypathpart(arg_classdir) = 0;

    if(arg_mkfile_dest[0])
      gen_makefile(arg_mkfile_dest, &classlist);
  }

  while((n = list_remhead(&classlist)) != NULL)
  {
    free_classdef(n->data);
    free(n);
  }

  return 0;
}

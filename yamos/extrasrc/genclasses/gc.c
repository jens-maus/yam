/***************************************************************************

 GenClasses - MUI class dispatcher generator
 Copyright (C) 2001 by Andrew Bell <mechanismx@lineone.net>

 Contributed to the YAM Open Source Team as a special version
 Copyright (C) 2001-2002 by YAM Open Source Team

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
    - DISPATCHERPROTO macro should be included in generated source.
    - DECLARE param parser should allow normal C style comments to be used
      too.

***************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "gc.h"
#include "lists.h"

/*
 * When compiling for YAM under VBCC use:
 *
 * GenClasses <classespath> -bYAM -gpl -i"/YAM.h" -mkfilevmakefile,vc,-o,-+,-c* 
 *
 *
 * History
 * -------
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

char *verstr = "0.9";

/* Every shitty hack wouldn't be complete without some shitty globals... */

char  arg_classdir          [256];
char  arg_basename          [256];
char  arg_includes          [256];
char  arg_mkfile            [256];
char  arg_mkfile_dest       [256];
char *arg_mkfile_cc       = NULL;
char *arg_mkfile_outarg   = NULL;
char *arg_mkfile_ccopts   = NULL;
long   arg_gpl             = 0;     /* booleans */
long   arg_storm           = 0;
long   arg_v               = 0;

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
#define TAG_USER ((unsigned long)(1UL<<31))
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
	char c;
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
	char *strvec, *estr, c;			
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

long myaddpart( char *path, char *name, size_t len )
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

long *collision_cnts;

unsigned long gethash( char *str )
{
    static unsigned long primes[10] = { 3, 5, 7, 11, 13, 17, 19, 23, 27, 31 };
    unsigned long i = 0;
    unsigned long hash = strlen(str);
    while (*str)
    {
        hash *= 13;
        hash += (*str++) * primes[(++i) % 10];
    }
    return hash % BUCKET_CNT;
}

unsigned long gettagvalue( char *tag )
{
	unsigned long hash = gethash(tag);
	unsigned long val = TAG_BASE | ((hash << 16) | (hash << 8) | (++collision_cnts[hash]));
	if (arg_v) printf("Assigning tag %-35s with value %lx\n", tag, val);
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
	if (od->name) free(od->name);
}

void free_declare( struct declaredef *dd )
{
	if (!dd) return;
	if (dd->name) free(dd->name);
	if (dd->params) free(dd->params);
}

void free_exportblk( struct exportdef *ed )
{
	if (!ed) return;
	if (ed->exporttext) free(ed->exporttext);
}

void free_attr( struct attrdef *ad )
{
	if (!ad) return;
	if (ad->name) free(ad->name);
}

void free_classdef( struct classdef *cd )
{
	struct node *n;
	if (!cd) return;
	if (cd->classdata) free(cd->classdata);
	while (n = list_remhead(&cd->overloadlist)) {
		free_overload(n->data);
		free(n);
	}	
	while (n = list_remhead(&cd->declarelist)) {
		free_declare(n->data);
		free(n);
	}	
	while (n = list_remhead(&cd->attrlist)) {
		free_attr(n->data);
		free(n);
	}	
	while (n = list_remhead(&cd->exportlist)) {
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
		if (arg_v) printf("ATTR %s already collected, skipped.\n", name);
		free_attr(ad);
    return;
	}
	list_saveitem(&cd->attrlist, ad->name, ad);
}

struct classdef *processclasssrc( char *path )
{
	FILE *fp;
	long lineno = 0;
	char line[256], *p, *ob, *cb, *sub;
	struct classdef *cd;
	long spos, epos = 0, exlineno = lineno;
  char *blk = NULL;

	if (!(cd = calloc(1, sizeof(struct classdef)))) return NULL;
	if (!(fp = fopen(path, "r")))
	{
		printf("ERROR: Unable to open %s\n", path);
		free(cd); return NULL;
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
			if (arg_v) printf(KEYWD_SUPERCLASS " keyword found at line %ld in file %s\n", lineno, path);
			sub += sizeof(KEYWD_SUPERCLASS) - 1;
			cd->superclass = stralloc(skipwhitespaces(sub));
		}
		else if (!cd->desc && (sub = strstr(p, KEYWD_DESC)))
		{
			if (arg_v) printf(KEYWD_DESC " keyword found at line %ld\n", lineno);
			sub += sizeof(KEYWD_DESC) - 1;
			cd->desc = stralloc(skipwhitespaces(sub));
		}
		else if (!cd->classdata && strstr(line, KEYWD_CLASSDATA))
		{
			if (arg_v) printf(KEYWD_CLASSDATA " keyword found at line %ld\n", lineno);
			spos = ftell(fp);
			while(fgets(p = line, 255, fp))
			{
				lineno++; epos = ftell(fp);
				if (!(p = strstr(line, "*/"))) continue;
				epos += (p - line) - 3; /* + offset into line... */
				fseek(fp, spos, SEEK_SET);
				if (!(blk = calloc(1, (size_t)(epos - spos + 1))))
				{
					printf("WARNING: Cannot read " KEYWD_CLASSDATA " block at line %ld, out of memory!\n", exlineno); break;
				}
				fread(blk, (size_t)(epos - spos), 1, fp);
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
			if (arg_v) printf(KEYWD_OVERLOAD " keyword found at line %ld in file %s\n", lineno, path);
			p += sizeof(KEYWD_OVERLOAD) - 1;
			if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
			if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
			*cb = 0; add_overload(cd, ++ob);
		}
		else if (strncmp(KEYWD_DECLARE, p, sizeof(KEYWD_DECLARE) - 1) == 0)
		{
			if (arg_v) printf(KEYWD_DECLARE " keyword found at line %ld in file %s\n", lineno, path);
			p += sizeof(KEYWD_DECLARE) - 1;
			if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
			if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
			if ((p = strstr(cb + 1, "//"))) p += 2;
			*cb = 0; add_declare(cd, ++ob, p);
		}
		else if (strncmp(KEYWD_ATTR, p, sizeof(KEYWD_ATTR) - 1) == 0)
		{
			if (arg_v) printf(KEYWD_ATTR " keyword found at line %ld in file %s\n", lineno, path);
			p += sizeof(KEYWD_ATTR) - 1;
			if (!(ob = strchr(p, '('))) continue; /* There's no open bracket, ignore it... */
			if (!(cb = strchr(ob, ')'))) cb = p + strlen(p);
			*cb = 0; add_attr(cd, ++ob);
		}
		else if (strncmp("/*", p, 2) == 0) /* TODO: Use strstr() here, like CLASSDATA */
		{
			p = skipwhitespaces(p + 2);
			if (strncmp(KEYWD_EXPORT, p, sizeof(KEYWD_EXPORT) - 1) == 0)
			{
				if (arg_v) printf(KEYWD_EXPORT " keyword found at line %ld in file %s\n", lineno, path);
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
						printf("WARNING: Cannot read " KEYWD_EXPORT " block at line %ld, out of memory!\n", exlineno); break;
					}
					fread(blk, (size_t)(epos - spos), 1, fp);
					add_exportblk(cd, blk);
					free(blk);
					break;
				}
				if (epos == 0) printf("WARNING: Unterminated EXPORT block at line %ld\n", lineno);
			}
		}
	}	/* while() */
	if (!cd->superclass)
	{
		printf("WARNING: Source file '%s' doesn't contain a " KEYWD_SUPERCLASS " keyword. Skipping.\n", path);
		free_classdef(cd); cd = NULL;
	}
	else if (!cd->classdata)
	{
		printf("WARNING: Source file '%s' doesn't contain a " KEYWD_CLASSDATA " keyword. Skipping.\n", path);
		free_classdef(cd); cd = NULL;
	}
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
	printf("The following keywords were extracted:\n");
	for (nn = NULL; nn = list_getnext(classlist, nn, (void **) &cd);)
	{
		printf("CLASS: %s\n", cd->name);
		for (n = NULL; n = list_getnext(&cd->overloadlist, n, (void **) &od);)
			printf("  OVERLOAD: %s\n", od->name);
		for (n = NULL; n = list_getnext(&cd->declarelist, n, (void **) &dd);)
			printf("   DECLARE: %s\n", dd->name);
		for (n = NULL; n = list_getnext(&cd->attrlist, n, (void **) &ad);)
			printf("      ATTR: %s\n", ad->name);
	}
}

long scanclasses( char *dirname, struct list *classlist )
{
	DIR *dir;
	struct dirent *de;
	char *n, dirbuf[256];
	long len, srccnt = 0;
	struct classdef *cd;
	strncpy(dirbuf, dirname, 255);
	if (arg_v) printf("scanning classes dir %s\n", dirbuf);
	if (!(dir = opendir(dirname)))
	{
		printf("Unable to open directory %s\n", dirname);
		return 0;
	}
	while ((de = readdir(dir)))
	{
		n = de->d_name; len = strlen(n);
		if (len < 2) continue;
		if ((n[len - 2] != '.') || (tolower(n[len - 1]) != 'c'))
		{
			printf("Skipping: %s\n", n); continue;
		}	
		if (!strcmp(SOURCE_NAME, n) || !strcmp(HEADER_NAME, n)) continue;
		++srccnt;
		myaddpart(dirbuf, de->d_name, 255);
		printf("processing: %s\n", dirbuf);
		if ((cd = processclasssrc(dirbuf)))
			list_saveitem(classlist, cd->name, cd);
		*mypathpart(dirbuf) = 0;
	}
	closedir(dir);
	if (srccnt == 0)
	{
		printf("ERROR: Was unable to find any sources in %s\n", dirname);
		return 0;
	}
	if (arg_v) printclasslist(classlist);
	return 1;
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
  " Copyright (C) 2000-2002 by YAM Open Source Team\n"
  "\n"
  " This program is free software; you can redistribute it and/or modify\n"
  " it under the terms of the GNU General Public License as published by\n"
  " the Free Software Foundation; either version 2 of the License, or\n"
  " (at your option) any later version.\n"
  "\n"
  " This program is distributed in the hope that it will be useful,\n"
  " but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
  " GNU General Public License for more details.\n"
  "\n"
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
	fprintf(fp,
"%s%s%s"
"Object * STDARGS %s_NewObject(STRPTR class, ...)\n"
"{\n"
"  long i;\n"
"  for(i = 0; i < NUMBEROFCLASSES; i++)\n"
"  {\n"
"    if(!strcmp(MCCInfo[i].Name, class))\n"
"    {\n"
"#if defined(__MORPHOS__)\n"
"			 va_list va;\n"
"			 va_start(va, class);\n"
"			 return NewObjectA(%sClasses[i]->mcc_Class, NULL, (struct TagItem *)va->overflow_arg_area);\n"
"#else\n"
"			 return NewObjectA(%sClasses[i]->mcc_Class, NULL, (struct TagItem *)(&class+1));\n"
"#endif\n"
"    }\n"
"  }\n"
"  return NULL;\n"
"}\n"
"%s"
"\n"
"%s%s%s"
"long %s_SetupClasses(void)\n"
"{\n"
"  long i;\n"
"  memset(%sClasses, 0, sizeof(%sClasses));\n"
"  for (i = 0; i < NUMBEROFCLASSES; i++)\n"
"  {\n"
"    struct MUI_CustomClass *superMCC = MCCInfo[i].SuperMCC == -1 ? NULL : %sClasses[MCCInfo[i].SuperMCC];\n"
"    %sClasses[i] = MUI_CreateCustomClass(NULL, MCCInfo[i].SuperClass, superMCC, (int)MCCInfo[i].GetSize(), MCCInfo[i].Dispatcher);\n"
"    if (!%sClasses[i])\n"
"    {\n"
"      %s_CleanupClasses();\n"
"      return 0;\n"
"    }\n"
"  }\n"
"  return 1;\n"
"}\n"
"%s"
"\n"
"%s%s%s"
"void %s_CleanupClasses(void)\n"
"{\n"
"  long i;\n"
"  for (i = NUMBEROFCLASSES-1; i >= 0; i--)\n"
"  {\n"
"    MUI_DeleteCustomClass(%sClasses[i]);\n"
"    %sClasses[i] = NULL;\n"
"  }\n"
"}\n"
"%s"
"\n",

	arg_storm ? "/// "                : "",	
	arg_storm ? bn                    : "",	
	arg_storm ? "_NewObject()\n"      : "",
	bn, bn, bn,
	arg_storm ? "///\n"               : "",	

	arg_storm ? "/// "                : "",	
	arg_storm ? bn                    : "",	
	arg_storm ? "_SetupClasses()\n"   : "",
	bn, bn, bn, bn, bn, bn, bn,
	arg_storm ? "///\n"               : "",	

	arg_storm ? "/// "                : "",	
	arg_storm ? bn                    : "",	
	arg_storm ? "_CleanupClasses()\n" : "",
	bn, bn, bn,
	arg_storm ? "///\n"               : "");
}

long gen_source( char *destfile, struct list *classlist )
{
	FILE *fp;
	struct classdef *nextcd;
	struct overloaddef *nextod;
	struct declaredef *nextdd;
	struct node *n, *nn;

	printf("Creating source      : %s\n", destfile);
	if (!(fp = fopen(destfile, "w")))
	{
		printf("ERROR: Unable to open %s\n", destfile);
		return 0;
	}

	/***************************************/
	/*           Write header...           */
	/***************************************/

	gen_gpl(fp);
	fprintf(fp, "\n /* " EDIT_WARNING " */\n\n"
		"#include <string.h>\n"
		"#include \"Classes.h\"\n\n"
		"struct MUI_CustomClass *%sClasses[NUMBEROFCLASSES];\n\n",
		arg_basename);

	/***************************************/
	/*        Write dispatchers...         */
	/***************************************/
	
	for (nn = NULL; nn = list_getnext(classlist, nn, (void **) &nextcd);)
	{
		if (arg_storm) fprintf(fp, "/// %sDispatcher()\n", nextcd->name);
		fprintf(fp, "DISPATCHERPROTO(%sDispatcher)\n"
			"{\n  switch(msg->MethodID)\n  {\n", nextcd->name);
		/* Write OVERLOADs */
		for (n = NULL; n = list_getnext(&nextcd->overloadlist, n, (void **) &nextod);)
		{
			fprintf(fp, "    case %-40s: return m_%s_%-20s(cl, obj, msg);\n",
				nextod->name, nextcd->name, nextod->name);
		}
		/* Write DECLAREs */
		for (n = NULL; n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd);)
		{
			char name[128];
			sprintf(name, "MUIM_%s_%s", nextcd->name, nextdd->name);
			fprintf(fp, "    case %-40s: return m_%s_%-20s(cl, obj, (APTR)msg);\n",
				name, nextcd->name, nextdd->name);
		}
		fprintf(fp, "  }\n  return DoSuperMethodA(cl, obj, msg);\n}\n");
		if (arg_storm) fprintf(fp, "///\n");
		fprintf(fp, "\n");
	}

	/*****************************************/
	/*        Write MCCInfo struct           */
	/*****************************************/

	fprintf(fp,
		"const struct\n"
    "{\n"
    "  STRPTR Name; STRPTR SuperClass; LONG SuperMCC; ULONG (*GetSize) (void); APTR Dispatcher;\n"
		"} MCCInfo[NUMBEROFCLASSES] =\n"
    "{\n");

	for (n = NULL; n = list_getnext(classlist, n, (void **) &nextcd);)
	{
		fprintf(fp, "  { MUIC_%s, %s, -1, %sGetSize, ENTRY(%sDispatcher) }",
			nextcd->name, nextcd->superclass, nextcd->name, nextcd->name);
		if (nextcd) fprintf(fp, ",\n"); else fprintf(fp, "\n");
	}
	fprintf(fp,  "};\n\n");
	
	/*****************************************/
	/*        Append support routines        */
	/*****************************************/

	gen_supportroutines(fp);
	fclose(fp);
	return 1;
}

long gen_header( char *destfile, struct list *classlist )
{
	FILE *fp;
	char *bn = arg_basename, *cn, *p;
	struct classdef *nextcd;
	struct declaredef *nextdd;
	struct exportdef *nexted;
	struct attrdef *nextad;
	struct overloaddef *nextod;
	struct node *n, *nn;

	printf("Creating header      : %s\n", destfile);
	if (!(fp = fopen(destfile, "w")))
	{
		printf("ERROR: Unable to open %s\n", destfile);
		return 0;
	}

	/***************************************/
	/*           Write header...           */
	/***************************************/

	gen_gpl(fp);
	fprintf(fp, "\n#ifndef CLASSES_CLASSES_H\n"
		"#define CLASSES_CLASSES_H\n"
		"\n /* " EDIT_WARNING " */\n\n");

	/* TODO: write class tree in header here */

	/***************************************/
	/*          Write includes...          */
	/***************************************/
	
	fprintf(fp, 
		"#include <clib/alib_protos.h>\n"
		"#include <libraries/mui.h>\n"
		"#include <proto/intuition.h>\n"
		"#include <proto/muimaster.h>\n"
		"#include <proto/utility.h>\n");
	if (arg_includes[0])
	{
		char *nx, *p = arg_includes;
		do
		{
			if ((nx = strchr(p, ','))) *nx++ = 0;
			fprintf(fp, "#include \"%s\"\n", p);
		}
		while ((p = nx));
	}

	/***************************************/
	/*            Write misc...            */
	/***************************************/

	fprintf(fp, 
		"\n"
		"#define inittags(msg)   (((struct opSet *)msg)->ops_AttrList)\n"
		"#define GETDATA         struct Data *data = (struct Data *)INST_DATA(cl,obj)\n"
		"#define NUMBEROFCLASSES %ld\n"
		"\n"
		"extern struct MUI_CustomClass *%sClasses[NUMBEROFCLASSES];\n"
		"Object * STDARGS %s_NewObject( STRPTR class, ... ) VARARGS68K;\n"
		"long %s_SetupClasses( void );\n"
		"void %s_CleanupClasses( void );\n"
		"\n",
		classlist->cnt, bn, bn, bn, bn);

	/***************************************/
	/*             Class loop              */
	/***************************************/

	for (nn = NULL; nn = list_getnext(classlist, nn, (void **) &nextcd);)
	{
		cn = nextcd->name;

		/***********************************************/
		/* Write MUIC_, xxxObject, etc. for this class */
		/***********************************************/

		fprintf(fp, 
			"/******** Class: %s ********/\n"
			"\n"
			"#define MUIC_%s \"%s_%s\"\n"
			"#define %sObject %s_NewObject(MUIC_%s\n",
			cn, cn, bn, cn, cn, bn, cn);

		for (n = NULL; n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd);)
		{
			char name[128];
			sprintf(name, "MUIM_%s_%s", cn, nextdd->name);
			fprintf(fp, "#define %-45s 0x%08lx\n", name, gettagvalue(name));
		}

		/***************************************/
		/* Write attributes for this class     */
		/***************************************/

		for (n = NULL; n = list_getnext(&nextcd->attrlist, n, (void **) &nextad);)
		{
			char name[128];
			sprintf(name, "MUIA_%s_%s", cn, nextad->name);
			fprintf(fp, "#define %-45s 0x%08lx\n", name, gettagvalue(name));
		}
		fprintf(fp, "\n");

		/*****************************************/
		/* Write MUIP_ structures for this class */
		/*****************************************/

		for (n = NULL; n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd);)
		{
			fprintf(fp,
				"struct MUIP_%s_%s\n"
				"{\n"
				"  ULONG methodID;\n", cn, nextdd->name);
			if (strlen(nextdd->params) > 0)
			{
				for (p = nextdd->params;;) if ((p = strchr(p, ','))) *p++ = ';'; else break;
				fprintf(fp, "  %s;\n", nextdd->params);
			}
			fprintf(fp, "};\n\n");
		}
		fprintf(fp, "\n");

		/***************************************/
		/* Write protos for this class         */
		/***************************************/

		fprintf(fp, "ULONG %sGetSize( void );\n", cn);

		/* Write OVERLOADs */
		for (n = NULL; n = list_getnext(&nextcd->overloadlist, n, (void **) &nextod);)
		{
			fprintf(fp, "ULONG m_%s_%-20s(struct IClass *cl, Object *obj, Msg msg);\n",
				nextcd->name, nextod->name);
		}
		/* Write DECLAREs */
		for (n = NULL; n = list_getnext(&nextcd->declarelist, n, (void **) &nextdd);)
		{
			fprintf(fp, "ULONG m_%s_%-20s(struct IClass *cl, Object *obj, struct MUIP_%s_%s *msg);\n",
				nextcd->name, nextdd->name, cn, nextdd->name);
		}
		fprintf(fp, "\n");

		/***************************************/
		/* Write exported text for this class  */
		/***************************************/
			
		if (nextcd->exportlist.cnt)
		{
			fprintf(fp, "/* Exported text */\n\n");
			for (n = NULL; n = list_getnext(&nextcd->exportlist, n, (void **) &nexted);)
				fprintf(fp, "%s\n\n", nexted->exporttext);	

		}
	}
	fprintf(fp, "\n#endif /* CLASSES_CLASSES_H */\n\n");
	fclose(fp);
	return 1;
}

long gen_classheaders( struct list *classlist )
{
	struct node *n;
	struct classdef *nextcd;
	FILE *fp;
	for (n = NULL; n = list_getnext(classlist, n, (void **) &nextcd);)
	{
		char name[128], buf[128], *p;
		char *cn = nextcd->name;
		sprintf(name, "%s_cl.h", cn);
		myaddpart(arg_classdir, name, 255);
		printf("Creating class header: %s\n", arg_classdir);
		if (!(fp = fopen(arg_classdir, "w")))
		{
			printf("WARNING: Unable to write %s\n", name);
			*mypathpart(arg_classdir) = 0;

			continue;
		}
		strncpy(buf, cn, 127); for (p = buf; *p; p++) *p = toupper(*p);

    /* write the gpl to this class header also */
  	gen_gpl(fp);
	  fprintf(fp, "\n /* " EDIT_WARNING " */\n\n");

		fprintf(fp,
			"#ifndef %s_H\n"
			"#define %s_H\n"
			"\n"
			"#ifndef CLASSES_CLASSES_H\n"
			"#include \"Classes.h\"\n"
			"#endif /* CLASSES_CLASSES_H */\n"
			"\n"
			"#define DECLARE(method)  ULONG m_%s_## method( struct IClass *cl, Object *obj, struct MUIP_%s_## method *msg )\n"
			"#define OVERLOAD(method) ULONG m_%s_## method( struct IClass *cl, Object *obj, Msg msg )\n"
			"#define ATTR(attr)       case MUIA_%s_## attr\n"
			"\n"
			"/* Exported CLASSDATA */\n"
			"\n"
			"struct Data\n"
			"{\n"
			"%s\n"
			"};\n"
			"\n"
			"ULONG %sGetSize( void ) { return sizeof(struct Data); }\n"
			"\n"
			"#endif /* %s_H */\n",
				buf, buf, cn, cn, cn, cn, nextcd->classdata, cn, buf);	

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

long gen_makefile( char *destfile, struct list *classlist )
{
	struct classdef *nextcd;
	char *cn, *p;
	FILE *fp;
	struct node *n;

	for (p = arg_mkfile_ccopts;;) if ((p = strchr(p, ','))) *p++ = ' '; else break;
	printf("Creating makefile    : %s\n", destfile);
	if (!(fp = fopen(destfile, "w")))
	{
		printf("ERROR: Unable to open %s\n", destfile);
		return 0;
	}
	fprintf(fp, "#\n"
				"# " EDIT_WARNING "\n"
				"#\n");
	fprintf(fp, "\nCC = %s\n"
				"CCOPTS = %s\n\n", arg_mkfile_cc, arg_mkfile_ccopts);
	fprintf(fp, "all.o : %s ", OBJECT_NAME);
	for (n = NULL; n = list_getnext(classlist, n, (void **) &nextcd);)
		fprintf(fp, "%s.o ", nextcd->name);
	fprintf(fp, "\n\tjoin " OBJECT_NAME " ");
	for (n = NULL; n = list_getnext(classlist, n, (void **) &nextcd);)
		fprintf(fp, "%s.o ", nextcd->name);
	fprintf(fp, "AS all.o\n\n");
	fprintf(fp, "%s : %s %s ", OBJECT_NAME, SOURCE_NAME, HEADER_NAME);
	fprintf(fp, "\n\t$(CC) %s %s %s $(CCOPTS)\n\n",
				SOURCE_NAME, arg_mkfile_outarg, OBJECT_NAME);
	for (n = NULL; n = list_getnext(classlist, n, (void **) &nextcd);)
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

long getstrarg( char *argid, char *argline, char *dest, size_t destlen )
{
	char *p;
	size_t arglen = strlen(argid);
	if (strncmp(argid, argline, arglen) != 0) return 0;
	p = &argline[arglen];
	strncpy(dest, p, destlen);
	return 1;
}

long getblnarg( char *argid, char *argline, long *blnlong )
{
	if (strncmp(argid, argline, strlen(argid)) != 0) return 0;
	*blnlong = 1;
	return 1;
}

long doargs( int argc, char *argv[] )
{
	long i, success;
	if (argc < 2)
	{
		printf(
			"Usage: GenClasses <classdir> -b<basename> <options>\n"
		   	"\n"
			"Options:\n"
			"\n"
			" -b<basename>                                 - basename (.i.e. YAM) used in sources (required)\n"
			" -gpl                                         - write GPL headers onto sources\n"
			" -storm                                       - include storm/GoldED fold markers\n"
			" -i<includes>                                 - includes for Classes.h (.i.e. -i\"YAM.h\",\"YAM_hook.h\",<stdlib.h>\n"
			" -mkfile<makefile>,<cc>,<outarg>,<ccopts,...> - Create a makefile\n"
			"    (.i.e. -mkfileVMakefile,vc,-o,-O3,-+\n");
		return 0;
	}
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
	}
	success = 1;
	strncpy(arg_classdir, argv[1], 255);
	if (arg_classdir[0] == 0 || arg_classdir[0] == '-')
	{
		printf("No class dir specified, using current directory.\n");
		strcpy(arg_classdir, "");
	}
	if (!arg_basename[0])
	{
		success = 0;
		printf("ERROR: You MUST provide a basename using the -b argument\n");
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
  printf("GenClasses v%s by Andrew Bell <mechanismx@lineone.net>\n\n", verstr);

	if (!doargs(argc, argv)) return 0;

  /* get memory for the hash */
  if(!(collision_cnts = calloc(BUCKET_CNT, sizeof(long)))) return 0;

	if (scanclasses(arg_classdir, &classlist))
	{
		myaddpart(arg_classdir, SOURCE_NAME, 255);
		gen_source(arg_classdir, &classlist);
		*mypathpart(arg_classdir) = 0;

		myaddpart(arg_classdir, HEADER_NAME, 255);
		gen_header(arg_classdir, &classlist);
		*mypathpart(arg_classdir) = 0;

		gen_classheaders(&classlist);
		if (arg_mkfile_dest[0]) gen_makefile(arg_mkfile_dest, &classlist);
	}
	while (n = list_remhead(&classlist)) {
		free_classdef(n->data);
		free(n);
	}
	return 0;
}

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

#include <stddef.h>

#include <rexx/storage.h>

#include "YAM_rexx_rxif.h"
#include "YAM_rexx_rxcl.h"

#define RESINDEX(stype) (((long)offsetof(struct stype,res)) / sizeof(long))

struct rxs_command rxs_commandlist[] =
{
	{ "ADDRDELETE", "ALIAS", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrdelete, 1 },
	{ "ADDREDIT", "ALIAS,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBER/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addredit, 1 },
	{ "ADDRFIND", "PATTERN/A,NAMEONLY/S,EMAILONLY/S", "ALIAS/M", RESINDEX(rxd_addrfind), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrfind, 1 },
	{ "ADDRGOTO", "ALIAS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrgoto, 1 },
	{ "ADDRINFO", "ALIAS/A", "TYPE,NAME,EMAIL,PGP,HOMEPAGE,STREET,CITY,COUNTRY,PHONE,COMMENT,BIRTHDATE/N,IMAGE,MEMBERS/M", RESINDEX(rxd_addrinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrinfo, 1 },
	{ "ADDRLOAD", "FILENAME/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrload, 1 },
	{ "ADDRNEW", "TYPE,ALIAS,NAME,EMAIL", "ALIAS", RESINDEX(rxd_addrnew), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrnew, 1 },
	{ "ADDRRESOLVE", "ALIAS/A", "RECPT", RESINDEX(rxd_addrresolve), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrresolve, 1 },
	{ "ADDRSAVE", "FILENAME", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_addrsave, 1 },
	{ "APPBUSY", "TEXT", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_appbusy, 1 },
	{ "APPNOBUSY", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_appnobusy, 1 },
	{ "FOLDERINFO", "FOLDER", "NUMBER/N,NAME,PATH,TOTAL/N,NEW/N,UNREAD/N,SIZE/N,TYPE/N", RESINDEX(rxd_folderinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_folderinfo, 1 },
	{ "GETCONFIGINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getconfiginfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getconfiginfo, 1 },
	{ "GETFOLDERINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getfolderinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getfolderinfo, 1 },
	{ "GETMAILINFO", "ITEM/A", "VALUE", RESINDEX(rxd_getmailinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getmailinfo, 1 },
	{ "GETSELECTED", NULL, "NUM/N/M", RESINDEX(rxd_getselected), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_getselected, 1 },
	{ "GETURL", "URL/A,FILENAME/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_geturl, 1 },
	{ "HELP", "FILE", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_help, 1 },
	{ "HIDE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_hide, 1 },
	{ "INFO", "ITEM/A", "VALUE", RESINDEX(rxd_info), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_info, 1 },
	{ "ISONLINE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_isonline, 1 },
	{ "LISTSELECT", "MODE/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_listselect, 1 },
	{ "MAILARCHIVE", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailarchive, 1 },
	{ "MAILBOUNCE", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailbounce), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailbounce, 1 },
	{ "MAILCHANGESUBJECT", "SUBJECT/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailchangesubject, 1 },
	{ "MAILCHECK", "POP/K/N,MANUAL/S", "DOWNLOADED/N,ONSERVER/N,DUPSKIPPED/N,DELETED/N", RESINDEX(rxd_mailcheck), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailcheck, 1 },
	{ "MAILCOPY", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailcopy, 1 },
	{ "MAILDELETE", "ATONCE/S,FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_maildelete, 1 },
	{ "MAILEDIT", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailedit), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailedit, 1 },
	{ "MAILEXPORT", "FILENAME/A,ALL/S,APPEND/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailexport, 1 },
	{ "MAILFILTER", "ALL/S", "CHECKED/N,BOUNCED/N,FORWARDED/N,REPLIED/N,EXECUTED/N,MOVED/N,DELETED/N", RESINDEX(rxd_mailfilter), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailfilter, 1 },
	{ "MAILFORWARD", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailforward), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailforward, 1 },
	{ "MAILIMPORT", "FILENAME/A,WAIT/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailimport, 1 },
	{ "MAILINFO", "INDEX/N", "INDEX/N,STATUS,FROM,TO,REPLYTO,SUBJECT,FILENAME,SIZE/N,DATE,FLAGS,MSGID", RESINDEX(rxd_mailinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailinfo, 1 },
	{ "MAILMOVE", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailmove, 1 },
	{ "MAILREAD", "WINDOW/N,QUIET/S", "WINDOW/N", RESINDEX(rxd_mailread), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailread, 1 },
	{ "MAILREPLY", "QUIET/S", "WINDOW/N", RESINDEX(rxd_mailreply), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailreply, 1 },
	{ "MAILSEND", "ALL/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailsend, 1 },
	{ "MAILSENDALL", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailsendall, 1 },
	{ "MAILSTATUS", "STATUS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailstatus, 1 },
	{ "MAILUPDATE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailupdate, 1 },
	{ "MAILWRITE", "WINDOW/N,QUIET/S", "WINDOW/N", RESINDEX(rxd_mailwrite), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_mailwrite, 1 },
	{ "NEWMAILFILE", "FOLDER", "FILENAME", RESINDEX(rxd_newmailfile), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_newmailfile, 1 },
	{ "QUIT", "FORCE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_quit, 1 },
	{ "READCLOSE", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readclose, 1 },
	{ "READINFO", NULL, "FILENAME/M,FILETYPE/M,FILESIZE/N/M,TEMPFILE/M", RESINDEX(rxd_readinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readinfo, 1 },
	{ "READPRINT", "PART/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readprint, 1 },
	{ "READSAVE", "PART/N,FILENAME/K,OVERWRITE/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_readsave, 1 },
	{ "REQUEST", "BODY/A,GADGETS/A", "RESULT/N", RESINDEX(rxd_request), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_request, 1 },
	{ "REQUESTFOLDER", "BODY/A,EXCLUDEACTIVE/S", "FOLDER", RESINDEX(rxd_requestfolder), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_requestfolder, 1 },
	{ "REQUESTSTRING", "BODY/A,STRING/K,SECRET/S", "STRING", RESINDEX(rxd_requeststring), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_requeststring, 1 },
	{ "SCREENTOBACK", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_screentoback, 1 },
	{ "SCREENTOFRONT", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_screentofront, 1 },
	{ "SETFLAG", "VOL/K/N,PER/K/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setflag, 1 },
	{ "SETFOLDER", "FOLDER/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setfolder, 1 },
	{ "SETMAIL", "NUM/N/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setmail, 1 },
	{ "SETMAILFILE", "MAILFILE/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_setmailfile, 1 },
	{ "SHOW", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_show, 1 },
	{ "USERINFO", NULL, "USERNAME,EMAIL,REALNAME,CONFIG,MAILDIR,FOLDERS/N", RESINDEX(rxd_userinfo), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_userinfo, 1 },
	{ "WRITEATTACH", "FILE/A,DESC,ENCMODE,CTYPE", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeattach, 1 },
	{ "WRITEBCC", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writebcc, 1 },
	{ "WRITECC", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writecc, 1 },
	{ "WRITEEDITOR", "COMMAND/A", "RESULT", RESINDEX(rxd_writeeditor), (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeeditor, 1 },
	{ "WRITEFROM", "ADDRESS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writefrom, 1 },
	{ "WRITELETTER", "FILE/A,NOSIG/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeletter, 1 },
	{ "WRITEMAILTO", "ADDRESS/A/M", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writemailto, 1 },
	{ "WRITEOPTIONS", "DELETE/S,RECEIPT/S,NOTIF/S,ADDINFO/S,IMPORTANCE/N,SIG/N,SECURITY/N", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeoptions, 1 },
	{ "WRITEQUEUE", "HOLD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writequeue, 1 },
	{ "WRITEREPLYTO", "ADDRESS/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writereplyto, 1 },
	{ "WRITESEND", NULL, NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writesend, 1 },
	{ "WRITESUBJECT", "SUBJECT/A", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writesubject, 1 },
	{ "WRITETO", "ADDRESS/A/M,ADD/S", NULL, 0, (void (*)(struct RexxHost *,void **,long,struct RexxMsg *)) rx_writeto, 1 },
	{ NULL, NULL, NULL, 0, NULL, 0 }
};

static struct arb_p_link link0[] = {
	{"WRITE", 1}, {"USERINFO", 16}, {"S", 17}, {"RE", 28}, {"QUIT", 37}, {"NEWMAILFILE", 38},
	{"MAIL", 39}, {"LISTSELECT", 67}, {"I", 68}, {"H", 71}, {"GET", 74}, {"FOLDERINFO", 80},
	{"A", 81}, {NULL, 0} };

static struct arb_p_link link1[] = {
	{"TO", 2}, {"S", 3}, {"REPLYTO", 6}, {"QUEUE", 7}, {"OPTIONS", 8}, {"MAILTO", 9},
	{"LETTER", 10}, {"FROM", 11}, {"EDITOR", 12}, {"CC", 13}, {"BCC", 14}, {"ATTACH", 15},
	{NULL, 0} };

static struct arb_p_link link3[] = {
	{"UBJECT", 4}, {"END", 5}, {NULL, 0} };

static struct arb_p_link link17[] = {
	{"HOW", 18}, {"ET", 19}, {"CREENTO", 25}, {NULL, 0} };

static struct arb_p_link link19[] = {
	{"MAIL", 20}, {"F", 22}, {NULL, 0} };

static struct arb_p_link link20[] = {
	{"FILE", 21}, {NULL, 0} };

static struct arb_p_link link22[] = {
	{"OLDER", 23}, {"LAG", 24}, {NULL, 0} };

static struct arb_p_link link25[] = {
	{"FRONT", 26}, {"BACK", 27}, {NULL, 0} };

static struct arb_p_link link28[] = {
	{"QUEST", 29}, {"AD", 32}, {NULL, 0} };

static struct arb_p_link link29[] = {
	{"STRING", 30}, {"FOLDER", 31}, {NULL, 0} };

static struct arb_p_link link32[] = {
	{"SAVE", 33}, {"PRINT", 34}, {"INFO", 35}, {"CLOSE", 36}, {NULL, 0} };

static struct arb_p_link link39[] = {
	{"WRITE", 40}, {"UPDATE", 41}, {"S", 42}, {"RE", 46}, {"MOVE", 49}, {"I", 50},
	{"F", 53}, {"E", 56}, {"DELETE", 59}, {"C", 60}, {"BOUNCE", 65}, {"ARCHIVE", 66},
	{NULL, 0} };

static struct arb_p_link link42[] = {
	{"TATUS", 43}, {"END", 44}, {NULL, 0} };

static struct arb_p_link link44[] = {
	{"ALL", 45}, {NULL, 0} };

static struct arb_p_link link46[] = {
	{"PLY", 47}, {"AD", 48}, {NULL, 0} };

static struct arb_p_link link50[] = {
	{"NFO", 51}, {"MPORT", 52}, {NULL, 0} };

static struct arb_p_link link53[] = {
	{"ORWARD", 54}, {"ILTER", 55}, {NULL, 0} };

static struct arb_p_link link56[] = {
	{"XPORT", 57}, {"DIT", 58}, {NULL, 0} };

static struct arb_p_link link60[] = {
	{"OPY", 61}, {"H", 62}, {NULL, 0} };

static struct arb_p_link link62[] = {
	{"ECK", 63}, {"ANGESUBJECT", 64}, {NULL, 0} };

static struct arb_p_link link68[] = {
	{"SONLINE", 69}, {"NFO", 70}, {NULL, 0} };

static struct arb_p_link link71[] = {
	{"IDE", 72}, {"ELP", 73}, {NULL, 0} };

static struct arb_p_link link74[] = {
	{"URL", 75}, {"SELECTED", 76}, {"MAILINFO", 77}, {"FOLDERINFO", 78}, {"CONFIGINFO", 79}, {NULL, 0} };

static struct arb_p_link link81[] = {
	{"PP", 82}, {"DDR", 85}, {NULL, 0} };

static struct arb_p_link link82[] = {
	{"NOBUSY", 83}, {"BUSY", 84}, {NULL, 0} };

static struct arb_p_link link85[] = {
	{"SAVE", 86}, {"RESOLVE", 87}, {"NEW", 88}, {"LOAD", 89}, {"INFO", 90}, {"GOTO", 91},
	{"FIND", 92}, {"EDIT", 93}, {"DELETE", 94}, {NULL, 0} };

struct arb_p_state arb_p_state[] = {
	{-1, link0}, {59, link1}, {71, NULL}, {69, link3}, {70, NULL},
	{69, NULL}, {68, NULL}, {67, NULL}, {66, NULL}, {65, NULL},
	{64, NULL}, {63, NULL}, {62, NULL}, {61, NULL}, {60, NULL},
	{59, NULL}, {58, NULL}, {51, link17}, {57, NULL}, {53, link19},
	{55, link20}, {56, NULL}, {53, link22}, {54, NULL}, {53, NULL},
	{51, link25}, {52, NULL}, {51, NULL}, {44, link28}, {48, link29},
	{50, NULL}, {49, NULL}, {44, link32}, {47, NULL}, {46, NULL},
	{45, NULL}, {44, NULL}, {43, NULL}, {42, NULL}, {22, link39},
	{41, NULL}, {40, NULL}, {37, link42}, {39, NULL}, {37, link44},
	{38, NULL}, {35, link46}, {36, NULL}, {35, NULL}, {34, NULL},
	{32, link50}, {33, NULL}, {32, NULL}, {30, link53}, {31, NULL},
	{30, NULL}, {28, link56}, {29, NULL}, {28, NULL}, {27, NULL},
	{24, link60}, {26, NULL}, {24, link62}, {25, NULL}, {24, NULL},
	{23, NULL}, {22, NULL}, {21, NULL}, {19, link68}, {20, NULL},
	{19, NULL}, {17, link71}, {18, NULL}, {17, NULL}, {12, link74},
	{16, NULL}, {15, NULL}, {14, NULL}, {13, NULL}, {12, NULL},
	{11, NULL}, {0, link81}, {9, link82}, {10, NULL}, {9, NULL},
	{0, link85}, {8, NULL}, {7, NULL}, {6, NULL}, {5, NULL},
	{4, NULL}, {3, NULL}, {2, NULL}, {1, NULL}, {0, NULL} 
	};

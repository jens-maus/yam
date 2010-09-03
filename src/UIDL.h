#ifndef UIDL_H
#define UIDL_H

/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2010 by YAM Open Source Team

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

#include "HashTable.h"

struct MailServerNode;

struct UIDLhash
{
  struct HashTable *hash;            // the hash table to hold all data
  struct MailServerNode *mailServer; // the mail server for which the data are to be managed
  BOOL isDirty;                      // did anything change during the POP/IMAP session?
};

struct UIDLtoken
{
  struct HashEntryHeader hash; // a standard hash entry header
  const char *uidl;            // the UIDL token
  ULONG flags;                 // flags for this UIDL, see below
};

#define UIDLF_OLD     (1<<0) // we knew this UIDL before
#define UIDLF_NEW     (1<<1) // this is a new UIDL

struct UIDLhash *InitUIDLhash(const struct MailServerNode *msn);
struct UIDLtoken *AddUIDLtoHash(struct UIDLhash *uidlHash, const char *uidl, const ULONG flags);
struct UIDLtoken *FindUIDL(const struct UIDLhash *uidlHash, const char *uidl);
void CleanupUIDLhash(struct UIDLhash *uidlHash);

#endif /* UIDL_H */

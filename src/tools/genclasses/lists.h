/***************************************************************************

 GenClasses - MUI class dispatcher generator
 Copyright (C) 2001 by Andrew Bell <mechanismx@lineone.net>

 Contributed to the YAM Open Source Team as a special version
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

 $Id$

***************************************************************************/

#ifndef LISTS_H
#define LISTS_H

struct node
{
  struct node *succ;
  struct node *pred;
  char        *name;
  void        *data;
};


struct list
{
  struct node *head;
  struct node *tail;
  struct node *tailpred;
  long         cnt;
};

void list_init( struct list *l );
void list_saveitem( struct list *l, char *name, void *data );
struct node *list_getnext( struct list *l, struct node *n, void **data_ptr );
struct node *list_findname( struct list *l, char *name );
void list_insert( struct list *l, struct node *n, struct node *npred );
void list_remove( struct list *l, struct node *n );
void list_addhead( struct list *l, struct node *n );
void list_addtail( struct list *l, struct node *n );
struct node *list_remhead( struct list *l );
struct node *list_remtail( struct list *l );
void list_sort( struct list *l, int (* compare)( const struct node *, const struct node *) );

#endif /* LISTS_H */

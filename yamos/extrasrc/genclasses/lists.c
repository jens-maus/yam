/***************************************************************************

 GenClasses - MUI class dispatcher generator
 Copyright (C) 2001 by Andrew Bell <mechanismx@lineone.net>

 Contributed to the YAM Open Source Team as a special version

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

#include "lists.h"
#include <string.h>
#include <stdlib.h>

/* Portable linked list support */

void list_init( struct list *l )
{
	l->tailpred = (struct node *) l;
	l->tail		= NULL;
	l->head		= (struct node *) &l->tail;
	l->cnt      = 0;
}

void list_saveitem( struct list *l, char *name, void *data )
{
	struct node *n;
	if (!(n = (struct node *) calloc(1, sizeof(struct node)))) return;
	n->name = name;
	n->data = data;
	list_addtail(l, n);
}

struct node *list_getnext( struct list *l, struct node *n, void **data_ptr )
{
	if (!n)
	{
		if (!l->cnt) return NULL;
		if ((n = l->head)) if (data_ptr) *data_ptr = n->data;
		return n;
	}
	n = n->succ;
	if (!n->succ) return NULL;
	if (data_ptr) *data_ptr = n->data;
	return n;
}

struct node *list_findname( struct list *l, char *name )
{
	struct node *n;
	for (n = l->head; n; n = n->succ)
	{
		if(!n->name) continue;
		if (!strcmp(n->name, name)) break;
	}
	return n;
}

void list_insert( struct list *l, struct node *n, struct node *npred )
{
	if (!npred)
	{
		list_addtail(l, n);
		return;
	}
	if (npred->succ)
	{
		n->succ			  = npred->succ;
		n->pred			  = npred;
		npred->succ->pred = n;
		npred->succ		  = n;
	}
	else
	{
		n->succ			  = (struct node *) &l->tail;
		n->pred			  = l->tailpred;
		l->tailpred->succ = n;
		l->tailpred		  = n;
	}
	l->cnt += 1;
}

void list_remove( struct list *l, struct node *n )
{
	n->pred->succ = n->succ;
	n->succ->pred = n->pred;
	l->cnt		 -= 1;
}


void list_addhead( struct list *l, struct node *n )
{
	n->succ		  = l->head;
	n->pred		  = (struct node *) &l->head;
	l->head->pred = n;
	l->head		  = n;
	l->cnt		 += 1;
}

void list_addtail( struct list *l, struct node *n )
{
	n->succ			  = (struct node *) &l->tail;
	n->pred			  = l->tailpred;
	l->tailpred->succ = n;
	l->tailpred		  = n;
	l->cnt			 += 1;
}

struct node *list_remhead( struct list *l )
{
	struct node *n;
	if ((n = l->head->succ))
	{
		n->pred = (struct node *) l;
		n		= l->head;
		l->head = n->succ;
		l->cnt -= 1;
	}
	return n;
}

struct node *list_remtail( struct list *l )
{
	struct node *n;
	if ((n = l->tail))
	{
		n->pred->succ = n->succ;
		n->succ->pred = n->pred;
		l->cnt		 -= 1;
	}
	return n;
}

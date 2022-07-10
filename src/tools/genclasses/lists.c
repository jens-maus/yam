/***************************************************************************

 GenClasses - MUI class dispatcher generator
 Copyright (C) 2001 by Andrew Bell <mechanismx@lineone.net>

 Contributed to the YAM Open Source Team as a special version
 Copyright (C) 2000-2022 YAM Open Source Team

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
  l->head     = (struct node *)&l->tail;
  l->tail     = NULL;
  l->tailpred = (struct node *)&l->head;
  l->cnt      = 0;
}

void list_saveitem( struct list *l, char *name, void *data )
{
  struct node *n;
  if(!name || !data) return;
  if (!(n = (struct node *) calloc(1, sizeof(struct node)))) return;
  n->name = name;
  n->data = data;
  list_addtail(l, n);
}

struct node *list_getnext( struct list *l, struct node *n, void **data_ptr )
{
  if (!n)
  {
    if(l->cnt == 0) return NULL;
    if((n = l->head))
    {
      if (data_ptr) *data_ptr = n->data;
    }

    return n;
  }
  if (!n->succ || !n->succ->succ) return NULL;
  n = n->succ;
  if (data_ptr) *data_ptr = n->data;
  return n;
}

struct node *list_findname( struct list *l, char *name )
{
  struct node *res = NULL;
  struct node *n;

  for(n = l->head; n != NULL && n->succ; n = n->succ)
  {
    if(n->name != NULL && strcmp(n->name, name) == 0)
    {
      res = n;
      break;
    }
  }

  return res;
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
    n->succ        = npred->succ;
    n->pred        = npred;
    npred->succ->pred = n;
    npred->succ      = n;
  }
  else
  {
    n->succ        = (struct node *)&l->tail;
    n->pred        = l->tailpred;
    l->tailpred->succ = n;
    l->tailpred      = n;
  }
  l->cnt += 1;
}

void list_remove( struct list *l, struct node *n )
{
  n->pred->succ = n->succ;
  n->succ->pred = n->pred;
  l->cnt     -= 1;
}


void list_addhead( struct list *l, struct node *n )
{
  n->succ      = l->head;
  n->pred      = (struct node *)&l->head;
  l->head->pred = n;
  l->head      = n;
  l->cnt     += 1;
}

void list_addtail( struct list *l, struct node *n )
{
  n->succ        = (struct node *)&l->tail;
  n->pred        = l->tailpred;
  l->tailpred->succ = n;
  l->tailpred      = n;
  l->cnt       += 1;
}

struct node *list_remhead( struct list *l )
{
  struct node *n;
  if ((n = l->head->succ))
  {
    n->pred = (struct node *)l;
    n    = l->head;
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
    l->cnt     -= 1;
  }
  return n;
}

void list_sort( struct list *l, int (* compare)( const struct node *, const struct node *) )
{
  struct list list[2], *from, *to;
  struct node *ln;
  int insize;

  from = &list[0];
  to = &list[1];
  list_init(from);
  list_init(to);

  while((ln = list_remhead(l)) != NULL)
  {
    list_addhead(from, ln);
  }

  insize = 1;

  while(1)
  {
    int nmerges, psize, qsize, i;
    struct node *p, *q, *e;

    list_init(to);
    p = from->head;
    nmerges = 0;

    while(p->succ != NULL)
    {
      nmerges++;
      q = p;
      psize = 0;
      for(i = 0; i < insize; i++)
      {
        if(q->succ != NULL && q->succ->succ == NULL)
          break;

        q = q->succ;
        psize++;
      }

      qsize = insize;

      while(psize > 0 || (qsize > 0 && q->succ != NULL))
      {
        if(psize == 0)
        {
          e = q;
          q = q->succ;
          qsize--;
        }
        else if(qsize == 0 || q->succ == NULL)
        {
          e = p;
          p = p->succ;
          psize--;
        }
        else if(compare(p, q) <= 0)
        {
          e = p;
          p = p->succ;
          psize--;
        }
        else
        {
          e = q;
          q = q->succ;
          qsize--;
        }

        list_remove(from, e);
        list_addtail(to, e);
      }

      p = q;
    }

    if(nmerges <= 1)
    {
      break;
    }
    else
    {
      struct list *tmp;

      tmp = from;
      from = to;
      to = tmp;

      list_init(to);

      insize *= 2;
    }
  }

  list_init(l);
  while((ln = list_remhead(to)) != NULL)
  {
    list_addtail(l, ln);
  }
}


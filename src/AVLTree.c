/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2012 YAM Open Source Team

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

#include "AVLTree.h"

#include <proto/exec.h>
#include <exec/memory.h>

#include "extrasrc.h"

#include "YAM.h"

#include "Debug.h"

#define AVL_MAX_HEIGHT 92

/// CreateAVLTree
// create an AVL tree using 'compare' as comparison function for the single nodes
struct AVL_Tree *CreateAVLTree(int (*compare)(const void *a, const void *b))
{
  struct AVL_Tree *tree;

  ENTER();

  if((tree = AllocVecPooled(G->SharedMemPool, sizeof(*tree))) != NULL)
  {
    tree->root = NULL;

    // set up a semaphore
    if((tree->lock = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) != NULL)
    {
      tree->compare = compare;
    }
    else
    {
      FreeVecPooled(G->SharedMemPool, tree);
      tree = NULL;
    }
  }

  RETURN(tree);
  return tree;
}

///
/// DeleteAVLTree
// clear and delete an AVL tree
void DeleteAVLTree(struct AVL_Tree *tree)
{
  ENTER();

  // first remove all nodes
  ClearAVLTree(tree);

  // delete the semaphore
  FreeSysObject(ASOT_SEMAPHORE, tree->lock);

  // free the tree itself
  FreeVecPooled(G->SharedMemPool, tree);

  LEAVE();
}

///
/// ClearAVLTree
// remove all nodes from an AVL tree
void ClearAVLTree(struct AVL_Tree *tree)
{
  struct AVL_Node *p, *q;

  ENTER();

  ObtainSemaphore(tree->lock);

  // iterate over all nodes in the tree
  for(p = tree->root; p != NULL; p = q)
  {
    if(p->link[0] == NULL)
    {
      q = p->link[1];
      // free the node's memory
      ItemPoolFree(G->avlNodeItemPool, p);
    }
    else
    {
      q = p->link[0];
      p->link[0] = q->link[1];
      q->link[1] = p;
    }
  }

  ReleaseSemaphore(tree->lock);

  LEAVE();
}

///
/// probe
// check if a node with the given key already exists and add a new node
// to the tree if that is not the case
static void **probe(struct AVL_Tree *tree, const void *key)
{
  struct AVL_Node *y, *z;  // Top node to update balance factor, and parent
  struct AVL_Node *p, *q;  // Iterator, and parent
  struct AVL_Node *n;      // Newly inserted node
  struct AVL_Node *w;      // New root of rebalanced subtree
  int dir;                // Direction to descend
  unsigned char da[AVL_MAX_HEIGHT]; // Cached comparison results
  int k = 0;                        // Number of cached results

  z = (struct AVL_Node *)&tree->root;
  y = tree->root;
  dir = 0;
  for(q = z, p = y; p != NULL; q = p, p = p->link[dir])
  {
    int cmp;

    cmp = tree->compare(key, p->key);
    if(cmp == 0)
      return &p->key;

    if(p->balance != 0)
    {
      z = q;
      y = p;
      k = 0;
    }
    da[k++] = dir = (cmp > 0);
  }

  if((n = ItemPoolAlloc(G->avlNodeItemPool)) == NULL)
    return NULL;

  q->link[dir] = n;
  n->link[0] = NULL;
  n->link[1] = NULL;
  n->key = (void *)key;
  n->balance = 0;

  if(y == NULL)
    return &n->key;

  for(p = y, k = 0; p != n; p = p->link[da[k]], k++)
  {
    if(da[k] == 0)
      p->balance--;
    else
      p->balance++;
  }

  // rebalance the tree
  if(y->balance == -2)
  {
    struct AVL_Node *x = y->link[0];

    if(x->balance == -1)
    {
      w = x;
      y->link[0] = x->link[1];
      x->link[1] = y;
      x->balance = 0;
      y->balance = 0;
    }
    else
    {
      w = x->link[1];
      x->link[1] = w->link[0];
      w->link[0] = x;
      y->link[0] = w->link[1];
      w->link[1] = y;
      if(w->balance == -1)
      {
        x->balance = 0;
        y->balance = +1;
      }
      else if(w->balance == 0)
      {
        x->balance = 0;
        y->balance = 0;
      }
      else
      {
        x->balance = -1;
        y->balance = 0;
      }

      w->balance = 0;
    }
  }
  else if(y->balance == +2)
  {
    struct AVL_Node *x = y->link[1];

    if(x->balance == +1)
    {
      w = x;
      y->link[1] = x->link[0];
      x->link[0] = y;
      x->balance = 0;
      y->balance = 0;
    }
    else
    {
      w = x->link[0];
      x->link[0] = w->link[1];
      w->link[1] = x;
      y->link[1] = w->link[0];
      w->link[0] = y;
      if(w->balance == +1)
      {
        x->balance = 0;
        y->balance = -1;
      }
      else if(w->balance == 0)
      {
        x->balance = 0;
        y->balance = 0;
      }
      else
      {
        x->balance = +1;
        y->balance = 0;
      }

      w->balance = 0;
    }
  }
  else
  {
    return &n->key;
  }

  z->link[y != z->link[0]] = w;

  return &n->key;
}

///
/// InsertInAVLTree
// insert a new node into the AVL tree
void *InsertInAVLTree(struct AVL_Tree *tree, const void *key)
{
  void *result;
  void **p;

  ENTER();

  ObtainSemaphore(tree->lock);

  p = probe(tree, key);
  if(p == NULL || *p == key)
    result = NULL;
  else
    result = *p;

  ReleaseSemaphore(tree->lock);

  RETURN(result);
  return result;
}

///
/// ReplaceInAVLTree
// replace an existing node's key in the AVL tree
void *ReplaceInAVLTree(struct AVL_Tree *tree, const void *key)
{
  void *result;
  void **p;

  ENTER();

  ObtainSemaphore(tree->lock);

  p = probe(tree, key);
  if(p == NULL || *p == key)
    result = NULL;
  else
  {
    result = *p;
    *p = (void *)key;
  }

  ReleaseSemaphore(tree->lock);

  RETURN(result);
  return result;
}

///
/// RemoveFromAVLTree
// remove a node from an AVL tree
void *RemoveFromAVLTree(struct AVL_Tree *tree, const void *key)
{
  struct AVL_Node *pa[AVL_MAX_HEIGHT]; // nodes
  unsigned char da[AVL_MAX_HEIGHT];   // link indexes
  int k;                              // stack pointer
  struct AVL_Node *p; // Traverses tree to find node to delete
  int cmp;           // Result of comparison between key and p
  void *removedKey = NULL;

  ENTER();

  ObtainSemaphore(tree->lock);

  k = 0;
  p = (struct AVL_Node *)&tree->root;
  for(cmp = -1; cmp != 0; cmp = tree->compare(key, p->key))
  {
    int dir = cmp > 0;

    pa[k] = p;
    da[k++] = dir;

    p = p->link[dir];
    if(p == NULL)
      break;
  }

  if(p != NULL)
  {
    removedKey = p->key;

    if(p->link[1] == NULL)
      pa[k - 1]->link[da[k - 1]] = p->link[0];
    else
    {
      struct AVL_Node *r = p->link[1];

      if(r->link[0] == NULL)
      {
        r->link[0] = p->link[0];
        r->balance = p->balance;
        pa[k - 1]->link[da[k - 1]] = r;
        da[k] = 1;
        pa[k++] = r;
      }
      else
      {
        struct AVL_Node *s;
        int j = k++;

        for(;;)
        {
          da[k] = 0;
          pa[k++] = r;
          s = r->link[0];
          if(s->link[0] == NULL)
            break;

          r = s;
        }

        s->link[0] = p->link[0];
        r->link[0] = s->link[1];
        s->link[1] = p->link[1];
        s->balance = p->balance;

        pa[j - 1]->link[da[j - 1]] = s;
        da[j] = 1;
        pa[j] = s;
      }
    }

    ItemPoolFree(G->avlNodeItemPool, p);

    while(--k > 0)
    {
      struct AVL_Node *y = pa[k];

      if(da[k] == 0)
      {
        y->balance++;
        if(y->balance == +1)
          break;
        else if(y->balance == +2)
        {
          struct AVL_Node *x = y->link[1];

          if(x->balance == -1)
          {
            struct AVL_Node *w;

            w = x->link[0];
            x->link[0] = w->link[1];
            w->link[1] = x;
            y->link[1] = w->link[0];
            w->link[0] = y;
            if(w->balance == +1)
            {
              x->balance = 0;
              y->balance = -1;
            }
            else if(w->balance == 0)
            {
              x->balance = 0;
              y->balance = 0;
            }
            else /* |w->balance == -1| */
            {
              x->balance = +1;
              y->balance = 0;
            }
            w->balance = 0;
            pa[k - 1]->link[da[k - 1]] = w;
          }
          else
          {
            y->link[1] = x->link[0];
            x->link[0] = y;
            pa[k - 1]->link[da[k - 1]] = x;
            if(x->balance == 0)
            {
              x->balance = -1;
              y->balance = +1;
              break;
            }
            else
              x->balance = y->balance = 0;
          }
        }
      }
      else
      {
        y->balance--;
        if(y->balance == -1)
          break;
        else if(y->balance == -2)
        {
          struct AVL_Node *x = y->link[0];

          if(x->balance == +1)
          {
            struct AVL_Node *w;

            w = x->link[1];
            x->link[1] = w->link[0];
            w->link[0] = x;
            y->link[0] = w->link[1];
            w->link[1] = y;
            if(w->balance == -1)
            {
              x->balance = 0;
              y->balance = +1;
            }
            else if(w->balance == 0)
            {
              x->balance = 0;
              y->balance = 0;
            }
            else /* |w->balance == +1| */
            {
              x->balance = -1;
              y->balance = 0;
            }
            w->balance = 0;
            pa[k - 1]->link[da[k - 1]] = w;
          }
          else
          {
            y->link[0] = x->link[1];
            x->link[1] = y;
            pa[k - 1]->link[da[k - 1]] = x;
            if(x->balance == 0)
            {
              x->balance = +1;
              y->balance = -1;
              break;
            }
            else
            {
              x->balance = 0;
              y->balance = 0;
            }
          }
        }
      }
    }
  }

  ReleaseSemaphore(tree->lock);

  RETURN(removedKey);
  return removedKey;
}

///
/// FindInAVLTree
// search for 'needle' in the hay stack 'tree'
void *FindInAVLTree(struct AVL_Tree *tree, const void *needle)
{
  struct AVL_Node *p;
  void *result = NULL;

  ENTER();

  ObtainSemaphoreShared(tree->lock);

  for(p = tree->root; p != NULL; )
  {
    int cmp = tree->compare(needle, p->key);

    if(cmp < 0)
      p = p->link[0];
    else if(cmp > 0)
      p = p->link[1];
    else
    {
      result = p->key;
      break;
    }
  }

  ReleaseSemaphore(tree->lock);

  RETURN(result);
  return result;
}

///

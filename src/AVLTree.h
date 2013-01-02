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

/*
This is a custom implementation of balanced AVL trees based on libavl.
See http://www.stanford.edu/~blp/avl/ for details.

This implementation is necessary, because Exec's AVL trees are a addition of
AmigaOS 3.5, but since YAM only requires AmigaOS 3.0 we cannot rely on those
functions to be available.
*/

#ifndef AVL_TREE_H
#define AVL_TREE_H 1

#include <exec/semaphores.h>

struct AVL_Node
{
  struct AVL_Node *link[2]; // the left and right children of this node
  signed char balance;      // balance value, either -1, 0 or +1
  void *key;                // a user supplied key value for comparison
};

struct AVL_Tree
{
  struct AVL_Node *root;                        // the root node of the tree
  int (*compare)(const void *a, const void *b); // the comparison function
  struct SignalSemaphore *lock;                 // a semaphore to allow concurrent access
};

struct AVL_Tree *CreateAVLTree(int (*compare)(const void *a, const void *b));
void DeleteAVLTree(struct AVL_Tree *tree);
void ClearAVLTree(struct AVL_Tree *tree);
void *InsertInAVLTree(struct AVL_Tree *tree, const void *key);
void *ReplaceInAVLTree(struct AVL_Tree *tree, const void *key);
void *RemoveFromAVLTree(struct AVL_Tree *tree, const void *key);
void *FindInAVLTree(struct AVL_Tree *tree, const void *needle);

#endif /* AVL_TREE_H */

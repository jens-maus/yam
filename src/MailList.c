/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 by Marcel Beck <mbeck@yam.ch>
 Copyright (C) 2000-2008 by YAM Open Source Team

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

#include <stdlib.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "extrasrc.h"

#include "MailList.h"

#include "Debug.h"

/// CreateMailList
// create a new list for mails
struct MailList *CreateMailList(void)
{
  struct MailList *mlist;

  ENTER();

  // at first create the list itself
  if((mlist = AllocSysObjectTags(ASOT_LIST, ASOLIST_Size, sizeof(*mlist),
                                            ASOLIST_Min,  TRUE,
                                            TAG_DONE)) != NULL)
  {
    // now create the arbitration semaphore
    if((mlist->lockSemaphore = AllocSysObjectTags(ASOT_SEMAPHORE, TAG_DONE)) != NULL)
    {
      // no mails in the list so far
      mlist->count = 0;
    }
    else
    {
      // free the list again on failure
      FreeSysObject(ASOT_LIST, mlist);
      mlist = NULL;
    }
  }

  RETURN(mlist);
  return mlist;
}

///
/// DeleteMailList
// remove all nodes from a list and free it completely
void DeleteMailList(struct MailList *mlist)
{
  ENTER();

  if(mlist != NULL)
  {
    if(mlist->lockSemaphore != NULL)
    {
      struct MailNode *node;

      // lock the list just, just for safety reasons
      LockMailList(mlist);

      // remove and free all remaining nodes in the list
      while((node = (struct MailNode *)RemHead((struct List *)&mlist->list)) != NULL)
        FreeSysObject(ASOT_NODE, node);

      // unlock the list again
      UnlockMailList(mlist);

      // free the semaphore
      FreeSysObject(ASOT_SEMAPHORE, mlist->lockSemaphore);
      mlist->lockSemaphore = NULL;
    }

    // free the list itself
    FreeSysObject(ASOT_PORT, mlist);
  }

  LEAVE();
}

///
/// CloneMailList
// create a clone copy of a mail list
struct MailList *CloneMailList(struct MailList *mlist)
{
  struct MailList *clone = NULL;

  ENTER();

  if(mlist != NULL && (clone = CreateMailList()) != NULL)
  {
    struct MailNode *mnode;

    LockMailList(mlist);

    ForEachMailNode(mlist, mnode)
    {
      AddMailNode(clone, mnode->mail);
    }

    UnlockMailList(mlist);
  }

  RETURN(clone);
  return clone;
}

///
/// AddMailNode
struct MailNode *AddMailNode(struct MailList *mlist, struct Mail *mail)
{
  struct MailNode *mnode = NULL;

  ENTER();

  // we only accept existing mails
  if(mail != NULL && mlist != NULL)
  {
    if((mnode = AllocSysObjectTags(ASOT_NODE, ASONODE_Size, sizeof(*mnode),
                                              ASONODE_Min,  TRUE,
                                              TAG_DONE)) != NULL)
    {
      // initialize the node's contents
      mnode->mail = mail;

      // add the new mail node to the end of the list
      AddTail((struct List *)&mlist->list, (struct Node *)&mnode->node);

      // and increase the counter
      mlist->count++;
    }
  }

  // return the new mail node in case someone is interesed in it
  RETURN(mnode);
  return mnode;
}

///
/// RemoveMailNode
void RemoveMailNode(struct MailList *mlist, struct MailNode *mnode)
{
  ENTER();

  // remove the mail node from the list
  Remove((struct Node *)&mnode->node);

  // and decrease the counter
  mlist->count--;

  LEAVE();
}

///
/// LockMailList
void LockMailList(struct MailList *list)
{
  ENTER();

  ObtainSemaphore(list->lockSemaphore);

  LEAVE();
}

///
/// UnlockMailList
void UnlockMailList(struct MailList *mlist)
{
  ENTER();

  ReleaseSemaphore(mlist->lockSemaphore);

  LEAVE();
}

///
/// SortMailList
// sort a list of mails with a comparison function
void SortMailList(struct MailList *mlist, int (* compare)(const struct Mail *m1, const struct Mail *m2))
{
  ENTER();

  // sort only if there is something to sort at all
  if(mlist->count > 1)
  {
    // we use standard lists and node instead of MinLists and MinNodes to avoid too
    // much type casting, which in fact only pleases the compiler
    struct List list[2];
    struct List *from;
    struct List *to;
    struct Node *node;
    LONG insize;

    from = &list[0];
    to = &list[1];

    NewList(from);
    // move all nodes to the source list
    while((node = RemHead((struct List *)&mlist->list)) != NULL)
    {
      AddTail(from, node);
    }

    // we start sorting with lists of one node max
    insize = 1;

    while(TRUE)
    {
      LONG nmerges;
      LONG psize;
      LONG qsize;
      struct Node *p;
      struct Node *q;
      struct Node *e;

      // initialize the destination list
      NewList(to);
      p = from->lh_Head;
      // count number of merges we do in this pass
      nmerges = 0;

      while(p->ln_Succ != NULL)
      {
        LONG i;

        // there exists a merge to be done
        nmerges++;
        // step insize places along from p
        q = p;
        psize = 0;
        for(i = 0; i < insize; i++)
        {
          if(q->ln_Succ != NULL && q->ln_Succ->ln_Succ == NULL)
            break;

          q = q->ln_Succ;
          psize++;
        }

        // if q hasn't fallen off end, we have two lists to merge
        qsize = insize;

        // now we have two lists; merge them
        while(psize > 0 || (qsize > 0 && q->ln_Succ != NULL))
        {
          // decide whether next element of merge comes from p or q
          if(psize == 0)
          {
            // p is empty; e must come from q
            e = q;
            q = q->ln_Succ;
            qsize--;
          }
          else if(qsize == 0 || q->ln_Succ == NULL)
          {
            // q is empty; e must come from p
            e = p;
            p = p->ln_Succ;
            psize--;
          }
          else if(compare(((struct MailNode *)p)->mail, ((struct MailNode *)q)->mail) <= 0)
          {
            // first element of p is lower (or same); e must come from p
            e = p;
            p = p->ln_Succ;
            psize--;
          }
          else
          {
            // first element of q is lower; e must come from q
            e = q;
            q = q->ln_Succ;
            qsize--;
          }

          // add the next element to the merged list
          Remove(e);
          AddTail(to, e);
        }

        // now p has stepped insize places along, and q has too
        p = q;
      }

      // if we have done only one merge at most, we're finished
      if(nmerges <= 1)
      {
        break;
      }
      else
      {
        struct List *tmp;

        // otherwise repeat, merging lists twice the size
        tmp = from;
        from = to;
        to = tmp;

        NewList(to);
        insize *= 2;
      }
    }

    // put all the sorted nodes back into the original list
    NewList((struct List *)&mlist->list);
    while((node = RemHead(to)) != NULL)
    {
      AddTail((struct List *)&mlist->list, node);
    }
  }

  LEAVE();
}

///

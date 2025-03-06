/***************************************************************************

 YAM - Yet Another Mailer
 Copyright (C) 1995-2000 Marcel Beck
 Copyright (C) 2000-2025 YAM Open Source Team

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

 YAM Official Support Site :  http://www.yam.ch/
 YAM OpenSource project    :  https://github.com/jens-maus/yam/

 NOTE:
 This implementation of AllocSysObject() and FreeSysObject() is inspired by
 Ilkka 'itix' Lehtoranta's AllocSysObject implementation available on Aminet
 (http://aminet.net/dev/c/AllocSysObject.lha). Some modifications had to be
 done, because we don't need the full range of possible object types here,
 or itix' implementation was incomplete in some ways (i.e. FreeSysObject(),
 RemPort()'s a public port automatically).

***************************************************************************/

#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/utility.h>
#include <clib/alib_protos.h>
#include <exec/interrupts.h>
#include <exec/io.h>
#include <exec/memory.h>
#include <exec/nodes.h>
#include <exec/ports.h>
#include <exec/semaphores.h>

#include "AllocSysObject.h"
#include "ItemPool.h"
#include "SDI_compiler.h"
#include "SDI_stdarg.h"

#include "YAM_utilities.h"

#define DEBUG_USE_MALLOC_REDEFINE
#include "Debug.h"

#include "extrasrc.h"

#if defined(NEED_ALLOCSYSOBJECT)

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack(2)
   #endif
#elif defined(__VBCC__)
   #pragma amiga-align
#endif

// internal structures for certain objects to keep the information what has been allocated
struct SysMsgPort
{
  LONG signal;
  STRPTR name;
  BOOL public;
  BOOL copy;
  struct MsgPort port;
};

struct SysSignalSemaphore
{
  STRPTR name;
  BOOL public;
  BOOL copy;
  struct SignalSemaphore semaphore;
};

#ifdef __GNUC__
   #ifdef __PPC__
    #pragma pack()
   #endif
#elif defined(__VBCC__)
   #pragma default-align
#endif

/// AllocSysObject
// allocate a system object just like OS4 does
// this function does not cover all the types of OS4, because some are simply not
// needed or cannot be simulated
// no ENTER/RETURN macro calls on purpose as this would blow up the trace log too much
APTR AllocSysObject(ULONG type, struct TagItem *tags)
{
  union {
    APTR pointer;
    struct IORequest *iorequest;
    struct Hook *hook;
    struct List *list;
    struct Node *node;
    struct MsgPort *port;
    struct Message *message;
    struct SignalSemaphore *semaphore;
    struct TagItem *taglist;
    APTR mempool;
    struct ItemPool *itempool;
    struct Interrupt *interrupt;
  } object;
  struct TagItem *tstate = tags;
  struct TagItem *tag;
  ULONG memFlags;

  object.pointer = NULL;

  memFlags = GetTagData(ASO_MemoryOvr, MEMF_ANY, tags);

  switch(type)
  {
    case ASOT_IOREQUEST:
    {
      ULONG size = sizeof(struct IORequest);
      struct MsgPort *port = NULL;
      struct IORequest *duplicate = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOIOR_Size:
              size = tag->ti_Data;
            break;

            case ASOIOR_ReplyPort:
              port = (struct MsgPort *)tag->ti_Data;
            break;

            case ASOIOR_Duplicate:
              duplicate = (struct IORequest *)tag->ti_Data;
            break;
          }
        }
      }

      // if no reply port is given but an existing IO request is to be duplicated,
      // then we will use its reply port instead
      if(duplicate != NULL)
      {
        if(size == sizeof(struct IORequest))
          size = duplicate->io_Message.mn_Length;
        if(port == NULL)
          port = duplicate->io_Message.mn_ReplyPort;
      }

      // just create the IO request the usual way
      object.iorequest = CreateIORequest(port, size);
      if(object.iorequest != NULL && duplicate != NULL)
        CopyMem(duplicate, object.iorequest, size);
    }
    break;

    case ASOT_HOOK:
    {
      HOOKFUNC entry = NULL;
      HOOKFUNC subentry = NULL;
      HOOKFUNC data = NULL;
      ULONG size = sizeof(struct Hook);

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOHOOK_Size:
              size = tag->ti_Data;
            break;

            case ASOHOOK_Entry:
              entry = (HOOKFUNC)tag->ti_Data;
            break;

            case ASOHOOK_Subentry:
              subentry = (HOOKFUNC)tag->ti_Data;
            break;

            case ASOHOOK_Data:
              data = (HOOKFUNC)tag->ti_Data;
            break;
          }
        }
      }

      if((object.hook = AllocVec(size, memFlags)) != NULL)
      {
        #if defined(__MORPHOS__) || defined(__AROS__)
        // MorphOS and AROS need to go through the standard HookEntry function
        // to pass the parameters in the correct registers. The normal entry
        // becomes the subentry in this case
        object.hook->h_Entry = (HOOKFUNC)HookEntry;
        object.hook->h_SubEntry = (entry != NULL) ? entry : subentry;
        #else
        object.hook->h_Entry = entry;
        object.hook->h_SubEntry = subentry;
        #endif
        object.hook->h_Data = data;
      }
    }
    break;

    case ASOT_LIST:
    {
      BOOL min = (BOOL)GetTagData(ASOLIST_Min, FALSE, tags);
      ULONG size;

      if(min == FALSE)
        size = GetTagData(ASOLIST_Size, sizeof(struct List), tags);
      else
        size = GetTagData(ASOLIST_Size, sizeof(struct MinList), tags);

      if((object.list = AllocVec(size, memFlags)) != NULL)
      {
        NewList(object.list);

        if(min == FALSE)
          object.list->lh_Type = GetTagData(ASOLIST_Type, NT_UNKNOWN, tags);
      }
    }
    break;

    case ASOT_NODE:
    {
      BOOL min = (BOOL)GetTagData(ASONODE_Min, FALSE, tags);
      ULONG size;

      if(min == FALSE)
        size = GetTagData(ASONODE_Size, sizeof(struct Node), tags);
      else
        size = GetTagData(ASONODE_Size, sizeof(struct MinNode), tags);

      if((object.node = AllocVec(size, memFlags)) != NULL)
      {
        object.node->ln_Succ = (struct Node *)0xffffffff;
        object.node->ln_Pred = (struct Node *)0xffffffff;

        if(min == FALSE)
        {
          object.node->ln_Type = GetTagData(ASONODE_Type, NT_UNKNOWN, tags);
          object.node->ln_Pri = GetTagData(ASONODE_Pri, 0, tags);
          object.node->ln_Name = (STRPTR)GetTagData(ASONODE_Name, (IPTR)NULL, tags);
        }
      }
    }
    break;

    case ASOT_PORT:
    {
      STRPTR name = NULL;
      ULONG size = sizeof(struct MsgPort);
      ULONG action = 0;
      LONG pri = 0;
      LONG signum = -1;
      APTR target = NULL;
      BOOL allocsig = TRUE;
      BOOL public = FALSE;
      BOOL copy = FALSE;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch (tag->ti_Tag)
          {
            case ASOPORT_Size:
              size = MAX(size, tag->ti_Data);
            break;

            case ASOPORT_AllocSig:
              allocsig = tag->ti_Data;
            break;

            case ASOPORT_Action:
              action = tag->ti_Data;
            break;

            case ASOPORT_Pri:
              pri = tag->ti_Data;
            break;

            case ASOPORT_Name:
              name = (STRPTR)tag->ti_Data;
            break;

            case ASOPORT_Signal:
              signum = tag->ti_Data;
            break;

            case ASOPORT_Target:
              target = (APTR)tag->ti_Data;
            break;

            case ASOPORT_Public:
              public = tag->ti_Data;
            break;

            case ASOPORT_CopyName:
              copy = tag->ti_Data;
            break;
          }
        }
      }

      // add our own data size to the allocation
      size += sizeof(struct SysMsgPort) - sizeof(struct MsgPort);

      if((object.port = AllocVec(size, memFlags)) != NULL)
      {
        struct SysMsgPort *sobject = (struct SysMsgPort *)object.port;

        sobject->name = NULL;
        sobject->signal = -1;
        sobject->public = public;
        sobject->copy = copy;

        // allocate a signal if needed
        if(signum == -1 || allocsig != FALSE)
        {
          if((signum = AllocSignal(signum)) < 0)
          {
            FreeVec(object.port);
            object.port = NULL;
            goto done;
          }

          sobject->signal = signum;
        }

        // duplicate the given name if requested
        if(copy != FALSE && name != NULL)
        {
          name = strdup(name);
          sobject->name = name;
        }

        NewList(&sobject->port.mp_MsgList);
        sobject->port.mp_SigTask = (target != NULL) ? (struct Task *)target : FindTask(NULL);
        sobject->port.mp_Node.ln_Name = name;
        sobject->port.mp_Node.ln_Pri = pri;
        sobject->port.mp_Node.ln_Type = NT_MSGPORT;
        sobject->port.mp_Flags = action;
        sobject->port.mp_SigBit = signum;

        // make the port public if requested
        if(public != FALSE && name != NULL)
          AddPort(&sobject->port);

        object.port = &sobject->port;
      }
    }
    break;

    case ASOT_MESSAGE:
    {
      STRPTR name = NULL;
      ULONG size = sizeof(struct Message);
      ULONG length = 0;
      APTR port = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOMSG_Size:
              size = MAX(size, tag->ti_Data);
            break;

            case ASOMSG_ReplyPort:
              port = (APTR)tag->ti_Data;
            break;

            case ASOMSG_Length:
              length = MAX(length, tag->ti_Data);
            break;

            case ASOMSG_Name:
              name = (STRPTR)tag->ti_Data;
            break;
          }
        }
      }

      if(length == 0)
        length = size;

      if((object.message = AllocVec(size, memFlags)) != NULL)
      {
        object.message->mn_Node.ln_Name = name;
        object.message->mn_Node.ln_Type = NT_MESSAGE;
        object.message->mn_ReplyPort = port;
        object.message->mn_Length = length;
      }
    }
    break;

    case ASOT_SEMAPHORE:
    {
      ULONG size = sizeof(struct SignalSemaphore);
      LONG pri = 0;
      BOOL public = FALSE;
      BOOL copy = FALSE;
      STRPTR name = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOSEM_Size:
              size = MAX(size, tag->ti_Data);
            break;

            case ASOSEM_Name:
              name = (STRPTR)tag->ti_Data;
            break;

            case ASOSEM_Pri:
              pri = tag->ti_Data;
            break;

            case ASOSEM_Public:
              public = tag->ti_Data;
            break;

            case ASOSEM_CopyName:
              copy = tag->ti_Data;
            break;
          }
        }
      }

      // add our own data size to the allocation
      size += sizeof(struct SysSignalSemaphore) - sizeof(struct SignalSemaphore);

      if((object.semaphore = AllocVec(size, memFlags)) != NULL)
      {
        struct SysSignalSemaphore *sobject = (struct SysSignalSemaphore *)object.semaphore;

        sobject->name = NULL;
        sobject->public = public;
        sobject->copy = copy;

        // duplicate the given name if requested
        if(copy != FALSE && name != NULL)
        {
          name = strdup(name);
          sobject->name = name;
        }

        // initialize the semaphore
        memset(&sobject->semaphore, 0, sizeof(sobject->semaphore));
        InitSemaphore(&sobject->semaphore);
        sobject->semaphore.ss_Link.ln_Pri = pri;
        sobject->semaphore.ss_Link.ln_Name = name;

        // make the port public if requested
        if(public != FALSE && name != NULL)
          AddSemaphore(&sobject->semaphore);

        object.semaphore = &sobject->semaphore;
      }
    }
    break;

    case ASOT_TAGLIST:
    {
      ULONG entries = 0;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOTAGS_NumEntries:
              entries = tag->ti_Data;
            break;
          }
        }
      }

      object.taglist = AllocVec(entries * sizeof(struct TagItem), memFlags);
    }
    break;

    case ASOT_MEMPOOL:
    {
      ULONG memFlags = MEMF_ANY;
      ULONG puddle = 8192;
      ULONG thresh = 8000;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOPOOL_MFlags:
              memFlags = tag->ti_Data;
            break;

            case ASOPOOL_Puddle:
              puddle = tag->ti_Data;
            break;

            case ASOPOOL_Threshold:
              thresh = tag->ti_Data;
            break;

            // named pools are not possible with OS3, so we simply ignore the name
          }
        }

        if(thresh > puddle)
          puddle = thresh;
      }

      object.mempool = CreatePool(memFlags, puddle, thresh);
    }
    break;

    case ASOT_ITEMPOOL:
    {
      ULONG memFlags = MEMF_ANY;
      ULONG itemSize = 8;
      ULONG batchSize = 32;
      BOOL protected = FALSE;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOITEM_MFlags:
              memFlags = tag->ti_Data;
            break;

            case ASOITEM_ItemSize:
              itemSize = MAX(8, tag->ti_Data);
            break;

            case ASOITEM_BatchSize:
              batchSize = tag->ti_Data;
            break;

            case ASOITEM_Protected:
              protected = tag->ti_Data;
            break;

            // all other tags are ignored as they cannot be emulated with OS3's Exec pools
          }
        }
      }

      if((object.itempool = AllocVec(sizeof(struct ItemPool), MEMF_CLEAR)) != NULL)
      {
        #if !defined(__amigaos3__)
        // let the system handle the semaphore protection
        if(protected != FALSE)
          memFlags |= MEMF_SEM_PROTECTED;
        #endif

        if((object.itempool->pool = CreatePool(memFlags, batchSize*itemSize, batchSize*itemSize)) != NULL)
        {
          object.itempool->itemSize = itemSize;
          #if defined(__amigaos3__)
          object.itempool->protected = protected;
          InitSemaphore(&object.itempool->semaphore);
          #endif
        }
        else
        {
          FreeVec(object.itempool);
          object.itempool = NULL;
        }
      }
    }
    break;

    case ASOT_INTERRUPT:
    {
      ULONG size = sizeof(struct Interrupt);
      void (*code)() = NULL;
      void (*data)() = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem((APTR)&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOINTR_Size:
              size = tag->ti_Data;
            break;

            case ASOINTR_Code:
              code = (void (*)())tag->ti_Data;
            break;

            case ASOINTR_Data:
              data = (void (*)())tag->ti_Data;
            break;
          }
        }
      }

      if((object.interrupt = AllocVec(size, memFlags)) != NULL)
      {
        object.interrupt->is_Code = code;
        object.interrupt->is_Data = data;
        object.interrupt->is_Node.ln_Type = NT_INTERRUPT;
      }
    }
    break;

    default:
    {
      // ignored
    }
    break;
  }

done:

  return object.pointer;
}

///
/// FreeSysObject
// free a system object and perform basic cleanups depending on the type
// no ENTER/RETURN macro calls on purpose as this would blow up the trace log too much
void FreeSysObject(ULONG type, APTR object)
{
  if(object != NULL)
  {
    switch(type)
    {
      case ASOT_PORT:
      {
        struct SysMsgPort *sobject = (struct SysMsgPort *)((IPTR)object - OFFSET_OF(struct SysMsgPort, port));

        // remove the port from the public list, if it was public
        if(sobject->public != FALSE)
          RemPort(&sobject->port);

        // free the name memory, if it was duplicated
        if(sobject->copy != FALSE)
          free(sobject->name);

        // free the signal
        if(sobject->signal != -1)
          FreeSignal(sobject->signal);

        #if defined(DEBUG)
        if(IsListEmpty(&sobject->port.mp_MsgList) == FALSE)
          W(DBF_UTIL, "freeing MsgPort %08lx with pending messages", &sobject->port);
        #endif

        FreeVec(sobject);
      }
      break;

      case ASOT_IOREQUEST:
      {
        DeleteIORequest(object);
      }
      break;

      case ASOT_HOOK:
      {
        FreeVec(object);
      }
      break;

      case ASOT_LIST:
      {
        #if defined(DEBUG)
        if(IsListEmpty((struct List *)object) == FALSE)
          W(DBF_UTIL, "freeing non-empty list %08lx", object);
        #endif

        FreeVec(object);
      }
      break;

      case ASOT_NODE:
      {
        FreeVec(object);
      }
      break;

      case ASOT_MESSAGE:
      {
        FreeVec(object);
      }
      break;

      case ASOT_TAGLIST:
      {
        FreeVec(object);
      }
      break;

      case ASOT_SEMAPHORE:
      {
        struct SysSignalSemaphore *sobject = (struct SysSignalSemaphore *)((IPTR)object - OFFSET_OF(struct SysSignalSemaphore, semaphore));

        // remove the semaphore from the public list, if it was public
        if(sobject->public != FALSE)
          RemSemaphore(&sobject->semaphore);

        // free the name memory, if it was duplicated
        if(sobject->copy != FALSE)
          free(sobject->name);

        FreeVec(sobject);
      }
      break;

      case ASOT_MEMPOOL:
      {
        DeletePool(object);
      }
      break;

      case ASOT_ITEMPOOL:
      {
        struct ItemPool *sobject = (struct ItemPool *)object;

        DeletePool(sobject->pool);
        FreeVec(sobject);
      }
      break;

      case ASOT_INTERRUPT:
      {
        FreeVec(object);
      }
      break;

      default:
      {
        // ignored
      }
      break;
    }
  }
}

#else
  #warning "NEED_ALLOCSYSOBJECT missing or compilation unnecessary"
#endif

///

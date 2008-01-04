#include <proto/exec.h>
#include <proto/utility.h>

#include "AllocSysObject.h"
#include "SDI_compiler.h"

#include "Debug.h"

/*
This implementation of AllocSysObject() and FreeSysObject() is inspired by Ilkka 'itix' Lehtoranta's
AllocSysObject implementation available on Aminet (http://aminet.net/dev/c/AllocSysObject.lha)
Some modifications had to be done, because we don't need the full range of possible object types here,
or itix' implementation was incomplete in some ways (i.e. FreeSysObject() RemPort()'s a public port
automatically).
*/

#define MEMF_MASK   (MEMF_CLEAR|MEMF_LARGEST|MEMF_REVERSE|MEMF_TOTAL|MEMF_NO_EXPUNGE)

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
APTR AllocSysObject(ULONG type, struct TagItem *tags)
{
  APTR object = NULL;
  struct TagItem *tstate, *tag;

  ENTER();

  tstate  = tags;

  switch(type)
  {
    case ASOT_IOREQUEST :
    {
      ULONG size = sizeof(struct IORequest);
      APTR port = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOIOR_Size:
              size = tag->ti_Data;
            break;

            case ASOIOR_ReplyPort:
              port = (APTR)tag->ti_Data;
            break;
          }
        }
      }

      // just create the IO request the usual way
      object = CreateIORequest(port, size);
    }
    break;

    case ASOT_HOOK:
    {
      APTR entry = NULL;
      APTR subentry = NULL;
      APTR data = NULL;
      ULONG size = sizeof(struct Hook);

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOHOOK_Size:
              size = tag->ti_Data;
            break;

            case ASOHOOK_Entry:
              entry = (APTR)tag->ti_Data;
            break;

            case ASOHOOK_Subentry:
              subentry = (APTR)tag->ti_Data;
            break;
            
            case ASOHOOK_Data:
              data = (APTR)tag->ti_Data;
            break;
          }
        }
      }

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        ((struct Hook *)object)->h_Entry = entry;
        ((struct Hook *)object)->h_SubEntry = subentry;
        ((struct Hook *)object)->h_Data = data;
      }
    }
    break;

    case ASOT_LIST:
    {
      ULONG size = sizeof(struct List);
      ULONG type = 0;
      BOOL min = FALSE;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOLIST_Size:
              size = tag->ti_Data;
            break;

            case ASOLIST_Type:
              type = tag->ti_Data;
            break;

            case ASOLIST_Min:
              min = tag->ti_Data;
            break;
          }
        }
      }

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        NewList((struct List *)object);

        if(min == FALSE)
          ((struct List *)object)->lh_Type = type;
      }
    }
    break;

    case ASOT_NODE:
    {
      STRPTR name = NULL;
      ULONG size  = sizeof(struct Node);
      ULONG type = 0;
      LONG pri = 0;
      BOOL min = FALSE;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASONODE_Size:
              size = tag->ti_Data;
            break;

            case ASONODE_Min:
              min = tag->ti_Data;
            break;

            case ASONODE_Type:
              type = tag->ti_Data;
            break;

            case ASONODE_Pri:
              pri = tag->ti_Data;
            break;

            case ASONODE_Name:
              name = (STRPTR)tag->ti_Data;
            break;
          }
        }
      }

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        if(min == FALSE)
        {
          ((struct Node *)object)->ln_Type = type;
          ((struct Node *)object)->ln_Pri = pri;
          ((struct Node *)object)->ln_Name = name;
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
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch (tag->ti_Tag)
          {
              case ASOPORT_Size:
                size = tag->ti_Data;
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

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        struct SysMsgPort *sobject = (struct SysMsgPort *)object;

        sobject->name = NULL;
        sobject->signal = -1;
        sobject->public = public;
        sobject->copy = copy;

        // allocate a signal if needed
        if(signum == -1 || allocsig != FALSE)
        {
          if((signum = AllocSignal(signum)) < 0)
          {
            FreeVec(object);
            object = NULL;
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

        object = &sobject->port;
      }
    }
    break;

    case ASOT_MESSAGE:
    {
      STRPTR name = NULL;
      ULONG size = sizeof(struct Message);
      APTR port = NULL;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOMSG_Size:
              size = tag->ti_Data;
            break;
            
            case ASOMSG_ReplyPort:
              port = (APTR)tag->ti_Data;
            break;

            case ASOMSG_Length:
              size = tag->ti_Data;
            break;

            case ASOMSG_Name:
              name = (STRPTR)tag->ti_Data;
            break;
          }
        }
      }

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        ((struct Message *)object)->mn_Node.ln_Type = NT_MESSAGE;
        ((struct Message *)object)->mn_ReplyPort = port;
        ((struct Message *)object)->mn_Length = size;
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
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOSEM_Size:
              size = tag->ti_Data;
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

      if((object = AllocVec(size, MEMF_CLEAR)) != NULL)
      {
        struct SysSignalSemaphore *sobject = (struct SysSignalSemaphore *)object;

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
        InitSemaphore(&sobject->semaphore);
        sobject->semaphore.ss_Link.ln_Pri = pri;
        sobject->semaphore.ss_Link.ln_Name = name;

        // make the port public if requested
        if(public != FALSE && name != NULL)
          AddSemaphore(&sobject->semaphore);

        object = &sobject->semaphore;
      }
    }
    break;

    case ASOT_TAGLIST:
    {
      ULONG entries = 0;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOTAGS_NumEntries:
              entries = tag->ti_Data;
            break;
          }
        }
      }

      object = AllocVec(entries * sizeof(struct TagItem), MEMF_CLEAR);
    }
    break;

    case ASOT_MEMPOOL:
    {
      ULONG flags = MEMF_ANY;
      ULONG puddle = 8192;
      ULONG thresh = 8000;

      if(tags != NULL)
      {
        while((tag = NextTagItem(&tstate)) != NULL)
        {
          switch(tag->ti_Tag)
          {
            case ASOPOOL_MFlags:
              flags = tag->ti_Data & MEMF_MASK;
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

      object = CreatePool(flags, puddle, thresh);
    }
    break;
  }

done:
  RETURN(object);
  return object;
}

///
/// AllocSysObjectTags
// varargs stub for AllocSysObject() for 68k
#if !defined(PPC)
APTR VARARGS68K AllocSysObjectTags(ULONG type, ...)
{
  VA_LIST args;
  APTR object;

  ENTER();

  VA_START(args, type);
  object = _AllocSysObject(type, VA_ARG(args, struct TagItem *));
  VA_END(args);

  RETURN(object);
  return object;
}
#endif

///
/// FreeSysObject
// free a system object and perform basic cleanups depending on the type
void FreeSysObject(ULONG type, APTR object)
{
  ENTER();

  if(object != NULL)
  {
    switch(type)
    {
      case ASOT_PORT:
      {
        struct SysMsgPort *sobject = (struct SysMsgPort *)((STRPTR)object - (sizeof(struct SysMsgPort) - sizeof(struct MsgPort)));

        // remove the port from the public list, if it was public
        if(sobject->public != FALSE)
          RemPort(&sobject->port);

        // free the name memory, if it was duplicated
        if(sobject->copy != FALSE && sobject->name != NULL)
          free(sobject->name);

        // free the signal
        if(sobject->signal != -1)
          FreeSignal(sobject->signal);

        FreeVec(object);
      }
      break;

      case ASOT_IOREQUEST:
      {
        DeleteIORequest(object);
      }
      break;

      case ASOT_HOOK:
      case ASOT_LIST:
      case ASOT_NODE:
      case ASOT_MESSAGE:
      case ASOT_TAGLIST:
      {
        FreeVec(object);
      }
      break;

      case ASOT_SEMAPHORE:
      {
        struct SysSignalSemaphore *sobject = (struct SysSignalSemaphore *)((STRPTR)object - (sizeof(struct SysSignalSemaphore) - sizeof(struct SignalSemaphore)));

        // remove the semaphore from the public list, if it was public
        if(sobject->public != FALSE)
          RemSemaphore(&sobject->semaphore);

        // free the name memory, if it was duplicated
        if(sobject->copy != FALSE && sobject->name != NULL)
          free(sobject->name);

        FreeVec(object);
      }
      break;

      case ASOT_MEMPOOL:
      {
        DeletePool(object);
      }
      break;

      default:
      {
        // ignored
      }
      break;
    }
  }

  LEAVE();
}
///


#ifndef ALLOCSYSOBJECT_H
#define ALLOCSYSOBJECT_H 1

#include "extrasrc.h"

#if defined(NEED_ALLOCSYSOBJECT)

#include <utility/tagitem.h>

#define ASO_NoTrack         (TAG_USER + 1)
#define ASO_MemoryOvr       (TAG_USER + 2)

#define ASOIOR_Size         (TAG_USER + 10)
#define ASOIOR_ReplyPort    (TAG_USER + 11)
#define ASOIOR_Duplicate    (TAG_USER + 12)

#define ASOHOOK_Size        (TAG_USER + 10)
#define ASOHOOK_Entry       (TAG_USER + 11)
#define ASOHOOK_Subentry    (TAG_USER + 12)
#define ASOHOOK_Data        (TAG_USER + 13)

#define ASOLIST_Size        (TAG_USER + 10)
#define ASOLIST_Type        (TAG_USER + 11)
#define ASOLIST_Min         (TAG_USER + 12)

#define ASONODE_Size        (TAG_USER + 10)
#define ASONODE_Min         (TAG_USER + 11)
#define ASONODE_Type        (TAG_USER + 12)
#define ASONODE_Pri         (TAG_USER + 13)
#define ASONODE_Name        (TAG_USER + 14)

#define ASOPORT_Size        (TAG_USER + 10)
#define ASOPORT_AllocSig    (TAG_USER + 11)
#define ASOPORT_Action      (TAG_USER + 12)
#define ASOPORT_Pri         (TAG_USER + 13)
#define ASOPORT_Name        (TAG_USER + 14)
#define ASOPORT_Signal      (TAG_USER + 15)
#define ASOPORT_Target      (TAG_USER + 16)
#define ASOPORT_Public      (TAG_USER + 17)
#define ASOPORT_CopyName    (TAG_USER + 18)

#define ASOMSG_Size         (TAG_USER + 10)
#define ASOMSG_ReplyPort    (TAG_USER + 11)
#define ASOMSG_Length       (TAG_USER + 12)
#define ASOMSG_Name         (TAG_USER + 13)

#define ASOSEM_Size         (TAG_USER + 10)
#define ASOSEM_Name         (TAG_USER + 11)
#define ASOSEM_Pri          (TAG_USER + 12)
#define ASOSEM_Public       (TAG_USER + 13)
#define ASOSEM_CopyName     (TAG_USER + 14)

#define ASOTAGS_Size        (TAG_USER + 10)
#define ASOTAGS_NumEntries  (TAG_USER + 11)

#define ASOPOOL_MFlags      (TAG_USER + 10)
#define ASOPOOL_Puddle      (TAG_USER + 11)
#define ASOPOOL_Threshold   (TAG_USER + 12)
#define ASOPOOL_Name        (TAG_USER + 13)

#define ASOINTR_Size        (TAG_USER + 10)
#define ASOINTR_Code        (TAG_USER + 11)
#define ASOINTR_Data        (TAG_USER + 12)

#define ASOT_IOREQUEST      0
#define ASOT_HOOK           1
#define ASOT_LIST           3
#define ASOT_NODE           5
#define ASOT_PORT           6
#define ASOT_MESSAGE        7
#define ASOT_SEMAPHORE      8
#define ASOT_TAGLIST        9
#define ASOT_MEMPOOL        10
#define ASOT_INTERRUPT      11

#endif

#endif /* ALLOCSYSOBJECT_H */


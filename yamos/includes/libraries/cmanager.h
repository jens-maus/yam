#ifndef CMANAGER_H
#define CMANAGER_H

#include "amiga-align.h"

/*
    $VER: CManager.h 1.4 (30.5.98) © Simone Tellini
*/

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d)    ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif


#define CME_GROUP   0
#define CME_USER    1
#define CME_WWW     2
#define CME_FTP     3
#define CME_CHAT    4

#define CME_LIST    253
#define CME_IMAGE   254
#define CME_SECTION 255

struct CMGroup {
        struct CMGroup *Succ;
        struct CMGroup *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];
        struct MinList  Entries;
        struct MinList  SubGroups;
};

#define GRPF_BOLD       (1 << 0)
#define GRPF_OPEN       (1 << 1)

struct CMUser {
        struct CMUser  *Succ;
        struct CMUser  *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];       //  FirstName
        TEXT            Address[128];
        TEXT            City[128];
        TEXT            Country[80];
        TEXT            ZIP[20];
        TEXT            Comment[512];
        TEXT            Alias[40];
        TEXT            Phone[40];
        TEXT            Fax[40];
        TEXT            EMail[128];
        TEXT            WWW[256];
        TEXT            FTP[256];
        TEXT            LastName[80];
        TEXT            Mobile[40];
        struct CMImage *Image;
};

struct CMWWW {
        struct CMWWW   *Succ;
        struct CMWWW   *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];
        TEXT            WWW[256];
        TEXT            Comment[512];
        TEXT            WebMaster[80];
        TEXT            EMail[128];
};

struct CMFTP {
        struct CMFTP   *Succ;
        struct CMFTP   *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Name[80];
        TEXT            FTP[256];
        TEXT            Comment[512];
        TEXT            Username[60];
        TEXT            Password[60];
        ULONG           Port;
        ULONG           Retries;
        TEXT            Local[256];
};

#define FTPF_ADVANCED   (1 << 0)
#define FTPF_QUIET      (1 << 1)
#define FTPF_COMPRESS   (1 << 2)
#define FTPF_ADT        (1 << 3)
#define FTPF_ANONYMOUS  (1 << 4)
#define FTPF_LOCAL      (1 << 5)

struct CMChat {
        struct CMChat  *Succ;
        struct CMChat  *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        TEXT            Channel[128];
        TEXT            Server[128];
        TEXT            Maintainer[80];
        TEXT            Nick[20];
        TEXT            WWW[256];
        TEXT            Comment[512];
        TEXT            Password[60];
};


struct CMImage {
        struct CMImage *Succ;
        struct CMImage *Prec;
        UBYTE           Type;
        UBYTE           Flags;
        struct BitMap  *BitMap;
        APTR            Colors;
};


/*
 *          Structure to be passed to CM_LoadData(), CM_SaveData(), CM_FreeData()
 */

struct CMData {
        struct CMGroup *Users;
        struct CMGroup *WWWs;
        struct CMGroup *FTPs;
        struct CMGroup *Chat;
};

/*
 *          Flags for CM_GetEntry()
 */

#define CMGE_USER           (1 << 0)
#define CMGE_WWW            (1 << 1)
#define CMGE_FTP            (1 << 2)
#define CMGE_CHAT           (1 << 3)
#define CMGE_MULTISELECT    (1 << 4)    //  CM_GetEntry returns a struct MinList *

#include "default-align.h"

#endif /* CMANAGER_H */

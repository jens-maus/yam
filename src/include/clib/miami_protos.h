#ifndef  CLIB_MIAMI_PROTOS_H
#define  CLIB_MIAMI_PROTOS_H

/*
**  $VER: miami_protos.h 2.1 (10.07.97)
**
**  C prototypes. For use with 32 bit integers only.
**
**  (C) Copyright 1996 Holger Kruse
**      All Rights Reserved
**
** Adapted by the YAMOS team to only carry the miami.library specific
*/

long MiamiSysCtl(long *,unsigned long,void *,unsigned long *,void *,long);
void MiamiDisallowDNS(long);
void *MiamiGetPid(void);
void *MiamiPFAddHook(struct Hook *,unsigned char *,struct TagItem *);
void MiamiPFRemoveHook(void *);
long MiamiGetHardwareLen(char *);
struct Library *MiamiOpenSSL(struct TagItem *);
void MiamiCloseSSL(void);
long MiamiSetSocksConn(struct sockaddr *,long);
long MiamiIsOnline(char *);
void MiamiOnOffline(char *,long);
long MiamiSupportsIPV6(void);
long MiamiResGetOptions(void);
void MiamiResSetOptions(long);
void MiamiSupportedCPUs(unsigned long *,unsigned long *,unsigned long *);
long MiamiGetFdCallback(void **);
long MiamiSetFdCallback(void *,long);
struct UserGroupCredentials *MiamiGetCredentials(void);

#endif

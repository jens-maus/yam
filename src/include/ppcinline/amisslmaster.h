/* Automatically generated header! Do not edit! */

#ifndef _PPCINLINE_AMISSLMASTER_H
#define _PPCINLINE_AMISSLMASTER_H

#ifndef __PPCINLINE_MACROS_H
#include <ppcinline/macros.h>
#endif /* !__PPCINLINE_MACROS_H */

#ifndef AMISSLMASTER_BASE_NAME
#define AMISSLMASTER_BASE_NAME AmiSSLMasterBase
#endif /* !AMISSLMASTER_BASE_NAME */

#define InitAmiSSLMaster(__p0, __p1) \
  LP2(30, LONG , InitAmiSSLMaster, \
    LONG , __p0, d0, \
    LONG , __p1, d1, \
    , AMISSLMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CloseAmiSSLCipher(__p0) \
  LP1NR(54, CloseAmiSSLCipher, \
    struct Library *, __p0, a0, \
    , AMISSLMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OpenAmiSSL() \
  LP0(36, struct Library *, OpenAmiSSL, \
    , AMISSLMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define OpenAmiSSLCipher(__p0) \
  LP1(48, struct Library *, OpenAmiSSLCipher, \
    LONG , __p0, d0, \
    , AMISSLMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#define CloseAmiSSL() \
  LP0NR(42, CloseAmiSSL, \
    , AMISSLMASTER_BASE_NAME, 0, 0, 0, 0, 0, 0)

#endif /* !_PPCINLINE_AMISSLMASTER_H */

#ifndef PROTO_GENESIS_H
#define PROTO_GENESIS_H

/*
**  $Id: genesis.h 2024 2006-03-05 10:27:25Z damato $
**  Includes Release 50.1
**
**  Prototype/inline/pragma header file combo
**
**  (C) Copyright 2003-2004 Amiga, Inc.
**      All Rights Reserved
*/

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef LIBRARIES_GENESIS_H
#include <libraries/genesis.h>
#endif

/****************************************************************************/

#ifndef __NOLIBBASE__
extern struct Library * GenesisBase;
#endif /* __NOLIBBASE__ */

/****************************************************************************/

#ifdef __amigaos4__
 #include <interfaces/genesis.h>
 #ifdef __USE_INLINE__
  #include <inline4/genesis.h>
 #endif /* __USE_INLINE__ */
 #ifndef CLIB_GENESIS_PROTOS_H
  #define CLIB_GENESIS_PROTOS_H 1
 #endif /* CLIB_GENESIS_PROTOS_H */
 #ifndef __NOGLOBALIFACE__
  extern struct GenesisIFace *IGenesis;
 #endif /* __NOGLOBALIFACE__ */
#else /* __amigaos4__ */
 #ifndef CLIB_GENESIS_PROTOS_H
  #include <clib/genesis_protos.h>
 #endif /* CLIB_GENESIS_PROTOS_H */
 #if defined(__GNUC__)
  #ifndef __PPC__
   #include <inline/genesis.h>
  #else
   #include <ppcinline/genesis.h>
  #endif /* __PPC__ */
 #elif defined(__VBCC__)
  #ifndef __PPC__
   #include <inline/genesis_protos.h>
  #endif /* __PPC__ */
 #else
  #include <pragmas/genesis_pragmas.h>
 #endif /* __GNUC__ */
#endif /* __amigaos4__ */

/****************************************************************************/

#endif /* PROTO_GENESIS_H */

#ifndef PROTO_POPUPMENU_H
#define PROTO_POPUPMENU_H

#ifndef EXEC_TYPES_H
#include <exec/types.h>
#endif
#ifndef CLIB_POPUPMENU_PROTOS_H
#include <clib/pm_protos.h>
#endif

#ifndef __NOLIBBASE__
extern struct PopupMenuBase *
  #ifdef __CONSTLIBBASEDECL__
  __CONSTLIBBASEDECL__
  #endif /* __CONSTLIBBASEDECL__ */
PopupMenuBase;
#endif

#if defined(__GNUC__) || defined(__VBCC__)
#include <inline/pm.h>
#endif

#if defined(LATTICE) || defined(__SASC) || defined(_DCC) || defined(__STORM__)
#include <pragmas/pm_pragmas.h>
#endif

#endif  /*  PROTO_POPUPMENU_H  */


#ifndef _COMPILER_H
#define _COMPILER_H

/*
** first "exceptions"
*/

#if defined(__SASC)
  #if !defined(_M68060)
    #if !defined(_M68040)
      #if !defined(_M68030) && !defined(_M68020)
         #define __mc68000__
      #else
        #define __mc68020__
      #endif
    #else
      #define __mc68040__
    #endif
  #else
    #define __mc68060__
  #endif
  #if defined(_M68881)
    #define __HAVE_68881__
  #endif
//  #define ASM(arg) arg __asm
 #define ASM __asm
#elif defined(__VBCC__)
  #define REG(reg,arg) __reg(#reg) arg
  #define INLINE static
  #define REGARGS /**/
  #define STDARGS /**/
  #define STACKEXT /**/
#elif defined(__GNUC__)
  #define REG(reg,arg) arg __asm(#reg)
  #define __near
  #define LREG(reg,arg) register REG(reg,arg)
  #define ALIAS(a,b) __asm(".stabs \"_" #a "\",11,0,0,0\n\t.stabs \"_" #b "\",1,0,0,0");
#elif defined(_DCC)
  #define REG(reg,arg) __ ## reg arg
  #define STACKEXT __stkcheck
  #define STDARGS __stkargs
  #define INLINE static
#elif defined(__STORM__)
  #define STDARGS /**/
  #define REGARGS /**/
  #define STACKEXT /**/
  #define REG(reg,arg) arg
#endif

/*
** then "common" ones
*/

#if !defined(ASM)
//  #define ASM(arg) arg
 #define ASM
#endif
#if !defined(REG)
  #define REG(reg,arg) register __##reg arg
#endif
#if !defined(LREG)
  #define LREG(reg,arg) register arg
#endif
#if !defined(ALIAS)
  #define ALIAS(a,b)
#endif
#if !defined(CONST)
  #define CONST const
#endif
#if !defined(LOCAL)
  #define LOCAL static
#endif
#if !defined(SAVEDS)
  #define SAVEDS __saveds
#endif
#if !defined(INLINE)
  #define INLINE static __inline
#endif
#if !defined(REGARGS)
  #define REGARGS __regargs
#endif
#if !defined(STDARGS)
  #define STDARGS __stdargs
#endif
#if !defined(STACKEXT)
  #define STACKEXT __stackext
#endif
#if defined(__mc68020__) || defined(__mc68030__) || defined(__mc68040__) || defined(__mc68060__)
  #define PLAIN(x)
  #define REQUIRES_68020(x) ((x & AFF_68020) == 0)
#else
  #define REQUIRES_68020(x) (0)
  #define PLAIN(x) x
#endif

/*
** stacksize definitions
*/

#ifdef __STORMGCC__
  #define __YAM_STACK __stacksize
#else
  #define __YAM_STACK __stack
#endif

#endif /* _COMPILER_H */

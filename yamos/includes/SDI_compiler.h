#ifndef SDI_COMPILER_H
#define SDI_COMPILER_H

/* Includeheader

	Name:           SDI_compiler.h
	Versionstring:	$VER: SDI_compiler.h 1.8 (12.06.2002)
	Author:         SDI
	Distribution:   PD
	Description:    defines to hide compiler stuff

 1.1   25.06.98 : created from data made by Gunter Nikl
 1.2   17.11.99 : added VBCC
 1.3   29.02.00 : fixed VBCC REG define
 1.4   30.03.00 : fixed SAVEDS for VBCC
 1.5   29.07.00 : added #undef statements (needed e.g. for AmiTCP together with vbcc)
 1.6   19.05.01 : added STACKEXT and Dice stuff
 1.7   09.06.02 : added #ifdef check for __stackext as the cross-gcc of morphos doesn`t support stackext
 1.8   12.06.02 : added ALIGN and PACKED for PPC (MorphOS) support
*/

#ifdef ASM
#undef ASM
#endif
#ifdef REG
#undef REG
#endif
#ifdef LREG
#undef LREG
#endif
#ifdef CONST
#undef CONST
#endif
#ifdef SAVEDS
#undef SAVEDS
#endif
#ifdef INLINE
#undef INLINE
#endif
#ifdef REGARGS
#undef REGARGS
#endif
#ifdef STDARGS
#undef STDARGS
#endif

/* first "exceptions" */

#if defined(__MAXON__)
  #define STDARGS
  #define STACKEXT
  #define REGARGS
  #define SAVEDS
  #define INLINE inline
#elif defined(__VBCC__)
  #define STDARGS
  #define STACKEXT
  #define REGARGS
  #define INLINE
  #define REG(reg,arg) __reg(#reg) arg
#elif defined(__STORM__)
  #define STDARGS
  #define STACKEXT
  #define REGARGS
  #define INLINE inline
#elif defined(__SASC)
  #define ASM(arg) arg __asm
#elif defined(__GNUC__)
  #define REG(reg,arg) arg __asm(#reg)
  #define LREG(reg,arg) register REG(reg,arg)
  #if defined(__MORPHOS__)
    #define ALIGN __attribute__((aligned(2)))
    #define PACKED __attribute__((packed))
  #endif
#elif defined(_DCC)
  #define REG(reg,arg) __ ## reg arg
  #define STACKEXT __stkcheck
  #define STDARGS __stkargs
  #define INLINE static
#endif

/* then "common" ones */

#if !defined(ASM)
  #define ASM(arg) arg
#endif
#if !defined(REG)
  #define REG(reg,arg) register __##reg arg
#endif
#if !defined(LREG)
  #define LREG(reg,arg) register arg
#endif
#if !defined(CONST)
  #define CONST const
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
  #if defined(__stackext)         // the cross-gcc of morphos doesn`t support __stackext somehow :/
    #define STACKEXT __stackext
  #else
    #define STACKEXT
  #endif
#endif
#if !defined(ALIGN)
  #define ALIGN
#endif
#if !defined(PACKED)
  #define PACKED
#endif

#endif /* SDI_COMPILER_H */

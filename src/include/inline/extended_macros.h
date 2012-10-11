#ifndef __INLINE_EXTENDED_MACROS_H
#define __INLINE_EXTENDED_MACROS_H

#ifndef __INLINE_MACROS_H
#include <inline/macros.h>
#endif

// define some macros which are missing in inline/macros.h but are required by AmiSSL
#define LP1NRFP(offs, name, t1, v1, r1, bt, bn, fpt) \
({                                                              \
   typedef fpt;                                                 \
   t1 _##name##_v1 = (v1);                                      \
   {                                                            \
      register int _d0 __asm("d0");                             \
      register int _d1 __asm("d1");                             \
      register int _a0 __asm("a0");                             \
      register int _a1 __asm("a1");                             \
      register void *const _##name##_bn __asm("a6") = (bn);     \
      register t1 _n1 __asm(#r1) = _##name##_v1;                \
      __asm volatile ("jsr a6@(-"#offs":W)"                     \
      : "=r" (_d0), "=r" (_d1), "=r" (_a0), "=r" (_a1)          \
      : "r" (_##name##_bn), "rf"(_n1)                           \
      : "fp0", "fp1", "cc", "memory");                          \
   }                                                            \
})

#define LP2NRFP(offs, name, t1, v1, r1, t2, v2, r2, bt, bn, fpt) \
({                                                              \
   typedef fpt;                                                 \
   t1 _##name##_v1 = (v1);                                      \
   t2 _##name##_v2 = (v2);                                      \
   {                                                            \
      register int _d0 __asm("d0");                             \
      register int _d1 __asm("d1");                             \
      register int _a0 __asm("a0");                             \
      register int _a1 __asm("a1");                             \
      register void *const _##name##_bn __asm("a6") = (bn);     \
      register t1 _n1 __asm(#r1) = _##name##_v1;                \
      register t2 _n2 __asm(#r2) = _##name##_v2;                \
      __asm volatile ("jsr a6@(-"#offs":W)"                     \
      : "=r" (_d0), "=r" (_d1), "=r" (_a0), "=r" (_a1)          \
      : "r" (_##name##_bn), "rf"(_n1), "rf"(_n2)                \
      : "fp0", "fp1", "cc", "memory");                          \
   }                                                            \
})

#define LP5NRFP(offs, name, t1, v1, r1, t2, v2, r2, t3, v3, r3, t4, v4, r4, t5, v5, r5, bt, bn, fpt) \
({                                                              \
   typedef fpt;                                                 \
   t1 _##name##_v1 = (v1);                                      \
   t2 _##name##_v2 = (v2);                                      \
   t3 _##name##_v3 = (v3);                                      \
   t4 _##name##_v4 = (v4);                                      \
   t5 _##name##_v5 = (v5);                                      \
   {                                                            \
      register int _d0 __asm("d0");                             \
      register int _d1 __asm("d1");                             \
      register int _a0 __asm("a0");                             \
      register int _a1 __asm("a1");                             \
      register void *const _##name##_bn __asm("a6") = (bn);     \
      register t1 _n1 __asm(#r1) = _##name##_v1;                \
      register t2 _n2 __asm(#r2) = _##name##_v2;                \
      register t3 _n2 __asm(#r3) = _##name##_v3;                \
      register t4 _n2 __asm(#r4) = _##name##_v4;                \
      register t5 _n2 __asm(#r5) = _##name##_v5;                \
      __asm volatile ("jsr a6@(-"#offs":W)"                     \
      : "=r" (_d0), "=r" (_d1), "=r" (_a0), "=r" (_a1)          \
      : "r" (_##name##_bn), "rf"(_n1), "rf"(_n2), "rf"(_n3), "rf"(_n4), "rf"(_n5) \
      : "fp0", "fp1", "cc", "memory");                          \
   }                                                            \
})

#endif /* __INLINE_EXTENDED_MACROS_H */

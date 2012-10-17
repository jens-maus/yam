#ifndef __PPCINLINE_EXTENDED_MACROS_H
#define __PPCINLINE_EXTENDED_MACROS_H

#ifndef __INLINE_MACROS_H
#include <ppcinline/macros.h>
#endif

// define some macros which are missing in ppcinline/macros.h but are required by AmiSSL

#define LP1NRFP(offs, name, t1, v1, r1, bt, bn, fpt, cm1, cs1, cl1, cm2, cs2, cl2 ) \
({                                                        \
        typedef fpt;                                      \
        t1 _##name##_v1 = v1;                             \
        REG_##r1           = (ULONG) _##name##_v1;        \
        REG_A6             = (ULONG) (bn);                \
        (void) (*MyEmulHandle->EmulCallDirectOS)(-offs);  \
})

#define LP2NRFP(offs, name, t1, v1, r1, t2, v2, r2, bt, bn, fpt, cm1, cs1, cl1, cm2, cs2, cl2 ) \
({                                                        \
        typedef fpt;                                      \
        t1 _##name##_v1 = v1;                             \
        t2 _##name##_v2 = v2;                             \
        REG_##r1           = (ULONG) _##name##_v1;        \
        REG_##r2           = (ULONG) _##name##_v2;        \
        REG_A6             = (ULONG) (bn);                \
        (void) (*MyEmulHandle->EmulCallDirectOS)(-offs);  \
})

#define LP5NRFP(offs, name, t1, v1, r1, t2, v2, r2, t3, v3, r3, t4, v4, r4, t5, v5, r5, bt, bn, fpt, cm1, cs1, cl1, cm2, cs2, cl2 ) \
({                                                        \
        typedef fpt;                                      \
        t1 _##name##_v1 = v1;                             \
        t2 _##name##_v2 = v2;                             \
        t3 _##name##_v3 = v3;                             \
        t4 _##name##_v4 = v4;                             \
        t5 _##name##_v5 = v5;                             \
        REG_##r1           = (ULONG) _##name##_v1;        \
        REG_##r2           = (ULONG) _##name##_v2;        \
        REG_##r3           = (ULONG) _##name##_v3;        \
        REG_##r4           = (ULONG) _##name##_v4;        \
        REG_##r5           = (ULONG) _##name##_v5;        \
        REG_A6             = (ULONG) (bn);                \
        (void) (*MyEmulHandle->EmulCallDirectOS)(-offs);  \
})

#endif /* __PPCINLINE_EXTENDED_MACROS_H */

#include "extrasrc/md5.c"
#include "extrasrc/astcsma.c"
#include "extrasrc/getft.c"
#include "extrasrc/stccpy.c"
#include "extrasrc/stcgfe.c"
#include "extrasrc/stcgfn.c"
#include "extrasrc/stpblk.c"
#include "extrasrc/strmfp.c"
#include "extrasrc/strsfn.c"
#include "extrasrc/wbpath.c"
#include "extrasrc/NewReadArgs.c"
#include "extrasrc/stch_i.c"
#include "extrasrc/dice.c"

#ifdef __libnix__
int __oslibversion = 0;
#endif

#ifdef __ixemul__
struct Library *IFFParseBase, *KeymapBase;
struct UtilityBase *UtilityBase;
struct RxsLib *RexxSysBase;

struct WBStartup *_WBenchMsg;
#endif

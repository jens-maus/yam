#ifndef CLASSES_CLASSES_EXTRA_H
#define CLASSES_CLASSES_EXTRA_H

#include <clib/alib_protos.h>
#include <proto/intuition.h>
#include <proto/muimaster.h>
#include <proto/utility.h>
#include <libraries/iffparse.h>
#include <libraries/mui.h>
#include <mui/BetterString_mcc.h>
#include <mui/NList_mcc.h>
#include <mui/TextEditor_mcc.h>
#include <stdlib.h>
#include <string.h>

#include "YAM.h"
#include "YAM_addressbook.h"
#include "YAM_config.h"
#include "YAM_debug.h"
#include "YAM_folderconfig.h"
#include "YAM_hook.h"
#include "YAM_locale.h"
#include "YAM_locale.h"
#include "YAM_main.h"
#include "YAM_mainFolder.h"
#include "YAM_utilities.h"
#include "YAM_utilities.h"
#include "YAM_write.h"

#ifndef MUIF_NONE
#define MUIF_NONE                    0
#endif

/* some private MUI stuff... */
#ifndef MUIM_GoActive
#define MUIM_GoActive                0x8042491a
#endif
#ifndef MUIM_GoInactive
#define MUIM_GoInactive              0x80422c0c
#endif
#ifndef MUIA_Window_DisableKeys
#define MUIA_Window_DisableKeys      0x80424c36 /* V15 isg ULONG */
#endif
#ifndef MUIA_String_Popup
#define MUIA_String_Popup            0x80420d71
#endif
#define MUIA_List_CursorType         0x8042c53e /* V4  is. LONG  */
#define MUIV_List_CursorType_Bar 1

#define inittags(msg) (((struct opSet *)msg)->ops_AttrList)
#define GETDATA struct Data *data = (struct Data *)INST_DATA(cl,obj)

enum { IECODE_RETURN = 68, IECODE_ESCAPE = 69, IECODE_HELP = 95, IECODE_BACKSPACE = 65, IECODE_DEL = 70, IECODE_UP = 76, IECODE_DOWN = 77 };

#endif

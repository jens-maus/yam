/*h
Class:      MCC_NListtree
Copyright:  (c)1999-2000 by Carsten Scholling
Author:     Carsten Scholling
Contact:    cs@aphaso.de
Contents:   Registered class of the Magic User Interface.
h*/


/*** Include stuff ***/

#ifndef NLISTTREE_MCC_H
#define NLISTTREE_MCC_H

#ifndef LIBRARIES_MUI_H
#include "libraries/mui.h"
#endif

#include "amiga-align.h"

/*** MUI Defines ***/

#define MUIC_NListtree  "NListtree.mcc"
#define NListtreeObject MUI_NewObject(MUIC_NListtree






/*** Attributes ***/

#define MUIA_NListtree_Active                               0xfec81201  // *** [.SGN]
#define MUIA_NListtree_ActiveList                           0xfec81202  // *** [..GN]
#define MUIA_NListtree_CloseHook                            0xfec81203  // *** [IS..]
#define MUIA_NListtree_ConstructHook                        0xfec81204  // *** [IS..]
#define MUIA_NListtree_DestructHook                         0xfec81205  // *** [IS..]
#define MUIA_NListtree_DisplayHook                          0xfec81206  // *** [IS..]
#define MUIA_NListtree_DoubleClick                          0xfec81207  // *** [ISGN]
#define MUIA_NListtree_DragDropSort                         0xfec81208  // *** [IS..]
#define MUIA_NListtree_DupNodeName                          0xfec81209  // *** [IS..]
#define MUIA_NListtree_EmptyNodes                           0xfec8120a  // *** [IS..]
#define MUIA_NListtree_Format                               0xfec8120b  // *** [IS..]
#define MUIA_NListtree_OpenHook                             0xfec8120c  // *** [IS..]
#define MUIA_NListtree_Quiet                                0xfec8120d  // *** [.S..]
#define MUIA_NListtree_CompareHook                          0xfec8120e  // *** [IS..]
#define MUIA_NListtree_Title                                0xfec8120f  // *** [IS..]
#define MUIA_NListtree_TreeColumn                           0xfec81210  // *** [ISG.]
#define MUIA_NListtree_AutoVisible                          0xfec81211  // *** [ISG.]
#define MUIA_NListtree_FindNameHook                         0xfec81212  // *** [IS..]
#define MUIA_NListtree_MultiSelect                          0xfec81213  // *** [I...]
#define MUIA_NListtree_MultiTestHook                        0xfec81214  // *** [IS..]
#define MUIA_NListtree_CopyToClipHook                       0xfec81217  // *** [IS..]
#define MUIA_NListtree_DropType                             0xfec81218  // *** [..G.]
#define MUIA_NListtree_DropTarget                           0xfec81219  // *** [..G.]
#define MUIA_NListtree_DropTargetPos                        0xfec8121a  // *** [..G.]
#define MUIA_NListtree_FindUserDataHook                     0xfec8121b  // *** [IS..]
#define MUIA_NListtree_ShowTree                             0xfec8121c  // *** [ISG.]
#define MUIA_NListtree_SelectChange                         0xfec8121d  // *** [ISGN]


/*** Special attribute values ***/

#define MUIV_NListtree_Active_Off                           0
#define MUIV_NListtree_Active_Parent                        -2
#define MUIV_NListtree_Active_First                         -3
#define MUIV_NListtree_Active_FirstVisible                  -4
#define MUIV_NListtree_Active_LastVisible                   -5

#define MUIV_NListtree_ActiveList_Off                       0

#define MUIV_NListtree_AutoVisible_Off                      0
#define MUIV_NListtree_AutoVisible_Normal                   1
#define MUIV_NListtree_AutoVisible_FirstOpen                2
#define MUIV_NListtree_AutoVisible_Expand                   3

#define MUIV_NListtree_CompareHook_Head                     0
#define MUIV_NListtree_CompareHook_Tail                     -1
#define MUIV_NListtree_CompareHook_LeavesTop                -2
#define MUIV_NListtree_CompareHook_LeavesMixed              -3
#define MUIV_NListtree_CompareHook_LeavesBottom             -4

#define MUIV_NListtree_ConstructHook_String                 -1
#define MUIV_NListtree_ConstructHook_Flag_AutoCreate        (1<<15)

#define MUIV_NListtree_CopyToClipHook_Default               0

#define MUIV_NListtree_DestructHook_String                  -1

#define MUIV_NListtree_DisplayHook_Default                  -1

#define MUIV_NListtree_DoubleClick_Off                      -1
#define MUIV_NListtree_DoubleClick_All                      -2
#define MUIV_NListtree_DoubleClick_Tree                     -3
#define MUIV_NListtree_DoubleClick_NoTrigger                -4

#define MUIV_NListtree_DropType_None                        0
#define MUIV_NListtree_DropType_Above                       1
#define MUIV_NListtree_DropType_Below                       2
#define MUIV_NListtree_DropType_Onto                        3
#define MUIV_NListtree_DropType_Sorted                      4

#define MUIV_NListtree_FindNameHook_CaseSensitive           0
#define MUIV_NListtree_FindNameHook_CaseInsensitive         -1
#define MUIV_NListtree_FindNameHook_Part                    -2
#define MUIV_NListtree_FindNameHook_PartCaseInsensitive     -3
#define MUIV_NListtree_FindNameHook_PointerCompare          -4

#define MUIV_NListtree_FindUserDataHook_CaseSensitive       0
#define MUIV_NListtree_FindUserDataHook_CaseInsensitive     -1
#define MUIV_NListtree_FindUserDataHook_Part                -2
#define MUIV_NListtree_FindUserDataHook_PartCaseInsensitive -3
#define MUIV_NListtree_FindUserDataHook_PointerCompare      -4

#define MUIV_NListtree_MultiSelect_None                     0
#define MUIV_NListtree_MultiSelect_Default                  1
#define MUIV_NListtree_MultiSelect_Shifted                  2
#define MUIV_NListtree_MultiSelect_Always                   3


#define MUIV_NListtree_ShowTree_Toggle                      -1


/*** Structures & Flags ***/

struct MUI_NListtree_TreeNode {
    struct  MinNode tn_Node;    // ***  To make it a node.
    STRPTR  tn_Name;            // ***  Simple name field.
    UWORD   tn_Flags;           // ***  Used for the flags below.
    APTR    tn_User;            // ***  Free for user data.
};


#define TNF_OPEN                    (1<<0)
#define TNF_LIST                    (1<<1)
#define TNF_FROZEN                  (1<<2)
#define TNF_NOSIGN                  (1<<3)
#define TNF_SELECTED                (1<<4)



struct MUI_NListtree_TestPos_Result {
    struct  MUI_NListtree_TreeNode *tpr_TreeNode;
    UWORD   tpr_Type;
    LONG    tpr_ListEntry;
    UWORD   tpr_ListFlags;
    WORD    tpr_Column;

};

#define tpr_Flags tpr_Type      /* OBSOLETE */


/*** Methods ***/

#define MUIM_NListtree_Open                                 0xfec81101
#define MUIM_NListtree_Close                                0xfec81102
#define MUIM_NListtree_Insert                               0xfec81103
#define MUIM_NListtree_Remove                               0xfec81104
#define MUIM_NListtree_Exchange                             0xfec81105
#define MUIM_NListtree_Move                                 0xfec81106
#define MUIM_NListtree_Rename                               0xfec81107
#define MUIM_NListtree_FindName                             0xfec81108
#define MUIM_NListtree_GetEntry                             0xfec81109
#define MUIM_NListtree_GetNr                                0xfec8110a
#define MUIM_NListtree_Sort                                 0xfec8110b
#define MUIM_NListtree_TestPos                              0xfec8110c
#define MUIM_NListtree_Redraw                               0xfec8110d
#define MUIM_NListtree_NextSelected                         0xfec81110
#define MUIM_NListtree_MultiTest                            0xfec81111
#define MUIM_NListtree_Select                               0xfec81112
#define MUIM_NListtree_Copy                                 0xfec81113
#define MUIM_NListtree_InsertStruct                         0xfec81114  // *** Insert a struct (like a path) into the list.
#define MUIM_NListtree_Active                               0xfec81115  // *** Method which gives the active node/number.
#define MUIM_NListtree_DoubleClick                          0xfec81116  // *** Occurs on every double click.
#define MUIM_NListtree_PrevSelected                         0xfec81118  // *** Like reverse NextSelected.
#define MUIM_NListtree_CopyToClip                           0xfec81119  // *** Copy an entry or part to the clipboard.
#define MUIM_NListtree_FindUserData                         0xfec8111a  // *** Find a node upon user data.
#define MUIM_NListtree_Clear                                0xfec8111b  // *** Clear complete tree.
#define MUIM_NListtree_DropType                             0xfec8111e  // ***
#define MUIM_NListtree_DropDraw                             0xfec8111f  // ***


/*** Method structs ***/

struct MUIP_NListtree_Open {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode;
    struct MUI_NListtree_TreeNode *TreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Close {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode;
    struct MUI_NListtree_TreeNode *TreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Insert {
    ULONG   MethodID;
    STRPTR  Name;
    APTR    User;
    struct  MUI_NListtree_TreeNode *ListNode;
    struct  MUI_NListtree_TreeNode *PrevNode;
    ULONG   Flags;
};


struct MUIP_NListtree_Remove {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode;
    struct MUI_NListtree_TreeNode *TreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Clear {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode;
    ULONG Flags;
};


struct MUIP_NListtree_FindName {
    ULONG   MethodID;
    struct  MUI_NListtree_TreeNode *ListNode;
    STRPTR  Name;
    ULONG   Flags;
};


struct MUIP_NListtree_FindUserData {
    ULONG   MethodID;
    struct  MUI_NListtree_TreeNode *ListNode;
    APTR    User;
    ULONG   Flags;
};


struct MUIP_NListtree_GetEntry {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *Node;
    LONG  Position;
    ULONG Flags;
};


struct MUIP_NListtree_GetNr {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Move {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *OldListNode;
    struct MUI_NListtree_TreeNode *OldTreeNode;
    struct MUI_NListtree_TreeNode *NewListNode;
    struct MUI_NListtree_TreeNode *NewTreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Exchange {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode1;
    struct MUI_NListtree_TreeNode *TreeNode1;
    struct MUI_NListtree_TreeNode *ListNode2;
    struct MUI_NListtree_TreeNode *TreeNode2;
    ULONG Flags;
};


struct MUIP_NListtree_Rename {
    ULONG   MethodID;
    struct  MUI_NListtree_TreeNode *TreeNode;
    STRPTR  NewName;
    ULONG   Flags;
};


struct MUIP_NListtree_Sort {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *ListNode;
    ULONG Flags;
};


struct MUIP_NListtree_TestPos {
    ULONG MethodID;
    LONG  X;
    LONG  Y;
    APTR  Result;
};


struct MUIP_NListtree_Redraw {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_Select {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    LONG    SelType,
            SelFlags,
            *State;
};


struct MUIP_NListtree_NextSelected {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode **TreeNode;
};


struct MUIP_NListtree_MultiTest {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    LONG    SelType,
            SelFlags,
            CurrType;
};


struct MUIP_NListtree_Copy {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *SourceListNode;
    struct MUI_NListtree_TreeNode *SourceTreeNode;
    struct MUI_NListtree_TreeNode *DestListNode;
    struct MUI_NListtree_TreeNode *DestTreeNode;
    ULONG Flags;
};


struct MUIP_NListtree_InsertStruct {
    ULONG   MethodID;
    STRPTR  Name;
    APTR    User;
    STRPTR  Delimiter;
    ULONG   Flags;
};


struct MUIP_NListtree_Active {
    ULONG MethodID;
    LONG Pos;
    struct MUI_NListtree_TreeNode *ActiveNode;
};


struct MUIP_NListtree_DoubleClick {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    LONG Entry;
    LONG Column;
};


struct MUIP_NListtree_PrevSelected {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode **TreeNode;
};


struct MUIP_NListtree_CopyToClip {
    ULONG MethodID;
    struct MUI_NListtree_TreeNode *TreeNode;
    LONG Pos;
    LONG Unit;
};


struct  MUIP_NListtree_DropType {
    ULONG MethodID;
    LONG *Pos;
    LONG *Type;
    LONG MinX, MaxX, MinY, MaxY;
    LONG MouseX, MouseY;
};


struct  MUIP_NListtree_DropDraw {
    ULONG MethodID;
    LONG Pos;
    LONG Type;
    LONG MinX, MaxX, MinY, MaxY;
};


/*** Special method values ***/

#define MUIV_NListtree_Close_ListNode_Root                  0
#define MUIV_NListtree_Close_ListNode_Parent                -1
#define MUIV_NListtree_Close_ListNode_Active                -2

#define MUIV_NListtree_Close_TreeNode_Head                  0
#define MUIV_NListtree_Close_TreeNode_Tail                  -1
#define MUIV_NListtree_Close_TreeNode_Active                -2
#define MUIV_NListtree_Close_TreeNode_All                   -3



#define MUIV_NListtree_Exchange_ListNode1_Root              0
#define MUIV_NListtree_Exchange_ListNode1_Active            -2

#define MUIV_NListtree_Exchange_TreeNode1_Head              0
#define MUIV_NListtree_Exchange_TreeNode1_Tail              -1
#define MUIV_NListtree_Exchange_TreeNode1_Active            -2

#define MUIV_NListtree_Exchange_ListNode2_Root              0
#define MUIV_NListtree_Exchange_ListNode2_Active            -2

#define MUIV_NListtree_Exchange_TreeNode2_Head              0
#define MUIV_NListtree_Exchange_TreeNode2_Tail              -1
#define MUIV_NListtree_Exchange_TreeNode2_Active            -2
#define MUIV_NListtree_Exchange_TreeNode2_Up                -5
#define MUIV_NListtree_Exchange_TreeNode2_Down              -6


#define MUIV_NListtree_FindName_ListNode_Root               0
#define MUIV_NListtree_FindName_ListNode_Active             -2

#define MUIV_NListtree_FindName_Flag_SameLevel              (1<<15)
#define MUIV_NListtree_FindName_Flag_Visible                (1<<14)
#define MUIV_NListtree_FindName_Flag_Activate               (1<<13)
#define MUIV_NListtree_FindName_Flag_Selected               (1<<11)
#define MUIV_NListtree_FindName_Flag_StartNode              (1<<10)
#define MUIV_NListtree_FindName_Flag_Reverse                (1<<9)


#define MUIV_NListtree_FindUserData_ListNode_Root           0
#define MUIV_NListtree_FindUserData_ListNode_Active         -2

#define MUIV_NListtree_FindUserData_Flag_SameLevel          (1<<15)
#define MUIV_NListtree_FindUserData_Flag_Visible            (1<<14)
#define MUIV_NListtree_FindUserData_Flag_Activate           (1<<13)
#define MUIV_NListtree_FindUserData_Flag_Selected           (1<<11)
#define MUIV_NListtree_FindUserData_Flag_StartNode          (1<<10)
#define MUIV_NListtree_FindUserData_Flag_Reverse            (1<<9)


#define MUIV_NListtree_GetEntry_ListNode_Root               0
#define MUIV_NListtree_GetEntry_ListNode_Active             -2
#define MUIV_NListtree_GetEntry_TreeNode_Active             -3

#define MUIV_NListtree_GetEntry_Position_Head               0
#define MUIV_NListtree_GetEntry_Position_Tail               -1
#define MUIV_NListtree_GetEntry_Position_Active             -2
#define MUIV_NListtree_GetEntry_Position_Next               -3
#define MUIV_NListtree_GetEntry_Position_Previous           -4
#define MUIV_NListtree_GetEntry_Position_Parent             -5

#define MUIV_NListtree_GetEntry_Flag_SameLevel              (1<<15)
#define MUIV_NListtree_GetEntry_Flag_Visible                (1<<14)


#define MUIV_NListtree_GetNr_TreeNode_Active                -2

#define MUIV_NListtree_GetNr_Flag_CountAll                  (1<<15)
#define MUIV_NListtree_GetNr_Flag_CountLevel                (1<<14)
#define MUIV_NListtree_GetNr_Flag_CountList                 (1<<13)
#define MUIV_NListtree_GetNr_Flag_ListEmpty                 (1<<12)
#define MUIV_NListtree_GetNr_Flag_Visible                   (1<<11)


#define MUIV_NListtree_Insert_ListNode_Root                 0
#define MUIV_NListtree_Insert_ListNode_Active               -2
#define MUIV_NListtree_Insert_ListNode_LastInserted         -3
#define MUIV_NListtree_Insert_ListNode_ActiveFallback       -4

#define MUIV_NListtree_Insert_PrevNode_Head                 0
#define MUIV_NListtree_Insert_PrevNode_Tail                 -1
#define MUIV_NListtree_Insert_PrevNode_Active               -2
#define MUIV_NListtree_Insert_PrevNode_Sorted               -4

#define MUIV_NListtree_Insert_Flag_Active                   (1<<13)
#define MUIV_NListtree_Insert_Flag_NextNode                 (1<<12)


#define MUIV_NListtree_Move_OldListNode_Root                0
#define MUIV_NListtree_Move_OldListNode_Active              -2

#define MUIV_NListtree_Move_OldTreeNode_Head                0
#define MUIV_NListtree_Move_OldTreeNode_Tail                -1
#define MUIV_NListtree_Move_OldTreeNode_Active              -2

#define MUIV_NListtree_Move_NewListNode_Root                0
#define MUIV_NListtree_Move_NewListNode_Active              -2

#define MUIV_NListtree_Move_NewTreeNode_Head                0
#define MUIV_NListtree_Move_NewTreeNode_Tail                -1
#define MUIV_NListtree_Move_NewTreeNode_Active              -2
#define MUIV_NListtree_Move_NewTreeNode_Sorted              -4

#define MUIV_NListtree_Move_Flag_KeepStructure              (1<<13)


#define MUIV_NListtree_Open_ListNode_Root                   0
#define MUIV_NListtree_Open_ListNode_Parent                 -1
#define MUIV_NListtree_Open_ListNode_Active                 -2
#define MUIV_NListtree_Open_TreeNode_Head                   0
#define MUIV_NListtree_Open_TreeNode_Tail                   -1
#define MUIV_NListtree_Open_TreeNode_Active                 -2
#define MUIV_NListtree_Open_TreeNode_All                    -3



#define MUIV_NListtree_Remove_ListNode_Root                 0
#define MUIV_NListtree_Remove_ListNode_Active               -2
#define MUIV_NListtree_Remove_TreeNode_Head                 0
#define MUIV_NListtree_Remove_TreeNode_Tail                 -1
#define MUIV_NListtree_Remove_TreeNode_Active               -2
#define MUIV_NListtree_Remove_TreeNode_All                  -3
#define MUIV_NListtree_Remove_TreeNode_Selected             -4

#define MUIV_NListtree_Remove_Flag_NoActive                 (1<<13)




#define MUIV_NListtree_Rename_TreeNode_Active               -2

#define MUIV_NListtree_Rename_Flag_User                     (1<<8)
#define MUIV_NListtree_Rename_Flag_NoRefresh                (1<<9)


#define MUIV_NListtree_Sort_ListNode_Root                   0
#define MUIV_NListtree_Sort_ListNode_Active                 -2
#define MUIV_NListtree_Sort_TreeNode_Active                 -3

#define MUIV_NListtree_Sort_Flag_RecursiveOpen              (1<<13)
#define MUIV_NListtree_Sort_Flag_RecursiveAll               (1<<12)


#define MUIV_NListtree_TestPos_Result_None                  0
#define MUIV_NListtree_TestPos_Result_Above                 1
#define MUIV_NListtree_TestPos_Result_Below                 2
#define MUIV_NListtree_TestPos_Result_Onto                  3
#define MUIV_NListtree_TestPos_Result_Sorted                4

#define MUIV_NListtree_Redraw_Active                        -1
#define MUIV_NListtree_Redraw_All                           -2

#define MUIV_NListtree_Redraw_Flag_Nr                       (1<<15)

#define MUIV_NListtree_Select_Active                        -1
#define MUIV_NListtree_Select_All                           -2
#define MUIV_NListtree_Select_Visible                       -3

#define MUIV_NListtree_Select_Off                           0
#define MUIV_NListtree_Select_On                            1
#define MUIV_NListtree_Select_Toggle                        2
#define MUIV_NListtree_Select_Ask                           3

#define MUIV_NListtree_Select_Flag_Force                    (1<<15)


#define MUIV_NListtree_NextSelected_Start                   -1
#define MUIV_NListtree_NextSelected_End                     -1


#define MUIV_NListtree_Copy_SourceListNode_Root             0
#define MUIV_NListtree_Copy_SourceListNode_Active           -2

#define MUIV_NListtree_Copy_SourceTreeNode_Head             0
#define MUIV_NListtree_Copy_SourceTreeNode_Tail             -1
#define MUIV_NListtree_Copy_SourceTreeNode_Active           -2

#define MUIV_NListtree_Copy_DestListNode_Root               0
#define MUIV_NListtree_Copy_DestListNode_Active             -2

#define MUIV_NListtree_Copy_DestTreeNode_Head               0
#define MUIV_NListtree_Copy_DestTreeNode_Tail               -1
#define MUIV_NListtree_Copy_DestTreeNode_Active             -2
#define MUIV_NListtree_Copy_DestTreeNode_Sorted             -4

#define MUIV_NListtree_Copy_Flag_KeepStructure              (1<<13)


#define MUIV_NListtree_PrevSelected_Start                   -1
#define MUIV_NListtree_PrevSelected_End                     -1


#define MUIV_NListtree_CopyToClip_Active                    -1


/*** Hook message structs ***/

struct MUIP_NListtree_CloseMessage
{
    ULONG HookID;
    struct MUI_NListtree_TreeNode *TreeNode;
};


struct MUIP_NListtree_CompareMessage
{
    ULONG HookID;
    struct MUI_NListtree_TreeNode *TreeNode1;
    struct MUI_NListtree_TreeNode *TreeNode2;
    LONG SortType;
};


struct MUIP_NListtree_ConstructMessage
{
    ULONG HookID;
    STRPTR Name;
    APTR UserData;
    APTR MemPool;
    ULONG Flags;
};


struct MUIP_NListtree_DestructMessage
{
    ULONG HookID;
    STRPTR Name;
    APTR UserData;
    APTR MemPool;
    ULONG Flags;
};


struct MUIP_NListtree_DisplayMessage
{
    ULONG   HookID;
    struct  MUI_NListtree_TreeNode *TreeNode;
    LONG    EntryPos;
    STRPTR  *Array;
    STRPTR  *Preparse;
};


struct MUIP_NListtree_CopyToClipMessage
{
    ULONG   HookID;
    struct  MUI_NListtree_TreeNode *TreeNode;
    LONG    Pos;
    LONG    Unit;
};


struct MUIP_NListtree_FindNameMessage
{
    ULONG   HookID;
    STRPTR  Name;
    STRPTR  NodeName;
    APTR    UserData;
    ULONG   Flags;
};


struct MUIP_NListtree_FindUserDataMessage
{
    ULONG   HookID;
    APTR    User;
    APTR    UserData;
    STRPTR  NodeName;
    ULONG   Flags;
};


struct MUIP_NListtree_OpenMessage
{
    ULONG HookID;
    struct MUI_NListtree_TreeNode *TreeNode;
};


struct MUIP_NListtree_MultiTestMessage
{
    ULONG HookID;
    struct MUI_NListtree_TreeNode *TreeNode;
    LONG    SelType,
            SelFlags,
            CurrType;
};

#include "default-align.h"

#endif /* NLISTTREE_MCC_H */

/*

      MCC_Listtree (c) by kMel, Klaus Melchior

      Registered class of the Magic User Interface.

      Listtree_mcc.h

*/


/*** Include stuff ***/

#ifndef LISTTREE_MCC_H
#define LISTTREE_MCC_H

#ifndef LIBRARIES_MUI_H
#include "libraries/mui.h"
#endif


/*** MUI Defines ***/

#define MUIC_Listtree "Listtree.mcc"
#define ListtreeObject MUI_NewObject(MUIC_Listtree



/*** Methods ***/

#define MUIM_Listtree_Close                0x8002001f
#define MUIM_Listtree_Exchange             0x80020008
#define MUIM_Listtree_FindName             0x8002003c
#define MUIM_Listtree_GetEntry             0x8002002b
#define MUIM_Listtree_GetNr                0x8002000e
#define MUIM_Listtree_Insert               0x80020011
#define MUIM_Listtree_Move                 0x80020009
#define MUIM_Listtree_Open                 0x8002001e
#define MUIM_Listtree_Remove               0x80020012
#define MUIM_Listtree_Rename               0x8002000c
#define MUIM_Listtree_SetDropMark          0x8002004c
#define MUIM_Listtree_Sort                 0x80020029
#define MUIM_Listtree_TestPos              0x8002004b

/*** Method structs ***/

struct MUIP_Listtree_Close {
   ULONG MethodID;
   APTR  ListNode;
   APTR  TreeNode;
   ULONG Flags;
};

struct MUIP_Listtree_Exchange {
   ULONG MethodID;
   APTR  ListNode1;
   APTR  TreeNode1;
   APTR  ListNode2;
   APTR  TreeNode2;
   ULONG Flags;
};

struct MUIP_Listtree_FindName {
   ULONG MethodID;
   APTR  ListNode;
   char *Name;
   ULONG Flags;
};

struct MUIP_Listtree_GetEntry {
   ULONG MethodID;
   APTR  Node;
   LONG  Position;
   ULONG Flags;
};

struct MUIP_Listtree_GetNr {
   ULONG MethodID;
   APTR  TreeNode;
   ULONG Flags;
};

struct MUIP_Listtree_Insert {
   ULONG MethodID;
   char *Name;
   APTR  User;
   APTR  ListNode;
   APTR  PrevNode;
   ULONG Flags;
};

struct MUIP_Listtree_Move {
   ULONG MethodID;
   APTR  OldListNode;
   APTR  OldTreeNode;
   APTR  NewListNode;
   APTR  NewTreeNode;
   ULONG Flags;
};

struct MUIP_Listtree_Open {
   ULONG MethodID;
   APTR  ListNode;
   APTR  TreeNode;
   ULONG Flags;
};

struct MUIP_Listtree_Remove {
   ULONG MethodID;
   APTR  ListNode;
   APTR  TreeNode;
   ULONG Flags;
};

struct MUIP_Listtree_Rename {
   ULONG MethodID;
   APTR  TreeNode;
   char *NewName;
   ULONG Flags;
};

struct MUIP_Listtree_SetDropMark {
   ULONG MethodID;
   LONG  Entry;
   ULONG Values;
};

struct MUIP_Listtree_Sort {
   ULONG MethodID;
   APTR  ListNode;
   ULONG Flags;
};

struct MUIP_Listtree_TestPos {
   ULONG MethodID;
   LONG  X;
   LONG  Y;
   APTR  Result;
};


/*** Special method values ***/

#define MUIV_Lt_Close_ListNode_Root          0
#define MUIV_Lt_Close_ListNode_Parent       -1
#define MUIV_Lt_Close_ListNode_Active       -2

#define MUIV_Lt_Close_TreeNode_Head          0
#define MUIV_Lt_Close_TreeNode_Tail         -1
#define MUIV_Lt_Close_TreeNode_Active       -2
#define MUIV_Lt_Close_TreeNode_All          -3

#define MUIV_Lt_Exchange_ListNode1_Root      0
#define MUIV_Lt_Exchange_ListNode1_Active   -2

#define MUIV_Lt_Exchange_TreeNode1_Head      0
#define MUIV_Lt_Exchange_TreeNode1_Tail     -1
#define MUIV_Lt_Exchange_TreeNode1_Active   -2

#define MUIV_Lt_Exchange_ListNode2_Root      0
#define MUIV_Lt_Exchange_ListNode2_Active   -2

#define MUIV_Lt_Exchange_TreeNode2_Head      0
#define MUIV_Lt_Exchange_TreeNode2_Tail     -1
#define MUIV_Lt_Exchange_TreeNode2_Active   -2
#define MUIV_Lt_Exchange_TreeNode2_Up       -5
#define MUIV_Lt_Exchange_TreeNode2_Down     -6

#define MUIV_Lt_FindName_ListNode_Root       0
#define MUIV_Lt_FindName_ListNode_Active    -2

#define MUIV_Lt_GetEntry_ListNode_Root       0
#define MUIV_Lt_GetEntry_ListNode_Active    -2

#define MUIV_Lt_GetEntry_Position_Head       0
#define MUIV_Lt_GetEntry_Position_Tail      -1
#define MUIV_Lt_GetEntry_Position_Active    -2
#define MUIV_Lt_GetEntry_Position_Next      -3
#define MUIV_Lt_GetEntry_Position_Previous  -4
#define MUIV_Lt_GetEntry_Position_Parent    -5

#define MUIV_Lt_GetNr_TreeNode_Active       -2

#define MUIV_Lt_Insert_ListNode_Root         0
#define MUIV_Lt_Insert_ListNode_Active      -2

#define MUIV_Lt_Insert_PrevNode_Head         0
#define MUIV_Lt_Insert_PrevNode_Tail        -1
#define MUIV_Lt_Insert_PrevNode_Active      -2
#define MUIV_Lt_Insert_PrevNode_Sorted      -4

#define MUIV_Lt_Move_OldListNode_Root        0
#define MUIV_Lt_Move_OldListNode_Active     -2

#define MUIV_Lt_Move_OldTreeNode_Head        0
#define MUIV_Lt_Move_OldTreeNode_Tail       -1
#define MUIV_Lt_Move_OldTreeNode_Active     -2

#define MUIV_Lt_Move_NewListNode_Root        0
#define MUIV_Lt_Move_NewListNode_Active     -2

#define MUIV_Lt_Move_NewTreeNode_Head        0
#define MUIV_Lt_Move_NewTreeNode_Tail       -1
#define MUIV_Lt_Move_NewTreeNode_Active     -2
#define MUIV_Lt_Move_NewTreeNode_Sorted     -4

#define MUIV_Lt_Open_ListNode_Root           0
#define MUIV_Lt_Open_ListNode_Parent        -1
#define MUIV_Lt_Open_ListNode_Active        -2
#define MUIV_Lt_Open_TreeNode_Head           0
#define MUIV_Lt_Open_TreeNode_Tail          -1
#define MUIV_Lt_Open_TreeNode_Active        -2
#define MUIV_Lt_Open_TreeNode_All           -3

#define MUIV_Lt_Remove_ListNode_Root         0
#define MUIV_Lt_Remove_ListNode_Active      -2
#define MUIV_Lt_Remove_TreeNode_Head         0
#define MUIV_Lt_Remove_TreeNode_Tail        -1
#define MUIV_Lt_Remove_TreeNode_Active      -2
#define MUIV_Lt_Remove_TreeNode_All         -3

#define MUIV_Lt_Rename_TreeNode_Active      -2

#define MUIV_Lt_SetDropMark_Entry_None      -1

#define MUIV_Lt_SetDropMark_Values_None      0
#define MUIV_Lt_SetDropMark_Values_Above     1
#define MUIV_Lt_SetDropMark_Values_Below     2
#define MUIV_Lt_SetDropMark_Values_Onto      3
#define MUIV_Lt_SetDropMark_Values_Sorted    4

#define MUIV_Lt_Sort_ListNode_Root           0
#define MUIV_Lt_Sort_ListNode_Active        -2

#define MUIV_Lt_TestPos_Result_Flags_None    0
#define MUIV_Lt_TestPos_Result_Flags_Above   1
#define MUIV_Lt_TestPos_Result_Flags_Below   2
#define MUIV_Lt_TestPos_Result_Flags_Onto    3
#define MUIV_Lt_TestPos_Result_Flags_Sorted   4


/*** Special method flags ***/

#define MUIV_Lt_Close_Flags_Nr             (1<<15)
#define MUIV_Lt_Close_Flags_Visible        (1<<14)

#define MUIV_Lt_FindName_Flags_SameLevel   (1<<15)
#define MUIV_Lt_FindName_Flags_Visible     (1<<14)

#define MUIV_Lt_GetEntry_Flags_SameLevel   (1<<15)
#define MUIV_Lt_GetEntry_Flags_Visible     (1<<14)

#define MUIV_Lt_GetNr_Flags_ListEmpty      (1<<12)
#define MUIV_Lt_GetNr_Flags_CountList      (1<<13)
#define MUIV_Lt_GetNr_Flags_CountLevel     (1<<14)
#define MUIV_Lt_GetNr_Flags_CountAll       (1<<15)

#define MUIV_Lt_Insert_Flags_Nr            (1<<15)
#define MUIV_Lt_Insert_Flags_Visible       (1<<14)
#define MUIV_Lt_Insert_Flags_Active        (1<<13)
#define MUIV_Lt_Insert_Flags_NextNode      (1<<12)

#define MUIV_Lt_Move_Flags_Nr              (1<<15)
#define MUIV_Lt_Move_Flags_Visible         (1<<14)

#define MUIV_Lt_Open_Flags_Nr              (1<<15)
#define MUIV_Lt_Open_Flags_Visible         (1<<14)

#define MUIV_Lt_Remove_Flags_Nr            (1<<15)
#define MUIV_Lt_Remove_Flags_Visible       (1<<14)

#define MUIV_Lt_Rename_Flags_User          (1<<8)
#define MUIV_Lt_Rename_Flags_NoRefresh     (1<<9)

#define MUIV_Lt_Sort_Flags_Nr              (1<<15)
#define MUIV_Lt_Sort_Flags_Visible         (1<<14)



/*** Attributes ***/

#define MUIA_Listtree_Active               0x80020020
#define MUIA_Listtree_CloseHook            0x80020033
#define MUIA_Listtree_ConstructHook        0x80020016
#define MUIA_Listtree_DestructHook         0x80020017
#define MUIA_Listtree_DisplayHook          0x80020018
#define MUIA_Listtree_DoubleClick          0x8002000d
#define MUIA_Listtree_DragDropSort         0x80020031
#define MUIA_Listtree_DuplicateNodeName    0x8002003d
#define MUIA_Listtree_EmptyNodes           0x80020030
#define MUIA_Listtree_Format               0x80020014
#define MUIA_Listtree_MultiSelect          0x800200c3
#define MUIA_Listtree_NList                0x800200c4
#define MUIA_Listtree_OpenHook             0x80020032
#define MUIA_Listtree_Quiet                0x8002000a
#define MUIA_Listtree_SortHook             0x80020010
#define MUIA_Listtree_Title                0x80020015
#define MUIA_Listtree_TreeColumn           0x80020013

/*** Special attribute values ***/

#define MUIV_Lt_Active_Off             0

#define MUIV_Lt_ConstructHook_String  -1

#define MUIV_Lt_DestructHook_String   -1

#define MUIV_Lt_DisplayHook_Default   -1

#define MUIV_Lt_DoubleClick_Off       -1
#define MUIV_Lt_DoubleClick_All       -2
#define MUIV_Lt_DoubleClick_Tree      -3

#define MUIV_Lt_SortHook_Head          0
#define MUIV_Lt_SortHook_Tail         -1
#define MUIV_Lt_SortHook_LeavesTop    -2
#define MUIV_Lt_SortHook_LeavesMixed  -3
#define MUIV_Lt_SortHook_LeavesBottom  -4



/*** Structures, Flags & Values ***/

struct MUIS_Listtree_TreeNode {
   LONG  tn_Private1;
   LONG  tn_Private2;
   char *tn_Name;
   UWORD tn_Flags;
   APTR  tn_User;
};

struct MUIS_Listtree_TestPos_Result {
   APTR  tpr_TreeNode;
   UWORD tpr_Flags;
   LONG  tpr_ListEntry;
   UWORD tpr_ListFlags;
};


#define TNF_OPEN   (1<<00)
#define TNF_LIST   (1<<01)
#define TNF_FROZEN (1<<02)
#define TNF_NOSIGN (1<<03)




/*** Configs ***/



#endif /* LISTTREE_MCC_H */

/* PrMake.rexx 0.10 (16.2.1996) Copyright 1995 kmel, Klaus Melchior */


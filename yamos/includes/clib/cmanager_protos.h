#ifndef CMANAGER_PROTOS_H
#define CMANAGER_PROTOS_H

extern struct CMGroup  *CM_GetParent(struct CMGroup *, struct CMGroup *);
extern BOOL             CM_LoadData(STRPTR, struct CMData *, STRPTR);
extern void             CM_SaveData(STRPTR, struct CMData *, STRPTR);
extern void             CM_FreeData(struct CMData *);
extern APTR             CM_StartManager(STRPTR, STRPTR);
extern void             CM_FreeHandle(APTR, BOOL);
extern APTR             CM_AllocEntry(ULONG);
extern void             CM_FreeEntry(APTR);
extern APTR             CM_GetEntry(APTR, ULONG);
extern struct BitMap   *CM_CreateBitMap(ULONG, ULONG, ULONG, ULONG, struct BitMap *);
extern void             CM_DeleteBitMap(struct BitMap *);
extern BOOL             CM_AddEntry(APTR);
extern void             CM_FreeList(struct MinList *);

#endif /* CMANAGER_PROTOS_H */

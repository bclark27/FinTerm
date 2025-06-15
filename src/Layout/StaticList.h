#ifndef STATIC_LIST_H_
#define STATIC_LIST_H_

#include "Layout.h"

typedef struct StaticList
{
    Layout layout;

    int labelToItemIdx[LAYOUT_MAX_DIV];
    char** strListPtr;
    int strListLen;

    int itemSize;
    int scrollIdx;
    int selectIdx;
    int hoverIdx;
    bool boarder;

    bool strIsCpy;
    bool strDisposed;

} StaticList;

StaticList * StaticList_Create();
void StaticList_SetStrListPtr(StaticList* sl, char** strListPtr, int len);
void StaticList_SetStrListCpy(StaticList* sl, char** strList, int len);
void StaticList_SetBoarder(StaticList* sl, bool hasBoarder);
void StaticList_SetDirection(StaticList* sl, LayoutStrategy dir);

#endif

#ifndef STATIC_LIST_H_
#define STATIC_LIST_H_

#include "Layout.h"

typedef struct StaticList
{
    Layout layout;

    char** strListPtr;
    int strListLen;

    int itemSize;
    int indexOffset;
    bool boarder;

    int innerpad_up;
    int innerpad_down;
    int innerpad_left;
    int innerpad_right;

    bool strIsCpy;
    bool strDisposed;

} StaticList;

StaticList * StaticList_Create();
void StaticList_SetStrListPtr(StaticList* sl, char** strListPtr, int len);
void StaticList_SetStrListCpy(StaticList* sl, char** strList, int len);
void StaticList_SetInnerPad(StaticList* sl, int pad_up, int pad_down, int pad_left, int pad_right);
void StaticList_SetBoarder(StaticList* sl, bool hasBoarder);
void StaticList_SetDirection(StaticList* sl, LayoutStrategy dir);

#endif

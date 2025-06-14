#include "StaticList.h"

#include "Label.h"

void staticList_draw(Layout* l, WINDOW *win, int x, int y, int width, int height);
void staticList_onDestroy(Layout* l);
void staticList_onPtrEnter(Layout* l, InputEvent* e);
void staticList_onPtrExit(Layout* l, InputEvent* e);
void staticList_onPtrMove(Layout* l, InputEvent* e);
void staticList_onFocus(Layout* l);
void staticList_onUnFocus(Layout* l);
void staticList_onBblEvt(Layout* l, BblEvt* e);

void staticList_refreshPaddings(StaticList* sl);
void staticList_createlabelChildren(StaticList* sl, int count);

// vtable

static Layout_VT vtable = 
{
    .draw = staticList_draw,
    .onDestroy = staticList_onDestroy,
    .onPtrEnter = staticList_onPtrEnter,
    .onPtrExit = staticList_onPtrExit,
    .onPtrMove = staticList_onPtrMove,
    .onFocus = staticList_onFocus,
    .onUnFocus = staticList_onUnFocus,
    .onBblEvt = staticList_onBblEvt,
    .onBblEvtCapture = NULL,
};


// PUBLIC


StaticList * StaticList_Create()
{
    StaticList * sl = calloc(1, sizeof(StaticList));
    Layout_Init((Layout*)sl);
    sl->layout.vtable = vtable;
    sl->layout.layoutStrategy = LayoutStrategy_vert;
    return sl;
}

void StaticList_SetStrListPtr(StaticList* sl, char*** strListPtr, int len)
{
    if (!sl) return;

    sl->strListPtr = strListPtr;
    sl->strListLen = len;
    sl->layout.redraw = true;
}

void StaticList_SetInnerPad(StaticList* sl, int pad_up, int pad_down, int pad_left, int pad_right)
{
    if (!sl) return;

    sl->innerpad_up = pad_up;
    sl->innerpad_down = pad_down;
    sl->innerpad_left = pad_left;
    sl->innerpad_right = pad_right;

    staticList_refreshPaddings(sl);
}

void StaticList_SetBoarder(StaticList* sl, bool hasBoarder)
{
    if (!sl) return;
    sl->boarder = hasBoarder;
    staticList_refreshPaddings(sl);
    sl->layout.redraw = true;
}

void StaticList_SetDirection(StaticList* sl, LayoutStrategy dir)
{
    if (!sl) return;

    sl->layout.layoutStrategy = dir;
    sl->layout.redraw = true;
}

// PRIV

void staticList_draw(Layout* l, WINDOW *win, int x, int y, int width, int height)
{
    if (!l) return;
    StaticList* sl = (StaticList*)l;

    if (sl->boarder)
    {
        int bcolor = -1;
        if (l->isFocus)
        {
            bcolor = DEFATTR_SELECTED_BOX();
        }
        
        if (bcolor >= 0) wattron(win, COLOR_PAIR(bcolor));
        box(win, 0, 0);
        if (bcolor >= 0) wattroff(win, COLOR_PAIR(bcolor));
    }

    if (!sl->strListPtr || sl->strListLen <= 0) return;
}
void staticList_onDestroy(Layout* l)
{

}
void staticList_onPtrEnter(Layout* l, InputEvent* e)
{

}
void staticList_onPtrExit(Layout* l, InputEvent* e)
{

}
void staticList_onPtrMove(Layout* l, InputEvent* e)
{

}
void staticList_onFocus(Layout* l)
{
    l->redraw = true;
}
void staticList_onUnFocus(Layout* l)
{
    l->redraw = true;
}
void staticList_onBblEvt(Layout* l, BblEvt* e)
{

}
void staticList_refreshPaddings(StaticList* sl)
{
    sl->layout.pad_up = sl->innerpad_up + (sl->boarder ? 1 : 0);
    sl->layout.pad_down = sl->innerpad_down + (sl->boarder ? 1 : 0);
    sl->layout.pad_left = sl->innerpad_left + (sl->boarder ? 1 : 0);
    sl->layout.pad_right = sl->innerpad_right + (sl->boarder ? 1 : 0);
}

void staticList_createlabelChildren(StaticList* sl, int count)
{
    for (int i = 0; i < MAX(count, sl->layout.childrenCount); i++)
    {
        if (i >= sl->layout.childrenCount)
        {
            Label* l = Label_Create();
            l->textWrap = false;
            Layout_AddChild(&sl->layout, l);
        }
    }

    for (int i = 0; i < sl->layout.childrenCount; i++)
    {
        Layout_SetVis(sl->layout.children[i], i < count);
    }
}
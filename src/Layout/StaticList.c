#include "StaticList.h"

#include "Label.h"

#define MAX_VIS_COUNT(sl)       (staticList_listSpaceSize(sl) / ((StaticList*)(sl))->itemSize)
#define CURR_VIS_COUNT(sl)      (MIN(MAX_VIS_COUNT(sl), ((StaticList*)(sl))->strListLen - ((StaticList*)(sl))->scrollIdx))

void staticList_draw(Layout* l, WINDOW *win, int x, int y, int width, int height);
void staticList_onDestroy(Layout* l);
void staticList_onPtrEnter(Layout* l, InputEvent* e);
void staticList_onPtrExit(Layout* l, InputEvent* e);
void staticList_onPtrMove(Layout* l, InputEvent* e);
void staticList_onFocus(Layout* l);
void staticList_onUnFocus(Layout* l);
void staticList_onBblEvt(Layout* l, BblEvt* e);

void staticList_ensureLabelCount(StaticList* sl, int count);
void staticList_setLabelCountVis(StaticList* sl, int count);
void staticList_refreshLabelItemIdx(StaticList* sl, int startItemIdx, int count);
void staticList_fixScrollOffset(StaticList* sl);
void staticList_refreshLabelStyles(StaticList* sl, int count);
void staticList_disposeStr(StaticList* sl);
void staticList_refreshLabelDims(StaticList* sl);
void staticList_selectLabelAttrs(Label* l, bool selected, bool hover);
int staticList_listSpaceSize(StaticList* sl);
void staticList_hoverLabel(StaticList* sl, int x, int y);

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
    sl->itemSize = 1;
    sl->selectIdx = -1;
    sl->hoverIdx = -1;
    return sl;
}

void StaticList_SetStrListPtr(StaticList* sl, char** strListPtr, int len)
{
    if (!sl) return;
    staticList_disposeStr(sl);
    sl->layout.redraw = true;
    if (!strListPtr) return;

    sl->strDisposed = false;
    sl->strIsCpy = false;

    sl->strListPtr = strListPtr;
    sl->strListLen = len;

    sl->selectIdx = -1;
    sl->hoverIdx = -1;
}

void StaticList_SetStrListCpy(StaticList* sl, char** strList, int len)
{
    if (!sl) return;
    staticList_disposeStr(sl);
    sl->layout.redraw = true;
    if (!strList) return;

    sl->strDisposed = false;
    sl->strIsCpy = true;
    
    sl->strListLen = len;
    sl->strListPtr = malloc(sizeof(char*) * len);

    sl->selectIdx = -1;
    sl->hoverIdx = -1;

    for (int i = 0; i < len; i++)
    {
        sl->strListPtr[i] = NULL;
        if (strList[i])
        {
            int l = strlen(strList[i]) + 1;
            sl->strListPtr[i] = malloc(l);
            strcpy(sl->strListPtr[i], strList[i]);
            sl->strListPtr[i][l] = 0;
        }
    }
}

void StaticList_SetBoarder(StaticList* sl, bool hasBoarder)
{
    if (!sl) return;
    sl->boarder = hasBoarder;
    sl->layout.pad_up = hasBoarder;
    sl->layout.pad_down = hasBoarder;
    sl->layout.pad_left = hasBoarder;
    sl->layout.pad_right = hasBoarder;
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

    int count = 0;
    if (sl->strListPtr && sl->strListLen > 0)
    { 
        count = sl->strListLen;
    }

    staticList_fixScrollOffset(sl);
    int visCount = CURR_VIS_COUNT(sl);
    staticList_ensureLabelCount(sl, visCount);
    staticList_setLabelCountVis(sl, visCount);
    staticList_refreshLabelItemIdx(sl, sl->scrollIdx, visCount);
    staticList_refreshLabelStyles(sl, visCount);
    
    for (int i = 0; i < visCount; i++)
    {
        Label* l = (Label*)sl->layout.children[i];
        Label_SetTextCpy(l, sl->strListPtr[sl->labelToItemIdx[i]]);
    }
}
void staticList_onDestroy(Layout* l)
{

}
void staticList_onPtrEnter(Layout* l, InputEvent* e)
{
    staticList_hoverLabel((StaticList*)l, e->mevent.x, e->mevent.y);
}
void staticList_onPtrExit(Layout* l, InputEvent* e)
{
    StaticList* sl = (StaticList*)l;
    sl->hoverIdx = -1;
    l->redraw = true;
}
void staticList_onPtrMove(Layout* l, InputEvent* e)
{
    staticList_hoverLabel((StaticList*)l, e->mevent.x, e->mevent.y);
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
    StaticList* sl = (StaticList*)l;
    if (e->type == BblEvtType_Scroll)
    {
        sl->scrollIdx += e->data.scroll.up ? -1 : 1;
        l->redraw = true;
    }
    else if (e->type == BblEvtType_Click && e->data.click.leftClick)
    {
        if (e->hit_target == l) return;
        // if the target is one of the child labels then set that guy
        int hitIdx = -1;
        for (int i = 0; i < l->childrenCount; i++)
        {
            if (l->children[i] == e->hit_target)
            {
                hitIdx = i;
                break;
            }
        }

        int prev = sl->selectIdx;
        if (hitIdx >= 0)
        {
            sl->selectIdx = sl->labelToItemIdx[hitIdx];
        }
        else
        {
            sl->selectIdx = -1;
        }

        l->redraw = prev != sl->selectIdx;
    }
    else if (e->type == BblEvtType_Key)
    {
        if (e->data.key.raw == KEY_DOWN || 
            e->data.key.raw == KEY_UP)
        {
            if (sl->hoverIdx < 0)
            {
                sl->hoverIdx = sl->scrollIdx;
            }
            else
            {
                sl->hoverIdx += e->data.key.raw == KEY_UP ? -1 : 1;
                sl->hoverIdx = MAX(sl->hoverIdx, 0);
                sl->hoverIdx = MIN(sl->hoverIdx, sl->strListLen - 1);
            }
            l->redraw = true;
        }
    }
}

void staticList_ensureLabelCount(StaticList* sl, int count)
{
    for (int i = 0; i < MAX(count, sl->layout.childrenCount); i++)
    {
        if (i >= sl->layout.childrenCount)
        {
            Label* l = Label_Create();
            l->textWrap = false;
            l->layout.absSize = sl->itemSize;
            staticList_selectLabelAttrs(l, false, false);
            Layout_AddChild(&sl->layout, (Layout*)l);
        }
    }
}

void staticList_setLabelCountVis(StaticList* sl, int count)
{
    for (int i = 0; i < sl->layout.childrenCount; i++)
    {
        Layout_SetVis((Layout*)sl->layout.children[i], i < count);
    }
}

void staticList_refreshLabelItemIdx(StaticList* sl, int startItemIdx, int count)
{
    for (int i = 0; i < count; i++)
    {
        sl->labelToItemIdx[i] = startItemIdx + i;
    }
}

void staticList_fixScrollOffset(StaticList* sl)
{
    if (!sl) return;
    int maxVisCount = MAX_VIS_COUNT(sl);
    int maxScroll = MAX(0, sl->strListLen - maxVisCount);
    if (sl->scrollIdx > maxScroll)
    {
        sl->scrollIdx = maxScroll;
    }

    sl->scrollIdx = MAX(sl->scrollIdx, 0);
}

void staticList_refreshLabelStyles(StaticList* sl, int count)
{
    for (int i = 0; i < count; i++)
    {
        staticList_selectLabelAttrs(
            (Label*)sl->layout.children[i], 
            sl->labelToItemIdx[i] == sl->selectIdx, 
            sl->labelToItemIdx[i] == sl->hoverIdx);
    }
}

void staticList_disposeStr(StaticList* sl)
{
    if (!sl) return;
    if (sl->strDisposed) return;
    sl->layout.redraw = true;
    sl->strDisposed = true;

    if (sl->strIsCpy)
    {
        if (sl->strListPtr == NULL) return;
        for (int i = 0; i < sl->strListLen; i++)
        {
            free(sl->strListPtr[i]);
        }
        free(sl->strListPtr);
    }
    
    sl->strListLen = 0;
    sl->strListPtr = NULL;
}

void staticList_refreshLabelDims(StaticList* sl)
{
    if (!sl) return;

    for (int i = 0; i < sl->layout.childrenCount; i++)
    {
        Label* l = (Label*)sl->layout.children[i];
        l->layout.absSize = sl->itemSize;
        l->layout.resize = true;
    }
}


void staticList_selectLabelAttrs(Label* l, bool selected, bool hover)
{
    if (!l) return;

    if (selected)
    {
        Label_SetHighlight(l, COLOR_CYAN);
        Label_SetTextColor(l, COLOR_BLACK);
    }
    else if (hover)
    {
        Label_SetHighlight(l, COLOR_MAGENTA);
        Label_SetTextColor(l, COLOR_BLACK);
    }
    else
    {
        Label_SetHighlight(l, -1);
        Label_SetTextColor(l, -1);
    }
}

int staticList_listSpaceSize(StaticList* sl)
{
    if (sl->layout.layoutStrategy == LayoutStrategy_vert)
    {
        return sl->layout.height - (sl->layout.pad_up + sl->layout.pad_down);
    }
    else
    {
        return sl->layout.width - (sl->layout.pad_left + sl->layout.pad_right);
    }
}

void staticList_hoverLabel(StaticList* sl, int x, int y)
{
    int curr = sl->hoverIdx;
    
    Layout* hovBuff[3];
    int len = 0;
    Layout_HitTest(&sl->layout, x, y, hovBuff, 3, &len);
    
    Layout* last = hovBuff[len - 1];
    for (int i = 0; i < sl->layout.childrenCount; i++)
    {
        if (sl->layout.children[i] == last)
        {
            sl->hoverIdx = sl->labelToItemIdx[i];
            break;
        }
    }

    sl->layout.redraw = curr != sl->hoverIdx;
}
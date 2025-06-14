#include "Layout.h"

#include "../Common.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../Logger.h"

// PRIV DECLERATIONS
bool layout_contains_point(Layout *layout, int x, int y);
void compute_layout_sizes(int total_size, struct Layout** elements, int* ret, int count);
void DrawThisLayout(Layout * l, bool force);
void removeArrayItem(void* arr, int eleSize, int eleCount, int idx);
void drawPhase1(Layout* l, bool force);
void drawPhase2(Layout* l);
bool insert_child_by_zindex(Layout* parent, Layout* child);

void layoutStat_horz(Layout* l, bool force);
void layoutStat_vert(Layout* l, bool force);
void layoutStat_abs(Layout* l, bool force);

// PUBLIC FUNCS



Layout * Layout_Create()
{
    Layout* l = calloc(1, sizeof(Layout));
    Layout_Init(l);
    return l;
}

void Layout_Init(Layout * l)
{
    memset(l, 0, sizeof(Layout));
    l->sizeRatio = 1;
    l->absSize = -1;
    l->layoutStrategy = LayoutStrategy_horz;
    l->visible = true;
    l->redraw = true;
    l->resize = true;
    l->focusable = true;
    l->zindex = 0;
}

void Layout_Destroy(Layout * l)
{
    if (!l) return;

    if (l->vtable.onDestroy) l->vtable.onDestroy(l);
    
    // check if i am in the parent children
    // if i am, silently remove self
    if (l->parent)
    {
        for (int i = 0; i < l->parent->childrenCount; i++)
        {
            if (l->parent->children[i] == l)
            {
                removeArrayItem(l->parent->children, sizeof(Layout*), l->parent->childrenCount, i);
                l->parent->childrenCount--;
                break;
            }
        }

        l->parent = NULL;
    }

    // call destroy on all my children
    // they will also try to remove themselves from my children arr so we should go in reverse
    for (int i = l->childrenCount - 1; i >= 0; i--)
    {
        Layout_Destroy(l->children[i]);
    }
    l->childrenCount = 0;


    if (l->pan) del_panel(l->pan);
    if (l->win) delwin(l->win);
    l->win = NULL;
    l->pan = NULL;
    free(l);
}

void Layout_Draw(Layout * l, bool force)
{
    drawPhase1(l, force);
    drawPhase2(l);
}

void Layout_SizeRefresh(Layout * l, int x, int y, int width, int height, bool force)
{
    if (!l) return;
    
    // if the size changed then lets redo the window and mark dirty
    if (!l->pan || !l->win || l->x != x || l->y != y || l->width != width || l->height != height || l->resize || force)
    {
        if (l->pan)
        {
            del_panel(l->pan);
            l->pan = NULL;
        }

        if (l->win)
        {
            delwin(l->win);
            l->win = NULL;
        }

        l->height = height;
        l->width = width;
        l->x = x;
        l->y = y;

        l->win = newwin(l->height, l->width, l->y, l->x);
        l->pan = new_panel(l->win);
        l->redraw = true;
        l->resize = false;
    }

    if (l->layoutStrategy == LayoutStrategy_horz)
    {
        layoutStat_horz(l, force);
    }
    else if (l->layoutStrategy == LayoutStrategy_vert)
    {
        layoutStat_vert(l, force);
    }
    else if (l->layoutStrategy == LayoutStrategy_abs)
    {
        layoutStat_abs(l, force);
    }
}

void Layout_SizeRefreshSameParams(Layout* l, bool force)
{
    if (!l) return;
    Layout_SizeRefresh(l, l->x, l->y, l->width, l->height, force);
}

bool Layout_AddChild(Layout * parent, Layout * child)
{
    // check if there is space available
    if (!parent || !child) return false;
    if (parent->childrenCount >= LAYOUT_MAX_DIV) return false;

    // if the child has a parent already, bad
    if (child->parent)
    {
        Layout_DetatchFromParent(child);
    }

    /*
    parent->children[parent->childrenCount++] = child;
    */
    if (!insert_child_by_zindex(parent, child)) return false;
    child->parent = parent;

    Layout_SizeRefreshSameParams(parent, false);

    return true;
}

void Layout_SetVis(Layout * l, bool visible)
{
    if (!l) return;
    if (l->visible == visible) return;
    l->visible = visible;
    l->resize = true;
}

void Layout_SetSize(Layout * l, int size, bool isAbs)
{
    if (!l) return;

    if (isAbs)
    {
        l->absSize = size;
        l->sizeRatio = 1;
    }
    else
    {
        l->absSize = -1;
        l->sizeRatio = size;
    }

    l->resize = true;
}

bool Layout_SetZIndex(Layout* l, int zindex)
{
    if (!l) return false;
    l->zindex = zindex;
    if (!l->parent) return true;
    return insert_child_by_zindex(l->parent, l);
}

void Layout_SetLayoutStrategy(Layout* l, LayoutStrategy s)
{
    if (!l) return;

    l->layoutStrategy = s;

    for (int i = 0; i < l->childrenCount; i++)
    {
        l->children[i]->resize = true;
    }
}

void Layout_SetDims(Layout* l, int x, int y, int width, int height)
{
    if (!l) return;

    l->x_rel = x;
    l->y_rel = y;
    l->width = width;
    l->height = height;
    l->resize = true;
}

void Layout_DetatchFromParent(Layout * child)
{
    if (!child) return;
    
    if (child->parent)
    {
        Layout * p = child->parent;
        for (int i = 0; i < p->childrenCount; i++)
        {
            if (p->children[i] == child)
            {
                removeArrayItem(p->children, sizeof(Layout*), p->childrenCount, i);
                p->childrenCount--;
                break;
            }
        }
        child->parent = NULL;
        Layout_SizeRefreshSameParams(p, false);
    }
}

Layout * Layout_RemoveChildIdx(Layout * parent, int idx)
{
    if (!parent || idx < 0 || idx >= parent->childrenCount) return NULL;

    Layout * child = parent->children[parent->childrenCount];
    Layout_DetatchFromParent(child);
    return child;
}

bool Layout_DestroyChildIdx(Layout * parent, int idx)
{
    Layout * child = Layout_RemoveChildIdx(parent, idx);
    Layout_Destroy(child);

    return child != NULL;
}

// PRIV

bool layout_contains_point(Layout *layout, int x, int y)
{
    if (!layout) return false;

    int left   = layout->x;
    int top    = layout->y;
    int right  = layout->x + layout->width - 1;
    int bottom = layout->y + layout->height - 1;

    return (x >= left && x <= right && y >= top && y <= bottom);
}

void compute_layout_sizes(int total_size, struct Layout** elements, int* ret, int count) {
    if (count <= 0 || total_size <= 0) return;

    int remaining = total_size;
    double total_ratio = 0.0;

    // First pass: handle absolute sizes and visibility
    for (int i = 0; i < count; i++) {
        if (!elements[i]->visible) {
            ret[i] = 0;
        } else if (elements[i]->absSize >= 0) {
            ret[i] = elements[i]->absSize;
            remaining -= ret[i];
        } else {
            ret[i] = -1; // mark as ratio-sized
            total_ratio += elements[i]->sizeRatio;
        }
    }

    if (remaining < 0) remaining = 0; // prevent overflow if absSizes exceed total

    // Second pass: distribute remaining space based on ratio
    double ideal_sizes[LAYOUT_MAX_DIV];
    int sum_ratio_sizes = 0;

    for (int i = 0; i < count; i++) {
        if (!elements[i]->visible || elements[i]->absSize >= 0) continue;

        ideal_sizes[i] = (elements[i]->sizeRatio / total_ratio) * remaining;
        ret[i] = (int)floor(ideal_sizes[i]);
        sum_ratio_sizes += ret[i];
    }

    // Distribute leftover pixels
    int leftover = remaining - sum_ratio_sizes;
    while (leftover > 0) {
        double max_fraction = -1.0;
        int max_index = -1;

        for (int i = 0; i < count; i++) {
            if (!elements[i]->visible || elements[i]->absSize >= 0) continue;
            double fraction = ideal_sizes[i] - ret[i];
            if (fraction > max_fraction) {
                max_fraction = fraction;
                max_index = i;
            }
        }

        if (max_index >= 0) {
            ret[max_index]++;
            leftover--;
        } else {
            break;
        }
    }
}


void DrawThisLayout(Layout * l, bool force)
{
    if (!l) return;

    if ((l->redraw || force) && l->win)
    {
        l->redraw = false;
        
        werase(l->win);
        if (l->vtable.draw) l->vtable.draw(l, l->win, l->x, l->y, l->width, l->height);
        wnoutrefresh(l->win);
    }
}

void Layout_HitTest(Layout * l, int x, int y, Layout* hovBuff[], int hovBuffArrSize, int* hovBuffCurrLen)
{
    if (!l || !hovBuffCurrLen || !hovBuff) return;
    if (*hovBuffCurrLen >= hovBuffArrSize) return;
    if (!layout_contains_point(l, x, y)) return;

    hovBuff[*hovBuffCurrLen] = l;
    (*hovBuffCurrLen)++;

    for (int i = 0; i < l->childrenCount; i++)
    {
        Layout_HitTest(l->children[i], x, y, hovBuff, hovBuffArrSize, hovBuffCurrLen);
    }
}

void removeArrayItem(void* arr, int eleSize, int eleCount, int idx)
{
    if (!arr) return;
    int tail_count = (eleCount - idx) - 1;
    if (tail_count > 0)
    {
        memmove(
            (char*)arr + idx * eleSize,
            (char*)arr + (idx + 1) * eleSize,
            tail_count * eleSize
        );
    }
}

void drawPhase1(Layout* l, bool force)
{
    if (!l) return;

    if ((l->redraw || force) && l->win && l->pan)
    {
        l->redraw = false;
        
        werase(l->win);
        if (l->vtable.draw)
        {
            l->vtable.draw(l, l->win, l->x, l->y, l->width, l->height);
        }
    }

    for (int i = 0; i < l->childrenCount; i++)
    {
        drawPhase1(l->children[i], force);
    }
}
void drawPhase2(Layout* l)
{
    if (!l) return;

    //if (l->win) wnoutrefresh(l->win);
    if (l->pan) top_panel(l->pan);

    for (int i = 0; i < l->childrenCount; i++)
    {
        drawPhase2(l->children[i]);
    }
}

bool insert_child_by_zindex(Layout* parent, Layout* child)
{
    if (!parent || ! child) return false;

    // first check if the child is here already.
    // if it is, then remove and place again
    for (int i = 0; i < parent->childrenCount; i++)
    {
        if (parent->children[i] == child)
        {
            removeArrayItem(parent->children, sizeof(Layout*), parent->childrenCount, i);
            parent->childrenCount--;
            break;
        }
    }

    if (parent->childrenCount >= LAYOUT_MAX_DIV) return false;

    int insertIndex = 0;

    // Find where to insert (keep ascending zIndex order)
    while (insertIndex < parent->childrenCount &&
           parent->children[insertIndex]->zindex <= child->zindex) {
        insertIndex++;
    }

    // Shift children to make room
    for (int i = parent->childrenCount; i > insertIndex; i--) {
        parent->children[i] = parent->children[i - 1];
    }

    // Insert the new child
    parent->children[insertIndex] = child;
    parent->childrenCount++;
    return true;
}

void layoutStat_horz(Layout* l, bool force)
{
    if (!l) return;

    int innerWidth = MAX(0, l->width - (l->pad_left + l->pad_right));
    int innerHeight = MAX(0, l->height - (l->pad_up + l->pad_down));;

    int xOffset = l->pad_left;
    int yOffset = l->pad_up;

    int corner_x = l->x + xOffset;
    int corner_y = l->y + yOffset;
    

    int c_x;
    int c_y;
    int c_w;
    int c_h;
    int ans[LAYOUT_MAX_DIV];

    int curr_x = corner_x;
    compute_layout_sizes(innerWidth, l->children, ans, l->childrenCount);
    for (int i = 0; i < l->childrenCount; i++)
    {
        c_w = ans[i];
        c_h = innerHeight;

        c_x = curr_x;
        c_y = corner_y;

        curr_x += ans[i];
        
        l->children[i]->x_rel = c_x - l->x;
        l->children[i]->y_rel = c_y - l->y;

        Layout_SizeRefresh(l->children[i], c_x, c_y, c_w, c_h, force);
    }
}

void layoutStat_vert(Layout* l, bool force)
{
    if (!l) return;

    int innerWidth = MAX(0, l->width - (l->pad_left + l->pad_right));
    int innerHeight = MAX(0, l->height - (l->pad_up + l->pad_down));;

    int xOffset = l->pad_left;
    int yOffset = l->pad_up;

    int corner_x = l->x + xOffset;
    int corner_y = l->y + yOffset;
    

    int c_x;
    int c_y;
    int c_w;
    int c_h;
    int ans[LAYOUT_MAX_DIV];

    int curr_y = corner_y;
    compute_layout_sizes(innerHeight, l->children, ans, l->childrenCount);
    for (int i = 0; i < l->childrenCount; i++)
    {
        c_w = innerWidth;
        c_h = ans[i];

        c_x = corner_x;
        c_y = curr_y;

        curr_y += ans[i];

        l->children[i]->x_rel = c_x - l->x;
        l->children[i]->y_rel = c_y - l->y;

        Layout_SizeRefresh(l->children[i], c_x, c_y, c_w, c_h, force);
    }
}

void layoutStat_abs(Layout* l, bool force)
{
    if (!l) return;

    int c_x;
    int c_y;
    int c_w;
    int c_h;

    for (int i = 0; i < l->childrenCount; i++)
    {
        c_w = l->children[i]->width;
        c_h = l->children[i]->height;

        c_x = l->x + l->children[i]->x_rel;
        c_y = l->y + l->children[i]->y_rel;

        Layout_SizeRefresh(l->children[i], c_x, c_y, c_w, c_h, force);
    }
}
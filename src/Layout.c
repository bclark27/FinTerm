#include "Layout.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "Logger.h"

// PRIV DECLERATIONS
bool layout_contains_point(Layout *layout, int x, int y);
void compute_ratios(int total_size, struct Layout** elements, int* ret, int count);
void DrawThisLayout(Layout * l, bool force);
void processClick(Layout * l, LayoutClickedEvent* evt);
void removeArrayItem(void* arr, int eleSize, int eleCount, int idx);

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
    l->orientation = LayoutOrientation_H;
    l->visible = true;
}

void Layout_Destroy(Layout * l)
{
    if (!l) return;

    if (l->onDestroy) l->onDestroy(l);
    
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


    if (l->win) delwin(l->win);
    l->win = NULL;
    free(l);
}

void Layout_Draw(Layout * l, bool force)
{
    if (!l) return;

    if (l->isDirty || force)
    {
        force = true;
        DrawThisLayout(l, force);
    }

    for (int i = 0; i < l->childrenCount; i++)
        if (l->children[i])
            Layout_Draw(l->children[i], force);
}

void Layout_SizeRefresh(Layout * l)
{
    if (!l) return;
    
    if (l->win)
    {
        delwin(l->win);
        l->win = NULL;
    }
    
    l->win = newwin(l->height, l->width, l->abs_y, l->abs_x);
    DrawThisLayout(l, true);

    int ans[LAYOUT_MAX_DIV];
    if (l->orientation == LayoutOrientation_V)
    {
        int curr_y = l->abs_y;
        compute_ratios(l->height, l->children, ans, l->childrenCount);
        for (int i = 0; i < l->childrenCount; i++)
        {
            l->children[i]->width = l->width;
            l->children[i]->height = ans[i];

            l->children[i]->abs_x = l->abs_x;
            l->children[i]->abs_y = curr_y;

            curr_y += ans[i];
        }
    }
    else
    {
        int curr_x = l->abs_x;
        compute_ratios(l->width, l->children, ans, l->childrenCount);
        for (int i = 0; i < l->childrenCount; i++)
        {
            l->children[i]->width = ans[i];
            l->children[i]->height = l->height;

            l->children[i]->abs_x = curr_x;
            l->children[i]->abs_y = l->abs_y;

            curr_x += ans[i];
        }
    }
    
    
    for (int i = 0; i < l->childrenCount; i++)
    {
        Layout_SizeRefresh(l->children[i]);
    }
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

    parent->children[parent->childrenCount++] = child;
    child->parent = parent;
    
    Layout_SizeRefresh(parent);

    return true;
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
        Layout_SizeRefresh(p);
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


void Layout_ProcessClick(Layout * l, MEVENT* mevent)
{
    if (!l || !mevent) return;

    LayoutClickedEvent evt;
    evt.mevent = mevent;
    evt.click_x = mevent->x;
    evt.click_y = mevent->y;
    evt.left_click = mevent->bstate & BUTTON1_CLICKED;
    evt.left_click = mevent->bstate & BUTTON2_CLICKED;
    processClick(l, &evt);
}

// PRIV

bool layout_contains_point(Layout *layout, int x, int y)
{
    if (!layout) return false;

    int left   = layout->abs_x;
    int top    = layout->abs_y;
    int right  = layout->abs_x + layout->width;
    int bottom = layout->abs_y + layout->height;

    return (x >= left && x <= right && y >= top && y <= bottom);
}

void compute_ratios(int total_size, struct Layout** elements, int* ret, int count) 
{
    if (count <= 0 || total_size <= 0) return;

    double total_weight = 0.0;
    for (int i = 0; i < count; i++) {
        total_weight += elements[i]->sizeRatio;
    }
    // First pass: compute ideal floating sizes and round down
    int sum = 0;
    double ideal_sizes[LAYOUT_MAX_DIV];
    for (int i = 0; i < count; i++) {
        ideal_sizes[i] = (elements[i]->sizeRatio / total_weight) * total_size;
        ret[i] = (int)floor(ideal_sizes[i]);
        sum += ret[i];
    }

    // Distribute remaining pixels
    int remaining = total_size - sum;
    while (remaining > 0) {
        double max_fraction = -1.0;
        int max_index = -1;

        for (int i = 0; i < count; i++) {
            double fraction = ideal_sizes[i] - ret[i];
            if (fraction > max_fraction) {
                max_fraction = fraction;
                max_index = i;
            }
        }

        if (max_index >= 0) {
            ret[max_index]++;
            remaining--;
        } else {
            break;  // Should not happen
        }
    }
}


void DrawThisLayout(Layout * l, bool force)
{
    if (!l) return;

    if ((l->isDirty || force) && l->win)
    {
        l->isDirty = false;
        
        werase(l->win);
        if (l->draw) l->draw(l, l->win, l->abs_x, l->abs_y, l->width, l->height);
        wnoutrefresh(l->win);
    }
}

void processClick(Layout * l, LayoutClickedEvent* evt)
{
    if (!l || !evt || !evt->mevent) return;

    Layout* clickedChild = NULL;
    for (int i = 0; i < l->childrenCount; i++)
    {
        if (layout_contains_point(l->children[i], evt->mevent->x, evt->mevent->y))
        {
            clickedChild = l->children[i];
            processClick(l->children[i], evt);
            break;
        }
    }

    if (!clickedChild)
    {
        evt->clickedLayout = l;
        evt->click_rx = evt->click_x - l->abs_x;
        evt->click_ry = evt->click_y - l->abs_y;
    }

    if (l->onClick)
        l->onClick(l, evt);
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
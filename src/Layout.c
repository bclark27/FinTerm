#include "Layout.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

// PRIV DECLERATIONS
bool layout_contains_point(Layout *layout, int x, int y);
void compute_ratios(int total_size, struct Layout** elements, int* ret, int count);
void DrawThisLayout(Layout * l, bool force);
void processClick(Layout * l, LayoutClickedEvent* evt);

// PUBLIC FUNCS




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

Layout * Layout_AddNewChild(Layout * l)
{
    if (!l || l->childrenCount >= LAYOUT_MAX_DIV) return NULL;
    
    Layout * n = __Layout_Create();
    l->children[l->childrenCount] = n;
    n->parent = l;
    l->childrenCount++;

    Layout_SizeRefresh(l);

    return n;
}

bool Layout_RemoveChildPtr(Layout * l, Layout * c)
{
    if (!l || !c) return false;
    
    for (int i = 0; i < l->childrenCount; i++)
    {
        if (l->children[i] == c)
        {
            return Layout_RemoveChildIdx(l, i);
        }
    }

    return false;
}

bool Layout_RemoveChildIdx(Layout * l, int idx)
{
    if (idx < 0 || idx >= l->childrenCount) return false;
    
    __Layout_Destroy(l->children[idx]);

    int tail_count = (l->childrenCount - idx) - 1;
    if (tail_count > 0) memmove(&l->children[idx], &l->children[idx + 1], tail_count * sizeof(Layout*));

    l->childrenCount--;

    Layout_SizeRefresh(l);

    return true;
}

void Layout_RemoveChildren(Layout * l)
{
    if (!l) return;

    for (int i = 0; i < l->childrenCount; i++)
    {
        __Layout_Destroy(l->children[i]);
    }

    l->childrenCount = 0;
}

Layout * __Layout_Create()
{
    Layout* l = calloc(1, sizeof(Layout));
    l->sizeRatio = 1;
    l->orientation = LayoutOrientation_H;
    l->visible = true;
    return l;
}

void __Layout_Destroy(Layout * l)
{
    if (!l) return;

    Layout_RemoveChildren(l);

    if (l->win)
        delwin(l->win);
    l->win = NULL;
    l->parent = NULL;
    free(l);
}

void __Layout_ProcessClick(Layout * l, MEVENT* mevent)
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
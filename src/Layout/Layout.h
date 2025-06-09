#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <ncursesw/ncurses.h>

#include "../Logger.h"
#include "../InputManager.h"

#define LAYOUT_MAX_DIV  (20)

// types

// not used in layout base, but many derived types might use it in some way
typedef enum Alignment
{
    Alignment_Start,
    Alignment_Center,
    Alignment_End,
} Alignment;

typedef enum LayoutOrientation
{
    LayoutOrientation_V,
    LayoutOrientation_H,
} LayoutOrientation;

typedef enum LayoutBubbleEventType
{
    LayoutBubbleEventType_Clicked  // LayoutBubbleEvent_Clicked
} LayoutBubbleEventType;

// pre defs

struct Layout;
struct LayoutBubbleEvent;

// callbacks
typedef void (*LayoutDrawCallback)(struct Layout* l, WINDOW *win, int x, int y, int width, int height);
typedef void (*OnLayoutDestroy)(struct Layout* l);

// called with mouse movement event, but ONLY if it is the first move interaction int the layout
typedef void (*OnLayoutPointerEnter)(struct Layout* l, InputEvent* e);
// called with mouse movement event, but ONLY if it is the first movement outside of the layout
typedef void (*OnLayoutPointerExit)(struct Layout* l, InputEvent* e);
// called with mouse movement event, will be called for movement while pointer is in this layout, excluding the OnLayoutPointerEnter and OnLayoutPointerExit
typedef void (*OnLayoutPointerMove)(struct Layout* l, InputEvent* e);
// called only when the GUI manager decides this layout is now in focus
typedef void (*OnLayoutPointerFocus)(struct Layout* l, InputEvent* e);
// called only when the GUI manager decides this layout is no longer in focus
typedef void (*OnLayoutPointerUnFocus)(struct Layout* l, InputEvent* e);

typedef struct Layout_VT
{
    LayoutDrawCallback draw;
    OnLayoutDestroy onDestroy;
    OnLayoutPointerEnter onPtrEnter;
    OnLayoutPointerExit onPtrExit;
    OnLayoutPointerMove onPtrMove;
    OnLayoutPointerFocus onFocus;
    OnLayoutPointerUnFocus onUnFocus;
} Layout_VT;

typedef struct Layout
{
    Layout_VT vtable;

    WINDOW * win;

    // this list should be mantained such that all in use children are compressed to the front of tthe list
    struct Layout * children[LAYOUT_MAX_DIV];
    struct Layout * parent;
    LayoutOrientation orientation;

    double sizeRatio;
    int absSize;
    int width;
    int height;
    int abs_x;
    int abs_y;
    int childrenCount;

    bool isDirty;
    bool isHover;
    bool isFocus;
    bool visible;
} Layout;

Layout * Layout_Create();
void Layout_Init(Layout * l);
void Layout_Destroy(Layout * l);
void Layout_Draw(Layout * l, bool force);
void Layout_SizeRefresh(Layout * l);
bool Layout_AddChild(Layout * parent, Layout * child);

void Layout_DetatchFromParent(Layout * child);
Layout * Layout_RemoveChildIdx(Layout * parent, int idx);
bool Layout_DestroyChildIdx(Layout * parent, int idx);

void Layout_GetChildNodeAtPoint(Layout * l, int x, int y, Layout* hovBuff[], int hovBuffArrSize, int* hovBuffCurrLen);

void Layout_Hover(Layout* l, InputEvent* e, bool hover);
void Layout_Focus(Layout* l, InputEvent* e, bool focus);


#endif
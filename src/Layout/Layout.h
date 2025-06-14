#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <ncursesw/ncurses.h>
#include <panel.h>

#include "../Logger.h"
#include "../InputManager.h"
#include "../Colors.h"
#include "../Common.h"

#define LAYOUT_MAX_DIV  (100)

#define REDRAW(l)    (((Layout*)(l))->redraw = true)
#define RESIZE(l)    (((Layout*)(l))->resize = true)

// types

// pre defs

struct Layout;
struct BblEvt;

// not used in layout base, but many derived types might use it in some way
typedef enum Alignment
{
    Alignment_Start,
    Alignment_Center,
    Alignment_End
} Alignment;

typedef enum LayoutStrategy
{
    LayoutStrategy_vert,
    LayoutStrategy_horz,
    LayoutStrategy_abs
} LayoutStrategy;

// all bubble type events go here

typedef enum BblEvtType
{
    BblEvtType_Click,
    BblEvtType_Scroll,
    BblEvtType_Key,
    BblEvtType_Focus
} BblEvtType;

typedef struct BblEvt_Click
{
    int x;
    int y;
    bool leftClick;
    bool rightClick;
    bool midClick;
} BblEvt_Click;

typedef struct BblEvt_Key
{
    int raw;
    char keyCode;
    bool isAscii;
    bool isCtrl;
    bool isSpecial;
} BblEvt_Key;

typedef struct BblEvt_Scroll
{
    int x;
    int y;
    bool up;
    bool down;
} BblEvt_Scroll;

typedef struct BblEvt_Focus
{
    struct Layout* target;
    bool focus;
} BblEvt_Focus;

typedef struct BblEvt
{
    struct Layout* focus_target;
    struct Layout* hit_target;
    BblEvtType type;
    bool handled;
    union
    {
        struct BblEvt_Click click;
        struct BblEvt_Key key;
        struct BblEvt_Scroll scroll;
        struct BblEvt_Focus focus;
    } data;
} BblEvt;

// draw!!!
typedef void (*LayoutDrawCallback)(struct Layout* l, WINDOW *win, int x, int y, int width, int height);
// called when the layout is about to be freed
typedef void (*OnLayoutDestroy)(struct Layout* l);
// called with mouse movement event, but ONLY if it is the first move interaction int the layout
typedef void (*OnLayoutPointerEnter)(struct Layout* l, InputEvent* e);
// called with mouse movement event, but ONLY if it is the first movement outside of the layout
typedef void (*OnLayoutPointerExit)(struct Layout* l, InputEvent* e);
// called with mouse movement event, will be called for movement while pointer is in this layout, excluding the OnLayoutPointerEnter and OnLayoutPointerExit
typedef void (*OnLayoutPointerMove)(struct Layout* l, InputEvent* e);
// called only when the GUI manager decides this layout is now in focus
typedef void (*OnLayoutFocus)(struct Layout* l);
// called only when the GUI manager decides this layout is no longer in focus
typedef void (*OnLayoutUnFocus)(struct Layout* l);
// called during the target->root pass
typedef void (*OnLayoutBblEvt)(struct Layout* l, struct BblEvt* e);
// called during the root->target pass
typedef void (*OnLayoutBblEvtCapture)(struct Layout* l, struct BblEvt* e);

typedef struct Layout_VT
{
    LayoutDrawCallback draw;
    OnLayoutDestroy onDestroy;
    OnLayoutPointerEnter onPtrEnter;
    OnLayoutPointerExit onPtrExit;
    OnLayoutPointerMove onPtrMove;
    OnLayoutFocus onFocus;
    OnLayoutUnFocus onUnFocus;
    OnLayoutBblEvt onBblEvt;
    OnLayoutBblEvtCapture onBblEvtCapture;
} Layout_VT;

typedef struct Layout
{
    Layout_VT vtable;

    WINDOW * win;
    PANEL * pan;

    // this list should be mantained such that all in use children are compressed to the front of the list
    struct Layout * children[LAYOUT_MAX_DIV];
    struct Layout * parent;
    
    double sizeRatio;
    
    LayoutStrategy layoutStrategy;
    int absSize;
    int width;
    int height;
    int x;
    int y;
    int x_rel;
    int y_rel;
    int childrenCount;
    int pad_up;
    int pad_down;
    int pad_left;
    int pad_right;
    int zindex;
    
    bool redraw;
    bool resize;
    bool isHover;
    bool isFocus;
    bool visible;
    bool acceptsLiteralTab;
    bool focusable;
} Layout;


Layout * Layout_Create();
void Layout_Init(Layout * l);
void Layout_Destroy(Layout * l);
void Layout_Draw(Layout * l, bool force);
void Layout_SizeRefresh(Layout * l, int x, int y, int width, int height, bool force);
void Layout_SizeRefreshSameParams(Layout* l, bool force);
bool Layout_AddChild(Layout * parent, Layout * child);
void Layout_SetVis(Layout * l, bool visible);
void Layout_SetSize(Layout * l, int size, bool isAbs);
bool Layout_SetZIndex(Layout* l, int zindex);
void Layout_SetLayoutStrategy(Layout* l, LayoutStrategy s);
void Layout_SetDims(Layout* l, int x, int y, int width, int height);

void Layout_DetatchFromParent(Layout * child);
Layout * Layout_RemoveChildIdx(Layout * parent, int idx);
bool Layout_DestroyChildIdx(Layout * parent, int idx);

void Layout_HitTest(Layout * l, int x, int y, Layout* hovBuff[], int hovBuffArrSize, int* hovBuffCurrLen);


#endif

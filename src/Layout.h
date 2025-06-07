#ifndef LAYOUT_H_
#define LAYOUT_H_

#include <ncursesw/ncurses.h>

#define LAYOUT_MAX_DIV  (20)

// types

typedef enum LayoutOrientation
{
    LayoutOrientation_V,
    LayoutOrientation_H,
} LayoutOrientation;

// pre defs

struct Layout;
struct LayoutClickedEvent;

// callbacks
typedef void (*LayoutDrawCallback)(struct Layout*, WINDOW *win, int x, int y, int width, int height);
typedef void (*LayoutClickedCallback)(struct Layout*, struct LayoutClickedEvent*);

typedef struct Layout
{
    WINDOW * win;

    // this list should be mantained such that all in use children are compressed to the front of tthe list
    struct Layout * children[LAYOUT_MAX_DIV];
    struct Layout * parent;
    LayoutOrientation orientation;
    LayoutDrawCallback draw;
    LayoutClickedCallback onClick;

    double sizeRatio;
    int width;
    int height;
    int abs_x;
    int abs_y;
    int childrenCount;

    bool isDirty;

    bool visible;
} Layout;

typedef struct LayoutClickedEvent
{
    MEVENT* mevent;
    Layout* clickedLayout;
    int click_x;
    int click_y;

    int click_rx;
    int click_ry;

    bool right_click;
    bool left_click;

} LayoutClickedEvent;

void Layout_Draw(Layout * l, bool force);
void Layout_SizeRefresh(Layout * l);
Layout * Layout_AddNewChild(Layout * l);
bool Layout_RemoveChildPtr(Layout * l, Layout * c);
bool Layout_RemoveChildIdx(Layout * l, int idx);
void Layout_RemoveChildren(Layout * l);


Layout * __Layout_Create();
bool __Layout_AddChild(Layout * l);
void __Layout_Destroy(Layout * l);
void __Layout_ProcessClick(Layout * l, MEVENT* mevent);

#endif
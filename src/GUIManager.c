#include "GUIManager.h"
#include "Logger.h"
#include <ncursesw/ncurses.h>
#include <locale.h>

typedef struct GUIManager
{
    Layout* root;
    Layout* focused;
    bool init;
} GUIManager;

static GUIManager manager;

// declerations
void getCurrTermSize(int* rows, int* cols);

// PUBLIC

void GUIManager_Init()
{
    if (manager.init) return;

    setlocale(LC_ALL, "en_US.UTF-8");
    initscr();
    start_color();
    use_default_colors();
    curs_set(FALSE);
    keypad(stdscr, TRUE);   // Enable function keys like KEY_RESIZE
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);  // Enable mouse events

    manager.init = true;
    manager.root = Layout_Create();
    GUIManager_SizeRefresh();

    for (int i = 0; i < 2; i++)
    {
        GUIManager_SizeRefresh();
        GUIManager_Draw(true);
        refresh();
    }
}

void GUIManager_Destroy()
{
    if (!manager.init) return;
    Layout_Destroy(manager.root);
    manager.root = NULL;
    manager.init = false;
}

Layout* GUIManager_GetRoot()
{
    if (!manager.init) return NULL;
    return manager.root;
}

Layout* GUIManager_GetFocused()
{
    if (!manager.init) return NULL;
    return manager.focused;
}

void GUIManager_OnKeys(InputEvent* evts, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (evts[i].isMouse)
        {
            Layout* clicked = Layout_GetChildNodeAtPoint(manager.root, evts[i].mevent.x, evts[i].mevent.y);
            if (clicked)
            {
                manager.focused = clicked;
                
                LayoutBubbleEvent bbl;
                LayoutBubbleEvent_Clicked bbl_click;
    
                bbl_click.mevent = &evts[i].mevent;
                bbl_click.click_x = evts[i].mevent.x;
                bbl_click.click_y = evts[i].mevent.y;
                bbl_click.click_rx = evts[i].mevent.x - clicked->abs_x;
                bbl_click.click_ry = evts[i].mevent.y - clicked->abs_y;
                bbl_click.right_click = evts[i].mevent.bstate & BUTTON3_CLICKED;
                bbl_click.left_click = evts[i].mevent.bstate & BUTTON1_CLICKED;
                bbl_click.clickedLayout = clicked;

                bbl.type = LayoutBubbleEventType_Clicked;
                bbl.evt = &bbl_click;

                Layout_BubbleUp(clicked, &bbl);
            }

        }
    }
}

void GUIManager_SizeRefresh()
{
    if (!manager.init) return;

    int r,c;
    getCurrTermSize(&r, &c);

    if (manager.root->width != c || manager.root->height != r)
    {
        manager.root->width = c;
        manager.root->height = r;
        Layout_SizeRefresh(manager.root);
    }
}

void GUIManager_Draw(bool force)
{
    Layout_Draw(manager.root, force);
    doupdate();
}

// PRIV

void getCurrTermSize(int* rows, int* cols)
{
  int r, c;
  getmaxyx(stdscr, r, c);
  *rows = r;
  *cols = c;
}
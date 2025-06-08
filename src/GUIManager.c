#include "GUIManager.h"
#include "Logger.h"
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <string.h>

#define MAX_DEPTH   (1000)

typedef struct GUIManager
{
    Layout* root;
    Layout* focused;
    int hovBuffCurrSize;
    Layout* hovBuffer[MAX_DEPTH];
    bool init;
} GUIManager;

static GUIManager manager;

// declerations
void getCurrTermSize(int* rows, int* cols);
void compareHovBuffs(Layout* ogHovBuff[], int ogHovBuffSize, Layout* newHovBuff[], int newHovBuffSize, Layout* exiting[], int* exitingSize, Layout* entering[], int* enteringSize);

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

    manager.hovBuffCurrSize = 0;
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
        LayoutBubbleEvent bbl;

        if (evts[i].isMouse)
        {
            
            Layout* newHovBuff[MAX_DEPTH];
            int newHovBuffLen = 0;
            Layout_GetChildNodeAtPoint(manager.root, evts[i].mevent.x, evts[i].mevent.y, newHovBuff, MAX_DEPTH, &newHovBuffLen);

            Layout* entering[MAX_DEPTH];
            Layout* exiting[MAX_DEPTH];
            int enteringSize = 0;
            int exitingSize = 0;
            compareHovBuffs(
                manager.hovBuffer, manager.hovBuffCurrSize,
                newHovBuff, newHovBuffLen,
                exiting, &exitingSize,
                entering, &enteringSize
            );

            for (int i = 0; i < enteringSize; i++)
            {
                Layout_Hover(entering[i], true);
            }
            for (int i = 0; i < exitingSize; i++)
            {
                Layout_Hover(exiting[i], false);
            }

            memmove(manager.hovBuffer, newHovBuff, newHovBuffLen * sizeof(Layout*));
            manager.hovBuffCurrSize = newHovBuffLen;

            Logger_Log("A: %d\nB: %d\n\n", newHovBuffLen, exitingSize);
/*
            manager.hovered = interacting;
            if (evts[i].leftClick || evts[i].rightClick)
            {
                manager.focused = interacting;
            }
            if (clicked)
            {
                manager.focused = clicked;
                
                
                LayoutBubbleEvent_Clicked bbl_click;
                bbl_click.click_rx = evts[i].mevent.x - clicked->abs_x;
                bbl_click.click_ry = evts[i].mevent.y - clicked->abs_y;
                bbl_click.clickedLayout = clicked;
                bbl_click.event = &evts[i];
                
                bbl.type = LayoutBubbleEventType_Clicked;
                bbl.evt = &bbl_click;
                bbl.focused = manager.focused;
                
                Layout_BubbleUp(clicked, &bbl);
            }
            */
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

void compareHovBuffs(Layout* ogHovBuff[], int ogHovBuffSize,
    Layout* newHovBuff[], int newHovBuffSize,
    Layout* exiting[], int* exitingSize,
    Layout* entering[], int* enteringSize) 
    {
    // Initialize counters
    *exitingSize = 0;
    *enteringSize = 0;

    // Find exiting: in og but not in new
    for (int i = 0; i < ogHovBuffSize; ++i) 
    {
        Layout* l = ogHovBuff[i];
        bool found = false;
        for (int j = 0; j < newHovBuffSize; ++j) 
        {
            if (newHovBuff[j] == l) 
            {
                found = true;
                break;
            }
        }
        if (!found) {
            exiting[(*exitingSize)++] = l;
        }
    }

    // Find entering: in new but not in og
    for (int i = 0; i < newHovBuffSize; ++i) 
    {
        Layout* l = newHovBuff[i];
        bool found = false;
        for (int j = 0; j < ogHovBuffSize; ++j) 
        {
            if (ogHovBuff[j] == l) {
                found = true;
                break;
            }
        }
        if (!found) 
        {
            entering[(*enteringSize)++] = l;
        }
    }
}
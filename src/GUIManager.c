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
void compareHovBuffs(
    Layout* ogHovBuff[], int ogHovBuffSize, 
    Layout* newHovBuff[], int newHovBuffSize, 
    Layout* exiting[], int* exitingSize, 
    Layout* entering[], int* enteringSize,
    Layout* stillHovering[], int* stillHoveringSize);

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
        // first if it is a mouse event then redo all the hover states
        // also redo the focus guy
        if (evts[i].isMouse)
        {
            Layout* newHovBuff[MAX_DEPTH];
            int newHovBuffLen = 0;
            Layout_GetChildNodeAtPoint(manager.root, evts[i].mevent.x, evts[i].mevent.y, newHovBuff, MAX_DEPTH, &newHovBuffLen);
            
            Layout* entering[MAX_DEPTH];
            Layout* exiting[MAX_DEPTH];
            Layout* stillHovering[MAX_DEPTH];
            int enteringSize = 0;
            int exitingSize = 0;
            int stillHoveringSize = 0;
            compareHovBuffs(
                manager.hovBuffer, manager.hovBuffCurrSize,
                newHovBuff, newHovBuffLen,
                exiting, &exitingSize,
                entering, &enteringSize,
                stillHovering, &stillHoveringSize
            );
            
            memmove(manager.hovBuffer, newHovBuff, newHovBuffLen * sizeof(Layout*));
            manager.hovBuffCurrSize = newHovBuffLen;

            Layout* newFocus = NULL;
            int lastIdx = manager.hovBuffCurrSize - 1;
            if (lastIdx >= 0)
            {
                newFocus = manager.hovBuffer[lastIdx];
            }

            bool isFocusEvent = evts[i].leftClick;

            // first do the unfocus and all exiting hovers
            if (newFocus != manager.focused && 
                manager.focused && 
                isFocusEvent)
            {
                Layout_Focus(manager.focused, &evts[i], false);
            }

            for (int k = 0; k < exitingSize; k++)
            {
                Layout_Hover(exiting[k], &evts[i], false);
            }

            // next do all the still hovering guys
            for (int k = 0; k < stillHoveringSize; k++)
            {
                Layout_Hover(stillHovering[k], &evts[i], true);
            }

            for (int k = 0; k < enteringSize; k++)
            {
                Layout_Hover(entering[k], &evts[i], true);
            }
            
            if (newFocus != manager.focused && 
                newFocus && 
                isFocusEvent)
            {
                Layout_Focus(newFocus, &evts[i], true);
            }

            if (isFocusEvent)
            {
                manager.focused = newFocus;
            } 
                

            continue;
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
    Layout* entering[], int* enteringSize,
    Layout* stillHovering[], int* stillHoveringSize) 
    {
    // Initialize counters
    *exitingSize = 0;
    *enteringSize = 0;
    *stillHoveringSize = 0;

    bool isExitOrEnter[MAX_DEPTH];
    memset(isExitOrEnter, 0, sizeof isExitOrEnter);

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
            isExitOrEnter[i] = true;
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
            isExitOrEnter[i] = true;
        }
    }

    for (int i = 0; i < ogHovBuffSize; i++)
    {
        Layout* l = ogHovBuff[i];
        if (!isExitOrEnter[i])
        {
            stillHovering[(*stillHoveringSize)++] = l;
        }
    }
}
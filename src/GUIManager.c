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
void rootEvtCapture(Layout* l, BblEvt* e);
void dispatchMouseEvt(InputEvent* evt, Layout* target);
void focusLayout(Layout* l);
void handleMouseEvent(InputEvent* evt);
void getCurrTermSize(int* rows, int* cols);
void compareHovBuffs(
    Layout* ogHovBuff[], int ogHovBuffSize, 
    Layout* newHovBuff[], int newHovBuffSize, 
    Layout* exiting[], int* exitingSize, 
    Layout* entering[], int* enteringSize,
    Layout* stillHovering[], int* stillHoveringSize);
void doBubble(Layout * target, BblEvt * e);

// layout vtable

static Layout_VT vtable = 
{
    .draw = NULL,
    .onDestroy = NULL,
    .onPtrEnter = NULL,
    .onPtrExit = NULL,
    .onPtrMove = NULL,
    .onFocus = NULL,
    .onUnFocus = NULL,
    .onBblEvt = NULL,
    .onBblEvtCapture = rootEvtCapture,
};

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
    manager.root->vtable = vtable;
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
            handleMouseEvent(&evts[i]);
        }
        else
        {
            Layout* target = manager.root;
            if (manager.focused) target = manager.focused;

            BblEvt be;
            BblEvt_Key kbe;

            kbe.raw = evts[i].raw;
            kbe.isAscii = evts[i].isAscii;
            kbe.isCtrl = evts[i].isCtrl;
            kbe.isSpecial = evts[i].isSpecial;
            kbe.keyCode = evts[i].keyCode;

            be.evtData = &kbe;
            be.handled = false;
            be.target = target;
            be.type = BblEvtType_Key;

            doBubble(target, &be);
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

void rootEvtCapture(Layout* l, BblEvt* e)
{
    if (!l || !e) return;

    if (e->type == BblEvtType_Key)
    {
        BblEvt_Key* ke = (BblEvt_Key*)e->evtData;

        if (e->target && 
            !e->target->acceptsLiteralTab &&
            (ke->raw == 0x9 || ke->raw == KEY_BTAB))
        {
            e->handled = true;

            bool shiftForward = ke->raw == 0x9;

            // do a shift tab
            Logger_Log("Tab Nav\n");
            return;
        }
    }
}

void handleMouseEvent(InputEvent* evt)
{
    if (!evt->isMouse) return;

    Layout* newHovBuff[MAX_DEPTH];
    int newHovBuffLen = 0;
    Layout_HitTest(manager.root, evt->mevent.x, evt->mevent.y, newHovBuff, MAX_DEPTH, &newHovBuffLen);
    
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

    bool isFocusEvent = evt->leftClick;
    Layout* target = NULL;
    int lastIdx = manager.hovBuffCurrSize - 1;
    if (lastIdx >= 0)
    {
        target = manager.hovBuffer[lastIdx];
    }

    if (isFocusEvent)
    {
        focusLayout(target);
    }

    for (int k = 0; k < exitingSize; k++)
    {
        if (exiting[k]->vtable.onPtrExit)
        {
            exiting[k]->isHover = false;
            exiting[k]->vtable.onPtrExit(exiting[k], evt);
        }
    }

    // next do all the still hovering guys
    for (int k = 0; k < stillHoveringSize; k++)
    {
        if (stillHovering[k]->vtable.onPtrMove)
        {
            stillHovering[k]->isHover = true;
            stillHovering[k]->vtable.onPtrMove(stillHovering[k], evt);
        }
    }

    for (int k = 0; k < enteringSize; k++)
    {
        if (entering[k]->vtable.onPtrEnter)
        {
            entering[k]->isHover = true;
            entering[k]->vtable.onPtrEnter(entering[k], evt);
        }
    }
    
    dispatchMouseEvt(evt, target);
}

void dispatchMouseEvt(InputEvent* evt, Layout* target)
{
    if (!evt || !evt->isMouse || !target) return;
    
    if (evt->rightClick ||
        evt->leftClick ||
        evt->midClick)
    {
        BblEvt be;
        BblEvt_Click cbe;

        cbe.target = target;
        cbe.x = evt->mevent.x;
        cbe.y = evt->mevent.y;
        cbe.leftClick = evt->leftClick;
        cbe.rightClick = evt->rightClick;
        cbe.midClick = evt->midClick;

        be.evtData = &cbe;
        be.handled = false;
        be.target = target;
        be.type = BblEvtType_Click;

        doBubble(target, &be);
    }

    if (evt->scrollDown ||
        evt->scrollUp)
    {
        BblEvt be;
        BblEvt_Scroll sbe;

        sbe.target = target;
        sbe.x = evt->mevent.x;
        sbe.y = evt->mevent.y;
        sbe.up = evt->scrollUp;
        sbe.down = evt->scrollDown;

        be.evtData = &sbe;
        be.handled = false;
        be.target = target;
        be.type = BblEvtType_Scroll;

        doBubble(target, &be);
    }
}

void focusLayout(Layout* l)
{
    if (l != manager.focused)
    {
        if (manager.focused)
        {
            manager.focused->isFocus = false;
            if (manager.focused->vtable.onUnFocus) manager.focused->vtable.onUnFocus(manager.focused);
        }

        manager.focused = l;

        if (manager.focused)
        {
            manager.focused->isFocus = true;
            if (manager.focused->vtable.onUnFocus) manager.focused->vtable.onUnFocus(manager.focused);
        }
        
    }
}

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


void doBubble(Layout * target, BblEvt * e)
{
    if (!target || !e) return;

    Layout *stack[MAX_DEPTH];
    int depth = 0;
    for (Layout *p = target; p; p = p->parent) stack[depth++] = p;

    // capture phase
    for (int i = depth - 1; i >= 0; --i) 
    {
        if (stack[i]->vtable.onBblEvtCapture)
        {
            stack[i]->vtable.onBblEvtCapture(stack[i], e);
            if (e->handled) return;
        }
    }

    // bubble phase
    for (int i = 0; i < depth; i++) 
    {
        if (stack[i]->vtable.onBblEvt)
        {
            stack[i]->vtable.onBblEvt(stack[i], e);
            if (e->handled) return;
        }
    }
}
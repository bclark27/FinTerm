#include "GUIManager.h"
#include "Common.h"
#include "Logger.h"
#include <ncursesw/ncurses.h>
#include <locale.h>
#include <string.h>
#include "Colors.h"

#define MAX_DEPTH   (2000)
#define MAX_EVT_Q   (2000)

typedef struct GUIManager
{
    Layout* root;
    Layout* focused;
    int hovBuffCurrSize;
    Layout* hovBuffer[MAX_DEPTH];
    int eventQueueSize;
    int currEventQueueIdx;
    BblEvt eventQueue[MAX_EVT_Q];
    bool init;
    bool forceRedraw;
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
void doBubble(BblEvt * e);
int treeSize(Layout* node, bool includeInvisible);
Layout* getNextTabNav(bool forward);
void freezeEventQueue(BblEvt* buffer, int* size);
void depthFirstSizeRef(Layout* l);
void root_draw(Layout*l , WINDOW *win, int x, int y, int width, int height);
void fill_window(WINDOW* win, chtype ch);

// layout vtable

static Layout_VT vtable = 
{
    .draw = root_draw,
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

    initscr();
    setlocale(LC_ALL, "en_US.UTF-8");
    start_color();
    use_default_colors();
    keypad(stdscr, TRUE);   // Enable function keys like KEY_RESIZE
    noecho();
    cbreak();
    nodelay(stdscr, TRUE);
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);  // Enable mouse events

    Colors_Init();

    manager.hovBuffCurrSize = 0;
    manager.init = true;
    manager.root = Layout_Create();
    manager.root->vtable = vtable;
    Layout_SetLayoutStrategy(manager.root, LayoutStrategy_horz);
    manager.currEventQueueIdx = 0;
    manager.eventQueueSize = 0;
    GUIManager_LayoutRefresh(true);
    
    for (int i = 0; i < 2; i++)
    {
        GUIManager_LayoutRefresh(true);
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

            be.data.key.raw = evts[i].raw;
            be.data.key.isAscii = evts[i].isAscii;
            be.data.key.isCtrl = evts[i].isCtrl;
            be.data.key.isSpecial = evts[i].isSpecial;
            be.data.key.keyCode = evts[i].keyCode;

            be.handled = false;
            be.target = target;
            be.type = BblEvtType_Key;

            GUIManager_QueueEvent(be);
        }
    }
}

void GUIManager_QueueEvent(BblEvt evt)
{
    int nextIdx = manager.currEventQueueIdx + 1;
    if (nextIdx >= MAX_EVT_Q) nextIdx = 0;

    manager.eventQueue[nextIdx] = evt;
    manager.currEventQueueIdx = nextIdx;

    manager.eventQueueSize = MIN(manager.eventQueueSize + 1, INPUT_EVENT_BUFFER_SIZE);
}

void GUIManager_HandleEventQueue()
{
    BblEvt frozenEvts[MAX_EVT_Q];
    int frozenSize;

    while (manager.eventQueueSize)
    {
        freezeEventQueue(frozenEvts, &frozenSize);
        manager.eventQueueSize = 0;
        manager.currEventQueueIdx = 0;
        for (int i = 0; i < frozenSize; i++)
        {
            doBubble(&(frozenEvts[i]));
        }
    }
}

void GUIManager_LayoutRefresh(bool force)
{
    if (!manager.init) return;

    int r,c;
    getCurrTermSize(&r, &c);

    if (manager.root->width != c || manager.root->height != r || force)
    {
        Layout_SizeRefresh(manager.root, 0, 0, c, r, force);
        manager.forceRedraw = true;
    }
    else
    {
        depthFirstSizeRef(manager.root);
    }
}

void GUIManager_Draw(bool force)
{
    Layout_Draw(manager.root, force || manager.forceRedraw);
    manager.forceRedraw = false;
    update_panels();
    doupdate();
}

// PRIV

void rootEvtCapture(Layout* l, BblEvt* e)
{
    if (!l || !e) return;

    if (e->type == BblEvtType_Key)
    {
        BblEvt_Key* ke = (BblEvt_Key*)&(e->data.key);


        bool doTabNav = ke->raw == 0x9 || ke->raw == KEY_BTAB;

        if (e->target && 
            e->target->acceptsLiteralTab &&
            ke->raw == 0x9)
        {
            doTabNav = false;
        }

        if (doTabNav)
        {
            e->handled = true;
            bool shiftForward = ke->raw == 0x9;
            Layout* nextTab = getNextTabNav(shiftForward);
            focusLayout(nextTab);

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
    for (int i = manager.hovBuffCurrSize - 1; i >= 0; i--)
    {
        if (manager.hovBuffer[i]->focusable)
        {
            target = manager.hovBuffer[i];
            break;
        }
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

        be.data.click.target = target;
        be.data.click.x = evt->mevent.x;
        be.data.click.y = evt->mevent.y;
        be.data.click.leftClick = evt->leftClick;
        be.data.click.rightClick = evt->rightClick;
        be.data.click.midClick = evt->midClick;

        be.handled = false;
        be.target = target;
        be.type = BblEvtType_Click;

        GUIManager_QueueEvent(be);
    }

    if (evt->scrollDown ||
        evt->scrollUp)
    {
        BblEvt be;

        be.data.scroll.target = target;
        be.data.scroll.x = evt->mevent.x;
        be.data.scroll.y = evt->mevent.y;
        be.data.scroll.up = evt->scrollUp;
        be.data.scroll.down = evt->scrollDown;

        be.handled = false;
        be.target = target;
        be.type = BblEvtType_Scroll;

        GUIManager_QueueEvent(be);
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
            
            BblEvt be;

            be.data.focus.focus = false;
            be.data.focus.target = manager.focused;

            be.handled = false;
            be.target = manager.focused;
            be.type = BblEvtType_Focus;

            GUIManager_QueueEvent(be);
        }

        manager.focused = l;

        if (manager.focused)
        {
            manager.focused->isFocus = true;
            if (manager.focused->vtable.onFocus) manager.focused->vtable.onFocus(manager.focused);

            BblEvt be;

            be.data.focus.focus = true;
            be.data.focus.target = manager.focused;

            be.handled = false;
            be.target = manager.focused;
            be.type = BblEvtType_Focus;

            GUIManager_QueueEvent(be);
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


void doBubble(BblEvt * e)
{
    if (!e || !e->target) return;

    Layout *stack[MAX_DEPTH];
    int depth = 0;
    for (Layout *p = e->target; p; p = p->parent) stack[depth++] = p;

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

int treeSize(Layout* node, bool includeInvisible)
{
    if (!node || (!node->visible && !includeInvisible)) return 0;

    int count = 1;

    for (int i = 0; i < node->childrenCount; i++)
    {
        count += treeSize(node->children[i], includeInvisible);
    }

    return count;
}

Layout* find_prev_focusable_internal(Layout *node, Layout *current, bool *foundCurrent) 
{
    if (!node || !node->visible || !node->focusable) return NULL;

    // 1) Dive into children in reverse order
    for (int i = node->childrenCount - 1; i >= 0; --i) 
    {
        Layout *res = find_prev_focusable_internal(
        node->children[i], current, foundCurrent);
        if (res) return res;
    }
    // 2) When you see current, mark it
    if (node == current) 
    {
        *foundCurrent = true;
    }
    // 3) If you’ve seen current already, this is the previous candidate
    else if (*foundCurrent) 
    {
        return node;
    }
        return NULL;
}

Layout* find_prev_focusable(Layout *root, Layout *current) 
{
    bool found = false;
    Layout *prev = find_prev_focusable_internal(root, current, &found);
    if (prev) return prev;

    // wrap: scan from the end
    bool dummy = false;
    return find_prev_focusable_internal(root, NULL, &dummy);
}

Layout* find_next_focusable_internal(Layout *node, Layout *current, bool *foundCurrent) 
{
    if (!node || !node->visible || !node->focusable) return NULL;

    // 1) If this is the current, mark that we’ve seen it.
    if (node == current) {
        *foundCurrent = true;
    }
    // 2) If we already saw current, then this is the next candidate.
    else if (*foundCurrent) 
    {
        return node;
    }

    // 3) Recurse into children in natural order
    for (int i = 0; i < node->childrenCount; ++i) 
    {
        Layout *res = find_next_focusable_internal(
        node->children[i], current, foundCurrent);
        if (res) return res;
    }
    return NULL;
}

Layout* find_next_focusable(Layout *root, Layout *current) {
    bool found = false;
    Layout *next = find_next_focusable_internal(root, current, &found);
    if (next) return next;

    // wrap: start again from the very first node
    bool dummy = false;
    return find_next_focusable_internal(root, NULL, &dummy);
}

Layout* getNextTabNav(bool forward)
{
    if (!manager.focused) return manager.root;

    return forward ? 
        find_next_focusable(manager.root, manager.focused) :
        find_prev_focusable(manager.root, manager.focused);
}

void freezeEventQueue(BblEvt* buffer, int* size)
{
    int qidx = manager.currEventQueueIdx;
    for (int i = 0; i < manager.eventQueueSize; i++)
    {
        buffer[i] = manager.eventQueue[qidx];
        qidx--;
        if (qidx < 0) qidx = MAX_EVT_Q - 1;
    }
    *size = manager.eventQueueSize;
}

void depthFirstSizeRef(Layout* l)
{
    if (!l) return;
    
    if (l->resize)
    {
        if (l->parent)
        {
            Layout_SizeRefreshSameParams(l->parent, false);
        }
        else
        {
            int r,c;
            getCurrTermSize(&r, &c);
            Layout_SizeRefresh(manager.root, 0, 0, c, r, false);
        }
        return;
    }
    else
    {
        for (int i = 0; i < l->childrenCount; i++)
        {
            depthFirstSizeRef(l->children[i]);
        }
    }
}

void root_draw(Layout*l , WINDOW *win, int x, int y, int width, int height)
{
    fill_window(win, 'A');
}

void fill_window(WINDOW* win, chtype ch)
{
    if (!win) return;

    int height, width;
    getmaxyx(win, height, width);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            mvwaddch(win, y, x, ch);
        }
    }
}
#include "Entry.h"
#include "../Common.h"

// declerations

void entry_draw(Layout*, WINDOW *win, int x, int y, int width, int height);
void entry_onDestroy(Layout* l);
void entry_onFocus(Layout* l);
void entry_onUnFocus(Layout* l);
void entry_onBblEvt(Layout* l, BblEvt* e);

void setTabInput(Entry* entry, bool en);

// vtable

static Layout_VT vtable = 
{
    .draw = entry_draw,
    .onDestroy = entry_onDestroy,
    .onPtrEnter = NULL,
    .onPtrExit = NULL,
    .onPtrMove = NULL,
    .onFocus = entry_onFocus,
    .onUnFocus = entry_onUnFocus,
    .onBblEvt = entry_onBblEvt,
    .onBblEvtCapture = NULL,
};

// public
Entry * Entry_Create()
{
    Entry * entry = calloc(1, sizeof(Entry));
    Layout_Init((Layout*)entry);
    ((Layout*)entry)->vtable = vtable;
    
    // set the default padding
    entry->layout.pad_down = 1;
    entry->layout.pad_up = 1;
    entry->layout.pad_left = 1;
    entry->layout.pad_right = 1;

    // create the label inside
    entry->labelChild = Label_Create();
    ((Layout*)(entry->labelChild))->tabNavSkip = true;
    Layout_AddChild((Layout*)entry, (Layout*)entry->labelChild);

    // set default text
    Label_SetTextCpy(entry->labelChild, "entry here");

    return entry;
}

void entry_draw(Layout*, WINDOW *win, int x, int y, int width, int height)
{
    box(win, 0, 0);
}

void entry_onDestroy(Layout* l)
{

}

void entry_onFocus(Layout* l)
{
    
}

void entry_onUnFocus(Layout* l)
{
    setTabInput((Entry*)l, false);
}

void entry_onBblEvt(Layout* l, BblEvt* e)
{
    Entry* this = (Entry*)l;

    bool enableTabInput = false;
    enableTabInput |= e->type == BblEvtType_Click && e->data.click.leftClick;
    enableTabInput |= e->type == BblEvtType_Key && (e->data.key.isAscii || e->data.key.isSpecial);
    if (enableTabInput) setTabInput(this, true);

    bool actuallyDisableTabs = false;
    actuallyDisableTabs |= e->type == BblEvtType_Key && ((e->data.key.isSpecial && e->data.key.raw == 27) || (e->data.key.isCtrl && e->data.key.raw == 10));
    if (actuallyDisableTabs) setTabInput(this, false);
}

void setTabInput(Entry* entry, bool en)
{
    ((Layout*)(entry))->acceptsLiteralTab = en;
    ((Layout*)(entry->labelChild))->acceptsLiteralTab = en;
}
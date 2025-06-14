#include "Entry.h"
#include "../Common.h"

// declerations

void entry_draw(Layout*, WINDOW *win, int x, int y, int width, int height);
void entry_onDestroy(Layout* l);
void entry_onFocus(Layout* l);
void entry_onUnFocus(Layout* l);
void entry_onBblEvt(Layout* l, BblEvt* e);
void entry_onPtrEnter(Layout* l, InputEvent* e);
void entry_onPtrExit(Layout* l, InputEvent* e);
void entry_onPtrMove(Layout* l, InputEvent* e);

void entry_setTabInput(Entry* entry, bool en);
void entry_consumeTextChar(Entry* entry, BblEvt_Key* key);
char* entry_modifyString(char* original, char ch, bool backspace);

// vtable

static Layout_VT vtable = 
{
    .draw = entry_draw,
    .onDestroy = entry_onDestroy,
    .onPtrEnter = entry_onPtrEnter,
    .onPtrExit = entry_onPtrExit,
    .onPtrMove = entry_onPtrMove,
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
    ((Layout*)(entry->labelChild))->focusable = false;
    entry->labelChild->textWrap = true;
    Layout_AddChild((Layout*)entry, (Layout*)entry->labelChild);

    // set default text
    Label_SetTextCpy(entry->labelChild, "entry here");

    return entry;
}

void entry_draw(Layout* l, WINDOW *win, int x, int y, int width, int height)
{
    int color = -1;

    if (l->isFocus)
    {
        color = DEFATTR_SELECTED_BOX();
        Logger_Log("ASDASD: %d\n", color);
    }
    
    if (color >= 0) wattron(win, COLOR_PAIR(color));
    box(win, 0, 0);
    if (color >= 0) wattroff(win, COLOR_PAIR(color));
}

void entry_onDestroy(Layout* l)
{

}

void entry_onFocus(Layout* l)
{
    l->redraw = true;
}

void entry_onUnFocus(Layout* l)
{
    l->redraw = true;
    entry_setTabInput((Entry*)l, false);
}

void entry_onBblEvt(Layout* l, BblEvt* e)
{
    Entry* this = (Entry*)l;

    if (e->type == BblEvtType_Click || e->type == BblEvtType_Key)
    {

        bool enableTabInput = false;
        enableTabInput |= e->type == BblEvtType_Click && e->data.click.leftClick;
        enableTabInput |= e->type == BblEvtType_Key && (e->data.key.isAscii || e->data.key.isSpecial);
        if (enableTabInput) entry_setTabInput(this, true);
    
        bool actuallyDisableTabs = false;
        actuallyDisableTabs |= e->type == BblEvtType_Key && ((e->data.key.isSpecial && e->data.key.raw == 27)); //(e->data.key.isCtrl && e->data.key.raw == 10) add this for entry escape
        if (actuallyDisableTabs) entry_setTabInput(this, false);

        if (e->type == BblEvtType_Key && (e->data.key.isAscii || e->data.key.raw == KEY_BACKSPACE || e->data.key.raw == 0x9))// )
        {
            entry_consumeTextChar(this, &(e->data.key));
        }
    }
    else if (e->type == BblEvtType_Focus)
    {
        if (e->data.focus.focus && e->data.focus.target == (Layout*)(this->labelChild))
        {
            entry_setTabInput(this, true);
        }
    }
}

void entry_onPtrEnter(Layout* l, InputEvent* e)
{

}

void entry_onPtrExit(Layout* l, InputEvent* e)
{

}

void entry_onPtrMove(Layout* l, InputEvent* e)
{

}

void entry_setTabInput(Entry* entry, bool en)
{
    ((Layout*)(entry))->acceptsLiteralTab = en;
    ((Layout*)(entry->labelChild))->acceptsLiteralTab = en;
}

void entry_consumeTextChar(Entry* entry, BblEvt_Key* key)
{
    bool handled = false;
    if (key->isAscii)
    {
        char* newStr = entry_modifyString(entry->labelChild->str, key->keyCode, false);
        Label_SetTextCpy(entry->labelChild, newStr);
        handled = true;
    }
    else if (key->raw == KEY_BACKSPACE)
    {
        char* newStr = entry_modifyString(entry->labelChild->str, 0, true);
        Label_SetTextCpy(entry->labelChild, newStr);
        handled = true;
    }
    else if (key->raw == 0x9)
    {
        char* newStr = entry_modifyString(entry->labelChild->str, '\t', false);
        Label_SetTextCpy(entry->labelChild, newStr);
        handled = true;
    }
}

char* entry_modifyString(char* original, char ch, bool backspace)
{
    size_t len = original ? strlen(original) : 0;

    if (backspace) {
        if (len == 0) {
            // Already empty, return a new empty string
            char* new_str = malloc(1);
            if (new_str) new_str[0] = '\0';
            return new_str;
        }
        char* new_str = malloc(len); // One less than original
        if (!new_str) return NULL;
        memcpy(new_str, original, len - 1);
        new_str[len - 1] = '\0';
        return new_str;
    } else {
        // Append case
        char* new_str = malloc(len + 2); // +1 for new char, +1 for null terminator
        if (!new_str) return NULL;
        if (original) memcpy(new_str, original, len);
        new_str[len] = ch;
        new_str[len + 1] = '\0';
        return new_str;
    }
}
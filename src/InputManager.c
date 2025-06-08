#include "InputManager.h"

#include "Common.h"



// types

typedef struct InputManager
{
    InputEvent buffer[INPUT_EVENT_BUFFER_SIZE];
    int currBuffIdx;
    int currUpdateSize;

} InputManager;

static bool manager_init = false;
static InputManager manager;

// declerations

void parseCh(int ch, InputEvent* info);

// public

void InputManager_Init()
{
    memset(&manager, 0, sizeof(InputManager));
    manager_init = true;
    manager.currBuffIdx = INPUT_EVENT_BUFFER_SIZE - 1;
}

void InputManager_Update()
{
    if (!manager_init) return;

    int ch;
    int count = 0;
    while ((ch = getch()) != 0)
    {
        if (ch == ERR) break;

        // get the next circular buffer idx
        manager.currBuffIdx++;
        if (manager.currBuffIdx >= INPUT_EVENT_BUFFER_SIZE) manager.currBuffIdx = 0;
        parseCh(ch, &manager.buffer[manager.currBuffIdx]);

        count++;
    }

    manager.currUpdateSize = MIN(count, INPUT_EVENT_BUFFER_SIZE);
}

void InputManager_GetKeyEvents(InputEvent* events, int* count)
{
    if (!manager_init)
    {
        *count = 0;
        return;
    }

    int idx = manager.currBuffIdx - manager.currUpdateSize + 1;
    if (idx < 0) idx += INPUT_EVENT_BUFFER_SIZE;

    for (int i = 0; i < manager.currUpdateSize; i++)
    {
        memcpy(&events[i], &manager.buffer[idx++], sizeof(InputEvent));
        if (idx >= INPUT_EVENT_BUFFER_SIZE) idx = 0;
    }

    *count = manager.currUpdateSize;
}

// priv

void parseCh(int ch, InputEvent* info)
{
    memset(info, 0, sizeof(InputEvent));

    info->timestamp = current_time();
    info->raw = ch;

    if (ch == KEY_MOUSE)
    {
        info->isMouse = getmouse(&info->mevent) == OK;
    } 
    else if (ch >= 1 && ch <= 26) 
    {
        // Ctrl + A = 1, Ctrl + B = 2, ..., Ctrl + Z = 26
        info->keyCode = ch + 'A' - 1;
        info->isCtrl = true;
    } 
    else if (ch >= 32 && ch < 127) 
    {
        // Printable ASCII characters
        info->isAscii = true;
        info->keyCode = ch;
    } 
    else 
    {
        // Special keys (arrows, function keys, etc.)
        switch (ch) {
            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_BACKSPACE:
            case KEY_DC: // delete
            case KEY_HOME:
            case KEY_END:
            case KEY_NPAGE:
            case KEY_PPAGE:
            case KEY_ENTER:
                // Handle special keys
                info->isSpecial = true;
                break;
            default:
                // Possibly unknown
                break;
        }
    }
    
}

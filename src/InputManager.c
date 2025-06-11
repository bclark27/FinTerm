#include "InputManager.h"

#include "Logger.h"
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

bool parseCh(int ch, InputEvent* info);
void enable_mouse_tracking();
void disable_mouse_tracking();

// public

void InputManager_Init()
{
    memset(&manager, 0, sizeof(InputManager));
    manager_init = true;
    manager.currBuffIdx = INPUT_EVENT_BUFFER_SIZE - 1;
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    enable_mouse_tracking();
}

void InputManager_Destroy()
{
    disable_mouse_tracking();
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
        int nextIdx = manager.currBuffIdx + 1;
        if (nextIdx >= INPUT_EVENT_BUFFER_SIZE) nextIdx = 0;

        if (parseCh(ch, &manager.buffer[nextIdx]))
        {
            manager.currBuffIdx = nextIdx;
            count++;
        }
        
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

bool parseCh(int ch, InputEvent* info)
{
    memset(info, 0, sizeof(InputEvent));

    info->timestamp = current_time();
    info->raw = ch;

    if (ch == KEY_MOUSE)
    {
        info->isMouse = getmouse(&info->mevent) == OK;
        info->leftClick = info->mevent.bstate & BUTTON1_RELEASED;
        info->midClick = info->mevent.bstate & BUTTON2_RELEASED;
        info->rightClick = info->mevent.bstate & BUTTON3_RELEASED;
        info->scrollUp = info->mevent.bstate & BUTTON4_PRESSED;
        info->scrollDown = info->mevent.bstate & BUTTON5_PRESSED;

        return info->isMouse && (
            (info->mevent.bstate & REPORT_MOUSE_POSITION) ||
            info->leftClick ||
            info->midClick ||
            info->rightClick ||
            info->scrollUp ||
            info->scrollDown
        );
    } 
    else if (ch >= 1 && ch <= 26) 
    {
        // Ctrl + A = 1, Ctrl + B = 2, ..., Ctrl + Z = 26
        info->keyCode = ch + 'A' - 1;
        info->isCtrl = true;
        return true;
    } 
    else if (ch >= 32 && ch < 127) 
    {
        // Printable ASCII characters
        info->isAscii = true;
        info->keyCode = ch;
        return true;
    } 
    else 
    {
        // Special keys (arrows, function keys, etc.)
        switch (ch) {
            case KEY_UP:
            case 27: // escape
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
            case KEY_BTAB:
                // Handle special keys
                info->isSpecial = true;
                return true;
                break;
            default:
                // Possibly unknown
                Logger_Log("Unknown Key: %d\n", ch);
                return false;
                break;
        }
    }
    
}


void enable_mouse_tracking()
{
    printf("\033[?1003h\n");
    fflush(stdout);
}

void disable_mouse_tracking() 
{
    printf("\033[?1003l\n"); // Disable mouse movement reporting
    fflush(stdout);
}
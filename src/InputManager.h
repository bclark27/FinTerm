#ifndef INPUT_MANAGER_H_
#define INPUT_MANAGER_H_

#include <stdbool.h>
#include <ncursesw/ncurses.h>

// defines
#define INPUT_EVENT_BUFFER_SIZE   (1000)

typedef struct InputEvent
{
    long timestamp;

    MEVENT mevent;
    bool isMouse;
    bool rightClick;
    bool leftClick;
    bool midClick;
    bool scrollUp;
    bool scrollDown;
    
    int raw;
    char keyCode;
    bool isAscii;
    bool isCtrl;
    bool isSpecial;

} InputEvent;

void InputManager_Init();
void InputManager_Destroy();
void InputManager_Update();
void InputManager_GetKeyEvents(InputEvent* events, int* count);
//void KeysManager_GetKeyStrokes(KeyInfo ** )

#endif
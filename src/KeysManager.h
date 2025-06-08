#ifndef KEYS_MANAGER_H_
#define KEYS_MANAGER_H_

#include <stdbool.h>

typedef struct KeyInfo
{
    char key;
} KeyInfo;

void KeysManager_Init();
void KeysManager_Update();
bool KeysManager_KeyIsDown(char key);
//void KeysManager_GetKeyStrokes(KeyInfo ** )

#endif
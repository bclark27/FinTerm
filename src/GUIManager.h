#ifndef __GUI_MANAGER
#define __GUI_MANAGER

#include "InputManager.h"
#include "Layout/Layout.h"


void GUIManager_Init();
void GUIManager_Destroy();
Layout* GUIManager_GetRoot();
Layout* GUIManager_GetFocused();
void GUIManager_OnKeys(InputEvent* evts, int count);

void GUIManager_SizeRefresh();
void GUIManager_Draw(bool force);

#endif
#ifndef __GUI_MANAGER
#define __GUI_MANAGER

#include "Layout.h"


void GUIManager_Init();
void GUIManager_Destroy();
Layout* GUIManager_GetRoot();

void GUIManager_SizeRefresh();
void GUIManager_Draw(bool force);

#endif
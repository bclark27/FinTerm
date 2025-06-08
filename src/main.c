#include <ncursesw/ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <panel.h>
#include "Logger.h"
#include "Common.h"
#include "InputManager.h"
#include "GUIManager.h"

#define REF_TIME  (100)

void onEvt(Layout * this, LayoutBubbleEvent* evt)
{
  if (evt->type == LayoutBubbleEventType_Clicked)
  {
    LayoutBubbleEvent_Clicked* evt_c = (LayoutBubbleEvent_Clicked*)evt->evt;
    if (this == evt_c->clickedLayout)
    {
      this->isDirty = true;
    }
  }
}

void draw(Layout * l, WINDOW * win, int x, int y, int width, int height)
{
  box(win, 0, 0);
  mvwprintw(win, 1, 1, "%d, %d", width, height);
  mvwprintw(win, 1, 2, "%d", l->childrenCount);
  if (l == GUIManager_GetFocused()) mvwprintw(win, height / 2, width / 2, "X");
}

void addChildren(Layout * l, LayoutOrientation o, int count)
{
  l->orientation = o;
  
  for (int i = 0; i < count; i++)
  {
    Layout* c = Layout_Create();
    c->orientation = 0;
    c->draw = draw;
    c->onBubble = onEvt;
    Layout_AddChild(l, c);
  }
}

void buildTestGUI(Layout* root)
{
  addChildren(root, LayoutOrientation_V, 3);
  
  Layout* c1 = root->children[1];
  addChildren(c1, LayoutOrientation_H, 3);
  
  Layout* c2 = c1->children[2];
  addChildren(c2, LayoutOrientation_H, 3);
  
  Layout* temp = c2->children[2];
  
  Layout_DetatchFromParent(temp);
  Layout_Destroy(temp);
}

int main()
{
  long lastTime = 0;
  int events_count;
  InputEvent events[INPUT_EVENT_BUFFER_SIZE];

  Logger_Open();
  GUIManager_Init();
  InputManager_Init();
  

  Layout * root = GUIManager_GetRoot();
  buildTestGUI(root);
  
  while (1)
  {
    long now = current_time();
    if (now - lastTime >= REF_TIME)
    {
      lastTime = now;
            
      GUIManager_SizeRefresh();
      
      InputManager_Update();
      InputManager_GetKeyEvents(events, &events_count);
      GUIManager_OnKeys(events, events_count);
      GUIManager_Draw(true);
      refresh();
    }
  }
  
  GUIManager_Destroy();

  endwin();               // End ncurses mode
  
  return 0;
}

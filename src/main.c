#include <ncursesw/ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <panel.h>
#include "Logger.h"
#include "Common.h"
#include "InputManager.h"
#include "Layout/Label.h"
#include "Layout/Entry.h"
#include "GUIManager.h"

#define REF_TIME  (50)


void handle_sigint(int sig) {
  InputManager_Destroy();
  endwin();
  exit(0);
}
static Layout* dissapear;
void click(Layout* l)
{
  Layout_SetVis(dissapear, false);
  Logger_Log("clicked\n");
}

Layout* buildTriple(LayoutOrientation o)
{
  Layout* r = Layout_Create();
  r->orientation = o;

  for (int i = 0; i < 3; i++)
  {
    Label * l = Label_Create();
    // Label_SetTextFmt(l, "hello there\nA\nB\nfriend\n%d\n", 400);
    Label_SetTextFmt(l, "hello\n1");
    l->textOption = LabelTextOptions_Center;
    l->horzAlign = Alignment_Start + i;
    l->vertAlign = Alignment_Start;// + i;
    Layout_AddChild(r, (Layout*)l);

    if (i == 0) dissapear = (Layout*)l;
  }

  return r;
}

void buildTestGUI(Layout* root)
{
  for (int i = 0; i < 1; i++)
  {
    Layout_AddChild(root, buildTriple(LayoutOrientation_H));
  }
  
  Layout_AddChild(root, (Layout*)Entry_Create());
  Layout_AddChild(root, (Layout*)Entry_Create());

  for (int i = 0; i < 2; i++)
  {
    Label * l = Label_Create();
    Label_SetTextCpy(l, "hello there\nA\nB\nfriend");
    l->textOption = LabelTextOptions_Center;
    l->horzAlign = Alignment_Start + i;
    l->vertAlign = Alignment_Start + i;
    //l->layout.vtable.onFocus = click;
    Layout_AddChild(root, (Layout*)l);
  }

  root->orientation = LayoutOrientation_V;
}


int main()
{
  signal(SIGINT, handle_sigint);  // Register Ctrl+C handler

  long lastTime = 0;
  int events_count;
  InputEvent events[INPUT_EVENT_BUFFER_SIZE];

  Logger_Open();
  GUIManager_Init();
  
  InputManager_Init();
  

  Layout * root = GUIManager_GetRoot();
  buildTestGUI(root);
  GUIManager_LayoutRefresh(true);

  while (1)
  {
    long now = current_time();
    if (now - lastTime >= REF_TIME)
    {
      lastTime = now;
      
      long s,e;
      s = current_time();
      
      GUIManager_LayoutRefresh(false);
      InputManager_Update();
      InputManager_GetKeyEvents(events, &events_count);
      GUIManager_OnKeys(events, events_count);
      GUIManager_HandleEventQueue();
      GUIManager_Draw(false);
      
      e = current_time();
      //Logger_Log("Render Time: %ldms\n", e - s);

      s = current_time();
      refresh();
      e = current_time();
      //Logger_Log("Refresh Time: %ldms\n", e - s);
    }
  }

  InputManager_Destroy();
  GUIManager_Destroy();

  endwin();               // End ncurses mode
  
  return 0;
}

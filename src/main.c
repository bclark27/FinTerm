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
#include "Layout/StaticList.h"
#include "GUIManager.h"

#define REF_TIME  (1000.0 / 30.0)


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

Layout* buildTriple(LayoutStrategy o)
{
  Layout* r = Layout_Create();
  r->layoutStrategy = o;

  for (int i = 0; i < 3; i++)
  {
    Label * l = Label_Create();
    // Label_SetTextFmt(l, "hello there\nA\nB\nfriend\n%d\n", 400);
    Label_SetTextFmt(l, "hello\n1");
    l->textOption = LabelTextOptions_Center;
    l->horzAlign = Alignment_Start + i;
    l->vertAlign = Alignment_Start;
    l->border = true;
    Layout_AddChild(r, (Layout*)l);

    if (i == 0) dissapear = (Layout*)l;
  }

  return r;
}

void addFloatingWindows(Layout* root)
{
  Entry* e1 = Entry_Create();
  Layout_SetZIndex((Layout*)e1, 10);
  Layout_SetDims((Layout*)e1, 20, 20, 80, 20);
  
  Entry* e2 = Entry_Create();
  Layout_SetZIndex((Layout*)e2, 1);
  Layout_SetDims((Layout*)e2, 5, 5, 80, 20);
 
  StaticList* sl = StaticList_Create();
  Layout_SetZIndex((Layout*)sl, 100);
  Layout_SetDims((Layout*)sl, 25, 25, 20, 30);
  StaticList_SetBoarder(sl, true);
  sl->listenScrollEvt = false;
  sl->listenHoverEvt = false;
  sl->listenSelectEvt = false;
  char* sampleTexts[] = {
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
    "Hello",
    "World",
    "Ncurses",
    "Panel",
    "Layout",
    "Focus",
    "Click",
    "Draw",
    "Buffer",
    "Update",
  };
  StaticList_SetStrListCpy(sl, sampleTexts, 60);


  Layout_AddChild(root, (Layout*)e2);
  Layout_AddChild(root, (Layout*)e1);
  Layout_AddChild(root, (Layout*)sl);
}

void addSizedWindows(Layout* root)
{
  Layout_AddChild(root, buildTriple(LayoutStrategy_horz));
  Layout_AddChild(root, buildTriple(LayoutStrategy_vert));
  Layout_AddChild(root, buildTriple(LayoutStrategy_horz));
  Layout_AddChild(root, buildTriple(LayoutStrategy_vert));
  Layout_AddChild(root, buildTriple(LayoutStrategy_horz));
  Layout_AddChild(root, buildTriple(LayoutStrategy_vert));

  return;
  for (int i = 0; i < 1; i++)
  {
    Layout_AddChild(root, buildTriple(LayoutStrategy_horz));
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
  

  Layout * froot = GUIManager_GetRoot();
  Layout * aroot = GUIManager_GetSizingRoot();
  addSizedWindows(aroot);
  addFloatingWindows(froot);
  GUIManager_LayoutRefresh(true);

  while (1)
  {
    long now = current_time();
    if (now - lastTime >= REF_TIME)
    {
      lastTime = now;
      GUIManager_LayoutRefresh(false);
      long start = current_time();
      InputManager_Update();
      long end = current_time();
      InputManager_GetKeyEvents(events, &events_count);
      GUIManager_OnKeys(events, events_count);
      GUIManager_HandleEventQueue();
      GUIManager_Draw(false);
      
      //refresh();
      //Logger_Log("Refresh Time: %ldms\n", end - start);
    }
    else
    {
      usleep((REF_TIME / 2) * 1000);
    }
  }

  InputManager_Destroy();
  GUIManager_Destroy();

  endwin();               // End ncurses mode
  
  return 0;
}

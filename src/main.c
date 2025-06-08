#include <ncursesw/ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <panel.h>
#include "Logger.h"

#include "GUIManager.h"

#define REF_TIME  (100)


void initTerm()
{
  setlocale(LC_ALL, "en_US.UTF-8");
  initscr();
  start_color();
  use_default_colors();
  curs_set(FALSE);
  keypad(stdscr, TRUE);   // Enable function keys like KEY_RESIZE
  noecho();
  cbreak();
  nodelay(stdscr, TRUE);
  mousemask(ALL_MOUSE_EVENTS, NULL);  // Enable mouse events
}

long currMs()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000L) + (tv.tv_usec / 1000L);
}

void clicked(Layout * this, LayoutClickedEvent* evt)
{
  if (this == evt->clickedLayout)
  {
    this->isDirty = true;
    this->visible = false;
  }
}

void draw(Layout * l, WINDOW * win, int x, int y, int width, int height)
{
  box(win, 0, 0);
  mvwprintw(win, 1, 1, "%d, %d", width, height);
  mvwprintw(win, 1, 2, "%d", l->childrenCount);
  if (!l->visible) mvwprintw(win, height / 2, width / 2, "X");
}

void addChildren(Layout * l, LayoutOrientation o, int count)
{
  l->orientation = o;
  
  for (int i = 0; i < count; i++)
  {
    Layout* c = Layout_Create();
    c->orientation = 0;
    c->draw = draw;
    c->onClick = clicked;
    Layout_AddChild(l, c);
  }
}

int main()
{
  Logger_Open();
  
  
  
  initTerm();
  long lastTime = 0;
  MEVENT mevent;
  GUIManager_Init();
  Layout * root = GUIManager_GetRoot();
  addChildren(root, LayoutOrientation_V, 3);
  
  Layout* c1 = root->children[1];
  addChildren(c1, LayoutOrientation_H, 3);
  
  Layout* c2 = c1->children[2];
  addChildren(c2, LayoutOrientation_H, 3);
  
  Layout* temp = c2->children[1];
  
  Layout_DetatchFromParent(temp);
  Layout_Destroy(temp);

  while (1)
  {
    long now = currMs();
    if (now - lastTime >= REF_TIME)
    {
      lastTime = now;
      
      // getTermSize(&r, &c);
      // if (lr != r || lc != c)
      // {
        //   root->width = c;
        //   root->height = r;
        //   Layout_SizeRefresh(root);
        // }
        // lr=r;
        // lc=c;
        
        // ch = getch();
        // if (ch == KEY_MOUSE)
        // {
          //   if (getmouse(&mevent) == OK)
          //   {
            //     __Layout_ProcessClick(root, &mevent);
            //   }
            // }
            
      GUIManager_SizeRefresh();
      GUIManager_Draw(true);
      refresh();
    }
  }
  
  endwin();               // End ncurses mode
  
  return 0;
}

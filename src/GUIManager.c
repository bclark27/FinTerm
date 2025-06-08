#include "GUIManager.h"
#include "Logger.h"


typedef struct GUIManager
{
    Layout* root;
    bool init;
} GUIManager;

static GUIManager manager;

// declerations
void getCurrTermSize(int* rows, int* cols);

// PUBLIC

void GUIManager_Init()
{
    if (manager.init) return;

    manager.init = true;
    manager.root = Layout_Create();
    GUIManager_SizeRefresh();
}

void GUIManager_Destroy()
{
    if (!manager.init) return;
    Layout_Destroy(manager.root);
    manager.root = NULL;
    manager.init = false;
}

Layout* GUIManager_GetRoot()
{
    if (!manager.init) return NULL;
    return manager.root;
}

void GUIManager_SizeRefresh()
{
    if (!manager.init) return;

    int r,c;
    getCurrTermSize(&r, &c);

    if (manager.root->width != c || manager.root->height != r)
    {
        manager.root->width = c;
        manager.root->height = r;
        Layout_SizeRefresh(manager.root);
    }
}

void GUIManager_Draw(bool force)
{
    Layout_Draw(manager.root, force);
    doupdate();
}

// PRIV

void getCurrTermSize(int* rows, int* cols)
{
  int r, c;
  getmaxyx(stdscr, r, c);
  *rows = r;
  *cols = c;
}
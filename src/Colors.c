#include "Colors.h"

#include <ncursesw/ncurses.h>

#define ATTR_START      (1)
#define COLOR_COUNT     (8 + 1)

typedef struct ColorPair
{
    int attrId;
    char fg;
    char bg;
} ColorPair;

typedef struct ColorsManager
{
    int currFG;
    int currBG;
    ColorPair pairs[COLOR_COUNT][COLOR_COUNT];
} ColorsManager;

static ColorsManager manager;



// public

void Colors_Init()
{
    int count = 0;
    for (int i = 0; i < COLOR_COUNT; i++)
    {
        for (int k = 0; k < COLOR_COUNT; k++)
        {
            ColorPair* p = &manager.pairs[i][k];
            p->attrId = ATTR_START + count;
            count++;
            p->fg = (COLOR_BLACK + i) - 1;
            p->bg = (COLOR_BLACK + k) - 1;

            init_pair(p->attrId,
                p->fg,
                p->bg);
        }   
    }
}

int Colors_GetAttr(int fg, int bg)
{
    if (fg < -1 || fg > COLOR_WHITE || bg < -1 || bg > COLOR_WHITE) return -1;
    fg++;
    bg++;

    return manager.pairs[fg][bg].attrId;
}
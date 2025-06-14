#ifndef COLORS_H_
#define COLORS_H_

#include "Common.h"

#define DEFCOLOR_SELECTED        (11)
#define DEFATTR_SELECTED_BOX()   Colors_GetAttr(DEFCOLOR_SELECTED, -1)

void Colors_Init();
int Colors_GetAttr(int fg, int bg);

#endif
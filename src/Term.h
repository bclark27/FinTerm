#ifndef TERM_H_
#define TERM_H_

#include <ncursesw/ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <locale.h>

// TYPES
typedef struct Term_InputData
{
  
} Term_InputData;

void Term_Init();
void Term_GetInputData(Term_InputData * data);
void Term_Refresh();

#endif

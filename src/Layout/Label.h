#ifndef LABEL_H_
#define LABEL_H_

#include "Layout.h"

// types



typedef struct Label
{
    Layout layout;

    bool strIsCpy;
    bool strDisposed;
    char* str;
    Alignment vertAlign;
    Alignment horzAlign;
} Label;

// funcs

Label * Label_Create();
void Label_SetTextCpy(Label * label, char* str);
void Label_SetTextPtr(Label * label, char* str);

#endif
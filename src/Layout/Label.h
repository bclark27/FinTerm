#ifndef LABEL_H_
#define LABEL_H_

#include "Layout.h"

// types

typedef enum LabelTextOption
{
    LabelTextOptions_Left,
    LabelTextOptions_Center,
    LabelTextOptions_Right
} LabelTextOption;


typedef struct Label
{
    Layout layout;

    bool strIsCpy;
    bool strDisposed;
    char* str;
    Alignment vertAlign;
    Alignment horzAlign;
    LabelTextOption textOption;
    bool textWrap;
    bool border;
    int highlight;
    int textColor;
} Label;

// funcs

Label * Label_Create();
void Label_SetTextCpy(Label * label, char* str);
void Label_SetTextPtr(Label * label, char* str);
void Label_SetTextFmt(Label *label, const char *fmt, ...);
void Label_SetBorder(Label *label, bool border);
void Label_SetHighlight(Label *label, int highlight);
void Label_SetTextColor(Label *label, int textColor);

#endif
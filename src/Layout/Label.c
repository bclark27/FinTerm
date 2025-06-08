#include "Label.h"

#include "../Common.h"

// declerations

void label_draw(Layout*, WINDOW *win, int x, int y, int width, int height);
void label_onBubble(Layout* l, LayoutBubbleEvent* bbl);
void label_onDestroy(Layout* l);

void label_disposeStr(Label * label);
int label_lineCount(Label * label);

// vtable

static Layout_VT vtable = 
{
    .draw = label_draw,
    .onBubble = label_onBubble,
    .onDestroy = label_onDestroy,
};

// public

Label * Label_Create()
{
    Label * label = calloc(sizeof(Label), 1);
    Layout_Init((Layout*)label);
    ((Layout*)label)->vtable = vtable;
    return label;
}

void Label_SetTextCpy(Label * label, char* str)
{
    label_disposeStr(label);

    int len = strlen(str);
    label->str = malloc(len + 1);
    memmove(label->str, str, len);
    label->str[len] = 0;
    label->strIsCpy = true;
    label->strDisposed = false;

    ((Layout*)label)->isDirty = true;
}

void Label_SetTextPtr(Label * label, char* str)
{
    label_disposeStr(label);
    label->str = str;
    label->strIsCpy = false;
    label->strDisposed = false;
    ((Layout*)label)->isDirty = true;
}

// priv

void label_draw(Layout*l , WINDOW *win, int x, int y, int width, int height)
{
    if (!l) return;

    Label * label = (Label*)l;
    box(win, 0, 0);
    if (!label->strDisposed && label->str)
    {
        int len = strlen(label->str);
        int lc = label_lineCount(label);
        int x = 0;
        int y = 0;

        switch (label->horzAlign)
        {
            default:
            case Alignment_Start:
                break;
            case Alignment_Center:
                int len_2 = len / 2;
                int width_2 = width / 2;
                x = width_2 - len_2 + 1;
                break;
            case Alignment_End:
                x = width - len;
                break;
        }

        switch (label->vertAlign)
        {
            default:
            case Alignment_Start:
                break;
            case Alignment_Center:
                int lc_2 = lc / 2;
                int height_2 = height / 2;
                y = height_2 - lc_2 + 1;
                break;
            case Alignment_End:
                y = height - lc;
                break;
        }

        mvwprintw(win, y, x, "%s", label->str);
    }
}

void label_onBubble(Layout* l, LayoutBubbleEvent* bbl)
{
    
}

void label_onDestroy(Layout* l)
{
    label_disposeStr((Label*)l);
}

void label_disposeStr(Label * label)
{
    if (!label || !label->str || label->strDisposed) return;
    
    if (label->strIsCpy)
    {
        free(label->str);
    }

    label->str = NULL;
}

int label_lineCount(Label * label)
{
    if (!label || !label->str || label->strDisposed) return 0;

    int len = strlen(label->str);
    int lc = 1;
    for (int i = 0; i < len; i++)
    {
        if (label->str[i] == '\n') lc++;
    }

    return lc;
}
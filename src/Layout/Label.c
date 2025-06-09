#include "Label.h"

#include "../Common.h"

// declerations

void label_draw(Layout*, WINDOW *win, int x, int y, int width, int height);
void label_onDestroy(Layout* l);
void label_onPtrEnter(Layout* l, InputEvent* e);
void label_onPtrExit(Layout* l, InputEvent* e);
void label_onPtrMove(Layout* l, InputEvent* e);
void label_onFocus(Layout* l, InputEvent* e);
void label_onUnFocus(Layout* l, InputEvent* e);

void label_disposeStr(Label * label);
int label_lineCount(Label * label);
int label_textWidth(Label * label);

// vtable

static Layout_VT vtable = 
{
    .draw = label_draw,
    .onDestroy = label_onDestroy,
    .onPtrEnter = label_onPtrEnter,
    .onPtrExit = label_onPtrExit,
    .onPtrMove = label_onPtrMove,
    .onFocus = label_onFocus,
    .onUnFocus = label_onUnFocus,
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
    if (l->isFocus)
    {
        box(win, 0, 0);
    }
    if (!label->strDisposed && label->str)
    {
        int len = strlen(label->str);
        int textWidth = label_textWidth(label);
        int textWidth_2 = textWidth / 2;
        int lc = label_lineCount(label);
        int corner_x = 0;
        int corner_y = 0;
        int height_2 = height / 2;
        int width_2 = width / 2;

        switch (label->horzAlign)
        {
            default:
            case Alignment_Start:
                break;
            case Alignment_Center:
                corner_x = width_2 - textWidth_2 + 1;
                break;
            case Alignment_End:
                corner_x = width - textWidth;
                break;
        }

        switch (label->vertAlign)
        {
            default:
            case Alignment_Start:
                break;
            case Alignment_Center:
                int lc_2 = lc / 2;
                corner_y = height_2 - lc_2 + 1;
                break;
            case Alignment_End:
                corner_y = height - lc;
                break;
        }

        char* currLine = label->str;
        int currLineNum = 0;
        int currLineLen = 0;
        for (int i = 0; i < len; i++)
        {
            if (label->str[i] == '\n')
            {
                label->str[i] = 0;
                currLineLen = strlen(currLine);

                int currLineX = 0;
                switch (label->textOption)
                {
                    default:
                    case LabelTextOptions_Left:
                        break;
                    case LabelTextOptions_Center:
                        currLineX = textWidth_2 - (currLineLen / 2);
                        break;
                    case LabelTextOptions_Right:
                        currLineX = textWidth - currLineLen;
                        break;
                }

                mvwprintw(win, corner_y + currLineNum, corner_x + currLineX, "%s", currLine);
                label->str[i] = '\n';
                currLineNum++;
                currLine = &(label->str[i + 1]);
            }
            else if (i == len - 1)
            {
                currLineLen = strlen(currLine);
                int currLineX = 0;
                switch (label->textOption)
                {
                    default:
                    case LabelTextOptions_Left:
                        break;
                    case LabelTextOptions_Center:
                        currLineX = textWidth_2 - (currLineLen / 2);
                        break;
                    case LabelTextOptions_Right:
                        currLineX = textWidth - currLineLen;
                        break;
                }
                mvwprintw(win, corner_y + currLineNum, corner_x + currLineX, "%s", currLine); 
            }
        }
    }
}

void label_onDestroy(Layout* l)
{
    label_disposeStr((Label*)l);
}

void label_onPtrEnter(Layout* l, InputEvent* e)
{
    
}

void label_onPtrExit(Layout* l, InputEvent* e)
{
    
}

void label_onPtrMove(Layout* l, InputEvent* e)
{

}

void label_onFocus(Layout* l, InputEvent* e)
{
    l->isDirty = true;
}

void label_onUnFocus(Layout* l, InputEvent* e)
{
    l->isDirty = true;
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

int label_textWidth(Label * label)
{
    if (!label || !label->str || label->strDisposed) return 0;

    int len = strlen(label->str);
    int longestLine = 0;
    int currLine = 0;
    for (int i = 0; i < len; i++)
    {
        if (label->str[i] == '\n')
        {
            longestLine = MAX(currLine, longestLine);
            currLine = 0;
        }
        else
        {
            currLine++;
        }
    }

    return longestLine;
}

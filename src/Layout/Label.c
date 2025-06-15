#include "Label.h"

#include "../Common.h"

// declerations

void label_draw(Layout*, WINDOW *win, int x, int y, int width, int height);
void label_onDestroy(Layout* l);
void label_onPtrEnter(Layout* l, InputEvent* e);
void label_onPtrExit(Layout* l, InputEvent* e);
void label_onPtrMove(Layout* l, InputEvent* e);
void label_onFocus(Layout* l);
void label_onUnFocus(Layout* l);

void label_disposeStr(Label * label);
int label_lineCount(Label * label);
int label_textWidth(Label * label);
char* label_wrapText(char* str, int maxWidth, bool wrap);

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
    .onBblEvt = NULL,
    .onBblEvtCapture = NULL,
};

// public

Label * Label_Create()
{
    Label * label = calloc(1, sizeof(Label));
    Layout_Init((Layout*)label);
    ((Layout*)label)->vtable = vtable;
    return label;
}

void Label_SetTextCpy(Label * label, char* str)
{
    label_disposeStr(label);

    if (str)
    {
        int len = strlen(str);
        label->str = malloc(len + 1);
        memmove(label->str, str, len);
        label->str[len] = 0;
    }
    else
    {
        label->str = NULL;
    }

    label->strIsCpy = true;
    label->strDisposed = false;
    ((Layout*)label)->redraw = true;
}

void Label_SetTextPtr(Label * label, char* str)
{
    label_disposeStr(label);
    label->str = str;
    label->strIsCpy = false;
    label->strDisposed = false;
    ((Layout*)label)->redraw = true;
}

void Label_SetTextFmt(Label *label, const char *fmt, ...) {
    label_disposeStr(label);

    va_list args;
    va_start(args, fmt);
    va_list args_copy;
    va_copy(args_copy, args);
    
    int len = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    label->str = malloc(len + 1);
    if (!label->str) {
        va_end(args_copy);
        return;
    }

    vsnprintf(label->str, len + 1, fmt, args_copy);
    va_end(args_copy);

    label->strIsCpy = false;
    label->strDisposed = false;
    ((Layout*)label)->redraw = true;
}

void Label_SetBorder(Label *label, bool border)
{
    if (!label) return;
    if (label->border == border) return;
    label->border = border;
}

// priv

void label_draw(Layout*l , WINDOW *win, int x, int y, int width, int height)
{
    if (!l) return;

    Label * label = (Label*)l;
    int color = DEFATTR_SELECTED_BOX();
    if (l->isFocus && color >= 0) wattron(win, COLOR_PAIR(color));
    if (label->border) box(win, 0, 0);
    if (l->isFocus && color >= 0) wattroff(win, COLOR_PAIR(color));
    
    int innerWidth = MAX(0, l->width - (l->pad_left + l->pad_right));
    int innerHeight = MAX(0, l->height - (l->pad_up + l->pad_down));;

    int xOffset = l->pad_left;
    int yOffset = l->pad_up;

    width = innerWidth;
    height = innerHeight;

    char* wrappedText = label_wrapText(label->str, width, label->textWrap);

    if (!label->strDisposed && wrappedText)
    {
        int len = strlen(wrappedText);
        int textWidth = label_textWidth(label);
        int textWidth_2 = textWidth / 2;
        int lc = label_lineCount(label);
        int corner_x = xOffset;
        int corner_y = yOffset;
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

        char* currLine = wrappedText;
        int currLineNum = 0;
        int currLineLen = 0;
        for (int i = 0; i < len; i++)
        {
            if (wrappedText[i] == '\n')
            {
                wrappedText[i] = 0;
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
                wrappedText[i] = '\n';
                currLineNum++;
                currLine = &(wrappedText[i + 1]);
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

    if (wrappedText)
    {
        free(wrappedText);
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

void label_onFocus(Layout* l)
{
    l->redraw = true;
}

void label_onUnFocus(Layout* l)
{
    l->redraw = true;
}

void label_disposeStr(Label * label)
{
    if (!label || label->strDisposed) return;

    label->strDisposed = true;
    
    if (!label->str) return;

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

char* label_wrapText(char* str, int maxWidth, bool wrap)
{
    if (!str) return NULL;
    if (!wrap)
    {
        char* ret = malloc(strlen(str) + 1);
        strcpy(ret, str);
        return ret;
    }

    int len = strlen(str);
    char* result = malloc(len * 2 + 1); // assume worst case: every word gets a newline
    if (!result) return NULL;

    int lineLen = 0;
    char* resPtr = result;
    const char* wordStart = str;
    const char* ptr = str;

    while (*ptr)
    {
        // Find end of word or newline
        if (*ptr == ' ' || *ptr == '\n' || *(ptr + 1) == '\0')
        {
            int wordLen = ptr - wordStart;
            if (*(ptr + 1) == '\0') wordLen++; // include last char

            // If word doesn't fit in current line
            if (lineLen + wordLen > maxWidth && lineLen > 0)
            {
                *resPtr++ = '\n';
                lineLen = 0;
            }

            // Copy the word
            strncpy(resPtr, wordStart, wordLen);
            resPtr += wordLen;
            lineLen += wordLen;

            // Add space or newline
            if (*ptr == ' ')
            {
                *resPtr++ = ' ';
                lineLen++;
            }
            else if (*ptr == '\n')
            {
                *resPtr++ = '\n';
                lineLen = 0;
            }

            wordStart = ptr + 1;
        }
        ptr++;
    }

    *resPtr = '\0';
    return result;
}
#ifndef ENTRY_H_
#define ENTRY_H_

#include "Layout.h"
#include "Label.h"

// types

typedef struct Entry
{
    Layout layout;
    Alignment vertAlign;
    Alignment horzAlign;
    Label* labelChild;
} Entry;

// funcs

Entry * Entry_Create();

#endif
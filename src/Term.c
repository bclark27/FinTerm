#include "Term.h"

// TYPES

typedef struct TermMeta
{
  bool initDone;
} TermMeta;

static TermMeta termMeta;

// PUBLIC FUNCS

void Term_Init()
{
  if (termMeta.initDone)
    return;

  termMeta.initDone = true;

}

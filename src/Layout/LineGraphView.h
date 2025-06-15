#ifndef LINE_GRAPH_VIEW_H_
#define LINE_GRAPH_VIEW_H_

#include "Layout.h"

#define LINE_GRAPH_REDRAW_PLOT(p)       \
    do { \
        ((LineGraphType*)(p))->redraw = true; \
        REDRAW(((LineGraphType*)(p))->parentView); \
    } while (0)

#define LINE_GRAPH_VIEW_MAX_PLOTS       (20)

struct LineGraphView;

typedef enum LineGraphType
{
    LineGraphType_Line,
    LineGraphType_Candle,
} LineGraphType;

typedef struct LineGraphProps
{
    struct LineGraphView* parentView;
    LineGraphType type;
    double* x;
    double* y;
    int dataLen;
    double viewStart_x;
    double viewEnd_x;
    double viewStart_y;
    double viewEnd_y;
    bool redraw;
    char** plot;
    int plotHeight;
    int plotWidth;
} LineGraphProps;

typedef struct LineGraphView
{
    int plotCount;
    int plotAreaWidth;
    int plotAreaHeight;
    LineGraphProps* plots[LINE_GRAPH_VIEW_MAX_PLOTS];
} LineGraphView;

LineGraphView* LineGraphView_Create();

#endif
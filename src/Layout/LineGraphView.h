#ifndef LINE_GRAPH_VIEW_H_
#define LINE_GRAPH_VIEW_H_

#include "Layout.h"

#define LINE_GRAPH_VIEW_MAX_PLOTS       (20)
#define GET_PLOT_TYPE(p)                (((LineGraphProps*)(p))->type)

typedef enum LineGraphType
{
    LineGraphType_Line,
    LineGraphType_Candle,
} LineGraphType;

struct LineGraphView;

typedef struct LineGraphProps
{
    struct LineGraphView* parent;
    LineGraphType type;
    bool redraw;
    char** plot;
    int plotHeight;
    int plotWidth;
    double viewStart_x;
    double viewEnd_x;
    double viewStart_y;
    double viewEnd_y;
    bool viewBoxAbsolute;
    int plotZIndex;

    // USER HANDLED MALLOC
    double* x;
    double* y;
    int dataLen;
} LineGraphProps;

typedef struct LineGraphProps_Line
{
    LineGraphProps lineGraphProps;

} LineGraphProps_Line;

typedef struct LineGraphProps_Candle
{
    LineGraphProps lineGraphProps;

    double candleWidth;
} LineGraphProps_Candle;

typedef struct LineGraphView
{
    Layout layout;

    int plotCount;
    int plotAreaWidth;
    int plotAreaHeight;
    LineGraphProps* plots[LINE_GRAPH_VIEW_MAX_PLOTS];
} LineGraphView;

LineGraphView* LineGraphView_Create();

int LineGraphView_AddPlot(LineGraphView* g, LineGraphType type);
void LineGraphView_SetViewBox(LineGraphView* g, int plot, double x_start, double x_end, double y_start, double y_end, bool absolute);
void LineGraphView_SetDataPonters(LineGraphView* g, int plot, double* x, double* y, int len);
void LineGraphView_SetPlotZIndex(LineGraphView* g, int plot, int zindex);
void LineGraphView_RedrawPlot(LineGraphView* g, int plot);
void LineGraphView_DestroyPlot(LineGraphView* g, int plot);

#endif
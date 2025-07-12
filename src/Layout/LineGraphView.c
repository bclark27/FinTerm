#include "LineGraphView.h"

// PRIV DECLERATIONS

void lineGraphView_draw(Layout* l, WINDOW *win, int x, int y, int width, int height);
void lineGraphView_onDestroy(Layout* l);
void lineGraphView_onPtrEnter(Layout* l, InputEvent* e);
void lineGraphView_onPtrExit(Layout* l, InputEvent* e);
void lineGraphView_onPtrMove(Layout* l, InputEvent* e);
void lineGraphView_onFocus(Layout* l);
void lineGraphView_onUnFocus(Layout* l);
void lineGraphView_onSizeRefreshed(Layout* l);

void lineGraphView_resizeCharBuffer(LineGraphProps* linep, int width, int height);
char lineGraphView_pick_char_for_slope(double dy, double dx);
void lineGraphView_renderPlot(LineGraphProps_Line* p);
void lineGraphView_renderCandles(LineGraphProps_Candle* p);
// vtable

static Layout_VT vtable = 
{
    .draw = lineGraphView_draw,
    .onDestroy = lineGraphView_onDestroy,
    .onPtrEnter = lineGraphView_onPtrEnter,
    .onPtrExit = lineGraphView_onPtrExit,
    .onPtrMove = lineGraphView_onPtrMove,
    .onFocus = lineGraphView_onFocus,
    .onUnFocus = lineGraphView_onUnFocus,
    .onBblEvt = NULL,
    .onBblEvtCapture = NULL,
    .onSizeRefreshed = lineGraphView_onSizeRefreshed,
};

// PUBLIC FUNCS

LineGraphView* LineGraphView_Create()
{
    LineGraphView * g = calloc(1, sizeof(LineGraphView));
    Layout_Init((Layout*)g);
    ((Layout*)g)->vtable = vtable;

    return g;
}

int LineGraphView_AddPlot(LineGraphView* g, LineGraphType type)
{
    int plotIdx = -1;
    for (int i = 0; i < LINE_GRAPH_VIEW_MAX_PLOTS; i++)
    {
        if (!g->plots[i])
        {
            plotIdx = i;
            break;
        }
    }

    if (plotIdx < 0) return -1;

    LineGraphProps* p;

    switch (type)
    {
        case LineGraphType_Line:
            p = calloc(1, sizeof(LineGraphProps_Line));
            break;
        case LineGraphType_Candle:
            p = calloc(1, sizeof(LineGraphProps_Candle));
            ((LineGraphProps_Candle*)p)->candleWidth = 1;
            break;
        default:
            p = NULL;
            break;
    }
    if (!p) return -1;

    p->redraw = true;
    p->viewStart_x = 0;
    p->viewEnd_x = 1;
    p->viewStart_y = 0;
    p->viewEnd_y = 1;
    p->viewBoxAbsolute = true;
    p->plotZIndex = 0;
    p->type = type;
    lineGraphView_resizeCharBuffer(p, g->layout.width, g->layout.height);

    g->plots[plotIdx] = p;
    REDRAW(g);

    return plotIdx;
}

void LineGraphView_SetViewBox(LineGraphView* g, int plot, double x_start, double x_end, double y_start, double y_end, bool absolute)
{
    if (!g || plot < 0 || plot >= LINE_GRAPH_VIEW_MAX_PLOTS) return;
    LineGraphProps* p = g->plots[plot];

    p->viewStart_x = x_start;
    p->viewEnd_x = x_end;
    p->viewStart_y = y_start;
    p->viewEnd_y = y_end;
    p->viewBoxAbsolute = absolute;

    g->plots[plot]->redraw = true;
    REDRAW(g);
}

void LineGraphView_SetDataPonters(LineGraphView* g, int plot, double* x, double* y, int len)
{
}
void LineGraphView_SetPlotZIndex(LineGraphView* g, int plot, int zindex);
void LineGraphView_RedrawPlot(LineGraphView* g, int plot)
{
    if (!g || plot < 0 || plot >= LINE_GRAPH_VIEW_MAX_PLOTS) return;
    g->plots[plot]->redraw = true;
    REDRAW(g);
}

void LineGraphView_DestroyPlot(LineGraphView* g, int plot)
{
    if (!g || plot < 0 || plot >= LINE_GRAPH_VIEW_MAX_PLOTS) return;
    LineGraphProps* p = g->plots[plot];

    lineGraphView_resizeCharBuffer(p, 0, 0);

    if (p)
    {
        switch (p->type)
        {
            case LineGraphType_Line:
                break;
            case LineGraphType_Candle:
                break;
            default:
                break;
        }
    }

    free(p);
    g->plots[plot] = NULL;

    REDRAW(g);
}

// priv

void lineGraphView_draw(Layout* l, WINDOW *win, int x, int y, int width, int height)
{

    LineGraphView* g = (LineGraphView*)l;
    
    for (int i = 0; i < LINE_GRAPH_VIEW_MAX_PLOTS; i++)
    {
        LineGraphProps* p = g->plots[i];
        if (!p || !p->redraw) continue;        
        switch (p->type)
        {
            case LineGraphType_Line:
                lineGraphView_renderPlot((LineGraphProps_Line*)p);
            case LineGraphType_Candle:
                lineGraphView_renderCandles((LineGraphProps_Candle*)p);
                break;
            default:
                break;
        }
    }
}

void lineGraphView_onDestroy(Layout* l)
{
    LineGraphView* g = (LineGraphView*)l;
    
    for (int i = 0; i < LINE_GRAPH_VIEW_MAX_PLOTS; i++)
    {
        LineGraphView_DestroyPlot(g, i);
    }
}

void lineGraphView_onPtrEnter(Layout* l, InputEvent* e)
{

}

void lineGraphView_onPtrExit(Layout* l, InputEvent* e)
{

}

void lineGraphView_onPtrMove(Layout* l, InputEvent* e)
{

}

void lineGraphView_onFocus(Layout* l)
{
    REDRAW(l);
}

void lineGraphView_onUnFocus(Layout* l)
{
    REDRAW(l);
}

void lineGraphView_onSizeRefreshed(Layout* l)
{

}

void lineGraphView_resizeCharBuffer(LineGraphProps* linep, int width, int height)
{
    if (!linep) return;

    // first destroy the last one
    if (linep->plot)
    {
        for (int y = 0; y < linep->plotHeight; y++)
        {
            if (linep->plot[y]) free(linep->plot[y]);
        }

        free(linep->plot);
    }

    linep->plot = NULL;
    linep->plotHeight = height;
    linep->plotWidth = width;

    if (width <= 0 || height <= 0) return;

    linep->plot = malloc(sizeof(char*) * linep->plotHeight);
    for (int y = 0; y < linep->plotHeight; y++)
    {
        if (linep->plot[y]) calloc(linep->plotWidth, sizeof(char));
    }

    linep->redraw = true;
    REDRAW(linep->parent);
}

// Basic pipe characters for line continuity
char lineGraphView_pick_char_for_slope(double dy, double dx) 
{
    double slope = dy / dx;
    if (slope > 2.5) return '|';
    if (slope > 0.5) return '/';
    if (slope < -2.5) return '|';
    if (slope < -0.5) return '\\';
    return '-';
}

void lineGraphView_renderPlot(LineGraphProps_Line* p) 
{
    if (!p || p->lineGraphProps.plotHeight <= 0 || p->lineGraphProps.plotWidth <= 0) return;

    // Clear buffer
    for (int r = 0; r < p->lineGraphProps.plotHeight; r++)
        for (int c = 0; c < p->lineGraphProps.plotWidth; c++)
            p->lineGraphProps.plot[r][c] = ' ';

    if (!p->lineGraphProps.x || !p->lineGraphProps.y || p->lineGraphProps.dataLen <= 1) return;

    double x_range = p->lineGraphProps.viewEnd_x - p->lineGraphProps.viewStart_x;
    double y_range = p->lineGraphProps.viewEnd_y - p->lineGraphProps.viewStart_y;
    if (x_range <= 0 || y_range <= 0) return;

    // Previous screen coords for interpolation
    int prev_col = -1, prev_row = -1;
    double prev_x = 0, prev_y = 0;
    bool has_prev = false;

    for (int i = 0; i < p->lineGraphProps.dataLen; i++) {
        double x_val = p->lineGraphProps.x[i];
        double y_val = p->lineGraphProps.y[i];

        if (x_val < p->lineGraphProps.viewStart_x || x_val > p->lineGraphProps.viewEnd_x ||
            y_val < p->lineGraphProps.viewStart_y || y_val > p->lineGraphProps.viewEnd_y) {
            has_prev = false;
            continue;
        }

        // Normalize and convert to screen coordinates
        int col = (int)(((x_val - p->lineGraphProps.viewStart_x) / x_range) * (p->lineGraphProps.plotWidth - 1));
        int row = p->lineGraphProps.plotHeight - 1 - (int)(((y_val - p->lineGraphProps.viewStart_y) / y_range) * (p->lineGraphProps.plotHeight - 1));

        // Plot
        p->lineGraphProps.plot[row][col] = '*';

        if (has_prev) {
            // Linear interpolate in screen space if skipping pixels
            int dx = abs(col - prev_col);
            int dy = abs(row - prev_row);
            int steps = dx > dy ? dx : dy;
            for (int s = 1; s < steps; s++) {
                double t = (double)s / steps;
                int interp_col = (int)(prev_col + t * (col - prev_col));
                int interp_row = (int)(prev_row + t * (row - prev_row));
                char ch = lineGraphView_pick_char_for_slope(y_val - prev_y, x_val - prev_x);
                if (p->lineGraphProps.plot[interp_row][interp_col] == ' ')
                    p->lineGraphProps.plot[interp_row][interp_col] = ch;
            }
        }

        prev_col = col;
        prev_row = row;
        prev_x = x_val;
        prev_y = y_val;
        has_prev = true;
    }
}

void lineGraphView_renderCandles(LineGraphProps_Candle* p) 
{
    if (!p || p->lineGraphProps.plotHeight <= 0 || p->lineGraphProps.plotWidth <= 0) return;

    // Clear buffer
    for (int r = 0; r < p->lineGraphProps.plotHeight; r++)
        for (int c = 0; c < p->lineGraphProps.plotWidth; c++)
            p->lineGraphProps.plot[r][c] = ' ';

    if (!p->lineGraphProps.x || !p->lineGraphProps.y || p->lineGraphProps.dataLen <= 1) return;

    double x_range = p->lineGraphProps.viewEnd_x - p->lineGraphProps.viewStart_x;
    double y_range = p->lineGraphProps.viewEnd_y - p->lineGraphProps.viewStart_y;
    if (x_range <= 0 || y_range <= 0) return;

    for (int col = 0; col < p->lineGraphProps.plotWidth; col++) {
        double x_start = p->lineGraphProps.viewStart_x + ((double)col / p->lineGraphProps.plotWidth) * x_range;
        double x_end   = p->lineGraphProps.viewStart_x + ((double)(col + 1) / p->lineGraphProps.plotWidth) * x_range;

        double y_min = INFINITY;
        double y_max = -INFINITY;
        bool found = false;

        for (int i = 0; i < p->lineGraphProps.dataLen; i++) {
            if (p->lineGraphProps.x[i] >= x_start && p->lineGraphProps.x[i] <= x_end) {
                if (p->lineGraphProps.y[i] < y_min) y_min = p->lineGraphProps.y[i];
                if (p->lineGraphProps.y[i] > y_max) y_max = p->lineGraphProps.y[i];
                found = true;
            }
        }

        if (!found) continue;

        int row_min = p->lineGraphProps.plotHeight - 1 - (int)(((y_min - p->lineGraphProps.viewStart_y) / y_range) * (p->lineGraphProps.plotHeight - 1));
        int row_max = p->lineGraphProps.plotHeight - 1 - (int)(((y_max - p->lineGraphProps.viewStart_y) / y_range) * (p->lineGraphProps.plotHeight - 1));

        if (row_max < 0) row_max = 0;
        if (row_min >= p->lineGraphProps.plotHeight) row_min = p->lineGraphProps.plotHeight - 1;
        if (row_min < 0) row_min = 0;
        if (row_max >= p->lineGraphProps.plotHeight) row_max = p->lineGraphProps.plotHeight - 1;

        for (int r = row_max; r <= row_min; r++) {
            p->lineGraphProps.plot[r][col] = '|';
        }
    }
}

/**
 * @file chart_handler.c
 * @brief Function used to manipulate the chart data like scaling and shifting of the signal
 *
 * @date Apr 26, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "chart_handler.h"

#include <string.h>

void chart_handler_init(ChartHandler * handler, void * api) {
    if (handler == NULL || api == NULL)
        return;
    handler->api = api;
    handler->x_scale = CHART_HANDLER_MIN_X_SCALE;
    memset(handler->scale, CHART_HANDLER_MAX_Y_SCALE, CHART_HANDLER_CHANNEL_COUNT * sizeof(uint32_t));
    memset(handler->offset, 0U, CHART_HANDLER_CHANNEL_COUNT * sizeof(int32_t));
    memset(handler->index, 0U, CHART_HANDLER_CHANNEL_COUNT * sizeof(size_t));
    memset(handler->data, 0U, CHART_HANDLER_CHANNEL_COUNT * CHART_HANDLER_SAMPLE_COUNT* sizeof(int32_t));
}

void chart_handler_add_point(ChartHandler * handler, ChartHandlerChannel ch, uint16_t value) {
    if (handler == NULL)
        return;

    handler->data[ch][handler->index[ch]] = value / 65.535;
    if ((++handler->index[ch]) >= CHART_HANDLER_SAMPLE_COUNT) {
        handler->index[ch] = 0;
        lv_api_update_points(handler->api, ch, handler->data[ch], CHART_HANDLER_SAMPLE_COUNT);
    }
}

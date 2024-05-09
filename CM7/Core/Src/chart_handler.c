/**
 * @file chart_handler.c
 * @brief Function used to manipulate the chart data like scaling and shifting of the signal
 *
 * @date Apr 26, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "chart_handler.h"

#include <string.h>

#include "lvgl_api.h"


void chart_handler_init(ChartHandler * handler, void * api) {
    if (handler == NULL || api == NULL)
        return;
    memset(handler, 0U, sizeof(ChartHandler));

    handler->api = api;
    for (size_t i = 0; i < CHART_HANDLER_CHANNEL_COUNT; ++i) {
        handler->x_scale[i] = 5000.0f; // CHART_MIN_X_SCALE;
        handler->scale[i] = 1000.0f;
    }
}

float chart_handler_get_offset(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0U;
    return handler->offset[ch];
}

void chart_handler_set_offset(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL)
        return;
    handler->offset[ch] = value;
}

float chart_handler_get_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0U;
    return handler->scale[ch];
}

void chart_handler_set_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL || value > CHART_MAX_Y_SCALE || value < CHART_MIN_Y_SCALE)
        return;
    handler->scale[ch] = value;
}

float chart_handler_get_x_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0U;
    return handler->x_scale[ch];
}

void chart_handler_set_x_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL || value > CHART_MAX_X_SCALE || value < CHART_MIN_X_SCALE)
        return;
    handler->x_scale[ch] = value;
}

void chart_handler_add_point(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL)
        return;
    handler->raw[ch][handler->index[ch]] = value;
    if ((++handler->index[ch]) >= CHART_SAMPLE_COUNT) {
        handler->index[ch] = 0;
        handler->ready[ch] = true;
    }
}

// TODO: Match chart X axis with time
void chart_handler_routine(ChartHandler * handler) {
    if (handler == NULL)
        return;

    for (size_t ch = 0 ; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        if (handler->ready[ch]) {
            for (size_t i = 0; i < CHART_SAMPLE_COUNT; ++i) {
                float val = handler->raw[ch][i];

                // Translate
                val += handler->offset[ch];

                // Convert to grid units
                val /= handler->scale[ch];

                handler->data[ch][i] = val;
            }

            lv_api_update_points(handler->api, ch, handler->data[ch], CHART_SAMPLE_COUNT);
            handler->ready[ch] = false;
        }
    }
}

void chart_handler_invalidate(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;

    // Reset data
    handler->ready[ch] = false;
    memset(handler->raw[ch], 0U, CHART_SAMPLE_COUNT * sizeof(handler->raw[ch][0U]));
    memset(handler->data[ch], 0U, CHART_SAMPLE_COUNT * sizeof(handler->data[ch][0U]));
}

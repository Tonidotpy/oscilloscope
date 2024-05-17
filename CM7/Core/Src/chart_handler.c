/**
 * @file chart_handler.c
 * @brief Function used to manipulate the chart data like scaling and shifting of the signal
 *
 * @date Apr 26, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "chart_handler.h"

#include <string.h>

#include "config.h"
#include "lvgl_api.h"


void chart_handler_init(ChartHandler * handler, void * api) {
    if (handler == NULL || api == NULL)
        return;
    memset(handler, 0U, sizeof(ChartHandler));

    handler->api = api;
    handler->enabled[CHART_HANDLER_CHANNEL_1] = true;
    for (size_t i = 0; i < CHART_HANDLER_CHANNEL_COUNT; ++i) {
        handler->x_scale[i] = 5000.0f; // CHART_MIN_X_SCALE;
        handler->scale[i] = 1000.0f;
    }
}

void chart_handler_set_enable(ChartHandler * handler, ChartHandlerChannel ch, bool enabled) {
    if (handler == NULL)
        return;
    handler->enabled[ch] = enabled;
}

void chart_handler_toggle_enable(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;
    handler->enabled[ch] = !handler->enabled[ch];
}

float chart_handler_get_offset(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->offset[ch];
}

void chart_handler_set_offset(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL)
        return;
    handler->offset[ch] = value;
    chart_handler_invalidate(handler, ch);
}

float chart_handler_get_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->scale[ch];
}

void chart_handler_set_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL || value > CHART_MAX_Y_SCALE || value < CHART_MIN_Y_SCALE)
        return;
    handler->scale[ch] = value;
    chart_handler_invalidate(handler, ch);
}

float chart_handler_get_x_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->x_scale[ch];
}

void chart_handler_set_x_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL || value > CHART_MAX_X_SCALE || value < CHART_MIN_X_SCALE)
        return;
    handler->x_scale[ch] = value;
    chart_handler_invalidate(handler, ch);
}

void chart_handler_update(ChartHandler * handler, uint32_t t) {
    if (handler == NULL)
        return;

    const uint16_t * raw[] = {
        (uint16_t *)CHART_CH1_RAW_DATA_ADDRESS,
        (uint16_t *)CHART_CH2_RAW_DATA_ADDRESS
    };
    
    // Get time for each sample in us
    const float time_per_sample = t / (float)CHART_SAMPLE_COUNT;

    for (size_t ch = 0; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        if (!handler->enabled[ch] || handler->ready[ch])
            continue;

        // Time between each value in us
        const float time_per_value = handler->x_scale[ch] / CHART_HANDLER_VALUES_PER_DIVISION;
        const float inc = time_per_value / time_per_sample;

        float delta = 0.0f;
        
        for(int i = 0; i < CHART_SAMPLE_COUNT; i++) {
            if(delta >= time_per_value) {
                delta = 0.0f;
                handler->raw[ch][handler->index[ch]++] = raw[ch][i];
            }
            delta += time_per_sample;

            // Check if ready
            if (handler->index[ch] >= CHART_HANDLER_VALUES_COUNT) {
                handler->index[ch] = 0U;
                handler->ready[ch] = true;
                break;
            }
        }
    }
}

// TODO: Match chart X axis with time
void chart_handler_routine(ChartHandler * handler) {
    if (handler == NULL)
        return;
    
    for (size_t ch = 0 ; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        if (!handler->enabled[ch] || !handler->ready[ch])
            continue;

        for (size_t i = 0; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            float val = ADC_VALUE_TO_VOLTAGE(handler->raw[ch][i]);

            // Translate
            val += handler->offset[ch];

            // Convert to grid units
            val /= handler->scale[ch];

            handler->data[ch][i] = val;
        }

        lv_api_update_points(handler->api, ch, handler->data[ch], CHART_HANDLER_VALUES_COUNT);
        handler->ready[ch] = false;
    }
}

void chart_handler_invalidate(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;

    // Reset data
    handler->ready[ch] = false;
    memset(handler->raw[ch], 0U, CHART_HANDLER_VALUES_COUNT * sizeof(handler->raw[ch][0U]));
    memset(handler->data[ch], 0U, CHART_HANDLER_VALUES_COUNT * sizeof(handler->data[ch][0U]));
}

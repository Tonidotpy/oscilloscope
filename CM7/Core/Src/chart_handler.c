/**
 * @file chart_handler.c
 * @brief Function used to manipulate the chart data like scaling and shifting of the signal
 *
 * @date Apr 26, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "chart_handler.h"

#include <string.h>
#include <math.h>

#include "config.h"
#include "lvgl_api.h"

// TODO: Update offsets
void _chart_handler_reload(
    ChartHandler * handler,
    float off,
    float scale,
    float x_off,
    float x_scale)
{
    if (handler == NULL)
        return;

    for (size_t ch = 0U; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        // Ignore if running
        if (handler->enabled[ch])
            continue;

        // Calculate the ratio between the current scale and the previous
        const float scale_ratio = handler->scale[ch] / scale;
        const float x_scale_ratio = handler->x_scale[ch] / x_scale;

        // Calculate the delta between the current offset and the previous
        const float doff = off - handler->offset[ch];
        const float dx_off = off - handler->offset[ch];

        for (size_t i = 0; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            // Rescale
            handler->data[ch][i] *= scale_ratio;
            handler->data[ch][i] += (doff / scale); 
        }

        lv_api_update_points(handler->api, ch, handler->data[ch], CHART_HANDLER_VALUES_COUNT);
    }
}

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

bool chart_handler_is_enabled(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return false;
    return handler->enabled[ch];
}

void chart_handler_set_enable(ChartHandler * handler, ChartHandlerChannel ch, bool enabled) {
    if (handler == NULL)
        return;
    handler->enabled[ch] = enabled;
    if (enabled)
        chart_handler_invalidate(handler, ch);
}

void chart_handler_toggle_enable(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;
    handler->enabled[ch] = !handler->enabled[ch];
    if (handler->enabled[ch])
        chart_handler_invalidate(handler, ch);
}

float chart_handler_get_offset(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->offset[ch];
}

void chart_handler_set_offset(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL)
        return;

    // Reload the chart
    if (!handler->enabled[ch]) {
        _chart_handler_reload(
            handler,
            value,
            handler->scale[ch],
            handler->x_offset[ch],
            handler->x_scale[ch]
        );
    }

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

    // Reload the chart
    if (!handler->enabled[ch]) {
        _chart_handler_reload(
            handler,
            handler->offset[ch],
            value,
            handler->x_offset[ch],
            handler->x_scale[ch]
        );
    }

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

// TODO: Add horizontal offset
void chart_handler_update(ChartHandler * handler, uint32_t t) {
    if (handler == NULL)
        return;

    volatile const uint16_t * raw[] = {
        (uint16_t *)CHART_CH1_RAW_DATA_ADDRESS,
        (uint16_t *)CHART_CH2_RAW_DATA_ADDRESS
    };
    
    // Get time for each sample in us
    const float time_per_sample = t / (float)CHART_SAMPLE_COUNT;

    for (size_t ch = 0U; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        if (!handler->enabled[ch] || handler->ready[ch])
            continue;

        // Time between each value in us
        const float time_per_value = handler->x_scale[ch] / CHART_HANDLER_VALUES_PER_DIVISION;
        // Number of values for each sample
        const float samples_per_value = time_per_value / time_per_sample;

        volatile static float off = 0.f; 
        for(size_t i = 0U; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            // Calculate samples index
            float samples = samples_per_value * i + off;
            size_t j = samples < 0.f ? 0U : (size_t)floorf(samples);

            // Break if more samples are needed
            if (j >= CHART_SAMPLE_COUNT) {
                // Calculate offset
                off = (samples + 1.f) - (float)CHART_SAMPLE_COUNT;
                break;
            }

            // Copy value
            handler->raw[ch][handler->index[ch]++] = raw[ch][j];

            // Check if the signal is ready to be displayed
            if (handler->index[ch] >= CHART_HANDLER_VALUES_COUNT) {
                off = 0.f;
                handler->index[ch] = 0U;
                handler->ready[ch] = true;
                break;
            }
        }

    }
}

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
    handler->index[ch] = 0U;
}

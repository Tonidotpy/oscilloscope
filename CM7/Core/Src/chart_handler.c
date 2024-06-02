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

void chart_handler_init(ChartHandler * handler, void * api) {
    if (handler == NULL || api == NULL)
        return;
    memset(handler, 0U, sizeof(ChartHandler));

    handler->api = api;
    handler->enabled[CHART_HANDLER_CHANNEL_1] = true;
    for (size_t ch = 0; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        handler->running[ch] = true;

        handler->x_scale_paused[ch] = handler->x_scale[ch] = CHART_DEFAULT_X_SCALE; // CHART_MIN_X_SCALE;
        handler->scale[ch] = CHART_DEFAULT_Y_SCALE;
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
    else
        lv_api_clear_channel_data(handler->api, ch);
}

void chart_handler_toggle_enable(ChartHandler * handler, ChartHandlerChannel ch) {
    chart_handler_set_enable(handler, ch, !handler->enabled[ch]);
}

bool chart_handler_is_running(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return false;
    // A stop request is counted as not running, otherwise check the running flag
    return (!handler->stop_request[ch]) && handler->running[ch];
}

void chart_handler_set_running(ChartHandler * handler, ChartHandlerChannel ch, bool running) {
    if (handler == NULL)
        return;
    // Request stop if needed
    handler->stop_request[ch] = !running;

    if (running) {
        handler->running[ch] = true;
        chart_handler_invalidate(handler, ch);
    }
    else {
        // Save current X offset and scale
        handler->x_offset_paused[ch] = handler->x_offset[ch];
        handler->x_scale_paused[ch] = handler->x_scale[ch];
    }
}

void chart_handler_toggle_running(ChartHandler * handler, ChartHandlerChannel ch) {
    chart_handler_set_running(handler, ch, !handler->running[ch]);
}

float chart_handler_get_offset(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->offset[ch];
}

void chart_handler_set_offset(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL) return;
    if (!handler->enabled[ch]) return;

    // Update offset and invalidate old data
    handler->offset[ch] = value;
    chart_handler_invalidate(handler, ch);
}

float chart_handler_get_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0;
    return handler->scale[ch];
}

void chart_handler_set_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL) return;
    if (!handler->enabled[ch]) return;
    if (value > CHART_MAX_Y_SCALE || value < CHART_MIN_Y_SCALE) return;

    // Update scale and invalidate old data
    handler->scale[ch] = value;
    chart_handler_invalidate(handler, ch);

    // Notify LVGL
    lv_api_update_div_text(handler->api);
}

float chart_handler_get_x_scale(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0.f;
    return handler->x_scale[ch];
}

void chart_handler_set_x_scale(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL) return;
    if (!handler->enabled[ch]) return;
    if (value > CHART_MAX_X_SCALE || value < CHART_MIN_X_SCALE) return;

    // Update scale and invalidate old data
    handler->x_scale[ch] = value;
    chart_handler_invalidate(handler, ch);

    // Notify LVGL
    lv_api_update_div_text(handler->api);
}

float chart_handler_get_x_offset(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0.f;
    return handler->x_offset[ch];
}

void chart_handler_set_x_offset(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL) return;
    if (!handler->enabled[ch]) return;

    // Update offset and invalidate old data
    handler->x_offset[ch] = value;
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
        if (!handler->enabled[ch] || !handler->running[ch] || handler->ready[ch])
            continue;

        // Time between each value in us
        const float time_per_value = handler->x_scale[ch] / CHART_HANDLER_VALUES_PER_DIVISION;
        // Number of values for each sample
        const float samples_per_value = time_per_value / time_per_sample;

        uint16_t trigger = ADC_VOLTAGE_TO_VALUE(1000.0f);

        uint16_t prev_raw = raw[ch][0];

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

            if(!handler->found_trigger[ch]) 
                handler->before_trigger_cnt[ch]++;
            else
                handler->after_trigger_cnt[ch]++;

            // Copy value
            handler->raw[ch][handler->index[ch]] = raw[ch][j];
            ++handler->index[ch];
            handler->index[ch] %= CHART_HANDLER_VALUES_COUNT;

            if(handler->before_trigger_cnt[ch] < CHART_HANDLER_VALUES_COUNT / 2)
                continue;

            if(!handler->found_trigger[ch] && ((prev_raw <= trigger && raw[ch][j] >= trigger) ||
                                (prev_raw >= trigger && raw[ch][j] <= trigger))) {
                handler->trigger_index[ch] = handler->index[ch];
                handler->found_trigger[ch] = 1;
            }
            prev_raw = raw[ch][j];


            // Check if the signal is ready to be displayed
            if (handler->after_trigger_cnt[ch] >= CHART_HANDLER_VALUES_COUNT / 2) {
                // Stop the update if requested
                if (handler->stop_request[ch]) {
                    handler->running[ch] = false;
                    handler->stop_request[ch] = false;

                    // Save current X scale and offset
                    handler->x_scale_paused[ch] = handler->x_scale[ch];
                    handler->x_offset_paused[ch] = handler->x_offset[ch];
                }
                handler->after_trigger_cnt[ch] = 0;
                // handler->before_trigger_cnt[ch] = 0;
                handler->found_trigger[ch] = 0;
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
        // Do not update if the channel is not enabled or it's running but the data is not ready
        if (!handler->enabled[ch] || (handler->running[ch] && !handler->ready[ch]))
            continue;

        const float x_scale_ratio = handler->x_scale[ch] / handler->x_scale_paused[ch];

        const float time_per_value = handler->x_scale[ch] / CHART_HANDLER_VALUES_PER_DIVISION;
        const float x_off = handler->x_offset[ch] - handler->x_offset_paused[ch];
        const float i_off = x_off / time_per_value;

        for (size_t i = 0; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            float val = NAN;
            if (!handler->running[ch]) {
                volatile int j = (int)((i - i_off) * x_scale_ratio);
                if (j >= 0 && j < CHART_HANDLER_VALUES_COUNT)
                    val = ADC_VALUE_TO_VOLTAGE(handler->raw[ch][j]);
            }
            else{
                size_t index = (i + handler->trigger_index[ch] + CHART_HANDLER_VALUES_COUNT / 2) % CHART_HANDLER_VALUES_COUNT;
                val = ADC_VALUE_TO_VOLTAGE(handler->raw[ch][index]);
            }

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
    handler->after_trigger_cnt[ch] = 0U;
    handler->index[ch] = 0U;
}

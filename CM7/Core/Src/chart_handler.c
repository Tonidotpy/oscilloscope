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

/** @brief Delta used for the trigger threshold to be considered as rising or falling edge */
#define CHART_HANDLER_TRIGGER_DELTA (100U)

/**
 * @brief Check if a rising edge is found in the signal
 *
 * @param prev The previous value of the signal
 * @param cur The current value of the signal
 * @param trigger The treshold used to check for the rising edge
 *
 * @return bool True if a rising edge is found, false otherwise
 */
inline bool _chart_handler_is_rising_edge(uint16_t prev, uint16_t cur, uint16_t trigger) {
    return prev <= trigger && cur > trigger;
}

/**
 * @brief Check if a falling edge is found in the signal
 *
 * @param prev The previous value of the signal
 * @param cur The current value of the signal
 * @param trigger The treshold used to check for the falling edge
 *
 * @return bool True if a falling edge is found, false otherwise
 */
inline bool _chart_handler_is_falling_edge(uint16_t prev, uint16_t cur, uint16_t trigger) {
    return prev >= trigger && cur < trigger;
}

/**
 * @brief Check if the signal data is ready to be plotted
 *
 * @param chart_handler A pointer to the chart handler structure
 * @param count The number of values taken after the signal crosses the trigger
 * @param index The index of the array where the data of the signal is stored
 */
inline bool _chart_handler_is_data_ready(ChartHandler * handler, size_t count, size_t index) {
    return chart_handler_is_trigger_enabled(handler) ?
        (count >= CHART_HANDLER_VALUES_COUNT / 2) :
        (index >= CHART_HANDLER_VALUES_COUNT);
}

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

        handler->trigger[ch] = ADC_VOLTAGE_TO_VALUE(1000.f);
        handler->trigger_index[ch] = -1;
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

uint16_t chart_handler_get_trigger(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return 0U;
    return handler->trigger[ch];
}

void chart_handler_set_trigger(
    ChartHandler * handler,
    ChartHandlerChannel ch,
    uint16_t value)
{
    if (handler == NULL || !chart_handler_is_trigger_enabled(handler))
        return;
    handler->trigger[ch] = value;
    lv_api_update_trigger_line(handler->api, ch, ADC_VALUE_TO_VOLTAGE(value));
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

        lv_api_enable_trigger_checkbox(handler->api);
    }
    else {
        // Save current X offset and scale
        handler->x_offset_paused[ch] = handler->x_offset[ch];
        handler->x_scale_paused[ch] = handler->x_scale[ch];

        lv_api_disable_trigger_checkbox(handler->api);
    }
}

void chart_handler_toggle_running(ChartHandler * handler, ChartHandlerChannel ch) {
    chart_handler_set_running(handler, ch, !handler->running[ch]);
}

/**
 * @brief Check if the trigger is enabled
 *
 * @param handler A pointer to the chart handler structure
 *
 * @return bool True if the trigger is enbaled, false otherwise
 */
bool chart_handler_is_trigger_enabled(ChartHandler * handler) {
    return handler->ascending_trigger || handler->descending_trigger;
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
    if (chart_handler_is_trigger_enabled(handler)) {
        lv_api_update_trigger_line(
            handler->api,
            CHART_HANDLER_CHANNEL_1,
            ADC_VALUE_TO_VOLTAGE(handler->trigger[ch])
        );
    }
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

float chart_handler_voltage_to_grid_units(ChartHandler * handler, ChartHandlerChannel ch, float value) {
    if (handler == NULL)
        return 0.f;
    return value / handler->scale[ch];
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

        static uint16_t prev_raw = 0U;

        volatile static float off = 0.f; 
        for(size_t i = 0U; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            // Calculate samples index
            float samples = samples_per_value * i + off;
            size_t j = samples < 0.f ? 0U : (size_t)floorf(samples);

            // Break if more samples are needed
            if (j >= CHART_SAMPLE_COUNT) {
                // Calculate offset
                off = (samples + 1.f) - (float)CHART_SAMPLE_COUNT;

                // Update loading bar
                if (!chart_handler_is_trigger_enabled(handler) && handler->x_scale[ch] >= CHART_LOADING_BAR_THRESHOLD)
                    lv_api_update_loading_bar(handler->api, handler->index[CHART_HANDLER_CHANNEL_1]);
                break;
            }

            // Copy value
            uint16_t value = raw[ch][j];
            handler->raw[ch][handler->index[ch]] = value;

            // Trigger
            // TODO: Add horizontal offset
            if (chart_handler_is_trigger_enabled(handler)) {
                // Wait until there are enough samples before the trigger
                if (handler->trigger_before_count[ch] < CHART_HANDLER_VALUES_COUNT / 2U) {
                    ++handler->trigger_before_count[ch];
                    if (handler->x_scale[ch] >= CHART_LOADING_BAR_THRESHOLD)
                        lv_api_update_loading_bar(handler->api, handler->trigger_before_count[CHART_HANDLER_CHANNEL_1]);
                }
                else {
                    bool asc = handler->ascending_trigger && _chart_handler_is_rising_edge(prev_raw, value, handler->trigger[ch]);
                    bool desc = handler->descending_trigger && _chart_handler_is_falling_edge(prev_raw, value, handler->trigger[ch]);

                    // Check if signal has crossed the trigger
                    if (handler->trigger_index[ch] < 0 && (asc || desc))
                        handler->trigger_index[ch] = handler->index[ch];

                    if (handler->trigger_index[ch] >= 0) {
                        ++handler->trigger_after_count[ch];
                        if (handler->x_scale[ch] >= CHART_LOADING_BAR_THRESHOLD)
                            lv_api_update_loading_bar(
                                handler->api,
                                handler->trigger_before_count[CHART_HANDLER_CHANNEL_1] +
                                handler->trigger_after_count[CHART_HANDLER_CHANNEL_1]
                            );
                    }
                }
                prev_raw = value;
            }
            ++handler->index[ch];


            // Check if the signal is ready to be displayed
            if (_chart_handler_is_data_ready(handler, handler->trigger_after_count[ch], handler->index[ch])) {
                // Hide loading bar when data is ready
                lv_api_hide_loading_bar(handler->api);

                // Stop the update if requested
                if (handler->stop_request[ch]) {
                    handler->running[ch] = false;
                    handler->stop_request[ch] = false;

                    // Save current X scale and offset
                    handler->x_scale_paused[ch] = handler->x_scale[ch];
                    handler->x_offset_paused[ch] = handler->x_offset[ch];
                }

                handler->trigger_before_count[ch] = 0U;
                handler->trigger_after_count[ch] = 0U;
                prev_raw = 0U;
                
                off = 0.f;
                handler->index[ch] = 0U;
                handler->ready[ch] = true;
                break;
            }
            handler->index[ch] %= CHART_HANDLER_VALUES_COUNT;
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

        const size_t half = CHART_HANDLER_VALUES_COUNT / 2U;

        // Shift index based on trigger if enabled
        size_t index = 0;
        if (chart_handler_is_trigger_enabled(handler)) {
            const size_t trigger_offset = half;
            index = (trigger_offset - handler->trigger_index[ch] + CHART_HANDLER_VALUES_COUNT) % CHART_HANDLER_VALUES_COUNT;
        }
        
        for (volatile size_t i = 0; i < CHART_HANDLER_VALUES_COUNT; ++i) {
            float val = NAN;
 
            if (!chart_handler_is_running(handler, ch)) {
                // Do not update the values if the oscilloscope is stopped
                volatile int j = ((int)(i - i_off) * x_scale_ratio);

                // Deal with edge cases
                if (chart_handler_is_trigger_enabled(handler)) {
                    j -= handler->trigger_index[ch] * (x_scale_ratio - 1);

                    // BUG: Time rescaling breaks signal
                    // if (x_scale_ratio >= 1) {
                    //     // Fix small issues with rescaling
                    //     if (index > half && j >= handler->trigger_index[ch] + half)
                    //         j = -1;
                    //     else if (index <= half && j >= handler->trigger_index[ch] - half)
                    //         j = (j + CHART_HANDLER_VALUES_COUNT) % CHART_HANDLER_VALUES_COUNT;
                    // }
                    // else {
                    //     if (index <= half && j > handler->trigger_index[ch] && j <= handler->trigger_index[ch] + half)
                    //         j = (j + half) % CHART_HANDLER_VALUES_COUNT;
                    // }
                }
                else
                    j -= half * ((int)x_scale_ratio - 1);

                if (j >= 0 && j < CHART_HANDLER_VALUES_COUNT)
                    val = ADC_VALUE_TO_VOLTAGE(handler->raw[ch][j]);
            }
            else
                val = ADC_VALUE_TO_VOLTAGE(handler->raw[ch][i]);

            // Translate
            val += handler->offset[ch];

            // Convert to grid units
            val = chart_handler_voltage_to_grid_units(handler, ch, val);

            // Copy data
            handler->data[ch][index] = val;
            
            // Update index
            ++index;
            index %= CHART_HANDLER_VALUES_COUNT;
        }

        lv_api_update_points(handler->api, ch, handler->data[ch], CHART_HANDLER_VALUES_COUNT);
        if (handler->running[ch])
            handler->trigger_index[ch] = -1;
        handler->trigger_before_count[ch] = 0U;
        handler->trigger_after_count[ch] = 0U;
        handler->ready[ch] = false;
    }
}

void chart_handler_invalidate(ChartHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;

    // Reset data
    handler->index[ch] = 0U;
    if (handler->running[ch])
        handler->trigger_index[ch] = -1;
    handler->trigger_before_count[ch] = 0;
    handler->trigger_after_count[ch] = 0;
    handler->ready[ch] = false;

    lv_api_hide_loading_bar(handler->api);
}

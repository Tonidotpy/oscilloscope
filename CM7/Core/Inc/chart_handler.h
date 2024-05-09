/**
 * @file chart_handler.h
 * @brief Function used to manipulate the chart data like scaling and shifting of the signal
 *
 * @date Apr 26, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef CHART_HANDLER_H
#define CHART_HANDLER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "config.h"


/** @brief Available channels of the oscilloscope */
typedef enum {
    CHART_HANDLER_CHANNEL_1,
    CHART_HANDLER_CHANNEL_2,
    CHART_HANDLER_CHANNEL_COUNT
} ChartHandlerChannel;

typedef struct {
    void * api;

    float x_scale[CHART_HANDLER_CHANNEL_COUNT]; // in us
    float scale[CHART_HANDLER_CHANNEL_COUNT]; // in mV

    float offset[CHART_HANDLER_CHANNEL_COUNT]; // in mV
 
    bool ready[CHART_HANDLER_CHANNEL_COUNT];
    size_t index[CHART_HANDLER_CHANNEL_COUNT];
    float raw[CHART_HANDLER_CHANNEL_COUNT][CHART_SAMPLE_COUNT] __attribute__((aligned(16)));
    float data[CHART_HANDLER_CHANNEL_COUNT][CHART_SAMPLE_COUNT];
} ChartHandler;

/**
 * @brief Initialize the chart handler
 * 
 * @param handler A pointer to the chart handler structure
 * @param api A pointer to an LVGL api handler structure
 */
void chart_handler_init(ChartHandler * handler, void * api);

/**
 * @brief Get the current offset of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to get the offset from
 *
 * @return float The offset in mV
 */
float chart_handler_get_offset(ChartHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Set the offset in mV of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @ch The channel to modify
 * @param The value of the offset in mV
 */
void chart_handler_set_offset(ChartHandler * handler, ChartHandlerChannel ch, float value);


/**
 * @brief Get the current scale of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to get the scale from
 *
 * @return float The scale in mV
 */
float chart_handler_get_scale(ChartHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Set the scale in mV of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @ch The channel to modify
 * @param The value of the scale in mV
 */
void chart_handler_set_scale(ChartHandler * handler, ChartHandlerChannel ch, float value);

/**
 * @brief Add a point in the chart on the selected channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to add the point to
 * @param value The converted value in mV
 */
void chart_handler_add_point(ChartHandler * handler, ChartHandlerChannel ch, float value);

/**
 * @brief Chart handler routine that updates all the values
 *
 * @param handler A pointer to the chart handler structure
 */
void chart_handler_routine(ChartHandler * handler);

/**
 * @brief Invalidate all the chart data of a single channel resetting all its values to 0
 *
 * @details Only the data is reset, the scales and offset remain the same and are not changed
 *
 * @param handler A pointer to the chart handler structure
 */
void chart_handler_invalidate(ChartHandler * handler, ChartHandlerChannel ch);

#endif  // CHART_HANDLER_H

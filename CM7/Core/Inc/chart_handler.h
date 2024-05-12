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

/** @brief Number of values required to fill a single division of the chart */
#define CHART_HANDLER_VALUES_PER_DIVISION (10U)

/** @brief Maximum number of raw samples that the chart handler can handler */
#define CHART_HANDLER_VALUES_COUNT (CHART_X_DIVISION_COUNT * CHART_HANDLER_VALUES_PER_DIVISION)

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

    float x_offset[CHART_HANDLER_CHANNEL_COUNT]; // in us
    float offset[CHART_HANDLER_CHANNEL_COUNT]; // in mV
 
    bool enabled[CHART_HANDLER_CHANNEL_COUNT];
    bool ready[CHART_HANDLER_CHANNEL_COUNT];
    size_t index[CHART_HANDLER_CHANNEL_COUNT];
    uint16_t raw[CHART_HANDLER_CHANNEL_COUNT][CHART_HANDLER_VALUES_COUNT];
    float data[CHART_HANDLER_CHANNEL_COUNT][CHART_HANDLER_VALUES_COUNT];
} ChartHandler;

/**
 * @brief Initialize the chart handler
 * 
 * @param handler A pointer to the chart handler structure
 * @param api A pointer to an LVGL api handler structure
 */
void chart_handler_init(ChartHandler * handler, void * api);

/**
 * @brief Enable or disable a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to enable/disable
 * @param enabled True to enable, false to disable
 */
void chart_handler_set_enable(ChartHandler * handler, ChartHandlerChannel ch, bool enabled);

/**
 * @brief Toggle the enabled state of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to enable/disable
 */
void chart_handler_toggle_enable(ChartHandler * handler, ChartHandlerChannel ch);

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
 * @brief Get the current time scale of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to get the scale from
 *
 * @return float The scale in us
 */
float chart_handler_get_x_scale(ChartHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Set the time scale in us of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @ch The channel to modify
 * @param The value of the scale in us
 */
void chart_handler_set_x_scale(ChartHandler * handler, ChartHandlerChannel ch, float value);

/**
 * @brief Update the chart handler values
 *
 * @param handler A pointer to the chart handler structure
 * @param t The amount of time taken by the ADC to make the sampling and conversion in us
 */
void chart_handler_update(ChartHandler * handler, uint32_t t);

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

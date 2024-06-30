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

/**
 * @brief Definition to the chart handler structure
 *
 * @details The api is a void pointer to avoid problems of circular dependency
 * @details When a channel is not enabled it is not displayed
 * @details When a channel is not running the signal is not updated
 * @details The stop request is required to avoid partial updates when stopping the signal from updating
 *
 * @param api A pointer to the LVGL api
 * @param x_scale The temporal scale per division in us
 * @param scale The voltage scale per division in mV
 * @param x_offset The temporal offset in us
 * @param offset The voltage offset in mV
 * @param x_offset_paused the temporal offset when the channel was paused in us
 * @param x_scale_paused The temporal scale when the channel was in mV
 * @param enabled Flag to enable/disable the channel
 * @param stop_request Flag used to stop the channel update (see details above)
 * @param running Flag to run/stop the channel from updating
 * @param ready Flag set to true when all the data is ready to be displayed
 * @param index The current index inside the raw data
 * @param raw The raw ADC data
 * @param data The converted with scale and offset applyed ready to be displayed
 */
typedef struct {
    void * api;

    // Settings
    float x_scale[CHART_HANDLER_CHANNEL_COUNT]; // in us
    float scale[CHART_HANDLER_CHANNEL_COUNT]; // in mV

    float x_offset[CHART_HANDLER_CHANNEL_COUNT]; // in us
    float offset[CHART_HANDLER_CHANNEL_COUNT]; // in mV

    float x_offset_paused[CHART_HANDLER_CHANNEL_COUNT]; // in us
    float x_scale_paused[CHART_HANDLER_CHANNEL_COUNT]; // in us

    // Trigger
    uint16_t trigger[CHART_HANDLER_CHANNEL_COUNT];
    bool ascending_trigger, descending_trigger;

    // Index of the raw value that crossed the trigger
    int32_t trigger_index[CHART_HANDLER_CHANNEL_COUNT];

    // Number of values before and after the trigger
    size_t trigger_before_count[CHART_HANDLER_CHANNEL_COUNT];
    size_t trigger_after_count[CHART_HANDLER_CHANNEL_COUNT];
 
    // Channels
    bool enabled[CHART_HANDLER_CHANNEL_COUNT];
    bool stop_request[CHART_HANDLER_CHANNEL_COUNT];
    bool running[CHART_HANDLER_CHANNEL_COUNT];
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
 * @brief Check if the chart handler channel is enabled
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to check
 *
 * @return bool True if the channel is enabled false otherwise
 */
bool chart_handler_is_enabled(ChartHandler * handler, ChartHandlerChannel ch);

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
 * @brief Check if the chart handler channel plot is updating
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to check
 *
 * @return bool True if the channel plot is updating false otherwise
 */
bool chart_handler_is_running(ChartHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Start or stop a single channel from updating
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to enable/disable
 * @param enabled True to start the update, false to stop
 */
void chart_handler_set_running(ChartHandler * handler, ChartHandlerChannel ch, bool running);

/**
 * @brief Toggle the running state of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to update
 */
void chart_handler_toggle_running(ChartHandler * handler, ChartHandlerChannel ch);

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
 * @brief Get the current time offset of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to get the offset from
 *
 * @return float The offset in us
 */
float chart_handler_get_x_offset(ChartHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Set the time offset in us of a single channel
 *
 * @param handler A pointer to the chart handler structure
 * @ch The channel to modify
 * @param The value of the offset in us
 */
void chart_handler_set_x_offset(ChartHandler * handler, ChartHandlerChannel ch, float value);

/**
 * @brief Convert a voltage in millivot to grid units (i.e. the divisions of the grid)
 *
 * @param handler A pointer to the chart handler structure
 * @param ch The channel to select
 * @param value The value to convert in millivolt
 *
 * @return float The converted value in grid units
 */
float chart_handler_voltage_to_grid_units(ChartHandler * handler, ChartHandlerChannel ch, float value);

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

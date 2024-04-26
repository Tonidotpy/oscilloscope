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

#define CHART_HANDLER_SAMPLE_COUNT (100U)

#define CHART_HANDLER_MIN_X_SCALE (1000.0f) // in us
#define CHART_HANDLER_MAX_X_SCALE (100000.0f) // in us

#define CHART_HANDLER_MIN_Y_SCALE (10.0f) // in mV
#define CHART_HANDLER_MAX_Y_SCALE (10000.0f) // in mV

/** @brief Available channels of the oscilloscope */
typedef enum {
    CHART_HANDLER_CHANNEL_1,
    CHART_HANDLER_CHANNEL_2,
    CHART_HANDLER_CHANNEL_COUNT
} ChartHandlerChannel;

typedef struct {
    void * api;

    float x_scale; // in us
    float scale[CHART_HANDLER_CHANNEL_COUNT]; // in mV

    float offset[CHART_HANDLER_CHANNEL_COUNT]; // in mV
 
    bool ready[CHART_HANDLER_CHANNEL_COUNT];
    size_t index[CHART_HANDLER_CHANNEL_COUNT];
    uint16_t raw[CHART_HANDLER_CHANNEL_COUNT][CHART_HANDLER_SAMPLE_COUNT];
    float data[CHART_HANDLER_CHANNEL_COUNT][CHART_HANDLER_SAMPLE_COUNT];
} ChartHandler;

/**
 * @brief Initialize the chart handler
 * 
 * @param handler A pointer to the char handler structure
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
 * @param handler A pointer to the char handler structure
 * @param ch The channel to add the point to
 * @param value The converted value in mV
 */
void chart_handler_add_point(ChartHandler * handler, ChartHandlerChannel ch, float value);

/**
 * @brief Chart handler routine that updates all the values
 *
 * @param handler A pointer to the char handler structure
 */
void chart_handler_routine(ChartHandler * handler);

#endif  // CHART_HANDLER_H

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

#define CHART_HANDLER_MIN_X_SCALE (1000U) // in us
#define CHART_HANDLER_MAX_X_SCALE (100000U) // in us

#define CHART_HANDLER_MIN_Y_SCALE (10U) // in mV
#define CHART_HANDLER_MAX_Y_SCALE (10000U) // in mV

/** @brief Available channels of the oscilloscope */
typedef enum {
    CHART_HANDLER_CHANNEL_1,
    CHART_HANDLER_CHANNEL_2,
    CHART_HANDLER_CHANNEL_COUNT
} ChartHandlerChannel;

typedef struct {
    void * api;

    uint32_t x_scale; // in us
    uint32_t scale[CHART_HANDLER_CHANNEL_COUNT]; // in mV

    int32_t offset[CHART_HANDLER_CHANNEL_COUNT]; // in mV
 
    size_t index[CHART_HANDLER_CHANNEL_COUNT];
    int32_t data[CHART_HANDLER_CHANNEL_COUNT][CHART_HANDLER_SAMPLE_COUNT];
} ChartHandler;

/**
 * @brief Initialize the chart handler
 * 
 * @param handler The chart handler structure
 * @param api A pointer to an LVGL api handler structure
 */
void chart_handler_init(ChartHandler * handler, void * api);

/**
 * @brief Add a point in the chart on the selected channel
 *
 * @param handler The chart handler structure
 * @param ch The channel to add the point to
 * @param value The Y value of the point
 */
void chart_handler_add_point(ChartHandler * handler, ChartHandlerChannel ch, uint16_t value);

#endif  // CHART_HANDLER_H

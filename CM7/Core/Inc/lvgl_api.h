/**
 * @file lvgl_api.h
 * @brief API used to show the desired output to the sceen
 * passing by the LVGL library
 *
 * @date Apr 9, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 * @author Enrico Dal Bianco [enrico.dalbianco@studenti.unitn.it]
 */

#ifndef LVGL_API_H
#define LVGL_API_H

#include <stddef.h>
#include <stdio.h>

#include "lvgl.h"
#include "touch_screen.h"
#include "chart_handler.h"
#include "config.h"

typedef struct {
    lv_theme_t theme;
    lv_display_t * display;
    lv_indev_t * touch_screen;

    lv_obj_t * header;
    lv_obj_t * div_time;
    lv_obj_t * div_volt;

    lv_obj_t * chart;
    lv_chart_series_t * series[CHART_HANDLER_CHANNEL_COUNT];

    int32_t channels[CHART_HANDLER_CHANNEL_COUNT][CHART_POINT_COUNT];
    ChartHandler chart_handler;
} LvHandler;

/**
 * @brief Initialize the LVGL internal library and register the
 * I/O devices
 *
 * @param handler The LVGL handler structure
 * @param screen_width The width of the screen
 * @param screen_height The height of the screen
 * @param frame_buffer_1 The first frame buffer address
 * @param frame_buffer_2 The second frame buffer address (can be NULL)
 * @param frame_buffer_size The lenght of the buffers in bytes
 */
void lv_api_init(
    LvHandler * handler,
    size_t screen_width,
    size_t screen_height,
    void * frame_buffer_1,
    void * frame_buffer_2,
    size_t frame_buffer_size
);

/**
 * @brief Update the text that display the time and voltage per division in the header
 *
 * @param handler A ponter to the LVGL handler structure
 * @param div_time The time per division value
 * @param div_volt The volt per division value
 */
void lv_api_update_div_text(LvHandler * handler, float div_time, float div_volt);

/**
 * @brief Update the current status of the touch screen
 * @attention This function does not work with more than one touch screen device
 *
 * @param state A pointer to a touch screen state structure
 */
void lv_api_update_ts_status(TsInfo * info);

/**
 * @brief Run the internal logic of LVGL
 * @details This function should be called as often as possible
 *
 * @param handler The LVGL handler structure
 */
void lv_api_run(LvHandler * handler);

/**
 * @brief Clear the channel data to avoid plotting unwanted values
 *
 * @param handler A pointer to the LVGL handler structure
 * @param ch The channel to modify
 */
void lv_api_clear_channel_data(LvHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Update all the points of the chart
 *
 * @param handler A pointer to the LVGL handler structure
 * @param ch The channel to update the point to
 * @param values The array of new values in grid units
 * @param size The lenght of the array
 */
void lv_api_update_points(
    LvHandler * handler,
    ChartHandlerChannel ch,
    float * values,
    size_t size
);

/**
 * @brief Update all the point of the chart on the display
 *
 * @param handler The LVGL handler structure
 */
void lv_api_refresh_chart(LvHandler * handler);


#endif  // LVGL_API_H

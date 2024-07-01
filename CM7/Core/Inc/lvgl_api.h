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
#include <stdint.h>

#include "lvgl.h"
#include "touch_screen.h"
#include "chart_handler.h"
#include "config.h"
#include "waves.h"

typedef struct {
    lv_theme_t theme;
    lv_display_t * display;
    lv_indev_t * touch_screen;

    // Header
    lv_obj_t * header;
    lv_obj_t * div_time;
    lv_obj_t * div_volt;
    bool div_update;

    // Menu
    lv_obj_t * menu;

    // Chart
    lv_obj_t * chart;
    lv_chart_series_t * series[CHART_HANDLER_CHANNEL_COUNT];

    // Trigger
    lv_point_precise_t trigger_points[CHART_HANDLER_CHANNEL_COUNT][2];
    lv_obj_t * trigger_line[CHART_HANDLER_CHANNEL_COUNT];
    bool trigger_update[CHART_HANDLER_CHANNEL_COUNT];
    lv_obj_t * trigger_checkbox_asc;
    lv_obj_t * trigger_checkbox_desc;

    // Loading bar
    lv_obj_t * loading_bar;
    size_t loading_bar_value;
    bool loading_bar_hide;

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
 * @brief Convert a value in grid units to chart space
 *
 * @details The chart space coordinate is a number from 0 to the height of the chart
 *
 * @param ch The selected channel
 * @param value The value in grid units to convert 
 *
 * @return float The converted value in chart space
 */
float lv_api_grid_units_to_chart(ChartHandlerChannel ch, float value);

/**
 * @brief Convert a value in grid units to screen space
 *
 * @param ch The selected channel
 * @param value The value in grid units to convert
 *
 * @return float The converted value in screen space
 */
float lv_api_grid_units_to_screen(ChartHandlerChannel ch, float value);

/**
 * @brief Update the text that display the time and voltage per division in the header
 *
 * @param handler A ponter to the LVGL handler structure
 */
void lv_api_update_div_text(LvHandler * handler);

/**
 * @brief Update the current status of the touch screen
 * @attention This function does not work with more than one touch screen device
 *
 * @param state A pointer to a touch screen state structure
 */
void lv_api_update_ts_status(TsInfo * info);

/** @brief Hide the trigger line */
void lv_api_hide_trigger_line(LvHandler * handler, ChartHandlerChannel ch);

/**
 * @brief Update the trigger line position on the screen
 *
 * @details If the line is hidden it shows automatically if inside the screen bounds
 *
 * @param handler The LVGL handler structure
 * @param ch The channel to select
 * @param volt The height of the trigger (in millivolt)
 */
void lv_api_update_trigger_line(LvHandler * handler, ChartHandlerChannel ch, float volt);

/**
 * @brief Hide the loading bar
 *
 * @param handler A pointer to the LVGL handler structure
 */
void lv_api_hide_loading_bar(LvHandler * handler);

/**
 * @brief Update the loading bar value
 *
 * @param handler A pointer to the LVGL handler structure
 * @param value The new value to set
 */
void lv_api_update_loading_bar(LvHandler * handler, size_t value);

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

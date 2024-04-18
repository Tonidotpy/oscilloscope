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

#include "lvgl.h"
#include "touch_screen.h"

/** @brief Horizontal and vertical line count for the chart */
#define LV_API_CHART_HOR_LINE_COUNT 10
#define LV_API_CHART_VER_LINE_COUNT 16

/** @brief Total number of points of the chart */
#define LV_API_CHART_POINT_COUNT 166


typedef struct {
    lv_theme_t theme;
    lv_display_t * display;
    lv_indev_t * touch_screen;

    lv_obj_t * chart;
    lv_chart_series_t * main_series;
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
 * @brief Update the current status of the touch screen
 * @attention This function does not work with more than one touch screen device
 *
 * @param state A pointer to a touch screen state structure
 */
void lv_api_update_ts_status(TsInfo * info);

/**
 * @brief Run the internal logic of LVGL
 * @details This function should be called as often as possible
 */
void lv_api_run(void);

/**
 * @brief Add a new point at the end of the main series
 *
 * @param handler The LVGL structure handler
 * @param value The Y value of the point to add
*/
void lv_api_draw_point(LvHandler * handler, int32_t value);


#endif  // LVGL_API_H

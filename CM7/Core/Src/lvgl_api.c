/**
 * @file lvgl_api.c
 * @brief API used to show the desired output to the sceen
 * passing by the LVGL library
 *
 * @date Apr 9, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 * @author Enrico Dal Bianco [enrico.dalbianco@studenti.unitn.it]
 */

#include "lvgl_api.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "chart_handler.h"
#include "config.h"
#include "lvgl.h"
#include "lvgl_colors.h"
#include "stm32h7xx_hal_ltdc.h"

#define LERP(A, B, T) ((A) * (1.0 - T) + (B) * (T))

extern LTDC_HandleTypeDef hltdc;

// Master touch screen status
static TsInfo ts_info;

/**
 * @brief Lvgl callback used by the library that gets called after the rendering has finished
 * and the content has to be displayed on the screen
 *
 * @param display A pointer to the lvgl display object
 * @param area The area coordinates that should be redraw
 * @param px_map The array of pixels to draw
 */
static void _lv_flush_callback(lv_display_t * display, const lv_area_t * area, uint8_t * px_map) {
    LTDC_LayerCfgTypeDef pLayerCfg = {
        .WindowX0 = 0,
        .WindowX1 = 800,
        .WindowY0 = 0,
        .WindowY1 = 480,
        .PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888,
        .Alpha = 255,
        .Alpha0 = 0,
        .BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA,
        .BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA,
        .FBStartAdress = (uint32_t)px_map,
        .ImageWidth = 800,
        .ImageHeight = 480,
        .Backcolor.Blue = 0,
        .Backcolor.Green = 0,
        .Backcolor.Red = 0
    };
    HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0);
    lv_display_flush_ready(display);
}
/**
 * @brief Apply all the custom styles to the theme
 *
 * @param th The current theme
 * @param obj The object to apply the style to
 */
static void _lv_apply_theme(lv_theme_t * th, lv_obj_t * obj) {
    LV_UNUSED(th);

    if (lv_obj_check_type(obj, &lv_chart_class)) {
        static lv_style_t chart_main_style;
        lv_style_init(&chart_main_style);
        lv_style_set_bg_color(&chart_main_style, LV_BLACK);
        lv_style_set_line_color(&chart_main_style, LV_WHITE);
        lv_style_set_line_opa(&chart_main_style, LV_OPA_20);
        lv_obj_add_style(obj, &chart_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_label_class)) {
        static lv_style_t label_main_style;
        lv_style_init(&label_main_style);
        lv_style_set_text_color(&label_main_style, LV_BLACK);
        lv_obj_add_style(obj, &label_main_style, LV_PART_MAIN);
    }
}
/**
 * @brief Update the status of the lvgl touch screen input device
 *
 * @param indev A pointer to the input device
 * @param data A pointer to the data to update
 */
static void _lv_update_ts_indev_callback(lv_indev_t * touch_screen, lv_indev_data_t * data) {
    if (ts_get_state() == TS_DISABLED)
        return;
    if (!ts_info.detected)
        return;

    lv_display_t * display = lv_indev_get_display(touch_screen);

    data->point.x = ts_info.x;
    data->point.y = lv_display_get_vertical_resolution(display) - ts_info.y;
    data->state = ts_info.detected ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    ts_get_info(&ts_info);
}
void _lv_api_header_init(LvHandler * handler) {
    lv_obj_t * screen = lv_display_get_screen_active(handler->display);
    size_t w = lv_display_get_horizontal_resolution(handler->display);

    handler->header = lv_obj_create(screen);
    lv_obj_set_size(handler->header, w, HEADER_SIZE);
    lv_obj_align(handler->header, LV_ALIGN_TOP_MID, 0, 0);

    // Create labels for the header
    handler->div_time = lv_label_create(handler->header);
    lv_label_set_long_mode(handler->div_time, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(handler->div_time, LV_ALIGN_LEFT_MID, 10, 0);

    handler->div_volt = lv_label_create(handler->header);
    lv_label_set_long_mode(handler->div_volt, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_align(handler->div_volt, LV_ALIGN_RIGHT_MID, -10, 0);

    // Update label text
    lv_api_update_div_text(handler);
}
/**
 * @brief Initialize the chart visualization for the oscilloscope
 */
void _lv_api_chart_init(LvHandler * handler) {
    lv_obj_t * screen = lv_display_get_screen_active(handler->display);
    size_t w = lv_display_get_horizontal_resolution(handler->display);
    size_t h = lv_display_get_vertical_resolution(handler->display);

    // Setup chart
    handler->chart = lv_chart_create(screen);
    lv_chart_set_type(handler->chart, LV_CHART_TYPE_LINE);
    lv_obj_set_size(handler->chart, w, h - HEADER_SIZE);
    lv_obj_align(handler->chart, LV_ALIGN_BOTTOM_MID, 0, 0);
    // lv_obj_center(handler->chart);

    // Set point and line count
    lv_chart_set_div_line_count(handler->chart, CHART_HORIZONTAL_LINE_COUNT, CHART_VERTICAL_LINE_COUNT);
    lv_chart_set_point_count(handler->chart, CHART_POINT_COUNT);

    // Set range values
    lv_chart_set_range(handler->chart, LV_CHART_AXIS_PRIMARY_Y, 0, CHART_AXIS_PRIMARY_Y_MAX_COORD);
    lv_chart_set_range(handler->chart, LV_CHART_AXIS_SECONDARY_Y, 0, CHART_AXIS_SECONDARY_Y_MAX_COORD);

    // Add series of points
    handler->series[CHART_HANDLER_CHANNEL_1] = lv_chart_add_series(handler->chart, LV_YELLOW, LV_CHART_AXIS_PRIMARY_Y);
    handler->series[CHART_HANDLER_CHANNEL_2] = lv_chart_add_series(handler->chart, LV_PURPLE, LV_CHART_AXIS_SECONDARY_Y);

    for (size_t i = 0; i < CHART_HANDLER_CHANNEL_COUNT; ++i)
        lv_chart_set_ext_y_array(handler->chart, handler->series[i], handler->channels[i]);
}

void _lv_api_chart_handler_init(LvHandler * handler) {
    chart_handler_init(&handler->chart_handler, handler);
}

void _lv_api_div_set_text(lv_obj_t * label, const char * fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char msg[HEADER_LABEL_STRING_SIZE] = { 0 };
    // Update time division label
    vsnprintf(
        msg,
        HEADER_LABEL_STRING_SIZE - 1U,
        fmt,
        args
    );
    lv_label_set_text(label, msg);

    va_end(args);
}

void lv_api_init(
    LvHandler * handler,
    size_t screen_width,
    size_t screen_height,
    void * frame_buffer_1,
    void * frame_buffer_2,
    size_t frame_buffer_size)
{
    if (handler == NULL)
        return;
    memset(handler, 0U, sizeof(LvHandler));

    // Set channels data to 0
    for (size_t i = 0; i < CHART_HANDLER_CHANNEL_COUNT; ++i)
        memset(handler->channels[i], LV_CHART_POINT_NONE, CHART_POINT_COUNT * sizeof(int32_t));

    // Init LVGL
    lv_init();

    // Create the display
    handler->display = lv_display_create(screen_width, screen_height);
    lv_display_set_buffers(
      handler->display,
      frame_buffer_1,
      frame_buffer_2,
      frame_buffer_size,
      LV_DISPLAY_RENDER_MODE_DIRECT
    );
    lv_display_set_flush_cb(handler->display, _lv_flush_callback);

    // Register touch screen as an input device
    handler->touch_screen = lv_indev_create();
    lv_indev_set_type(handler->touch_screen, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(handler->touch_screen, _lv_update_ts_indev_callback);

    // Initialize the theme and styles
    static lv_style_t main_style;
    lv_style_init(&main_style);
    lv_style_set_bg_color(&main_style, LV_BLACK);
    lv_obj_add_style(lv_display_get_screen_active(handler->display), &main_style, LV_PART_MAIN);

    lv_theme_t * simple_theme = lv_display_get_theme(handler->display);
    handler->theme = *simple_theme;
    lv_theme_set_parent(&handler->theme, simple_theme);
    lv_theme_set_apply_cb(&handler->theme, _lv_apply_theme);
    lv_display_set_theme(handler->display, &handler->theme); 

    // Initialize oscilloscope chart
    _lv_api_chart_init(handler);
    _lv_api_chart_handler_init(handler);
    _lv_api_header_init(handler);
}

void lv_api_update_div_text(LvHandler * handler) {
    if (handler == NULL)
        return;
    handler->div_update = true;
}

void lv_api_update_ts_status(TsInfo * info) {
    if (info == NULL)
        return; 
    memcpy(&ts_info, info, sizeof(ts_info));
}

void lv_api_run(LvHandler * handler) {
    if (handler == NULL)
        return;

    chart_handler_routine(&handler->chart_handler);

    // Update LVGL internal status
    if (handler->div_update) {
        _lv_api_div_set_text(
            handler->div_time,
            "%.0f us",
            chart_handler_get_x_scale(&handler->chart_handler, CHART_HANDLER_CHANNEL_1)
        );
        _lv_api_div_set_text(
            handler->div_volt,
            "%.0f mV",
            chart_handler_get_scale(&handler->chart_handler, CHART_HANDLER_CHANNEL_1)
        );

        handler->div_update = false;
    }

    lv_timer_handler_run_in_period(5);
}

void lv_api_clear_channel_data(LvHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;
    memset(handler->channels[ch], LV_CHART_POINT_NONE, CHART_POINT_COUNT * sizeof(int32_t));
}

// TODO: Check
void lv_api_update_points(
    LvHandler * handler,
    ChartHandlerChannel ch,
    float * values,
    size_t size)
{
    if (handler == NULL || values == NULL)
        return;

    const float div[CHART_HANDLER_CHANNEL_COUNT] = {
        CHART_AXIS_PRIMARY_Y_MAX_COORD / (float)(CHART_Y_DIVISION_COUNT),
        CHART_AXIS_SECONDARY_Y_MAX_COORD / (float)(CHART_Y_DIVISION_COUNT)
    };
    const float dt = size / (float)CHART_POINT_COUNT;
    const size_t step = dt == 0.f ? 1U : dt;
    
    size_t j = 0;
    float t = 0;
    for (size_t x = 0; x < CHART_POINT_COUNT; ++x) {
        // Interpolate
        size_t k = j >= (CHART_HANDLER_VALUES_COUNT - 1) ? (CHART_HANDLER_VALUES_COUNT - 1) : (j + step);
        float val = values[j]; // LERP(values[j], values[k], t);
        if (val == NAN)
            val = LV_CHART_POINT_NONE;
        else {
            // Convert to screen space
            val *= div[ch];
        }

        // Copy value
        handler->channels[ch][x] = val;
        t += dt;
        if (t >= 1.0f) {
            j += t;
            t = 0.f;
        }
    }

    lv_chart_refresh(handler->chart);
}

void lv_api_refresh_chart(LvHandler * handler) {
    if (handler == NULL)
        return;
    lv_chart_refresh(handler->chart);
}

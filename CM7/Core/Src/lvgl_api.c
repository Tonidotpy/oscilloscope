/**
 * @file lvgl_api.c
 * @brief API used to show the desired output to the sceen
 * passing by the LVGL library
 *
 * @date Apr 9, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "lvgl_api.h"

#include <string.h>

#include "lvgl.h"
#include "lvgl_colors.h"
#include "stm32h7xx_hal_ltdc.h"
#include "touch_screen.h"

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
static lv_obj_t * chart;
static lv_chart_series_t * main_series;
static void _lv_apply_theme(lv_theme_t * th, lv_obj_t * obj) {
    LV_UNUSED(th);

    // static lv_style_t main_style;
    // lv_style_init(&main_style);
    // lv_style_set_bg_color(&main_style, LV_BLACK);
    // lv_obj_add_style(obj, &main_style, LV_PART_MAIN);

    if (lv_obj_check_type(obj, &lv_chart_class)) {
        static lv_style_t chart_main_style;
        lv_style_init(&chart_main_style);
        lv_style_set_bg_color(&chart_main_style, LV_BLACK);
        lv_style_set_line_color(&chart_main_style, LV_WHITE);
        lv_style_set_line_opa(&chart_main_style, LV_OPA_20);
        lv_obj_add_style(obj, &chart_main_style, LV_PART_MAIN);
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
/**
 * @brief Initialize the chart visualization for the oscilloscope
 */
void _lv_init_chart(LvHandler * handler) {
    lv_obj_t * screen = lv_display_get_screen_active(handler->display);
    size_t w = lv_display_get_horizontal_resolution(handler->display);
    size_t h = lv_display_get_vertical_resolution(handler->display);

    chart = lv_chart_create(screen);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_obj_set_size(chart, w, h);
    lv_obj_center(chart);

    lv_chart_set_div_line_count(chart, 10, 16);
    lv_chart_set_point_count(chart, 166);

    main_series = lv_chart_add_series(chart, LV_RED, LV_CHART_AXIS_PRIMARY_Y);

    // TODO: Remove
    lv_chart_series_t * series = lv_chart_add_series(chart, LV_YELLOW, LV_CHART_AXIS_PRIMARY_Y);
    for (size_t i = 0; i < 10; i++)
        /*Set the next points on 'ser1'*/
        lv_chart_set_next_value(chart, series, lv_rand(10, 50));
    lv_chart_refresh(chart);
}

void lv_api_draw_point(LvHandler * handler, int32_t value) {
    lv_chart_set_next_value(chart, main_series, value);
    lv_chart_refresh(chart);
}

void lv_api_init(
    LvHandler * handler,
    size_t screen_width,
    size_t screen_height,
    void * frame_buffer_1,
    void * frame_buffer_2,
    size_t frame_buffer_size)
{
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
    lv_theme_t * simple_theme = lv_display_get_theme(handler->display);
    handler->theme = *simple_theme;
    lv_theme_set_parent(&handler->theme, simple_theme);
    lv_theme_set_apply_cb(&handler->theme, _lv_apply_theme);
    lv_display_set_theme(handler->display, &handler->theme); 

    // Initialize oscilloscope chart
    _lv_init_chart(handler);
}

void lv_api_update_ts_status(TsInfo * info) {
    if (info == NULL)
        return; 
    memcpy(&ts_info, info, sizeof(ts_info));
}

void lv_api_run(void) {
    // Update LVGL internal status
    lv_timer_handler_run_in_period(5);
}


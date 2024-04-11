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
#include "stm32h7xx_hal_ltdc.h"

extern LTDC_HandleTypeDef hltdc;

// Master touch screen status
static TsState ts_state;

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
 * @brief Update the status of the lvgl touch screen input device
 *
 * @param indev A pointer to the input device
 * @param data A pointer to the data to update
 */
static void _lv_update_ts_indev_callback(lv_indev_t * touch_screen, lv_indev_data_t * data) {
    if (!ts_state.detected)
        return;

    lv_display_t * display = lv_indev_get_display(touch_screen);

    data->point.x = ts_state.x;
    data->point.y = lv_display_get_vertical_resolution(display) - ts_state.y;
    data->state = ts_state.detected ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
    ts_get_state(&ts_state);
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
}

void lv_api_update_ts_status(TsState * state) {
    if (state == NULL)
        return; 
    memcpy(&ts_state, state, sizeof(ts_state));
}

void lv_api_run(void) {
    // Update LVGL internal status
    lv_timer_handler_run_in_period(5);
}


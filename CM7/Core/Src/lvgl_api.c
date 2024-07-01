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

struct _shared {
	uint32_t generator_index;
};
volatile struct _shared * const shared_data = (struct _shared *)0x38001000;

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
        // lv_style_set_text_color(&label_main_style, LV_BLACK);
        lv_obj_add_style(obj, &label_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_menu_class)) {
        static lv_style_t menu_main_style;
        lv_style_init(&menu_main_style);
        lv_style_set_bg_color(&menu_main_style, LV_WHITE);

        // Add shadow
        // lv_style_set_shadow_width(&menu_main_style, 7);
        // lv_style_set_shadow_color(&menu_main_style, LV_BLACK);

        lv_obj_add_style(obj, &menu_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_button_class)) {
        static lv_style_t button_main_style;
        static lv_style_t tabview_button_main_style;
        static lv_style_t tabview_button_checked_style;

        // Button pressed transition
        static lv_style_transition_dsc_t button_pressed_transition;
        const static lv_style_prop_t button_pressed_props[] = {
            LV_STYLE_BG_OPA, LV_STYLE_BG_COLOR,
            0
        };
        lv_style_transition_dsc_init(
            &button_pressed_transition,
            button_pressed_props,
            lv_anim_path_linear,
            200U,
            0U,
            NULL
        );

        /*
         * Change tabview button appearance
         *
         * Apparently the parent of the button in a tabview is not the tabview
         * itself but it is a bar (or another component) which is a child of the
         * actual tabview
         */
        lv_obj_t * bar = lv_obj_get_parent(obj);
        lv_obj_t * tabview = lv_obj_get_parent(bar);
        if (lv_obj_check_type(tabview, &lv_tabview_class)) {
            // Main button style
            lv_style_init(&tabview_button_main_style);
            lv_style_set_bg_color(&tabview_button_main_style, lv_color_lighten(LV_BLACK, 8));
            lv_style_set_text_color(&tabview_button_main_style, LV_WHITE);
            lv_obj_add_style(obj, &tabview_button_main_style, LV_PART_MAIN);

            // Checked button style
            lv_style_init(&tabview_button_checked_style);
            lv_style_set_bg_color(&tabview_button_checked_style, lv_color_lighten(LV_BLACK, 18));
            lv_obj_add_style(obj, &tabview_button_checked_style, LV_PART_MAIN | LV_STATE_CHECKED);

            // Style when pressed
            static lv_style_t button_pressed_style;
            lv_style_init(&button_pressed_style);
            lv_style_set_bg_color(&button_pressed_style, LV_BLACK);
            // lv_style_set_transition(&button_pressed_style, &button_pressed_transition);
            lv_obj_add_style(obj, &button_pressed_style, LV_STATE_PRESSED);
        }
        else {
            // Main button style
            lv_style_init(&button_main_style);
            lv_style_set_bg_color(&button_main_style, LV_WHITE);
            lv_style_set_pad_hor(&button_main_style, 15);
            // lv_style_set_transition(&button_main_style, &button_pressed_transition);
            lv_obj_add_style(obj, &button_main_style, LV_PART_MAIN);

            // Style when pressed
            static lv_style_t button_pressed_style;
            lv_style_init(&button_pressed_style);
            lv_style_set_bg_color(&button_pressed_style, LV_LIGHT_GRAY);
            // lv_style_set_transition(&button_pressed_style, &button_pressed_transition);
            lv_obj_add_style(obj, &button_pressed_style, LV_STATE_PRESSED);
        }
    }
    else if (lv_obj_check_type(obj, &lv_tabview_class)) {
        static lv_style_t tabview_main_style;
        lv_style_init(&tabview_main_style); 
        lv_style_set_bg_color(&tabview_main_style, LV_BLACK);
        lv_obj_add_style(obj, &tabview_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_checkbox_class)) {
        static lv_style_t checkbox_main_style;
        lv_style_init(&checkbox_main_style); 
        lv_style_set_bg_color(&checkbox_main_style, LV_BLACK);
        lv_style_set_text_color(&checkbox_main_style, LV_WHITE);

        // Space between checkbox and label
        lv_style_set_pad_column(&checkbox_main_style, 10U);

        // Margin
        lv_style_set_margin_top(&checkbox_main_style, 12U);
        lv_style_set_margin_bottom(&checkbox_main_style, 12U);
        lv_style_set_margin_left(&checkbox_main_style, 8U);
        lv_style_set_margin_right(&checkbox_main_style, 8U);

        lv_obj_add_style(obj, &checkbox_main_style, LV_PART_MAIN);

        static lv_style_t tabview_tickbox_style;
        lv_style_init(&tabview_tickbox_style);
        lv_style_set_text_font(&tabview_tickbox_style, LV_FONT_DEFAULT);
        lv_style_set_pad_all(&tabview_tickbox_style, 5U);
        lv_style_set_bg_color(&tabview_tickbox_style, LV_WHITE);
        lv_obj_add_style(obj, &tabview_tickbox_style, LV_PART_INDICATOR);

        static lv_style_t tabview_tickbox_checked_style;
        lv_style_init(&tabview_tickbox_checked_style);
        lv_style_set_bg_color(&tabview_tickbox_checked_style, LV_YELLOW);
        lv_obj_add_style(obj, &tabview_tickbox_checked_style, LV_PART_INDICATOR | LV_STATE_CHECKED);
    }
    else if (lv_obj_check_type(obj, &lv_list_class)) {
        static lv_style_t list_main_style;
        lv_style_init(&list_main_style);
        lv_style_set_bg_color(&list_main_style, LV_BLACK);
        lv_style_set_pad_all(&list_main_style, 20U);
        lv_obj_add_style(obj, &list_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_line_class)) {
        static lv_style_t line_main_style;
        lv_style_init(&line_main_style);
        lv_style_set_line_width(&line_main_style, 2U);
        lv_style_set_line_color(&line_main_style, LV_RED);
        lv_obj_add_style(obj, &line_main_style, LV_PART_MAIN);
    }
    else if (lv_obj_check_type(obj, &lv_bar_class)) { 
        static lv_style_t bar_main_style;
        lv_style_init(&bar_main_style);
        lv_style_set_bg_color(&bar_main_style, LV_LIGHT_GRAY);
        lv_obj_add_style(obj, &bar_main_style, LV_PART_MAIN);

        static lv_style_t bar_indicator_style;
        lv_style_init(&bar_indicator_style);
        lv_style_set_bg_color(&bar_indicator_style, LV_RED);
        lv_obj_add_style(obj, &bar_indicator_style, LV_PART_INDICATOR);
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

static void _lv_api_menu_btn_event_handler(lv_event_t * e) {
    LvHandler * handler = (LvHandler *)lv_event_get_user_data(e);
    if(lv_obj_has_flag(handler->menu, LV_OBJ_FLAG_HIDDEN))
        lv_obj_clear_flag(handler->menu, LV_OBJ_FLAG_HIDDEN);
    else
        lv_obj_add_flag(handler->menu, LV_OBJ_FLAG_HIDDEN);
}

static void _lv_api_trigger_checkbox_handler_asc(lv_event_t * e) {
    LvHandler * handler = (LvHandler *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        bool checked = lv_obj_get_state(obj) & LV_STATE_CHECKED;
        handler->chart_handler.ascending_trigger = checked;

        // TODO: Manage second channel
        ChartHandlerChannel ch = CHART_HANDLER_CHANNEL_1;
        if (!checked && !handler->chart_handler.descending_trigger)
            lv_api_hide_trigger_line(handler, ch);
        else {
            // Uncheck other button
            lv_obj_remove_state(handler->trigger_checkbox_desc, LV_STATE_CHECKED);
            handler->chart_handler.descending_trigger = false;

            lv_api_update_trigger_line(handler, ch, ADC_VALUE_TO_VOLTAGE(handler->chart_handler.trigger[ch]));
        }
    }
}

static void _lv_api_trigger_checkbox_handler_desc(lv_event_t * e) {
    LvHandler * handler = (LvHandler *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        bool checked = lv_obj_get_state(obj) & LV_STATE_CHECKED;
        handler->chart_handler.descending_trigger = checked;

        // TODO: Manage second channel
        ChartHandlerChannel ch = CHART_HANDLER_CHANNEL_1;
        if (!checked && !handler->chart_handler.ascending_trigger)
            lv_api_hide_trigger_line(handler, ch);
        else {
            // Uncheck other button
            lv_obj_remove_state(handler->trigger_checkbox_asc, LV_STATE_CHECKED);
            handler->chart_handler.ascending_trigger = false;

            lv_api_update_trigger_line(handler, ch, ADC_VALUE_TO_VOLTAGE(handler->chart_handler.trigger[ch]));
        }
    }
}

static void _lv_api_signal_generator_event_handler(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    shared_data->generator_index = lv_obj_get_index(obj);
    LvHandler * handler = (LvHandler *)lv_event_get_user_data(e);
    lv_obj_add_flag(handler->menu, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t * _lv_api_create_chart(lv_obj_t * parent, uint32_t *buffer) {
    // Create a chart object
    lv_obj_t * chart = lv_chart_create(parent);
    lv_obj_set_size(chart, 200, 150); // Set the size of the chart

    // Create a data series on the chart
    lv_chart_series_t * series = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);

    // Set the number of points in the chart
    lv_chart_set_point_count(chart, WAVES_SIZE);

    // Set the y-values from the array to the chart series
    for(int i = 0; i < WAVES_SIZE; i++) {
        lv_chart_set_next_value(chart, series, buffer[i]);
    }

    // Optionally, configure chart properties such as range, type, and grid
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0xffff); // Set the y-axis range
    return chart;
}

void _lv_api_init_trigger_tab(LvHandler * handler, lv_obj_t * tabview) {
    lv_obj_t * trigger_tab = lv_tabview_add_tab(tabview, "Trigger");
    
    lv_obj_set_flex_flow(trigger_tab, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(trigger_tab, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    handler->trigger_checkbox_asc = lv_checkbox_create(trigger_tab);
    lv_checkbox_set_text(handler->trigger_checkbox_asc, "Enable ascending trigger");
    lv_obj_add_event_cb(handler->trigger_checkbox_asc, _lv_api_trigger_checkbox_handler_asc, LV_EVENT_ALL, handler);
    lv_obj_update_layout(handler->trigger_checkbox_asc);

    handler->trigger_checkbox_desc = lv_checkbox_create(trigger_tab);
    lv_checkbox_set_text(handler->trigger_checkbox_desc, "Enable descending trigger");
    lv_obj_add_event_cb(handler->trigger_checkbox_desc, _lv_api_trigger_checkbox_handler_desc, LV_EVENT_ALL, handler);
    lv_obj_update_layout(handler->trigger_checkbox_desc);

    // Set style which cant be set inside the theme
    lv_obj_set_style_bg_color(trigger_tab, LV_BLACK, LV_PART_MAIN);
    lv_obj_set_style_pad_all(trigger_tab, 30U, LV_PART_MAIN);
}

void _lv_api_init_signal_generator_tab(LvHandler * handler, lv_obj_t * tabview) {
    lv_obj_t * generator_tab = lv_tabview_add_tab(tabview, "Signal generator");

    lv_obj_t * parent = lv_list_create(generator_tab);
    lv_obj_set_size(parent, lv_pct(100), lv_pct(100));
    lv_obj_center(parent);

    /* Create a number of child objects within the parent container */
    for (int i = 0; i < WAVES_TYPE_COUNT; i++) {
        lv_obj_t * obj = _lv_api_create_chart(parent, waves_table[i]);
                
        /* Add event handler for the object */
        lv_obj_add_event_cb(obj, _lv_api_signal_generator_event_handler, LV_EVENT_CLICKED, handler);
    }

    // Set style which cant be set inside the theme
    lv_obj_set_style_bg_color(generator_tab, LV_BLACK, LV_PART_MAIN);
    lv_obj_set_style_pad_row(parent, 20U, LV_PART_MAIN);
}

void _lv_api_menu_init(LvHandler * handler) {
    lv_obj_t * screen = lv_display_get_screen_active(handler->display);
    size_t h = lv_display_get_vertical_resolution(handler->display) - HEADER_SIZE;
    size_t w = lv_display_get_horizontal_resolution(handler->display);

    handler->menu = lv_menu_create(screen);
    lv_obj_set_size(handler->menu, w, h);
    lv_obj_align(handler->menu, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_add_flag(handler->menu, LV_OBJ_FLAG_HIDDEN);

    // Create a tabview object
    lv_obj_t * tabview = lv_tabview_create(handler->menu);
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_RIGHT);
    lv_tabview_set_tab_bar_size(tabview, 200U);

    // Add tabs to the tabview
    _lv_api_init_trigger_tab(handler, tabview);
    _lv_api_init_signal_generator_tab(handler, tabview);
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

    // Create a button
    lv_obj_t * btn = lv_btn_create(handler->header);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, HEADER_SIZE);
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);

    // Set button event
    lv_obj_add_event_cb(btn, _lv_api_menu_btn_event_handler, LV_EVENT_CLICKED, handler);
    
    // Add label to button
    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Menu");
    lv_obj_center(btn_label);
    lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_20, LV_PART_MAIN);

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

    for (size_t ch = 0; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        lv_chart_set_ext_y_array(handler->chart, handler->series[ch], handler->channels[ch]);

        // Initialize trigger lines
        handler->trigger_line[ch] = lv_line_create(handler->chart);
        handler->trigger_points[ch][1].x = LCD_WIDTH;

        // Set trigger points and hide line
        lv_line_set_points(handler->trigger_line[ch], handler->trigger_points[ch], 2U);
        lv_api_hide_trigger_line(handler, ch);
    }
}

void _lv_api_chart_handler_init(LvHandler * handler) {
    chart_handler_init(&handler->chart_handler, handler);
}

void _lv_api_bar_init(LvHandler * handler) {
    handler->loading_bar = lv_bar_create(handler->chart);
    lv_obj_set_size(handler->loading_bar, LCD_WIDTH, 5);
    lv_obj_add_flag(handler->loading_bar, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(handler->loading_bar, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(handler->loading_bar, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_bar_set_range(handler->loading_bar, 0, CHART_HANDLER_VALUES_COUNT);
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
    _lv_api_menu_init(handler);
    _lv_api_bar_init(handler);
}

float lv_api_grid_units_to_chart(ChartHandlerChannel ch, float value) {
    const float div[CHART_HANDLER_CHANNEL_COUNT] = {
        CHART_AXIS_PRIMARY_Y_MAX_COORD / (float)(CHART_Y_DIVISION_COUNT),
        CHART_AXIS_SECONDARY_Y_MAX_COORD / (float)(CHART_Y_DIVISION_COUNT)
    };
    return value * div[ch];
}

float lv_api_grid_units_to_screen(ChartHandlerChannel ch, float value) {
    const float div = CHART_HEIGHT / (float)(CHART_Y_DIVISION_COUNT);
    return value * div;
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

void lv_api_update_trigger_line(LvHandler * handler, ChartHandlerChannel ch, float volt) {
    if (handler == NULL)
        return;

    // Convert voltage to screen space
    float height = chart_handler_voltage_to_grid_units(&handler->chart_handler, ch, volt);
    height = lv_api_grid_units_to_screen(ch, height);

    // Reverse height because 0 start from the top
    // BUG: A correction factor is added to compensate for the error
    const float error = 6.f * (handler->chart_handler.scale[ch] / 1000.f);
    // height = CHART_HEIGHT - (height - error);
    height = CHART_HEIGHT - height;

    // Update points
    handler->trigger_points[ch][0].y = handler->trigger_points[ch][1].y = height;
    handler->trigger_update[ch] = true;
}

void lv_api_hide_trigger_line(LvHandler * handler, ChartHandlerChannel ch) {
    if (handler == NULL)
        return;
    lv_obj_add_flag(handler->trigger_line[ch], LV_OBJ_FLAG_HIDDEN);
}

void lv_api_hide_loading_bar(LvHandler * handler) {
    if (handler == NULL)
        return;
    handler->loading_bar_hide = true;
    handler->loading_bar_value = 0;
}

void lv_api_update_loading_bar(LvHandler * handler, size_t value) {
    if (handler == NULL)
        return;
    handler->loading_bar_value = value;
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

    // Update trigger line
    for (ChartHandlerChannel ch = CHART_HANDLER_CHANNEL_1; ch < CHART_HANDLER_CHANNEL_COUNT; ++ch) {
        if (handler->trigger_update[ch]) {
            /*
             * The line is hidden if it goes outside of the screen bound
             * The trigger line is always horizontal so only one value can
             * be checked to decide if the line should be hidden or not
             */
            if (handler->trigger_points[ch][0].y < 0 || handler->trigger_points[ch][0].y >= CHART_HEIGHT)
                lv_obj_add_flag(handler->trigger_line[ch], LV_OBJ_FLAG_HIDDEN);
            else {
                // Show line and update position
                lv_obj_clear_flag(handler->trigger_line[ch], LV_OBJ_FLAG_HIDDEN);
                lv_line_set_points(handler->trigger_line[ch], handler->trigger_points[ch], 2U);
            }

            handler->trigger_update[ch] = false;
        }
    }

    // Update loading bar
    if (handler->loading_bar_hide) {
        lv_obj_add_flag(handler->loading_bar, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(handler->loading_bar, 0, false);

        handler->loading_bar_hide = false;
    }
    else if (handler->loading_bar_value > 0) {
        lv_obj_clear_flag(handler->loading_bar, LV_OBJ_FLAG_HIDDEN);
        lv_bar_set_value(handler->loading_bar, handler->loading_bar_value, false);

        handler->loading_bar_value = 0;
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

    const float dt = size / (float)CHART_POINT_COUNT;
    // const size_t step = dt == 0.f ? 1U : dt;
    
    size_t j = 0;
    float t = 0;
    for (size_t x = 0; x < CHART_POINT_COUNT; ++x) {
        // Interpolate
        // size_t k = j >= (CHART_HANDLER_VALUES_COUNT - 1) ? (CHART_HANDLER_VALUES_COUNT - 1) : (j + step);
        float val = values[j]; // LERP(values[j], values[k], t);
        if (val == NAN)
            val = LV_CHART_POINT_NONE;
        else {
            // Convert to screen space
            val = lv_api_grid_units_to_chart(ch, val);
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

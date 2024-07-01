/**
 * @file lvgl_colors.h
 * @brief Collection of colors for LVGL
 *
 * @date Apr 12, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef LVGL_COLORS_H
#define LVGL_COLORS_H

#include "lvgl.h"

#define LV_WHITE  ((lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0xFF))
#define LV_LIGHT_GRAY ((lv_color_t)LV_COLOR_MAKE(0xDE, 0xDE, 0xDE))
#define LV_DARK_GRAY  ((lv_color_t)LV_COLOR_MAKE(0x4E, 0x4E, 0x4E))
#define LV_BLACK  ((lv_color_t)LV_COLOR_MAKE(0x14, 0x14, 0x14))

#define LV_RED    ((lv_color_t)LV_COLOR_MAKE(0xFF, 0x31, 0x31))
#define LV_GREEN  ((lv_color_t)LV_COLOR_MAKE(0x77, 0xDD, 0x77))
#define LV_BLUE   ((lv_color_t)LV_COLOR_MAKE(0x22, 0x71, 0xB3))

#define LV_YELLOW ((lv_color_t)LV_COLOR_MAKE(0xFF, 0xFF, 0x00))
#define LV_PURPLE ((lv_color_t)LV_COLOR_MAKE(0xFF, 0x00, 0xFF))

#endif  // LVGL_COLORS_H

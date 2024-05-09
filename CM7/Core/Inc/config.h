/**
 * @file config.h
 * @brief Configuration data for the discovery board
 *
 * @date Mar 31, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "main.h"

/*** General ***/

#define LED_GREEN_GPIO_Port LED1_GPIO_Port
#define LED_ORANGE_GPIO_Port LED2_GPIO_Port
#define LED_RED_GPIO_Port LED3_GPIO_Port
#define LED_BLUE_GPIO_Port LED4_GPIO_Port

#define LED_GREEN_Pin LED1_Pin
#define LED_ORANGE_Pin LED2_Pin
#define LED_RED_Pin LED3_Pin
#define LED_BLUE_Pin LED4_Pin

/*** LCD ***/

#define LCD_COLOR_DEPTH_RGB565 sizeof(uint16_t)
#define LCD_COLOR_DEPTH_ARGB8888 sizeof(uint32_t)

#define LCD_WIDTH (800U)
#define LCD_HEIGHT (480U)
#define LCD_COLOR_DEPTH LCD_COLOR_DEPTH_ARGB8888
#define LCD_RESOLUTION (LCD_WIDTH * LCD_HEIGHT)
#define LCD_BYTE_COUNT (LCD_RESOLUTION * LCD_COLOR_DEPTH)

#define LCD_FRAME_BUFFER_0_ADDRESS (0xD0000000)
#define LCD_FRAME_BUFFER_0_WIDTH LCD_BYTE_COUNT

#define LCD_FRAME_BUFFER_1_ADDRESS (LCD_FRAME_BUFFER_0_ADDRESS + LCD_FRAME_BUFFER_0_WIDTH)
#define LCD_FRAME_BUFFER_1_WIDTH LCD_BYTE_COUNT

#define LCD_INITIAL_BRIGHTNESS (255U)

/*** CHART ***/

/** @brief Primary and secondary Y axis maximum coordinates for the chart */
#define CHART_AXIS_PRIMARY_Y_MAX_COORD (500U)
#define CHART_AXIS_SECONDARY_Y_MAX_COORD (500U)

/** @brief Horizontal and vertical line count for the chart */
#define CHART_HORIZONTAL_LINE_COUNT (11U)
#define CHART_VERTICAL_LINE_COUNT (17U)

/** @brief Total number of points of the chart */
#define CHART_POINT_COUNT ((CHART_VERTICAL_LINE_COUNT) * 50U)

/** @brief Minimum and maximum values for the X value of the chart in us */
#define CHART_MIN_X_SCALE (1000.0f) // in us
#define CHART_MAX_X_SCALE (100000.0f) // in us

/** @brief Minimum and maximum values for the Y value of the chart in mV */
#define CHART_MIN_Y_SCALE (10.0f) // in mV
#define CHART_MAX_Y_SCALE (10000.0f) // in mV

/** @brief Maximum number of samples of the ADC */
#define CHART_SAMPLE_COUNT (((uint32_t)CHART_MAX_X_SCALE / 1000U) + 1U)

#endif  // CONFIG_H

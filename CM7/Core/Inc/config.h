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

#define LCD_WIDTH 800
#define LCD_HEIGHT 480
#define LCD_COLOR_DEPTH LCD_COLOR_DEPTH_ARGB8888
#define LCD_RESOLUTION (LCD_WIDTH * LCD_HEIGHT)
#define LCD_BYTE_COUNT (LCD_RESOLUTION * LCD_COLOR_DEPTH)

#define LCD_FRAME_BUFFER_0_ADDRESS 0xD0000000
#define LCD_FRAME_BUFFER_0_WIDTH LCD_BYTE_COUNT

#define LCD_FRAME_BUFFER_1_ADDRESS (LCD_FRAME_BUFFER_0_ADDRESS + LCD_FRAME_BUFFER_0_WIDTH)
#define LCD_FRAME_BUFFER_1_WIDTH LCD_BYTE_COUNT

#define LCD_INITIAL_BRIGHTNESS 255

#endif  // CONFIG_H

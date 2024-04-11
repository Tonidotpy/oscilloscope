/**
 * @file lcd.h
 * @brief LCD communication utility functions
 *
 * @date Mar 31, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef LCD_H
#define LCD_H

#include "main.h"

#include <stdbool.h>

typedef enum {
    LCD_OFF,
    LCD_ON
} LcdStatus;

/**
 * @brief Initialize the LCD internal structure
 * @details
 *
 * @param hdsi The DSI HOST handler
 * @return HAL_StatusTypeDef HAL_OK if everything was initialized correctly
 */
HAL_StatusTypeDef lcd_init(DSI_HandleTypeDef * hdsi, uint32_t brightness);

/**
 * @brief Reset the DSI LCD controller
 * @details This function takes at least 30 ms to execute
 */
void lcd_reset_dsi_controller(void);

/**
 * @brief Get the current status of the LCD
 */
LcdStatus lcd_get_status(void);

/**
 * @brief Turn on the display
 */
void lcd_on(void);

/**
 * @brief Turn off the display
 */
void lcd_off(void);

/**
 * @brief Get brightness level
 */
uint32_t lcd_get_brightness(void);

/**
 * @brief Set brightness level
 */
void lcd_set_brightness(uint32_t brightness);

#endif  // LCD_H

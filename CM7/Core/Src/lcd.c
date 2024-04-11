/**
 * @file lcd.c
 * @brief LCD communication utility functions
 *
 * @date Mar 31, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "lcd.h"

#include <stdint.h>

#include "config.h"
#include "otm8009a.h"

struct {
    LcdStatus status;
    uint32_t brightness;

    DSI_HandleTypeDef * hdsi;
    uint32_t otm8009a_id;
    OTM8009A_IO_t io_context;
    OTM8009A_Object_t otm8009a;
} hlcd;

/**
  * @brief  DCS or Generic short/long write command
  *
  * @param  channel_nbr Virtual channel ID
  * @param  reg Register to be written
  * @param  data Pointer to the data to write
  * @param  size Number of bytes of the data
  * @retval HAL_StatusTypeDef HAL_OK if the write operation is done correctly
  */
static int32_t _lcd_dsi_write(
    short unsigned int channel_nbr,
    short unsigned int reg,
    unsigned char * data,
    short unsigned int size)
{
    if (size <= 1U)
        return HAL_DSI_ShortWrite(hlcd.hdsi, channel_nbr, DSI_DCS_SHORT_PKT_WRITE_P1, reg, (uint32_t)data[size]);
    return HAL_DSI_LongWrite(hlcd.hdsi, channel_nbr, DSI_DCS_LONG_PKT_WRITE, size, (uint32_t)reg, data);
}
/**
  * @brief  DCS or Generic read command
  *
  * @param  channel_nbr Virtual channel ID
  * @param  reg Register to be read
  * @param  data Pointer to a buffer where the data is stored
  * @param  size Number of bytes that should be read
  * @retval HAL_StatusTypeDef HAL_OK if the read operations was done correctly
  */
static int32_t _lcd_dsi_read(
    short unsigned int channel_nbr,
    short unsigned int reg,
    unsigned char * data,
    short unsigned int size)
{
    return HAL_DSI_Read(hlcd.hdsi, channel_nbr, data, size, DSI_DCS_SHORT_PKT_READ, reg, data);
}

/**
 * @brief Initialize the OTM8009A display controller
 * @attention This function should be called before the LCD
 * is used, otherwise nothing will be shown
 *
 * @return HAL_StatusTypeDef HAL_OK if the controller is initialized corretly
 */
HAL_StatusTypeDef _lcd_display_controller_init(void) {
    /* Configure the audio driver */
    hlcd.io_context.Address = 0;
    hlcd.io_context.GetTick = (int32_t(*)(void))HAL_GetTick;
    hlcd.io_context.WriteReg = _lcd_dsi_write;
    hlcd.io_context.ReadReg = _lcd_dsi_read;

    if (OTM8009A_RegisterBusIO(&hlcd.otm8009a, &hlcd.io_context) != OTM8009A_OK)
        return HAL_ERROR;
    if (OTM8009A_ReadID(&hlcd.otm8009a, &hlcd.otm8009a_id) != OTM8009A_OK)
        return HAL_ERROR;
    if (OTM8009A_Init(&hlcd.otm8009a, OTM8009A_FORMAT_RGB888, OTM8009A_ORIENTATION_LANDSCAPE) != OTM8009A_OK)
        return HAL_ERROR;
    return HAL_OK;
}

HAL_StatusTypeDef lcd_init(DSI_HandleTypeDef * hdsi, uint32_t brightness) {
    hlcd.hdsi = hdsi;
    hlcd.status = true;
    hlcd.brightness = brightness;

    return _lcd_display_controller_init();
}

void lcd_reset_dsi_controller(void) {
    HAL_GPIO_WritePin(DSI_RESET_GPIO_Port, DSI_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(DSI_RESET_GPIO_Port, DSI_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
}

LcdStatus lcd_get_status(void) {
    return hlcd.status;
}

void lcd_on(void) {
    OTM8009A_DisplayOn(&hlcd.otm8009a);
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_SET);
    // Restore original brightness
    OTM8009A_SetBrightness(&hlcd.otm8009a, hlcd.brightness);
    hlcd.status = LCD_ON;
}

void lcd_off(void) {
    OTM8009A_DisplayOff(&hlcd.otm8009a);
    HAL_GPIO_WritePin(LCD_BACKLIGHT_GPIO_Port, LCD_BACKLIGHT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin, GPIO_PIN_RESET);
    // Set brightness without updating handler
    OTM8009A_SetBrightness(&hlcd.otm8009a, 0U);
    hlcd.status = LCD_OFF;
}

uint32_t lcd_get_brightness(void) {
    OTM8009A_GetBrightness(&hlcd.otm8009a, &hlcd.brightness);
    return hlcd.brightness;
}
void lcd_set_brightness(uint32_t brightness) {
    if (OTM8009A_SetBrightness(&hlcd.otm8009a, brightness) == OTM8009A_OK)
        hlcd.brightness = brightness;
}


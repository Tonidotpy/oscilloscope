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

/** @brief Time to wait until a new button press is handled in ms (used for debounce) */
#define BUTTON_DEBOUNCE_TIME (130U)

/** @brief The LED GPIO ports */
#define LED_GREEN_GPIO_Port LED1_GPIO_Port
#define LED_ORANGE_GPIO_Port LED2_GPIO_Port
#define LED_RED_GPIO_Port LED3_GPIO_Port
#define LED_BLUE_GPIO_Port LED4_GPIO_Port

/** @brief The LED pins */
#define LED_GREEN_Pin LED1_Pin
#define LED_ORANGE_Pin LED2_Pin
#define LED_RED_Pin LED3_Pin
#define LED_BLUE_Pin LED4_Pin

/** @brief ADC reolution in bits */
#define ADC_RESOLUTION (14U)
/** @brief ADC voltage reference in mV */
#define ADC_VREF (3300.0f)

/**
 * @brief Convert a value read from the ADC to the corresponding voltage in mV
 *
 * @param VAL The ADC value
 * 
 * @return float The voltage in mV
 */
#define ADC_VALUE_TO_VOLTAGE(VAL) (((VAL) / (float)((1 << ADC_RESOLUTION) - 1.0f)) * ADC_VREF)

/**
 * @brief Convert a voltage in mV to the corresponding raw value 0f the ADC
 *
 * @param VAL The voltage in mV
 * 
 * @return uint16_t The ADC value 
 */
#define ADC_VOLTAGE_TO_VALUE(VAL) ((uint16_t)(((VAL) / (ADC_VREF)) * ((float)((1U << ADC_RESOLUTION) - 1.0f))))

/*** LCD ***/

/** @brief The LCD color depth in bytes */
#define LCD_COLOR_DEPTH_RGB565 sizeof(uint16_t)
#define LCD_COLOR_DEPTH_ARGB8888 sizeof(uint32_t)

/** @brief LCD sizes */
#define LCD_WIDTH (800U)
#define LCD_HEIGHT (480U)
#define LCD_COLOR_DEPTH LCD_COLOR_DEPTH_ARGB8888
#define LCD_RESOLUTION (LCD_WIDTH * LCD_HEIGHT)
#define LCD_BYTE_COUNT (LCD_RESOLUTION * LCD_COLOR_DEPTH)

/** @brief LCD frame buffer info */
#define LCD_FRAME_BUFFER_0_ADDRESS (0xD0000000)
#define LCD_FRAME_BUFFER_0_WIDTH LCD_BYTE_COUNT

#define LCD_FRAME_BUFFER_1_ADDRESS (LCD_FRAME_BUFFER_0_ADDRESS + LCD_FRAME_BUFFER_0_WIDTH)
#define LCD_FRAME_BUFFER_1_WIDTH LCD_BYTE_COUNT

#define LCD_INITIAL_BRIGHTNESS (255U)

/*** CHART ***/

/** @brief Chart ADC data memory address */
#define CHART_RAW_DATA_BASE_ADDRESS (LCD_FRAME_BUFFER_1_ADDRESS + LCD_FRAME_BUFFER_1_WIDTH)
#define CHART_RAW_DATA_WIDTH (CHART_SAMPLE_COUNT * sizeof(uint16_t))

#define CHART_CH1_RAW_DATA_ADDRESS CHART_RAW_DATA_BASE_ADDRESS
#define CHART_CH1_RAW_DATA_WIDTH CHART_RAW_DATA_WIDTH

#define CHART_CH2_RAW_DATA_ADDRESS (CHART_CH1_RAW_DATA_ADDRESS + CHART_CH1_RAW_DATA_WIDTH)
#define CHART_CH2_RAW_DATA_WIDTH CHART_RAW_DATA_WIDTH

#define CHART_TOTAL_RAW_DATA_WIDTH (CHART_CH1_RAW_DATA_WIDTH + CHART_CH2_RAW_DATA_WIDTH)

/** @brief Primary and secondary Y axis maximum coordinates for the chart */
#define CHART_AXIS_PRIMARY_Y_MAX_COORD (500U)
#define CHART_AXIS_SECONDARY_Y_MAX_COORD (500U)

/** @brief Horizontal and vertical line count for the chart */
#define CHART_HORIZONTAL_LINE_COUNT (11U)
#define CHART_VERTICAL_LINE_COUNT (17U)

/** @brief X and Y divisions count of the chart */
#define CHART_X_DIVISION_COUNT (CHART_VERTICAL_LINE_COUNT - 1U)
#define CHART_Y_DIVISION_COUNT (CHART_HORIZONTAL_LINE_COUNT - 1U)

/** @brief Total number of points of the chart */
#define CHART_POINT_COUNT ((CHART_X_DIVISION_COUNT) * 100U)

/** @brief Minimum and maximum values per division for the X value of the chart in us */
#define CHART_MIN_X_SCALE (10.0f) // in us
#define CHART_MAX_X_SCALE (300000.0f) // in us
#define CHART_DEFAULT_X_SCALE (10000.0f) // in mV

/** @brief Minimum and maximum values per division for the Y value of the chart in mV */
#define CHART_MIN_Y_SCALE (10.0f) // in mV
#define CHART_MAX_Y_SCALE (10000.0f) // in mV
#define CHART_DEFAULT_Y_SCALE (1000.0f) // in mV

/** @brief Maximum number of samples of the ADC */
#define CHART_SAMPLE_COUNT (1024U)

/** @brief Height of the chart */
#define CHART_HEIGHT (LCD_HEIGHT - HEADER_SIZE)

/** @brief Threshold used to show the bar only if the time scale is big enough */ 
#define CHART_LOADING_BAR_THRESHOLD (50000.f)

/*** HEADER ***/

/** @brief Size of the header*/
#define HEADER_SIZE (50U)

/** @brief Maximum length of the string label '\0' included */
#define HEADER_LABEL_STRING_SIZE (32U)

#endif  // CONFIG_H

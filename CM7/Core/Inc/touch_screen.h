/**
 * @file touch_screen.h
 * @brief Touch screen handler functions
 *
 * @date Mar 31, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef TOUCH_SCREEN_H
#define TOUCH_SCREEN_H

#include "main.h"

#include <stdbool.h>

#define TS_SWAP_NONE 0x01U
#define TS_SWAP_X 0x02U
#define TS_SWAP_Y 0x04U
#define TS_SWAP_XY 0x08U


// Touch screen state
typedef enum {
    TS_DISABLED,
    TS_ENABLED
} TsState;

// Touch screen slave I2C addresses
typedef enum {
    TS_I2C_ADDRESS0 = 0x54U,
    TS_I2C_ADDRESS1 = 0x70U,
    TS_I2C_ADDRESS_COUNT = 2
} TsI2cAddresses;

// Touch screen orientation
typedef enum {
    TS_ORIENTATION_NORMAL = TS_SWAP_NONE,
    TS_ORIENTATION_SWAP_X = TS_SWAP_X,
    TS_ORIENTATION_SWAP_Y = TS_SWAP_Y,
    TS_ORIENTATION_SWAP_XY = TS_SWAP_XY,
    TS_ORIENTATION_COUNT = 4
} TsOrientation;

// Touch screen touch information
typedef struct {
    uint32_t detected;
    uint32_t x;
    uint32_t y;
} TsInfo;

// Touch screen configuration
typedef struct
{
    TsState state;
    uint32_t width;
    uint32_t height;
    uint32_t orientation; // Orientation from the upper left position
    uint32_t accuracy; // The x or y difference vs the old position to consider the new values valid (in pixel)
    uint32_t max_x;
    uint32_t max_y;
    uint32_t prev_x;
    uint32_t prev_y;
} TsConfig;

/**
 * @brief Initialize the touch screen handler
 *
 * @param hi2c The I2C handler structure
 * @param width The width of the screen
 * @param height The height of the screen
 * @param orientation The orientation of the screen
 * @param accuracy The accuracy of the touch screen in pixel
 * @return HAL_StatusTypeDef HAL_OK if the initialization is done correctly
 */
HAL_StatusTypeDef ts_init(
    I2C_HandleTypeDef * hi2c,
    size_t width,
    size_t height,
    TsOrientation orientation,
    uint32_t accuracy
);

/**
 * @brief Get the current state of the touch screen
 *
 * @return TsState The state of the touch screen
 */
TsState ts_get_state(void);

/**
  * @brief Returns positions of a single touch on the screen
  * @param state A pointer to a structure where the touch info are stored
  * @retval HAL_StatusTypeDef HAL_OK if the state is retreived correctly
  */
HAL_StatusTypeDef ts_get_info(TsInfo * info);

/** @brief Enable the touch screen */
void ts_enable(void);

/** @brief Disable the touch screen */
void ts_disable(void);

#endif  // TOUCH_SCREEN_H

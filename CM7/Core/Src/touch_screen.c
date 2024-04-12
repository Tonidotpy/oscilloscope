/**
 * @file touch_screen.c
 * @brief Touch screen handler functions
 *
 * @date Mar 31, 2024
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "touch_screen.h"
#include "ft6x06.h"

struct {
    I2C_HandleTypeDef * hi2c;
    TsConfig config;

    uint32_t ft6x06_id;
    FT6X06_Object_t ft6x06;
    FT6X06_Capabilities_t capabilities;
} hts;


int32_t _ts_i2c_init(void) {
    return HAL_OK;
}

/**
 * @brief Write a 8bit value in a register of the device through I2C
 *
 * @param device_address Device address
 * @param reg The target register address to write
 * @param data A pointer to the target register value to be written
 * @param size The buffer size
 * @return HAL_StatusTypeDef HAL_OK if the register was written correctly
 */
int32_t _ts_i2c_write_reg(uint16_t device_address, uint16_t reg, uint8_t * data, uint16_t size) {
    if (HAL_I2C_Mem_Write(hts.hi2c, device_address, reg, I2C_MEMADD_SIZE_8BIT, data, size, 1000) != HAL_OK)
        return HAL_ERROR;
    return HAL_OK;
}
/**
 * @brief Read a 8bit value from a register of the device through I2C
 *
 * @param device_address Device address
 * @param reg The target register address to read from
 * @param data A pointer to the buffer where the data is stored
 * @param size The buffer size
 * @return HAL_StatusTypeDef HAL_OK if the register was written correctly
 */
int32_t _ts_i2c_read_reg(uint16_t device_address, uint16_t reg, uint8_t * data, uint16_t size) {
    if (HAL_I2C_Mem_Read(hts.hi2c, device_address, reg, I2C_MEMADD_SIZE_8BIT, data, size, 1000) != HAL_OK)
        return HAL_ERROR;
    return HAL_OK;
}

HAL_StatusTypeDef _ts_controller_init(void) {
    /* Configure the touch screen driver */
    FT6X06_IO_t io_context;
    io_context.Init = _ts_i2c_init;
    io_context.DeInit = NULL;
    io_context.ReadReg = _ts_i2c_read_reg;
    io_context.WriteReg = _ts_i2c_write_reg;
    io_context.GetTick = (int32_t(*)(void))HAL_GetTick();

    const uint32_t address[] = { TS_I2C_ADDRESS0, TS_I2C_ADDRESS1 };
    HAL_StatusTypeDef ret = HAL_OK;
    for (size_t i = 0; i < TS_I2C_ADDRESS_COUNT; ++i) {
        io_context.Address = (uint16_t)address[i];

        if (FT6X06_RegisterBusIO(&hts.ft6x06, &io_context) != FT6X06_OK)
            ret = HAL_ERROR;
        else if (FT6X06_ReadID(&hts.ft6x06, &hts.ft6x06_id) != FT6X06_OK)
            ret = HAL_ERROR;
        else if (hts.ft6x06_id != FT6X06_ID)
            ret = HAL_ERROR;
        else if (FT6X06_GetCapabilities(&hts.ft6x06, &hts.capabilities) != FT6X06_OK)
            ret = HAL_ERROR;
        else if (FT6X06_Init(&hts.ft6x06) != FT6X06_OK)
            ret = HAL_ERROR;
        else {
            hts.config.max_x = hts.capabilities.MaxXl;
            hts.config.max_y = hts.capabilities.MaxYl;
            return HAL_OK;
        }
    }
    return ret;
}

HAL_StatusTypeDef ts_init(
    I2C_HandleTypeDef * hi2c,
    size_t width,
    size_t height,
    TsOrientation orientation,
    uint32_t accuracy)
{
    hts.hi2c = hi2c;
    hts.config.state = TS_ENABLED;
    hts.config.width = width;
    hts.config.height = height;
    hts.config.orientation = orientation;
    hts.config.accuracy = accuracy;
    hts.config.prev_x = width + accuracy + 1U;
    hts.config.prev_y = height + accuracy + 1U;

    return _ts_controller_init();
}

TsState ts_get_state() {
    return hts.config.state;
}

HAL_StatusTypeDef ts_get_info(TsInfo * info) {
    if (hts.config.state == TS_DISABLED)
        return HAL_BUSY;
    if (info == NULL)
        return HAL_ERROR;

    FT6X06_State_t stat;
    uint32_t x_oriented, y_oriented;
    uint32_t x_diff, y_diff;
    if (FT6X06_GetState(&hts.ft6x06, &stat) < 0)
        return HAL_ERROR;

    // Check if the touch was detected
    if (stat.TouchDetected != 0) {
        x_oriented = stat.TouchX;
        y_oriented = stat.TouchY;

        if ((hts.config.orientation & TS_SWAP_XY) == TS_SWAP_XY) {
            x_oriented = stat.TouchY;
            y_oriented = stat.TouchX;
        }
        if ((hts.config.orientation & TS_SWAP_X) == TS_SWAP_X)
        {
            x_oriented = hts.config.max_x - x_oriented - 1UL;
        }
        if ((hts.config.orientation & TS_SWAP_Y) == TS_SWAP_Y)
        {
            y_oriented = hts.config.max_y - y_oriented - 1UL;
        }

        // Apply boundary
        info->x = (x_oriented * hts.config.width) / hts.config.max_x;
        info->y = (y_oriented * hts.config.height) / hts.config.max_y;
        info->detected = stat.TouchDetected;

        // Check accuracy
        x_diff = (info->x > hts.config.prev_x)?
            (info->x - hts.config.prev_x):
            (hts.config.prev_x - info->x);

        y_diff = (info->y > hts.config.prev_y)?
            (info->y - hts.config.prev_y):
            (hts.config.prev_y - info->y);

        // Check if should be considered as new touch
        if (x_diff > hts.config.accuracy || y_diff > hts.config.accuracy) {
            hts.config.prev_x = info->x;
            hts.config.prev_y = info->y;
        }
        else {
            info->x = hts.config.prev_x;
            info->y = hts.config.prev_y;
        }
    }
    else {
        info->detected = 0;
        info->x = hts.config.prev_x;
        info->y = hts.config.prev_y;
    }

    return HAL_OK;
}

void ts_enable(void) {
    hts.config.state = TS_ENABLED;
}

void ts_disable(void) {
    hts.config.state = TS_DISABLED;
}


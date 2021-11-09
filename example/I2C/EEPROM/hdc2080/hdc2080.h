#ifndef HDC2080_H
#define HDC2080_H

#include <stdint.h>
#include <stdbool.h>

#define HDC2080_ADDR 0x40
#define HDC2080_MANUFACTURER_ID 0x5449
#define HDC2080_DEVICE_ID 0x1050
#define HDC2080_STARTUP_MS (30)         // Datasheet said 15ms
#define HDC2080_WAIT_READ_WRITE_MS (20) // Magic number
#define HDC2080_REG_TEMPERATURE 0x00
#define HDC2080_REG_HUMIDITY 0x02
#define HDC2080_REG_CONFIGURATION 0x0F
#define HDC2080_REG_MANUFACTURER_ID 0xFC
#define HDC2080_REG_DEVICE_ID 0xFE
//#define HDC2080_REG_SERIAL_ID_FIRST 0xFB
//#define HDC2080_REG_SERIAL_ID_MID 0xFC
//#define HDC2080_REG_SERIAL_ID_LAST 0xFD

#define HDC2080_DRIVER_DEFAULT()      \
    {                                     \
        .addr = HDC2080_ADDR,             \
        .startup_ms = HDC2080_STARTUP_MS, \
        .ready_cb = NULL,                 \
        .read = NULL,                     \
        .write = NULL,                     \
        .is_error = 0,                      \
    }

typedef enum
{
    HDC2080_RESOLUTION_9BIT,
    HDC2080_RESOLUTION_11BIT,
    HDC2080_RESOLUTION_14BIT,
} hdc2080_measurement_resolution_t;

typedef union
{
    uint8_t raw_data[6];
    struct
    {
        uint16_t serial_first;
        uint16_t serial_mid;
        uint16_t serial_last;
    } __attribute__((packed)) name;
} __attribute__((packed)) hdc2080_serial_number_t;

typedef union
{
    uint8_t raw_data;
    struct
    {
        uint8_t temperature_measurement_resolution : 2;
        uint8_t humidity_measurement_resolution : 2;
        uint8_t reverse : 1;
        uint8_t mode: 2;
        uint8_t trigger: 2;
    }__attribute__((packed)) name;
} hdc2080_registers_t;

typedef void (*hdc2080_ready_cb_t)(void);
typedef bool (*hdc2080_write_i2c_t)(uint8_t addr, uint8_t *reg, uint8_t *data, uint32_t len);
typedef bool (*hdc2080_read_i2c_t)(uint8_t addr, uint8_t *reg, uint8_t *data, uint32_t len);
typedef void (*hdc2080_delay_cb_t)(uint32_t ms);

typedef struct
{
    uint8_t addr;
    uint32_t startup_ms;
    hdc2080_write_i2c_t write;
    hdc2080_read_i2c_t read;
    hdc2080_ready_cb_t ready_cb;
    hdc2080_delay_cb_t delay_ms;
    uint8_t is_error;
} dev_hdc2080_t;

/**
 * @brief Initialize temperature and humidity hdc2080
 *
 * @param[in] config - Pointer to configuration parameter
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_init(dev_hdc2080_t *config);

/**
 * @brief Set measurement resolution for temperature and humidity sensor
 *
 * @param[in] humidity - Humidity resolution parameter
 * @param[in] temperature - Temperature resolution parameter
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_set_resolution(dev_hdc2080_t *dev, hdc2080_measurement_resolution_t temperature, hdc2080_measurement_resolution_t humidity);

/**
 * @brief Write HDC2080 register
 *
 * @param[in] reg - register value
 * @retval : bool - true : Success
 *                  false: Failed
 */
static bool hdc2080_write_register(dev_hdc2080_t *dev, hdc2080_registers_t reg);

/**
 * @brief Read HDC2080 humidity value
 *
 * @param[out] humidity - Pointer to humidity value
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_read_humidity(dev_hdc2080_t *dev, float *humidity);

/**
 * @brief Read HDC2080 temperature value
 *
 * @param[out] temperature - Pointer to temperature value
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_read_temperature(dev_hdc2080_t *dev, float *temperature);

/**
 * @brief Read HDC2080 manufacture id
 *
 * @param[out] id - Pointer to HDC2080 manufacture id
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_read_manufacture_id(dev_hdc2080_t *dev, uint16_t *manufacture_id);

/**
 * @brief Read HDC2080 device id
 *
 * @param[out] id - Pointer to HDC2080 device id
 * @retval : bool - true : Success
 *                  false: Failed
 */
bool hdc2080_read_device_id(dev_hdc2080_t *dev, uint16_t *device_id);

/**
 * @brief Detect HDC2080
 *
 * @retval : bool - true : Device ready on i2c bus
 *                  false: Communication error
 */
bool hdc2080_verify_device_on_bus(dev_hdc2080_t *dev);

#endif

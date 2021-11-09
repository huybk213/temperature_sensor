#include "hdc2080.h"
#include "app_debug.h"
#include <stdlib.h>
#include <string.h>

static bool _read_register_data(dev_hdc2080_t *dev, uint8_t pointer, uint16_t *ret);

bool hdc2080_verify_device_on_bus(dev_hdc2080_t *dev)
{
    uint16_t manufacture_id = 0, device_id = 0;

    if ((hdc2080_read_manufacture_id(dev, &manufacture_id)) 
        && hdc2080_read_device_id(dev, &device_id))
    {
        if (manufacture_id == HDC2080_MANUFACTURER_ID 
            && device_id == HDC2080_DEVICE_ID)
        {
            DEBUG_INFO("HDC2080 manufacture id %d, device id %d\r\n", HDC2080_MANUFACTURER_ID, HDC2080_DEVICE_ID);
            return true;
        }
    }

    DEBUG_ERROR("HDC2080 verify device on bus error\r\n");
    return false;
}

bool hdc2080_init(dev_hdc2080_t *config)
{
    if (!config)
        return false;
    
    config->delay_ms(HDC2080_STARTUP_MS);

    return true;
}

bool hdc2080_set_resolution(dev_hdc2080_t *dev, hdc2080_measurement_resolution_t temperature, hdc2080_measurement_resolution_t humidity)
{
    hdc2080_registers_t reg;
    reg.name.humidity_measurement_resolution = 0;
    reg.name.temperature_measurement_resolution = 0;

    if (temperature == HDC2080_RESOLUTION_11BIT)
    {
        reg.name.temperature_measurement_resolution = 0x01;
    }
    else if (temperature == HDC2080_RESOLUTION_14BIT)
    {
        reg.name.temperature_measurement_resolution = 0x00;
    }
    else if (temperature == HDC2080_RESOLUTION_9BIT)
    {
        reg.name.temperature_measurement_resolution = 0x02;
    }
    else // Not supported 8 bit resolution, see hdc2080 datasheet for more details
    {
        return false;
    }

    if (humidity == HDC2080_RESOLUTION_11BIT)
    {
        reg.name.humidity_measurement_resolution = 0x01;
    }
    else if (temperature == HDC2080_RESOLUTION_14BIT)
    {
        reg.name.humidity_measurement_resolution = 0x00;
    }
    else if (humidity == HDC2080_RESOLUTION_9BIT)
    {
        reg.name.humidity_measurement_resolution = 0x02;
    }
    else
    {
        return false;
    }

    return hdc2080_write_register(dev, reg);
}

bool hdc2080_write_register(dev_hdc2080_t *dev, hdc2080_registers_t reg)
{
    uint8_t tmp_reg = HDC2080_REG_CONFIGURATION;

    uint8_t buf[2] = {reg.raw_data, 0x00};

    if (dev->write(dev->addr, &tmp_reg, buf, sizeof(buf)) == 0)
    {
        DEBUG_ERROR("HCD2080 write err\r\n");
        return false;
    }

    dev->delay_ms(HDC2080_WAIT_READ_WRITE_MS); // ugly sensor

    return true;
}

bool hdc2080_read_temperature(dev_hdc2080_t *dev, float *temperature)
{
    uint16_t raw_t;
    uint8_t reg = HDC2080_REG_TEMPERATURE;
    if (_read_register_data(dev, reg, &raw_t) == 0)
        return false;
    *temperature = ((float)raw_t / 65536.0f) * 165.0f - 40.0f;
    return true;
}

bool hdc2080_read_humidity(dev_hdc2080_t *dev, float *humidity)
{
    uint16_t raw_h;
    uint8_t reg = HDC2080_REG_HUMIDITY;

    if (_read_register_data(dev, reg, &raw_h) == 0)
        return false;
    *humidity = ((float)raw_h / 65536.0f) * 100.0f;

    return true;
}

bool hdc2080_read_manufacture_id(dev_hdc2080_t *dev, uint16_t *manufacture_id)
{
    uint8_t reg = HDC2080_REG_MANUFACTURER_ID;

    return (_read_register_data(dev, reg, manufacture_id));
}

bool hdc2080_read_device_id(dev_hdc2080_t *dev, uint16_t *device_id)
{
    uint8_t reg = HDC2080_REG_DEVICE_ID;
    if (_read_register_data(dev, reg, device_id))
        return true;
    return false;
}

static bool _read_register_data(dev_hdc2080_t *dev, uint8_t pointer, uint16_t *ret)
{
    if (dev->write(dev->addr, NULL, &pointer, 1) == 0)
    {
        DEBUG_ERROR("HCD2080 read data err\r\n");
        return false;
    }

    dev->delay_ms(HDC2080_WAIT_READ_WRITE_MS);
    uint8_t buf[2];

    if (dev->read(dev->addr, NULL, buf, sizeof(buf)) == 0)
    {
        DEBUG_ERROR("HCD2080 read data err 1\r\n");
        return 0;
    }

    *ret = buf[0] * 256 + buf[1];

    return true;
}

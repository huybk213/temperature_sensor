#include "app_flash.h"
#include <string.h>
#include "app_debug.h"
#include "ht32.h"

static __ALIGN4 app_flash_data_t m_data;

void app_flash_data_initialize(void)
{
    app_flash_data_t *data = (app_flash_data_t*)APP_FLASH_WRITE_ADDR;
    if (data->flag != APP_FLASH_VALID_FLAG)
    {
        m_data.temp_high = DEFAULT_TEMP_HIGH;
        m_data.temp_low = DEFAULT_TEMP_LOW;
        m_data.humi_high = DEFAULT_HUMI_HIGH;
        m_data.humi_low = DEFAULT_HUMI_LOW;
    }
    else
    {
        memcpy(&m_data, data, sizeof(app_flash_data_t));
    }
}

app_flash_data_t *app_flash_load_cfg(void)
{
    return &m_data;
}


/**
 * @brief           Store data into flash
 */
void app_flash_store_data(app_flash_data_t *data)
{
    memcpy(&m_data, data, sizeof(app_flash_data_t));
    m_data.flag = APP_FLASH_VALID_FLAG;

    app_flash_data_t *verify;
    FLASH_State err;
    uint32_t addr = APP_FLASH_WRITE_ADDR;
    uint32_t *p = (uint32_t *)&m_data;

    // Erase data
    __disable_irq();
    err = FLASH_ErasePage(addr);
    __enable_irq();
    if (err != FLASH_COMPLETE)
    {
        NVIC_SystemReset();
    }

    // Write
    __disable_irq();
    for (uint32_t i = 0; i < (sizeof(app_flash_data_t) + 3) / sizeof(uint32_t); i++)
    {
        err = FLASH_ProgramWordData(addr, p[i]);
        addr += 4;
    }
    __enable_irq();

    // Verify
    verify = (app_flash_data_t *)APP_FLASH_WRITE_ADDR;
    if (memcmp(&data, verify, sizeof(app_flash_data_t)) != 0)
    {
        NVIC_SystemReset();
    }
}


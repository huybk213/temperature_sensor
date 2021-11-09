#ifndef APP_FLASH_H
#define APP_FLASH_H

#include <stdint.h>

#define APP_FLASH_WRITE_ADDR              (15*1024) 
#define APP_FLASH_VALID_FLAG                0x12345678

#define DEFAULT_TEMP_LOW                    (20)
#define DEFAULT_TEMP_HIGH                   (40)
#define DEFAULT_HUMI_LOW                    (20)
#define DEFAULT_HUMI_HIGH                   (80)

typedef struct
{
    float temp_low;
    float temp_high;
    float humi_low;
    float humi_high;
    uint32_t flag;
} app_flash_data_t;

/**
 * @brief           Load config from flash
 */
app_flash_data_t *app_flash_load_cfg(void);

/**
 * @brief           Store data into flash
 */
void app_flash_store_data(app_flash_data_t *data);

/**
 * @brief           Initialize data from flash
 */
void app_flash_data_initialize(void);

#endif /* APP_FLASH_H */

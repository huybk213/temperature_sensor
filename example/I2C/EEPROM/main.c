#include <stdlib.h>
#include "ht32.h"
#include "ht32_board.h"
#include "app_btn.h"
#include "lcd.h"
#include "app_debug.h"
#include "lcd.h"
#include "app_sync.h"
#include "app_flash.h"

#define RELAY_PORT          HT_GPIOA
#define RELAY_PIN           GPIO_PIN_1
#define RELAY_AFIO_PORT      GPIO_PA
#define RELAY_AFIO_PIN       AFIO_PIN_1

#define RELAY_ALARM_ON()        GPIO_WriteOutBits(RELAY_PORT, RELAY_PIN, SET)
#define RELAY_ALARM_OFF()       GPIO_WriteOutBits(RELAY_PORT, RELAY_PIN, RESET)

#define BTN0_PORT          HT_GPIOA
#define BTN0_PIN           GPIO_PIN_1
#define BTN0_AFIO_PORT      GPIO_PA
#define BTN0_AFIO_PIN       AFIO_PIN_1

#define BTN1_PORT          HT_GPIOA
#define BTN1_PIN           GPIO_PIN_2
#define BTN1_AFIO_PORT      GPIO_PA
#define BTN1_AFIO_PIN       AFIO_PIN_2

#define BTN2_PORT          HT_GPIOA
#define BTN2_PIN           GPIO_PIN_3
#define BTN2_AFIO_PORT      GPIO_PA
#define BTN2_AFIO_PIN       AFIO_PIN_3

#define LCD_D0_PORT          HT_GPIOA
#define LCD_D0_PIN           GPIO_PIN_4
#define LCD_D0_AFIO_PORT      GPIO_PA
#define LCD_D0_AFIO_PIN       AFIO_PIN_4

#define LCD_D1_PORT          HT_GPIOA
#define LCD_D1_PIN           GPIO_PIN_5
#define LCD_D1_AFIO_PORT      GPIO_PA
#define LCD_D1_AFIO_PIN       AFIO_PIN_5

#define LCD_D2_PORT          HT_GPIOA
#define LCD_D2_PIN           GPIO_PIN_6
#define LCD_D2_AFIO_PORT      GPIO_PA
#define LCD_D2_AFIO_PIN       AFIO_PIN_6

#define LCD_D3_PORT          HT_GPIOA
#define LCD_D3_PIN           GPIO_PIN_6
#define LCD_D3_AFIO_PORT      GPIO_PA
#define LCD_D3_AFIO_PIN       AFIO_PIN_6

#define LCD_RS_PORT          HT_GPIOA
#define LCD_RS_PIN           GPIO_PIN_7
#define LCD_RS_AFIO_PORT      GPIO_PA
#define LCD_RS_AFIO_PIN       AFIO_PIN_7

#define LCD_EN_PORT          HT_GPIOA
#define LCD_EN_PIN           GPIO_PIN_8
#define LCD_EN_AFIO_PORT      GPIO_PA
#define LCD_EN_AFIO_PIN       AFIO_PIN_8

#define BUTTON_ENTER_INDEX      0
#define BUTTON_DOWN_INDEX       1
#define BUTTON_UP_INDEX         2

typedef enum
{
    BUTTON_SETUP_STATE_INIT,
    BUTTON_SETUP_STATE_T_HIGH,
    BUTTON_SETUP_STATE_T_LOW,
    BUTTON_SETUP_STATE_FINISH,
} button_setup_state_t;

void wdt_configuration(void);
volatile uint32_t m_sys_tick;
volatile uint32_t m_feed_wdt = 0;
void gpio_config(void);
void btn_configuration(void);
uint32_t sys_get_tick_ms(void);
button_setup_state_t m_btn_state = BUTTON_SETUP_STATE_INIT;
static void task_blink_lcd(void *arg);
Lcd_HandleTypeDef lcd;
void sys_delay_ms(uint32_t ms);
void task_btn_scan(void *arg);
void task_sensor(void *arg);

uint32_t tmp_cfg_t_high;
uint32_t tmp_cfg_t_low;
bool lcd_blinking = false;

int main(void)
{
    /* Config system tick */
    SYSTICK_ClockSourceConfig(SYSTICK_SRC_STCLK);       // Default : CK_AHB/8
    SYSTICK_SetReloadValue(SystemCoreClock / 8 / 1000); // (CK_AHB/8/1000) = 1ms on chip
    SYSTICK_IntConfig(ENABLE);                          // Enable SYSTICK Interrupt
    SYSTICK_CounterCmd(SYSTICK_COUNTER_CLEAR);          // Clear Initial Counter
    SYSTICK_CounterCmd(SYSTICK_COUNTER_ENABLE);         // Enable Systick Counter
    
    /* Config wdt */
    wdt_configuration();
    WDT_Restart();
    
    gpio_config();
    RELAY_ALARM_OFF();
    btn_configuration();
    
    static Lcd_PortType ports[] = 
    {
        LCD_D0_PORT, LCD_D1_PORT, LCD_D2_PORT, LCD_D3_PORT,
    };

    static Lcd_PinType pins[] = {LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN};

    lcd.mode = LCD_4_BIT_MODE;

	lcd.en_pin = LCD_EN_PIN;
	lcd.en_port = LCD_EN_PORT;

	lcd.rs_pin = LCD_RS_PIN;
	lcd.rs_port = LCD_RS_PORT;

	lcd.data_pin = pins;
	lcd.data_port = ports;
    
    lcd.lcd_delay = sys_delay_ms;
    
    Lcd_create(&lcd);
    Lcd_cursor(&lcd, 0,0);
    
    app_sync_config_t config;
    config.get_ms = sys_get_tick_ms;
    config.polling_interval_ms = 1;
    app_sync_sytem_init(&config);
        
    app_sync_register_callback(task_btn_scan,
                               20,
                               SYNC_DRV_REPEATED,
                               SYNC_DRV_SCOPE_IN_LOOP);
                               
    while (1)
    {
        app_sync_polling_task();
        task_sensor(NULL);
    }
}

static void task_btn_scan(void *arg)
{
    app_btn_scan(NULL);
}

static void task_sensor(void *arg)
{
//    static bool over_temp = false;
    static uint32_t m_last_tick = 0;
    if (m_sys_tick - m_last_tick >= (uint32_t)1000)
    {
        m_last_tick = m_sys_tick;
                        
//        if (SUCCESS == Read_TempAndHumidity (&sensor))
//        {
//            DEBUG_INFO("Temp %d.%d, humi %d.%d\r\n", sensor.temp_int, sensor.temp_deci, 
//                                               sensor.humi_int, sensor.humi_deci);
//            temperature = sensor.temp_int; 
//            humidity = sensor.humi_int;
//            if (lcd_blinking == false)
//            {
//                char tmp[16];
//                memset(tmp, 0, sizeof(tmp));
//                app_flash_data_t *cfg = app_flash_load_cfg();
//                
//                sprintf(tmp, "T: %uoC, H: %u%% ", temperature, humidity);
//                Lcd_cursor(&lcd, 0,0);
//                Lcd_string(&lcd, tmp);
//                
//                sprintf(tmp, "H  %02u,  L  %u  ", cfg->temp_high, cfg->temp_low);  
//                Lcd_cursor(&lcd, 1,0);
//                Lcd_string(&lcd, tmp);
//            }
//        }
//        else
//        {
//            DEBUG_INFO("Read sensor error\r\n");
//            buzzer_beep = 0;
            RELAY_ALARM_OFF();
//        }
    }
}

volatile uint32_t m_delay = 0;
void sys_delay_ms(uint32_t ms)
{
    __disable_irq();
    m_delay = ms;
    __enable_irq();
    while (m_delay)
    {
//        WDT_Restart();
    }
}

void SysTick_Handler(void)
{
    m_sys_tick++;
    if (m_feed_wdt++ == 100)
    {
        // Feed wdt
        HT_WDT->CR = ((uint32_t)0x5FA00000) | 0x1;
        m_feed_wdt = 0;
    }
    if (m_delay)
    {
        --m_delay;
    }
}

void wdt_configuration(void)
{
    CKCU_PeripClockConfig_TypeDef CKCU_clock = {{0}};
    CKCU_clock.Bit.WDT = 1;
    CKCU_PeripClockConfig(CKCU_clock, ENABLE);

    /* Enable WDT APB clock                                                                                   */
    /* Reset WDT                                                                                              */
    WDT_DeInit();
    /* Set Prescaler Value, 32K/128 = 250 Hz                                                                   */
    WDT_SetPrescaler(WDT_PRESCALER_128);
    /* Set Prescaler Value, 250 Hz/4000 = 0.0625 Hz                                                            */
    WDT_SetReloadValue(4000);
    /* Set Delta Value, 250 Hz/4000 = 0.0625 Hz                                                               */
    WDT_SetDeltaValue(4000);
    WDT_Restart();          // Reload Counter as WDTV Value
    WDT_Cmd(ENABLE);        // Enable WDT
    WDT_ProtectCmd(ENABLE); // Enable WDT Protection
}

void gpio_config(void)
{
    /* Config relay */
    CKCU_PeripClockConfig_TypeDef CKCUClock = {{ 0 }};
    CKCUClock.Bit.AFIO = 1;
    CKCUClock.Bit.PA = 1;
    CKCUClock.Bit.PB = 1;
    CKCUClock.Bit.PC = 1;
    CKCUClock.Bit.PD = 1;
    CKCU_PeripClockConfig(CKCUClock, ENABLE);

    /* Config relay */
    AFIO_GPxConfig(RELAY_AFIO_PORT, RELAY_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(RELAY_PORT, RELAY_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(RELAY_PORT, RELAY_PIN, RESET);
    GPIO_DirectionConfig(RELAY_PORT, RELAY_PIN, GPIO_DIR_OUT);
    
    /* Config 3 button */
    AFIO_GPxConfig(BTN0_AFIO_PORT, BTN0_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(BTN0_PORT, BTN0_PIN, GPIO_PR_UP);
    GPIO_DirectionConfig(BTN0_PORT, BTN0_PIN, GPIO_DIR_IN);
    GPIO_InputConfig(BTN0_PORT, BTN0_PIN, ENABLE);
    
    AFIO_GPxConfig(BTN1_AFIO_PORT, BTN1_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(BTN1_PORT, BTN1_PIN, GPIO_PR_UP);
    GPIO_DirectionConfig(BTN1_PORT, BTN1_PIN, GPIO_DIR_IN);
    GPIO_InputConfig(BTN1_PORT, BTN1_PIN, ENABLE);
    
    AFIO_GPxConfig(BTN2_AFIO_PORT, BTN2_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(BTN2_PORT, BTN2_PIN, GPIO_PR_UP);
    GPIO_DirectionConfig(BTN2_PORT, BTN2_PIN, GPIO_DIR_IN);
    GPIO_InputConfig(BTN2_PORT, BTN2_PIN, ENABLE);
    
    /* Config LCD */
    AFIO_GPxConfig(LCD_D0_AFIO_PORT, LCD_D0_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_D0_PORT, LCD_D0_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_D0_PORT, LCD_D0_PIN, RESET);
    GPIO_DirectionConfig(LCD_D0_PORT, LCD_D0_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(LCD_D1_AFIO_PORT, LCD_D1_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_D1_PORT, LCD_D1_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_D1_PORT, LCD_D1_PIN, RESET);
    GPIO_DirectionConfig(LCD_D1_PORT, LCD_D1_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(LCD_D2_AFIO_PORT, LCD_D2_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_D2_PORT, LCD_D2_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_D2_PORT, LCD_D2_PIN, RESET);
    GPIO_DirectionConfig(LCD_D2_PORT, LCD_D2_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(LCD_D3_AFIO_PORT, LCD_D3_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_D3_PORT, LCD_D3_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_D3_PORT, LCD_D3_PIN, RESET);
    GPIO_DirectionConfig(LCD_D3_PORT, LCD_D3_PIN, GPIO_DIR_OUT);
    
    
    AFIO_GPxConfig(LCD_RS_AFIO_PORT, LCD_RS_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_RS_PORT, LCD_RS_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_RS_PORT, LCD_RS_PIN, RESET);
    GPIO_DirectionConfig(LCD_RS_PORT, LCD_RS_PIN, GPIO_DIR_OUT);
    
    AFIO_GPxConfig(LCD_EN_AFIO_PORT, LCD_EN_AFIO_PIN, AFIO_FUN_GPIO);
    GPIO_PullResistorConfig(LCD_EN_PORT, LCD_EN_PIN, GPIO_PR_UP);
    GPIO_WriteOutBits(LCD_EN_PORT, LCD_EN_PIN, RESET);
    GPIO_DirectionConfig(LCD_EN_PORT, LCD_EN_PIN, GPIO_DIR_OUT);
}

void on_button_event_cb(int32_t button_pin, int32_t event, void *data)
{    
    static const char *event_name[] = {"pressed", "release", "hold", "hold so long", "double click", "tripple click", "idle", "idle break", "invalid"};
    DEBUG_INFO("Button %d %s\r\n", button_pin, event_name[event]);
    switch (event)
    {
        case APP_BTN_EVT_PRESSED:
        {
            switch (m_btn_state)
            {
                case BUTTON_SETUP_STATE_INIT:
                    if (button_pin == BUTTON_ENTER_INDEX)
                    {
                        m_btn_state = BUTTON_SETUP_STATE_T_HIGH;
                        tmp_cfg_t_high = app_flash_load_cfg()->temp_high;
                        tmp_cfg_t_low = app_flash_load_cfg()->temp_low;
                        lcd_blinking = true;
                        app_sync_register_callback(task_blink_lcd,
                                                   500,
                                                   SYNC_DRV_REPEATED,
                                                   SYNC_DRV_SCOPE_IN_LOOP);
                    }
                    break;
                    
                case BUTTON_SETUP_STATE_T_HIGH:
                    if (button_pin == BUTTON_ENTER_INDEX)
                    {
                        m_btn_state = BUTTON_SETUP_STATE_T_LOW;
                    }
                    else if (button_pin == BUTTON_UP_INDEX)
                    {
                        tmp_cfg_t_high++;
                        DEBUG_INFO("Increase T high\r\n");
                    }
                    else
                    {
                        tmp_cfg_t_high--;
                        DEBUG_INFO("Decrease T high\r\n");
                    }
                    break;
                    
                case BUTTON_SETUP_STATE_T_LOW:
                    if (button_pin == BUTTON_ENTER_INDEX)
                    {
                        m_btn_state = BUTTON_SETUP_STATE_FINISH;
                        app_flash_data_t cfg;
                        cfg.temp_high = tmp_cfg_t_high;
                        cfg.temp_low = tmp_cfg_t_low;
                        app_flash_store_data(&cfg);
                        DEBUG_INFO("Stored data to flash\r\n");
                        m_btn_state = BUTTON_SETUP_STATE_INIT;
                        char tmp[16];
                        sprintf(tmp, "H  %02u,  L  %u  ", tmp_cfg_t_high, tmp_cfg_t_low); 
                        Lcd_cursor(&lcd, 1,0);
                        Lcd_string(&lcd, tmp);
                        lcd_blinking = false;
                    }
                    else if (button_pin == BUTTON_UP_INDEX)
                    {
                        tmp_cfg_t_low++;
                        DEBUG_INFO("Increase T low\r\n");
                    }
                    else
                    {
                        tmp_cfg_t_low--;
                        DEBUG_INFO("Decrease T low\r\n");
                    }
                    break;
                    
                case BUTTON_SETUP_STATE_FINISH:
                    if (button_pin == BUTTON_ENTER_INDEX)
                    {
                    }
                    break;
                    
                default:
                    break;
            }
        }
            break;

        case APP_BTN_EVT_RELEASED:
        {
            switch (m_btn_state)
            {
                case BUTTON_SETUP_STATE_INIT:
                    break;
                case BUTTON_SETUP_STATE_T_HIGH:
                    break;
                case BUTTON_SETUP_STATE_T_LOW:
                    break;
                case BUTTON_SETUP_STATE_FINISH:
                    break;
                default:
                    break;
            }
        }
            break;

        case APP_BTN_EVT_HOLD:
            break;

        case APP_BTN_EVT_DOUBLE_CLICK:
        case APP_BTN_EVT_TRIPLE_CLICK:
        {
            
        }
            break;

        case APP_BTN_EVT_HOLD_SO_LONG:
        {
            
        }
            break;

        default:
            DEBUG_INFO("[%s] Unhandle button event %d\r\n", __FUNCTION__, event);
            break;
    }
}

uint32_t board_hw_button_read(uint32_t pin)
{
    uint32_t val = 0;
    if (pin == 0)
    {
        val = GPIO_ReadInBit(BTN0_PORT, BTN0_PIN) ? 1 : 0;
    }
    else if(pin == 1)
    {
        val = GPIO_ReadInBit(BTN1_PORT, BTN1_PIN) ? 1 : 0;
    }
    else if (pin == 2)
    {
        val = GPIO_ReadInBit(BTN2_PORT, BTN2_PIN) ? 1 : 0;
    }
    return val;
}

void btn_configuration(void)
{
    static app_btn_hw_config_t m_hw_button_initialize_value[APP_BTN_MAX_BTN_SUPPORT];

    for (uint32_t i = 0; i < APP_BTN_MAX_BTN_SUPPORT; i++)
    {
        m_hw_button_initialize_value[i].idle_level = 1;
        m_hw_button_initialize_value[i].last_state = board_hw_button_read(i);
        m_hw_button_initialize_value[i].pin = i;
    }

    app_btn_config_t conf;
    conf.config = &m_hw_button_initialize_value[0];
    conf.btn_count = APP_BTN_MAX_BTN_SUPPORT;
    conf.get_tick_cb = sys_get_tick_ms;
    conf.btn_initialize = NULL;
    conf.btn_read = board_hw_button_read;

    app_btn_initialize(&conf);
    app_btn_register_callback(APP_BTN_EVT_PRESSED, on_button_event_cb, NULL);
    app_btn_register_callback(APP_BTN_EVT_RELEASED, on_button_event_cb, NULL);
    app_btn_register_callback(APP_BTN_EVT_TRIPLE_CLICK, on_button_event_cb, NULL);
}

uint32_t sys_get_tick_ms(void)
{
    return m_sys_tick;
}

static void task_blink_lcd(void *arg)
{
    if (!lcd_blinking)
    {
        return;
    }
    lcd_blinking = true;
    static uint32_t i = 0;
    char tmp[32];
    if (i++ %2 == 0)
    {
        if (m_btn_state == BUTTON_SETUP_STATE_T_HIGH)
        {
            sprintf(tmp, "H  %02u,  L  %u  ", tmp_cfg_t_high, tmp_cfg_t_low);      
        }
        else if (m_btn_state == BUTTON_SETUP_STATE_T_LOW)
        {
            sprintf(tmp, "H  %02u,  L  %02u  ", tmp_cfg_t_high, tmp_cfg_t_low); 
        }            
    }
    else
    {
        if (m_btn_state == BUTTON_SETUP_STATE_T_HIGH)
        {
            sprintf(tmp, "H    ,  L  %u  ", tmp_cfg_t_low); 
        }
        else if (m_btn_state == BUTTON_SETUP_STATE_T_LOW)
        {
            sprintf(tmp, "H  %02u,  L    ", tmp_cfg_t_high); 
        }  
    }
    
    Lcd_cursor(&lcd, 1,0);
    Lcd_string(&lcd, tmp);
}

///*********************************************************************************************************//**
//  * @brief  Read and write EEPROM to check the entire data.
//  * @retval None
//  ***********************************************************************************************************/
//void EEPROM_WriteReadTest(void)
//{
//  u32 i, j, *pBuf;

//  for (j = 0; j < EEPROM_PAGE_CNT; j++)
//  {
//    /* generate test pattern                                                                                */
//    pBuf = (u32 *)gWriteBuf;
//    for (i = 0; i < (I2C_EEPROM_PAGE_SIZE / 4); i++)
//    {
//      *pBuf++ = rand();
//    }

//    /* Clean read buffer as 0xC3C3C3C3                                                                      */
//    pBuf = (u32 *)gReadBuf;
//    for (i = 0; i < (I2C_EEPROM_PAGE_SIZE / 4); i++)
//    {
//      *pBuf++ = 0xC3C3C3C3;
//    }

//    /* Write data                                                                                           */
//    I2C_EEPROM_BufferWrite((u8 *)gWriteBuf, I2C_EEPROM_PAGE_SIZE * j, I2C_EEPROM_PAGE_SIZE);

//    /* Read data                                                                                            */
//    I2C_EEPROM_BufferRead((u8 *)gReadBuf, I2C_EEPROM_PAGE_SIZE * j, I2C_EEPROM_PAGE_SIZE);

//    /* Compare Data                                                                                         */
//    for (i = 0; i < (I2C_EEPROM_PAGE_SIZE); i++)
//    {
//      if (gReadBuf[i] != gWriteBuf[i])
//      {
//        printf(" [Failed]\n");
//        while (1);
//      }
//    }

//    printf("\r\t\t\t\t%d%%", (int)((j + 1) * 100)/EEPROM_PAGE_CNT);
//  }

//  printf(" [Pass]\n");
//}

#if (HT32_LIB_DEBUG == 1)
/*********************************************************************************************************//**
  * @brief  Report both the error name of the source file and the source line number.
  * @param  filename: pointer to the source file name.
  * @param  uline: error line source number.
  * @retval None
  ***********************************************************************************************************/
void assert_error(u8* filename, u32 uline)
{
  /*
     This function is called by IP library that the invalid parameters has been passed to the library API.
     Debug message can be added here.
     Example: printf("Parameter Error: file %s on line %d\r\n", filename, uline);
  */
    NVIC_SystemReset();
    while (1)
    {
    }
}
#endif


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
